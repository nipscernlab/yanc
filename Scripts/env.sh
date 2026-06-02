#!/usr/bin/env bash
# ****************************************************************************
# Shared environment loader for the go_*.sh scripts (Linux).
#
# POSIX counterpart of Scripts/env.bat. Resolves the repository root and the
# locations of the simulation tools (Icarus, Verilator, GTKWave) WITHOUT any
# hardcoded paths:
#
#   1. Loads Scripts/tools.local.sh if present -- the cache that
#      Scripts/setup.sh writes after verifying / installing the tools.
#   2. For anything not in that cache, falls back to a PATH lookup, so the
#      go_*.sh still work if the tools are already on PATH.
#
# Meant to be `source`d (not executed) so the variables reach the caller:
#
#   ROOT_DIR  repo root          YANC_BIN  <repo>/bin (the built binaries)
#   IVERILOG  iverilog           VVP       vvp
#   VERILATOR verilator          GTKWAVE   gtkwave        FST2VCD  fst2vcd
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

export ROOT_DIR YANC_BIN IVERILOG VVP VERILATOR GTKWAVE FST2VCD
