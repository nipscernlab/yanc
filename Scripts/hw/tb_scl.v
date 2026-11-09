`timescale 1ns/1ps
// F_SCL (ula_scl) at the ends of the exponent's range, #FROUND 0, 1 and 2
// (#184). Format 32/23/8: a float is s, e (8 bits, two's complement, EMIN
// -128 is the zero's) and m (23 bits, normalized: m >= 2^22), value
// m * 2^e. At level >= 1 F_SCL follows the normaliser: below the range the
// canonical zero, above it the largest float of the sign. At level 0 the
// exponent saturates and the mantissa stays. At every level a zero stays a
// zero, whatever k (at level 0 a zero with a large exponent made F_ADD shift
// the other operand out: ldexp(0, 200) + 1.0 gave 0).
module tb;

parameter FROUND = 0;
localparam MAN = 23, EXP = 8, W = MAN + EXP + 1;

reg  signed [W-1:0] k;
reg         [W-1:0] x;
wire        [W-1:0] y;
ula_scl #(MAN, EXP, FROUND) dut (k, x, y);

localparam [EXP-1:0] EMIN = 8'h80, EMAX = 8'h7f;
localparam [MAN-1:0] M1   = 23'h400000;                // 1.0 is M1 * 2^-22
localparam [W-1:0]   ONE  = {1'b0, 8'hea, M1};         // e = -22
localparam [W-1:0]   MONE = {1'b1, 8'hea, M1};
localparam [W-1:0]   ZERO = {1'b0, EMIN, {MAN{1'b0}}}; // the canonical zero
localparam [W-1:0]   NZER = {1'b1, EMIN, {MAN{1'b0}}}; // a zero with the sign set (level 0)
localparam [W-1:0]   BIG  = {1'b0, EMAX, {MAN{1'b1}}}; // the largest float
localparam [W-1:0]   NBIG = {1'b1, EMAX, {MAN{1'b1}}};

integer errs = 0;
task check(input signed [W-1:0] kk, input [W-1:0] xx, input [W-1:0] want, input [8*40-1:0] what);
	begin
		k = kk; x = xx; #1;
		if (y !== want) begin errs = errs + 1; $display("FAIL FROUND=%0d %0s: got %h, want %h", FROUND, what, y, want); end
	end
endtask

initial begin
	// in range: 1.0 * 2^10 = 1024
	check(10, ONE, {1'b0, 8'hf4, M1}, "1.0 * 2^10");
	// k that fits EXP+1 bits but takes e past the ends
	check(-200, ONE,  (FROUND != 0) ? ZERO : {1'b0, EMIN, M1}, "1.0 * 2^-200");
	check( 200, ONE,  (FROUND != 0) ? BIG  : {1'b0, EMAX, M1}, "1.0 * 2^200");
	check( 200, MONE, (FROUND != 0) ? NBIG : {1'b1, EMAX, M1}, "-1.0 * 2^200");
	// k wider than the exponent: saturates by its sign
	check(-100000, ONE, (FROUND != 0) ? ZERO : {1'b0, EMIN, M1}, "1.0 * 2^-100000");
	check( 100000, ONE, (FROUND != 0) ? BIG  : {1'b0, EMAX, M1}, "1.0 * 2^100000");
	// the smallest normal stays; one step below is the zero at level >= 1
	check(-106, ONE, {1'b0, EMIN, M1}, "1.0 * 2^-106 (the smallest normal)");
	check(-107, ONE, (FROUND != 0) ? ZERO : {1'b0, EMIN, M1}, "1.0 * 2^-107");
	// a zero stays a zero
	check( 200, ZERO, ZERO, "0 * 2^200");
	check(-200, ZERO, ZERO, "0 * 2^-200");
	check( 200, NZER, (FROUND != 0) ? ZERO : NZER, "-0 * 2^200");
	if (errs == 0) $display("tb_scl FROUND=%0d: ok", FROUND);
	$finish;
end

endmodule
