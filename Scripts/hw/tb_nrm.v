`timescale 1ns/1ps
// ula_nrm against the division it replaced (#160): every 8-bit input, for
// NUGAIN 1 to 64, against Verilog's signed /, which truncates toward zero as
// C does. tb_ula checks NRM at its NUGAIN of 128 only.
module tb;
reg signed [7:0] x; integer v, bad = 0;
wire signed [7:0] o1, o2, o4, o8, o16, o32, o64;
ula_nrm #(8, 1)  n1 (x, o1);  ula_nrm #(8, 2)  n2 (x, o2);  ula_nrm #(8, 4)  n4 (x, o4);
ula_nrm #(8, 8)  n8 (x, o8);  ula_nrm #(8, 16) n16(x, o16); ula_nrm #(8, 32) n32(x, o32);
ula_nrm #(8, 64) n64(x, o64);
task chk(input signed [7:0] got, input integer g);
	reg signed [7:0] want;
	begin want = x / g; if (got !== want) begin bad = bad + 1; if (bad < 5) $display("FAIL %0d / %0d: got %0d want %0d", x, g, got, want); end end
endtask
initial begin
	for (v = 0; v < 256; v = v + 1) begin
		x = v; #1;
		chk(o1,1); chk(o2,2); chk(o4,4); chk(o8,8); chk(o16,16); chk(o32,32); chk(o64,64);
	end
	if (bad == 0) $display("tb_nrm: ok (256 inputs x NUGAIN 1 to 64)");
	else          $display("FAIL tb_nrm: %0d mismatches over 256 inputs x 7 gains", bad);
	$finish;
end
endmodule
