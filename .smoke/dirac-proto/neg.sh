#!/bin/bash
# the Dirac negative fixtures: the message each gives, in both languages
W=/c/nipscern/yanc-dirac
S=/c/nipscern/yanc-dirac/.smoke/dirac-proto
mkdir -p $S/neg
for f in $W/Compilers/CMMComp/NegTests/dirac_*.cmm; do
  n=$(basename $f .cmm)
  for l in en pt; do
    rm -rf $S/neg/$n; mkdir -p $S/neg/$n; cp $f $S/neg/$n/
    "$W/bin/cmmcomp.exe" --lang=$l -o "$S/neg/$n/out" "$S/neg/$n/$n.cmm" > $S/neg/$n/log 2>&1
    echo "$n [$l] rc=$?: $(grep -iE 'error|erro' $S/neg/$n/log | head -1)"
  done
done
