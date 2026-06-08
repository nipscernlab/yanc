#!/usr/bin/env bash
# ****************************************************************************
# single_proc_cpp - end-to-end example of the YANC single-processor pipeline,
# C++ edition. POSIX counterpart of single_proc_cpp.bat: it feeds a C++ (.cpp)
# program through the C++ front end (cpppp -> cppcomp) instead of the C+- one
# (cmmcomp), then assembles, simulates and opens the result in GTKWave.
#
#   ./single_proc_cpp.sh                  # simulate with Icarus (default)
#   ./single_proc_cpp.sh --sim verilator  # simulate with Verilator (+YANC_TRACE)
#   ./single_proc_cpp.sh --no-view        # run the pipeline, skip GTKWave
#
# Reads the HDL, macros and binaries straight from the repo; the only files it
# creates live under Teste/ (gitignored). Run Scripts/setup.sh once first.
# ****************************************************************************
set -uo pipefail

SELF_DIR="$(cd "$(dirname "${BASH_SOURCE[0]:-$0}")" && pwd)"
. "$SELF_DIR/env.sh"
cd "$ROOT_DIR"

# --- Args: --sim iverilog|verilator (default iverilog), --no-view ------------
SIM=iverilog
VIEW=1
while [ $# -gt 0 ]; do
    case "$1" in
        --sim)     SIM="${2:-}"; shift 2 ;;
        --sim=*)   SIM="${1#--sim=}"; shift ;;
        --no-view) VIEW=0; shift ;;
        -h|--help) echo "usage: ${0##*/} [--sim iverilog|verilator] [--no-view]"; exit 0 ;;
        *) echo "[single_proc_cpp] unknown argument: $1"; exit 1 ;;
    esac
done
case "$SIM" in
    iverilog|icarus) SIM=iverilog ;;
    verilator|vl)    SIM=verilator ;;
    *) echo "[single_proc_cpp] --sim must be 'iverilog' or 'verilator' (got '$SIM')"; exit 1 ;;
esac

# --- Tools: binaries always, the chosen simulator, GTKWave only if viewing --
[ -x "$YANC_BIN/cppcomp" ] || { echo "[single_proc_cpp] binaries missing in $YANC_BIN - run Scripts/setup.sh first."; exit 1; }
if [ "$SIM" = iverilog ]; then
    [ -n "$IVERILOG" ] || { echo "[single_proc_cpp] iverilog not found - run Scripts/setup.sh (or install iverilog)."; exit 1; }
else
    [ -n "$VERILATOR" ] || { echo "[single_proc_cpp] verilator not found - run Scripts/setup.sh (or install verilator)."; exit 1; }
fi
[ "$VIEW" = 1 ] && [ -z "$GTKWAVE" ] && { echo "[single_proc_cpp] gtkwave not found - run Scripts/setup.sh, or pass --no-view."; exit 1; }

# --- What to compile/simulate (a project under Compilers/CPPComp/Tests) -----
PROC=proc_cpp             # processor type / project folder
FNAM=proc_cpp.cpp         # cpp filename where the program is defined
FRE_CLK=100               # processor operating frequency in MHz
NUM_CLK=100000            # number of clocks to simulate

# --- Repo sources, read in place (never written to) -------------------------
HDL_DIR="$ROOT_DIR/HDL"
MAC_DIR="$ROOT_DIR/Compilers/CMMComp/Includes"      # asmcomp macros (shared)
INC_DIR="$ROOT_DIR/Compilers/CPPComp/Includes"      # C++ headers
BIN_DIR="$YANC_BIN"
SRC_PROC="$ROOT_DIR/Compilers/CPPComp/Tests/$PROC"

# --- Work area: the only files this script creates (Teste/ is gitignored) ---
WORK="$ROOT_DIR/Teste"
rm -rf "$WORK"
PROC_DIR="$WORK/$PROC"
SOFT_DIR="$PROC_DIR/Software"; HARD_DIR="$PROC_DIR/Hardware"
TMP_DIR="$WORK/Temp"; TMP_PRO="$TMP_DIR/$PROC"
VL_DIR="$TMP_PRO/vl"

# The .cpp is the only input; cppcomp/asmcomp create Software/Hardware outputs.
mkdir -p "$TMP_PRO" "$SOFT_DIR"
cp "$SRC_PROC/Software/$FNAM" "$SOFT_DIR/"

