#!/bin/bash
# Runs Scripts/hw/tb_nrm.v: NRM (ula_nrm, a shift since #160) equals the
# signed division by NUGAIN for every 8-bit input and NUGAIN 1 to 64. Needs
# iverilog. Exit status 1 on any mismatch.
#
# Usage: bash Scripts/hw/tb_nrm.sh
ROOT=$(cd "$(dirname "$0")/../.." && pwd)
cd "$ROOT" || exit 1
TMP=$(mktemp -d)
trap 'rm -rf "$TMP"' EXIT
if ! iverilog -g2012 -s tb -o "$TMP/tb.vvp" Scripts/hw/tb_nrm.v SAPHO/ula.v 2>"$TMP/e.log"; then
    echo "tb_nrm: ELABORATION FAILED"; head -10 "$TMP/e.log"; exit 1
fi
out=$(vvp -n "$TMP/tb.vvp" | grep -v '\$finish')
echo "$out"
case $out in *FAIL*) exit 1 ;; *": ok"*) exit 0 ;; *) exit 1 ;; esac
