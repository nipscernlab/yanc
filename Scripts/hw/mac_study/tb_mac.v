`timescale 1ns/1ps
// One MAC variant of mac.v against its model (mac.py expect): every vector
// of vec.txt through the block, the result compared bit for bit with the
// expected word. Vectors the model marks out of range (or overflowing the
// window of B) are counted apart. Run through check.sh.
module tb_mac;
parameter VAR = 2, NB = 32, M = 23, E = 8, W = 64, LGN = 10, XB = 12, FR = 2;
parameter EXP = "exp.txt";
reg clk = 0; always #5 clk = ~clk;
reg clr = 0, en = 0; reg [NB-1:0] a = 0, b = 0; reg signed [XB-1:0] lsb = 0;
wire [NB-1:0] res;
generate
	if (VAR == 0)      begin : va mac_a #(NB, M, E, W, FR)     u (clk, clr, en, a, b, res); end
	else if (VAR == 1) begin : vb mac_b #(NB, M, E, W, XB, FR) u (clk, clr, en, a, b, lsb, res); end
	else               begin : vc mac_c #(NB, M, E, LGN, FR)   u (clk, clr, en, a, b, res); end
endgenerate
integer fv, fe, nv, v, n, k, r, L, ok, good = 0, bad = 0, skip = 0;
reg [NB-1:0] want;
initial begin
	fv = $fopen("vec.txt", "r"); fe = $fopen(EXP, "r");
	r = $fscanf(fv, "%d\n", nv);
	for (v = 0; v < nv; v = v + 1) begin
		r = $fscanf(fv, "%d\n", n);
		r = $fscanf(fe, "%d %h %d\n", L, want, ok);
		@(negedge clk) begin clr = 1; lsb = L; end
		@(negedge clk) clr = 0;
		for (k = 0; k < n; k = k + 1) begin
			r = $fscanf(fv, "%h %h\n", a, b);
			en = 1; @(negedge clk);
		end
		en = 0;
		repeat (3) @(negedge clk);
		if (!ok) skip = skip + 1;
		else if (res === want) good = good + 1;
		else begin
			bad = bad + 1;
			if (bad <= 5) $display("vector %0d (%0d terms): got %h, the model %h", v, n, res, want);
		end
	end
	$display("VAR %0d W %0d FR %0d: %0d equal, %0d different, %0d out of range (not compared)", VAR, W, FR, good, bad, skip);
	$finish;
end
endmodule
