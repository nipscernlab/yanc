#!/bin/bash
# fit_normalisers.sh that can be stopped and started again: it skips every fit
# whose result line is already in .smoke/hw/fmax.txt (a laptop closed in the
# middle of a fit loses only that fit). Same variants, seeds and tags.
cd "$(dirname "$0")/../../.." || exit 1
ROOT=$PWD
LOG="$ROOT/.smoke/hw/fmax.txt"
run() {   # run <variant> <seed>
    tag="de10_${1}_s$2"
    if grep -qF "($tag) seed $2: Fmax=" "$LOG" 2>/dev/null; then
        echo "$tag: done before, skipped"; return
    fi
    HDL="$ROOT/.smoke/hw/clock_exp/$1" TAG="$tag" SEED=$2 \
        bash Scripts/hw/fmax.sh sapho_all 5CSEBA6U23I7 2>&1 | head -1
}
for s in 2 3; do run pnorm $s; done
for v in nshift rpar nsr; do
    for s in 1 2 3; do run $v $s; done
done
echo ALL_DONE
