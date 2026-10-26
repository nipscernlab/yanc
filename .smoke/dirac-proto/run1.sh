#!/bin/bash
# compile and simulate a small C± program: run1.sh dir name
W=/c/nipscern/yanc-dirac; H=$W/SAPHO
D=$1; n=$2; cd "$D" || exit 1
"$W/bin/cmmcomp.exe" -o "$D/$n" "$D/$n.cmm" > cmm.log 2>&1 || { echo cmmcomp failed; cat cmm.log; exit 1; }
"$W/bin/asmcomp.exe" -f 100 -c ${3:-2000} "$D/$n/$n.asm" > asm.log 2>&1 || { echo asmcomp failed; tail -3 asm.log; exit 1; }
iverilog -s ${n}_tb -o s.vvp $n/${n}_tb.v $n/$n.v $H/addr_dec.v $H/instr_dec.v $H/processor.v $H/core.v $H/ula.v > iv.log 2>&1 || { cat iv.log; exit 1; }
(cd $n && vvp ../s.vvp > ../vvp.log 2>&1)
echo "--- output:"; cat $n/output_0.txt
