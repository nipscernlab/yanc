#!/bin/bash
# Build the tools of this worktree with the flags CI gates on.
W=/c/nipscern/yanc-dirac
S=/c/nipscern/yanc-dirac/.smoke/dirac-proto
cd "$W" || exit 1
make CFLAGS="-O2 -Wall -Werror" > "$S/make.log" 2>&1
rc=$?
echo "make rc=$rc"
tail -3 "$S/make.log"
which iverilog vvp python3
ls -la "$W/bin/cmmcomp.exe" "$W/bin/asmcomp.exe"
