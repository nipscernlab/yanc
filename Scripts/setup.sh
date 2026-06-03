#!/usr/bin/env bash
# ****************************************************************************
# YANC setup (Linux) -- one-time prep for the single_proc.sh / multi_proc.sh runners.
#
# POSIX counterpart of Scripts/setup.bat. Run it once after cloning the repo
# and it will:
#
#   1. Locate the build toolchain (gcc / bison / flex) and, if a supported
#      package manager is present, offer to install whatever is missing.
#   2. Produce the YANC binaries in <repo>/bin :
#        * if they are already there, keep them
#        * else if the build toolchain is present, COMPILE them from source
#        * else DOWNLOAD the prebuilt Linux binaries from the latest release
#   3. Locate the simulators (Icarus Verilog, Verilator) and GTKWave, offering
#      to install the missing ones via the package manager.
#   4. Cache every resolved path in Scripts/tools.local.sh, which the runners
#      load through Scripts/env.sh. No path is hardcoded anywhere.
#
# Flags:
#   setup.sh             auto: keep / build / download as described above
#   setup.sh --rebuild   force a fresh compile from source
#   setup.sh --download  force fetching the prebuilt Linux binaries
#
# NOTE on GTKWave: this uses your distro's gtkwave package. The Windows flow
# expects the nipscernlab GTKWave fork; the stock gtkwave still opens the
# waveform and applies the generated .gtkw, but its panel chrome differs.
# ****************************************************************************

set -uo pipefail

SCRIPTS_DIR="$(cd "$(dirname "${BASH_SOURCE[0]:-$0}")" && pwd)"
ROOT_DIR="$(cd "$SCRIPTS_DIR/.." && pwd)"
YANC_BIN="$ROOT_DIR/bin"
CACHE="$SCRIPTS_DIR/tools.local.sh"

FORCE=""
case "${1:-}" in
    --rebuild|--build) FORCE=build ;;
    --download)        FORCE=download ;;
    "" ) ;;
    * ) echo "usage: $0 [--rebuild|--download]"; exit 2 ;;
esac

# Latest release on GitHub (prebuilt Linux tarball: yanc-bin-linux-*.tar.gz).
RELEASE_API="https://api.github.com/repos/nipscernlab/yanc/releases/latest"

echo "============================================================"
echo " YANC setup (Linux)"
echo " repo : $ROOT_DIR"
echo "============================================================"

# ---------------------------------------------------------------------------
# Package manager detection --------------------------------------------------
# ---------------------------------------------------------------------------
PKG=""; PKG_INSTALL=""
if   command -v apt-get >/dev/null 2>&1; then PKG=apt;    PKG_INSTALL="sudo apt-get install -y"
elif command -v dnf     >/dev/null 2>&1; then PKG=dnf;    PKG_INSTALL="sudo dnf install -y"
elif command -v pacman  >/dev/null 2>&1; then PKG=pacman; PKG_INSTALL="sudo pacman -S --needed --noconfirm"
elif command -v zypper  >/dev/null 2>&1; then PKG=zypper; PKG_INSTALL="sudo zypper install -y"
fi
if [ -n "$PKG" ]; then echo "package manager: $PKG"; else echo "package manager: none detected (manual installs only)"; fi

# Map a logical tool name to the package providing it, for the detected manager.
pkg_name() {
    case "$1" in
        gcc)       case "$PKG" in apt) echo build-essential ;; *) echo gcc ;; esac ;;
        make)      case "$PKG" in apt) echo build-essential ;; *) echo make ;; esac ;;
        bison)     echo bison ;;
        flex)      echo flex ;;
        iverilog)  echo iverilog ;;
        verilator) echo verilator ;;
        gtkwave)   echo gtkwave ;;
    esac
}

# want <command> <logical-tool> <required|optional> <note>
# Returns 0 if the command is available afterwards.
want() {
    local cmd="$1" tool="$2" need="$3" note="${4:-}"
    if command -v "$cmd" >/dev/null 2>&1; then return 0; fi
    local p; p="$(pkg_name "$tool")"
    echo
    echo "[setup] '$cmd' not found.${note:+ ($note)}"
    if [ -n "$PKG_INSTALL" ] && [ -n "$p" ]; then
        local ans=""
        read -r -p "        Install '$p' via $PKG now? [y/N] " ans
        case "$ans" in
            y|Y) eval "$PKG_INSTALL $p" || echo "        install failed." ;;
        esac
    else
        echo "        Install it with your package manager (package: ${p:-$cmd})."
    fi
    command -v "$cmd" >/dev/null 2>&1
}

