// For each DOT or DOTB: the clocks its opcode stays decoded, and the clocks from its
// last one to the next OUT decoded (1 means no fetch clock lost).
module dot_probe;
`define C dot_c_tb.proc.p_dot_c.core
integer run = 0, gap = -1;
always @ (posedge `C.clk) begin
	if (`C.id_opcode == 108 || `C.id_opcode == 109) run = run + 1;
	else if (run > 0) begin gap = 1; $write("DOT decoded %0d clocks", run); run = 0; end
	else if (gap > 0) gap = gap + 1;
	if (`C.id_opcode == 15 && gap > 0) begin $display(", OUT %0d clock(s) later", gap); gap = -1; end
end
endmodule
