#!/bin/bash
# Elaborates HDL/ula.v at every #FROUND level, in two formats, and lints it.
# Usage: bash Scripts/hw/elab.sh
ROOT=$(cd "$(dirname "$0")/../.." && pwd)
cd "$ROOT" || exit 1
TMP=$(mktemp -d)
trap 'rm -rf "$TMP"' EXIT

OPS="-Pula.F_ADD=1 -Pula.F_SU1=1 -Pula.F_SU2=1 -Pula.F_MLT=1 -Pula.F_DIV=1 -Pula.I2F=1 -Pula.I2F_M=1 -Pula.F2I=1 -Pula.F_NEG=1 -Pula.F_SGN=1 -Pula.F_LES=1 -Pula.F_GRE=1 -Pula.F_ROT=1 -Pula.ADD=1"
OPS="$OPS -Pula.SHL=1 -Pula.SHR=1 -Pula.SRS=1 -Pula.DIV=1 -Pula.MOD=1"
rc=0

for L in 0 1 2; do
    for FMT in "32 23 8" "16 10 5"; do
        set -- $FMT
        if iverilog -g2012 -s ula -Pula.FROUND=$L -Pula.NUBITS=$1 -Pula.NBMANT=$2 -Pula.NBEXPO=$3 \
                    $OPS -o /dev/null HDL/ula.v 2>"$TMP/e.log"; then
            echo "$1/$2/$3 FROUND=$L: ok"
        else
            echo "$1/$2/$3 FROUND=$L: FAIL"; head -20 "$TMP/e.log"; rc=1
        fi
    done
done

# Verilator lint at level 2 (what the C++ flow builds). WIDTH/UNOPTFLAT/
# CASEINCOMPLETE are the known, pre-existing classes -- see TODO item 10.7.
echo "--- verilator lint (FROUND=2)"
verilator --lint-only -Wno-fatal -Wno-DECLFILENAME -Wno-UNUSEDPARAM -Wno-UNUSEDSIGNAL \
          --top-module ula -GFROUND=2 -GF_ADD=1 -GF_SU1=1 -GF_SU2=1 -GF_MLT=1 -GF_DIV=1 \
          -GI2F=1 -GF2I=1 -GF_NEG=1 -GF_LES=1 -GF_GRE=1 HDL/ula.v 2>&1 \
    | grep -v '^%Warning-\(WIDTH\|UNOPTFLAT\|CASEINCOMPLETE\)' | head -30
echo "lint done"
exit $rc
