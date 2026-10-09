`timescale 1ns/1ps
// Project pass FIFO, second testbench: a SAPHO writer and a SAPHO reader on
// two clocks of unrelated periods (20 ns and 27.3 ns), joined by
// SAPHO/myDCFIFO.v with wires only (docs/myfifo.md):
//   writer: wrreq = out_en[0], data = out
//   reader: in() of port 1 is rdempty, in() of port 0 is q, rdreq = req_in[0]
// Two pairs: writer fast -> reader slow (output_dcfifo_fs.txt) and writer
// slow -> reader fast (output_dcfifo_sf.txt); regress wants 1..8, 11..14 in
// each. Each clock domain leaves the reset through its own two flip-flops.
module dcfifo_tb;

reg clk_f = 0; always #10    clk_f = ~clk_f;   // 20 ns
reg clk_s = 0; always #13.65 clk_s = ~clk_s;   // 27.3 ns

reg  rst = 1;
reg  rf1 = 1, rf2 = 1, rs1 = 1, rs2 = 1;
always @(posedge clk_f) begin rf1 <= rst; rf2 <= rf1; end
always @(posedge clk_s) begin rs1 <= rst; rs2 <= rs1; end
wire rst_f = rf2, rst_s = rs2;

// pair 1: writer on clk_f, reader on clk_s
wire signed [15:0] w1_out, r1_out, r1_in;
wire [0:0] w1_en, r1_en;
wire [1:0] r1_req;
wire [15:0] q1;
wire full1, empty1, ae1;
wire [4:0] wu1, ru1;
fifo_w W1 (.clk(clk_f), .rst(rst_f), .out(w1_out), .out_en(w1_en));
myDCFIFO #(.WORD(16), .LENGTH(16), .ALMOST(1)) D1 (.aclr(rst),
    .wrclk(clk_f), .data(w1_out), .wrreq(w1_en[0]), .wrfull(full1), .wrusedw(wu1),
    .rdclk(clk_s), .rdreq(r1_req[0]), .rdempty(empty1), .rdalmost_empty(ae1), .q(q1), .rdusedw(ru1));
assign r1_in = r1_req[1] ? {15'd0, empty1} : q1;
fifo_r R1 (.clk(clk_s), .rst(rst_s), .in(r1_in), .req_in(r1_req), .out(r1_out), .out_en(r1_en));

// pair 2: writer on clk_s, reader on clk_f
wire signed [15:0] w2_out, r2_out, r2_in;
wire [0:0] w2_en, r2_en;
wire [1:0] r2_req;
wire [15:0] q2;
wire full2, empty2, ae2;
wire [4:0] wu2, ru2;
fifo_w W2 (.clk(clk_s), .rst(rst_s), .out(w2_out), .out_en(w2_en));
myDCFIFO #(.WORD(16), .LENGTH(16), .ALMOST(1)) D2 (.aclr(rst),
    .wrclk(clk_s), .data(w2_out), .wrreq(w2_en[0]), .wrfull(full2), .wrusedw(wu2),
    .rdclk(clk_f), .rdreq(r2_req[0]), .rdempty(empty2), .rdalmost_empty(ae2), .q(q2), .rdusedw(ru2));
assign r2_in = r2_req[1] ? {15'd0, empty2} : q2;
fifo_r R2 (.clk(clk_f), .rst(rst_f), .in(r2_in), .req_in(r2_req), .out(r2_out), .out_en(r2_en));

integer f_fs, f_sf, n_fs = 0, n_sf = 0;
always @(posedge clk_s) if (!rst_s && r1_en[0]) begin $fdisplay(f_fs, "%0d", r1_out); n_fs = n_fs + 1; end
always @(posedge clk_f) if (!rst_f && r2_en[0]) begin $fdisplay(f_sf, "%0d", r2_out); n_sf = n_sf + 1; end

task close_all; begin $fclose(f_fs); $fclose(f_sf); end endtask

initial begin
    f_fs = $fopen("output_dcfifo_fs.txt", "w");
    f_sf = $fopen("output_dcfifo_sf.txt", "w");
    #95 rst = 0;
    wait (n_fs >= 12 && n_sf >= 12);
    #2000;                                  // anything extra would show up here
    close_all;
    $finish;
end

// a pair that never delivers its 12 words ends here: the check then fails
initial begin #400000; close_all; $finish; end

endmodule
