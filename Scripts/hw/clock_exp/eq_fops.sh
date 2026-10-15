#!/bin/bash
# The whole ALU of pnorm against a variant's (eq_fops.sh <variant> [N]) on the
# eight operations that go through the normaliser, in five formats and
# rounding levels, with normalised operands (RAW=0) and with any bit pattern
# (RAW=1). Every output must be the same. Run make_variants.py first.
# Exit 0 when all RAW=0 runs are equal (RAW=1 is reported, not required: some
# methods assume normalised operands).
set -u
ROOT=$(cd "$(dirname "$0")/../../.." && pwd)
V=${1:?variant}
N=${2:-20000}
REF="$ROOT/.smoke/hw/clock_exp/pnorm/ula.v"
NEW="$ROOT/.smoke/hw/clock_exp/$V/ula.v"
TB="$ROOT/Scripts/hw/clock_exp/tb_fops.v"
D="$ROOT/.smoke/hw/clock_exp/eqf_$V"
CACHE="$ROOT/.smoke/hw/clock_exp/eqf_ref"
mkdir -p "$CACHE"
[ -f "$NEW" ] || { echo "no $NEW: run make_variants.py first"; exit 1; }
mkdir -p "$D"; cd "$D" || exit 1
fail=0
# NUBITS MAN EXP FROUND
for cfg in "32 23 8 0" "32 23 8 1" "32 23 8 2" "16 10 5 0" "16 10 5 2"; do
    set -- $cfg
    for raw in 0 1; do
        P="-Ptb_fops.NUBITS=$1 -Ptb_fops.MAN=$2 -Ptb_fops.EXP=$3 -Ptb_fops.FROUND=$4 -Ptb_fops.RAW=$raw -Ptb_fops.N=$N"
        # the reference print is kept between runs (same tb, pnorm, cfg and N)
        C="$CACHE/ref_${1}_${2}_${3}_${4}_${raw}_$N.txt"
        if [ ! -s "$C" ] || [ "$C" -ot "$REF" ] || [ "$C" -ot "$TB" ]; then
            iverilog -g2012 -s tb_fops $P -o ref.vvp "$TB" "$REF" || { echo "build ref failed: $cfg"; fail=1; continue; }
            vvp -n ref.vvp | grep -v finish > "$C.tmp" && mv "$C.tmp" "$C"
        fi
        cp "$C" ref.txt
        iverilog -g2012 -s tb_fops $P -o new.vvp "$TB" "$NEW" || { echo "build new failed: $cfg"; fail=1; continue; }
        vvp -n new.vvp | grep -v finish > new.txt
        if cmp -s ref.txt new.txt; then echo "same  ($cfg raw=$raw): $(wc -l < ref.txt) outputs"
        else
            nd=$(diff ref.txt new.txt | grep -c '^<')
            echo "DIFF  ($cfg raw=$raw): $nd of $(wc -l < ref.txt) outputs, by op: $(diff ref.txt new.txt | grep '^<' | awk '{print $2}' | sort -n | uniq -c | tr -s ' \n' ' ')"
            diff ref.txt new.txt | head -4
            [ $raw = 0 ] && fail=1
        fi
    done
done
exit $fail
