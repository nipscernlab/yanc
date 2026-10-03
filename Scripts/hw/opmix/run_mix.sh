#!/bin/bash
# Re-simulate every program the regress simulated with Icarus, with probe.v
# counting the ALU operation of each cycle. One histogram per program in mix/.
S=$(cd "$(dirname "$0")" && pwd); R=$(cd "$S/../../.." && pwd); H=$R/SAPHO
mkdir -p "$R/.smoke/hw/opmix"; S_OUT="$R/.smoke/hw/opmix"
mkdir -p $S_OUT/mix
one() {   # one <name> <prname> <tb.v> <proc.v> <rundir>
    local name=$1 pr=$2 tb=$3 pv=$4 run=$5
    local fim=$(grep -m1 "^@fim " "$run/app_log.txt" 2>/dev/null | awk '{print $2}')
    local out=$(cygpath -m "$S_OUT/mix/$name.txt" 2>/dev/null || echo "$S_OUT/mix/$name.txt")
    iverilog -g2012 -s ${pr}_tb -s probe -DTB=${pr}_tb -DCORE=${pr}_tb.proc.p_${pr}.core \
        -DOUTF="\"$out\"" -DFIM=${fim:--1} -o $S_OUT/mix/$name.vvp "$S/probe.v" "$tb" "$pv" \
        $H/addr_dec.v $H/instr_dec.v $H/processor.v $H/core.v $H/ula.v >/dev/null 2>&1 || { echo "$name BUILD-FAIL"; return; }
    ( cd "$run" && vvp $S_OUT/mix/$name.vvp >/dev/null 2>&1 )
    [ -f "$S_OUT/mix/$name.txt" ] && echo "$name ok" || echo "$name NO-OUTPUT"
}
for d in $R/.smoke/tmp/*/; do
    p=$(basename $d)
    [ -f "$d/${p}_tb.v" ] && [ -f "$R/.smoke/work/$p/Hardware/$p.v" ] || continue
    one "cmm_$p" $p "$d/${p}_tb.v" "$R/.smoke/work/$p/Hardware/$p.v" "$d"
done
for d in $R/.smoke/work_cpp/test*/; do
    t=$(basename $d)
    tb=$(ls $d/_tmp/*_tb.v 2>/dev/null | head -1); [ -n "$tb" ] || continue
    [ -f "$d/Software/$t.in" ] && continue          # Verilator tests: no Icarus run
    pr=$(basename $tb _tb.v)
    one "cpp_$t" $pr "$tb" "$d/Hardware/$pr.v" "$d/_tmp"
done
echo done
