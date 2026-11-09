#!/bin/bash
# Runs Scripts/hw/tb_dec_reset.v: the decoder drives no memory write and no
# input read during a reset, whatever is in decode (#165). Needs iverilog.
# Exit status 1 on any mismatch.
#
# Usage: bash Scripts/hw/tb_dec_reset.sh
ROOT=$(cd "$(dirname "$0")/../.." && pwd)
cd "$ROOT" || exit 1
TMP=$(mktemp -d)
trap 'rm -rf "$TMP"' EXIT
if ! iverilog -g2012 -s tb -o "$TMP/tb.vvp" Scripts/hw/tb_dec_reset.v SAPHO/instr_dec.v 2>"$TMP/e.log"; then
    echo "tb_dec_reset: ELABORATION FAILED"; head -10 "$TMP/e.log"; exit 1
fi
out=$(vvp -n "$TMP/tb.vvp" | grep -v '\$finish')
echo "$out"
case $out in *FAIL*) exit 1 ;; *": ok"*) exit 0 ;; *) exit 1 ;; esac
