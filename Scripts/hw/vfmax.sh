#!/bin/bash
# Real Fmax of a generated processor through Vivado (AMD/Xilinx), the
# counterpart of fmax.sh (Quartus). Out of context: no pins, the timing of the
# processor itself against a 10 ns clock, after placement and routing.
#
# Usage:  bash Scripts/hw/vfmax.sh <proc> [part]
#         bash Scripts/hw/vfmax.sh sapho_all                 # ZYBO: xc7z010clg400-1
#
# <proc> is a processor the regress already built (.smoke/work/<proc>/Hardware/
# <proc>.v, whose .mif paths are absolute). Results in .smoke/hw/<proc>_<part>/
# and appended to .smoke/hw/vfmax.txt. One run takes minutes to an hour; run one
# at a time.
set -e
PROC=$1; PART=${2:-xc7z010clg400-1}
[ -n "$PROC" ] || { echo "usage: vfmax.sh <proc> [part]"; exit 1; }

ROOT=$(cd "$(dirname "$0")/../.." && pwd)
VIVADO=${VIVADO:-/c/AMDDesignTools/2025.2/Vivado/bin/vivado.bat}
TOP="$ROOT/.smoke/work/$PROC/Hardware/$PROC.v"
OUT="$ROOT/.smoke/hw"
DIR="$OUT/${PROC}_${PART}"

[ -f "$TOP" ] || { echo "no $TOP -- run Scripts/regress.sh first"; exit 1; }
[ -f "$VIVADO" ] || { echo "Vivado not at $VIVADO (set VIVADO=...)"; exit 1; }

rm -rf "$DIR"; mkdir -p "$DIR/hdl"; cd "$DIR"
W() { cygpath -m "$1" 2>/dev/null || echo "$1"; }
# A copy of SAPHO/ with both memories asked into block RAM, as Quartus puts
# them: left alone, Vivado builds the program ROM (its contents are known) out
# of LUTs, which lengthens every path that ends at the instruction fetch.
cp "$ROOT"/SAPHO/*.v hdl/
sed -i 's/^reg \[NBDATA-1:0\] mem \[0:NADDRE-1\];/(* ram_style = "block", rom_style = "block" *) reg [NBDATA-1:0] mem [0:NADDRE-1];/' hdl/processor.v
[ "$(grep -c 'rom_style = "block"' hdl/processor.v)" = 2 ] || { echo "vfmax: could not mark the memories"; exit 1; }
# Vivado 2025.2 sometimes stops in opt_design with an empty "[Synth 20-411]"
# (seen 3 times in 2026-10; the same run went through when repeated): up to 3
# attempts on that error only.
for try in 1 2 3; do
    cmd //c "$(W "$VIVADO")" -mode batch -nojournal -log vivado.log \
        -source "$(W "$ROOT/Scripts/hw/vfmax.tcl")" \
        -tclargs "$PROC" "$(W "$TOP")" "$(W "$DIR/hdl")" "$PART" "$(W "$DIR")" > run.log 2>&1 && break
    grep -q 'Synth 20-411' vivado.log && [ $try -lt 3 ] && { echo "$PROC: Synth 20-411, again ($try)"; continue; }
    echo "$PROC: VIVADO FAILED"; grep -m5 'ERROR' vivado.log; exit 1
done
cat result.txt | tee -a "$OUT/vfmax.txt"
