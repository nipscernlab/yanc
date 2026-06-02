#!/usr/bin/env bash
# ****************************************************************************
# Like go_proj.sh, but simulates the multi-proc project with Verilator
# (--binary --timing --trace-fst, +define+YANC_TRACE). Linux counterpart of
# go_proj_vl.bat. Run Scripts/setup.sh once first.
# ****************************************************************************
set -uo pipefail

SELF_DIR="$(cd "$(dirname "${BASH_SOURCE[0]:-$0}")" && pwd)"
. "$SELF_DIR/Scripts/env.sh"
cd "$ROOT_DIR"

[ -x "$YANC_BIN/cmmcomp" ] || { echo "[go_proj_vl] binaries missing in $YANC_BIN - run Scripts/setup.sh first."; exit 1; }
[ -n "$VERILATOR" ]        || { echo "[go_proj_vl] verilator not found - run Scripts/setup.sh (or install verilator)."; exit 1; }
[ -n "$GTKWAVE" ]          || { echo "[go_proj_vl] gtkwave not found - run Scripts/setup.sh (or install gtkwave)."; exit 1; }
[ -n "$FST2VCD" ]          || { echo "[go_proj_vl] fst2vcd not found - it ships with gtkwave; run Scripts/setup.sh."; exit 1; }

TESTE_DIR="$ROOT_DIR/Teste"
rm -rf "$TESTE_DIR"

# --- Parameters -------------------------------------------------------------
PROJET=DTW
PROC_LIST=(ProcDTW ZeroCross)
TB=top_level_tb
GTKW=dtw.gtkw

# Verilator warning suppressions (the design is not lint-clean and mixes
# timescale'd top modules with non-timescale'd HDL; none affect the sim).
VL_WARN=(-Wno-lint -Wno-MULTIDRIVEN -Wno-BLKANDNBLK -Wno-WIDTH -Wno-CASEINCOMPLETE
         -Wno-IMPLICIT -Wno-COMBDLY -Wno-STMTDLY -Wno-INFINITELOOP -Wno-UNOPTFLAT
         -Wno-PINMISSING -Wno-SELRANGE -Wno-TIMESCALEMOD -Wno-INITIALDLY)

INST_DIR="$TESTE_DIR/saphoComponents"
BIN_DIR="$INST_DIR/bin"; HDL_DIR="$INST_DIR/HDL"; MAC_DIR="$INST_DIR/Macros"
SCR_DIR="$INST_DIR/Scripts"; TMP_DIR="$INST_DIR/Temp"
USER_DIR="$TESTE_DIR/Projetos"
PROJ_DIR="$USER_DIR/$PROJET"; TOPL_DIR="$PROJ_DIR/TopLevel"
VL_DIR="$TMP_DIR/vl"

mkdir -p "$BIN_DIR" "$HDL_DIR" "$MAC_DIR" "$SCR_DIR" "$TMP_DIR" "$USER_DIR"
for i in "${PROC_LIST[@]}"; do mkdir -p "$TMP_DIR/$i"; done

cp -r "$ROOT_DIR/Compilers/CMMComp/Tests/." "$USER_DIR/"
cp -r "$ROOT_DIR/HDL/."                      "$HDL_DIR/"
cp -r "$ROOT_DIR/Compilers/CMMComp/Includes/." "$MAC_DIR/"
cp -r "$ROOT_DIR/Scripts/."                  "$SCR_DIR/"
cp    "$YANC_BIN"/*                           "$BIN_DIR/"

for i in "${PROC_LIST[@]}"; do
    "$BIN_DIR/cmmcomp" -i "$i.cmm" -n "$i" -p "$USER_DIR/$i" -m "$MAC_DIR" -t "$TMP_DIR/$i" --array
done
for i in "${PROC_LIST[@]}"; do
    "$BIN_DIR/appcomp" -i "$USER_DIR/$i/Software/$i.asm" -t "$TMP_DIR/$i"
done
for i in "${PROC_LIST[@]}"; do
    "$BIN_DIR/asmcomp" -i "$USER_DIR/$i/Software/$i.asm" -p "$USER_DIR/$i" -d "$HDL_DIR" -m "$MAC_DIR" -t "$TMP_DIR/$i" -f 0 -c 0
    cp "$USER_DIR/$i/Hardware/$i.v" "$TMP_DIR/$i/"
done

# --- Build the simulation with Verilator ------------------------------------
cd "$TMP_DIR"
HDL_V=("$HDL_DIR"/*.v)
TOP_V=("$TOPL_DIR"/*.v)
PRO_V=(); for i in "${PROC_LIST[@]}"; do PRO_V+=("$TMP_DIR/$i/$i.v"); done

echo "#### Running Verilator (+define+YANC_TRACE, --binary --timing --trace-fst)"
"$VERILATOR" --binary --timing --trace-fst +define+YANC_TRACE --top-module "$TB" \
    "${VL_WARN[@]}" --Mdir "$VL_DIR" "${HDL_V[@]}" "${PRO_V[@]}" "${TOP_V[@]}"

# Stage the files the run reads with relative paths into the CWD (= TMP_DIR).
# (the _data/_inst .mif are referenced by absolute path, so they resolve.)
for f in "$TOPL_DIR"/*.txt; do [ -e "$f" ] && cp "$f" .; done
for i in "${PROC_LIST[@]}"; do cp "$TMP_DIR/$i/pc_${i}_mem.txt" .; done

# --- Run the simulation -----------------------------------------------------
echo "#### Running the Verilator simulation"
# header-only pass -> tiny FST; fst2vcd extracts a text VCD for the formatter.
"$VL_DIR/V$TB" +HEADER_ONLY
"$FST2VCD" -f "$TB.vcd" -o "${TB}_hdr.vcd" >/dev/null 2>&1
# +WAVE arms the tb's $dumpvars (off by default so the heavy sim doesn't crash).
"$VL_DIR/V$TB" +WAVE

# --- Run GTKWave ------------------------------------------------------------
echo "#### Generating the .gtkw layout and launching GTKWave"
if [ -f "$TOPL_DIR/$GTKW" ]; then
    "$GTKWAVE" --dark --zoom-fit --left-justify "$TMP_DIR/$TB.vcd" -a "$TOPL_DIR/$GTKW"
else
    "$BIN_DIR/gen_gtkw" "$TMP_DIR/${TB}_hdr.vcd" "$TMP_DIR/$TB.gtkw" "$TMP_DIR" "$BIN_DIR/comp2gtkw"
    "$GTKWAVE" --dark --zoom-fit --left-justify "$TMP_DIR/$TB.vcd" -a "$TMP_DIR/$TB.gtkw"
fi

cd "$ROOT_DIR"
