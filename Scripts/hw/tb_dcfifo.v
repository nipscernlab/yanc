`timescale 1ns/1ps
// checks SAPHO/myDCFIFO.v (a FIFO between two clock domains, docs/myfifo.md).
// WH / RH: half periods of wrclk / rdclk; run it both ways round:
//   iverilog -g2012 -s tb_dcfifo -o d.vvp Scripts/hw/tb_dcfifo.v SAPHO/myDCFIFO.v && vvp -n d.vvp
//   iverilog -g2012 -s tb_dcfifo -Ptb_dcfifo.WH=13.65 -Ptb_dcfifo.RH=10 -o d.vvp ... && vvp -n d.vvp
// Each case prints PASS or FAIL, then "<n> of 4 failed":
//   C1 the word is the one on `data` in the wrreq cycle;
//   C2 LENGTH words fit, a write into a full FIFO is refused, nothing lost;
//   C3 a word written into an empty FIFO reaches the reader;
//   C4 a stream of 300 words, both sides pausing at random, arrives once and
//      in order.
module tb_dcfifo;
parameter real WH = 10;      // wrclk half period (ns)
parameter real RH = 13.65;   // rdclk half period (ns)
localparam L = 4;

reg wrclk = 0; always #(WH) wrclk = ~wrclk;
reg rdclk = 0; always #(RH) rdclk = ~rdclk;
reg aclr = 1, wrreq = 0, rdreq = 0;
reg  [15:0] data = 0;
wire wrfull, rdempty, rdae;
wire [15:0] q;
wire [2:0] wrusedw, rdusedw;
myDCFIFO #(.WORD(16), .LENGTH(L), .ALMOST(1)) F (.aclr(aclr),
    .wrclk(wrclk), .data(data), .wrreq(wrreq), .wrfull(wrfull), .wrusedw(wrusedw),
    .rdclk(rdclk), .rdreq(rdreq), .rdempty(rdempty), .rdalmost_empty(rdae), .q(q), .rdusedw(rdusedw));

reg [15:0] got [0:15];
integer n, i, fails = 0;

task clear; begin
    aclr = 1; #(8*WH + 8*RH); aclr = 0; #(8*WH + 8*RH);
end endtask
task put(input [15:0] v); begin      // data valid in the wrreq cycle only
    @(negedge wrclk); data = v; wrreq = 1;
    @(negedge wrclk); wrreq = 0; data = 16'hDEAD;
end endtask
task settle; begin repeat (6) @(negedge rdclk); repeat (6) @(negedge wrclk); end endtask
task drain; begin                    // reads everything into got[0..n-1]
    n = 0; settle;
    while (!rdempty && n < 16) begin @(negedge rdclk); got[n] = q; n = n + 1; rdreq = 1; @(negedge rdclk); rdreq = 0; end
end endtask

// C4: writer and reader at once
integer wn, rn, bad;
reg [15:0] want;
task stream; begin
    wn = 0; rn = 0; bad = 0;
    fork
        begin : writer
            while (wn < 300) begin
                @(negedge wrclk);
                if (!wrfull && ($random & 3) != 0) begin data = wn; wrreq = 1; wn = wn + 1; end
                else wrreq = 0;
                @(posedge wrclk); #0.1 wrreq = 0;
            end
        end
        begin : reader
            while (rn < 300) begin
                @(negedge rdclk);
                if (!rdempty && ($random & 3) != 0) begin
                    if (q !== rn[15:0]) bad = bad + 1;
                    rdreq = 1; rn = rn + 1;
                end else rdreq = 0;
                @(posedge rdclk); #0.1 rdreq = 0;
            end
        end
    join
end endtask

initial begin
    clear;

    put(5); put(6);
    drain;
    if (n == 2 && got[0] == 5 && got[1] == 6) $display("PASS C1 data in the wrreq cycle");
    else begin $display("FAIL C1 data in the wrreq cycle: %0d words, first %0d %0d (want 5 6)", n, got[0], got[1]); fails = fails + 1; end

    clear;
    for (i = 1; i <= L+1; i = i + 1) put(i);          // one more than fits
    drain;
    if (n == L && got[0] == 1 && got[L-1] == L) $display("PASS C2 LENGTH words fit, the next is refused");
    else begin $display("FAIL C2 full: %0d words, first %0d last %0d (want %0d words 1..%0d)", n, got[0], got[n-1], L, L); fails = fails + 1; end

    clear;
    put(7);
    i = 0; while (rdempty && i < 20) begin @(negedge rdclk); i = i + 1; end
    got[0] = q; rdreq = 1; @(negedge rdclk); rdreq = 0;
    drain;
    if (got[0] == 7 && n == 0) $display("PASS C3 a word reaches the reader (%0d rdclk cycles)", i);
    else begin $display("FAIL C3 reach: q %0d after %0d cycles, %0d left (want 7, 0)", got[0], i, n); fails = fails + 1; end

    clear;
    stream;
    settle;
    if (bad == 0 && rn == 300 && rdempty) $display("PASS C4 300 words in order");
    else begin $display("FAIL C4 stream: %0d out of order, %0d read, empty %0d", bad, rn, rdempty); fails = fails + 1; end

    $display("%0d of 4 failed", fails);
    $finish;
end

// a FIFO that loses words leaves the C4 reader waiting: stop it and say so
// (the whole run takes about 50 us)
initial begin
    #400000;
    $display("FAIL C4 stream: stuck, %0d written, %0d read (words lost or never seen)", wn, rn);
    $display("%0d of 4 failed", fails + 1);
    $finish;
end
endmodule
