// ----------------------------------------------------------------------------
// a FIFO between two clock domains (in place of the Quartus dcfifo IP) -------
// ----------------------------------------------------------------------------
// The writer side runs on wrclk, the reader side on rdclk; nothing else is
// shared. Each side keeps its own pointer and hands the other side a copy in
// Gray code through two flip-flops (one bit changes per step, so the copy is
// either the old value or the new one, never a mix). wrfull is computed on
// the writer's clock, rdempty on the reader's, both against a copy that may
// be 2-3 cycles old: they can say "full"/"empty" a little longer than true,
// never shorter, so no word is overwritten or read twice.
//
// Same behaviour as myFIFO.v inside each domain: the word on `data` is
// stored in the cycle `wrreq` is high (a SAPHO's `out` bus holds it in its
// `out_en` cycle only), a write into a full FIFO is refused, and the read is
// show-ahead (`q` is the oldest word; `rdreq` moves to the next), read
// combinationally like myFIFO. A word written becomes visible to the reader
// 2-3 rdclk cycles later.
//
// LENGTH: a power of two, at least 4; the FIFO holds LENGTH words (one more
// pointer bit tells full from empty). aclr: asynchronous clear of both
// sides; each side leaves it on its own clock (two flip-flops), so assert it
// for a few cycles of the slower clock. docs/design/myfifo.md; checked by
// Scripts/hw/tb_dcfifo.v and the regress FIFO pass.

module myDCFIFO
#(
	parameter WORD   =  16,
	parameter LENGTH = 128,
	parameter ALMOST =   2
)
(
	input                         aclr          ,

	input                         wrclk         ,
	input      [WORD        -1:0] data          ,
	input                         wrreq         ,
	output                        wrfull        ,
	output     [$clog2(LENGTH):0] wrusedw       ,   // words in, as the writer sees them

	input                         rdclk         ,
	input                         rdreq         ,
	output                        rdempty       ,
	output                        rdalmost_empty,   // fewer than ALMOST words
	output     [WORD        -1:0] q             ,
	output     [$clog2(LENGTH):0] rdusedw           // words in, as the reader sees them
);

localparam A = $clog2(LENGTH);                    // address bits; pointers have A+1

function [A:0] gray2bin(input [A:0] g);
	integer i;
	begin
		gray2bin[A] = g[A];
		for (i = A-1; i >= 0; i = i-1) gray2bin[i] = gray2bin[i+1] ^ g[i];
	end
endfunction

reg [WORD-1:0] mem [0:LENGTH-1];

reg [A:0] wbin = 0, wgray = 0;   // the writer's pointer, binary and Gray (wrclk)
reg [A:0] rbin = 0, rgray = 0;   // the reader's pointer, binary and Gray (rdclk)
reg [A:0] rg1 = 0, rg2 = 0;      // rgray on wrclk, through two flip-flops
reg [A:0] wg1 = 0, wg2 = 0;      // wgray on rdclk, through two flip-flops

// each side leaves the clear on its own clock --------------------------------

reg [1:0] wrst_s = 2'b11, rrst_s = 2'b11;
always @ (posedge wrclk or posedge aclr) if (aclr) wrst_s <= 2'b11; else wrst_s <= {wrst_s[0], 1'b0};
always @ (posedge rdclk or posedge aclr) if (aclr) rrst_s <= 2'b11; else rrst_s <= {rrst_s[0], 1'b0};
wire wrst = wrst_s[1];
wire rrst = rrst_s[1];

// writer side (wrclk) ---------------------------------------------------------

wire       wput  = wrreq & ~wrfull;
wire [A:0] wbin_n = wbin + wput;

always @ (posedge wrclk) if (wput) mem[wbin[A-1:0]] <= data;

always @ (posedge wrclk) begin
	if (wrst) begin
		wbin <= 0; wgray <= 0; rg1 <= 0; rg2 <= 0;
	end else begin
		wbin  <= wbin_n;
		wgray <= (wbin_n >> 1) ^ wbin_n;
		rg1   <= rgray;
		rg2   <= rg1;
	end
end

// full: the pointers differ by LENGTH -- in Gray, the two top bits inverted
assign wrfull  = (wgray == (rg2 ^ {2'b11, {(A-1){1'b0}}}));
assign wrusedw = wbin - gray2bin(rg2);

// reader side (rdclk) ---------------------------------------------------------

wire       rget  = rdreq & ~rdempty;
wire [A:0] rbin_n = rbin + rget;

always @ (posedge rdclk) begin
	if (rrst) begin
		rbin <= 0; rgray <= 0; wg1 <= 0; wg2 <= 0;
	end else begin
		rbin  <= rbin_n;
		rgray <= (rbin_n >> 1) ^ rbin_n;
		wg1   <= wgray;
		wg2   <= wg1;
	end
end

assign rdempty        = (rgray == wg2);
assign rdusedw        = gray2bin(wg2) - rbin;
assign rdalmost_empty = (rdusedw < ALMOST);
assign q              = mem[rbin[A-1:0]];

endmodule
