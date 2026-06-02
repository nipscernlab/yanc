#!/usr/bin/env bash
# ****************************************************************************
# Emulate SAPHO when compiling a single processor and simulating it.
#   ./go_proc.sh                 # simulate with Icarus (default)
#   ./go_proc.sh --sim verilator # simulate with Verilator (+define+YANC_TRACE)
# Linux counterpart of go_proc.bat. Run Scripts/setup.sh once first.
# ****************************************************************************
set -uo pipefail

SELF_DIR="$(cd "$(dirname "${BASH_SOURCE[0]:-$0}")" && pwd)"
. "$SELF_DIR/Scripts/env.sh"
cd "$ROOT_DIR"

# --- Pick the simulator (--sim iverilog|verilator, default iverilog) ---------
SIM=iverilog
while [ $# -gt 0 ]; do
    case "$1" in
        --sim)     SIM="${2:-}"; shift 2 ;;
        --sim=*)   SIM="${1#--sim=}"; shift ;;
        -h|--help) echo "usage: ${0##*/} [--sim iverilog|verilator]"; exit 0 ;;
        *) echo "[go_proc] unknown argument: $1 (try --sim iverilog|verilator)"; exit 1 ;;
    esac
done
case "$SIM" in
    iverilog|icarus) SIM=iverilog ;;
    verilator|vl)    SIM=verilator ;;
    *) echo "[go_proc] --sim must be 'iverilog' or 'verilator' (got '$SIM')"; exit 1 ;;
esac

# --- Tools: binaries + GTKWave always, plus the chosen simulator ------------
[ -x "$YANC_BIN/cmmcomp" ] || { echo "[go_proc] binaries missing in $YANC_BIN - run Scripts/setup.sh first."; exit 1; }
[ -n "$GTKWAVE" ]          || { echo "[go_proc] gtkwave not found - run Scripts/setup.sh (or install gtkwave)."; exit 1; }
if [ "$SIM" = iverilog ]; then
    [ -n "$IVERILOG" ] || { echo "[go_proc] iverilog not found - run Scripts/setup.sh (or install iverilog)."; exit 1; }
else
    [ -n "$VERILATOR" ] || { echo "[go_proc] verilator not found - run Scripts/setup.sh (or install verilator)."; exit 1; }
fi

TESTE_DIR="$ROOT_DIR/Teste"
rm -rf "$TESTE_DIR"

# --- Parameters defined by the SAPHO user for compilation -------------------
PROJET=FFT                # project folder name
PROC=proc_fft             # processor type to simulate (a project subfolder)
FNAM=proc_fft.cmm         # cmm filename where the processor is defined
TB=errado                 # testbench (without .v); if missing, default sim is used (Icarus only)
GTKW=teste.gtkw           # gtkwave layout filename (if missing, gen_gtkw is used)
FRE_CLK=100               # processor operating frequency in MHz
NUM_CLK=1000000           # number of clocks to simulate

# --- Folder tree (mirrors the installed layout) -----------------------------
INST_DIR="$TESTE_DIR/saphoComponents"
BIN_DIR="$INST_DIR/bin"; HDL_DIR="$INST_DIR/HDL"; MAC_DIR="$INST_DIR/Macros"
SCR_DIR="$INST_DIR/Scripts"; TMP_DIR="$INST_DIR/Temp"
USER_DIR="$TESTE_DIR/Projetos"
PROC_DIR="$USER_DIR/$PROC"; SOFT_DIR="$PROC_DIR/Software"
HARD_DIR="$PROC_DIR/Hardware"; SIMU_DIR="$PROC_DIR/Simulation"
TMP_PRO="$TMP_DIR/$PROC"
VL_DIR="$TMP_PRO/vl"      # Verilator obj_dir (C++ model + V<proc>_tb sim exe)

mkdir -p "$BIN_DIR" "$HDL_DIR" "$MAC_DIR" "$SCR_DIR" "$TMP_PRO" "$USER_DIR"

# --- Stage example projects, HDL, macros and the prebuilt binaries ----------
cp -r "$ROOT_DIR/Compilers/CMMComp/Tests/." "$USER_DIR/"
cp -r "$ROOT_DIR/HDL/."                      "$HDL_DIR/"
cp -r "$ROOT_DIR/Compilers/CMMComp/Includes/." "$MAC_DIR/"
cp -r "$ROOT_DIR/Scripts/."                  "$SCR_DIR/"
cp    "$YANC_BIN"/*                           "$BIN_DIR/"

# --- Run the CMM compiler ---------------------------------------------------
echo "#### Running the CMM compiler"
"$BIN_DIR/cmmcomp" -i "$FNAM" -n "$PROC" -p "$PROC_DIR" -m "$MAC_DIR" -t "$TMP_PRO"

# --- Run the Assembler pre-processor ----------------------------------------
echo "#### Running the Pre-assembler"
ASM_FILE="$SOFT_DIR/$PROC.asm"
"$BIN_DIR/appcomp" -i "$ASM_FILE" -t "$TMP_PRO"

# --- Run the Assembler compiler ---------------------------------------------
echo "#### Running the Assembler"
"$BIN_DIR/asmcomp" -i "$ASM_FILE" -p "$PROC_DIR" -d "$HDL_DIR" -m "$MAC_DIR" -t "$TMP_PRO" -f "$FRE_CLK" -c "$NUM_CLK"

# --- Build + run the simulation ---------------------------------------------
# WAVE_VCD: the waveform GTKWave opens; HDR_VCD: the (text) VCD gen_gtkw parses
# for the signal list; GTKW_OUT: the layout gen_gtkw writes when none is given.
UPROC="$HARD_DIR/$PROC"
if [ "$SIM" = iverilog ]; then
    echo "#### Running Icarus"
    if [ -f "$SIMU_DIR/$TB.v" ]; then
        TB_MOD="$TB"
    else
        cp "$TMP_PRO/${PROC}_tb.v" "$SIMU_DIR/"
        TB_MOD="${PROC}_tb"
    fi
    ( cd "$HDL_DIR" && "$IVERILOG" -s "$TB_MOD" -o "$TMP_PRO/$PROC.vvp" \
        "$SIMU_DIR/$TB_MOD.v" "$UPROC.v" addr_dec.v instr_dec.v processor.v core.v ula.v )

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

# --- Run GTKWave ------------------------------------------------------------
echo "#### Generating the .gtkw layout and launching GTKWave"
if [ -f "$SIMU_DIR/$GTKW" ]; then
    "$GTKWAVE" --dark --zoom-fit --left-justify "$WAVE_VCD" -a "$SIMU_DIR/$GTKW"
else
    "$BIN_DIR/gen_gtkw" "$HDR_VCD" "$GTKW_OUT" "$TMP_DIR" "$BIN_DIR/comp2gtkw"
    "$GTKWAVE" --dark --zoom-fit --left-justify "$WAVE_VCD" -a "$GTKW_OUT"
fi

cd "$ROOT_DIR"
