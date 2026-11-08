#!/bin/bash
# Runs Scripts/hw/tb_ret_priority.v on the processor of ret_priority.asm: a
# RET in decode does not win over a reset or an interrupt (#164, #166).
# Needs bin/yanc built and iverilog. Exit status 1 on any mismatch.
#
# Usage: bash Scripts/hw/tb_ret_priority.sh
ROOT=$(cd "$(dirname "$0")/../.." && pwd)
cd "$ROOT" || exit 1
TMP=$(mktemp -d)
trap 'rm -rf "$TMP"' EXIT
cp Scripts/hw/ret_priority.asm "$TMP/"
if ! (cd "$TMP" && "$ROOT/bin/yanc" ret_priority.asm >yanc.log 2>&1); then
    echo "tb_ret_priority: the program does not build"; cat "$TMP/yanc.log"; exit 1
fi
H=SAPHO
if ! iverilog -g2012 -s tb -o "$TMP/tb.vvp" Scripts/hw/tb_ret_priority.v "$TMP/ret_priority.v" \
        $H/processor.v $H/core.v $H/ula.v $H/addr_dec.v $H/instr_dec.v 2>"$TMP/e.log"; then
    echo "tb_ret_priority: ELABORATION FAILED"; head -10 "$TMP/e.log"; exit 1
fi
out=$(cd "$TMP" && vvp -n tb.vvp | grep -v '\$finish')
echo "$out"
case $out in *FAIL*) exit 1 ;; *": ok"*) exit 0 ;; *) exit 1 ;; esac
