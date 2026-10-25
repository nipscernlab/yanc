#!/bin/bash
# Every C++ test simulated with iverilog (no .in) through the prototype, the
# array memory on, output_0.txt against golden.txt. The .in tests (Verilator)
# are listed as skipped.
W=$(cd "$(dirname "$0")/../../.." && pwd)
H=${HDL:-$W/SAPHO}     # HDL=<dir>: a modified copy of SAPHO/ (fmax.sh takes the same)
export YANC_ARRMEM=1
for e in "$W"/Compilers/CPPComp/Tests/test*/; do
    b=$(basename "$e")
    [ -f "$e/$b.cpp" ] || continue
    if [ -f "$e/$b.in" ]; then echo "$b: skipped (Verilator)"; continue; fi
    O=$W/.smoke/array_proto/cpptest/$b; od=$O/$b
    rm -rf "$O"; mkdir -p "$O"
    "$W/bin/cppcomp.exe" -o "$od" "$e/$b.cpp" > "$O/cpp.log" 2>&1 || { echo "$b: cppcomp failed"; continue; }
    "$W/bin/asmcomp.exe" -f 100 -c 5000000 "$od/$b.asm" > "$O/asm.log" 2>&1 || { echo "$b: asmcomp failed: $(tail -1 $O/asm.log)"; continue; }
    iverilog -s "${b}_tb" -o "$od/$b.vvp" "$od/${b}_tb.v" "$od/$b.v" $H/addr_dec.v $H/instr_dec.v $H/processor.v $H/core.v $H/ula.v > "$O/iv.log" 2>&1 || { echo "$b: iverilog failed: $(head -2 $O/iv.log)"; continue; }
    (cd "$od" && timeout 900 vvp "$od/$b.vvp" > "$O/vvp.log" 2>&1)
    if [ ! -f "$e/golden.txt" ]; then echo "$b: no golden.txt"; continue; fi
    if cmp -s "$od/output_0.txt" "$e/golden.txt"; then echo "$b: OK"; else echo "$b: output_0.txt DIFFERS"; fi
done
