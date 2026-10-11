`timescale 1ns/1ps
// Project pass LINK3 (Scripts/regress.sh): several links per processor
// (docs/toma-and-cade-links.md), on two clocks of unrelated periods.
//   chain  : lk_chain_a (20 ns) -> lk_chain_b (20 ns) -> lk_chain_c (27.3 ns);
//            A->B on one clock with the _sync forms, B->C synchronized
//   chain2 : the same with the C++ middle lk_cpp_chain_b
//   fan-out: lk_fan_w (20 ns) hands n on link 0 to lk_echo E (20 ns) and n+100
//            on link 1 to lk_echo F (27.3 ns)
//   fan-in : lk_merge (20 ns) takes link 0 from lk_count (27.3 ns) and link 1
//            from lk_count7 (20 ns), puts a*1000 + b
//   float  : lk_flt_w (20 ns) hands n*1.5 as float words, lk_flt_r (27.3 ns)
//            takes them with fcade and puts 3n
// Wiring of writer W's link i to reader R's link j, wires only:
//   W.toma_i -> R.taqui_j,  R.cade_j -> W.valeu_i,  W.toma_dado -> R.cade_dado_j
// regress checks each output against the sequence it computes.
module link3_tb;

reg clk_f = 0; always #10    clk_f = ~clk_f;   // 20 ns
reg clk_s = 0; always #13.65 clk_s = ~clk_s;   // 27.3 ns

// one reset source, released in each domain through two flip-flops
reg  rst = 1;
reg  rf1 = 1, rf2 = 1, rs1 = 1, rs2 = 1;
always @(posedge clk_f) begin rf1 <= rst; rf2 <= rf1; end
always @(posedge clk_s) begin rs1 <= rst; rs2 <= rs1; end
wire rst_f = rf2, rst_s = rs2;

// ---- chain: A -> B (one clock, _sync) -> C (another clock)
wire [31:0] a_dado, b_dado, c_out, a_out, b_out;
wire        a_t0, b_c0, b_t1, c_c0;
wire [0:0]  c_en;
lk_chain_a A (.clk(clk_f), .rst(rst_f), .out(a_out), .toma_0(a_t0), .valeu_0(b_c0), .toma_dado(a_dado));
lk_chain_b B (.clk(clk_f), .rst(rst_f), .out(b_out),
              .cade_0(b_c0), .taqui_0(a_t0), .cade_dado_0(a_dado),
              .toma_1(b_t1), .valeu_1(c_c0), .toma_dado(b_dado));
lk_chain_c C (.clk(clk_s), .rst(rst_s), .out(c_out), .out_en(c_en),
              .cade_0(c_c0), .taqui_0(b_t1), .cade_dado_0(b_dado));

// ---- chain2: the same with the C++ middle
wire [31:0] a2_dado, b2_dado, c2_out, a2_out, b2_out;
wire        a2_t0, b2_c0, b2_t1, c2_c0;
wire [0:0]  c2_en;
lk_chain_a     A2 (.clk(clk_f), .rst(rst_f), .out(a2_out), .toma_0(a2_t0), .valeu_0(b2_c0), .toma_dado(a2_dado));
lk_cpp_chain_b B2 (.clk(clk_f), .rst(rst_f), .out(b2_out),
                   .cade_0(b2_c0), .taqui_0(a2_t0), .cade_dado_0(a2_dado),
                   .toma_1(b2_t1), .valeu_1(c2_c0), .toma_dado(b2_dado));
lk_chain_c     C2 (.clk(clk_s), .rst(rst_s), .out(c2_out), .out_en(c2_en),
                   .cade_0(c2_c0), .taqui_0(b2_t1), .cade_dado_0(b2_dado));

// ---- fan-out: D -> E (link 0, one clock), D -> F (link 1, another clock)
wire [31:0] d_dado, d_out, e_out, f_out;
wire        d_t0, d_t1, e_c0, f_c0;
wire [0:0]  e_en, f_en;
lk_fan_w D (.clk(clk_f), .rst(rst_f), .out(d_out),
            .toma_0(d_t0), .valeu_0(e_c0), .toma_1(d_t1), .valeu_1(f_c0), .toma_dado(d_dado));
lk_echo  E (.clk(clk_f), .rst(rst_f), .out(e_out), .out_en(e_en), .cade_0(e_c0), .taqui_0(d_t0), .cade_dado_0(d_dado));
lk_echo  F (.clk(clk_s), .rst(rst_s), .out(f_out), .out_en(f_en), .cade_0(f_c0), .taqui_0(d_t1), .cade_dado_0(d_dado));

// ---- fan-in: H (another clock) -> G link 0, I (one clock) -> G link 1
wire [31:0] h_dado, i_dado, g_out, h_out, i_out;
wire        h_t0, i_t0, g_c0, g_c1;
wire [0:0]  g_en;
lk_count  H (.clk(clk_s), .rst(rst_s), .out(h_out), .toma_0(h_t0), .valeu_0(g_c0), .toma_dado(h_dado));
lk_count7 I (.clk(clk_f), .rst(rst_f), .out(i_out), .toma_0(i_t0), .valeu_0(g_c1), .toma_dado(i_dado));
lk_merge  G (.clk(clk_f), .rst(rst_f), .out(g_out), .out_en(g_en),
             .cade_0(g_c0), .taqui_0(h_t0), .cade_dado_0(h_dado),
             .cade_1(g_c1), .taqui_1(i_t0), .cade_dado_1(i_dado));

// ---- float: J -> K (another clock)
wire [31:0] j_dado, j_out, k_out;
wire        j_t0, k_c0;
wire [0:0]  k_en;
lk_flt_w J (.clk(clk_f), .rst(rst_f), .out(j_out), .toma_0(j_t0), .valeu_0(k_c0), .toma_dado(j_dado));
lk_flt_r K (.clk(clk_s), .rst(rst_s), .out(k_out), .out_en(k_en), .cade_0(k_c0), .taqui_0(j_t0), .cade_dado_0(j_dado));

integer fc, fc2, fe, ff, fg, fk;
integer nc = 0, nc2 = 0, ne = 0, nf = 0, ng = 0, nk = 0;
always @(posedge clk_s) if (!rst_s && c_en[0])  begin $fdisplay(fc,  "%0d", c_out);  nc  = nc  + 1; end
always @(posedge clk_s) if (!rst_s && c2_en[0]) begin $fdisplay(fc2, "%0d", c2_out); nc2 = nc2 + 1; end
always @(posedge clk_f) if (!rst_f && e_en[0])  begin $fdisplay(fe,  "%0d", e_out);  ne  = ne  + 1; end
always @(posedge clk_s) if (!rst_s && f_en[0])  begin $fdisplay(ff,  "%0d", f_out);  nf  = nf  + 1; end
always @(posedge clk_f) if (!rst_f && g_en[0])  begin $fdisplay(fg,  "%0d", g_out);  ng  = ng  + 1; end
always @(posedge clk_s) if (!rst_s && k_en[0])  begin $fdisplay(fk,  "%0d", k_out);  nk  = nk  + 1; end

task close_all; begin $fclose(fc); $fclose(fc2); $fclose(fe); $fclose(ff); $fclose(fg); $fclose(fk); end endtask

initial begin
    fc  = $fopen("output_chain.txt",  "w");
    fc2 = $fopen("output_chain2.txt", "w");
    fe  = $fopen("output_fan0.txt",   "w");
    ff  = $fopen("output_fan1.txt",   "w");
    fg  = $fopen("output_merge.txt",  "w");
    fk  = $fopen("output_float.txt",  "w");
    #95 rst = 0;
    wait (nc == 16 && nc2 == 16 && ne == 16 && nf == 16 && ng == 16 && nk == 16);
    #1000;
    close_all;
    $finish;
end

// a link that never closes ends here: the outputs are then short
initial begin #3000000; close_all; $finish; end

endmodule
