#!/bin/bash
# Runs Scripts/hw/tb_div_monitor.v on the processor of div_monitor.asm:
# the rounding-error monitors show a division's real error (#183).
# Needs bin/yanc built and iverilog. Exit status 1 on any mismatch.
#
# Usage: bash Scripts/hw/tb_div_monitor.sh
ROOT=$(cd "$(dirname "$0")/../.." && pwd)
cd "$ROOT" || exit 1
TMP=$(mktemp -d)
trap 'rm -rf "$TMP"' EXIT
cp Scripts/hw/div_monitor.asm "$TMP/"
if ! (cd "$TMP" && "$ROOT/bin/yanc" div_monitor.asm >yanc.log 2>&1); then
    echo "tb_div_monitor: the program does not build"; cat "$TMP/yanc.log"; exit 1
fi
H=SAPHO
if ! iverilog -g2012 -s tb -o "$TMP/tb.vvp" Scripts/hw/tb_div_monitor.v "$TMP/div_monitor.v" \
        $H/processor.v $H/core.v $H/ula.v $H/addr_dec.v $H/instr_dec.v 2>"$TMP/e.log"; then
    echo "tb_div_monitor: ELABORATION FAILED"; head -10 "$TMP/e.log"; exit 1
fi
out=$(cd "$TMP" && vvp -n tb.vvp | grep -v '\$finish')
echo "$out" | head -12
case $out in *FAIL*) exit 1 ;; *": ok"*) exit 0 ;; *) exit 1 ;; esac
