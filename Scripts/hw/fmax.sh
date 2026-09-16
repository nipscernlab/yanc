#!/bin/bash
# Real Fmax and ALM count of a generated processor, through Quartus Prime Lite.
#
# Usage:  [FR=<0|1|2>] [TAG=<name>] bash Scripts/hw/fmax.sh <proc> [device] [family]
# e.g.    bash Scripts/hw/fmax.sh cmm_cexp
#         FR=2 TAG=lvl2 bash Scripts/hw/fmax.sh cmm_cexp
#
# <proc> is a processor the regress already built: it reads
# .smoke/work/<proc>/Hardware/<proc>.v, whose .mif paths are absolute, so the
# memories initialise. FR rewrites the .FROUND() parameter of that top (the
# .mif stay valid -- the level does not change the instruction encoding), which
# is how a C+- program gets timed at a level its source did not ask for.
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

HDLW=$(cygpath -m "$ROOT/HDL" 2>/dev/null || echo "$ROOT/HDL")

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
echo "$PROC $DEV${FR:+ FROUND=$FR}${TAG:+ ($TAG)}: Fmax=$FMAX logic=$ALM" | tee -a "$OUT/fmax.txt"
