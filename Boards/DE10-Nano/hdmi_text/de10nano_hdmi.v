// DE10-Nano: a SAPHO (hdmi_demo, built by yanc from hdmi_demo.cpp) writing
// text to the HDMI output through SAPHO/myADV7513Text.v on link 1, everything on
// the 25.175 MHz pixel clock that the PLL makes from the board's 50 MHz.
// KEY[0] pressed is the reset: the screen clears, the program starts again
// and the ADV7513 is set up again 260 ms later. Pins: de10nano_hdmi.qsf, from
// the DE10-Nano user manual (Terasic), tables 3-5, 3-7 and 3-13.
module de10nano_hdmi (
	input         FPGA_CLK1_50,
	input  [0:0]  KEY,
	output [23:0] HDMI_TX_D,
	output        HDMI_TX_CLK,
	output        HDMI_TX_DE,
	output        HDMI_TX_HS,
	output        HDMI_TX_VS,
	inout         I2C_SCL,                  // to the ADV7513 only, pulled up on the board
	inout         I2C_SDA
);

// the pixel clock of 640x480 at 60 Hz: 25.175 MHz nominal, 25.1786 MHz from
// the PLL (50 x 141 / 8 / 35)
wire clk, locked;
pll_pixel u_pll (.refclk(FPGA_CLK1_50), .outclk(clk), .locked(locked));

// the reset, held until the PLL locks and released on the clock
wire      arst = ~KEY[0] | ~locked;
reg [1:0] rs   = 2'b11;
always @ (posedge clk or posedge arst)
	if (arst) rs <= 2'b11;
	else                   rs <= {rs[0], 1'b0};
wire rst = rs[1];

// the processor: toma_sync(1, x) is the screen
wire       toma_1, valeu_1;
wire [7:0] toma_dado, out;
hdmi_demo sapho (.clk(clk), .rst(rst), .out(out),
                 .toma_1(toma_1), .valeu_1(valeu_1), .toma_dado(toma_dado));

// the screen: 80 x 30 characters, and the ADV7513's set-up over I2C
wire hs, vs, ativo, pixel;
myADV7513Text #(.NUBITS(8), .LINK_SYNC(1)) tela (
	.clk(clk), .rst(rst), .toma(toma_1), .dado(toma_dado), .valeu(valeu_1),
	.hs(hs), .vs(vs), .ativo(ativo), .pixel(pixel), .scl(I2C_SCL), .sda(I2C_SDA));

// the ADV7513: D[23:16] red, D[15:8] green, D[7:0] blue (input ID 0, ADV7513
// Programming Guide, Table 16), white on black. The chip latches on the rising
// edge of its clock; the outputs change on the rising edge of clk, so the
// clock goes out inverted and the chip samples half a pixel later. The margin
// is checked by de10nano_hdmi.sdc.
assign HDMI_TX_D   = {24{pixel}};
assign HDMI_TX_DE  = ativo;
assign HDMI_TX_HS  = hs;
assign HDMI_TX_VS  = vs;
assign HDMI_TX_CLK = ~clk;

endmodule
