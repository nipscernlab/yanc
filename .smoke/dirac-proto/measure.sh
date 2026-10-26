#!/bin/bash
# proc_rls compiled as main compiles it today, with [4] replaced by [N] and
# the read of x repeated N times. For each N: words of program (n_ins in
# app_log.txt), instruction width, and, when simulated, the clocks between
# the first OUT of two loop turns (out_probe.v).
# Usage: measure.sh SIM_NS BIG_NS   e.g. measure.sh "4 8 16" "32 128"
W=/c/nipscern/yanc-dirac
S=/c/nipscern/yanc-dirac/.smoke/dirac-proto
H=$W/SAPHO
T=$W/Compilers/CMMComp/Tests/proc_rls
SIM=$(echo ${1:-4,8,16} | tr , " ")
BIG=$(echo ${2:-32,128} | tr , " "); [ "$2" = none ] && BIG=""

mk() {  # mk N dir
  local N=$1 O=$2
  rm -rf "$O"; mkdir -p "$O"
  sed "s/\[4\]/[$N]/g" "$T/proc_rls.cmm" > "$O/rls.cmm"
  python3 - "$O/rls.cmm" "$N" <<'EOF'
import sys
p, n = sys.argv[1], int(sys.argv[2])
s = open(p, encoding='utf-8').read()
line = "        x # fin(0)*0.001 -> |x⟩;\n"
k = s.count(line)
assert k == 4, k
s = s.replace(line * k, line * n)
open(p, 'w', encoding='utf-8').write(s)
EOF
  cp "$T"/input_*.txt "$O/"
}

for N in $SIM; do
  O=$S/m/sim_$N; mk $N "$O"
  "$W/bin/cmmcomp.exe" -o "$O/rls" "$O/rls.cmm" > "$O/cmm.log" 2>&1 || { echo "N=$N cmmcomp failed"; tail -3 "$O/cmm.log"; continue; }
  "$W/bin/asmcomp.exe" -f 100 -c 200000 "$O/rls/rls.asm" > "$O/asm.log" 2>&1 || { echo "N=$N asmcomp failed"; tail -3 "$O/asm.log"; continue; }
  words=$(grep -m1 "^n_ins" "$O/rls/app_log.txt" | awk '{print $2}')
  iw=$(head -1 "$O/rls/rls_inst.mif" | tr -d '\r' | awk '{print length($1)}')
  sed "s/dot_c_tb.proc.p_dot_c.core/rls_tb.proc.p_rls.core/" "$S/out_probe.v" > "$O/probe.v"
  iverilog -s rls_tb -s out_probe -o "$O/s.vvp" "$O/rls/rls_tb.v" "$O/rls/rls.v" "$O/probe.v" $H/addr_dec.v $H/instr_dec.v $H/processor.v $H/core.v $H/ula.v > "$O/iv.log" 2>&1 || { echo "N=$N iverilog failed"; head -3 "$O/iv.log"; continue; }
  (cd "$O/rls" && vvp "$O/s.vvp" > "$O/vvp.log" 2>&1)
  turns=$(grep "TURN" "$O/vvp.log" | awk '{print $2}' | sort -u | tr '\n' ' ')
  echo "N=$N: $words words of $iw bits, clocks per turn seen: $turns"
done

for N in $BIG; do
  O=$S/m/big_$N; mk $N "$O"
  start=$(date +%s)
  "$W/bin/cmmcomp.exe" -o "$O/rls" "$O/rls.cmm" > "$O/cmm.log" 2>&1 || { echo "N=$N cmmcomp failed"; tail -3 "$O/cmm.log"; continue; }
  "$W/bin/asmcomp.exe" -f 100 -c 1000 "$O/rls/rls.asm" > "$O/asm.log" 2>&1 || { echo "N=$N asmcomp failed"; tail -3 "$O/asm.log"; continue; }
  words=$(grep -m1 "^n_ins" "$O/rls/app_log.txt" | awk '{print $2}')
  dat=$(grep -m1 "^n_dat" "$O/rls/app_log.txt" | awk '{print $2}')
  iw=$(head -1 "$O/rls/rls_inst.mif" | tr -d '\r' | awk '{print length($1)}')
  echo "N=$N: $words words of $iw bits ($((words*iw)) bits), $dat words of data, $(( $(date +%s) - start )) s"
done
