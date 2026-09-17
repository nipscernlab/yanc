#!/bin/bash
# Runs Scripts/hw/tb_alu.v (shared shifter, F2I, float comparison) in four
# formats and at every #FROUND level, against references derived in the
# testbench. Needs iverilog only.
#
# Usage: bash Scripts/hw/tb_alu.sh [N]        (N = vectors per block, default 4000)
ROOT=$(cd "$(dirname "$0")/../.." && pwd)
cd "$ROOT" || exit 1
N=${1:-4000}
TMP=$(mktemp -d)
trap 'rm -rf "$TMP"' EXIT
rc=0

for FMT in "8 4 3" "16 10 5" "32 23 8" "64 52 11"; do
    set -- $FMT
    for L in 0 1 2; do
        if iverilog -g2012 -s tb -Ptb.NUBITS=$1 -Ptb.MAN=$2 -Ptb.EXP=$3 -Ptb.FROUND=$L -Ptb.N=$N \
                    -o "$TMP/tb.vvp" Scripts/hw/tb_alu.v HDL/ula.v 2>"$TMP/e.log"; then
            out=$(vvp -n "$TMP/tb.vvp" | grep -v '\$finish')
            echo "$out"
            case $out in *FAIL*) rc=1 ;; esac
        else
            echo "$1/$2/$3 FROUND=$L: ELABORATION FAILED"; head -10 "$TMP/e.log"; rc=1
        fi
    done
done
exit $rc
