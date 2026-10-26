#!/bin/bash
cd /c/nipscern/yanc-dirac/.smoke/dirac-proto/lp
/c/nipscern/yanc-dirac/bin/cmmcomp.exe -o dot dot.cmm > cmm.log 2>&1 || cat cmm.log
cat dot/dot.asm
ls dot
