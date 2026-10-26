#!/bin/bash
S=/c/nipscern/yanc-dirac/.smoke/dirac-proto
echo "== operand order probe (bad pairs out of 1500)"
bash $S/run1.sh $S/cm cm 200000
echo "== bits"
bash $S/bits.sh
