#!/usr/bin/env bash
# ****************************************************************************
# single_proc - end-to-end example of the YANC single-processor pipeline.
#
# It takes one C+- (.cmm) program and runs the full compile pipeline that
# turns it into the synthesizable Verilog (.v) of a SAPHO processor -- the
# core you would synthesize onto an FPGA to actually run this code in
# hardware. It then runs the simulate-and-view routines: the simulator
# compiles the asm-generated testbench (<proc>_tb.v) together with that
# synthesizable <proc>.v, and the result is opened in GTKWave, where you can
# watch the processor execute its program instruction by instruction.
#
# The bundled example (proc_fft) computes an order-8 FFT, using the C+-
# language's bit-reversed ("inverted") addressing for the FFT data.
#
#   ./single_proc.sh                 # simulate with Icarus (default)
#   ./single_proc.sh --sim verilator # simulate with Verilator (+define+YANC_TRACE)
#
# Reads the HDL, macros and binaries straight from the repo; the only files it
# creates live under Teste/ (gitignored). Run Scripts/setup.sh once first.
# ****************************************************************************
set -uo pipefail

SELF_DIR="$(cd "$(dirname "${BASH_SOURCE[0]:-$0}")" && pwd)"
. "$SELF_DIR/env.sh"
cd "$ROOT_DIR"

# --- Pick the simulator (--sim iverilog|verilator, default iverilog) ---------
SIM=iverilog
while [ $# -gt 0 ]; do
    case "$1" in
        --sim)     SIM="${2:-}"; shift 2 ;;
        --sim=*)   SIM="${1#--sim=}"; shift ;;
        -h|--help) echo "usage: ${0##*/} [--sim iverilog|verilator]"; exit 0 ;;
        *) echo "[single_proc] unknown argument: $1 (try --sim iverilog|verilator)"; exit 1 ;;
    esac
done
case "$SIM" in
    iverilog|icarus) SIM=iverilog ;;
    verilator|vl)    SIM=verilator ;;
    *) echo "[single_proc] --sim must be 'iverilog' or 'verilator' (got '$SIM')"; exit 1 ;;
esac

# --- Tools: binaries + GTKWave always, plus the chosen simulator ------------
[ -x "$YANC_BIN/cmmcomp" ] || { echo "[single_proc] binaries missing in $YANC_BIN - run Scripts/setup.sh first."; exit 1; }
[ -n "$GTKWAVE" ]          || { echo "[single_proc] gtkwave not found - run Scripts/setup.sh (or install gtkwave)."; exit 1; }
if [ "$SIM" = iverilog ]; then
    [ -n "$IVERILOG" ] || { echo "[single_proc] iverilog not found - run Scripts/setup.sh (or install iverilog)."; exit 1; }
else
    [ -n "$VERILATOR" ] || { echo "[single_proc] verilator not found - run Scripts/setup.sh (or install verilator)."; exit 1; }
fi

# --- What to simulate (a project under Compilers/CMMComp/Tests) -------------
PROC=proc_fft             # processor type / project folder to simulate
FNAM=proc_fft.cmm         # cmm filename where the processor is defined
FRE_CLK=100               # processor operating frequency in MHz
NUM_CLK=1000000           # number of clocks to simulate

# --- Repo sources, read in place (never written to) -------------------------
HDL_DIR="$ROOT_DIR/HDL"
MAC_DIR="$ROOT_DIR/Compilers/CMMComp/Includes"
BIN_DIR="$YANC_BIN"
SRC_PROC="$ROOT_DIR/Compilers/CMMComp/Tests/$PROC"

# --- Work area: the only files this script creates (Teste/ is gitignored) ---
WORK="$ROOT_DIR/Teste"
rm -rf "$WORK"
PROC_DIR="$WORK/$PROC"            # writable project dir (tools read the .cmm and emit here)
SOFT_DIR="$PROC_DIR/Software"; HARD_DIR="$PROC_DIR/Hardware"
TMP_DIR="$WORK/Temp"; TMP_PRO="$TMP_DIR/$PROC"   # scratch (-t, sim cwd, trad_*.txt for gen_gtkw)
VL_DIR="$TMP_PRO/vl"             # Verilator obj_dir

# The .cmm is the only input; cmmcomp/asmcomp create Software/Hardware outputs here.
mkdir -p "$TMP_PRO" "$SOFT_DIR"
cp "$SRC_PROC/Software/$FNAM" "$SOFT_DIR/"

