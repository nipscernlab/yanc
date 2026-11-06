// ----------------------------------------------------------------------------
// Text screen through an ADV7513, written by the SAPHO with toma() ----------
// ----------------------------------------------------------------------------
// The text screen of myVGAText (80 x 30 characters, 640x480 at 60 Hz, the
// same words, the same toma link, the same pins) plus the I2C writes that set
// up an ADV7513 HDMI transmitter, as on the DE10-Nano. A program that writes
// to myVGAText writes to this block unchanged: the top swaps the module and
// adds the two I2C pins (docs/design/adv7513-text.md). The block is tied to
// that chip: the writes below are its registers, and it sends DVI (picture
// only) on the HDMI connector. Another transmitter needs another block.
//
// The screen is myVGAText itself, instantiated below: one copy of the font,
// the words and the scan for both blocks.
//
// The ADV7513 is set once after reset to take 24-bit RGB 4:4:4 with separate
// syncs and send it as DVI, which is its default (ADV7513 Programming Guide,
// Rev. B, section 3 and Table 14). The eleven writes, in order:
//   0xD6 = 0xC0   HPD taken as always high: the chip powers up and keeps its
//                 registers with no monitor or with the cable pulled
//   0x41 = 0x10   power up (bit 6 = 0)
//   0x98 = 0x03, 0x9A = 0xE0, 0x9C = 0x30, 0x9D = 0x61, 0xA2 = 0xA4,
//   0xA3 = 0xA4, 0xE0 = 0xD0, 0xF9 = 0x00
//                 the fixed registers the guide says must be set after power up
//   0x16 = 0x30   8 bits a colour (its default, 00, is "invalid")
// Every other setting is the chip's default: input ID 0 (0x15), syncs passed
// through and 4:3 (0x17), no colour conversion (0x18), DVI with HDCP off
// (0xAF). Dênis's project (2026-11-05) wrote those four too, with values
// equal to the defaults except 0xAF bit 4 (HDCP frame encryption, which does
// nothing with HDCP off).
//
// The I2C is timed by the scan's lines, not by a divider: one quarter of an
// I2C bit per line (31.8 us at 25.175 MHz), so SCL runs at 7.9 kHz (I2C has no
// lower limit) and every setup and hold time is about 32 us. The writes start
// 2^WAIT_BITS lines after reset (2^13 lines = 260 ms; the chip needs 200 ms
// after its supply comes up) and take 11 x 32 x 4 lines = 45 ms. The ACK is not
// read: nothing could be done about a missing one but write again, and the
// picture shows whether the chip took the writes. SCL and SDA are open drain;
// the board pulls them up.
//
// One clock, the 25 MHz pixel clock, as myVGAText. A board's top maps the
// outputs to the ADV7513 (DE10-Nano, Boards/DE10-Nano/hdmi_text/):
//   assign HDMI_TX_D  = {24{pixel}};  (pixel is 0 outside the visible window)
//   assign HDMI_TX_DE = ativo;   assign HDMI_TX_HS = hs;   assign HDMI_TX_VS = vs;
//   assign HDMI_TX_CLK = ~clk;   (the chip latches on the rising edge: mid-pixel)
//
// Wiring, link k:
//   myADV7513Text hdmi (.clk(clk25), .rst(rst), .toma(toma_k), .dado(toma_dado),
//       .valeu(valeu_k), .hs(hs), .vs(vs), .ativo(ativo), .pixel(pixel),
//       .scl(I2C_SCL), .sda(I2C_SDA));
//   with the SAPHO on clk25 too: myADV7513Text #(.LINK_SYNC(1)) hdmi (...);
// Checked by Scripts/hw/tb_adv7513text.v.

