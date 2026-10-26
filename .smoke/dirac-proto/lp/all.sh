#!/bin/bash
D=/c/nipscern/yanc-dirac/.smoke/dirac-proto/lp
for v in "$@"; do bash $D/a.sh $v; done
