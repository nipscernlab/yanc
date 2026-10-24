`timescale 1ns/1ps
// Float addition (F_ADD) at one #FROUND level, measured: 200000 random pairs
// and 300 running sums of 1000 values of mixed signs, at 32/23/8. Writes the
// operand and result words; check.py compares them with the exact results
// (docs/design/fround1-subtraction.md). Run through run.sh.
module tb;
parameter FR = 1;
reg [5:0] op = 6'd3; reg [31:0] in1, in2; wire [31:0] out, of; reg clk = 0;
ula #(.NUBITS(32), .NBMANT(23), .NBEXPO(8), .NUGAIN(128), .FROUND(FR), .F_ADD(1)) dut
    (.op(op), .in1(in1), .in2(in2), .out(out), .out_fast(of), .clk(clk));
integer seed = 7, k, t, f;
// a normalized float of value in [2^lo, 2^hi), random sign
function [31:0] rf; input integer lo, hi; reg [22:0] m; integer e; reg [31:0] r;
  begin r = $random(seed); m = {1'b1, r[21:0]}; e = lo + ($unsigned($random(seed)) % (hi - lo)) - 22;
        rf = {$random(seed) & 1'b1, e[7:0], m}; end endfunction
initial begin
  f = $fopen("pairs.txt", "w");
  for (k = 0; k < 200000; k = k + 1) begin
    in1 = rf(0, 10); in2 = rf(0, 10); #1; $fdisplay(f, "%h %h %h", in1, in2, out);
  end
  $fclose(f);
  f = $fopen("sums.txt", "w");
  for (t = 0; t < 300; t = t + 1) begin
    in2 = 32'h40000000;                       // the canonical zero
    for (k = 0; k < 1000; k = k + 1) begin
      in1 = rf(0, 10); #1; $fdisplay(f, "%h", in1); in2 = out;
    end
    $fdisplay(f, "= %h", in2);
  end
  $fclose(f);
  $finish;
end
endmodule
