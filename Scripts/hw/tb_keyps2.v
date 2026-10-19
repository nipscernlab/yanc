`timescale 1ns/1ps
// Testbench of SAPHO/myKeyPS2.v (docs/design/ps2-keyboard.md). A keyboard
// model sends PS/2 frames with a real keyboard's timing (12.5 kHz clock,
// odd parity); a reader plays the SAPHO's side of the cade link (it takes a
// word when T != C and flips C, after a random 0-3 cycle delay, as CAD does
// when it gets there). The characters expected are written here from the
// layout tables of the design note, not from the block's table.
//
// Cases: every letter with and without Shift (both Shift keys), the digit
// row and its shifted symbols, the layout-dependent punctuation, space /
// Enter / Backspace / Tab / Esc; releases give nothing; extended keys
// (arrows, keypad Enter E0 5A and its release E0 F0 5A, a fake shift E0 12)
// give nothing and do not disturb the next key; keypad, F1, Caps Lock give
// nothing; a frame with a bad parity is dropped; a frame cut in the middle is
// forgotten after the 1 ms timeout and the next one decodes; the keyboard's
// power-on byte AA and an FA are ignored; a held key repeats; with the reader
// stalled, the queue keeps the first DEPTH characters and drops the rest.
//
// Run:  iverilog -g2012 -s tb -Ptb.LAYOUT=\"ABNT2\" -o k.vvp Scripts/hw/tb_keyps2.v SAPHO/myKeyPS2.v && vvp -n k.vvp
//       (Scripts/hw/tb_keyps2.sh runs both layouts at two clocks)
module tb;

parameter LAYOUT  = "US";
parameter CLK_KHZ = 2000;          // small by default: the same circuit, fewer cycles to simulate
parameter DEPTH   = 4;            // the block's default

localparam ABNT2 = (LAYOUT == "ABNT2");

reg clk = 0, rst = 1;
always #(500000.0 / CLK_KHZ) clk = ~clk;

reg  ps2_clk = 1, ps2_dat = 1;
wire taqui;
wire [31:0] dado;
reg  c = 0;

myKeyPS2 #(.LAYOUT(LAYOUT), .NUBITS(32), .DEPTH(DEPTH), .CLK_KHZ(CLK_KHZ))
	dut (.clk(clk), .rst(rst), .ps2_clk(ps2_clk), .ps2_dat(ps2_dat),
	     .taqui(taqui), .dado(dado), .cade(c));

// ---------------------------------------------------------------- keyboard --

localparam HALF = 40000;           // ns: half a bit at 12.5 kHz

task frame;                        // one byte; bad = 1 sends a wrong parity
	input [7:0] b;
	input       bad;
	reg  [10:0] f;
	integer j;
	begin
		f = {1'b1, ~^b ^ bad, b, 1'b0};    // stop, odd parity, data, start
		for (j = 0; j < 11; j = j + 1) begin
			ps2_dat = f[j];
			#(HALF) ps2_clk = 0;
			#(HALF) ps2_clk = 1;
		end
		ps2_dat = 1;
		#(2 * HALF);                       // gap between bytes
	end
endtask

task cut;                          // a frame that stops after 5 bits
	input [7:0] b;
	reg  [10:0] f;
	integer j;
	begin
		f = {1'b1, ~^b, b, 1'b0};
		for (j = 0; j < 5; j = j + 1) begin
			ps2_dat = f[j];
			#(HALF) ps2_clk = 0;
			#(HALF) ps2_clk = 1;
		end
		ps2_dat = 1;
		#(2000000);                        // 2 ms: past the 1 ms timeout
	end
endtask

task press;   input [7:0] sc; frame(sc, 0);                              endtask
task solta; input [7:0] sc; begin frame(8'hF0, 0); frame(sc, 0); end  endtask
task tap;     input [7:0] sc; begin press(sc); solta(sc); end          endtask
task tap_ext; input [7:0] sc; begin frame(8'hE0, 0); frame(sc, 0);
                                    frame(8'hE0, 0); frame(8'hF0, 0); frame(sc, 0); end endtask

// ------------------------------------------------------------ expectations --

reg [7:0] exp [0:1023];
integer   nexp = 0;
task want; input [7:0] ch; begin exp[nexp] = ch; nexp = nexp + 1; end endtask

// ------------------------------------------------- the SAPHO side (reader) --

reg [7:0] got [0:1023];
integer   ngot = 0, dly = 0, errs = 0;
reg       run  = 1;
integer   seed = 7;

always @ (posedge clk) begin
	if (!rst && run && taqui != c) begin
		if (dly == 0) begin
			if (dado[31:8] !== 24'd0) begin
				errs = errs + 1;
				$display("  word %h: high bits not zero", dado);
			end
			got[ngot] = dado[7:0];
			ngot = ngot + 1;
			c   <= ~c;
			dly = {$random(seed)} % 4;
		end else
			dly = dly - 1;
	end
end

// ------------------------------------------------------------------ scancodes --

reg [7:0] letter [0:25];           // a..z
reg [7:0] digit  [0:9];            // 1..9, 0
initial begin
	letter[ 0]=8'h1C; letter[ 1]=8'h32; letter[ 2]=8'h21; letter[ 3]=8'h23; letter[ 4]=8'h24;
	letter[ 5]=8'h2B; letter[ 6]=8'h34; letter[ 7]=8'h33; letter[ 8]=8'h43; letter[ 9]=8'h3B;
	letter[10]=8'h42; letter[11]=8'h4B; letter[12]=8'h3A; letter[13]=8'h31; letter[14]=8'h44;
	letter[15]=8'h4D; letter[16]=8'h15; letter[17]=8'h2D; letter[18]=8'h1B; letter[19]=8'h2C;
	letter[20]=8'h3C; letter[21]=8'h2A; letter[22]=8'h1D; letter[23]=8'h22; letter[24]=8'h35;
	letter[25]=8'h1A;
	digit[0]=8'h16; digit[1]=8'h1E; digit[2]=8'h26; digit[3]=8'h25; digit[4]=8'h2E;
	digit[5]=8'h36; digit[6]=8'h3D; digit[7]=8'h3E; digit[8]=8'h46; digit[9]=8'h45;
end

// the shifted digit row, 1..0
function [7:0] shifted_digit; input integer k;
	case (k)
		0: shifted_digit = "!";  1: shifted_digit = "@";  2: shifted_digit = "#";
		3: shifted_digit = "$";  4: shifted_digit = "%";
		5: shifted_digit = ABNT2 ? 8'd0 : "^";                    // ABNT2: diaeresis
		6: shifted_digit = "&";  7: shifted_digit = "*";  8: shifted_digit = "(";
		default: shifted_digit = ")";
	endcase
endfunction

// the punctuation keys: scancode, then {US, US+Shift, ABNT2, ABNT2+Shift}
localparam NP = 14;
reg [ 7:0] psc [0:NP-1];
reg [31:0] pch [0:NP-1];
initial begin
	psc[ 0]=8'h0E; pch[ 0]={"`", "~", "'", "\""};
	psc[ 1]=8'h54; pch[ 1]={"[", "{", 8'd0, "`"};
	psc[ 2]=8'h5B; pch[ 2]={"]", "}", "[", "{"};
	psc[ 3]=8'h4C; pch[ 3]={";", ":", 8'd0, 8'd0};
	psc[ 4]=8'h52; pch[ 4]={"'", "\"", "~", "^"};
	psc[ 5]=8'h5D; pch[ 5]={"\\", "|", "]", "}"};
	psc[ 6]=8'h4A; pch[ 6]={"/", "?", ";", ":"};
	psc[ 7]=8'h61; pch[ 7]={"\\", "|", "\\", "|"};
	psc[ 8]=8'h51; pch[ 8]={8'd0, 8'd0, "/", "?"};
	psc[ 9]=8'h4E; pch[ 9]={"-", "_", "-", "_"};
	psc[10]=8'h55; pch[10]={"=", "+", "=", "+"};
	psc[11]=8'h41; pch[11]={",", "<", ",", "<"};
	psc[12]=8'h49; pch[12]={".", ">", ".", ">"};
	psc[13]=8'h29; pch[13]={" ", " ", " ", " "};
