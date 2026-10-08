`timescale 1ns/1ps
// Project pass LINK (Scripts/regress.sh): two pairs of SAPHOs over toma/cade
// (docs/toma-and-cade.md), wires only between them.
//   link_envia  -> link_recebe : 16 values n*7+3, speeds crossing over
//   link_ping  <-> link_pong   : one link both ways, pong returns n + 1
// Each value a reader puts on its port 0 is written to output_link.txt /
// output_pingpong.txt; regress compares them with the sim golden and with the
// sequences it computes itself.
module link_tb;

reg clk = 0; always #10 clk = ~clk;
reg rst = 1;

// envia -> recebe: A.toma -> B.taqui, B.cade -> A.valeu, A.out -> B.cade_dado
wire [31:0] e_out, r_out;
wire        e_toma, r_cade;
wire [0:0]  r_out_en;
link_envia  E (.clk(clk), .rst(rst), .out(e_out), .toma(e_toma), .valeu(r_cade));
link_recebe R (.clk(clk), .rst(rst), .out(r_out), .out_en(r_out_en),
               .cade(r_cade), .taqui(e_toma), .cade_dado(e_out));

// ping <-> pong: the same three wires each way
wire [31:0] p_out, q_out;
wire        p_toma, p_cade, q_toma, q_cade;
wire [0:0]  p_out_en;
link_ping P (.clk(clk), .rst(rst), .out(p_out), .out_en(p_out_en),
             .toma(p_toma), .valeu(q_cade), .cade(p_cade), .taqui(q_toma), .cade_dado(q_out));
link_pong Q (.clk(clk), .rst(rst), .out(q_out),
             .toma(q_toma), .valeu(p_cade), .cade(q_cade), .taqui(p_toma), .cade_dado(p_out));

integer f_link, f_pp, n_link = 0, n_pp = 0;
always @(posedge clk) if (!rst && r_out_en[0]) begin $fdisplay(f_link, "%0d", r_out); n_link = n_link + 1; end
always @(posedge clk) if (!rst && p_out_en[0]) begin $fdisplay(f_pp,   "%0d", p_out); n_pp   = n_pp   + 1; end

initial begin
    f_link = $fopen("output_link.txt", "w");
    f_pp   = $fopen("output_pingpong.txt", "w");
    #95 rst = 0;
    wait (n_link == 16 && n_pp == 16);
    #1000;
    $fclose(f_link); $fclose(f_pp);
    $finish;
end

// a link that never closes (a lost or a stuck handshake) ends here: the
// outputs are then short and the golden compare fails
initial begin #2000000; $fclose(f_link); $fclose(f_pp); $finish; end

endmodule