# --- Compile: C++ -> asm -> Verilog -----------------------------------------
echo "#### Running the C++ preprocessor (cpppp)"
"$BIN_DIR/cpppp" -i "$SOFT_DIR/$FNAM" -o "$TMP_PRO/pp.cpp" -I "$INC_DIR" -I "$SOFT_DIR"

echo "#### Running the C++ compiler (cppcomp)"
"$BIN_DIR/cppcomp" -i "$TMP_PRO/pp.cpp" -p "$PROC_DIR" -n "$PROC" -t "$TMP_PRO"

echo "#### Running the Pre-assembler (appcomp)"
ASM_FILE="$SOFT_DIR/$PROC.asm"
"$BIN_DIR/appcomp" -i "$ASM_FILE" -t "$TMP_PRO"

echo "#### Running the Assembler (asmcomp)"
"$BIN_DIR/asmcomp" -i "$ASM_FILE" -p "$PROC_DIR" -d "$HDL_DIR" -m "$MAC_DIR" -t "$TMP_PRO" -f "$FRE_CLK" -c "$NUM_CLK"

# --- Build + run the simulation ---------------------------------------------
UPROC="$HARD_DIR/$PROC"
if [ "$SIM" = iverilog ]; then
    echo "#### Running Icarus"
    TB_MOD="${PROC}_tb"
    ( cd "$HDL_DIR" && "$IVERILOG" -s "$TB_MOD" -o "$TMP_PRO/$PROC.vvp" \
        "$TMP_PRO/${PROC}_tb.v" "$UPROC.v" addr_dec.v instr_dec.v processor.v core.v ula.v )

    echo "#### Running VVP"
    cp "${UPROC}_data.mif" "$TMP_PRO/"
    cp "${UPROC}_inst.mif" "$TMP_PRO/"
    cd "$TMP_PRO"
    "$VVP" "$PROC.vvp" +HEADER_ONLY
    cp "$TB_MOD.vcd" "${TB_MOD}_hdr.vcd"
    "$VVP" "$PROC.vvp" -fst
    WAVE_VCD="$TMP_PRO/$TB_MOD.vcd"; HDR_VCD="$TMP_PRO/${TB_MOD}_hdr.vcd"; GTKW_OUT="$TMP_PRO/$TB_MOD.gtkw"
else
    echo "#### Running Verilator (+define+YANC_TRACE, --binary --timing --trace)"
    "$VERILATOR" --binary --timing --trace +define+YANC_TRACE \
        --top-module "${PROC}_tb" \
        -Wno-lint -Wno-UNOPTFLAT -Wno-MULTIDRIVEN -Wno-BLKANDNBLK -Wno-WIDTH \
        -Wno-CASEINCOMPLETE -Wno-IMPLICIT -Wno-COMBDLY -Wno-STMTDLY -Wno-INFINITELOOP \
        --Mdir "$VL_DIR" \
        "$TMP_PRO/${PROC}_tb.v" "$UPROC.v" \
        "$HDL_DIR/processor.v" "$HDL_DIR/core.v" "$HDL_DIR/ula.v" \
        "$HDL_DIR/addr_dec.v" "$HDL_DIR/instr_dec.v"

    echo "#### Running the Verilator simulation"
    cd "$TMP_PRO"
    "$VL_DIR/V${PROC}_tb"
    WAVE_VCD="$TMP_PRO/${PROC}_tb.vcd"; HDR_VCD="$WAVE_VCD"; GTKW_OUT="$TMP_PRO/${PROC}_tb.gtkw"
fi

# --- View in GTKWave (layout built by gen_gtkw) -----------------------------
echo "#### Generating the .gtkw layout (gen_gtkw)"
"$BIN_DIR/gen_gtkw" "$HDR_VCD" "$GTKW_OUT" "$TMP_DIR" "$BIN_DIR/comp2gtkw"
if [ "$VIEW" = 0 ]; then
    echo "#### --no-view: skipping GTKWave. Waveform: $WAVE_VCD"
    cd "$ROOT_DIR"; exit 0
fi
echo "#### Launching GTKWave"
"$GTKWAVE" --dark --zoom-fit --left-justify "$WAVE_VCD" -a "$GTKW_OUT"

cd "$ROOT_DIR"
