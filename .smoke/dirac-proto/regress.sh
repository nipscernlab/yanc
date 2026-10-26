#!/bin/bash
cd /c/nipscern/yanc-dirac || exit 1
bash Scripts/regress.sh > /c/nipscern/yanc-dirac/.smoke/dirac-proto/regress.log 2>&1
echo "regress rc=$?"
