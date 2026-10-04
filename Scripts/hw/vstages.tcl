# The worst path of each #PIPELN stage of a staged operator, through Vivado
# (out of context, as vfmax.tcl): into each cut register, and from the last
# one onward. Args: <proc> <top.v> <SAPHO dir> <part> <out dir> <operator>
# e.g. operator my_fdiv. Writes stages.txt in <out dir>.
set proc [lindex $argv 0]
set top  [lindex $argv 1]
set hdl  [lindex $argv 2]
set part [lindex $argv 3]
set out  [lindex $argv 4]
set op   [lindex $argv 5]
set_param tcl.collectionResultDisplayLimit 0

read_verilog $top
foreach f {processor.v core.v ula.v instr_dec.v addr_dec.v} { read_verilog [file join $hdl $f] }
synth_design -top $proc -part $part -mode out_of_context
create_clock -period 10.000 -name clk [get_ports clk]
opt_design
place_design
route_design
write_checkpoint -force [file join $out routed.dcp]

set fh [open [file join $out stages.txt] w]
foreach c [lsort -unique [regexp -all -inline {cut\[\d+\]} [get_cells -hier -filter "NAME =~ *${op}*cut*"]]] {
	set p [get_timing_paths -max_paths 1 -setup -to [get_cells -hier -filter "NAME =~ *${op}*${c}*"]]
	puts $fh [format "into %-8s delay %.3f ns  levels %s  from %s" $c [get_property DATAPATH_DELAY $p] [get_property LOGIC_LEVELS $p] [get_property STARTPOINT_PIN $p]]
}
set last [lindex [lsort -dictionary [regexp -all -inline {cut\[\d+\]} [get_cells -hier -filter "NAME =~ *${op}*cut*"]]] end]
set p [get_timing_paths -max_paths 1 -setup -from [get_cells -hier -filter "NAME =~ *${op}*${last}*"]]
puts $fh [format "from %-8s delay %.3f ns  levels %s  to %s" $last [get_property DATAPATH_DELAY $p] [get_property LOGIC_LEVELS $p] [get_property ENDPOINT_PIN $p]]
set p [get_timing_paths -max_paths 1 -setup]
puts $fh [format "worst overall     delay %.3f ns  from %s to %s" [get_property DATAPATH_DELAY $p] [get_property STARTPOINT_PIN $p] [get_property ENDPOINT_PIN $p]]
close $fh
