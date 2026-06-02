#!/usr/bin/env bash
# ****************************************************************************
# Emulate SAPHO when compiling a single processor, simulated with Icarus.
# Linux counterpart of go_proc.bat. Run Scripts/setup.sh once first.
# ****************************************************************************
set -uo pipefail

# Resolve repo root + binaries (YANC_BIN) and tools (IVERILOG/VVP/GTKWAVE).
SELF_DIR="$(cd "$(dirname "${BASH_SOURCE[0]:-$0}")" && pwd)"
. "$SELF_DIR/Scripts/env.sh"
cd "$ROOT_DIR"

# This Icarus flow needs the binaries plus iverilog/vvp and GTKWave ----------
[ -x "$YANC_BIN/cmmcomp" ] || { echo "[go_proc] binaries missing in $YANC_BIN - run Scripts/setup.sh first."; exit 1; }
[ -n "$IVERILOG" ]         || { echo "[go_proc] iverilog not found - run Scripts/setup.sh (or install iverilog)."; exit 1; }
[ -n "$GTKWAVE" ]          || { echo "[go_proc] gtkwave not found - run Scripts/setup.sh (or install gtkwave)."; exit 1; }

TESTE_DIR="$ROOT_DIR/Teste"
rm -rf "$TESTE_DIR"

# --- Parameters defined by the SAPHO user for compilation -------------------
PROJET=FFT                # project folder name
PROC=proc_fft             # processor type to simulate (a project subfolder)
FNAM=proc_fft.cmm         # cmm filename where the processor is defined
TB=errado                 # testbench (without .v); if missing, default sim is used
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

# --- Build the testbench with Icarus ----------------------------------------
echo "#### Running Icarus"
UPROC="$HARD_DIR/$PROC"
if [ -f "$SIMU_DIR/$TB.v" ]; then
    TB_MOD="$TB"
else
    cp "$TMP_PRO/${PROC}_tb.v" "$SIMU_DIR/"
    TB_MOD="${PROC}_tb"
fi
( cd "$HDL_DIR" && "$IVERILOG" -s "$TB_MOD" -o "$TMP_PRO/$PROC.vvp" \
    "$SIMU_DIR/$TB_MOD.v" "$UPROC.v" addr_dec.v instr_dec.v processor.v core.v ula.v )

# --- Run the testbench with vvp ---------------------------------------------
echo "#### Running VVP"
cp "${UPROC}_data.mif" "$TMP_PRO/"
cp "${UPROC}_inst.mif" "$TMP_PRO/"
cd "$TMP_PRO"

# header-only pass (text VCD): gives gen_gtkw the signal list fast.
"$VVP" "$PROC.vvp" +HEADER_ONLY
cp "$TB_MOD.vcd" "${TB_MOD}_hdr.vcd"
# real run -> the FST waveform GTKWave opens
"$VVP" "$PROC.vvp" -fst

# --- Run GTKWave ------------------------------------------------------------
echo "#### Generating the .gtkw layout and launching GTKWave"
if [ -f "$SIMU_DIR/$GTKW" ]; then
    "$GTKWAVE" --dark --zoom-fit --left-justify "$TMP_PRO/$TB_MOD.vcd" -a "$SIMU_DIR/$GTKW"
else
    "$BIN_DIR/gen_gtkw" "$TMP_PRO/${TB_MOD}_hdr.vcd" "$TMP_PRO/$TB_MOD.gtkw" "$TMP_DIR" "$BIN_DIR/comp2gtkw"
    "$GTKWAVE" --dark --zoom-fit --left-justify "$TMP_PRO/$TB_MOD.vcd" -a "$TMP_PRO/$TB_MOD.gtkw"
fi

cd "$ROOT_DIR"
