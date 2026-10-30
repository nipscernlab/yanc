#!/bin/bash
# RAM blocks of the array memory, by organisation (docs/design/array-unit.md,
# section 7, step 3): mem.v synthesized by Quartus (Cyclone V of the DE10-Nano)
# and Vivado (the ZYBO's Z-7010), 32-bit words. MODE 0 simple dual port
# (today's mem_data), 1 true dual port.
#
# Usage: [MODES=1] bash Scripts/hw/mac_study/mem.sh [depths...]     (default 256, the one measured;
#        the block count does not change the decision, Luciano 2026-10-24)
# Quartus: synthesis and fit (the block count is the fitter's); Vivado: synthesis.
# Results appended to .smoke/hw/mac_study/mem.txt.
ROOT=$(cd "$(dirname "$0")/../../.." && pwd)
S="$ROOT/Scripts/hw/mac_study"
QUARTUS=${QUARTUS:-/c/intelFPGA_lite/24.1std/quartus/bin64}
VIVADO=${VIVADO:-/c/AMDDesignTools/2025.2/Vivado/bin/vivado.bat}
W_() { cygpath -m "$1" 2>/dev/null || echo "$1"; }
OUTF="$ROOT/.smoke/hw/mac_study/mem.txt"
for D in ${@:-256}; do
  for MODE in ${MODES:-0 1}; do
    DIR="$ROOT/.smoke/hw/mac_study/mem_${MODE}_$D"
    rm -rf "$DIR"; mkdir -p "$DIR"; cd "$DIR"; cp "$S/mem.v" .
    cat > proj.tcl <<EOF
project_new mem_top -overwrite
set_global_assignment -name FAMILY "Cyclone V"
set_global_assignment -name DEVICE 5CSEBA6U23I7
set_global_assignment -name TOP_LEVEL_ENTITY mem_top
set_global_assignment -name VERILOG_FILE mem.v
set_global_assignment -name NUM_PARALLEL_PROCESSORS ALL
set_parameter -name MODE $MODE
set_parameter -name D $D
foreach p {wea web aa[*] ab[*] da[*] db[*] qa[*] qb[*]} { set_instance_assignment -name VIRTUAL_PIN ON -to \$p }
export_assignments
project_close
EOF
    "$QUARTUS/quartus_sh" -t proj.tcl > tcl.log 2>&1
    "$QUARTUS/quartus_map" mem_top > map.log 2>&1 || { echo "MAP FAILED $MODE $D"; continue; }
    # an array Quartus did not take as RAM becomes thousands of registers and
    # a fit of an hour: stop there and say why
    grep -q 'Info (276014)' map.log && { echo "MODE $MODE depth $D: NOT INFERRED AS RAM"; grep -m2 '276009' map.log; continue; }
    "$QUARTUS/quartus_fit" mem_top > fit.log 2>&1 || { echo "FIT FAILED $MODE $D"; continue; }
    M10K=$(grep -m1 'Total RAM Blocks' mem_top.fit.summary | sed 's/^[^:]*: *//')
    ALM=$(grep -m1 'Logic utilization (in ALMs)' mem_top.fit.summary | sed 's/^[^:]*: *//')
    cat > run.tcl <<EOF
read_verilog mem.v
synth_design -top mem_top -part xc7z010clg400-1 -mode out_of_context -generic MODE=$MODE -generic D=$D
report_utilization -file util.txt
EOF
    cmd //c "$(W_ "$VIVADO")" -mode batch -nojournal -log vivado.log -source run.tcl > run.log 2>&1 || echo "VIVADO FAILED $MODE $D"
    BRAM=$(grep -m1 -E '^\| Block RAM Tile' util.txt | awk -F'|' '{print $3}' | tr -d ' ')
    LUT=$(grep -m1 -E '^\| Slice LUTs' util.txt | awk -F'|' '{print $3}' | tr -d ' ')
    echo "MODE $MODE depth $D x 32: quartus M10K=$M10K ALMs=$ALM | vivado BRAM tiles=$BRAM LUTs=$LUT" | tee -a "$OUTF"
  done
done
