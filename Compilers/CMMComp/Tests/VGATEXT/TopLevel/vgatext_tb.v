`timescale 1ns/1ps
// Regress project pass VGATEXT: SAPHO/myKeyPS2.v and SAPHO/myVGAText.v on
// whole processors, everything on one 25 MHz clock. One keyboard drives two
// systems: the C+- SAPHO vga_cmm with a US keyboard block, the C++ SAPHO
// vga_cpp with an ABNT2 one, both of 8 bits, each echoing the keys to its own
// screen. The keyboard types "Oi, SAPHO!", Enter, "x = 1" and a Backspace
// (keys that are the same on both layouts). The bench reads each screen
// memory through its top register and prints one line per system.
module vgatext_tb;

reg clk = 0, rst = 1;
always #20 clk = ~clk;                      // 25 MHz

reg ps2_clk = 1, ps2_dat = 1;

// system A: C+-, US
wire ca, ta, va, tma; wire [7:0] ka, da;
wire [7:0] oa; wire hsa, vsa, aa, pa;
vga_cmm A (.clk(clk), .rst(rst), .out(oa), .cade_0(ca), .taqui_0(ta), .cade_dado_0(ka),
           .toma_1(tma), .valeu_1(va), .toma_dado(da));
myKeyPS2  #(.LAYOUT("US"), .NUBITS(8), .CLK_KHZ(25000)) KA (.clk(clk), .rst(rst),
           .ps2_clk(ps2_clk), .ps2_dat(ps2_dat), .taqui(ta), .dado(ka), .cade(ca));
myVGAText #(.NUBITS(8)) VA (.clk(clk), .rst(rst), .toma(tma), .dado(da), .valeu(va),
           .hs(hsa), .vs(vsa), .ativo(aa), .pixel(pa));

// system B: C++, ABNT2
wire cb, tb_, vb, tmb; wire [7:0] kb, db;
wire [7:0] ob; wire hsb, vsb, ab, pb;
vga_cpp B (.clk(clk), .rst(rst), .out(ob), .cade_0(cb), .taqui_0(tb_), .cade_dado_0(kb),
           .toma_1(tmb), .valeu_1(vb), .toma_dado(db));
myKeyPS2  #(.LAYOUT("ABNT2"), .NUBITS(8), .CLK_KHZ(25000)) KB (.clk(clk), .rst(rst),
           .ps2_clk(ps2_clk), .ps2_dat(ps2_dat), .taqui(tb_), .dado(kb), .cade(cb));
myVGAText #(.NUBITS(8)) VB (.clk(clk), .rst(rst), .toma(tmb), .dado(db), .valeu(vb),
           .hs(hsb), .vs(vsb), .ativo(ab), .pixel(pb));

// the keyboard
localparam HALF = 10000;                    // ns: a 50 kHz PS/2 clock, four times the norm, so the
                                            // regress runs faster; myKeyPS2 samples at 25 MHz either way
task frame; input [7:0] b; reg [10:0] w; integer j; begin
	w = {1'b1, ~^b, b, 1'b0};
	for (j = 0; j < 11; j = j + 1) begin ps2_dat = w[j]; #(HALF) ps2_clk = 0; #(HALF) ps2_clk = 1; end
	ps2_dat = 1; #(2 * HALF);
end endtask
task tap;  input [7:0] sc; begin frame(sc); frame(8'hF0); frame(sc); end endtask
task stap; input [7:0] sc; begin frame(8'h12); tap(sc); frame(8'hF0); frame(8'h12); end endtask

// a screen, through its top register
function [6:0] at_a; input integer r, c; integer m; begin
	m = r + VA.top; if (m >= 30) m = m - 30; at_a = VA.smem[m*80 + c];
end endfunction
function [6:0] at_b; input integer r, c; integer m; begin
	m = r + VB.top; if (m >= 30) m = m - 30; at_b = VB.smem[m*80 + c];
end endfunction

integer ea = 0, eb = 0;
task want; input integer sys, r, c; input [8*16-1:0] s; input integer n; integer k; reg [7:0] e, g; begin
	for (k = 0; k < n; k = k + 1) begin
		e = s[8*(n-1-k) +: 8];
		g = (sys == 0) ? at_a(r, c + k) : at_b(r, c + k);
		if (g !== e) begin
			if (sys == 0) ea = ea + 1; else eb = eb + 1;
			$display("  %s screen (%0d,%0d): '%c' (%0d), want '%c'", (sys == 0) ? "vga_cmm" : "vga_cpp", r, c + k, g, g, e);
		end
	end
end endtask

initial begin
	#200 rst = 0;
	#200000;                                 // the screens clear (2,400 cycles) and the programs write
	stap(8'h44); tap(8'h43); tap(8'h41); tap(8'h29);                 // O i , space
	stap(8'h1B); stap(8'h1C); stap(8'h4D); stap(8'h33); stap(8'h44); // S A P H O
	stap(8'h16);                                                     // !
	tap(8'h5A);                                                      // Enter
	tap(8'h22); tap(8'h29); tap(8'h55); tap(8'h29); tap(8'h16);      // x space = space 1
	tap(8'h66);                                                      // Backspace
	#200000;
	want(0, 0, 0, "> Oi, SAPHO!", 12);  want(0, 1, 0, "x =  ", 5);
	want(1, 0, 0, "SAPHO", 5);          want(1, 2, 4, "Oi, SAPHO!", 10);  want(1, 3, 0, "x =  ", 5);
	if (VA.crow !== 1 || VA.ccol !== 4) begin ea = ea + 1; $display("  vga_cmm cursor (%0d,%0d), want (1,4)", VA.crow, VA.ccol); end
	if (VB.crow !== 3 || VB.ccol !== 4) begin eb = eb + 1; $display("  vga_cpp cursor (%0d,%0d), want (3,4)", VB.crow, VB.ccol); end
	if (ea == 0) $display("VGATEXT vga_cmm (C+-, 8 bits, US): ok");
	else         $display("VGATEXT vga_cmm (C+-, 8 bits, US): FAIL, %0d", ea);
	if (eb == 0) $display("VGATEXT vga_cpp (C++, 8 bits, ABNT2): ok");
	else         $display("VGATEXT vga_cpp (C++, 8 bits, ABNT2): FAIL, %0d", eb);
	$finish;
end

endmodule
