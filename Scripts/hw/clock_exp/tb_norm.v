// ula_norm on random inputs, biased to many leading zeros and to zero
// mantissas; prints every output. Run once with the original ula.v and once
// with the experiment's: the two prints must be identical.
module tb_norm;
parameter MAN = 23, EXP = 8, FROUND = 0, G = 0, LZC = 1, DIR = 0, N = 200000;
localparam WI = MAN+EXP+G+3;
localparam W  = MAN+G;
reg  [WI-1:0] in, in_d;
reg           use_d;
wire [MAN+EXP:0] out;
ula_norm #(MAN, EXP, FROUND, G, LZC, DIR) dut (in, in_d, use_d, out);
integer k, sh, seed;
reg [127:0] r;
initial begin
    seed = 12345;
    for (k = 0; k < N; k = k+1) begin
        r  = {$random(seed), $random(seed), $random(seed), $random(seed)};
        in = r[WI-1:0];
        sh = {$random(seed)} % (W+1);                    // 0..W leading zeros wanted
        in[W-1:0] = in[W-1:0] >> sh;                      // sh = W gives a zero mantissa
        r    = {$random(seed), $random(seed), $random(seed), $random(seed)};
        in_d = r[WI-1:0];
        use_d = (DIR != 0) ? $random(seed) : 1'b0;
        #1 $display("%h %h %b %h", in, in_d, use_d, out);
    end
    $finish;
end
endmodule