# ---------------------------------------------------------------------------
# 1) Build toolchain ---------------------------------------------------------
# ---------------------------------------------------------------------------
echo
echo "--- Build toolchain -----------------------------------------------------"
build_ok=1
want gcc   gcc   required || build_ok=0
want make  make  required || build_ok=0
want bison bison required || build_ok=0
want flex  flex  required || build_ok=0

bins_present=0
if [ -x "$YANC_BIN/cmmcomp" ] && [ -x "$YANC_BIN/asmcomp" ] && \
   [ -x "$YANC_BIN/appcomp" ] && [ -x "$YANC_BIN/gen_gtkw" ]; then
    bins_present=1
fi

# C++ toolchain sanity (Verilator needs a working libstdc++) ------------------
# MSYS2's mingw-w64-x86_64-gcc 16.1.0-5 shipped a broken libstdc++ whose
# std::string move-constructor symbol is missing, so Verilator's verilated.cpp
# fails to link. Probe it functionally (catches the real defect, not just one
# version string) and warn loudly if broken. Non-fatal: the Icarus flow is fine.
check_cxx_toolchain() {
    command -v g++ >/dev/null 2>&1 || return 0
    local t; t="${TMPDIR:-/tmp}/yanc_cxx_probe.$$"; mkdir -p "$t" 2>/dev/null
    printf '#include <string>\nint main(){std::string a="x";std::string b=std::move(a)+"y";return b.size()?0:1;}\n' > "$t/probe.cpp"
    if ! g++ -Os -o "$t/probe" "$t/probe.cpp" >/dev/null 2>&1; then
        local ver; ver="$(g++ -dumpfullversion 2>/dev/null || echo '?')"
        echo
        echo "**************************************************************************"
        echo " WARNING: g++ $ver cannot link std::string move -- the Verilator flow"
        echo " WILL FAIL. MSYS2's mingw-w64-x86_64-gcc 16.1.0-5 ships a broken"
        echo " libstdc++. Downgrade to a known-good gcc (15.1.0-5), e.g.:"
        echo "   pacman -U /var/cache/pacman/pkg/mingw-w64-x86_64-gcc-15.1.0-5-any.pkg.tar.zst \\"
        echo "             /var/cache/pacman/pkg/mingw-w64-x86_64-gcc-libs-15.1.0-5-any.pkg.tar.zst"
        echo " Do NOT use gcc 16.1.0-5. (The Icarus flow is unaffected.)"
        echo "**************************************************************************"
    fi
    rm -rf "$t" 2>/dev/null
}
check_cxx_toolchain

# ---------------------------------------------------------------------------
# 2) Build the binaries into <repo>/bin --------------------------------------
# ---------------------------------------------------------------------------
# The compile rules live in the top-level Makefile (single source of truth);
# setup just drives it. $1 may be "clean all" to force a full rebuild.
build_bins() {
    echo "[build] Building YANC binaries via make into $YANC_BIN"
    make -C "$ROOT_DIR" BIN="$YANC_BIN" ${1:-all}
}

# curl/wget shims so the download works with whichever is installed.
_fetch_stdout() {  # $1=url -> body on stdout
    if   command -v curl >/dev/null 2>&1; then curl -fsSL -H 'User-Agent: yanc-setup' "$1"
    elif command -v wget >/dev/null 2>&1; then wget -q --header='User-Agent: yanc-setup' -O- "$1"
    else return 1; fi
}
_fetch_file() {    # $1=url $2=dest
    if   command -v curl >/dev/null 2>&1; then curl -fSL -H 'User-Agent: yanc-setup' -o "$2" "$1"
    elif command -v wget >/dev/null 2>&1; then wget -q --header='User-Agent: yanc-setup' -O "$2" "$1"
    else return 1; fi
}

