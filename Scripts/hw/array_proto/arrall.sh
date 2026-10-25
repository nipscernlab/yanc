#!/bin/bash
# Every C+- fixture with a golden_sim and a .cmm of its own name through arrtest.sh
D=$(dirname "$0")
W=$(cd "$D/../../.." && pwd)
for t in "$W"/Compilers/CMMComp/Tests/*/; do
    n=$(basename "$t")
    [ -d "$t/golden_sim" ] && [ -f "$t/$n.cmm" ] || continue
    timeout 900 bash "$D/arrtest.sh" "$n" || echo "$n: (status $?)"
done
