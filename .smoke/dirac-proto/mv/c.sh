#!/bin/bash
cd /c/nipscern/yanc-dirac/.smoke/dirac-proto/mv
/c/nipscern/yanc-dirac/bin/cmmcomp.exe -o mv mv.cmm > cmm.log 2>&1 || cat cmm.log
grep -n "#array\|@main\|Lwh" mv/mv.asm | head -20
grep -c "" mv/mv.asm
