#!/usr/bin/env bash
# ****************************************************************************
# Emulate SAPHO when compiling a multi-processor project, simulated with
# Icarus. Linux counterpart of go_proj.bat. Run Scripts/setup.sh once first.
# ****************************************************************************
set -uo pipefail

SELF_DIR="$(cd "$(dirname "${BASH_SOURCE[0]:-$0}")" && pwd)"
. "$SELF_DIR/Scripts/env.sh"
cd "$ROOT_DIR"

[ -x "$YANC_BIN/cmmcomp" ] || { echo "[go_proj] binaries missing in $YANC_BIN - run Scripts/setup.sh first."; exit 1; }
[ -n "$IVERILOG" ]         || { echo "[go_proj] iverilog not found - run Scripts/setup.sh (or install iverilog)."; exit 1; }
[ -n "$GTKWAVE" ]          || { echo "[go_proj] gtkwave not found - run Scripts/setup.sh (or install gtkwave)."; exit 1; }

TESTE_DIR="$ROOT_DIR/Teste"
rm -rf "$TESTE_DIR"

# --- Parameters -------------------------------------------------------------
PROJET=DTW                        # project folder name
PROC_LIST=(ProcDTW ZeroCross)     # processor types in the project
TB=top_level_tb                   # testbench (without .v), in TopLevel/
GTKW=dtw.gtkw                     # gtkwave layout (if missing, gen_gtkw is used)

INST_DIR="$TESTE_DIR/saphoComponents"
BIN_DIR="$INST_DIR/bin"; HDL_DIR="$INST_DIR/HDL"; MAC_DIR="$INST_DIR/Macros"
SCR_DIR="$INST_DIR/Scripts"; TMP_DIR="$INST_DIR/Temp"
USER_DIR="$TESTE_DIR/Projetos"
PROJ_DIR="$USER_DIR/$PROJET"; TOPL_DIR="$PROJ_DIR/TopLevel"

mkdir -p "$BIN_DIR" "$HDL_DIR" "$MAC_DIR" "$SCR_DIR" "$TMP_DIR" "$USER_DIR"
for i in "${PROC_LIST[@]}"; do mkdir -p "$TMP_DIR/$i"; done

cp -r "$ROOT_DIR/Compilers/CMMComp/Tests/." "$USER_DIR/"
cp -r "$ROOT_DIR/HDL/."                      "$HDL_DIR/"
cp -r "$ROOT_DIR/Compilers/CMMComp/Includes/." "$MAC_DIR/"
cp -r "$ROOT_DIR/Scripts/."                  "$SCR_DIR/"
cp    "$YANC_BIN"/*                           "$BIN_DIR/"

# --- Run the CMM compiler (per processor, with array waveform signals) ------
for i in "${PROC_LIST[@]}"; do
    "$BIN_DIR/cmmcomp" -i "$i.cmm" -n "$i" -p "$USER_DIR/$i" -m "$MAC_DIR" -t "$TMP_DIR/$i" --array
done

# --- Run the Assembler pre-processor ----------------------------------------
for i in "${PROC_LIST[@]}"; do
    "$BIN_DIR/appcomp" -i "$USER_DIR/$i/Software/$i.asm" -t "$TMP_DIR/$i"
done

# --- Run the Assembler compiler ---------------------------------------------
for i in "${PROC_LIST[@]}"; do
    "$BIN_DIR/asmcomp" -i "$USER_DIR/$i/Software/$i.asm" -p "$USER_DIR/$i" -d "$HDL_DIR" -m "$MAC_DIR" -t "$TMP_DIR/$i" -f 0 -c 0
    cp "$USER_DIR/$i/Hardware/$i.v" "$TMP_DIR/$i/"
done

# --- Build the testbench with Icarus ----------------------------------------
cd "$TMP_DIR"
HDL_V=("$HDL_DIR"/*.v)
TOP_V=("$TOPL_DIR"/*.v)
PRO_V=(); for i in "${PROC_LIST[@]}"; do PRO_V+=("$TMP_DIR/$i/$i.v"); done

"$IVERILOG" -s "$TB" -o "$TMP_DIR/$PROJET.vvp" "${HDL_V[@]}" "${PRO_V[@]}" "${TOP_V[@]}"

for i in "${PROC_LIST[@]}"; do cp "$TMP_DIR/$i/${i}_tb.v" "$USER_DIR/$i/Simulation/"; done

# --- Run the testbench with vvp ---------------------------------------------
# Stage the files the run reads with relative paths into the CWD (= TMP_DIR).
for f in "$TOPL_DIR"/*.txt; do [ -e "$f" ] && cp "$f" .; done
for i in "${PROC_LIST[@]}"; do
    cp "$USER_DIR/$i/Hardware/${i}_inst.mif" .
    cp "$USER_DIR/$i/Hardware/${i}_data.mif" .
    cp "$TMP_DIR/$i/pc_${i}_mem.txt" .
done

# header-only pass (tiny text VCD) then the real FST run (+WAVE arms $dumpvars)
"$VVP" "$PROJET.vvp" +HEADER_ONLY
cp "$TB.vcd" "${TB}_hdr.vcd"
"$VVP" "$PROJET.vvp" -fst +WAVE

# --- Run GTKWave ------------------------------------------------------------
if [ -f "$TOPL_DIR/$GTKW" ]; then
    "$GTKWAVE" --dark --zoom-fit --left-justify "$TMP_DIR/$TB.vcd" -a "$TOPL_DIR/$GTKW"
else
    "$BIN_DIR/gen_gtkw" "$TMP_DIR/${TB}_hdr.vcd" "$TMP_DIR/$TB.gtkw" "$TMP_DIR" "$BIN_DIR/comp2gtkw"
    "$GTKWAVE" --dark --zoom-fit --left-justify "$TMP_DIR/$TB.vcd" -a "$TMP_DIR/$TB.gtkw"
fi

cd "$ROOT_DIR"
