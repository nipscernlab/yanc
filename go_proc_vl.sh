#!/usr/bin/env bash
# ****************************************************************************
# Like go_proc.sh, but simulates with Verilator (--binary --timing --trace,
# +define+YANC_TRACE), so the waveform shows all the user variables/arrays.
# Linux counterpart of go_proc_vl.bat. Run Scripts/setup.sh once first.
# ****************************************************************************
set -uo pipefail

SELF_DIR="$(cd "$(dirname "${BASH_SOURCE[0]:-$0}")" && pwd)"
. "$SELF_DIR/Scripts/env.sh"
cd "$ROOT_DIR"

[ -x "$YANC_BIN/cmmcomp" ] || { echo "[go_proc_vl] binaries missing in $YANC_BIN - run Scripts/setup.sh first."; exit 1; }
[ -n "$VERILATOR" ]        || { echo "[go_proc_vl] verilator not found - run Scripts/setup.sh (or install verilator)."; exit 1; }
[ -n "$GTKWAVE" ]          || { echo "[go_proc_vl] gtkwave not found - run Scripts/setup.sh (or install gtkwave)."; exit 1; }

TESTE_DIR="$ROOT_DIR/Teste"
rm -rf "$TESTE_DIR"

# --- Parameters -------------------------------------------------------------
PROJET=FFT
PROC=proc_fft
FNAM=proc_fft.cmm
GTKW=teste.gtkw
FRE_CLK=100
NUM_CLK=1000000

INST_DIR="$TESTE_DIR/saphoComponents"
BIN_DIR="$INST_DIR/bin"; HDL_DIR="$INST_DIR/HDL"; MAC_DIR="$INST_DIR/Macros"
SCR_DIR="$INST_DIR/Scripts"; TMP_DIR="$INST_DIR/Temp"
USER_DIR="$TESTE_DIR/Projetos"
PROC_DIR="$USER_DIR/$PROC"; SOFT_DIR="$PROC_DIR/Software"
HARD_DIR="$PROC_DIR/Hardware"; SIMU_DIR="$PROC_DIR/Simulation"
TMP_PRO="$TMP_DIR/$PROC"
VL_DIR="$TMP_PRO/vl"      # Verilator obj_dir (C++ model + V<proc>_tb sim exe)

mkdir -p "$BIN_DIR" "$HDL_DIR" "$MAC_DIR" "$SCR_DIR" "$TMP_PRO" "$USER_DIR"

cp -r "$ROOT_DIR/Compilers/CMMComp/Tests/." "$USER_DIR/"
cp -r "$ROOT_DIR/HDL/."                      "$HDL_DIR/"
cp -r "$ROOT_DIR/Compilers/CMMComp/Includes/." "$MAC_DIR/"
cp -r "$ROOT_DIR/Scripts/."                  "$SCR_DIR/"
cp    "$YANC_BIN"/*                           "$BIN_DIR/"

echo "#### Running the CMM compiler"
"$BIN_DIR/cmmcomp" -i "$FNAM" -n "$PROC" -p "$PROC_DIR" -m "$MAC_DIR" -t "$TMP_PRO"

echo "#### Running the Pre-assembler"
ASM_FILE="$SOFT_DIR/$PROC.asm"
"$BIN_DIR/appcomp" -i "$ASM_FILE" -t "$TMP_PRO"

echo "#### Running the Assembler"
"$BIN_DIR/asmcomp" -i "$ASM_FILE" -p "$PROC_DIR" -d "$HDL_DIR" -m "$MAC_DIR" -t "$TMP_PRO" -f "$FRE_CLK" -c "$NUM_CLK"

# --- Build the simulation with Verilator ------------------------------------
# Reuse the generated <proc>_tb.v as the Verilator top. --timing handles the
# testbench's #-delays / clock; +define+YANC_TRACE pulls in the visibility
# harness; --trace dumps every signal to <proc>_tb.vcd.
echo "#### Running Verilator (+define+YANC_TRACE, --binary --timing --trace)"
UPROC="$HARD_DIR/$PROC"
TB="$TMP_PRO/${PROC}_tb.v"
"$VERILATOR" --binary --timing --trace +define+YANC_TRACE \
    --top-module "${PROC}_tb" \
    -Wno-lint -Wno-UNOPTFLAT -Wno-MULTIDRIVEN -Wno-BLKANDNBLK -Wno-WIDTH \
    -Wno-CASEINCOMPLETE -Wno-IMPLICIT -Wno-COMBDLY -Wno-STMTDLY -Wno-INFINITELOOP \
    --Mdir "$VL_DIR" \
    "$TB" "$UPROC.v" \
    "$HDL_DIR/processor.v" "$HDL_DIR/core.v" "$HDL_DIR/ula.v" \
    "$HDL_DIR/addr_dec.v" "$HDL_DIR/instr_dec.v"

# --- Run the simulation -----------------------------------------------------
# cd into TMP_PRO: <proc>.v $readmemb("pc_<proc>_mem.txt") and the tb's
# $dumpfile are relative to the CWD. The tb self-terminates at $finish.
echo "#### Running the Verilator simulation"
cd "$TMP_PRO"
"$VL_DIR/V${PROC}_tb"

# --- Run GTKWave ------------------------------------------------------------
echo "#### Generating the .gtkw layout and launching GTKWave"
if [ -f "$SIMU_DIR/$GTKW" ]; then
    "$GTKWAVE" --dark --zoom-fit --left-justify "$TMP_PRO/${PROC}_tb.vcd" -a "$SIMU_DIR/$GTKW"
else
    "$BIN_DIR/gen_gtkw" "$TMP_PRO/${PROC}_tb.vcd" "$TMP_PRO/${PROC}_tb.gtkw" "$TMP_DIR" "$BIN_DIR/comp2gtkw"
    "$GTKWAVE" --dark --zoom-fit --left-justify "$TMP_PRO/${PROC}_tb.vcd" -a "$TMP_PRO/${PROC}_tb.gtkw"
fi

cd "$ROOT_DIR"
