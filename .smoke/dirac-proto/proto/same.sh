#!/bin/bash
# the prototype core with NPTR=0, RPT=0 against the original on the Mv program
P=/c/nipscern/yanc-dirac/.smoke/dirac-proto/proto/SAPHO
cd /c/nipscern/yanc-dirac/.smoke/dirac-proto/mv/run_mv_r
iverilog -s mv_tb -s out_probe -o p.vvp mv/mv_tb.v mv/mv.v probe.v $P/addr_dec.v $P/instr_dec.v $P/processor.v $P/core.v $P/ula.v 2>&1 | head
cp mv/output_0.txt out_orig.txt
(cd mv && vvp ../p.vvp > ../p.log 2>&1)
cmp out_orig.txt mv/output_0.txt && echo "prototype core, NPTR=0 RPT=0: same output"; grep TURN p.log | sort -u | head -3
