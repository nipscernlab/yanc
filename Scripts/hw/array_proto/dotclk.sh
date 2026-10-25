#!/bin/bash
# Step 4b: the clocks DOT and DOB stay decoded for several N, and whether the
# next instruction loses a fetch clock. dot_probe.v counts the clocks with the
# opcode decoded and, for each, the clocks until the next OUT is decoded.
# Vectors of 1.0 (the SAPHO word 1967128576), so the outputs read N.
D=$(cd "$(dirname "$0")" && pwd)
W=$(cd "$D/../../.." && pwd)
H=$W/SAPHO
O=$W/.smoke/array_proto/dotclk
rm -rf "$O"; mkdir -p "$O/dot_c"
export YANC_ARRMEM=1
{
echo "#PRNAME dot_c"; echo "#NUBITS 32"; echo "#NBMANT 23"; echo "#NBEXPO 8"
echo "#NUIOIN 1"; echo "#NUIOOU 1"; echo "#FROUND 0"
echo "#arrayd a 1 40 40 $(for k in $(seq 40); do printf '%d ' 1967128576; done)"
for n in 0 1 2 3 5 40; do echo "LEA a"; echo "P_LEA a"; echo "DOT $n"; echo "OUT 0"; done
for n in 0 1 2 3 5 40; do echo "LEA a"; echo "P_LOD $n"; echo "DOB a"; echo "OUT 0"; done
echo "@fim JMP fim"
} > "$O/dot_c/dot_c.asm"
cd "$O/dot_c"
"$W/bin/asmcomp.exe" -f 100 -c 2000 dot_c.asm > ../asm.log 2>&1 || { echo "asmcomp failed"; tail -3 ../asm.log; exit 1; }
iverilog -s dot_c_tb -s dot_probe -o s.vvp dot_c_tb.v dot_c.v "$D/dot_probe.v" $H/addr_dec.v $H/instr_dec.v $H/processor.v $H/core.v $H/ula.v > ../iv.log 2>&1 || { echo "iverilog failed"; head -5 ../iv.log; exit 1; }
vvp s.vvp | grep DOT
echo "N: 0 1 2 3 5 40, DOT then DOB; outputs: $(tr '\n' ' ' < output_0.txt)"
