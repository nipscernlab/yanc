#!/usr/bin/env bash
# ****************************************************************************
# Shared environment loader for the runner scripts (single_proc.sh,
# multi_proc.sh, regress.sh, fcond/run.sh).
#
# POSIX counterpart of Scripts/env.bat. Resolves the repository root and the
# locations of the simulation tools (Icarus, Verilator, GTKWave) WITHOUT any
# hardcoded paths:
#
#   1. Loads Scripts/tools.local.sh if present -- the cache that
#      Scripts/setup.sh writes after verifying / installing the tools.
#   2. For anything not in that cache, falls back to a PATH lookup, so the
#      runners still work if the tools are already on PATH.
#
# Meant to be `source`d (not executed) so the variables reach the caller:
#
#   ROOT_DIR  repo root          YANC_BIN  <repo>/bin (the built binaries)
#   IVERILOG  iverilog           VVP       vvp
#   VERILATOR verilator          GTKWAVE   gtkwave        FST2VCD  fst2vcd
#   YANC_VL_CFLAGS   what every Verilator call adds as -CFLAGS (see below)
# ****************************************************************************

# Resolve repo root from this file's location (works whether sourced or run).
_yanc_env_self="${BASH_SOURCE[0]:-$0}"
SCRIPTS_DIR="$(cd "$(dirname "$_yanc_env_self")" && pwd)"
ROOT_DIR="$(cd "$SCRIPTS_DIR/.." && pwd)"
YANC_BIN="$ROOT_DIR/bin"

# Resolved tool paths cached by Scripts/setup.sh (machine-specific, gitignored)
[ -f "$SCRIPTS_DIR/tools.local.sh" ] && . "$SCRIPTS_DIR/tools.local.sh"

# PATH fallbacks for anything the cache did not provide
: "${IVERILOG:=$(command -v iverilog  2>/dev/null || true)}"
: "${VVP:=$(command -v vvp            2>/dev/null || true)}"
: "${VERILATOR:=$(command -v verilator 2>/dev/null || true)}"
: "${GTKWAVE:=$(command -v gtkwave    2>/dev/null || true)}"
: "${FST2VCD:=$(command -v fst2vcd    2>/dev/null || true)}"

# MSYS2's libstdc++ (gcc 16.1 and 16.2, MINGW64 and UCRT64) lacks the
# std::string move constructor that Verilator's code needs at -Os. With
# _GLIBCXX_ASSERTIONS libstdc++ drops its extern template for basic_string,
# so the compiler instantiates the constructor itself (#117). Every Verilator
# call passes it as -CFLAGS; empty on Linux. A value already set is kept.
if [ -z "${YANC_VL_CFLAGS+set}" ]; then
    case "$(uname -s)" in
        MINGW*|MSYS*|UCRT*|CLANG*) YANC_VL_CFLAGS="-D_GLIBCXX_ASSERTIONS" ;;
        *)                         YANC_VL_CFLAGS="" ;;
    esac
fi

export ROOT_DIR YANC_BIN IVERILOG VVP VERILATOR GTKWAVE FST2VCD YANC_VL_CFLAGS
