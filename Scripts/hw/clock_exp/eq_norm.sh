#!/bin/bash
# ula_norm of SAPHO/ against the tree leading-zero count of the lzc experiment:
# 200000 random inputs (biased to many leading zeros and to zero mantissas) in
# nine formats and rounding levels; every output must be the same.
# Run make_variants.py first. Exit 0 when all are equal.
set -u
ROOT=$(cd "$(dirname "$0")/../../.." && pwd)
D="$ROOT/.smoke/hw/clock_exp/eq"
OLD="$ROOT/SAPHO/ula.v"
NEW="$ROOT/.smoke/hw/clock_exp/lzc/ula.v"
TB="$ROOT/Scripts/hw/clock_exp/tb_norm.v"
[ -f "$NEW" ] || { echo "no $NEW: run make_variants.py first"; exit 1; }
mkdir -p "$D"; cd "$D" || exit 1
fail=0
# MAN EXP FROUND G LZC DIR
for cfg in "23 8 0 0 1 0" "23 8 1 0 1 1" "23 8 2 3 1 1" "23 8 2 3 1 0" \
           "10 5 2 3 1 1" "10 5 0 0 1 0" "52 11 2 3 1 1" "4 3 0 0 1 0" "4 3 2 3 1 1"; do
    set -- $cfg
    P="-Ptb_norm.MAN=$1 -Ptb_norm.EXP=$2 -Ptb_norm.FROUND=$3 -Ptb_norm.G=$4 -Ptb_norm.LZC=$5 -Ptb_norm.DIR=$6"
    iverilog -g2012 -s tb_norm $P -o old.vvp "$TB" "$OLD" || { echo "build old failed: $cfg"; fail=1; continue; }
    iverilog -g2012 -s tb_norm $P -o new.vvp "$TB" "$NEW" || { echo "build new failed: $cfg"; fail=1; continue; }
    vvp -n old.vvp | grep -v finish > old.txt
    vvp -n new.vvp | grep -v finish > new.txt
    if cmp -s old.txt new.txt; then echo "same  ($cfg): $(wc -l < old.txt) outputs"
    else echo "DIFF  ($cfg)"; diff old.txt new.txt | head -4; fail=1; fi
done
exit $fail
