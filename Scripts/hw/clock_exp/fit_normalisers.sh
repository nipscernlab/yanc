#!/bin/bash
# The normaliser comparison of 2026-10-15: pnorm (reference) and its three
# alternatives, three placement seeds each, one Quartus fit at a time
# (hours: mains power). pnorm seed 1 is the earlier de10_pnorm fit.
# Results in .smoke/hw/fmax.txt, tagged <variant>_s<seed>.
cd "$(dirname "$0")/../../.." || exit 1
ROOT=$PWD
run() {   # run <variant> <seed>
    HDL="$ROOT/.smoke/hw/clock_exp/$1" TAG="de10_${1}_s$2" SEED=$2 \
        bash Scripts/hw/fmax.sh sapho_all 5CSEBA6U23I7 2>&1 | head -1
}
for s in 2 3; do run pnorm $s; done
for v in nshift rpar nsr; do
    for s in 1 2 3; do run $v $s; done
done
echo ALL_DONE
