// The array memory of the array unit (docs/design/array-unit.md, section 7,
// step 3): how many RAM blocks each organisation takes. A study, not SAPHO
// code. Read registered, as mem_data in SAPHO/processor.v.
//
//   MODE 0  simple dual port, one read and one write port: today's mem_data
//   MODE 1  true dual port, two ports that each read or write
// (two banks of half the depth were left out by Luciano, 2026-10-24)

module mem_sdp #(parameter W = 32, D = 1024) (
	input                      clk, we,
	input  [$clog2(D)-1:0]     wa, ra,
	input  [W-1:0]             wd,
	output reg [W-1:0]         rd
);
reg [W-1:0] mem [0:D-1];
always @ (posedge clk) begin
	if (we) mem[wa] <= wd;
	rd <= mem[ra];
end
endmodule

// two processes on one array, each port returning the word it writes: the
// Intel template for true dual port. Reading the old word on a write port
// (qa <= mem[aa] alone) is refused by Quartus, which then builds the array
// out of registers (276009, unsupported read-during-write)
module mem_tdp #(parameter W = 32, D = 1024) (
	input                      clk,
	input                      wea, web,
	input  [$clog2(D)-1:0]     aa, ab,
	input  [W-1:0]             da, db,
	output reg [W-1:0]         qa, qb
);
reg [W-1:0] mem [0:D-1];
always @ (posedge clk) begin
	if (wea) begin mem[aa] <= da; qa <= da; end
	else qa <= mem[aa];
end
always @ (posedge clk) begin
	if (web) begin mem[ab] <= db; qb <= db; end
	else qb <= mem[ab];
end
endmodule

module mem_top #(parameter MODE = 0, W = 32, D = 1024) (
	input                      clk,
	input                      wea, web,
	input  [$clog2(D)-1:0]     aa, ab,
	input  [W-1:0]             da, db,
	output [W-1:0]             qa, qb
);
generate
	if (MODE == 0) begin : sdp
		mem_sdp #(W, D) m (clk, wea, aa, ab, da, qa);
		assign qb = 0;
	end else begin : tdp
		mem_tdp #(W, D) m (clk, wea, web, aa, ab, da, db, qa, qb);
	end
endgenerate
endmodule
