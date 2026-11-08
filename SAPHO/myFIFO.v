// ----------------------------------------------------------------------------
// module to replace the use of Quartus FIFO IP -------------------------------
// ----------------------------------------------------------------------------
// The word on `data` is stored in the cycle `wrreq` is high (a SAPHO's `out`
// bus holds it in its `out_en` cycle only); `full` and `empty` come from the
// pointers alone, a write into a full FIFO is refused, and a word written
// can be read from the next cycle on (show-ahead: `q` is the oldest word).
// Holds LENGTH-1 words. LENGTH must be a power of two, at least 2: the
// pointers have $clog2(LENGTH) bits and wrap only at the power of two, so a
// LENGTH of 100 accepted 126 words and lost 26 of them, with full, empty and
// usedw all coherent (#180). Any other LENGTH stops the elaboration below.
// docs/design/myfifo.md; checked by Scripts/hw/tb_fifo.v.

module myFIFO
#(
	parameter WORD   =  16,
	parameter LENGTH = 128,
	parameter ALMOST =   2
)
(
	input                           clk  ,
	input      [WORD          -1:0] data ,
	input                           rdreq,
	input                           sclr ,
	input                           wrreq,
	output                          almost_empty,
	output                          empty,
	output                          full ,
	output reg [WORD          -1:0] q    ,
	output     [$clog2(LENGTH)-1:0] usedw
);

// a LENGTH outside the rule names this module, which does not exist: every
// tool stops there with the rule in its message (Verilog-2001 has no $error)
generate if (LENGTH < 2 || LENGTH != (1 << $clog2(LENGTH))) begin : length_check
	myFIFO_LENGTH_must_be_a_power_of_two_at_least_2 length_rule ();
end endgenerate

reg [WORD-1:0]  mem  [LENGTH-1:0];
reg [$clog2(LENGTH)-1:0] addr_w=0;
reg [$clog2(LENGTH)-1:0] addr_r=0;

always @ (posedge clk) begin
	if (sclr)
		addr_w <= 0;
	else if (wrreq & !full) begin
		mem[addr_w] <= data;
		addr_w <= addr_w+1'b1;
	end
end

always @ (posedge clk) begin
	if (sclr) begin
		addr_r <= 0;
	end else if (rdreq & !empty) begin
		addr_r <= addr_r+1'b1;
	end
end

always @ (*) q = mem[addr_r];

assign full         =  (addr_w+1'b1 == addr_r);
assign empty        =  (addr_w      == addr_r);
assign usedw        =  addr_w - addr_r;
assign almost_empty = ~(usedw       >= ALMOST);

endmodule
