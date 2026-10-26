#!/bin/bash
# How accurate is each MAC variant of the array unit? (docs/design/array-unit.md,
# section 7, step 1). mac.py writes the vectors (inner products of 4 to 1024
# terms: positive, mixed signs, a wide range of magnitudes, exact cancellation,
# a third of zeros); tb.v runs today's loop (F_MLT then F_ADD) on SAPHO/ula.v at
# each #FROUND; mac.py models the variants bit for bit and compares all of them
# with the exact sum. At 32/23/8 and 16/10/5. Touches no YANC code. Needs
# iverilog and python3.
#
# Usage: bash Scripts/hw/mac_study/run.sh [HDL]    (HDL: a modified copy of SAPHO/, default SAPHO/)
# A few minutes, most of it the Python models.
ROOT=$(cd "$(dirname "$0")/../../.." && pwd)
HDL=${1:-$ROOT/SAPHO}
OUT="$ROOT/.smoke/hw/mac_study"
for F in "32 23 8" "16 10 5"; do
    set -- $F
    D="$OUT/$1"
    mkdir -p "$D"
    python3 "$ROOT/Scripts/hw/mac_study/mac.py" gen $1 $2 $3 "$D" || exit 1
    for L in 0 1 2; do
        iverilog -g2012 -Ptb.NB=$1 -Ptb.NM=$2 -Ptb.NE=$3 -Ptb.FR=$L -o "$D/t$L.vvp" \
            "$ROOT/Scripts/hw/mac_study/tb.v" "$HDL/ula.v" || exit 1
        (cd "$D" && vvp -n "t$L.vvp" > /dev/null) || exit 1
    done
    python3 "$ROOT/Scripts/hw/mac_study/mac.py" report $1 $2 $3 "$D" | tee "$D/report.txt"
done
