#!/bin/bash
# assemble and simulate one variant: a.sh name   (reads name.asm, writes into run_name)
D=/c/nipscern/yanc-dirac/.smoke/dirac-proto/lp
H=/c/nipscern/yanc-dirac/SAPHO
n=$1; R=$D/run_$n; rm -rf $R; mkdir -p $R/dot; cp $D/$n.asm $R/dot/dot.asm; cp $D/input_0.txt $R/; cp $D/input_0.txt $R/dot/
cd $R
/c/nipscern/yanc-dirac/bin/asmcomp.exe -f 100 -c 20000 dot/dot.asm > asm.log 2>&1 || { tail -5 asm.log; exit 1; }
sed "s/dot_c_tb.proc.p_dot_c.core/dot_tb.proc.p_dot.core/" $D/../out_probe60.v > probe.v
iverilog -s dot_tb -s out_probe -o s.vvp dot/dot_tb.v dot/dot.v probe.v $H/addr_dec.v $H/instr_dec.v $H/processor.v $H/core.v $H/ula.v > iv.log 2>&1 || { head iv.log; exit 1; }
(cd dot && vvp ../s.vvp > ../vvp.log 2>&1)
words=$(grep -m1 "^n_ins" dot/app_log.txt | awk '{print $2}')
echo "$n: $words words; clocks per turn: $(grep TURN vvp.log | awk '{print $2}' | sort -u | tr '\n' ' '); out: $(tr '\n' ' ' < dot/output_0.txt)"
