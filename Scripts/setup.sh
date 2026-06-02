#!/usr/bin/env bash
# ****************************************************************************
# YANC setup (Linux) -- one-time preparation for running the go_*.sh examples.
#
# POSIX counterpart of Scripts/setup.bat. Run it once after cloning the repo
# and it will:
#
#   1. Locate the build toolchain (gcc / bison / flex) and, if a supported
#      package manager is present, offer to install whatever is missing.
#   2. Compile the YANC binaries into <repo>/bin (kept if already built,
#      unless --rebuild is given). There is no prebuilt Linux release, so the
#      source build is the path here.
#   3. Locate the simulators (Icarus Verilog, Verilator) and GTKWave, offering
#      to install the missing ones via the package manager.
#   4. Cache every resolved path in Scripts/tools.local.sh, which the go_*.sh
#      load through Scripts/env.sh. No path is hardcoded anywhere.
#
# Flags:
#   setup.sh             auto: build the binaries if they are not there yet
#   setup.sh --rebuild   force a fresh compile from source
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
    "" ) ;;
    * ) echo "usage: $0 [--rebuild]"; exit 2 ;;
esac

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
want bison bison required || build_ok=0
want flex  flex  required || build_ok=0

bins_present=0
if [ -x "$YANC_BIN/cmmcomp" ] && [ -x "$YANC_BIN/asmcomp" ] && \
   [ -x "$YANC_BIN/appcomp" ] && [ -x "$YANC_BIN/gen_gtkw" ]; then
    bins_present=1
fi

# ---------------------------------------------------------------------------
# 2) Build the binaries into <repo>/bin --------------------------------------
# ---------------------------------------------------------------------------
build_bins() {
    echo "[build] Compiling YANC binaries into $YANC_BIN"
    mkdir -p "$YANC_BIN"

    ( cd "$ROOT_DIR/Compilers/CMMComp/Sources" \
      && bison -y -d CMMComp.y && flex CMMComp.l \
      && gcc -O2 -o "$YANC_BIN/cmmcomp" \
            ast.c data_assign.c data_declar.c data_use.c itr.c diretivas.c \
            funcoes.c labels.c lex.yy.c oper.c saltos.c stdlib.c t2t.c \
            variaveis.c array_index.c global.c macros.c messages.c args.c y.tab.c \
            -lm \
      && rm -f lex.yy.c y.tab.c y.tab.h ) || return 1

    ( cd "$ROOT_DIR/Compilers/APPComp/Sources" \
      && flex -o app.c app.l \
      && gcc -O2 -o "$YANC_BIN/appcomp" app.c eval.c variaveis.c messages.c args.c -lm \
      && rm -f app.c ) || return 1

    ( cd "$ROOT_DIR/Compilers/ASMComp/Sources" \
      && flex -o ASMComp.c ASMComp.l \
      && gcc -O2 -o "$YANC_BIN/asmcomp" \
            ASMComp.c eval.c labels.c opcodes.c variaveis.c t2t.c hdl.c \
            simulacao.c array.c messages.c args.c -lm \
      && rm -f ASMComp.c ) || return 1

    ( cd "$ROOT_DIR/Compilers/CPPComp/Sources" \
      && gcc -O2 -Wall -o "$YANC_BIN/cpppp" cpppp.c \
      && bison -y -d CPPComp.y && flex CPPComp.l \
      && gcc -O2 -Wall -Wno-unused-but-set-variable -Wno-unused-variable \
            -Wno-unused-function -o "$YANC_BIN/cppcomp" \
            main.c messages.c types.c symtab.c ast.c codegen.c lex.yy.c y.tab.c -lm \
      && rm -f lex.yy.c y.tab.c y.tab.h ) || return 1

    ( cd "$ROOT_DIR/Scripts" \
      && gcc -O2 -o "$YANC_BIN/comp2gtkw" comp2gtkw.c -lm \
      && gcc -O2 -o "$YANC_BIN/gen_gtkw"  gen_gtkw.c  -lm ) || return 1

    echo "[build] Done."
}

echo
echo "--- YANC binaries -------------------------------------------------------"
if [ "$FORCE" = build ]; then
    [ "$build_ok" = 1 ] || { echo "ERROR: cannot build - gcc/bison/flex missing."; exit 1; }
    build_bins || { echo "ERROR: build failed."; exit 1; }
elif [ "$bins_present" = 1 ]; then
    echo "Binaries already present in $YANC_BIN - keeping them (use --rebuild to recompile)."
elif [ "$build_ok" = 1 ]; then
    build_bins || { echo "ERROR: build failed."; exit 1; }
else
    echo "ERROR: binaries missing and gcc/bison/flex not available - install them and re-run."
    exit 1
fi

# ---------------------------------------------------------------------------
# 3) Simulation tools + GTKWave ----------------------------------------------
# ---------------------------------------------------------------------------
echo
echo "--- Simulation tools ----------------------------------------------------"
want iverilog  iverilog  optional "needed for the Icarus flow: go_proc.sh / go_proj.sh"   || true
want verilator verilator optional "needed for the Verilator flow: go_proc_vl.sh / go_proj_vl.sh" || true
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
echo "   ./go_proc.sh        (one processor,      Icarus)"
echo "   ./go_proj.sh        (multi-proc project, Icarus)"
echo "   ./go_proc_vl.sh     (one processor,      Verilator)"
echo "   ./go_proj_vl.sh     (multi-proc project, Verilator)"
echo "============================================================"
