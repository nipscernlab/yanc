#!/bin/bash
# Step 4b: sapho_all with a DOT added by hand (on its float array fa, in the
# main loop, before #TOAQUI), and with "dob" a DOB as well, the array memory
# on; simulated once to see it runs, then fitted on the DE10-Nano (about an
# hour: mains power). Usage: fit4b.sh [dot|dob]
D=$(dirname "$0")
W=$(cd "$D/../../.." && pwd)
H=${HDL:-$W/SAPHO}     # HDL=<dir>: a modified copy of SAPHO/ (fmax.sh takes the same)
TAG=${1:-dot}
bash "$D/arrtest.sh" sapho_all || exit 1
P=$W/.smoke/work/sapho_all/sapho_all
rm -rf "$W/.smoke/work/sapho_all"; mkdir -p "$W/.smoke/work/sapho_all"
cp -r "$W/.smoke/array_proto/arrtest/sapho_all/sapho_all" "$W/.smoke/work/sapho_all/"
cd "$P"
if [ "$TAG" = dob ]; then
    add='print "LEA fa"; print "P_LEA fa"; print "DOT 8"; print "OUT 0"; print "LEA fa"; print "P_LOD 8"; print "DOB fa"; print "OUT 0"'
else
    add='print "LEA fa"; print "P_LEA fa"; print "DOT 8"; print "OUT 0"'
fi
awk "/^#TOAQUI/ && !done { $add; done = 1 } { print }" sapho_all.asm > t.asm && mv t.asm sapho_all.asm
grep -n -B1 -A4 '^DOT' sapho_all.asm
export YANC_ARRMEM=1
"$W/bin/asmcomp.exe" -f 100 -c 100000 sapho_all.asm > asm.log 2>&1 || { echo "asmcomp failed"; tail -3 asm.log; exit 1; }
grep -o 'ARRMEM(1)\|\.DOT(1)' sapho_all.v | tr '\n' ' '; echo
iverilog -s sapho_all_tb -o s.vvp sapho_all_tb.v sapho_all.v $H/addr_dec.v $H/instr_dec.v $H/processor.v $H/core.v $H/ula.v > iv.log 2>&1 || { echo "iverilog failed"; head -3 iv.log; exit 1; }
vvp s.vvp > vvp.log 2>&1
echo "simulated: $(wc -l < output_0.txt) outputs, first: $(head -12 output_0.txt | tr '\n' ' ')"
rm -f s.vvp *.vcd
date
TAG=$TAG bash "$W/Scripts/hw/fmax.sh" sapho_all 5CSEBA6U23I7
date
