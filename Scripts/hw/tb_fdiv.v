`timescale 1ns/1ps
// checks the explicit divider array of ula_fdiv against the `/` and `%` operators
// at every FROUND level, for random normalized operands (and the zero cases)
module tb;
parameter MAN = 23, EXP = 8;
localparam W2 = MAN+3;   // level 2 mantissa+GRS

reg  [MAN+EXP:0] a, b;
wire [MAN+EXP+2:0] o0, o1;          // levels 0/1: {s, e[EXP+1:0], m[MAN-1:0]}
wire [MAN+EXP+5:0] o2;              // level 2:    {s, e[EXP+1:0], m[MAN+2:0]}

ula_fdiv #(MAN,EXP,0,0) d0 (a, b, o0);
ula_fdiv #(MAN,EXP,1,0) d1 (a, b, o1);
ula_fdiv #(MAN,EXP,2,3) d2 (a, b, o2);

integer n, errs;
reg [MAN-1:0] m1, m2;
reg signed [EXP-1:0] e1, e2;
reg s1, s2;
reg [2*MAN-2:0] q0;  reg [2*MAN-1:0] q1;  reg [2*MAN+2:0] q2, r2;
reg signed [EXP+1:0] ed, e0, e1x, e2x;
reg [MAN-1:0] xm0, xm1;  reg [MAN+2:0] xm2;  reg top;
reg [MAN+EXP+2:0] x0, x1; reg [MAN+EXP+5:0] x2;

task check;
begin
	m1 = a[MAN-1:0]; m2 = b[MAN-1:0]; e1 = a[MAN+EXP-1:MAN]; e2 = b[MAN+EXP-1:MAN]; s1 = a[MAN+EXP]; s2 = b[MAN+EXP];
	ed = e1 - e2 - MAN;
	// level 0 reference: legacy slice
	q0  = ({m1, {(MAN-1){1'b0}}}) / m2;
	e0  = ed + 1;
	x0  = {s1^s2, e0, q0[MAN-1:0]};
	// level 1 reference
	q1  = ({m1, {MAN{1'b0}}}) / m2;  top = q1[MAN];
	e1x = ed + top; xm1 = top ? q1[MAN:1] : q1[MAN-1:0];
	x1  = {s1^s2, e1x, xm1};
	// level 2 reference: MAN+3 quotient bits, exact sticky from the remainder
	q2  = ({m1, {(MAN+2){1'b0}}}) / m2;  r2 = ({m1, {(MAN+2){1'b0}}}) % m2;  top = q2[MAN+2];
	xm2 = top ? {q2[MAN+2:1], q2[0] | (r2 != 0)} : {q2[MAN+1:0], (r2 != 0)};
	e2x = ed + top;
	x2  = {s1^s2, e2x, xm2};
	#1;
	if (m2 != 0) begin
		if (o0 !== x0) begin errs = errs + 1; if (errs < 6) $display("L0 mismatch a=%h b=%h got=%h exp=%h", a, b, o0, x0); end
		if (o1 !== x1) begin errs = errs + 1; if (errs < 6) $display("L1 mismatch a=%h b=%h got=%h exp=%h", a, b, o1, x1); end
		if (o2 !== x2) begin errs = errs + 1; if (errs < 6) $display("L2 mismatch a=%h b=%h got=%h exp=%h", a, b, o2, x2); end
	end else begin
		// division by zero at levels >= 1: exponent above every value, mantissa all ones -> ula_norm saturates
		if (o1[MAN+EXP+1:MAN] !== {1'b0, {(EXP+1){1'b1}}} || o1[MAN-1:0] !== {MAN{1'b1}}) begin errs = errs + 1; $display("L1 div-by-zero: %h", o1); end
		if (o2[MAN+EXP+4:W2]  !== {1'b0, {(EXP+1){1'b1}}} || o2[W2-1:0]  !== {W2{1'b1}})  begin errs = errs + 1; $display("L2 div-by-zero: %h", o2); end
	end
end
endtask

initial begin
	errs = 0;
	for (n = 0; n < 20000; n = n + 1) begin
		a = $random; b = $random;
		a[MAN-1] = 1'b1; b[MAN-1] = 1'b1;              // normalized mantissas
		if (n % 7 == 0) a[MAN-1:0] = b[MAN-1:0];       // equal mantissas: exact quotient, zero remainder
		if (n % 11 == 0) a[MAN-1:0] = {1'b1, {(MAN-1){1'b0}}}; // power of two
		if (n % 13 == 0) b[MAN-1:0] = {MAN{1'b1}};     // divisor all ones
		check;
	end
	a = 0; a[MAN+EXP-1:MAN] = 8'h80; b = $random; b[MAN-1] = 1; check;   // 0 / x
	a = $random; a[MAN-1] = 1; b = 0; check;                              // x / 0 (levels >= 1)
	if (errs == 0) $display("PASS: %0d random divisions, 3 levels, %0d/%0d", n, MAN, EXP); else $display("FAIL: %0d mismatches", errs);
	$finish;
end
endmodule
