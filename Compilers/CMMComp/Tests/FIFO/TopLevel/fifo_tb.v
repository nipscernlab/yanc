`timescale 1ns/1ps
// Project pass FIFO (Scripts/regress.sh): a SAPHO (fifo_w) writes into
// SAPHO/myFIFO.v through out()/out_en, wires only (wrreq = out_en, data =
// out). Once the writer is done the testbench drains the FIFO into
// output_fifo.txt; regress wants 1..8, 11..14 (docs/design/myfifo.md).
module fifo_tb;
reg clk = 0; always #10 clk = ~clk;
reg rst = 1;
wire signed [15:0] out;
wire [0:0] out_en;
fifo_w P (.clk(clk), .rst(rst), .out(out), .out_en(out_en));

reg rd = 0;
wire ae, empty, full;
wire [15:0] q;
wire [6:0] usedw;
myFIFO #(.WORD(16), .LENGTH(128), .ALMOST(2)) F (.clk(clk), .data(out), .rdreq(rd),
    .sclr(rst), .wrreq(out_en[0]), .almost_empty(ae), .empty(empty), .full(full), .q(q), .usedw(usedw));

integer fd, n = 0;
initial begin
    fd = $fopen("output_fifo.txt", "w");
    #95 rst = 0;
    #20000;                                 // the writer is done long before
    while (!empty && n < 128) begin
        @(negedge clk); $fdisplay(fd, "%0d", q); rd = 1; @(negedge clk); rd = 0; n = n + 1;
    end
    $fclose(fd);
    $finish;
end
endmodule
