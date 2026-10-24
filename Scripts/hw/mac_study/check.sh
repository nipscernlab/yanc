#!/bin/bash
# Each MAC variant of mac.v against its model in mac.py, bit for bit, on the
# vectors of the accuracy study (docs/design/array-unit.md, section 7, step 2):
# A (lazy, W = 2M+8), B (window, 2 and 3 x NUBITS bits), C (full range), at
# 32/23/8 and 16/10/5, #FROUND 0 and 2. Every line must say "0 different".
# Needs iverilog and python3.
#
# Usage: bash Scripts/hw/mac_study/check.sh      (a few minutes)
ROOT=$(cd "$(dirname "$0")/../../.." && pwd)
S="$ROOT/Scripts/hw/mac_study"
OUT="$ROOT/.smoke/hw/mac_study"
fail=0
for F in "32 23 8 10" "16 10 5 8"; do
    set -- $F
    NB=$1 M=$2 E=$3 LGN=$4
    D="$OUT/$NB"
    mkdir -p "$D"
    python3 "$S/mac.py" gen $NB $M $E "$D" || exit 1
    for V in "A $((2*M+8)) 0" "B $((NB*2)) 1" "B $((NB*3)) 1" "C 0 2"; do
        set -- $V
        NAME=$1$2; [ "$1" = C ] && NAME=C
        for L in 0 2; do
            python3 "$S/mac.py" expect $NB $M $E "$D" $NAME $L || exit 1
            cp "$D/exp_${NAME}_L$L.txt" "$D/exp.txt"
            iverilog -g2012 -s tb_mac -Ptb_mac.VAR=$3 -Ptb_mac.NB=$NB -Ptb_mac.M=$M -Ptb_mac.E=$E \
                -Ptb_mac.W=$2 -Ptb_mac.LGN=$LGN -Ptb_mac.XB=$((E+4)) -Ptb_mac.FR=$L \
                -o "$D/m.vvp" "$S/tb_mac.v" "$S/mac.v" || exit 1
            out=$(cd "$D" && vvp -n m.vvp | grep -v '^VCD\|finish')
            echo "$NB/$M/$E $NAME FROUND $L: $out"
            echo "$out" | grep -q ' 0 different' || fail=1
        done
    done
done
exit $fail
