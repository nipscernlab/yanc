#!/bin/bash
# Runs Scripts/hw/tb_ula.v (the whole ALU, every opcode on) in three formats
# and at every #FROUND level, against references derived in the testbench.
# Needs iverilog only. Exit status 1 on any mismatch.
#
# Usage: bash Scripts/hw/tb_ula.sh [N]        (N = vectors per operation, default 2000)
ROOT=$(cd "$(dirname "$0")/../.." && pwd)
cd "$ROOT" || exit 1
N=${1:-2000}
TMP=$(mktemp -d)
trap 'rm -rf "$TMP"' EXIT
rc=0

# the formats asmcomp accepts (NBMANT < 2^(NBEXPO-1)) that the fixtures use
for FMT in "16 10 5" "23 16 6" "32 23 8"; do
    set -- $FMT
    for L in 0 1 2; do
        if iverilog -g2012 -s tb -Ptb.NUBITS=$1 -Ptb.MAN=$2 -Ptb.EXP=$3 -Ptb.FROUND=$L -Ptb.N=$N \
                    -o "$TMP/tb.vvp" Scripts/hw/tb_ula.v SAPHO/ula.v 2>"$TMP/e.log"; then
            out=$(vvp -n "$TMP/tb.vvp" | grep -v '\$finish')
            echo "$out"
            case $out in *FAIL*|*operations\ listed*) rc=1 ;; esac
        else
            echo "$1/$2/$3 FROUND=$L: ELABORATION FAILED"; head -10 "$TMP/e.log"; rc=1
        fi
    done
done
exit $rc