# Pull the latest yanc-bin-linux-*.tar.gz release asset and unpack its bin/ into
# <repo>/bin -- the Linux counterpart of setup.bat's :download_bins.
download_bins() {
    echo "[download] Fetching prebuilt Linux binaries from the latest GitHub release..."
    if ! command -v curl >/dev/null 2>&1 && ! command -v wget >/dev/null 2>&1; then
        echo "[download] need curl or wget to download; install one, or use --rebuild to build from source."
        return 1
    fi
    if ! command -v tar >/dev/null 2>&1; then
        echo "[download] 'tar' not found; cannot unpack the release archive."
        return 1
    fi
    # The browser_download_url carries the asset filename, so grep the URL directly.
    local url
    url="$(_fetch_stdout "$RELEASE_API" 2>/dev/null \
           | grep -oE 'https://[^"]*yanc-bin-linux-[^"]*\.tar\.gz' | head -1)"
    if [ -z "$url" ]; then
        echo "[download] no yanc-bin-linux-*.tar.gz asset in the latest release."
        echo "           Download one from https://github.com/nipscernlab/yanc/releases"
        echo "           and extract its bin/ into $YANC_BIN."
        return 1
    fi
    echo "[download] $url"
    local tmp; tmp="$(mktemp -d)"
    if ! _fetch_file "$url" "$tmp/yanc-linux.tar.gz"; then
        echo "[download] download failed."; rm -rf "$tmp"; return 1
    fi
    if ! tar -xzf "$tmp/yanc-linux.tar.gz" -C "$tmp"; then
        echo "[download] could not unpack the archive."; rm -rf "$tmp"; return 1
    fi
    if [ ! -d "$tmp/bin" ]; then
        echo "[download] archive has no bin/ folder."; rm -rf "$tmp"; return 1
    fi
    mkdir -p "$YANC_BIN"
    cp -f "$tmp"/bin/* "$YANC_BIN"/ || { echo "[download] copy into $YANC_BIN failed."; rm -rf "$tmp"; return 1; }
    chmod +x "$YANC_BIN"/* 2>/dev/null || true
    rm -rf "$tmp"
    echo "[download] installed prebuilt binaries into $YANC_BIN"
}

echo
echo "--- YANC binaries -------------------------------------------------------"
if [ "$FORCE" = build ]; then
    [ "$build_ok" = 1 ] || { echo "ERROR: cannot build - gcc/bison/flex missing."; exit 1; }
    build_bins "clean all" || { echo "ERROR: build failed."; exit 1; }
elif [ "$FORCE" = download ]; then
    download_bins || { echo "ERROR: could not obtain the binaries."; exit 1; }
elif [ "$bins_present" = 1 ]; then
    echo "Binaries already present in $YANC_BIN - keeping them (--rebuild recompiles, --download refetches)."
elif [ "$build_ok" = 1 ]; then
    build_bins || { echo "ERROR: build failed."; exit 1; }
else
    echo "No build toolchain (gcc/bison/flex) - falling back to the prebuilt Linux release."
    download_bins || { echo "ERROR: no binaries, no toolchain, and download failed - install gcc/bison/flex or download manually."; exit 1; }
fi

# ---------------------------------------------------------------------------
# 3) Simulation tools + GTKWave ----------------------------------------------
# ---------------------------------------------------------------------------
echo
echo "--- Simulation tools ----------------------------------------------------"
want iverilog  iverilog  optional "needed for the Icarus flow: single_proc.sh / multi_proc.sh"   || true
want verilator verilator optional "needed for the Verilator flow: single_proc.sh --sim verilator" || true
want gtkwave   gtkwave   optional "needed to view the waveform"                            || true

command -v iverilog  >/dev/null 2>&1 && echo "[icarus]    $(command -v iverilog)"
command -v verilator >/dev/null 2>&1 && echo "[verilator] $(command -v verilator)"
if command -v gtkwave >/dev/null 2>&1; then
    echo "[gtkwave]   $(command -v gtkwave)  (distro build; not the nipscernlab fork)"
fi

# ---------------------------------------------------------------------------
# 4) Cache the resolved paths for env.sh -------------------------------------
# ---------------------------------------------------------------------------
{
    echo "# Generated by Scripts/setup.sh - machine-specific tool paths."
    echo "# Do not commit. Re-run setup.sh to regenerate."
    for pair in "IVERILOG iverilog" "VVP vvp" "VERILATOR verilator" \
                "GTKWAVE gtkwave" "FST2VCD fst2vcd"; do
        # shellcheck disable=SC2086
        set -- $pair
        path="$(command -v "$2" 2>/dev/null || true)"
        [ -n "$path" ] && echo "$1=\"$path\"; export $1"
    done
} > "$CACHE"

echo
echo "============================================================"
echo " Setup complete. Paths cached in:"
echo "   $CACHE"
echo
echo " You can now run the examples, e.g.:"
echo "   ./single_proc.sh                   (one processor,      Icarus)"
echo "   ./multi_proc.sh                   (multi-proc project, Icarus)"
echo "   ./single_proc.sh --sim verilator   (one processor,      Verilator)"
echo "   ./multi_proc.sh --sim verilator   (multi-proc project, Verilator)"
echo "============================================================"
