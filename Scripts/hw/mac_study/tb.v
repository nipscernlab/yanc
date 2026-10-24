`timescale 1ns/1ps
// Today's inner product, measured on SAPHO/ula.v (docs/design/array-unit.md,
// section 7, step 1): for each vector of vec.txt (mac.py gen), the Dirac loop
// of today, LOD a0; F_MLT b0; then per term P_LOD_V a; F_MLT_V b; SF_ADD.
// The ALU inputs as in core.v: in1 is the memory word or the popped stack,
// in2 the accumulator, so F_MLT gets in1 = b, in2 = a, and SF_ADD in1 = the
// sum so far, in2 = the product. Writes one result word per vector to
// base_L<FR>.txt. Run through run.sh.
module tb;
parameter NB = 32, NM = 23, NE = 8, FR = 0;
reg  [NB-1:0] a, b, acc;
wire [NB-1:0] prod, sum, of1, of2;
reg clk = 0;
ula #(.NUBITS(NB), .NBMANT(NM), .NBEXPO(NE), .NUGAIN(128), .FROUND(FR), .F_MLT(1)) mul
    (.op(6'd5), .in1(b), .in2(a), .out(prod), .out_fast(of1), .clk(clk));
ula #(.NUBITS(NB), .NBMANT(NM), .NBEXPO(NE), .NUGAIN(128), .FROUND(FR), .F_ADD(1)) add
    (.op(6'd3), .in1(acc), .in2(prod), .out(sum), .out_fast(of2), .clk(clk));
integer fi, fo, nv, v, n, k, r;
reg [8*16-1:0] name;
initial begin
  fi = $fopen("vec.txt", "r");
  $sformat(name, "base_L%0d.txt", FR);
  fo = $fopen(name, "w");
  r = $fscanf(fi, "%d\n", nv);
  for (v = 0; v < nv; v = v + 1) begin
    r = $fscanf(fi, "%d\n", n);
    for (k = 0; k < n; k = k + 1) begin
      r = $fscanf(fi, "%h %h\n", a, b); #1;
      if (k == 0) acc = prod;
      else begin #1; acc = sum; end
      #1;
    end
    $fdisplay(fo, "%h", acc);
  end
  $fclose(fo);
  $finish;
end
endmodule
