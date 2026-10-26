#!/bin/bash
S=/c/nipscern/yanc-dirac/.smoke/dirac-proto
echo "== alias (old: 2 2)";  bash $S/run1.sh $S/a1 al 3000 | tail -2 | tr '\n' ' '; echo
echo "== shift (old: 0 0 0)"; bash $S/run1.sh $S/a2 sh 3000 | tail -3 | tr '\n' ' '; echo
echo "== outer (old: 10 20 30 60 90 180 270 340)"; bash $S/run1.sh $S/a3 op 5000 | tail -8 | tr '\n' ' '; echo
echo "== or (want 11 7 3)"; bash $S/run1.sh $S/a6 orr 3000 | tail -3 | tr '\n' ' '; echo
