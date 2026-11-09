#!/bin/bash
# Runs Scripts/hw/tb_toaqui_return.v on the processor of toaqui_return.asm:
# cheguei rises at a #TOAQUI point reached through a RET (#189).
# Needs bin/yanc built and iverilog. Exit status 1 on any mismatch.
#
# Usage: bash Scripts/hw/tb_toaqui_return.sh
ROOT=$(cd "$(dirname "$0")/../.." && pwd)
cd "$ROOT" || exit 1
TMP=$(mktemp -d)
trap 'rm -rf "$TMP"' EXIT
cp Scripts/hw/toaqui_return.asm "$TMP/"
if ! (cd "$TMP" && "$ROOT/bin/yanc" toaqui_return.asm >yanc.log 2>&1); then
    echo "tb_toaqui_return: the program does not build"; cat "$TMP/yanc.log"; exit 1
fi
H=SAPHO
if ! iverilog -g2012 -s tb -o "$TMP/tb.vvp" Scripts/hw/tb_toaqui_return.v "$TMP/toaqui_return.v" \
        $H/processor.v $H/core.v $H/ula.v $H/addr_dec.v $H/instr_dec.v 2>"$TMP/e.log"; then
    echo "tb_toaqui_return: ELABORATION FAILED"; head -10 "$TMP/e.log"; exit 1
fi
out=$(cd "$TMP" && vvp -n tb.vvp | grep -v '\$finish')
echo "$out" | head -12
case $out in *FAIL*) exit 1 ;; *": ok"*) exit 0 ;; *) exit 1 ;; esac
