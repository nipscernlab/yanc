#!/bin/bash
# Area and Fmax of one MAC variant of mac.v, fitted alone (docs/design/array-unit.md,
# section 7, step 2): mac_fit, inputs and output registered, through Quartus
# (Cyclone V of the DE10-Nano, the pins virtual) or Vivado (the ZYBO's Z-7010,
# out of context). Besides the Fmax, the worst path into each group of
# registers: the product (stage 1), the prepared product (stage 2), the
# accumulator (the feedback) and the result (the rounding at the end). A block
# alone gives a bound; the critical path that counts is the whole processor's.
#
# Usage: [TOOL=q|v] bash Scripts/hw/mac_study/fit.sh VAR NB M E W [LGN] [FR]
#   VAR 0 = A (W the mantissa width), 1 = B (W the window), 2 = C (W ignored)
#   e.g. bash Scripts/hw/mac_study/fit.sh 2 32 23 8 0 10
#        TOOL=v bash Scripts/hw/mac_study/fit.sh 0 32 23 8 54
# PERIOD (default 10 ns, as fmax.sh) is the clock asked of the tools; 5 ns
# made a 16-bit fit take 27 min, 13 of them preparing the placement. Compare
# only fits made with the same PERIOD.
# Heavy (minutes of all cores each): one at a time, on mains power.
# Results appended to .smoke/hw/mac_study/fit.txt.
set -e
VAR=$1 NB=$2 M=$3 E=$4 W=$5 LGN=${6:-10} FR=${7:-2}
XB=$((E+4))
TOOL=${TOOL:-q}
ROOT=$(cd "$(dirname "$0")/../../.." && pwd)
S="$ROOT/Scripts/hw/mac_study"
NAME=$(echo "A B C" | cut -d' ' -f$((VAR+1)))
[ "$VAR" = 2 ] && TAGW=$((2*M + (1 << (E+1)) - 1 + LGN)) || TAGW=$W
DIR="$ROOT/.smoke/hw/mac_study/fit_${TOOL}_${NAME}${TAGW}_${NB}_FR${FR}"
rm -rf "$DIR"; mkdir -p "$DIR"; cd "$DIR"
cp "$S/mac.v" .
W_() { cygpath -m "$1" 2>/dev/null || echo "$1"; }

if [ "$TOOL" = q ]; then
    QUARTUS=${QUARTUS:-/c/intelFPGA_lite/24.1std/quartus/bin64}
    DEV=${DEV:-5CSEBA6U23I7}
    echo "create_clock -period ${PERIOD:-10}.000 -name clk [get_ports clk]" > clk.sdc
    echo "derive_clock_uncertainty" >> clk.sdc
    cat > proj.tcl <<EOF
project_new mac_fit -overwrite
set_global_assignment -name FAMILY "Cyclone V"
set_global_assignment -name DEVICE $DEV
set_global_assignment -name TOP_LEVEL_ENTITY mac_fit
set_global_assignment -name VERILOG_FILE mac.v
set_global_assignment -name SDC_FILE clk.sdc
set_global_assignment -name NUM_PARALLEL_PROCESSORS ALL
set_global_assignment -name OPTIMIZATION_MODE "HIGH PERFORMANCE EFFORT"
set_parameter -name VAR $VAR
set_parameter -name NB $NB
set_parameter -name M $M
set_parameter -name E $E
set_parameter -name W $W
set_parameter -name LGN $LGN
set_parameter -name XB $XB
set_parameter -name FR $FR
foreach p {clr en a[*] b[*] lsb[*] res[*]} { set_instance_assignment -name VIRTUAL_PIN ON -to \$p }
export_assignments
project_close
EOF
    "$QUARTUS/quartus_sh" -t proj.tcl > tcl.log 2>&1
    "$QUARTUS/quartus_map" mac_fit > map.log 2>&1 || { echo "MAP FAILED"; grep -m5 Error map.log; exit 1; }
    # MAPONLY=1: synthesis only (a minute or so, not the half hour of a fit):
    # its area estimate, no clock
    if [ -n "$MAPONLY" ]; then
        EST=$(grep -m1 'Estimate of Logic utilization (ALMs needed)' mac_fit.map.rpt | awk -F';' '{print $3}' | tr -d ' ')
        DSP=$(grep -m1 'Total DSP Blocks' mac_fit.map.rpt | awk -F';' '{print $3}' | tr -d ' ')
        REG=$(grep -m1 'Total registers' mac_fit.map.rpt | awk -F';' '{print $3}' | tr -d ' ')
        echo "quartus $DEV ${NAME}${TAGW} $NB/$M/$E FR$FR: synthesis estimate ALMs=$EST DSP=$DSP regs=$REG (MAPONLY)" | tee -a "$ROOT/.smoke/hw/mac_study/fit.txt"
        exit 0
    fi
    "$QUARTUS/quartus_fit" mac_fit > fit.log 2>&1 || { echo "FIT FAILED"; grep -m5 Error fit.log; exit 1; }
    "$QUARTUS/quartus_sta" mac_fit > sta.log 2>&1 || { echo "STA FAILED"; grep -m5 Error sta.log; exit 1; }
    cat > paths.tcl <<'EOF'
