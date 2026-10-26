#!/bin/bash
# bits of P and w after 20 turns: Dirac fixture against each loop variant
S=/c/nipscern/yanc-dirac/.smoke/dirac-proto
T=/c/nipscern/yanc-dirac/Compilers/CMMComp/Tests/proc_rls
bash $S/run1.sh $S/db db 60000 > /dev/null
cmp -s $S/db/db/output_0.txt $T/golden_sim/output_0.txt && echo "dirac+tail: port 0 = golden"
echo "dirac+tail: $(wc -l < $S/db/db/output_1.txt) words on port 1"
for v in same fix sub nat; do
  bash $S/run_loop.sh 4 $v
  if cmp -s $S/m/loop_${v}_4/rls/output_1.txt $S/db/db/output_1.txt; then echo "  $v: bits of P and w = Dirac"
  else echo "  $v: bits differ in $(paste -d' ' $S/m/loop_${v}_4/rls/output_1.txt $S/db/db/output_1.txt | awk '$1!=$2' | wc -l) of 20 words"; fi
done
