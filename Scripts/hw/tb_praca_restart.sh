#!/bin/bash
# Runs Scripts/hw/tb_praca_restart.v on the processors of praca_restart.asm
# and praca_tom.asm: a #PRACA pulse empties both stacks, and a pulse during a
# TOM wait makes the simulation warn (#188).
# Needs bin/yanc built and iverilog. Exit status 1 on any mismatch.
#
# Usage: bash Scripts/hw/tb_praca_restart.sh
ROOT=$(cd "$(dirname "$0")/../.." && pwd)
cd "$ROOT" || exit 1
TMP=$(mktemp -d)
trap 'rm -rf "$TMP"' EXIT
for p in praca_restart praca_tom; do
    cp "Scripts/hw/$p.asm" "$TMP/"
    if ! (cd "$TMP" && "$ROOT/bin/yanc" "$p.asm" >"$p.log" 2>&1); then
        echo "tb_praca_restart: $p does not build"; cat "$TMP/$p.log"; exit 1
    fi
done
H=SAPHO
if ! iverilog -g2012 -s tb -o "$TMP/tb.vvp" Scripts/hw/tb_praca_restart.v "$TMP/praca_restart.v" "$TMP/praca_tom.v" \
        $H/processor.v $H/core.v $H/ula.v $H/addr_dec.v $H/instr_dec.v 2>"$TMP/e.log"; then
    echo "tb_praca_restart: ELABORATION FAILED"; head -10 "$TMP/e.log"; exit 1
fi
out=$(cd "$TMP" && vvp -n tb.vvp | grep -v '\$finish')
echo "$out"
case $out in *"while toma(0) waits"*) ;; *) echo "FAIL: no warning for the pulse during TOM 0"; exit 1 ;; esac
case $out in *FAIL*) exit 1 ;; *": ok"*) exit 0 ;; *) exit 1 ;; esac
