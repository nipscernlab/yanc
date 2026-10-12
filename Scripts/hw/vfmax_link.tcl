# Real Fmax of the LINKALL pair through Vivado, out of context: two clocks
# declared asynchronous (every crossing is a toma/cade bit into a synchronizer
# or a word the protocol holds still), the worst setup slack of each. Called by
# vfmax_link.sh with: <dir with linkall_top.v, sapho_all_a.v, sapho_all_b.v>
# <SAPHO dir> <part>.
set dir  [lindex $argv 0]
set hdl  [lindex $argv 1]
set part [lindex $argv 2]

foreach f {linkall_top.v sapho_all_a.v sapho_all_b.v} { read_verilog [file join $dir $f] }
foreach f {processor.v core.v ula.v instr_dec.v addr_dec.v} { read_verilog [file join $hdl $f] }
synth_design -top linkall_top -part $part -mode out_of_context
create_clock -period 10.000 -name clk_a [get_ports clk_a]
create_clock -period 10.000 -name clk_b [get_ports clk_b]
set_clock_groups -asynchronous -group [get_clocks clk_a] -group [get_clocks clk_b]
opt_design
place_design
route_design

set res ""
foreach c {clk_a clk_b} {
    set wns [get_property SLACK [get_timing_paths -max_paths 1 -setup -group $c]]
    append res [format " %s=%.2fMHz(WNS=%.3f)" $c [expr {1000.0 / (10.0 - $wns)}] $wns]
}
report_utilization -file [file join $dir utilization.txt]
report_timing -max_paths 5 -setup -group clk_a -file [file join $dir worst_a.txt]
report_timing -max_paths 5 -setup -group clk_b -file [file join $dir worst_b.txt]
report_clock_interaction -file [file join $dir clock_interaction.txt]
set fh [open [file join $dir result.txt] w]
puts $fh "linkall (sapho_all_a -> sapho_all_b, two clocks) $part:$res"
close $fh
