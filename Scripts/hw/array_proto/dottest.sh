#!/bin/bash
# Step 4b: dot_gen.py's program through the prototype (array memory on),
# its outputs against the model. Usage: dottest.sh <FROUND> <seed>
D=$(dirname "$0")
W=$(cd "$D/../../.." && pwd)
H=$W/SAPHO
O=$W/.smoke/array_proto/dottest/fr$1_s$2
rm -rf "$O"; mkdir -p "$O"
export YANC_ARRMEM=1
python3 "$D/dot_gen.py" "$W/Scripts/hw/mac_study" "$O" "$1" "$2" || exit 1
od=$O/dot_t
"$W/bin/asmcomp.exe" -f 100 -c 20000 "$od/dot_t.asm" > "$O/asm.log" 2>&1 || { echo "asmcomp failed"; tail -3 "$O/asm.log"; exit 1; }
iverilog -s dot_t_tb -o "$od/dot_t.vvp" "$od/dot_t_tb.v" "$od/dot_t.v" $H/addr_dec.v $H/instr_dec.v $H/processor.v $H/core.v $H/ula.v > "$O/iv.log" 2>&1 || { echo "iverilog failed"; head -5 "$O/iv.log"; exit 1; }
(cd "$od" && vvp "$od/dot_t.vvp" > "$O/vvp.log" 2>&1)
paste -d' ' "$O/expect.txt" "$od/output_0.txt" | awk -v fr="$1" -v s="$2" '
  { n++; if ($1 == "-") { extra = extra " " $2; next } if ($1 == $2) ok++; else { bad++; print "  output " n ": expected " $1 ", got " $2 } }
  END { printf "FROUND %s seed %s: %d equal, %d different (by the float adder:%s)\n", fr, s, ok, bad, extra }'
grep -o 'ARRMEM(1)\|\.DOT(1)' "$od/dot_t.v" | tr '\n' ' '; echo
