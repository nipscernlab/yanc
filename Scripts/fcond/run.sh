#!/bin/bash
# The float-as-a-condition test of TODO item 33: the host g++ gives the C
# answer, cppcomp and cmmcomp (#FROUND 0/1/2) run on the simulated processor,
# compare.py lines them up. BIN (default .smoke/bin) and HDL (default SAPHO/)
# pick the compilers and the processor, so a prototype can be tried.
#   bash Scripts/fcond/run.sh [out-dir]
set -u
ROOT=$(cd "$(dirname "$0")/../.." && pwd)
OUT=${1:-$ROOT/.smoke/fcond}
BIN=${BIN:-$ROOT/.smoke/bin}
HDL=${HDL:-$ROOT/SAPHO}
. "$ROOT/Scripts/env.sh" >/dev/null 2>&1
IV=${IVERILOG:-iverilog}; VV=${VVP:-vvp}

python3 "$ROOT/Scripts/fcond/gen.py" "$OUT" || exit 1
cd "$OUT" || exit 1

# the reference: the same C++ file through the host compiler
g++ -std=c++17 -include host.h -o host.exe fcond.cpp > host_build.log 2>&1 || { echo "host g++ failed"; cat host_build.log; exit 1; }
./host.exe > ref.txt

sim() {   # $1 program name (folder written by the compiler)
    ( cd "$1" || exit 1
      "$BIN/asmcomp.exe" -f 100 -c 200000 "$1.asm" > asm.log 2>&1 || { echo "$1: asmcomp failed"; tail -3 asm.log; exit 1; }
      "$IV" -s "$1_tb" -o "$1.vvp" "$1_tb.v" "$1.v" "$HDL/addr_dec.v" "$HDL/instr_dec.v" \
            "$HDL/processor.v" "$HDL/core.v" "$HDL/ula.v" > iv.log 2>&1 || { echo "$1: iverilog failed"; head -3 iv.log; exit 1; }
      "$VV" "$1.vvp" > vvp.log 2>&1
      cp output_0.txt "../out_$1.txt" )
}

rm -rf fcond fcond_l0 fcond_l1 fcond_l2 out_*.txt
"$BIN/cppcomp.exe" -o fcond fcond.cpp > cpp_build.log 2>&1 || { echo "cppcomp failed"; tail -5 cpp_build.log; exit 1; }
sim fcond
for l in 0 1 2; do
    "$BIN/cmmcomp.exe" -o fcond_l$l fcond_l$l.cmm > cmm_build_l$l.log 2>&1 || { echo "cmmcomp (level $l) failed"; tail -5 cmm_build_l$l.log; exit 1; }
    sim fcond_l$l
done

python3 "$ROOT/Scripts/fcond/compare.py" .
