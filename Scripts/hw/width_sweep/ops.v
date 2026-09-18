// The device under test of the width sweep: every ALU-style operator as a
// CONTINUOUS assign in a pure signed context (the way ula.v writes them), plus
// the unsigned forms. Nothing else: the question is the simulator, not YANC.
module ops
#(
	parameter W = 64
)(
	 input  signed [W-1:0] a, b,
	output signed [W-1:0] add, sub, mul, div, mod, shl, shr, sra,
	output        [W-1:0] udiv, umod,
	output                lt, gt, ult, eq
);
assign add  = a + b;
assign sub  = a - b;
assign mul  = a * b;
assign div  = a / b;
assign mod  = a % b;
assign shl  = a <<  b;                       // the amount is always unsigned in Verilog
assign shr  = a >>  b;                       // logical, whatever the signedness of a
assign sra  = a >>> b;                       // arithmetic: a is signed
assign udiv = $unsigned(a) / $unsigned(b);
assign umod = $unsigned(a) % $unsigned(b);
assign lt   = a < b;
assign gt   = a > b;
assign ult  = $unsigned(a) < $unsigned(b);
assign eq   = a == b;
endmodule