project_open mac_fit
create_timing_netlist
read_sdc clk.sdc
update_timing_netlist
foreach {g pat} {product *mac_prod:p|* prepared *|p2* feedback *|acc* result res*} {
    set r [get_registers $pat]
    if {[get_collection_size $r] == 0} { continue }
    set p [get_timing_paths -to $r -setup -npaths 1]
    foreach_in_collection q $p {
        puts [format "%-9s slack %7.3f ns  data delay %7.3f ns" $g [get_path_info $q -slack] [get_path_info $q -data_delay]]
    }
}
delete_timing_netlist
project_close
EOF
    "$QUARTUS/quartus_sta" -t paths.tcl > paths.log 2>&1 || true
    ALM=$(grep -m1 'Logic utilization (in ALMs)' mac_fit.fit.summary | sed 's/^[^:]*: *//')
    DSP=$(grep -m1 'Total DSP Blocks' mac_fit.fit.summary | sed 's/^[^:]*: *//')
    REG=$(grep -m1 'Total registers' mac_fit.fit.summary | sed 's/^[^:]*: *//')
    FMAX=$(grep -A6 -E 'Slow 1[12]00mV (85|100)C Model Fmax Summary' mac_fit.sta.rpt | grep -m1 'MHz' | awk -F';' '{print $2}' | tr -d ' ')
    LINE="quartus $DEV ${NAME}${TAGW} $NB/$M/$E FR$FR: Fmax=$FMAX ALMs=$ALM DSP=$DSP regs=$REG"
    PATHS_=$(grep -E '^(product|prepared|feedback|result)' paths.log || true)
else
    VIVADO=${VIVADO:-/c/AMDDesignTools/2025.2/Vivado/bin/vivado.bat}
    PART=${PART:-xc7z010clg400-1}
    cat > run.tcl <<EOF
read_verilog mac.v
synth_design -top mac_fit -part $PART -mode out_of_context -generic VAR=$VAR -generic NB=$NB -generic M=$M -generic E=$E -generic W=$W -generic LGN=$LGN -generic XB=$XB -generic FR=$FR
create_clock -period ${PERIOD:-10}.000 -name clk [get_ports clk]
opt_design
place_design
route_design
set wns [get_property SLACK [get_timing_paths -max_paths 1 -setup]]
report_utilization -file util.txt
set fh [open result.txt w]
puts \$fh [format "Fmax=%.2fMHz" [expr {1000.0 / (${PERIOD:-10}.0 - \$wns)}]]
foreach {g pat} {product *p/m_reg* prepared *p2_reg* feedback *acc_reg* result res_reg*} {
    set c [get_cells -hier -quiet -filter "NAME =~ \$pat && IS_SEQUENTIAL"]
    if {[llength \$c] == 0} { continue }
    set p [get_timing_paths -to \$c -setup -max_paths 1]
    puts \$fh [format "%-9s slack %7.3f ns  data delay %7.3f ns" \$g [get_property SLACK \$p] [get_property DATAPATH_DELAY \$p]]
}
close \$fh
EOF
    cmd //c "$(W_ "$VIVADO")" -mode batch -nojournal -log vivado.log -source run.tcl > run.log 2>&1 || { echo "VIVADO FAILED"; grep -m5 ERROR vivado.log; exit 1; }
    FMAX=$(head -1 result.txt)
    LUT=$(grep -m1 -E '^\| Slice LUTs' util.txt | awk -F'|' '{print $3}' | tr -d ' ')
    FF=$(grep -m1 -E '^\| Slice Registers' util.txt | awk -F'|' '{print $3}' | tr -d ' ')
    DSP=$(grep -m1 -E '^\| DSPs' util.txt | awk -F'|' '{print $3}' | tr -d ' ')
    LINE="vivado $PART ${NAME}${TAGW} $NB/$M/$E FR$FR: $FMAX LUTs=$LUT DSP=$DSP FFs=$FF"
    PATHS_=$(tail -n +2 result.txt)
fi
echo "$LINE" | tee -a "$ROOT/.smoke/hw/mac_study/fit.txt"
echo "$PATHS_" | sed 's/^/    /' | tee -a "$ROOT/.smoke/hw/mac_study/fit.txt"
