#!/bin/bash
# Real Fmax of the LINKALL pair (two SAPHOs on two clocks over a synchronized
# toma/cade link) through Vivado, out of context: the counterpart of
# fmax_link.sh. The top is Scripts/hw/linkall_top.v.
#
# Usage:  bash Scripts/hw/vfmax_link.sh [part]       # ZYBO: xc7z010clg400-1
#
# Reads the regress-built .smoke/work/sapho_all_{a,b}/Hardware/*.v. Results in
# .smoke/hw/linkall_<part>/ and appended to .smoke/hw/vfmax.txt (one Fmax per
# clock). Minutes to an hour; run one at a time.
set -e
PART=${1:-xc7z010clg400-1}
ROOT=$(cd "$(dirname "$0")/../.." && pwd)
VIVADO=${VIVADO:-/c/AMDDesignTools/2025.2/Vivado/bin/vivado.bat}
OUT="$ROOT/.smoke/hw"
DIR="$OUT/linkall_${PART}"

for p in sapho_all_a sapho_all_b; do
    [ -f "$ROOT/.smoke/work/$p/Hardware/$p.v" ] || { echo "no .smoke/work/$p/Hardware/$p.v -- run Scripts/regress.sh first"; exit 1; }
done
[ -f "$VIVADO" ] || { echo "Vivado not at $VIVADO (set VIVADO=...)"; exit 1; }

rm -rf "$DIR"; mkdir -p "$DIR/hdl"; cd "$DIR"
W() { cygpath -m "$1" 2>/dev/null || echo "$1"; }
cp "$ROOT/Scripts/hw/linkall_top.v" "$ROOT/.smoke/work/sapho_all_a/Hardware/sapho_all_a.v" "$ROOT/.smoke/work/sapho_all_b/Hardware/sapho_all_b.v" .
# both memories into block RAM, as vfmax.sh does (and as Quartus does)
cp "$ROOT"/SAPHO/*.v hdl/
sed -i 's/^reg \[NBDATA-1:0\] mem \[0:NADDRE-1\];/(* ram_style = "block", rom_style = "block" *) reg [NBDATA-1:0] mem [0:NADDRE-1];/' hdl/processor.v
[ "$(grep -c 'rom_style = "block"' hdl/processor.v)" = 2 ] || { echo "vfmax_link: could not mark the memories"; exit 1; }
for try in 1 2 3; do
    cmd //c "$(W "$VIVADO")" -mode batch -nojournal -log vivado.log \
        -source "$(W "$ROOT/Scripts/hw/vfmax_link.tcl")" \
        -tclargs "$(W "$DIR")" "$(W "$DIR/hdl")" "$PART" > run.log 2>&1 && break
    grep -q 'Synth 20-411' vivado.log && [ $try -lt 3 ] && { echo "Synth 20-411, again ($try)"; continue; }
    echo "VIVADO FAILED"; grep -m5 'ERROR' vivado.log; exit 1
done
cat result.txt | tee -a "$OUT/vfmax.txt"
