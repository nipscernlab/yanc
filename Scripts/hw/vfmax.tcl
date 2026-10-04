# Real Fmax of a generated processor through Vivado, out of context (no pins):
# synthesis, placement, routing, then the worst setup slack against a 10 ns
# clock. Called by vfmax.sh with: <proc> <top.v> <SAPHO dir> <part> <out dir>.
set proc [lindex $argv 0]
set top  [lindex $argv 1]
set hdl  [lindex $argv 2]
set part [lindex $argv 3]
set out  [lindex $argv 4]

read_verilog $top
foreach f {processor.v core.v ula.v instr_dec.v addr_dec.v} { read_verilog [file join $hdl $f] }
synth_design -top $proc -part $part -mode out_of_context
create_clock -period 10.000 -name clk [get_ports clk]
opt_design
place_design
route_design

set wns [get_property SLACK [get_timing_paths -max_paths 1 -setup]]
set fmax [expr {1000.0 / (10.0 - $wns)}]
report_utilization -file [file join $out utilization.txt]
report_timing -max_paths 10 -setup -file [file join $out worst_paths.txt]
report_timing -max_paths 1 -setup -file [file join $out worst_path_full.txt]
set fh [open [file join $out result.txt] w]
puts $fh [format "%s %s: Fmax=%.2fMHz WNS=%.3f" $proc $part $fmax $wns]
close $fh
