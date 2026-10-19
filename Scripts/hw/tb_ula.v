`timescale 1ns/1ps
// Self-checking testbench of the whole ALU (module ula, every opcode parameter
// on, as in sapho_all): each operation is selected by its number on `op`, so
// the check covers the operator, its generate guard, its place in the output
// mux and out_fast, not the block alone. TODO item 6(a).
//
// The expected value of every operation is derived here from what the
// operation means, never blessed from a run:
//   - integer operations against Verilog's own operators on the same words;
//   - float operations that do not round (sign, negation, absolute value,
//     F_PST, F_SCL, XPO, F2I, the comparisons) against the meaning of the
//     format: value = mantissa * 2^(exponent - MAN), no hidden bit; zero is a
//     zero mantissa, kept canonical (+0, minimum exponent) at #FROUND >= 1.
// The operations that round (F_ADD, F_SU1/2, F_MLT, I2F, F_ROT) and the
// divisions are the next steps of item 6(a).
//
// in1 is the memory operand, in2 the accumulator (NEG negates in2, NEG_M in1;
// LES is in1 < in2).
//
// Run: bash Scripts/hw/tb_ula.sh   (formats x #FROUND levels)
module tb;

parameter NUBITS = 32;
parameter MAN    = 23;
parameter EXP    =  8;
parameter FROUND =  0;
parameter NUGAIN = 128;
parameter N      = 2000;

localparam signed [EXP:0] EMIN = -(1 << (EXP-1));     // minimum exponent
localparam signed [EXP:0] EMAX =  (1 << (EXP-1)) - 1; // maximum exponent
// the canonical zero: +, minimum exponent, zero mantissa
localparam [NUBITS-1:0] ZERO = {1'b0, 1'b1, {(NUBITS-2){1'b0}}};

// ------------------------------------------------------------- the ALU ---

reg  [       5:0] op;
reg  [NUBITS-1:0] in1, in2;
wire [NUBITS-1:0] out, out_fast;
reg               clk = 0;

ula #(.NUBITS(NUBITS), .NBMANT(MAN), .NBEXPO(EXP), .NUGAIN(NUGAIN), .FROUND(FROUND),
      .ADD(1), .F_ADD(1), .MLT(1), .F_MLT(1), .DIV(1), .F_DIV(1), .MOD(1),
      .SGN(1), .F_SGN(1), .NEG(1), .NEG_M(1), .F_NEG(1), .F_NEG_M(1),
      .ABS(1), .ABS_M(1), .F_ABS(1), .F_ABS_M(1), .PST(1), .PST_M(1), .F_PST(1), .F_PST_M(1),
      .NRM(1), .NRM_M(1), .I2F(1), .I2F_M(1), .F2I(1), .F2I_M(1),
      .AND(1), .ORR(1), .XOR(1), .INV(1), .INV_M(1), .LAN(1), .LOR(1), .LIN(1), .LIN_M(1),
      .LES(1), .F_LES(1), .GRE(1), .F_GRE(1), .EQU(1), .SHL(1), .SHR(1), .SRS(1),
      .F_ROT(1), .F_SU1(1), .F_SU2(1), .F_SCL(1), .XPO(1), .XPO_M(1))
    dut (.op(op), .in1(in1), .in2(in2), .out(out), .out_fast(out_fast), .clk(clk));

// ------------------------------------------------------------ random ---

integer seed = 1;

// 16 bits at a time: `| $random` on a wider word would sign-extend and bias it
function [15:0] r16; input integer d; reg [15:0] c; begin c = $random(seed); r16 = c; end endfunction

function [NUBITS-1:0] rnd;
	input integer d;
	integer j;
	begin
		rnd = 0;
		for (j = 0; j < NUBITS; j = j + 16) rnd = (rnd << 16) | {{(NUBITS > 16 ? NUBITS-16 : 0){1'b0}}, r16(0)};
	end
endfunction

// an integer operand: random, or one of the edges
function [NUBITS-1:0] gen_int;
	input integer d;
	begin
		case (r16(0) % 10)
			0      : gen_int = 0;
			1      : gen_int = 1;
			2      : gen_int = {NUBITS{1'b1}};                    // -1
			3      : gen_int = {1'b1, {(NUBITS-1){1'b0}}};        // INT_MIN
			4      : gen_int = {1'b0, {(NUBITS-1){1'b1}}};        // INT_MAX
			5      : gen_int = r16(0) % 64;                       // small
			default: gen_int = rnd(0);
		endcase
	end
endfunction

// a float the machine can hold: normalized, or zero; at #FROUND 0 also a
// denormal at the minimum exponent and a zero of either sign with any
// exponent field (level 0 does not canonicalize; levels 1 and 2 flush
// denormals and keep the zero +0 at the minimum exponent). Exponents favour
// both ends of the range.
function [NUBITS-1:0] gen_float;
	input integer d;
	reg                s;
	reg signed [EXP:0] e;
	reg   [MAN-1:0]    m;
	begin
		s = r16(0);
		m = rnd(0);
		case (r16(0) % 6)
			0      : e = EMIN;
			1      : e = EMAX;
			2      : e = EMIN + (r16(0) % 4);
			default: begin e = $signed(r16(0)) % (EMAX - EMIN + 1); if (e < EMIN) e = e + (EMAX - EMIN + 1); if (e > EMAX) e = EMAX; end
		endcase
		case (r16(0) % 10)
			0      : begin m = 0; e = EMIN; end                                   // zero
			1      : m = 0;                                                       // zero, any exponent
			2      : begin e = EMIN; m = m >> (1 + (r16(0) % (MAN-1))); end       // denormal
			default: m[MAN-1] = 1'b1;                                             // normalized
		endcase
		if (FROUND != 0 && !m[MAN-1]) begin s = 0; e = EMIN; m = 0; end       // levels 1, 2: the canonical zero
		gen_float = {s, e[EXP-1:0], m};
	end
endfunction

// ------------------------------------------------------- the format ---

function          f_sgn; input [NUBITS-1:0] w; f_sgn = w[MAN+EXP];                endfunction
function signed [EXP:0] f_exp; input [NUBITS-1:0] w; f_exp = $signed(w[MAN+EXP-1:MAN]); endfunction
function [MAN-1:0] f_man; input [NUBITS-1:0] w; f_man = w[MAN-1:0];               endfunction

// the real value (the comparison): the mantissa has at most 52 bits and the
// exponent stays within a double's range for EXP <= 10
function real fval;
	input [NUBITS-1:0] w;
	begin
		fval = f_man(w) * (2.0 ** (f_exp(w) - MAN));
		if (f_sgn(w)) fval = -fval;
	end
endfunction

// a word with this sign, exponent and mantissa; a zero mantissa at
// #FROUND >= 1 is the canonical zero whatever the sign
function [NUBITS-1:0] fword;
	input          s;
	input [EXP-1:0] e;
	input [MAN-1:0] m;
	begin
		fword = (FROUND != 0 && m == 0) ? {1'b0, e, m} : {s, e, m};
	end
endfunction

// XPO: floor(log2 |x|) of a normalized float, the exponent plus the position
// of the top set bit of the mantissa. Not a value, by convention (ula_xpo's
// comment, decided 2026-10-18): a zero mantissa or a level-0 denormal gives
// e + MAN-1 as if normalized (the machine's zero: EMIN + MAN-1, "very small"),
// and only the all-zero word gives 0.
function signed [NUBITS-1:0] ref_xpo;
	input [NUBITS-1:0] w;
	integer j, top;
	reg [MAN-1:0] m;
	begin
		top = -1;
		m   = f_man(w);
		for (j = 0; j < MAN; j = j + 1) if (m[j]) top = j;
		if (w[MAN+EXP-1:0] == 0) ref_xpo = 0;
		else if (top != MAN-1)   ref_xpo = f_exp(w) + (MAN-1);
		else                     ref_xpo = f_exp(w) + top;
	end
endfunction

// F2I: truncation toward zero of the value; at #FROUND >= 1 saturating to
// INT_MIN/INT_MAX past the word (level 0 wraps: the low bits of the integer)
localparam WM = MAN + (1 << (EXP-1)) + 1;          // the integer part of the largest float
function signed [NUBITS-1:0] ref_f2i;
	input [NUBITS-1:0] w;
	reg [WM-1:0] mag;
	reg signed [EXP:0] e;
	begin
		e   = f_exp(w);
		mag = f_man(w);
		mag = (e >= 0) ? (mag << e) : (mag >> (-e));
		// -2^(NUBITS-1) itself fits, and saturating it gives the same word
		if (FROUND != 0 && (mag >> (NUBITS-1)) != 0)
			ref_f2i = (f_sgn(w)) ? {1'b1, {(NUBITS-1){1'b0}}} : {1'b0, {(NUBITS-1){1'b1}}};
		else
			ref_f2i = (f_sgn(w)) ? -mag[NUBITS-1:0] : mag[NUBITS-1:0];
	end
endfunction

// F_SCL: in2 * 2^in1, the exponent saturating at the ends of its range; a
// zero stays the canonical zero at #FROUND >= 1
function [NUBITS-1:0] ref_scl;
	input signed [NUBITS-1:0] k;
	input        [NUBITS-1:0] w;
	reg signed [NUBITS+1:0] e;
	begin
		e = f_exp(w) + k;
		if (e > EMAX) e = EMAX;
		if (e < EMIN) e = EMIN;
		ref_scl = (FROUND != 0 && f_man(w) == 0) ? ZERO : {f_sgn(w), e[EXP-1:0], f_man(w)};
	end
endfunction

// ------------------------------------------------- what each op means ---

function [NUBITS-1:0] b2w; input b; b2w = {{(NUBITS-1){1'b0}}, b}; endfunction

function [NUBITS-1:0] ref_op;
	input [5:0] o;
	input signed [NUBITS-1:0] a, b;           // in1 (memory), in2 (accumulator)
	begin
		case (o)
			6'd0 : ref_op = b;                                         // NOP
			6'd1 : ref_op = a;                                         // LOD
			6'd2 : ref_op = a + b;                                     // ADD
			6'd4 : ref_op = a * b;                                     // MLT
			6'd9 : ref_op = (a < 0) ? -((b < 0) ? -b : b) : ((b < 0) ? -b : b);   // SGN: |in2| with in1's sign
			6'd10: ref_op = fword(f_sgn(a), b[MAN+EXP-1:MAN], f_man(b));         // F_SGN
			6'd11: ref_op = -b;                                        // NEG
			6'd12: ref_op = -a;                                        // NEG_M
			6'd13: ref_op = fword(~f_sgn(b), b[MAN+EXP-1:MAN], f_man(b));        // F_NEG
			6'd14: ref_op = fword(~f_sgn(a), a[MAN+EXP-1:MAN], f_man(a));        // F_NEG_M
			6'd15: ref_op = (b < 0) ? -b : b;                          // ABS
			6'd16: ref_op = (a < 0) ? -a : a;                          // ABS_M
			6'd17: ref_op = {1'b0, b[MAN+EXP-1:0]};                    // F_ABS
			6'd18: ref_op = {1'b0, a[MAN+EXP-1:0]};                    // F_ABS_M
			6'd19: ref_op = (b < 0) ? 0 : b;                           // PST
			6'd20: ref_op = (a < 0) ? 0 : a;                           // PST_M
			6'd21: ref_op = (f_sgn(b)) ? ZERO : b;                     // F_PST: a negative (or -0) becomes +0
			6'd22: ref_op = (f_sgn(a)) ? ZERO : a;                     // F_PST_M
			6'd23: ref_op = b / NUGAIN;                                // NRM (truncating, as C)
			6'd24: ref_op = a / NUGAIN;                                // NRM_M
			6'd27: ref_op = ref_f2i(b);                                // F2I
			6'd28: ref_op = ref_f2i(a);                                // F2I_M
			6'd29: ref_op = a & b;                                     // AND
			6'd30: ref_op = a | b;                                     // ORR
			6'd31: ref_op = a ^ b;                                     // XOR
			6'd32: ref_op = ~b;                                        // INV
			6'd33: ref_op = ~a;                                        // INV_M
			6'd34: ref_op = b2w(a != 0 && b != 0);                     // LAN
			6'd35: ref_op = b2w(a != 0 || b != 0);                     // LOR
			6'd36: ref_op = b2w(b == 0);                               // LIN
			6'd37: ref_op = b2w(a == 0);                               // LIN_M
			6'd38: ref_op = b2w(a < b);                                // LES
			6'd39: ref_op = b2w(fval(a) < fval(b));                    // F_LES
			6'd40: ref_op = b2w(a > b);                                // GRE
			6'd41: ref_op = b2w(fval(a) > fval(b));                    // F_GRE
			6'd42: ref_op = b2w(a == b);                               // EQU
			6'd43: ref_op = a << b;                                    // SHL (in1 by in2)
			6'd44: ref_op = a >> b;                                    // SHR
			6'd45: ref_op = a >>> b;                                   // SRS
			6'd49: ref_op = ref_scl(a, b);                             // F_SCL: in2 * 2^in1
			6'd50: ref_op = ref_xpo(b);                                // XPO
			6'd51: ref_op = ref_xpo(a);                                // XPO_M
			default: ref_op = {NUBITS{1'bx}};
		endcase
	end
endfunction

// the operands each op takes: 0 int/int, 1 float/float, 2 int k / float x
// (F_SCL), 3 int word / shift amount
function integer kind;
	input [5:0] o;
	case (o)
		6'd10, 6'd13, 6'd14, 6'd17, 6'd18, 6'd21, 6'd22, 6'd27, 6'd28,
		6'd39, 6'd41, 6'd50, 6'd51: kind = 1;
		6'd49                     : kind = 2;
		6'd43, 6'd44, 6'd45       : kind = 3;
		default                   : kind = 0;
	endcase
endfunction

// the operations of this step (the rounding ones and the divisions come next)
localparam NOPS = 42;
reg [5:0] ops [0:NOPS-1];
initial begin : list
	integer j, n; n = 0;
	for (j = 0; j <= 51; j = j + 1)
		if (!(j == 3 || j == 5 || j == 6 || j == 7 || j == 8 || j == 25 || j == 26 ||
		      j == 46 || j == 47 || j == 48)) begin
			if (n < NOPS) ops[n] = j;
			n = n + 1;
		end
	if (n != NOPS) begin $display("tb_ula: %0d operations listed, NOPS is %0d", n, NOPS); $finish; end
end

// ------------------------------------------------------------- run ---

integer k, j, errs, total, bad [0:63];
reg [NUBITS-1:0] want;
reg signed [NUBITS-1:0] kk;

initial begin
	#1;
	errs = 0; total = 0;
	for (j = 0; j < 64; j = j + 1) bad[j] = 0;
	for (j = 0; j < NOPS; j = j + 1) begin
		op = ops[j];
		for (k = 0; k < N; k = k + 1) begin
			case (kind(op))
				1: begin in1 = gen_float(0); in2 = gen_float(0);
				         if (k % 9 == 0) in1 = in2;                                     // equal words
				         if (k % 11 == 0) begin in1 = in2; in1[MAN+EXP] = ~in2[MAN+EXP]; end   // x and -x
				   end
				2: begin kk = $signed(r16(0)) % (2 * (EMAX - EMIN) + 8); in1 = kk; in2 = gen_float(0);
				         if (k % 7 == 0) in1 = gen_int(0); end                           // a k far out of range
				3: begin in1 = gen_int(0); in2 = (k % 3 == 0) ? (k % (NUBITS + 4)) : gen_int(0); end
				default: begin in1 = gen_int(0); in2 = gen_int(0); if (k % 13 == 0) in1 = in2; end
			endcase
			#1;
			want = ref_op(op, in1, in2);
			total = total + 1;
			if (out !== want || out_fast !== want) begin
				errs = errs + 1;
				if (bad[op] < 3) $display("  op %0d: in1=%h in2=%h -> out=%h out_fast=%h, expected %h",
				                          op, in1, in2, out, out_fast, want);
				bad[op] = bad[op] + 1;
			end
		end
	end
	if (errs == 0)
		$display("%0d/%0d/%0d FROUND=%0d: ok (%0d operations x %0d vectors)", NUBITS, MAN, EXP, FROUND, NOPS, N);
	else begin
		$write("%0d/%0d/%0d FROUND=%0d: FAIL, %0d of %0d vectors; by op:", NUBITS, MAN, EXP, FROUND, errs, total);
		for (j = 0; j < 64; j = j + 1) if (bad[j]) $write(" %0d:%0d", j, bad[j]);
		$display("");
	end
	$finish;
end

endmodule
