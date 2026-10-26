#!/bin/bash
# bison report of CMMComp.y in a scratch folder: conflicts and their states
S=/c/nipscern/yanc-dirac/.smoke/dirac-proto
mkdir -p $S/bison && cd $S/bison || exit 1
cp /c/nipscern/yanc-dirac/Compilers/CMMComp/Sources/CMMComp.y .
bison --version | head -1
bison -y -d -v CMMComp.y 2>&1 | head -20
grep -n "conflict" y.output | head -40
