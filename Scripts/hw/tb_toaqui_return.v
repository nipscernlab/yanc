`timescale 1ns/1ps
// cheguei rises at a #TOAQUI point the program reaches through a RET (#189).
// The processor is toaqui_return.asm, built by Scripts/hw/tb_toaqui_return.sh:
// a loop that calls f, with the point right after the CAL. Each pass of the
// loop puts 2 on port 0 (in f) and then returns to the point, so cheguei must
// rise once per value put out. Before #189 cheguei compared the address
// prefetch chose, which on a RET is not the return address, and never rose.
module tb;

reg clk = 0, rst = 1;
always #5 clk = ~clk;

wire signed [7:0] out;
wire        [0:0] out_en;
wire              cheguei;

toaqui_return dut (clk, rst, out, out_en, cheguei);

// the address fetched, and the #TOAQUI address asmcomp gave
wire [7:0] addr    = dut.p_toaqui_return.core.instr_fetch.addr;
wire [7:0] TOAQUI  = dut.p_toaqui_return.core.TOAQUIADDR;

integer errs = 0, outs = 0, rises = 0, at = 0;
reg     chg_d = 0;
always @ (posedge clk) if (!rst) begin
	if (out_en) outs = outs + 1;
	if (cheguei && !chg_d) rises = rises + 1;
	chg_d <= cheguei;
	if (addr == TOAQUI) at = at + 1;
	if (cheguei !== (addr == TOAQUI)) begin
		errs = errs + 1; $display("FAIL at %0t: cheguei %b while the address fetched is %0d (#TOAQUI at %0d)", $time, cheguei, addr, TOAQUI);
	end
end

initial begin
	repeat (2) @(negedge clk); rst = 0;
	wait (outs == 10);
	repeat (20) @(negedge clk);
	if (at == 0) begin errs = errs + 1; $display("FAIL: the program never fetched the #TOAQUI point"); end
	if (rises < outs - 1 || rises > outs) begin
		errs = errs + 1; $display("FAIL: cheguei rose %0d times for %0d returns", rises, outs);
	end
	if (errs == 0) $display("tb_toaqui_return: ok (cheguei rose %0d times, once per return to the point)", rises);
	$finish;
end

// a program that stops putting out ends here
initial begin #20000; $display("FAIL: no 10 values on port 0"); $finish; end

endmodule
