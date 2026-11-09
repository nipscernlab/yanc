`timescale 1ns/1ps
// A #PRACA pulse empties both stacks (#188). The processor is
// praca_restart.asm, built by Scripts/hw/tb_praca_restart.sh: a loop that
// calls a -> b -> c, with both stacks 3 deep (not a power of two) and full in
// c. The testbench pulls itr up while the program is in c, six times (one of
// them three clocks long), and checks that after each pulse both stacks are
// empty, that the fetch address never goes X, that no stack ever overflows
// and that the program keeps going: each value on port 0 is the previous one
// plus 3 (the pass the pulse left adds nothing). Before #188 the pointers
// kept the words of the routines left, climbed past 3 and the return address
// went X after a few pulses.
//
// The second processor, praca_tom.asm, waits in TOM 0 (nobody takes the
// word); a pulse during that wait must print the simulation's warning, which
// the script checks in the output, and the TOM after the restart hands over
// the word offered before the pulse, not its own (the handshake is not
// undone, docs/directives.md).
module tb;

reg clk = 0, rst = 1, itr = 0;
always #5 clk = ~clk;

wire signed [7:0] out;
wire        [0:0] out_en;
praca_restart dut (clk, rst, out, out_en, itr);

wire [7:0] ia   = dut.p_praca_restart.core.instr_addr;
wire [2:0] scnt = dut.p_praca_restart.core.instr_fetch.isp_blk.isp.fl_cnt;
wire [2:0] dcnt = dut.p_praca_restart.core.sp.fl_cnt;
wire       sovf = dut.p_praca_restart.core.instr_fetch.isp_blk.isp.fl_overflow;
wire       dovf = dut.p_praca_restart.core.sp.fl_overflow;
wire [7:0] SD   = dut.p_praca_restart.core.SDEPTH;
wire [7:0] DD   = dut.p_praca_restart.core.DDEPTH;

// the TOM processor: its partner's C stays 0 until the end, so the first TOM
// waits
reg  itr2 = 0, valeu2 = 0;
wire signed [7:0] out2, dado2;
wire        toma2;
praca_tom dut2 (.clk(clk), .rst(rst), .out(out2), .itr(itr2), .toma_0(toma2), .valeu_0(valeu2), .toma_dado(dado2));

integer errs = 0, outs = 0, p, k;
reg signed [7:0] last = 0;
always @ (posedge clk) if (!rst && out_en) begin
	if (outs > 0 && out != last + 8'sd3) begin
		errs = errs + 1; $display("FAIL: port 0 got %0d after %0d, not %0d", out, last, last + 8'sd3);
	end
	last = out; outs = outs + 1;
end
always @ (negedge clk) if (!rst && ^ia === 1'bx) begin
	errs = errs + 1; $display("FAIL: the fetch address is X at %0t", $time); $finish;
end

// wait until both stacks are full (the program is in c, after its pushes)
task wait_deep;
	begin
		k = 0;
		@(negedge clk);
		while (!(scnt == 3 && dcnt == 3) && k < 400) begin @(negedge clk); k = k + 1; end
		if (k >= 400) begin errs = errs + 1; $display("FAIL: both stacks never reached 3 words"); end
	end
endtask

initial begin
	repeat (2) @(negedge clk); rst = 0;
	if (SD != 3 || DD != 3) begin errs = errs + 1; $display("FAIL: SDEPTH %0d and DDEPTH %0d, the test needs 3 and 3", SD, DD); end

	for (p = 0; p < 6; p = p + 1) begin
		wait_deep;
		itr = 1;
		if (p == 3) repeat (2) @(negedge clk);   // one pulse three clocks long
		@(negedge clk); itr = 0;
		if (scnt != 0 || dcnt != 0) begin errs = errs + 1; $display("FAIL: after pulse %0d the stacks hold %0d and %0d words", p + 1, scnt, dcnt); end
		k = outs;
		while (outs < k + 2) @(negedge clk);     // the program goes on: two more values
	end
	if (sovf || dovf) begin errs = errs + 1; $display("FAIL: a stack overflowed (call %0d, data %0d)", sovf, dovf); end

	// a pulse while praca_tom waits in TOM 0 offering n = 1: the warning is
	// printed (the script looks for it). After the restart the program offers
	// n = 2, but its TOM finds the wait of the first one pending: when the
	// partner takes (C to 1), it gets 1, and the next offer is 3, so 2 is
	// never offered.
	repeat (20) @(negedge clk);
	if (toma2 !== 1'b1 || dado2 !== 8'sd1) begin errs = errs + 1; $display("FAIL: praca_tom offers T %b, word %0d, not 1 and 1", toma2, dado2); end
	itr2 = 1; @(negedge clk); itr2 = 0;
	repeat (20) @(negedge clk);
	if (toma2 !== 1'b1 || dado2 !== 8'sd1) begin errs = errs + 1; $display("FAIL: after the pulse praca_tom offers T %b, word %0d, not 1 and the old 1", toma2, dado2); end
	valeu2 = 1'b1;                                 // the partner takes
	repeat (20) @(negedge clk);
	if (toma2 !== 1'b0 || dado2 !== 8'sd3) begin errs = errs + 1; $display("FAIL: after the take praca_tom offers T %b, word %0d, not 0 and 3", toma2, dado2); end

	if (errs == 0) $display("tb_praca_restart: ok (6 pulses with both stacks full, depth 3)");
	$finish;
end

endmodule
