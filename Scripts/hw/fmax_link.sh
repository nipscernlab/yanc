#!/bin/bash
# Real Fmax of two linked processors on two clocks, through Quartus Prime Lite:
# the LINKALL pair (docs/toma-and-cade.md 8.6), sapho_all_a hands its result
# to sapho_all_b over a synchronized toma/cade link.
#
# Usage:  [TAG=<name>] [SEED=<n>] bash Scripts/hw/fmax_link.sh [device] [family]
#         bash Scripts/hw/fmax_link.sh 5CSEBA6U23I7      # DE10-Nano
#
# Reads the regress-built .smoke/work/sapho_all_{a,b}/<name>/*.v (absolute
# .mif paths, so the memories initialise). The top is Scripts/hw/linkall_top.v,
# the pair as a board would hold it. The two clocks are declared asynchronous:
# every path between them is a toma/cade bit (into a two-flip-flop
# synchronizer) or a word that the protocol holds still for at least two
# receiver cycles before it is read (toma_dado, and A's held output) -- clk.sdc
# is the constraint a board project needs (docs/toma-and-cade-links.md).
#
# One fit takes 20 min to 2 h: RUN ONE AT A TIME. Results go to
# .smoke/hw/fmax.txt (one Fmax per clock).
set -e
DEV=${1:-5CSEMA5F31C6}; FAM=${2:-Cyclone V}
ROOT=$(cd "$(dirname "$0")/../.." && pwd)
QUARTUS=${QUARTUS:-/c/intelFPGA_lite/24.1std/quartus/bin64}
OUT="$ROOT/.smoke/hw"
DIR="$OUT/linkall_${DEV}${TAG:+_$TAG}"
TOP=linkall_top

for p in sapho_all_a sapho_all_b; do
    [ -f "$ROOT/.smoke/work/$p/$p/$p.v" ] || { echo "no .smoke/work/$p/$p/$p.v -- run Scripts/regress.sh first"; exit 1; }
done
[ -x "$QUARTUS/quartus_map" ] || { echo "Quartus not at $QUARTUS (set QUARTUS=...)"; exit 1; }

rm -rf "$DIR"; mkdir -p "$DIR"; cd "$DIR"
cp "$ROOT/.smoke/work/sapho_all_a/sapho_all_a/sapho_all_a.v" "$ROOT/.smoke/work/sapho_all_b/sapho_all_b/sapho_all_b.v" .
cp "$ROOT/Scripts/hw/linkall_top.v" .
HDLW=$(cygpath -m "$ROOT/SAPHO" 2>/dev/null || echo "$ROOT/SAPHO")

cat > clk.sdc <<'EOF'
create_clock -period 10.000 -name clk_a [get_ports clk_a]
create_clock -period 10.000 -name clk_b [get_ports clk_b]
derive_clock_uncertainty
# toma/cade between the two domains: bits into two-flip-flop synchronizers,
# words held still by the protocol (docs/toma-and-cade-links.md)
set_clock_groups -asynchronous -group [get_clocks clk_a] -group [get_clocks clk_b]
EOF

cat > proj.tcl <<EOF
project_new $TOP -overwrite
set_global_assignment -name FAMILY "$FAM"
set_global_assignment -name DEVICE $DEV
set_global_assignment -name TOP_LEVEL_ENTITY $TOP
set_global_assignment -name VERILOG_FILE $TOP.v
set_global_assignment -name VERILOG_FILE sapho_all_a.v
set_global_assignment -name VERILOG_FILE sapho_all_b.v
set_global_assignment -name VERILOG_FILE $HDLW/processor.v
set_global_assignment -name VERILOG_FILE $HDLW/core.v
set_global_assignment -name VERILOG_FILE $HDLW/ula.v
set_global_assignment -name VERILOG_FILE $HDLW/instr_dec.v
set_global_assignment -name VERILOG_FILE $HDLW/addr_dec.v
set_global_assignment -name SDC_FILE clk.sdc
set_global_assignment -name NUM_PARALLEL_PROCESSORS ALL
set_global_assignment -name OPTIMIZATION_MODE "HIGH PERFORMANCE EFFORT"
set_global_assignment -name SEED ${SEED:-1}
export_assignments
project_close
EOF

"$QUARTUS/quartus_sh"  -t proj.tcl > tcl.log 2>&1
"$QUARTUS/quartus_map" $TOP > map.log 2>&1 || { echo "MAP FAILED"; grep -m5 'Error' map.log; exit 1; }
"$QUARTUS/quartus_fit" $TOP > fit.log 2>&1 || { echo "FIT FAILED"; grep -m5 'Error' fit.log; exit 1; }
"$QUARTUS/quartus_sta" $TOP > sta.log 2>&1 || { echo "STA FAILED"; grep -m5 'Error' sta.log; exit 1; }

ALM=$(grep -m1 -E 'Logic utilization \(in ALMs\)|Total logic elements' $TOP.fit.summary | sed 's/^[^:]*: *//')
FMAX=$(grep -A8 -E 'Slow 1[12]00mV (85|100)C Model Fmax Summary' $TOP.sta.rpt \
       | grep 'MHz' | awk -F';' '{gsub(/ /,"",$2); gsub(/ /,"",$4); printf "%s=%s ", $4, $2}')
echo "linkall (sapho_all_a -> sapho_all_b, two clocks) $DEV${TAG:+ ($TAG)}${SEED:+ seed $SEED}: $FMAX logic=$ALM" | tee -a "$OUT/fmax.txt"

# the worst path of each clock, and every path that crosses between them
# (all cut by set_clock_groups: listed to show what the constraint covers)
cat > paths.tcl <<EOF
project_open $TOP
create_timing_netlist
read_sdc clk.sdc
update_timing_netlist
report_timing -setup -npaths 5 -detail summary -from_clock clk_a -to_clock clk_a -file worst_a.txt
report_timing -setup -npaths 5 -detail summary -from_clock clk_b -to_clock clk_b -file worst_b.txt
report_clock_transfers -file transfers.txt
delete_timing_netlist
project_close
EOF
"$QUARTUS/quartus_sta" -t paths.tcl > paths.log 2>&1 || { echo "PATH REPORT FAILED"; grep -m5 'Error' paths.log; exit 1; }
echo "worst paths: $DIR/worst_a.txt, worst_b.txt; clock transfers: $DIR/transfers.txt"
