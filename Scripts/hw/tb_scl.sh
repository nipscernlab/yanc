#!/bin/bash
# Runs Scripts/hw/tb_scl.v, F_SCL at the ends of the exponent's range, at
# #FROUND 0, 1 and 2 (#184). Needs iverilog only. Exit status 1 on any
# mismatch.
#
# Usage: bash Scripts/hw/tb_scl.sh
ROOT=$(cd "$(dirname "$0")/../.." && pwd)
cd "$ROOT" || exit 1
TMP=$(mktemp -d)
trap 'rm -rf "$TMP"' EXIT
rc=0
for L in 0 1 2; do
    if iverilog -g2012 -s tb -Ptb.FROUND=$L -o "$TMP/tb.vvp" Scripts/hw/tb_scl.v SAPHO/ula.v 2>"$TMP/e.log"; then
        out=$(vvp -n "$TMP/tb.vvp" | grep -v '\$finish')
        echo "$out"
        case $out in *FAIL*) rc=1 ;; *": ok"*) ;; *) rc=1 ;; esac
    else
        echo "FROUND=$L: ELABORATION FAILED"; head -10 "$TMP/e.log"; rc=1
    fi
done
exit $rc
