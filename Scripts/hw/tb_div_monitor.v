`timescale 1ns/1ps
// The simulation's rounding-error monitors show a division's real error
// (#183). The processor is div_monitor.asm, built by
// Scripts/hw/tb_div_monitor.sh: 100 / -7, 100 % -7 and 10.0 / 3.0. At the
// word that reads each result (QUO, REM, F_QUO) delta_int and delta_float
// must be the error of that division: -100/7 + 14 for the quotient, which the
// integer division truncates, 0 for the remainder, and below 2^-20 for the
// float. Before #183 they compared the result with the inputs of the reading
// word (mem[0] and the accumulator) and showed whatever came out.
module tb;

reg clk = 0, rst = 1;
always #5 clk = ~clk;

wire signed [31:0] in;
wire signed [31:0] out;
wire        [0:0]  req_in, out_en;
div_monitor dut (clk, rst, in, out, req_in, out_en);

wire [5:0] op  = dut.p_div_monitor.core.ula.op;
wire [5:0] iop = dut.p_div_monitor.core.ula.iop;
wire [5:0] fop = dut.p_div_monitor.core.ula.fop;
real di, df;
always @ (*) di = dut.p_div_monitor.core.ula.delta_int;
always @ (*) df = dut.p_div_monitor.core.ula.delta_float;

// port 0 gives 100, then -7: the word changes after the edge that reads it
// (nonblocking), as a FIFO's would
integer n_in = 0;
always @ (posedge clk) if (!rst && req_in) n_in <= n_in + 1;
assign in = (n_in == 0) ? 32'sd100 : -32'sd7;

integer errs = 0, seen_q = 0, seen_r = 0, seen_f = 0, outs = 0;
real want_q;
initial want_q = -100.0 / 7.0 + 14.0;       // 100 / -7 = -14.2857..., the quotient -14

always @ (negedge clk) if (!rst) begin
	if (op == 6'd6 && iop == 6'd6) begin
		seen_q = seen_q + 1;
		if (di - want_q > 1e-9 || want_q - di > 1e-9) begin errs = errs + 1; $display("FAIL: QUO's delta_int is %g, not %g", di, want_q); end
	end
	if (op == 6'd8 && (iop == 6'd8 || iop == 6'd6)) begin
		seen_r = seen_r + 1;
		if (di != 0.0) begin errs = errs + 1; $display("FAIL: REM's delta_int is %g, not 0", di); end
	end
	if (op == 6'd7 && fop == 6'd7) begin
		seen_f = seen_f + 1;
		if (df > 1.0/1048576.0 || df < -1.0/1048576.0) begin errs = errs + 1; $display("FAIL: F_QUO's delta_float is %g", df); end
	end
	// on the cycles that start a division the monitors show 0
	if ((op == 6'd6 || op == 6'd8) && !(iop == 6'd6 || iop == 6'd8) && di != 0.0) begin
		errs = errs + 1; $display("FAIL: delta_int is %g on a cycle that starts a division", di);
	end
	if (op == 6'd7 && fop != 6'd7 && df != 0.0) begin
		errs = errs + 1; $display("FAIL: delta_float is %g on a cycle that starts a division", df);
	end
end
always @ (posedge clk) if (!rst && out_en) outs = outs + 1;

initial begin
	repeat (2) @(negedge clk); rst = 0;
	wait (outs == 3);
	repeat (5) @(negedge clk);
	if (seen_q != 1 || seen_r != 1 || seen_f != 1) begin
		errs = errs + 1; $display("FAIL: the reading words were seen %0d, %0d and %0d times, not once each", seen_q, seen_r, seen_f);
	end
	if (errs == 0) $display("tb_div_monitor: ok (QUO %g, REM 0, F_QUO within 2^-20)", want_q);
	$finish;
end

initial begin #20000; $display("FAIL: the program did not put out its three values"); $finish; end

endmodule
