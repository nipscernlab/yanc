#!/bin/bash
# Does the differential test (Scripts/iss/iss_diff.sh) catch a broken
# processor? Each mutant below is one small change to a copy of SAPHO/, in
# core.v or instr_dec.v (the ALU has its own test, Scripts/hw/tb_ula.sh); the
# test must fail on every one within the seeds given (GitHub issue #32).
#
# Usage: bash Scripts/iss/iss_mutants.sh [-n seeds]     (default 12)
# Exit status 1 if a mutant survives, or a change no longer applies (the
# line it edits moved: the mutant then tests nothing).
ROOT=$(cd "$(dirname "$0")/../.." && pwd)
cd "$ROOT" || exit 1
N=12
while getopts "n:" o; do case $o in n) N=$OPTARG ;; *) exit 2 ;; esac; done

TMP=$(mktemp -d)
trap 'rm -rf "$TMP"' EXIT

# name | file | sed expression (it must change the file)
MUTANTS=(
"JIZ tests the wrong bit|core.v|s/assign wJIZ = (opcode == OP_JIZ) \& ~is_um;/assign wJIZ = (opcode == OP_JIZ) \&  is_um;/"
"RET goes one word too far|core.v|s/assign addr = (ret_go) ? stack_out : pf_addr;/assign addr = (ret_go) ? stack_out + 1'b1 : pf_addr;/"
"the interrupt keeps the data stack|core.v|s/if (ITRADD > 0) begin : sp_clr_sel assign sp_clr = rst | itr; end/if (ITRADD > 0) begin : sp_clr_sel assign sp_clr = rst; end/"
"the interrupt keeps the INC write|core.v|s/inc_w <= id_inc \& ~(INC_ITR \& itr);/inc_w <= id_inc;/"
"ILI reads without the bit reversal|core.v|s/rel_addr #(.MDATAW(MDATAW), .FFTSIZ(FFTSIZ), .USEFFT(ILI)) ra_rd/rel_addr #(.MDATAW(MDATAW), .FFTSIZ(FFTSIZ), .USEFFT(0)) ra_rd/"
"the stack top is read before the push (a stack of 32 words)|core.v|s/assign out = (byp) ? bword : rtop;/assign out = rtop;/"
"a pop leaves the stack pointer|core.v|s/else if (pop ) pointer <= pmenoum;/else if (pop ) pointer <= pointer;/"
"S_ADD does not pop|instr_dec.v|/d24 /s/{6'd2 , 1'b0, 1'b1,/{6'd2 , 1'b0, 1'b0,/"
"SET_P keeps the accumulator|instr_dec.v|/d6 /s/{6'd1 , 1'b0, 1'b1, 1'b1,/{6'd0 , 1'b0, 1'b1, 1'b1,/"
"F_SU1 runs as F_SU2|instr_dec.v|/d35 /s/{6'd47,/{6'd48,/"
"P_INN does not push|instr_dec.v|/d13 /s/{6'd0 , 1'b1, 1'b0, 1'b0, 1'b1,/{6'd0 , 1'b0, 1'b0, 1'b0, 1'b1,/"
"QUO reads the remainder|instr_dec.v|/d111/s/{6'd6 ,/{6'd8 ,/"
"OUT does not raise out_en|instr_dec.v|/d15 /s/{6'd0 , 1'b0, 1'b0, 1'b0, 1'b0, 1'b1,/{6'd0 , 1'b0, 1'b0, 1'b0, 1'b0, 1'b0,/"
)

survived=0; stale=0
for m in "${MUTANTS[@]}"; do
    IFS='|' read -r name file expr <<< "$m"
    rm -rf "$TMP/H"; cp -r SAPHO "$TMP/H"
    sed -i "$expr" "$TMP/H/$file"
    if cmp -s "SAPHO/$file" "$TMP/H/$file"; then
        echo "STALE   $name: the change no longer applies to $file"; stale=$((stale+1)); continue
    fi
    if bash Scripts/iss/iss_diff.sh -n "$N" -q -H "$TMP/H" > "$TMP/out" 2>&1; then
        echo "SURVIVED $name ($N seeds)"; survived=$((survived+1))
    else
        echo "caught  $name: $(grep -m1 '^FAIL' "$TMP/out" | sed 's/:.*//')"
    fi
done
echo "iss_mutants: ${#MUTANTS[@]} mutants, $survived survived, $stale stale"
[ $survived -eq 0 ] && [ $stale -eq 0 ]
