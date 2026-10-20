`timescale 1ns/1ps
// Regress project pass VGATEXT: SAPHO/myKeyPS2.v and SAPHO/myVGAText.v on
// whole processors, everything on one 25 MHz clock. One keyboard drives two
// systems: the C+- SAPHO vga_cmm with a US keyboard block, the C++ SAPHO
// vga_cpp with an ABNT2 one, both of 8 bits, each echoing the keys to its own
// screen. The keyboard types "Oi, SAPHO!", Enter, "x = 1" and a Backspace
// (keys that are the same on both layouts). A third system, the 32-bit C++
// SAPHO vga_io, uses <iostream> on a keyboard and a screen of its own: its
// keyboard types "Anx", Backspace, "a", Enter, "12 30", Enter. The bench
// reads each screen memory through its top register and prints one line
// per system.
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

// system C: C++ with <iostream>, 32 bits, US, its own keyboard
reg ps2_clk_c = 1, ps2_dat_c = 1;
wire cc, tc, vc, tmc; wire [31:0] kc, dc;
wire [31:0] oc; wire hsc, vsc, ac, pc;
vga_io C (.clk(clk), .rst(rst), .out(oc), .cade_0(cc), .taqui_0(tc), .cade_dado_0(kc),
          .toma_1(tmc), .valeu_1(vc), .toma_dado(dc));
myKeyPS2  #(.LAYOUT("US"), .NUBITS(32), .CLK_KHZ(25000)) KC (.clk(clk), .rst(rst),
          .ps2_clk(ps2_clk_c), .ps2_dat(ps2_dat_c), .taqui(tc), .dado(kc), .cade(cc));
myVGAText #(.NUBITS(32)) VC (.clk(clk), .rst(rst), .toma(tmc), .dado(dc), .valeu(vc),
          .hs(hsc), .vs(vsc), .ativo(ac), .pixel(pc));

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
task frame_c; input [7:0] b; reg [10:0] w; integer j; begin      // system C's keyboard
	w = {1'b1, ~^b, b, 1'b0};
	for (j = 0; j < 11; j = j + 1) begin ps2_dat_c = w[j]; #(HALF) ps2_clk_c = 0; #(HALF) ps2_clk_c = 1; end
	ps2_dat_c = 1; #(2 * HALF);
end endtask
task tap_c;  input [7:0] sc; begin frame_c(sc); frame_c(8'hF0); frame_c(sc); end endtask
task stap_c; input [7:0] sc; begin frame_c(8'h12); tap_c(sc); frame_c(8'hF0); frame_c(8'h12); end endtask

// a screen, through its top register
function [6:0] at_a; input integer r, c; integer m; begin
	m = r + VA.top; if (m >= 30) m = m - 30; at_a = VA.smem[m*80 + c];
end endfunction
function [6:0] at_b; input integer r, c; integer m; begin
	m = r + VB.top; if (m >= 30) m = m - 30; at_b = VB.smem[m*80 + c];
end endfunction
function [6:0] at_c; input integer r, c; integer m; begin
	m = r + VC.top; if (m >= 30) m = m - 30; at_c = VC.smem[m*80 + c];
end endfunction

integer ea = 0, eb = 0, ec = 0;
task want; input integer sys, r, c; input [8*40-1:0] s; input integer n; integer k; reg [7:0] e, g; begin
	for (k = 0; k < n; k = k + 1) begin
		e = s[8*(n-1-k) +: 8];
		g = (sys == 0) ? at_a(r, c + k) : (sys == 1) ? at_b(r, c + k) : at_c(r, c + k);
		if (g !== e) begin
			if (sys == 0) ea = ea + 1; else if (sys == 1) eb = eb + 1; else ec = ec + 1;
			$display("  %s screen (%0d,%0d): '%c' (%0d), want '%c'", (sys == 0) ? "vga_cmm" : (sys == 1) ? "vga_cpp" : "vga_io", r, c + k, g, g, e);
		end
	end
end endtask

integer t;
initial begin                                // system C's keyboard, beside the other one
	#400000;
	stap_c(8'h1C); tap_c(8'h31); tap_c(8'h22); tap_c(8'h66); tap_c(8'h1C);   // A n x Backspace a
	tap_c(8'h5A);                                                           // Enter
	tap_c(8'h16); tap_c(8'h1E); tap_c(8'h29); tap_c(8'h26); tap_c(8'h45);   // 1 2 space 3 0
	tap_c(8'h5A);                                                           // Enter
end

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
	for (t = 0; t < 4000 && !(VC.crow == 6 && VC.ccol == 0); t = t + 1) #10000;   // system C still printing
	want(0, 0, 0, "> Oi, SAPHO!", 12);  want(0, 1, 0, "x =  ", 5);
	want(1, 0, 0, "SAPHO", 5);          want(1, 2, 4, "Oi, SAPHO!", 10);  want(1, 3, 0, "x =  ", 5);
	if (VA.crow !== 1 || VA.ccol !== 4) begin ea = ea + 1; $display("  vga_cmm cursor (%0d,%0d), want (1,4)", VA.crow, VA.ccol); end
	if (VB.crow !== 3 || VB.ccol !== 4) begin eb = eb + 1; $display("  vga_cpp cursor (%0d,%0d), want (3,4)", VB.crow, VB.ccol); end
	want(2, 0, 0, "Nome? Ana ", 10);        want(2, 1, 0, "Oi, Ana! ", 9);
	want(2, 2, 0, "a b? 12 30 ", 11);       want(2, 3, 0, "12 + 30 = 42 ", 13);
	want(2, 4, 0, "-7 ff 18 3.14159 1e+10 0.00015 ", 31);
	want(2, 5, 0, "Ana tem 3 letras ", 17);
	if (VC.crow !== 6 || VC.ccol !== 0) begin ec = ec + 1; $display("  vga_io cursor (%0d,%0d), want (6,0)", VC.crow, VC.ccol); end
	if (ea == 0) $display("VGATEXT vga_cmm (C+-, 8 bits, US): ok");
	else         $display("VGATEXT vga_cmm (C+-, 8 bits, US): FAIL, %0d", ea);
	if (eb == 0) $display("VGATEXT vga_cpp (C++, 8 bits, ABNT2): ok");
	else         $display("VGATEXT vga_cpp (C++, 8 bits, ABNT2): FAIL, %0d", eb);
	if (ec == 0) $display("VGATEXT vga_io (C++ <iostream>, 32 bits, US): ok");
	else         $display("VGATEXT vga_io (C++ <iostream>, 32 bits, US): FAIL, %0d", ec);
	$finish;
end

endmodule
