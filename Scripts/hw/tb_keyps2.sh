#!/bin/bash
# Runs Scripts/hw/tb_keyps2.v (SAPHO/myKeyPS2.v) for both layouts, at a small
# clock (fast) and at 25 MHz (the board's PLL clock), with the default queue
# (DEPTH 4), and the queue at 1 and 16 once. Needs iverilog only.
# Exit status 1 on any mismatch.
ROOT=$(cd "$(dirname "$0")/../.." && pwd)
cd "$ROOT" || exit 1
TMP=$(mktemp -d)
trap 'rm -rf "$TMP"' EXIT
rc=0
for C in "US 2000 4" "US 25000 4" "ABNT2 2000 4" "ABNT2 25000 4" "US 2000 1" "ABNT2 2000 16"; do
    set -- $C; L=$1; K=$2; D=$3
    if iverilog -g2012 -s tb -Ptb.LAYOUT=\"$L\" -Ptb.CLK_KHZ=$K -Ptb.DEPTH=$D -o "$TMP/k.vvp" \
                Scripts/hw/tb_keyps2.v SAPHO/myKeyPS2.v 2>"$TMP/e.log"; then
        out=$(vvp -n "$TMP/k.vvp" | grep -v '\$finish')
        echo "$out"
        case $out in *FAIL*|"") rc=1 ;; esac
    else
        echo "$L CLK_KHZ=$K DEPTH=$D: ELABORATION FAILED"; head -10 "$TMP/e.log"; rc=1
    fi
done
exit $rc
