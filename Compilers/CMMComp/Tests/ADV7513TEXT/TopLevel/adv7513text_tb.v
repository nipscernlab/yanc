`timescale 1ns/1ps
// Regress project pass ADV7513TEXT: SAPHO/myADV7513Text.v on a whole processor, the
// demonstration of the DE10-Nano (Boards/DE10-Nano/hdmi_text/hdmi_demo.cpp,
// a C++ SAPHO of 8 bits) on one 25 MHz clock, as its top wires it, with the
// I2C left at its own wait (262 ms, longer than this run, which checks that
// the bus stays quiet meanwhile). The bench reads the screen memory through
// its top register and checks the test card the program writes, up to the
// counter's first value, 000, and the cursor left after it. The counter's
// next value comes after the program's wait, 60 ms of simulation (about 3
// minutes): checked by hand, not here (docs/design/adv7513-text.md). The block
// alone, the I2C writes and their timing: Scripts/hw/tb_adv7513text.sh.
module adv7513text_tb;

reg clk = 0, rst = 1;
always #20 clk = ~clk;                      // 25 MHz

wire       tm, va, hs, vs, de, px;
wire [7:0] dd, o;
wire       scl, sda;
pullup (scl);
pullup (sda);

hdmi_demo P (.clk(clk), .rst(rst), .out(o), .toma_1(tm), .valeu_1(va), .toma_dado(dd));
myADV7513Text #(.NUBITS(8), .LINK_SYNC(1)) H (.clk(clk), .rst(rst),
	.toma(tm), .dado(dd), .valeu(va), .hs(hs), .vs(vs), .ativo(de), .pixel(px),
	.scl(scl), .sda(sda));

integer nstop = 0;                          // STOPs on the bus
always @ (posedge sda) if (scl === 1'b1) nstop = nstop + 1;

function [6:0] at; input integer r, c; integer m; begin
	m = r + H.tela.top; if (m >= 30) m = m - 30; at = H.tela.smem[m*80 + c];
end endfunction

integer e = 0;
task want; input integer r, c; input [8*80-1:0] s; input integer n; integer k; reg [7:0] x; begin
	for (k = 0; k < n; k = k + 1) begin
		x = s[8*(n-1-k) +: 8];
		if (at(r, c + k) !== x[6:0]) begin
			e = e + 1;
			if (e < 12) $display("  screen (%0d,%0d): '%c' (%0d), want '%c'", r, c + k, at(r, c + k), at(r, c + k), x);
		end
	end
end endtask

integer k, t;
initial begin
	#200 rst = 0;
	for (t = 0; t < 400 && !(H.tela.crow === 5'd7 && H.tela.ccol === 7'd13); t = t + 1) #10000;
	if (t == 400) begin e = e + 1; $display("  the counter not written in 4 ms"); end
	#1000;                                   // the screen memory is written a cycle after the cursor moves
	want(0, 0, "SAPHO + myADV7513Text na DE10-Nano: 80 x 30 caracteres, 640 x 480 a 60 Hz", 73);
	for (k = 0; k < 95; k = k + 1) if (at(2 + k / 80, k % 80) !== 32 + k) begin
		e = e + 1; if (e < 12) $display("  screen (%0d,%0d): %0d, want %0d", 2 + k / 80, k % 80, at(2 + k / 80, k % 80), 32 + k);
	end
	for (k = 0; k < 80; k = k + 1) if (at(5, k) !== "=") begin
		e = e + 1; if (e < 12) $display("  screen (5,%0d): %0d, want '='", k, at(5, k));
	end
	want(7, 0, "contador: 000", 13);
	want(29, 0, "linha 30 de 30", 14);
	if (H.tela.top !== 0) begin e = e + 1; $display("  the screen scrolled (top %0d)", H.tela.top); end
	if (nstop !== 0) begin e = e + 1; $display("  %0d I2C writes before the wait ended", nstop); end
	if (e == 0) $display("ADV7513TEXT hdmi_demo (C++, 8 bits): ok");
	else        $display("ADV7513TEXT hdmi_demo (C++, 8 bits): FAIL, %0d", e);
	$finish;
end

endmodule