module myADV7513Text
#(
	parameter NUBITS    = 32,               // width of the word toma() sends (at least 8)
	parameter LINK_SYNC = 0,                // 1: the SAPHO on this block's clock (toma_sync)
	parameter WAIT_BITS = 13                // the writes start 2^WAIT_BITS lines after reset
	                                        // (at least 11); lowered only by testbenches
)
(
	input               clk,                // the 25 MHz pixel clock
	input               rst,
	input               toma,               // from the SAPHO's toma_k: its T bit
	input  [NUBITS-1:0] dado,               // from the SAPHO's toma_dado
	output              valeu,              // to the SAPHO's valeu_k: the block's C bit
	output              hs,                 // horizontal sync, active low
	output              vs,                 // vertical sync, active low
	output              ativo,              // 1 in the visible 640x480 window (the DE)
	output              pixel,              // 1 = lit, 0 outside the window
	inout               scl,                // I2C to the ADV7513, open drain
	inout               sda
);

myVGAText #(.NUBITS(NUBITS), .LINK_SYNC(LINK_SYNC)) tela (
	.clk(clk), .rst(rst), .toma(toma), .dado(dado), .valeu(valeu),
	.hs(hs), .vs(vs), .ativo(ativo), .pixel(pixel));

// ----------------------------------------------------------------------------
// the ADV7513's writes: {register, value} ------------------------------------
// ----------------------------------------------------------------------------

localparam [3:0] NREGS = 4'd11;

function [15:0] cfg; input [3:0] n;
	case (n)
		4'd0:    cfg = 16'hD6_C0;
		4'd1:    cfg = 16'h41_10;
		4'd2:    cfg = 16'h98_03;
		4'd3:    cfg = 16'h9A_E0;
		4'd4:    cfg = 16'h9C_30;
		4'd5:    cfg = 16'h9D_61;
		4'd6:    cfg = 16'hA2_A4;
		4'd7:    cfg = 16'hA3_A4;
		4'd8:    cfg = 16'hE0_D0;
		4'd9:    cfg = 16'hF9_00;
		default: cfg = 16'h16_30;           // 10
	endcase
endfunction

// ----------------------------------------------------------------------------
// one counter, one step a line: the wait, then {write, slot, quarter} --------
// ----------------------------------------------------------------------------
// After the wait (c[WAIT_BITS] = 1), write n = c[10:7] takes 32 slots of four
// quarters: slot 0 the START, 1-27 the bits of {0x72, ACK, register, ACK,
// value, ACK}, 28 the STOP, 29-31 the bus free. The counter stops at write 11.

reg              hs_d = 1'b1;
reg [WAIT_BITS:0] c   = 0;
wire       [3:0] n    = c[10:7];
wire       [4:0] s    = c[6:2];
wire       [1:0] q    = c[1:0];
wire             fim  = c[WAIT_BITS] & (n == NREGS);
wire      [15:0] rv   = cfg(n);
wire      [26:0] bits = {8'h72, 1'b1, rv[15:8], 1'b1, rv[7:0], 1'b1};   // 1 = released

always @ (posedge clk) begin
	hs_d <= hs;
	if (rst)                c <= 0;
	else if (hs_d & ~hs & ~fim) c <= c + 1'b1;   // the start of each sync pulse: once a line
end

reg scl_r = 1'b1, sda_r = 1'b1;             // 1 = released, 0 = pulled low

always @ (posedge clk)
	if (~c[WAIT_BITS] | fim) {scl_r, sda_r} <= 2'b11;
	else if (s == 5'd0)      {scl_r, sda_r} <= {~q[1], q == 2'd0};           // START: SDA falls, SCL high
	else if (s <= 5'd27)     {scl_r, sda_r} <= {q[1] ^ q[0], bits[5'd27 - s]};   // SDA set, SCL high in q1-q2
	else if (s == 5'd28)     {scl_r, sda_r} <= {q != 2'd0, q[1]};            // STOP: SDA rises, SCL high
	else                     {scl_r, sda_r} <= 2'b11;

assign scl = scl_r ? 1'bz : 1'b0;
assign sda = sda_r ? 1'bz : 1'b0;

endmodule
