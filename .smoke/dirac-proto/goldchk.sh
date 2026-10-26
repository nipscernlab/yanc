#!/bin/bash
# compile a fixture and compare its .asm with golden.asm
W=/c/nipscern/yanc-dirac
S=/c/nipscern/yanc-dirac/.smoke/dirac-proto
for t in "$@"; do
  T=$W/Compilers/CMMComp/Tests/$t; O=$S/g/$t; rm -rf $O; mkdir -p $O
  cp $T/$t.cmm $O/
  "$W/bin/cmmcomp.exe" -o "$O/$t" "$O/$t.cmm" > "$O/cmm.log" 2>&1 || { echo "$t: cmmcomp failed"; tail -3 $O/cmm.log; continue; }
  tr -d '\r' < $O/$t/$t.asm > $O/new.asm
  tr -d '\r' < $T/golden.asm > $O/gold.asm
  if cmp -s $O/new.asm $O/gold.asm; then echo "$t: asm = golden"
  else echo "$t: asm DIFFERS"; diff $O/new.asm $O/gold.asm | head -10; fi
done
