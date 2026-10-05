#!/bin/bash
# Real Fmax and ALM count of a generated processor, through Quartus Prime Lite.
#
# Usage:  [FR=<0|1|2>] [TAG=<name>] [SEED=<n>] bash Scripts/hw/fmax.sh <proc> [device] [family]
# e.g.    bash Scripts/hw/fmax.sh cmm_cexp
#         FR=2 TAG=lvl2 bash Scripts/hw/fmax.sh cmm_cexp
#
# <proc> is a processor the regress already built: it reads
# .smoke/work/<proc>/Hardware/<proc>.v, whose .mif paths are absolute, so the
# memories initialise. SEED is the fitter's placement seed (default 1): a
# single fit can be off by several per cent, compare across seeds (give each
# its own TAG). The optimization mode below already runs the fitter's
# physical synthesis, retiming included (docs/pipeln-and-division.md): there
# is nothing more to switch on there. FR rewrites the .FROUND() parameter of
# that top (the .mif stay valid -- the level does not change the instruction
# encoding), which is how a C+- program gets timed at a level its source did
# not ask for.
#
# Besides the Fmax it writes the 10 worst paths (worst_paths.txt) and the
# worst one in full (worst_path_full.txt) in the project folder, and prints
# that one grouped by block (path_blocks.py): where the clock is lost.
#
# One fit takes 20 min to 2 h. RUN ONE AT A TIME: three in parallel brought a
# 22-thread machine to its knees. Results are appended to .smoke/hw/fmax.txt.
set -e
PROC=$1; DEV=${2:-5CSEMA5F31C6}; FAM=${3:-Cyclone V}
[ -n "$PROC" ] || { echo "usage: [FR=n] [TAG=x] fmax.sh <proc> [device] [family]"; exit 1; }

ROOT=$(cd "$(dirname "$0")/../.." && pwd)
QUARTUS=${QUARTUS:-/c/intelFPGA_lite/24.1std/quartus/bin64}
WORK="$ROOT/.smoke/work/$PROC/Hardware"
OUT="$ROOT/.smoke/hw"
DIR="$OUT/${PROC}_${DEV}${TAG:+_$TAG}"

[ -f "$WORK/$PROC.v" ] || { echo "no $WORK/$PROC.v -- run Scripts/regress.sh first"; exit 1; }
[ -x "$QUARTUS/quartus_map" ] || { echo "Quartus not at $QUARTUS (set QUARTUS=...)"; exit 1; }

rm -rf "$DIR"; mkdir -p "$DIR"; cd "$DIR"
cp "$WORK/$PROC.v" .
[ -n "$FR" ] && sed -i "s/\.FROUND([0-9])/.FROUND($FR)/" "$PROC.v"

HDLW=$(cygpath -m "$ROOT/SAPHO" 2>/dev/null || echo "$ROOT/SAPHO")

cat > clk.sdc <<EOF
create_clock -period 10.000 -name clk [get_ports clk]
derive_clock_uncertainty
EOF

cat > proj.tcl <<EOF
project_new $PROC -overwrite
set_global_assignment -name FAMILY "$FAM"
set_global_assignment -name DEVICE $DEV
set_global_assignment -name TOP_LEVEL_ENTITY $PROC
set_global_assignment -name VERILOG_FILE $PROC.v
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
"$QUARTUS/quartus_map" $PROC > map.log 2>&1 || { echo "$PROC: MAP FAILED";  grep -m5 'Error' map.log; exit 1; }
"$QUARTUS/quartus_fit" $PROC > fit.log 2>&1 || { echo "$PROC: FIT FAILED";  grep -m5 'Error' fit.log; exit 1; }
"$QUARTUS/quartus_sta" $PROC > sta.log 2>&1 || { echo "$PROC: STA FAILED";  grep -m5 'Error' sta.log; exit 1; }

ALM=$(grep -m1 -E 'Logic utilization \(in ALMs\)|Total logic elements' $PROC.fit.summary | sed 's/^[^:]*: *//')
FMAX=$(grep -A6 'Slow 1100mV 85C Model Fmax Summary\|Slow 1200mV 85C Model Fmax Summary' $PROC.sta.rpt \
       | grep -m1 'MHz' | awk -F';' '{print $2}' | tr -d ' ')
echo "$PROC $DEV${FR:+ FROUND=$FR}${TAG:+ ($TAG)}${SEED:+ seed $SEED}: Fmax=$FMAX logic=$ALM" | tee -a "$OUT/fmax.txt"

# Where the clock is lost: the worst register-to-register paths of the WHOLE
# processor (the ALU alone is a proxy, TODO.md item 8). worst_paths.txt lists
# the 10 worst (slack, from, to); worst_path_full.txt details the worst one,
# cell by cell, through the operand select, the ALU and the write-back.
cat > paths.tcl <<EOF
project_open $PROC
create_timing_netlist
read_sdc clk.sdc
update_timing_netlist
report_timing -setup -npaths 10 -detail summary   -file worst_paths.txt
report_timing -setup -npaths 1  -detail full_path -file worst_path_full.txt
delete_timing_netlist
project_close
EOF
"$QUARTUS/quartus_sta" -t paths.tcl > paths.log 2>&1 || { echo "$PROC: PATH REPORT FAILED"; grep -m5 'Error' paths.log; exit 1; }
echo "worst paths: $DIR/worst_paths.txt, worst in full: $DIR/worst_path_full.txt"
echo "the worst path, by block:"
PY=$(command -v python3 || command -v python)
"$PY" "$ROOT/Scripts/hw/path_blocks.py" worst_path_full.txt | tee -a "$OUT/fmax.txt"
