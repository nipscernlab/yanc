#!/bin/bash
# Runs Scripts/hw/tb_vgatext.v (SAPHO/myVGAText.v) with an 8-bit and a 32-bit
# word (or the widths given: tb_vgatext.sh 8). Needs iverilog only. Exit
# status 1 on any mismatch. About 3 minutes a width.
ROOT=$(cd "$(dirname "$0")/../.." && pwd)
cd "$ROOT" || exit 1
TMP=$(mktemp -d)
trap 'rm -rf "$TMP"' EXIT
rc=0
for N in ${@:-8 32}; do
    if iverilog -g2012 -s tb -Ptb.NUBITS=$N -o "$TMP/v.vvp" \
                Scripts/hw/tb_vgatext.v SAPHO/myVGAText.v 2>"$TMP/e.log"; then
        out=$(vvp -n "$TMP/v.vvp" | grep -v '\$finish')
        echo "$out"
        case $out in *FAIL*|"") rc=1 ;; esac
    else
        echo "NUBITS=$N: ELABORATION FAILED"; head -10 "$TMP/e.log"; rc=1
    fi
done
exit $rc
