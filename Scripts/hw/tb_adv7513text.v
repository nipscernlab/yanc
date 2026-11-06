`timescale 1ns/1ps
// Testbench of SAPHO/myADV7513Text.v (docs/design/adv7513-text.md). Two things:
//   - the screen: a myVGAText of reference takes the same reset and the same
//     words through the toma link; every output of the block (valeu, hs, vs,
//     ativo, pixel) must equal the reference's in every cycle, the screen
//     memories must hold the same text at the end, and that text must be the
//     one written;
//   - the I2C: an ADV7513 played on the bus (it answers every byte with an
//     ACK) decodes each transaction and checks the eleven writes, in order,
//     as {0x72, register, value} each with a START and a STOP, and nothing
//     after them; the master's own SCL and SDA are timed against the fast mode
//     I2C limits (every edge of one at least 1.3 us from every edge of the
//     other and from its own last edge; an SDA change while SCL is high
//     outside START and STOP shows as a START or STOP out of place, which the
//     chip above reports), SDA released in each ACK slot, and the first
//     START no sooner than 2^WAIT_BITS lines after reset (200 ms or more at the
//     real WAIT_BITS of 13). Then a second reset, and the eleven writes again.
// Run: bash Scripts/hw/tb_adv7513text.sh
module tb;

parameter NUBITS    = 8;
parameter LINK_SYNC = 1;
parameter WAIT_BITS = 13;

reg clk = 0, rst = 1;
always #20 clk = ~clk;                       // 25 MHz

reg  t = 0;                                  // the SAPHO's T bit
reg  [NUBITS-1:0] w = 0;
wire valeu, hs, vs, ativo, pixel;
wire valeu_r, hs_r, vs_r, ativo_r, pixel_r;
wire scl, sda;
pullup (scl);
pullup (sda);

myADV7513Text #(.NUBITS(NUBITS), .LINK_SYNC(LINK_SYNC), .WAIT_BITS(WAIT_BITS)) dut (
	.clk(clk), .rst(rst), .toma(t), .dado(w), .valeu(valeu),
	.hs(hs), .vs(vs), .ativo(ativo), .pixel(pixel), .scl(scl), .sda(sda));
myVGAText #(.NUBITS(NUBITS), .LINK_SYNC(LINK_SYNC)) vga (
	.clk(clk), .rst(rst), .toma(t), .dado(w), .valeu(valeu_r),
	.hs(hs_r), .vs(vs_r), .ativo(ativo_r), .pixel(pixel_r));

integer errs = 0;
task err; input [8*80-1:0] m; begin errs = errs + 1; if (errs < 20) $display("  %0s", m); end endtask

// ------------------------------------------------- the screen: the reference --

always @ (posedge clk)
	if ({valeu, hs, vs, ativo, pixel} !== {valeu_r, hs_r, vs_r, ativo_r, pixel_r}) begin
		errs = errs + 1;
		if (errs < 20) $display("  at %0t: valeu hs vs ativo pixel %b%b%b%b%b, myVGAText %b%b%b%b%b", $time,
		                        valeu, hs, vs, ativo, pixel, valeu_r, hs_r, vs_r, ativo_r, pixel_r);
	end

task toma; input integer v; begin
	@(posedge clk);
	w = v; t = ~t;
	while (valeu !== t) @(posedge clk);
	@(posedge clk);                          // the write lands the cycle after C flips
end endtask
task say; input [8*16-1:0] s; integer k; reg [7:0] ch; begin
	for (k = 15; k >= 0; k = k - 1) begin ch = s[8*k +: 8]; if (ch != 0) toma(ch); end
end endtask
function [6:0] at; input integer r, c; integer m; begin
	m = r + dut.tela.top; if (m >= 30) m = m - 30;
	at = dut.tela.smem[m*80 + c];
end endfunction
task want_text; input integer r, c; input [8*16-1:0] s; input integer n; integer k; reg [7:0] e; begin
	for (k = 0; k < n; k = k + 1) begin
		e = s[8*(n-1-k) +: 8];
		if (at(r, c + k) !== e[6:0]) begin
			errs = errs + 1;
			if (errs < 20) $display("  screen (%0d,%0d): %0d, want '%c'", r, c + k, at(r, c + k), e);
		end
	end
end endtask

// -------------------------------------------------------- the I2C: the chip --

reg [15:0] want [0:10];
initial begin
	want[0] = 16'hD6C0; want[1] = 16'h4110; want[2] = 16'h9803; want[3] = 16'h9AE0;
	want[4] = 16'h9C30; want[5] = 16'h9D61; want[6] = 16'hA2A4; want[7] = 16'hA3A4;
	want[8] = 16'hE0D0; want[9] = 16'hF900; want[10] = 16'h1630;
end

reg        ack = 0;                          // the chip pulls SDA low for its ACK
assign sda = ack ? 1'b0 : 1'bz;
reg        in_tx = 0;
integer    nbit = 0, nbyte = 0, ntx = 0;
reg  [7:0] sh;
reg  [7:0] got [0:2];

always @ (negedge sda) if (scl === 1'b1) begin          // START
	if (in_tx) err("START inside a transaction");
	in_tx = 1; nbit = 0; nbyte = 0;
end
always @ (posedge sda) if (scl === 1'b1 && in_tx) begin // STOP
	in_tx = 0;                               // SCL rose once with SDA low just before: the STOP's own
	if (nbyte != 3 || nbit != 1 || sh[0] !== 1'b0) begin
		errs = errs + 1; $display("  write %0d: STOP after %0d bytes and %0d bits", ntx % 11, nbyte, nbit);
	end else if (got[0] !== 8'h72 || {got[1], got[2]} !== want[ntx % 11]) begin
		errs = errs + 1;
		$display("  write %0d: %h %h %h, want 72 %h %h", ntx % 11, got[0], got[1], got[2],
		         want[ntx % 11][15:8], want[ntx % 11][7:0]);
	end
	ntx = ntx + 1;
end
always @ (posedge scl) if (in_tx) begin
	if (nbit < 8) begin sh = {sh[6:0], sda === 1'b0 ? 1'b0 : 1'b1}; nbit = nbit + 1; end
	else begin                               // the ACK's clock
		if (dut.sda_r !== 1'b1) err("the master holds SDA in an ACK slot");
		if (nbyte < 3) got[nbyte] = sh;
		nbyte = nbyte + 1; nbit = 0;
	end
end
always @ (negedge scl) if (in_tx) #100 ack = (nbit == 8);

// the master's own lines, timed
localparam real TMIN = 1300.0;               // ns: tLOW of fast mode, the largest of its limits
realtime t_scl = 0, t_sda = 0, t_rel = 0, t_first = -1;
reg      scl_m = 1, sda_m = 1;
always @ (dut.scl_r or dut.sda_r) begin
	if (dut.scl_r !== scl_m) begin
		if ($realtime - t_scl < TMIN || $realtime - t_sda < TMIN) err("SCL edge too close to another edge");
		t_scl = $realtime; scl_m = dut.scl_r;
	end
	if (dut.sda_r !== sda_m) begin
		if ($realtime - t_scl < TMIN || $realtime - t_sda < TMIN) err("SDA edge too close to another edge");
		t_sda = $realtime; sda_m = dut.sda_r;
	end
	if (t_first < 0 && (dut.scl_r === 1'b0 || dut.sda_r === 1'b0)) t_first = $realtime;
end

// ---------------------------------------------------------------- the run --

integer k;
realtime wait_ns;
initial begin
	wait_ns = (2.0 ** WAIT_BITS) * 800 * 40;
	#200 rst = 0; t_rel = $realtime;
	#100000;                                 // the screen clears (2,400 cycles)
	toma(-128); say("SAPHO"); toma(10); say("HDMI 640x480"); toma(-127); toma(5); toma(70); say("ok");
	wait (dut.fim);
	repeat (100 * 800) @(posedge clk);       // a hundred lines more: the bus stays quiet
	want_text(0, 0, "SAPHO", 5); want_text(1, 0, "HDMI 640x480", 12); want_text(5, 70, "ok", 2);
	for (k = 0; k < 2400; k = k + 1) if (dut.tela.smem[k] !== vga.smem[k]) begin
		errs = errs + 1; if (errs < 20) $display("  screen memory %0d: %0d, myVGAText %0d", k, dut.tela.smem[k], vga.smem[k]); end
	if (ntx != 11) begin errs = errs + 1; $display("  %0d writes, want 11", ntx); end
	if (t_first - t_rel < wait_ns) begin
		errs = errs + 1; $display("  first START %0.1f ms after reset, want %0.1f ms or more", (t_first - t_rel) / 1e6, wait_ns / 1e6); end
	if (WAIT_BITS == 13 && t_first - t_rel < 200e6) begin
		errs = errs + 1; $display("  first START %0.1f ms after reset, the ADV7513 needs 200", (t_first - t_rel) / 1e6); end
	$display("  writes done %0.1f ms after reset (first START at %0.1f ms)", ($realtime - t_rel) / 1e6 - 100 * 800 * 40 / 1e6,
	         (t_first - t_rel) / 1e6);
	// a second reset: the eleven writes again, after the wait
	rst = 1; repeat (10) @(posedge clk); rst = 0; t_rel = $realtime; t_first = -1;
	wait (dut.fim);
	repeat (100 * 800) @(posedge clk);
	if (ntx != 22) begin errs = errs + 1; $display("  %0d writes after the second reset, want 22", ntx); end
	if (t_first - t_rel < wait_ns) begin
		errs = errs + 1; $display("  second reset: first START %0.1f ms after it", (t_first - t_rel) / 1e6); end
	if (errs == 0) $display("myADV7513Text NUBITS=%0d LINK_SYNC=%0d WAIT_BITS=%0d: ok", NUBITS, LINK_SYNC, WAIT_BITS);
	else           $display("myADV7513Text NUBITS=%0d LINK_SYNC=%0d WAIT_BITS=%0d: FAIL, %0d errors", NUBITS, LINK_SYNC, WAIT_BITS, errs);
	$finish;
end

endmodule
