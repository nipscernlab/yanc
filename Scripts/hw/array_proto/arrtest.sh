#!/bin/bash
# One C+- fixture through the prototype, with the array memory on, its
# simulated output against golden_sim. Usage: arrtest.sh <fixture> [off]
# The generated folder is left in .smoke/array_proto/arrtest/<fixture>/<fixture>.
W=$(cd "$(dirname "$0")/../../.." && pwd)
T=$W/Compilers/CMMComp/Tests/$1
O=$W/.smoke/array_proto/arrtest/$1
rm -rf "$O"; mkdir -p "$O"
[ "$2" = off ] || export YANC_ARRMEM=1
cp "$T/$1.cmm" "$O/"
cp "$T"/*.txt "$O/" 2>/dev/null
"$W/bin/cmmcomp.exe" -o "$O/$1" "$O/$1.cmm" > "$O/cmm.log" 2>&1 || { echo "$1: cmmcomp failed"; exit 1; }
"$W/bin/asmcomp.exe" -f 100 -c 100000 "$O/$1/$1.asm" > "$O/asm.log" 2>&1 || { echo "$1: asmcomp failed: $(tail -2 $O/asm.log)"; exit 1; }
H=$W/SAPHO
iverilog -s "${1}_tb" -o "$O/$1.vvp" "$O/$1/${1}_tb.v" "$O/$1/$1.v" $H/addr_dec.v $H/instr_dec.v $H/processor.v $H/core.v $H/ula.v > "$O/iv.log" 2>&1 || { echo "$1: iverilog failed: $(head -3 $O/iv.log)"; exit 1; }
(cd "$O/$1" && vvp "$O/$1.vvp" > "$O/vvp.log" 2>&1) || { echo "$1: vvp failed"; exit 1; }
bad=0
for f in "$T"/golden_sim/output_*.txt; do
    b=$(basename "$f")
    cmp -s "$f" "$O/$1/$b" || { echo "$1: $b DIFFERS"; bad=1; }
done
[ $bad = 0 ] && echo "$1: OK ($(grep -o 'ARRMEM\|MARRS([0-9]*)' "$O/$1/$1.v" | tr '\n' ' '))"
exit $bad
