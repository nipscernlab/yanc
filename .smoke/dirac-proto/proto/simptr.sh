#!/bin/bash
# sapho_ptr (blocks on, unused) against sapho_all's output
P=/c/nipscern/yanc-dirac/.smoke/dirac-proto/proto/SAPHO
W=/c/nipscern/yanc-dirac/.smoke/work
D=/c/nipscern/yanc-dirac/.smoke/dirac-proto/proto/simptr
rm -rf $D; mkdir -p $D/sapho_ptr; cd $D
cp $W/sapho_ptr/sapho_ptr/* sapho_ptr/
sed 's/sapho_all/sapho_ptr/g' $W/sapho_all/sapho_all/sapho_all_tb.v > sapho_ptr/sapho_ptr_tb.v
ls $W/sapho_all/ | head
cp $W/sapho_all/input_* . 2>/dev/null; cp $W/sapho_all/input_* sapho_ptr/ 2>/dev/null
iverilog -s sapho_ptr_tb -o s.vvp sapho_ptr/sapho_ptr_tb.v sapho_ptr/sapho_ptr.v $P/addr_dec.v $P/instr_dec.v $P/processor.v $P/core.v $P/ula.v 2>&1 | head
(cd sapho_ptr && timeout 600 vvp -n ../s.vvp > ../vvp.log 2>&1)
for f in $W/sapho_all/sapho_all/output_*.txt; do b=$(basename $f); cmp $f sapho_ptr/$b && echo "$b same ($(wc -l < $f) lines)"; done