# --- Compile: C+- -> asm -> Verilog -----------------------------------------
echo "#### Running the CMM compiler"
"$BIN_DIR/cmmcomp" -i "$FNAM" -n "$PROC" -p "$PROC_DIR" -m "$MAC_DIR" -t "$TMP_PRO"

echo "#### Running the Pre-assembler"
ASM_FILE="$SOFT_DIR/$PROC.asm"
"$BIN_DIR/appcomp" -i "$ASM_FILE" -t "$TMP_PRO"

echo "#### Running the Assembler"
"$BIN_DIR/asmcomp" -i "$ASM_FILE" -p "$PROC_DIR" -d "$HDL_DIR" -m "$MAC_DIR" -t "$TMP_PRO" -f "$FRE_CLK" -c "$NUM_CLK"

# --- Build + run the simulation ---------------------------------------------
# WAVE_VCD: the waveform GTKWave opens; HDR_VCD: the (text) VCD gen_gtkw parses
# for the signal list; GTKW_OUT: the layout gen_gtkw writes when none is given.
UPROC="$HARD_DIR/$PROC"
if [ "$SIM" = iverilog ]; then
    echo "#### Running Icarus"
    # The testbench asmcomp generated (<proc>_tb.v in Temp).
    TB_MOD="${PROC}_tb"
    ( cd "$HDL_DIR" && "$IVERILOG" -s "$TB_MOD" -o "$TMP_PRO/$PROC.vvp" \
        "$TMP_PRO/${PROC}_tb.v" "$UPROC.v" addr_dec.v instr_dec.v processor.v core.v ula.v )

    echo "#### Running VVP"
    cp "${UPROC}_data.mif" "$TMP_PRO/"
    cp "${UPROC}_inst.mif" "$TMP_PRO/"
    cd "$TMP_PRO"
    # header-only pass (text VCD): gives gen_gtkw the signal list fast.
    "$VVP" "$PROC.vvp" +HEADER_ONLY
    cp "$TB_MOD.vcd" "${TB_MOD}_hdr.vcd"
    # real run -> the FST waveform GTKWave opens
    "$VVP" "$PROC.vvp" -fst
    WAVE_VCD="$TMP_PRO/$TB_MOD.vcd"; HDR_VCD="$TMP_PRO/${TB_MOD}_hdr.vcd"; GTKW_OUT="$TMP_PRO/$TB_MOD.gtkw"
else
    # Reuse the generated <proc>_tb.v as the Verilator top. --timing handles the
    # testbench's #-delays / clock; +define+YANC_TRACE pulls in the visibility
    # harness; --trace dumps every signal to <proc>_tb.vcd.
    echo "#### Running Verilator (+define+YANC_TRACE, --binary --timing --trace)"
    "$VERILATOR" --binary --timing --trace +define+YANC_TRACE \
        --top-module "${PROC}_tb" \
        -Wno-lint -Wno-UNOPTFLAT -Wno-MULTIDRIVEN -Wno-BLKANDNBLK -Wno-WIDTH \
        -Wno-CASEINCOMPLETE -Wno-IMPLICIT -Wno-COMBDLY -Wno-STMTDLY -Wno-INFINITELOOP \
        --Mdir "$VL_DIR" \
        "$TMP_PRO/${PROC}_tb.v" "$UPROC.v" \
        "$HDL_DIR/processor.v" "$HDL_DIR/core.v" "$HDL_DIR/ula.v" \
        "$HDL_DIR/addr_dec.v" "$HDL_DIR/instr_dec.v"

    # cd into TMP_PRO: <proc>.v $readmemb("pc_<proc>_mem.txt") and the tb's
    # $dumpfile are relative to the CWD. The tb self-terminates at $finish.
    echo "#### Running the Verilator simulation"
    cd "$TMP_PRO"
    "$VL_DIR/V${PROC}_tb"
    WAVE_VCD="$TMP_PRO/${PROC}_tb.vcd"; HDR_VCD="$WAVE_VCD"; GTKW_OUT="$TMP_PRO/${PROC}_tb.gtkw"
fi

# --- View in GTKWave (layout built by gen_gtkw) -----------------------------
echo "#### Generating the .gtkw layout (gen_gtkw) and launching GTKWave"
"$BIN_DIR/gen_gtkw" "$HDR_VCD" "$GTKW_OUT" "$TMP_DIR" "$BIN_DIR/comp2gtkw"
"$GTKWAVE" --dark --zoom-fit --left-justify "$WAVE_VCD" -a "$GTKW_OUT"

cd "$ROOT_DIR"
