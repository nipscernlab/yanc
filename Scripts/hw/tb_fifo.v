`timescale 1ns/1ps
// checks SAPHO/myFIFO.v against what a FIFO between two SAPHOs needs
// (docs/design/myfifo.md). Each case prints PASS or FAIL; the myFIFO.v of
// 2026-10-09 before its fix failed all three (the regress FIFO pass runs it):
//   C1 the word is the one on `data` in the wrreq cycle (a SAPHO's out bus
//      holds it for that cycle only);
//   C2 a write into a full FIFO is refused and nothing stored is lost;
//   C3 a word written into an empty FIFO can be read in the next cycle.
// Run: iverilog -g2012 -s tb_fifo -o f.vvp Scripts/hw/tb_fifo.v SAPHO/myFIFO.v && vvp -n f.vvp
module tb_fifo;
reg clk = 0; always #10 clk = ~clk;
reg sclr = 1, wrreq = 0, rdreq = 0;
reg  [15:0] data = 0;
wire ae, empty, full;
wire [15:0] q;
wire [1:0] usedw;
myFIFO #(.WORD(16), .LENGTH(4), .ALMOST(1)) F (.clk(clk), .data(data), .rdreq(rdreq),
    .sclr(sclr), .wrreq(wrreq), .almost_empty(ae), .empty(empty), .full(full), .q(q), .usedw(usedw));

reg [15:0] got [0:7];
integer n;
task drain; begin                         // reads everything into got[0..n-1]
    n = 0;
    while (!empty && n < 8) begin @(negedge clk); got[n] = q; n = n + 1; rdreq = 1; @(negedge clk); rdreq = 0; end
end endtask
task reset; begin
    @(negedge clk); sclr = 1; @(negedge clk); sclr = 0; @(negedge clk);
end endtask
// one write, data held for the wrreq cycle only (then it moves on, as a
// SAPHO's out bus does), or for two cycles (hold = 1)
task put(input [15:0] v, input hold); begin
    @(negedge clk); data = v; wrreq = 1;
    @(negedge clk); wrreq = 0; if (!hold) data = 16'hDEAD;
    @(negedge clk); data = 16'hDEAD;
end endtask

integer fails = 0;
initial begin
    repeat (2) @(negedge clk); sclr = 0;

    // C1: data valid in the wrreq cycle only
    put(5, 0); put(6, 0);
    drain;
    if (n == 2 && got[0] == 5 && got[1] == 6) $display("PASS C1 data in the wrreq cycle");
    else begin $display("FAIL C1 data in the wrreq cycle: read %0d words, first %0d %0d (want 5 6)", n, got[0], got[1]); fails = fails + 1; end

    // C2: LENGTH 4 holds 3 words; a fourth write must be refused
    reset;
    put(1, 1); put(2, 1); put(3, 1); put(4, 1);
    drain;
    if (n == 3 && got[0] == 1 && got[1] == 2 && got[2] == 3) $display("PASS C2 write into a full FIFO");
    else begin $display("FAIL C2 write into a full FIFO: read %0d words (want 1 2 3)", n); fails = fails + 1; end

    // C3: write 7 into an empty FIFO, read it in the cycle after wrreq
    reset;
    @(negedge clk); data = 7; wrreq = 1;
    @(negedge clk); wrreq = 0;                // data still 7
    got[0] = q; rdreq = !empty; @(negedge clk); rdreq = 0;
    drain;                                    // whatever is left
    if (got[0] == 7 && n == 0) $display("PASS C3 read right after a write");
    else begin $display("FAIL C3 read right after a write: q was %0d, %0d word(s) left (want 7, 0)", got[0], n); fails = fails + 1; end

    $display("%0d of 3 failed", fails);
    $finish;
end
endmodule
