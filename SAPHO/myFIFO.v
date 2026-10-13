// ----------------------------------------------------------------------------
// module to replace the use of Quartus FIFO IP -------------------------------
// ----------------------------------------------------------------------------
// The word on `data` is stored in the cycle `wrreq` is high (a SAPHO's `out`
// bus holds it in its `out_en` cycle only); `full` and `empty` come from the
// pointers alone, a write into a full FIFO is refused, and a word written
// can be read from the next cycle on (show-ahead: `q` is the oldest word).
// Holds LENGTH-1 words. docs/design/myfifo.md; checked by Scripts/hw/tb_fifo.v.

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
