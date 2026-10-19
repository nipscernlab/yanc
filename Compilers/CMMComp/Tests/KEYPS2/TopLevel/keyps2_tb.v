`timescale 1ns/1ps
// Regress project pass KEYPS2: SAPHO/myKeyPS2.v wired to whole processors.
// One keyboard drives two blocks: a US one on the C+- SAPHO kbd_cmm and an
// ABNT2 one on the C++ SAPHO kbd_cpp, both with an 8-bit word, both reading
// with cade_sync(0) and echoing to port 0. The keys typed have the same
// position on both keyboards; the characters expected differ only on the
// layout-dependent keys. Arrows and the keypad Enter (press and release)
// give nothing. Prints one line per processor, "ok" or what came out.
module keyps2_tb;

localparam CLK_KHZ = 2000;                 // a slow clock: the same circuit, fewer cycles
reg clk = 0, rst = 1;
always #(500000.0 / CLK_KHZ) clk = ~clk;

reg ps2_clk = 1, ps2_dat = 1;

wire signed [7:0] out_a, out_b;
wire [0:0] en_a, en_b;
wire cade_a, taqui_a, cade_b, taqui_b;
wire [7:0] dado_a, dado_b;

kbd_cmm A (.clk(clk), .rst(rst), .out(out_a), .out_en(en_a),
           .cade_0(cade_a), .taqui_0(taqui_a), .cade_dado_0(dado_a));
myKeyPS2 #(.LAYOUT("US"), .NUBITS(8), .CLK_KHZ(CLK_KHZ)) KA (.clk(clk), .rst(rst),
           .ps2_clk(ps2_clk), .ps2_dat(ps2_dat), .taqui(taqui_a), .dado(dado_a), .cade(cade_a));

kbd_cpp B (.clk(clk), .rst(rst), .out(out_b), .out_en(en_b),
           .cade_0(cade_b), .taqui_0(taqui_b), .cade_dado_0(dado_b));
myKeyPS2 #(.LAYOUT("ABNT2"), .NUBITS(8), .CLK_KHZ(CLK_KHZ)) KB (.clk(clk), .rst(rst),
           .ps2_clk(ps2_clk), .ps2_dat(ps2_dat), .taqui(taqui_b), .dado(dado_b), .cade(cade_b));

// what each processor wrote on port 0
reg [7:0] ga [0:63], gb [0:63];
integer na = 0, nb = 0;
always @ (posedge clk) begin
	if (en_a[0]) begin ga[na] = out_a; na = na + 1; end
	if (en_b[0]) begin gb[nb] = out_b; nb = nb + 1; end
end

// the keyboard
localparam HALF = 40000;                   // ns: half a bit at 12.5 kHz
task frame; input [7:0] b; reg [10:0] w; integer j; begin
	w = {1'b1, ~^b, b, 1'b0};
	for (j = 0; j < 11; j = j + 1) begin ps2_dat = w[j]; #(HALF) ps2_clk = 0; #(HALF) ps2_clk = 1; end
	ps2_dat = 1; #(2 * HALF);
end endtask
task tap;   input [7:0] sc; begin frame(sc); frame(8'hF0); frame(sc); end endtask
task stap;  input [7:0] sc; begin frame(8'h12); tap(sc); frame(8'hF0); frame(8'h12); end endtask
task etap;  input [7:0] sc; begin frame(8'hE0); frame(sc); frame(8'hE0); frame(8'hF0); frame(sc); end endtask

// expected, as text: US on A; "ok:" then ABNT2 on B
//   keys: Shift+H, I, space, 1, Shift+1, 4C, 4A, 52, 51, Shift+5B, up, keypad Enter, Enter
localparam [8*10-1:0] WANT_A = {"Hi 1!;/'}", 8'd10};
localparam [8*13-1:0] WANT_B = {"ok:", "Hi 1!;~/{", 8'd10};

function check;
	input integer which;
	integer k, n;
	reg [7:0] w, g;
	begin
		n = (which == 0) ? 10 : 13;
		check = ((which == 0) ? na : nb) == n;
		for (k = 0; k < n && check; k = k + 1) begin
			w = (which == 0) ? WANT_A[8*(n-1-k) +: 8] : WANT_B[8*(n-1-k) +: 8];
			g = (which == 0) ? ga[k] : gb[k];
			if (g !== w) check = 0;
		end
	end
endfunction

task show; input integer which; integer k; begin
	$write("got");
	for (k = 0; k < ((which == 0) ? na : nb); k = k + 1) $write(" %0d", (which == 0) ? ga[k] : gb[k]);
	$display("");
end endtask

initial begin
	#(5000000.0 / CLK_KHZ) rst = 0;
	stap(8'h33); tap(8'h43); tap(8'h29);              // H i space
	tap(8'h16); stap(8'h16);                          // 1 !
	tap(8'h4C); tap(8'h4A); tap(8'h52); tap(8'h51);   // US ; / ' (none), ABNT2 (none) ; ~ /
	stap(8'h5B);                                      // US }, ABNT2 {
	etap(8'h75); etap(8'h5A);                         // up arrow, keypad Enter: nothing
	tap(8'h5A);                                       // Enter
	#(2000000);
	if (check(0)) $display("KEYPS2 kbd_cmm (C+-, 8 bits, US): ok");
	else begin $write("KEYPS2 kbd_cmm (C+-, 8 bits, US): FAIL, "); show(0); end
	if (check(1)) $display("KEYPS2 kbd_cpp (C++, 8 bits, ABNT2): ok");
	else begin $write("KEYPS2 kbd_cpp (C++, 8 bits, ABNT2): FAIL, "); show(1); end
	$finish;
end

endmodule
