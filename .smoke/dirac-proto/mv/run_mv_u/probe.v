// Clocks between turns of a loop: one line "TURN <clocks>" each time an OUT
// follows a stretch of at least 60 clocks without one (the first OUT of a turn).
module out_probe;
`define C mv_tb.proc.p_mv.core
integer t = 0, last = -1, gap = 0;
always @ (posedge `C.clk) begin
	t = t + 1;
	if (`C.id_opcode == 15) begin
		if (gap >= 60) begin
			if (last >= 0) $display("TURN %0d", t - last);
			last = t;
		end
		gap = 0;
	end else gap = gap + 1;
end
endmodule
