#!/bin/bash
# Step 4b: the fitter's seed sweep, sapho_all on the DE10-Nano, one fit at a
# time (about an hour each: mains power). Three versions, each SEEDS:
#   arr     the array memory, no DOT (step 4a)
#   dob     a DOT and a DOB added before #TOAQUI, the result through ALU in1
#   ulaout  the same program on a copy of SAPHO/ with the result in ula_out's
#           place (idea 2, dropped in 665ef7f; the copy made by mk_ulaout.py)
# Usage: [SEEDS="2 3"] [VERSIONS="arr dob ulaout"] bash sweep.sh
# Each fit is TAG <version>_s<seed> in .smoke/hw/ and a line in .smoke/hw/fmax.txt.
D=$(cd "$(dirname "$0")" && pwd)
W=$(cd "$D/../../.." && pwd)
COPY=$W/.smoke/array_proto/sapho_ulaout
python3 "$D/mk_ulaout.py" "$W/SAPHO" "$COPY" || exit 1
for S in ${SEEDS:-2 3}; do
  for V in ${VERSIONS:-arr dob ulaout}; do
    if [ $V = ulaout ]; then export HDL=$COPY; else unset HDL; fi
    bash "$D/arrtest.sh" sapho_all || exit 1
    P=$W/.smoke/work/sapho_all/sapho_all
    rm -rf "$W/.smoke/work/sapho_all"; mkdir -p "$W/.smoke/work/sapho_all"
    cp -r "$W/.smoke/array_proto/arrtest/sapho_all/sapho_all" "$W/.smoke/work/sapho_all/"
    if [ $V != arr ]; then
      ( cd "$P" &&
        awk '/^#TOAQUI/ && !done { print "LEA fa"; print "P_LEA fa"; print "DOT 8"; print "OUT 0"; print "LEA fa"; print "P_LOD 8"; print "DOB fa"; print "OUT 0"; done = 1 } { print }' sapho_all.asm > t.asm && mv t.asm sapho_all.asm &&
        YANC_ARRMEM=1 "$W/bin/asmcomp.exe" -f 100 -c 100000 sapho_all.asm > asm.log 2>&1 ) || { echo "$V: asmcomp failed"; exit 1; }
    fi
    echo "$V seed $S: $(grep -o 'ARRMEM(1)\|\.DOT(1)\|\.DOB(1)' "$P/sapho_all.v" | tr '\n' ' ')${HDL:+ HDL=$HDL} $(date +%H:%M)"
    SEED=$S TAG=${V}_s$S bash "$W/Scripts/hw/fmax.sh" sapho_all 5CSEBA6U23I7 > "$W/.smoke/array_proto/sweep_${V}_s$S.log" 2>&1
    grep -m1 'Fmax=' "$W/.smoke/array_proto/sweep_${V}_s$S.log"
  done
done
date
