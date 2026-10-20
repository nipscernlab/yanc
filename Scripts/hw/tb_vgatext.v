`timescale 1ns/1ps
// Testbench of SAPHO/myVGAText.v (docs/design/vga-text.md). It plays the
// SAPHO's side of the toma link (offers a word: flips T and waits for C), and
// checks the block from outside where it can:
//   - the picture, rebuilt from `ativo`, `pixel`, `hs` and `vs` alone, as a
//     monitor would: the timing (800 x 525, the pulse widths and where they
//     sit), the glyph 'A' drawn against rows copied from the font by hand,
//     the cursor's underline on and off;
//   - the words: characters, the control codes, clear, the cursor command, a
//     scroll, ignored codes, read back from the screen memory through the top
//     register (screen line r is memory line (r + top) mod 30);
//   - the handshake: a clear holds the SAPHO about 2,400 cycles, a character
//     a few.
// Run: bash Scripts/hw/tb_vgatext.sh
module tb;

parameter NUBITS = 8;

reg clk = 0, rst = 1;
always #20 clk = ~clk;                       // 25 MHz

reg  t = 0;                                  // the SAPHO's T bit
reg  [NUBITS-1:0] w = 0;
wire valeu, hs, vs, ativo, pixel;

myVGAText #(.NUBITS(NUBITS)) dut (.clk(clk), .rst(rst), .toma(t), .dado(w), .valeu(valeu),
                                   .hs(hs), .vs(vs), .ativo(ativo), .pixel(pixel));

integer errs = 0;

// ------------------------------------------------------------------- toma --

integer wait_cycles;
task toma; input integer v; begin
	@(posedge clk); w = v; t = ~t;
	wait_cycles = 0;
	while (valeu !== t) begin @(posedge clk); wait_cycles = wait_cycles + 1; end
	@(posedge clk);                          // the write lands the cycle after C flips
end endtask
task say; input [8*40-1:0] s; integer k; reg [7:0] ch; begin
	for (k = 39; k >= 0; k = k - 1) begin ch = s[8*k +: 8]; if (ch != 0) toma(ch); end
end endtask

// --------------------------------------------- the screen, through top ---

function [6:0] at; input integer r, c; integer m; begin
	m = r + dut.top; if (m >= 30) m = m - 30;
	at = dut.smem[m*80 + c];
end endfunction
task want_text; input integer r, c; input [8*20-1:0] s; input integer n; integer k; reg [7:0] e; begin
	for (k = 0; k < n; k = k + 1) begin
		e = s[8*(n-1-k) +: 8];
		if (at(r, c + k) !== e[6:0]) begin
			errs = errs + 1;
			if (errs < 12) $display("  screen (%0d,%0d): %0d, want '%c'", r, c + k, at(r, c + k), e);
		end
	end
end endtask
task want_cursor; input integer r, c; begin
	if (dut.crow !== r || dut.ccol !== c) begin
		errs = errs + 1; $display("  cursor at (%0d,%0d), want (%0d,%0d)", dut.crow, dut.ccol, r, c);
	end
end endtask
task want_blank_line; input integer r; integer k; begin
	for (k = 0; k < 80; k = k + 1) if (at(r, k) !== 7'd32) begin
		errs = errs + 1; if (errs < 12) $display("  screen (%0d,%0d) not blank: %0d", r, k, at(r, k)); k = 80;
	end
end endtask

// ------------------------------------------- the picture, as a monitor ---

reg pic [0:639][0:479];                      // what the monitor shows
integer x = 0, y = 0, frame_done = 0;
reg ativo_d = 0, vs_d = 1;
// timing as seen on the pins
integer hcnt = 0, hlow = 0, hper = 0, h_from_vis = 0, vis_w = 0, vis_lines = 0;
integer vlines_low = 0, lines_since = 0;
reg hs_d = 1;

always @ (posedge clk) if (!rst) begin
	if (ativo) begin pic[x][y] = pixel; x = x + 1; end
	if (ativo_d && !ativo) begin vis_w = x; x = 0; y = y + 1; h_from_vis = 0; end
	else h_from_vis = h_from_vis + 1;
	if (!hs && hs_d) begin                   // hs falls
		hper = hcnt; hcnt = 0;
		if (h_from_vis != 16 && y > 0 && y < 480) begin errs = errs + 1; $display("  hs falls %0d pixels after the visible line, want 16", h_from_vis); end
	end else hcnt = hcnt + 1;
	if (!hs) hlow = hlow + 1;
	if (hs && !hs_d && hlow != 96) begin errs = errs + 1; $display("  hs low %0d, want 96", hlow); end
	if (hs && !hs_d) hlow = 0;
	if (!vs && vs_d) begin vis_lines = y; y = 0; frame_done = frame_done + 1; end
	ativo_d = ativo; vs_d = vs; hs_d = hs;
end

// rows 0-15 of 'A' in Modern DOS 8x16, copied from the font by hand
reg [7:0] glyph_A [0:15];
initial begin
	glyph_A[0]=8'h00; glyph_A[1]=8'h00; glyph_A[2]=8'h38; glyph_A[3]=8'h6C;
	glyph_A[4]=8'hC6; glyph_A[5]=8'hC6; glyph_A[6]=8'hC6; glyph_A[7]=8'hFE;
	glyph_A[8]=8'hC6; glyph_A[9]=8'hC6; glyph_A[10]=8'hC6; glyph_A[11]=8'hC6;
	glyph_A[12]=8'h00; glyph_A[13]=8'h00; glyph_A[14]=8'h00; glyph_A[15]=8'h00;
end

task check_cell_A; input integer r, c; integer i, j; reg [7:0] b; begin
	for (j = 0; j < 16; j = j + 1) begin
		for (i = 0; i < 8; i = i + 1) b[7-i] = pic[c*8 + i][r*16 + j];
		if (b !== glyph_A[j]) begin errs = errs + 1; $display("  'A' at (%0d,%0d), pixel row %0d: %h, want %h", r, c, j, b, glyph_A[j]); end
	end
end endtask
function [7:0] cell_row; input integer r, c, j; integer i; begin
	for (i = 0; i < 8; i = i + 1) cell_row[7-i] = pic[c*8 + i][r*16 + j];
end endfunction

task one_frame; integer f; begin f = frame_done; while (frame_done < f + 2) @(posedge clk); end endtask

// ------------------------------------------------------------------- run ---

integer k;
initial begin
	#200 rst = 0;
	repeat (2500) @(posedge clk);            // reset clears the screen
	want_blank_line(0); want_blank_line(29); want_cursor(0, 0);

	say("Hello");                             want_text(0, 0, "Hello", 5); want_cursor(0, 5);
	if (wait_cycles > 4) begin errs = errs + 1; $display("  a character held the SAPHO %0d cycles", wait_cycles); end
	toma(10); say("ab"); toma(8);             want_text(1, 0, "a ", 2); want_cursor(1, 1);
	toma(9);                                  want_cursor(1, 8);
	toma(129); toma(5); toma(10); toma(65);   want_text(5, 10, "A", 1); want_cursor(5, 11);
	toma(129); toma(40); toma(90);            want_cursor(0, 0);
	toma(0); toma(27); toma(127); toma(200);  want_cursor(0, 0); want_text(0, 0, "Hello", 5);
	toma(129); toma(1); toma(78); toma(8'h58); toma(8'h59);   // 'X' at 78, 'Y' at 79: past 79 goes to the next line
	want_text(1, 78, "XY", 2); want_cursor(2, 0);
	toma(129); toma(3); toma(75); toma(9);   want_cursor(4, 0);   // tab past column 72: next line

	// the picture: 'A' at (5,10), the cursor's underline blinking at (5,11)
	toma(129); toma(5); toma(11);
	force dut.frames = 5'h10; one_frame;
	check_cell_A(5, 10);
	if (cell_row(5, 11, 14) !== 8'hFF || cell_row(5, 11, 15) !== 8'hFF) begin errs = errs + 1; $display("  the cursor's underline is not lit"); end
	force dut.frames = 5'h00; one_frame;
	if (cell_row(5, 11, 14) !== 8'h00) begin errs = errs + 1; $display("  the cursor's underline does not blink off"); end
	release dut.frames;
	if (vis_w !== 640)   begin errs = errs + 1; $display("  a visible line has %0d pixels, want 640", vis_w); end
	if (vis_lines !== 480) begin errs = errs + 1; $display("  a frame has %0d visible lines, want 480", vis_lines); end
	if (hper !== 799)    begin errs = errs + 1; $display("  hs period %0d, want 800", hper + 1); end

	// scroll: a new line from the last screen line
	toma(129); toma(29); toma(0); say("last");
	toma(129); toma(0); toma(0); say("first");
	toma(129); toma(29); toma(4); toma(10);
	if (dut.top !== 1) begin errs = errs + 1; $display("  top %0d after a scroll, want 1", dut.top); end
	want_text(28, 0, "last", 4); want_blank_line(29); want_cursor(29, 0);
	if (wait_cycles < 80) begin errs = errs + 1; $display("  a scroll held the SAPHO %0d cycles, want at least 80", wait_cycles); end
	say("new"); want_text(29, 0, "new", 3);

	// clear
	toma(128);
	if (wait_cycles < 2400) begin errs = errs + 1; $display("  a clear held the SAPHO %0d cycles, want at least 2400", wait_cycles); end
	want_blank_line(0); want_blank_line(28); want_blank_line(29); want_cursor(0, 0);
	if (dut.top !== 0) begin errs = errs + 1; $display("  top %0d after a clear, want 0", dut.top); end

	if (errs == 0) $display("NUBITS=%0d: ok", NUBITS);
	else           $display("NUBITS=%0d: FAIL, %0d errors", NUBITS, errs);
	$finish;
end

endmodule