end

function [7:0] pchar; input integer k; input sh;
	pchar = ABNT2 ? (sh ? pch[k][7:0]   : pch[k][15:8])
	              : (sh ? pch[k][23:16] : pch[k][31:24]);
endfunction

// -------------------------------------------------------------------- run --

integer k, t;
reg [7:0] ch;

initial begin
	#(5000000.0 / CLK_KHZ) rst = 0;

	frame(8'hAA, 0);                          // the keyboard's power-on byte
	frame(8'hFA, 0);                          // an acknowledge

	for (k = 0; k < 26; k = k + 1) begin tap(letter[k]); want("a" + k); end
	press(8'h12);                             // left Shift
	for (k = 0; k < 26; k = k + 1) begin tap(letter[k]); want("A" + k); end
	solta(8'h12);
	press(8'h59);                             // right Shift
	tap(letter[16]); want("Q");
	solta(8'h59);
	tap(letter[16]); want("q");

	for (k = 0; k < 10; k = k + 1) begin tap(digit[k]); want((k == 9) ? "0" : "1" + k); end
	press(8'h12);
	for (k = 0; k < 10; k = k + 1) begin tap(digit[k]); ch = shifted_digit(k); if (ch != 0) want(ch); end
	solta(8'h12);

	for (k = 0; k < NP; k = k + 1) begin tap(psc[k]); ch = pchar(k, 0); if (ch != 0) want(ch); end
	press(8'h59);
	for (k = 0; k < NP; k = k + 1) begin tap(psc[k]); ch = pchar(k, 1); if (ch != 0) want(ch); end
	solta(8'h59);

	tap(8'h5A); want(8'd10);                  // Enter
	tap(8'h66); want(8'd8);                   // Backspace
	tap(8'h0D); want(8'd9);                   // Tab
	tap(8'h76); want(8'd27);                  // Esc

	// keys that give nothing, and must not disturb the next one
	tap_ext(8'h75); tap_ext(8'h6B);           // arrows
	tap_ext(8'h5A);                           // keypad Enter: E0 5A, then E0 F0 5A
	tap(8'h70); tap(8'h7C); tap(8'h79);       // keypad 0, *, +
	tap(8'h05); tap(8'h58);                   // F1, Caps Lock
	frame(8'hE0, 0); frame(8'h12, 0);         // a fake shift (Print Screen sends E0 12)
	tap(letter[0]); want("a");                // still lower case
	frame(8'hE0, 0); frame(8'hF0, 0); frame(8'h12, 0);

	// a bad parity, then a cut frame: both lost, the next key is fine
	frame(letter[1], 1);
	tap(letter[2]); want("c");
	cut(letter[3]);
	tap(letter[4]); want("e");

	// a held key repeats
	press(letter[25]); press(letter[25]); press(letter[25]); solta(letter[25]);
	want("z"); want("z"); want("z");

	// the reader stalls: the first DEPTH characters stay, the rest are dropped
	@(posedge clk); run = 0;
	for (k = 0; k < DEPTH + 3; k = k + 1) begin
		tap(letter[k % 26]);
		if (k < DEPTH) want("a" + (k % 26));
	end
	@(posedge clk); run = 1;

	tap(letter[24]); want("y");               // and the queue works after it

	for (t = 0; t < 200000 && ngot < nexp; t = t + 1) @(posedge clk);
	repeat (100) @(posedge clk);

	for (k = 0; k < nexp || k < ngot; k = k + 1)
		if (k >= nexp || k >= ngot || got[k] !== exp[k]) begin
			errs = errs + 1;
			if (errs <= 5) $display("  char %0d: got %h, expected %h", k,
			                        (k < ngot) ? got[k] : 8'hxx, (k < nexp) ? exp[k] : 8'hxx);
		end

	if (errs == 0) $display("%0s CLK_KHZ=%0d DEPTH=%0d: ok (%0d characters)", LAYOUT, CLK_KHZ, DEPTH, ngot);
	else           $display("%0s CLK_KHZ=%0d DEPTH=%0d: FAIL, %0d errors (%0d got, %0d expected)", LAYOUT, CLK_KHZ, DEPTH, errs, ngot, nexp);
	$finish;
end

endmodule
