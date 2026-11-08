`timescale 1ns/1ps
// What the decoder drives out of the processor during a reset (#165). The
// instruction memory has no reset, so an instruction can sit in decode while
// rst is 1. mem_wr (the data memory's write enable) and req_in (the read of
// the input port's FIFO) leave the processor as they are: in reset they must
// be 0 whatever the opcode, and out of reset they must follow it. Opcodes
// from Compilers/common/isa.tsv: SET 5, SET_P 6, STI 7, INN 11, P_INN 13.
module tb;

reg        clk = 0, rst = 1;
reg  [6:0] opcode = 0;
wire       push, pop, mem_wr, req_in, out_en, ldi, sti, fft;
wire [5:0] ula_op;

instr_dec #(.SET(1), .SET_P(1), .STI(1), .INN(1), .P_INN(1))
    dut (clk, rst, opcode, push, pop, ula_op, mem_wr, req_in, out_en, ldi, sti, fft);

integer errs = 0;

task check(input [6:0] op, input r, input want_wr, input want_in);
	begin
		opcode = op; rst = r; #1;
		if (mem_wr !== want_wr || req_in !== want_in) begin
			errs = errs + 1;
			$display("FAIL: opcode %0d rst %0d: mem_wr %b req_in %b, expected %b %b",
			         op, r, mem_wr, req_in, want_wr, want_in);
		end
	end
endtask

initial begin
	// in reset: nothing leaves, whatever is in decode
	check(7'd5,  1, 0, 0);   // SET
	check(7'd6,  1, 0, 0);   // SET_P
	check(7'd7,  1, 0, 0);   // STI
	check(7'd11, 1, 0, 0);   // INN
	check(7'd13, 1, 0, 0);   // P_INN
	// out of reset: each follows its opcode
	check(7'd5,  0, 1, 0);
	check(7'd6,  0, 1, 0);
	check(7'd7,  0, 1, 0);
	check(7'd11, 0, 0, 1);
	check(7'd13, 0, 0, 1);
	check(7'd0,  0, 0, 0);   // NOP
	if (errs == 0) $display("tb_dec_reset: ok (5 opcodes in and out of reset)");
	$finish;
end

endmodule
