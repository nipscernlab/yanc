#!/bin/bash
# Width sweep: does each simulator compute every integer operator right at a
# given word width? Every operator of width_sweep/ops.v (continuous assigns, the
# way ula.v writes them) and the same expressions in a procedural block, against
# an oracle of Python big integers. Decides whether YANC can go beyond 32 bits
# (TODO.md item 6): re-run it when Icarus or Verilator is upgraded.
#
# Usage: bash Scripts/hw/width_sweep.sh              (32 64 128, 3000 vectors)
#        WIDTHS="64 256" N=10000 bash Scripts/hw/width_sweep.sh
# Widths must be multiples of 4. Icarus's procedural `/` is slow at 256 bits
# (~5 min for 3000 vectors). Results land in .smoke/hw/width/.
ROOT=$(cd "$(dirname "$0")/../.." && pwd)
S="$ROOT/Scripts/hw/width_sweep"
D="$ROOT/.smoke/hw/width"
mkdir -p "$D"
N=${N:-3000}
WIDTHS=${WIDTHS:-"32 64 128"}
VW="-Wno-fatal -Wno-lint -Wno-WIDTH -Wno-UNOPTFLAT -Wno-CASEINCOMPLETE -Wno-DECLFILENAME"
echo "$(iverilog -V 2>&1 | head -1) | $(verilator --version 2>/dev/null)"

ms () { echo $(( ($2 - $1) / 1000000 )); }   # nanoseconds -> milliseconds

for W in $WIDTHS; do
    echo "################ W=$W ################"
    python3 "$S/gen_vectors.py" "$W" "$N" 1 "$D/vec_$W.txt" > /dev/null || exit 1

    if iverilog -g2012 -s tb -Ptb.W=$W -Ptb.VEC=\"$D/vec_$W.txt\" -o "$D/tb_$W.vvp" "$S/tb_width.v" "$S/ops.v" 2> "$D/iv_$W.log"; then
        s=$(date +%s%N); vvp -n "$D/tb_$W.vvp" | grep -v '\$finish'; e=$(date +%s%N)
        echo "  icarus run: $(ms $s $e) ms"
    else
        echo "  iverilog FAILED to elaborate:"; head -5 "$D/iv_$W.log"
    fi

    # Verilator builds share TMP: run this script alone, never two at once.
    O="$D/obj_$W"; rm -rf "$O"; mkdir -p "$O"
    s=$(date +%s%N)
    if verilator --cc --exe --build -j 4 $VW --x-assign fast -GW=$W --top-module ops -Mdir "$O" \
            "$S/ops.v" "$S/sim_width.cpp" > "$D/vl_$W.log" 2>&1; then
        e=$(date +%s%N); echo "  verilator build: $(ms $s $e) ms"
        "$O/Vops" "$D/vec_$W.txt" "$W"
    else
        echo "  verilator FAILED to build:"; grep -iE 'undefined reference|error:' "$D/vl_$W.log" | head -6
    fi
done
