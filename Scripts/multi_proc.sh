#!/usr/bin/env bash
# ****************************************************************************
# multi_proc - end-to-end example of the YANC multi-processor pipeline.
#
# It shows the pipeline of a top-level circuit that uses two SAPHO processors
# internally, taking each processor's C+- (.cmm) program through the compile
# pipeline to its synthesizable Verilog (.v):
#
#   * ZeroCross - FIR filters that smooth the 60 Hz mains (power-grid)
#                 waveform and compute its zero crossings.
#   * ProcDTW   - a DTW transform (2-D array) synchronized to the zero
#                 crossings produced by ZeroCross, doing novelty detection to
#                 spot disturbances on the power grid.
#
# The circuit is several Verilog files -- the top level plus one generated
# <proc>.v per processor -- and this project has been tested on an FPGA. The
# GTKWave view shows the top-level signals alongside the software execution of
# both processors in lockstep.
#
# Because the processors run together under a top level, the testbench here is
# the user-written TopLevel/top_level_tb.v. The asm-generated testbench can
# only drive one processor in isolation -- that is the single_proc flow.
#
#   ./multi_proc.sh                 # simulate with Icarus (default)
#   ./multi_proc.sh --sim verilator # simulate with Verilator (+define+YANC_TRACE)
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
        *) echo "[multi_proc] unknown argument: $1 (try --sim iverilog|verilator)"; exit 1 ;;
    esac
done
case "$SIM" in
    iverilog|icarus) SIM=iverilog ;;
    verilator|vl)    SIM=verilator ;;
    *) echo "[multi_proc] --sim must be 'iverilog' or 'verilator' (got '$SIM')"; exit 1 ;;
esac

# --- Tools: binaries + GTKWave always, plus the chosen simulator ------------
[ -x "$YANC_BIN/cmmcomp" ] || { echo "[multi_proc] binaries missing in $YANC_BIN - run Scripts/setup.sh first."; exit 1; }
[ -n "$GTKWAVE" ]          || { echo "[multi_proc] gtkwave not found - run Scripts/setup.sh (or install gtkwave)."; exit 1; }
if [ "$SIM" = iverilog ]; then
    [ -n "$IVERILOG" ] || { echo "[multi_proc] iverilog not found - run Scripts/setup.sh (or install iverilog)."; exit 1; }
else
    [ -n "$VERILATOR" ] || { echo "[multi_proc] verilator not found - run Scripts/setup.sh (or install verilator)."; exit 1; }
    [ -n "$FST2VCD" ]   || { echo "[multi_proc] fst2vcd not found - it ships with gtkwave; run Scripts/setup.sh."; exit 1; }
fi

# --- What to simulate (a project under Compilers/CMMComp/Tests) -------------
PROJET=DTW                        # project folder (holds TopLevel/ with the tb)
PROC_LIST=(ProcDTW ZeroCross)     # processor types in the project
TB=top_level_tb                   # top testbench (without .v), in TopLevel/

# Verilator warning suppressions (the design is not lint-clean and mixes
# timescale'd top modules with non-timescale'd HDL; none affect the sim).
VL_WARN=(-Wno-lint -Wno-MULTIDRIVEN -Wno-BLKANDNBLK -Wno-WIDTH -Wno-CASEINCOMPLETE
         -Wno-IMPLICIT -Wno-COMBDLY -Wno-STMTDLY -Wno-INFINITELOOP -Wno-UNOPTFLAT
         -Wno-PINMISSING -Wno-SELRANGE -Wno-TIMESCALEMOD -Wno-INITIALDLY)

# --- Repo sources, read in place (never written to) -------------------------
HDL_DIR="$ROOT_DIR/HDL"
MAC_DIR="$ROOT_DIR/Compilers/CMMComp/Includes"
BIN_DIR="$YANC_BIN"
TESTS_DIR="$ROOT_DIR/Compilers/CMMComp/Tests"

# --- Work area: the only files this script creates (Teste/ is gitignored) ---
WORK="$ROOT_DIR/Teste"
rm -rf "$WORK"
USER_DIR="$WORK/Projetos"         # writable copies of the projects we compile
TMP_DIR="$WORK/Temp"
PROJ_DIR="$USER_DIR/$PROJET"; TOPL_DIR="$PROJ_DIR/TopLevel"
VL_DIR="$TMP_DIR/vl"

# Only the inputs are copied: the project's TopLevel/ (user testbench, top-level
# Verilog and stimulus) and each processor's .cmm. The compilers create the
# Software/Hardware outputs in the writable copies.
mkdir -p "$TMP_DIR" "$TOPL_DIR"
cp -r "$TESTS_DIR/$PROJET/TopLevel/." "$TOPL_DIR/"
for i in "${PROC_LIST[@]}"; do
    mkdir -p "$USER_DIR/$i/Software" "$TMP_DIR/$i"
    cp "$TESTS_DIR/$i/Software/$i.cmm" "$USER_DIR/$i/Software/"
done

# --- Compile each processor: C+- -> asm -> Verilog --------------------------
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

# --- Build + run the simulation ---------------------------------------------
cd "$TMP_DIR"
HDL_V=("$HDL_DIR"/*.v)
TOP_V=("$TOPL_DIR"/*.v)
PRO_V=(); for i in "${PROC_LIST[@]}"; do PRO_V+=("$TMP_DIR/$i/$i.v"); done

if [ "$SIM" = iverilog ]; then
    echo "#### Running Icarus"
    "$IVERILOG" -s "$TB" -o "$TMP_DIR/$PROJET.vvp" "${HDL_V[@]}" "${PRO_V[@]}" "${TOP_V[@]}"

    # Stage the files the run reads with relative paths into the CWD (= TMP_DIR).
    echo "#### Running VVP"
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
else
    echo "#### Running Verilator (+define+YANC_TRACE, --binary --timing --trace-fst)"
    "$VERILATOR" --binary --timing --trace-fst +define+YANC_TRACE --top-module "$TB" \
        "${VL_WARN[@]}" --Mdir "$VL_DIR" "${HDL_V[@]}" "${PRO_V[@]}" "${TOP_V[@]}"

    # Stage the files the run reads with relative paths into the CWD (= TMP_DIR).
    # (the _data/_inst .mif are referenced by absolute path, so they resolve.)
    for f in "$TOPL_DIR"/*.txt; do [ -e "$f" ] && cp "$f" .; done
    for i in "${PROC_LIST[@]}"; do cp "$TMP_DIR/$i/pc_${i}_mem.txt" .; done

    echo "#### Running the Verilator simulation"
    # header-only pass -> tiny FST; fst2vcd extracts a text VCD for the formatter.
    "$VL_DIR/V$TB" +HEADER_ONLY
    "$FST2VCD" -f "$TB.vcd" -o "${TB}_hdr.vcd" >/dev/null 2>&1
    # +WAVE arms the tb's $dumpvars (off by default so the heavy sim doesn't crash).
    "$VL_DIR/V$TB" +WAVE
fi

# --- View in GTKWave (layout built by gen_gtkw) -----------------------------
echo "#### Generating the .gtkw layout (gen_gtkw) and launching GTKWave"
"$BIN_DIR/gen_gtkw" "$TMP_DIR/${TB}_hdr.vcd" "$TMP_DIR/$TB.gtkw" "$TMP_DIR" "$BIN_DIR/comp2gtkw"
"$GTKWAVE" --dark --zoom-fit --left-justify "$TMP_DIR/$TB.vcd" -a "$TMP_DIR/$TB.gtkw"

cd "$ROOT_DIR"
