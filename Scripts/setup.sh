#!/usr/bin/env bash
# ****************************************************************************
# YANC setup (Linux, MSYS2) -- one-time prep for the single_proc.sh / multi_proc.sh runners.
#
# POSIX counterpart of Scripts/setup.bat, for Linux and for MSYS2's UCRT64
# terminal on Windows. Run it once after cloning the repo and it will:
#
#   1. Locate the build toolchain (gcc / bison / flex) and, if a supported
#      package manager is present, offer to install whatever is missing.
#   2. Produce the YANC binaries in <repo>/bin :
#        * if they are already there, keep them
#        * else if the build toolchain is present, COMPILE them from source
#        * else DOWNLOAD the prebuilt binaries of this system from the latest release
#   3. Locate the simulators (Icarus Verilog, Verilator) and GTKWave, offering
#      to install the missing ones via the package manager.
#   4. Cache every resolved path in Scripts/tools.local.sh, which the runners
#      load through Scripts/env.sh. No path is hardcoded anywhere.
#
# Flags:
#   setup.sh             auto: keep / build / download as described above
#   setup.sh --rebuild   force a fresh compile from source
#   setup.sh --download  force fetching the prebuilt binaries
#
# GTKWave comes from the package manager too (MSYS2's or the distribution's).
# The runners give it only the options it knows (gtkwave_opts in env.sh).
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

# Latest release on GitHub (prebuilt binaries: yanc-bin-linux-*.tar.gz for
# Linux, yanc-bin-*.zip for Windows).
RELEASE_API="https://api.github.com/repos/nipscernlab/yanc/releases/latest"

echo "============================================================"
echo " YANC setup"
echo " repo : $ROOT_DIR"
echo "============================================================"

# ---------------------------------------------------------------------------
# Package manager detection --------------------------------------------------
# ---------------------------------------------------------------------------
PKG=""; PKG_INSTALL=""
if   command -v apt-get >/dev/null 2>&1; then PKG=apt;    PKG_INSTALL="sudo apt-get install -y"
elif command -v dnf     >/dev/null 2>&1; then PKG=dnf;    PKG_INSTALL="sudo dnf install -y"
elif command -v pacman  >/dev/null 2>&1 && [ -n "${MINGW_PACKAGE_PREFIX:-}" ]; then
    # MSYS2 (UCRT64, MINGW64, CLANG64...): no sudo, and the native tools carry
    # the environment's prefix, e.g. mingw-w64-ucrt-x86_64-iverilog (#117)
    PKG=msys2; PKG_INSTALL="pacman -S --needed --noconfirm"
elif command -v pacman  >/dev/null 2>&1; then PKG=pacman; PKG_INSTALL="sudo pacman -S --needed --noconfirm"
elif command -v zypper  >/dev/null 2>&1; then PKG=zypper; PKG_INSTALL="sudo zypper install -y"
fi
if [ -n "$PKG" ]; then echo "package manager: $PKG"; else echo "package manager: none detected (manual installs only)"; fi

# Map a logical tool name to the package providing it, for the detected manager.
pkg_name() {
    if [ "$PKG" = msys2 ]; then
        case "$1" in
            gcc|iverilog|python|lz4)       echo "$MINGW_PACKAGE_PREFIX-$1" ;;
            # MSYS2's Verilator does not depend on lz4, but its FST writer
            # (--trace-fst, multi_proc) includes lz4.h; and --binary builds
            # the C++ it writes with make and g++, through a Python script
            verilator) echo "$MINGW_PACKAGE_PREFIX-verilator $MINGW_PACKAGE_PREFIX-lz4 make $MINGW_PACKAGE_PREFIX-gcc $MINGW_PACKAGE_PREFIX-python" ;;
            gtkwave)   echo "$MINGW_PACKAGE_PREFIX-gtkwave" ;;
            make|bison|flex|diffutils|unzip) echo "$1" ;;
        esac
        return
    fi
    case "$1" in
        gcc)       case "$PKG" in apt) echo build-essential ;; *) echo gcc ;; esac ;;
        make)      case "$PKG" in apt) echo build-essential ;; *) echo make ;; esac ;;
        bison)     echo bison ;;
        flex)      echo flex ;;
        iverilog)  echo iverilog ;;
        verilator) echo verilator ;;
        gtkwave)   echo gtkwave ;;
        diffutils) echo diffutils ;;
        python)    case "$PKG" in apt|dnf|zypper) echo python3 ;; *) echo python ;; esac ;;
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
   [ -x "$YANC_BIN/cppcomp" ] && [ -x "$YANC_BIN/gen_gtkw" ]; then
    bins_present=1
fi

