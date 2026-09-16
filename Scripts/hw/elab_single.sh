#!/bin/bash
# Elaborates HDL/ula.v with ONE float operator at a time, at every #FROUND
# level: every generate branch has to stand on its own. A block that stopped
# being gated by the opcodes shows up here as a FAIL (or as a block that only
# elaborates when some other operator is present).
# Usage: bash Scripts/hw/elab_single.sh
ROOT=$(cd "$(dirname "$0")/../.." && pwd)
cd "$ROOT" || exit 1
TMP=$(mktemp -d)
trap 'rm -rf "$TMP"' EXIT
rc=0

for L in 0 1 2; do
    for OP in F_ADD F_SU1 F_SU2 F_MLT F_DIV I2F I2F_M F2I F2I_M F_ROT F_LES F_GRE F_NEG F_ABS F_PST F_SGN F_SCL XPO; do
        if iverilog -g2012 -s ula -Pula.FROUND=$L -Pula.$OP=1 -o /dev/null HDL/ula.v 2>"$TMP/e.log"; then
            printf "L%s %-7s ok  " "$L" "$OP"
        else
            printf "\nL%s %-7s FAIL\n" "$L" "$OP"; head -3 "$TMP/e.log"; rc=1
        fi
    done
    echo
done
exit $rc
