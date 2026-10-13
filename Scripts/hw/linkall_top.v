// Two SAPHOs on two clocks, joined by one synchronized toma/cade link: the
// LINKALL pair (docs/design/toma-and-cade.md 8.6) as a board would hold it, the top
// timed by fmax_link.sh (Quartus) and vfmax_link.sh (Vivado). clk_a and clk_b
// from two pins, the reset released in each domain through two flip-flops,
// A's input from four switches, B's input port fed by A's last output (a
// register on clk_a, as linkall_tb.v does), B's last output on four LEDs.
module linkall_top (
    input        clk_a, clk_b, rst,
    input  [3:0] sw,
    output [3:0] led
);

reg ra1 = 1, ra2 = 1, rb1 = 1, rb2 = 1;
always @(posedge clk_a) begin ra1 <= rst; ra2 <= ra1; end
always @(posedge clk_b) begin rb1 <= rst; rb2 <= rb1; end

wire signed [31:0] a_out, a_dado, b_out;
wire               a_toma, b_cade;
wire [0:0]         a_en, b_en;
reg  signed [31:0] a_q = 0;
reg         [3:0]  b_q = 0;
always @(posedge clk_a) if (a_en[0]) a_q <= a_out;
always @(posedge clk_b) if (b_en[0]) b_q <= b_out[3:0];

sapho_all_a A (.clk(clk_a), .rst(ra2), .in({28'd0, sw}), .out(a_out), .req_in(), .out_en(a_en),
               .itr(1'b0), .cheguei(), .toma_0(a_toma), .valeu_0(b_cade), .toma_dado(a_dado));
sapho_all_b B (.clk(clk_b), .rst(rb2), .in(a_q), .out(b_out), .req_in(), .out_en(b_en),
               .itr(1'b0), .cheguei(), .cade_0(b_cade), .taqui_0(a_toma), .cade_dado_0(a_dado));

assign led = b_q;

endmodule
