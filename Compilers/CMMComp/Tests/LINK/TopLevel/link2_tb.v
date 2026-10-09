`timescale 1ns/1ps
// Project pass LINK, second testbench: the same programs as link_tb.v, each
// pair on two clocks of unrelated periods (20 ns and 27.3 ns), every
// processor with LINKAS = 1 (docs/toma-and-cade.md section 8): the partner's
// bit enters through two flip-flops, the writer's value comes from toma_dado.
//   envia (fast) -> recebe (slow) : output_link2_fs.txt
//   envia (slow) -> recebe (fast) : output_link2_sf.txt
//   ping  (slow) <-> pong (fast)  : output_pingpong2.txt
// regress checks each against the sequences it computes (n*7+3, n+1).
module link2_tb;

reg clk_f = 0; always #10    clk_f = ~clk_f;   // 20 ns
reg clk_s = 0; always #13.65 clk_s = ~clk_s;   // 27.3 ns

// one reset source; each clock domain releases it through its own two
// flip-flops (the reset rule of section 8)
reg  rst = 1;
reg  rf1 = 1, rf2 = 1, rs1 = 1, rs2 = 1;
always @(posedge clk_f) begin rf1 <= rst; rf2 <= rf1; end
always @(posedge clk_s) begin rs1 <= rst; rs2 <= rs1; end
wire rst_f = rf2, rst_s = rs2;

// envia (fast) -> recebe (slow): A.toma -> B.taqui, B.cade -> A.valeu,
// A.toma_dado -> B.cade_dado
wire [31:0] e1_out, e1_dado, r1_out;
wire        e1_toma, r1_cade;
wire [0:0]  r1_out_en;
link_envia  #(.LINKAS(1)) E1 (.clk(clk_f), .rst(rst_f), .out(e1_out), .toma(e1_toma), .valeu(r1_cade), .toma_dado(e1_dado));
link_recebe #(.LINKAS(1)) R1 (.clk(clk_s), .rst(rst_s), .out(r1_out), .out_en(r1_out_en),
                              .cade(r1_cade), .taqui(e1_toma), .cade_dado(e1_dado));

// envia (slow) -> recebe (fast)
wire [31:0] e2_out, e2_dado, r2_out;
wire        e2_toma, r2_cade;
wire [0:0]  r2_out_en;
link_envia  #(.LINKAS(1)) E2 (.clk(clk_s), .rst(rst_s), .out(e2_out), .toma(e2_toma), .valeu(r2_cade), .toma_dado(e2_dado));
link_recebe #(.LINKAS(1)) R2 (.clk(clk_f), .rst(rst_f), .out(r2_out), .out_en(r2_out_en),
                              .cade(r2_cade), .taqui(e2_toma), .cade_dado(e2_dado));

// ping (slow) <-> pong (fast): the same three wires each way
wire [31:0] p_out, p_dado, q_out, q_dado;
wire        p_toma, p_cade, q_toma, q_cade;
wire [0:0]  p_out_en;
link_ping #(.LINKAS(1)) P (.clk(clk_s), .rst(rst_s), .out(p_out), .out_en(p_out_en),
                           .toma(p_toma), .valeu(q_cade), .cade(p_cade), .taqui(q_toma),
                           .cade_dado(q_dado), .toma_dado(p_dado));
link_pong #(.LINKAS(1)) Q (.clk(clk_f), .rst(rst_f), .out(q_out),
                           .toma(q_toma), .valeu(p_cade), .cade(q_cade), .taqui(p_toma),
                           .cade_dado(p_dado), .toma_dado(q_dado));

integer f_fs, f_sf, f_pp;
integer n_fs = 0, n_sf = 0, n_pp = 0;
always @(posedge clk_s) if (!rst_s && r1_out_en[0]) begin $fdisplay(f_fs, "%0d", r1_out); n_fs = n_fs + 1; end
always @(posedge clk_f) if (!rst_f && r2_out_en[0]) begin $fdisplay(f_sf, "%0d", r2_out); n_sf = n_sf + 1; end
always @(posedge clk_s) if (!rst_s && p_out_en[0])  begin $fdisplay(f_pp, "%0d", p_out);  n_pp = n_pp + 1; end

task close_all; begin $fclose(f_fs); $fclose(f_sf); $fclose(f_pp); end endtask

initial begin
    f_fs = $fopen("output_link2_fs.txt", "w");
    f_sf = $fopen("output_link2_sf.txt", "w");
    f_pp = $fopen("output_pingpong2.txt", "w");
    #95 rst = 0;
    wait (n_fs == 16 && n_sf == 16 && n_pp == 16);
    #1000;
    close_all;
    $finish;
end

// a link that never closes (a lost or a stuck handshake) ends here: the
// outputs are then short and the sequence check fails
initial begin #3000000; close_all; $finish; end

endmodule
