#!/bin/bash
S=/c/nipscern/yanc-dirac/.smoke/dirac-proto
echo "== operand order probe (bad pairs out of 1500)"
bash $S/run1.sh $S/cm cm
echo "== bits"
bash $S/bits.sh
echo "== N=32 simulated (input runs out after 400 reads; the timing has no data-dependent branch)"
bash $S/run_loop.sh 32 same
bash $S/measure.sh 32 none
