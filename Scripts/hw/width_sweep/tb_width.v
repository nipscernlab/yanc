// Icarus side of the width sweep (Scripts/hw/width_sweep.sh). Reads the
// oracle's vectors (hex, W bits) and checks every operator twice: the
// CONTINUOUS assigns of ops.v, and the same expressions evaluated
// PROCEDURALLY here -- so a simulator that gets one
// context right and the other wrong is caught as such.
`timescale 1ns/1ps
module tb;
parameter W = 64;
parameter VEC = "vec.txt";

reg  signed [W-1:0] a, b;
wire signed [W-1:0] add, sub, mul, div, mod, shl, shr, sra;
wire        [W-1:0] udiv, umod;
wire                lt, gt, ult, eq;

ops #(W) dut (a, b, add, sub, mul, div, mod, shl, shr, sra, udiv, umod, lt, gt, ult, eq);

// expected, from the oracle
reg [W-1:0] e_add, e_sub, e_mul, e_div, e_mod, e_udiv, e_umod, e_shl, e_shr, e_sra;
reg         e_lt, e_gt, e_ult, e_eq;

// procedural copies
reg signed [W-1:0] p_add, p_sub, p_mul, p_div, p_mod, p_shl, p_shr, p_sra;
reg        [W-1:0] p_udiv, p_umod;
reg                p_lt, p_gt, p_ult, p_eq;

integer f, n, rc, k;
integer c [0:13];   // continuous-assign mismatches per op
integer p [0:13];   // procedural mismatches per op

task chk;  input integer idx; input [W-1:0] got_c, got_p, exp; input integer wide;
	begin
		if (wide) begin
			if (got_c !== exp) begin c[idx] = c[idx] + 1; if (c[idx] == 1) $display("  cont op%0d: a=%h b=%h got=%h exp=%h", idx, a, b, got_c, exp); end
			if (got_p !== exp) begin p[idx] = p[idx] + 1; if (p[idx] == 1) $display("  proc op%0d: a=%h b=%h got=%h exp=%h", idx, a, b, got_p, exp); end
		end else begin
			if (got_c[0] !== exp[0]) c[idx] = c[idx] + 1;
			if (got_p[0] !== exp[0]) p[idx] = p[idx] + 1;
		end
	end
endtask

initial begin
	for (k = 0; k < 14; k = k + 1) begin c[k] = 0; p[k] = 0; end
	f = $fopen(VEC, "r");
	if (f == 0) begin $display("cannot open %s", VEC); $finish; end
	n = 0;
	while (!$feof(f)) begin
		rc = $fscanf(f, "%h %h %h %h %h %h %h %h %h %h %h %h %h %h %h %h\n",
		             a, b, e_add, e_sub, e_mul, e_div, e_mod, e_udiv, e_umod, e_shl, e_shr, e_sra, e_lt, e_gt, e_ult, e_eq);
		if (rc != 16) begin if (rc > 0) $display("short line after %0d vectors (rc=%0d)", n, rc); end
		else begin
			#1;
			p_add = a + b;   p_sub = a - b;   p_mul = a * b;
			p_div = a / b;   p_mod = a % b;
			p_shl = a << b;  p_shr = a >> b;  p_sra = a >>> b;
			p_udiv = $unsigned(a) / $unsigned(b);  p_umod = $unsigned(a) % $unsigned(b);
			p_lt = a < b;    p_gt = a > b;    p_ult = $unsigned(a) < $unsigned(b);  p_eq = a == b;
			chk( 0, add,  p_add,  e_add,  1);
			chk( 1, sub,  p_sub,  e_sub,  1);
			chk( 2, mul,  p_mul,  e_mul,  1);
			chk( 3, div,  p_div,  e_div,  1);
			chk( 4, mod,  p_mod,  e_mod,  1);
			chk( 5, udiv, p_udiv, e_udiv, 1);
			chk( 6, umod, p_umod, e_umod, 1);
			chk( 7, shl,  p_shl,  e_shl,  1);
			chk( 8, shr,  p_shr,  e_shr,  1);
			chk( 9, sra,  p_sra,  e_sra,  1);
			chk(10, {{(W-1){1'b0}}, lt},  {{(W-1){1'b0}}, p_lt},  {{(W-1){1'b0}}, e_lt},  0);
			chk(11, {{(W-1){1'b0}}, gt},  {{(W-1){1'b0}}, p_gt},  {{(W-1){1'b0}}, e_gt},  0);
			chk(12, {{(W-1){1'b0}}, ult}, {{(W-1){1'b0}}, p_ult}, {{(W-1){1'b0}}, e_ult}, 0);
			chk(13, {{(W-1){1'b0}}, eq},  {{(W-1){1'b0}}, p_eq},  {{(W-1){1'b0}}, e_eq},  0);
			n = n + 1;
		end
	end
	$fclose(f);
	$display("ICARUS W=%0d vectors=%0d", W, n);
	$display("  cont: add %0d sub %0d mul %0d div %0d mod %0d udiv %0d umod %0d shl %0d shr %0d sra %0d lt %0d gt %0d ult %0d eq %0d",
	         c[0], c[1], c[2], c[3], c[4], c[5], c[6], c[7], c[8], c[9], c[10], c[11], c[12], c[13]);
	$display("  proc: add %0d sub %0d mul %0d div %0d mod %0d udiv %0d umod %0d shl %0d shr %0d sra %0d lt %0d gt %0d ult %0d eq %0d",
	         p[0], p[1], p[2], p[3], p[4], p[5], p[6], p[7], p[8], p[9], p[10], p[11], p[12], p[13]);
	$finish;
end
endmodule
