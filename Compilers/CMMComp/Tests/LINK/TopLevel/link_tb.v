`timescale 1ns/1ps
// Project pass LINK (Scripts/regress.sh): two pairs of SAPHOs over toma/cade
// (docs/design/toma-and-cade.md), wires only between them.
//   link_envia  -> link_recebe : 16 values n*7+3, speeds crossing over
//   link_ping  <-> link_pong   : one link both ways, pong returns n + 1
// and the same two scenarios across the compilers (C+- <-> C++):
//   link_envia  -> link_cpp_recebe, link_cpp_ping <-> link_pong
// Each value a reader puts on its port 0 is written to output_link.txt,
// output_pingpong.txt, output_link_cpp.txt, output_pingpong_cpp.txt; regress
// compares them with the sim golden and with the sequences it computes itself.
module link_tb;

reg clk = 0; always #10 clk = ~clk;
reg rst = 1;

// envia -> recebe, link 0 of each: A.toma_0 -> B.taqui_0, B.cade_0 -> A.valeu_0,
// A.toma_dado -> B.cade_dado_0
wire [31:0] e_out, e_dado, r_out;
wire        e_toma, r_cade;
wire [0:0]  r_out_en;
link_envia  E (.clk(clk), .rst(rst), .out(e_out), .toma_0(e_toma), .valeu_0(r_cade), .toma_dado(e_dado));
link_recebe R (.clk(clk), .rst(rst), .out(r_out), .out_en(r_out_en),
               .cade_0(r_cade), .taqui_0(e_toma), .cade_dado_0(e_dado));

// ping <-> pong: the same three wires each way
wire [31:0] p_out, p_dado, q_out, q_dado;
wire        p_toma, p_cade, q_toma, q_cade;
wire [0:0]  p_out_en;
link_ping P (.clk(clk), .rst(rst), .out(p_out), .out_en(p_out_en),
             .toma_0(p_toma), .valeu_0(q_cade), .cade_0(p_cade), .taqui_0(q_toma), .cade_dado_0(q_dado), .toma_dado(p_dado));
link_pong Q (.clk(clk), .rst(rst), .out(q_out),
             .toma_0(q_toma), .valeu_0(p_cade), .cade_0(q_cade), .taqui_0(p_toma), .cade_dado_0(p_dado), .toma_dado(q_dado));

// mixed pairs: the same C+- writer and pong, with C++ partners (compiled by
// cppcomp from ../cpp/), so both directions cross from one compiler to the other
wire [31:0] e2_out, e2_dado, rc_out, pc_out, pc_dado, q2_out, q2_dado;
wire        e2_toma, rc_cade, pc_toma, pc_cade, q2_toma, q2_cade;
wire [0:0]  rc_out_en, pc_out_en;
link_envia      E2 (.clk(clk), .rst(rst), .out(e2_out), .toma_0(e2_toma), .valeu_0(rc_cade), .toma_dado(e2_dado));
link_cpp_recebe RC (.clk(clk), .rst(rst), .out(rc_out), .out_en(rc_out_en),
                    .cade_0(rc_cade), .taqui_0(e2_toma), .cade_dado_0(e2_dado));
link_cpp_ping   PC (.clk(clk), .rst(rst), .out(pc_out), .out_en(pc_out_en),
                    .toma_0(pc_toma), .valeu_0(q2_cade), .cade_0(pc_cade), .taqui_0(q2_toma), .cade_dado_0(q2_dado), .toma_dado(pc_dado));
link_pong       Q2 (.clk(clk), .rst(rst), .out(q2_out),
                    .toma_0(q2_toma), .valeu_0(pc_cade), .cade_0(q2_cade), .taqui_0(pc_toma), .cade_dado_0(pc_dado), .toma_dado(q2_dado));

integer f_link, f_pp, f_link_cpp, f_pp_cpp;
integer n_link = 0, n_pp = 0, n_link_cpp = 0, n_pp_cpp = 0;
always @(posedge clk) if (!rst && r_out_en[0])  begin $fdisplay(f_link,     "%0d", r_out);  n_link     = n_link     + 1; end
always @(posedge clk) if (!rst && p_out_en[0])  begin $fdisplay(f_pp,       "%0d", p_out);  n_pp       = n_pp       + 1; end
always @(posedge clk) if (!rst && rc_out_en[0]) begin $fdisplay(f_link_cpp, "%0d", rc_out); n_link_cpp = n_link_cpp + 1; end
always @(posedge clk) if (!rst && pc_out_en[0]) begin $fdisplay(f_pp_cpp,   "%0d", pc_out); n_pp_cpp   = n_pp_cpp   + 1; end

task close_all; begin $fclose(f_link); $fclose(f_pp); $fclose(f_link_cpp); $fclose(f_pp_cpp); end endtask

initial begin
    f_link     = $fopen("output_link.txt", "w");
    f_pp       = $fopen("output_pingpong.txt", "w");
    f_link_cpp = $fopen("output_link_cpp.txt", "w");
    f_pp_cpp   = $fopen("output_pingpong_cpp.txt", "w");
    #95 rst = 0;
    wait (n_link == 16 && n_pp == 16 && n_link_cpp == 16 && n_pp_cpp == 16);
    #1000;
    close_all;
    $finish;
end

// a link that never closes (a lost or a stuck handshake) ends here: the
// outputs are then short and the golden compare fails
initial begin #2000000; close_all; $finish; end

endmodule
