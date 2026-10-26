#!/bin/bash
cd /c/nipscern/yanc-dirac/.smoke/dirac-proto/a7
for v in 1 2; do /c/nipscern/yanc-dirac/bin/cmmcomp.exe -o o$v p$v.cmm > l$v 2>&1; echo "p$v rc=$?: $(grep -i error l$v | head -1)"; done
