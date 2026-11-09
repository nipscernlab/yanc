`timescale 1ns/1ps
// A RET in decode must not win over a reset or an interrupt (#164, #166).
// The processor is ret_priority.asm, built by Scripts/hw/tb_ret_priority.sh:
// a loop that calls f (f writes 2 and returns) and an #ITRAD point that writes
// 9. With a RET in decode, the fetch address must be 0 during a reset and the
// #ITRAD address during an interrupt, not the return address; after the
// interrupt the program writes 9.
module tb;

reg clk = 0, rst = 1, itr = 0;
always #5 clk = ~clk;

wire signed [7:0] out;
wire        [0:0] out_en;

ret_priority dut (clk, rst, out, out_en, itr);

// the instruction in decode, and the address fetched next
wire [6:0] opcode = dut.p_ret_priority.core.instr_fetch.pf.opcode;
wire [7:0] addr   = dut.p_ret_priority.core.instr_fetch.addr;
localparam OP_RET = 20;
wire [7:0] ITRADD = dut.p_ret_priority.core.ITRADD;   // the #ITRAD address asmcomp gave

integer errs = 0, got9 = 0, k;
always @ (posedge clk) if (!rst && out_en && out == 9) got9 = 1;

task wait_ret;
	begin
		k = 0;
		@(negedge clk);
		while (opcode != OP_RET && k < 200) begin @(negedge clk); k = k + 1; end
		if (opcode != OP_RET) begin errs = errs + 1; $display("FAIL: no RET reached"); end
	end
endtask

initial begin
	repeat (2) @(negedge clk); rst = 0;

	// a reset with a RET in decode: the next fetch is address 0
	wait_ret;
	rst = 1; #1;
	if (addr !== 0) begin errs = errs + 1; $display("FAIL: reset with RET in decode fetches %0d, not 0", addr); end
	@(negedge clk); rst = 0;

	// an interrupt pulse with a RET in decode: the next fetch is #ITRAD
	wait_ret;
	itr = 1; #1;
	if (addr !== ITRADD) begin errs = errs + 1; $display("FAIL: interrupt with RET in decode fetches %0d, not %0d", addr, ITRADD); end
	@(negedge clk); itr = 0;
	repeat (20) @(negedge clk);
	if (!got9) begin errs = errs + 1; $display("FAIL: the program did not reach #ITRAD (no 9 written)"); end

	if (errs == 0) $display("tb_ret_priority: ok (reset and interrupt with a RET in decode)");
	$finish;
end

endmodule
