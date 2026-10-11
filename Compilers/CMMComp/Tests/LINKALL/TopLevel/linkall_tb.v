`timescale 1ns/1ps
// Project pass LINKALL (Scripts/regress.sh): the whole processor (sapho_all)
// over toma/cade, and alone with its output fed back.
//   pair 1: sapho_all_a -> sapho_all_b, one clock (20 ns)
//   pair 2: sapho_all_a (20 ns) -> sapho_all_b (27.3 ns), the link synchronized,
//           the reset released in each domain through two flip-flops
//   alone : sapho_all with its input = its last output XOR 7 (starts at 7)
// A reads 7 on its port and hands its 4-bit result (model.py: 4) with toma;
// B's port carries A's last output and cade takes the same word (B checks
// that they match), so B shows model.py(4) = 10. Each port value comes from a
// register loaded on out_en, as a board would hold it for the LEDs.
// Outputs: output_a1/b1 (pair 1), output_a2/b2 (pair 2), output_fb (alone);
// regress checks every line against model.py.
module linkall_tb;

reg clk_f = 0; always #10    clk_f = ~clk_f;   // 20 ns
reg clk_s = 0; always #13.65 clk_s = ~clk_s;   // 27.3 ns

reg  rst = 1;
reg  rf1 = 1, rf2 = 1, rs1 = 1, rs2 = 1;
always @(posedge clk_f) begin rf1 <= rst; rf2 <= rf1; end
always @(posedge clk_s) begin rs1 <= rst; rs2 <= rs1; end
wire rst_f = rf2, rst_s = rs2;

// ---- pair 1, one clock
wire signed [31:0] a1_out, a1_dado, b1_out;
wire        a1_toma, b1_cade;
wire [0:0]  a1_en, b1_en;
reg  signed [31:0] a1_q = 0, b1_q = 0;
always @(posedge clk_f) if (a1_en[0]) a1_q <= a1_out;
always @(posedge clk_f) if (b1_en[0]) b1_q <= b1_out;
sapho_all_a A1 (.clk(clk_f), .rst(rst_f), .in(32'sd7), .out(a1_out), .out_en(a1_en), .itr(1'b0),
                .toma_0(a1_toma), .valeu_0(b1_cade), .toma_dado(a1_dado));
sapho_all_b B1 (.clk(clk_f), .rst(rst_f), .in(a1_q), .out(b1_out), .out_en(b1_en), .itr(1'b0),
                .cade_0(b1_cade), .taqui_0(a1_toma), .cade_dado_0(a1_dado));

// ---- pair 2, two clocks
wire signed [31:0] a2_out, a2_dado, b2_out;
wire        a2_toma, b2_cade;
wire [0:0]  a2_en, b2_en;
reg  signed [31:0] a2_q = 0, b2_q = 0;
always @(posedge clk_f) if (a2_en[0]) a2_q <= a2_out;
always @(posedge clk_s) if (b2_en[0]) b2_q <= b2_out;
sapho_all_a A2 (.clk(clk_f), .rst(rst_f), .in(32'sd7), .out(a2_out), .out_en(a2_en), .itr(1'b0),
                              .toma_0(a2_toma), .valeu_0(b2_cade), .toma_dado(a2_dado));
sapho_all_b B2 (.clk(clk_s), .rst(rst_s), .in(a2_q), .out(b2_out), .out_en(b2_en), .itr(1'b0),
                              .cade_0(b2_cade), .taqui_0(a2_toma), .cade_dado_0(a2_dado));

// ---- alone, output fed back
wire signed [31:0] f_out;
wire [0:0]  f_en;
reg  signed [31:0] f_q = 0;
always @(posedge clk_f) if (f_en[0]) f_q <= f_out;
sapho_all F (.clk(clk_f), .rst(rst_f), .in(f_q ^ 32'sd7), .out(f_out), .out_en(f_en), .itr(1'b0));

integer fa1, fb1, fa2, fb2, ffb;
integer na1 = 0, nb1 = 0, na2 = 0, nb2 = 0, nfb = 0;
always @(posedge clk_f) if (!rst_f && a1_en[0]) begin $fdisplay(fa1, "%0d", a1_out); na1 = na1 + 1; end
always @(posedge clk_f) if (!rst_f && b1_en[0]) begin $fdisplay(fb1, "%0d", b1_out); nb1 = nb1 + 1; end
always @(posedge clk_f) if (!rst_f && a2_en[0]) begin $fdisplay(fa2, "%0d", a2_out); na2 = na2 + 1; end
always @(posedge clk_s) if (!rst_s && b2_en[0]) begin $fdisplay(fb2, "%0d", b2_out); nb2 = nb2 + 1; end
always @(posedge clk_f) if (!rst_f && f_en[0])  begin $fdisplay(ffb, "%0d", f_out);  nfb = nfb + 1; end

task close_all; begin $fclose(fa1); $fclose(fb1); $fclose(fa2); $fclose(fb2); $fclose(ffb); end endtask

initial begin
    fa1 = $fopen("output_a1.txt", "w");
    fb1 = $fopen("output_b1.txt", "w");
    fa2 = $fopen("output_a2.txt", "w");
    fb2 = $fopen("output_b2.txt", "w");
    ffb = $fopen("output_fb.txt", "w");
    #95 rst = 0;
    wait (nb1 >= 12 && nb2 >= 12 && nfb >= 12);
    #1000;
    close_all;
    $finish;
end

// a stuck link ends here: the outputs are then short and the check fails
initial begin #5000000; close_all; $finish; end

endmodule
