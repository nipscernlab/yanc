#!/bin/bash
# The loop version of proc_rls (gen_loop.py): words, clocks per turn and the
# output compared with the Dirac fixture's golden at N=4.
# usage: run_loop.sh N,N,... variant [nosim]
W=/c/nipscern/yanc-dirac
S=/c/nipscern/yanc-dirac/.smoke/dirac-proto
H=$W/SAPHO
T=$W/Compilers/CMMComp/Tests/proc_rls
V=$2
for N in $(echo $1 | tr , " "); do
  O=$S/m/loop_${V}_$N; rm -rf "$O"; mkdir -p "$O"
  python3 "$S/gen_loop.py" $N $V "$O/rls.cmm"
  cp "$T"/input_*.txt "$O/"
  "$W/bin/cmmcomp.exe" -o "$O/rls" "$O/rls.cmm" > "$O/cmm.log" 2>&1 || { echo "N=$N cmmcomp failed"; tail -5 "$O/cmm.log"; continue; }
  "$W/bin/asmcomp.exe" -f 100 -c 400000 "$O/rls/rls.asm" > "$O/asm.log" 2>&1 || { echo "N=$N asmcomp failed"; tail -3 "$O/asm.log"; continue; }
  words=$(grep -m1 "^n_ins" "$O/rls/app_log.txt" | awk '{print $2}')
  iw=$(head -1 "$O/rls/rls_inst.mif" | tr -d '\r' | awk '{print length($1)}')
  if [ "$3" = nosim ]; then echo "loop $V N=$N: $words words of $iw bits"; continue; fi
  sed "s/dot_c_tb.proc.p_dot_c.core/rls_tb.proc.p_rls.core/" "$S/out_probe60.v" > "$O/probe.v"
  iverilog -s rls_tb -s out_probe -o "$O/s.vvp" "$O/rls/rls_tb.v" "$O/rls/rls.v" "$O/probe.v" $H/addr_dec.v $H/instr_dec.v $H/processor.v $H/core.v $H/ula.v > "$O/iv.log" 2>&1 || { echo "N=$N iverilog failed"; head -3 "$O/iv.log"; continue; }
  (cd "$O/rls" && vvp "$O/s.vvp" > "$O/vvp.log" 2>&1)
  turns=$(grep "TURN" "$O/vvp.log" | awk '{print $2}' | sort -u | tr '\n' ' ')
  same=""
  if [ $N = 4 ]; then cmp -s "$O/rls/output_0.txt" "$T/golden_sim/output_0.txt" && same="output = golden" || same="output differs from golden"; fi
  echo "loop $V N=$N: $words words of $iw bits, clocks per turn seen: $turns $same"
done
