#!/bin/bash
# How accurate is the float addition at each #FROUND level? (docs/design/fround1-subtraction.md)
# tb.v drives SAPHO/ula.v's F_ADD at 32/23/8 with 200000 random pairs (values
# 1 to 1024, random signs) and 300 running sums of 1000 such values; check.py
# compares every result with the exact one (Python fractions): how many pairs
# are not the exact truncation and by how many units of the last place, and
# the error of each running sum. Needs iverilog and python3.
#
# Usage: bash Scripts/hw/fadd_acc/run.sh [HDL]    (HDL: a modified copy of SAPHO/, default SAPHO/)
# About 10 minutes, most of it the exact arithmetic of check.py.
ROOT=$(cd "$(dirname "$0")/../../.." && pwd)
HDL=${1:-$ROOT/SAPHO}
OUT="$ROOT/.smoke/hw/fadd_acc"
for L in 0 1 2; do
    mkdir -p "$OUT/L$L"
    iverilog -g2012 -Ptb.FR=$L -o "$OUT/L$L/t.vvp" "$ROOT/Scripts/hw/fadd_acc/tb.v" "$HDL/ula.v" || exit 1
    (cd "$OUT/L$L" && vvp -n t.vvp > /dev/null)
    echo "== FROUND $L"
    python3 "$ROOT/Scripts/hw/fadd_acc/check.py" "$OUT/L$L"
done