# C++ toolchain sanity (Verilator needs a working libstdc++) ------------------
# MSYS2's gcc 16 (16.1 and 16.2, MINGW64 and UCRT64) ships a libstdc++ whose
# std::string move constructor is missing, both from the DLL and from
# libstdc++.a; it shows at -Os, which Verilator's build uses. With
# -D_GLIBCXX_ASSERTIONS the compiler instantiates the constructor itself, and
# the runners pass that to Verilator under MSYS2 (Scripts/env.sh,
# YANC_VL_CFLAGS; #117). Probe without it, then with it. Non-fatal: the
# Icarus flow does not need g++.
check_cxx_toolchain() {
    command -v g++ >/dev/null 2>&1 || return 0
    local t; t="${TMPDIR:-/tmp}/yanc_cxx_probe.$$"; mkdir -p "$t" 2>/dev/null
    printf '#include <string>\nint main(){std::string a="x";std::string b=std::move(a)+"y";return b.size()?0:1;}\n' > "$t/probe.cpp"
    local ver; ver="$(g++ -dumpfullversion 2>/dev/null || echo '?')"
    if g++ -Os -o "$t/probe" "$t/probe.cpp" >/dev/null 2>&1; then
        :
    elif g++ -Os -D_GLIBCXX_ASSERTIONS -o "$t/probe" "$t/probe.cpp" >/dev/null 2>&1; then
        echo "[g++]       $ver lacks the std::string move constructor at -Os; the runners"
        echo "            pass -D_GLIBCXX_ASSERTIONS to Verilator, which works around it."
    else
        echo
        echo "**************************************************************************"
        echo " WARNING: g++ $ver cannot link a std::string move, even with"
        echo " -D_GLIBCXX_ASSERTIONS: the Verilator flow will fail. The Icarus flow is"
        echo " unaffected. See docs/install.md."
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

# Pull the latest release asset of this system (yanc-bin-linux-*.tar.gz on
# Linux, yanc-bin-*.zip under MSYS2) and unpack its bin/ into <repo>/bin --
# the counterpart of setup.bat's :download_bins.
download_bins() {
    local pattern kind
    if [ "$PKG" = msys2 ]; then
        pattern='https://[^"]*/yanc-bin-v[^"]*\.zip'; kind=Windows
        command -v unzip >/dev/null 2>&1 || want unzip unzip optional "unpacks the Windows release" \
            || { echo "[download] 'unzip' not found; cannot unpack the release archive."; return 1; }
    else
        pattern='https://[^"]*/yanc-bin-linux-[^"]*\.tar\.gz'; kind=Linux
        command -v tar >/dev/null 2>&1 || { echo "[download] 'tar' not found; cannot unpack the release archive."; return 1; }
    fi
    echo "[download] Fetching the prebuilt $kind binaries from the latest GitHub release..."
    if ! command -v curl >/dev/null 2>&1 && ! command -v wget >/dev/null 2>&1; then
        echo "[download] need curl or wget to download; install one, or use --rebuild to build from source."
        return 1
    fi
    # The browser_download_url carries the asset filename, so grep the URL directly.
    local url
    url="$(_fetch_stdout "$RELEASE_API" 2>/dev/null | grep -oE "$pattern" | head -1)"
    if [ -z "$url" ]; then
        echo "[download] no $kind asset in the latest release."
        echo "           Download one from https://github.com/nipscernlab/yanc/releases"
        echo "           and extract its bin/ into $YANC_BIN."
        return 1
    fi
    echo "[download] $url"
    local tmp; tmp="$(mktemp -d)"
    if ! _fetch_file "$url" "$tmp/${url##*/}"; then
        echo "[download] download failed."; rm -rf "$tmp"; return 1
    fi
    if [ "$PKG" = msys2 ]; then
        unzip -q "$tmp/${url##*/}" -d "$tmp"
    else
        tar -xzf "$tmp/${url##*/}" -C "$tmp"
    fi || { echo "[download] could not unpack the archive."; rm -rf "$tmp"; return 1; }
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
    echo "No build toolchain (gcc/bison/flex) - falling back to the prebuilt release."
    download_bins || { echo "ERROR: no binaries, no toolchain, and download failed - install gcc/bison/flex or download manually."; exit 1; }
fi

# ---------------------------------------------------------------------------
# 3) Simulation tools + GTKWave ----------------------------------------------
# ---------------------------------------------------------------------------
echo
echo "--- Simulation tools ----------------------------------------------------"
want iverilog  iverilog  optional "needed for the Icarus flow: single_proc.sh / multi_proc.sh"   || true
want verilator verilator optional "needed for the Verilator flow: single_proc.sh --sim verilator" || true
# A Verilator installed before lz4 was asked for: the FST trace of
# multi_proc.sh --sim verilator stops at "lz4.h: No such file or directory".
if [ "$PKG" = msys2 ] && command -v verilator >/dev/null 2>&1 && [ ! -f "${MINGW_PREFIX:-/ucrt64}/include/lz4.h" ]; then
    echo
    echo "[setup] lz4.h not found (Verilator's FST trace needs it: multi_proc.sh --sim verilator)."
    ans=""
    read -r -p "        Install '$(pkg_name lz4)' via $PKG now? [y/N] " ans
    case "$ans" in
        y|Y) eval "$PKG_INSTALL $(pkg_name lz4)" || echo "        install failed." ;;
    esac
fi
want gtkwave   gtkwave   optional "needed to view the waveform" || true
want cmp       diffutils optional "Scripts/regress.sh compares files with cmp and diff"   || true
want python3   python    optional "Scripts/regress.sh and its checks are Python"         || true

command -v iverilog  >/dev/null 2>&1 && echo "[icarus]    $(command -v iverilog)"
command -v verilator >/dev/null 2>&1 && echo "[verilator] $(command -v verilator)"
command -v gtkwave   >/dev/null 2>&1 && echo "[gtkwave]   $(command -v gtkwave)"

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
