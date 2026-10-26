// Clocks of a stretch of code: the cycle the fetch reaches address A (its
// first instruction) and the cycle it reaches B (the instruction after it).
module region_probe;
parameter A = 0, B = 0;
`define C pptr_tb.proc.p_pptr.core
integer t = 0, ta = -1;
always @ (posedge `C.clk) begin
	t = t + 1;
	if (`C.instr_addr == A) ta = t;
	if (`C.instr_addr == B && ta >= 0) begin $display("REGION %0d", t - ta); ta = -1; end
end
endmodule
