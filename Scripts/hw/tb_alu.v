`timescale 1ns/1ps
// Self-checking unit testbench for the ALU blocks that no golden covers on its
// own: the shared shifter (SHL/SHR/SRS), F2I, and the float comparison. The
// expected values are derived here, never blessed:
//   - the shifts against Verilog's own <<, >> and >>>;
//   - F2I against an independent model that shifts natively in both directions;
//   - the comparison against real-valued arithmetic (the mantissa has at most
//     52 bits, so a double holds it exactly, and the exponent is kept inside
//     the double's range).
//
// The comparison is checked over the operand population the machine can hold -
// a normalized mantissa, or the minimum exponent (denormal / zero). That is
// what ula_norm produces and what f2mf encodes: it clamps every denormal to the
// same minimum exponent, which is what keeps the lexicographic order exact for
// denormals too. Words outside that invariant (an unnormalized mantissa at an
// arbitrary exponent) can only come from an input port writing raw bits; they
// are counted and printed as `info`, not as failures - see ula_fcmp's comment.
//
// Run: bash Scripts/hw/tb_alu.sh   (four formats x three #FROUND levels)
module tb;

parameter NUBITS = 32;
parameter MAN    = 23;
parameter EXP    =  8;
parameter FROUND =  0;
parameter N      = 4000;

// exponent window of the generated floats: the full format range, except that
// a double cannot hold 2**(e-MAN) for an 11-bit exponent, so it is clipped
localparam signed [EXP:0] EMIN = (EXP >= 10) ? -300 : -(1 << (EXP-1));
localparam signed [EXP:0] EMAX = (EXP >= 10) ?  300 :  (1 << (EXP-1)) - 1;

integer errs_shift, errs_f2i, errs_cmp, info_cmp, k, pop;

// ------------------------------------------------------------------ random --

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

// ------------------------------------------------ SHL / SHR / SRS (shared) --

reg  [NUBITS-1:0] a, b;
reg  [       5:0] op;
wire [NUBITS-1:0] sh;

ula_shift #(NUBITS,1,1,1) dut_shift (op, a, b, sh);

function [NUBITS-1:0] ref_shift;
	input [       5:0] o;
	input [NUBITS-1:0] x, n;
	begin
		case (o)
			6'd43  : ref_shift = x << n;
			6'd44  : ref_shift = x >> n;
			default: ref_shift = $signed(x) >>> n;
		endcase
	end
endfunction

// ------------------------------------------------------------------- F2I ---

reg  [MAN+EXP:0] fin;
wire signed [MAN+EXP:0] f2i;

ula_f2i #(MAN,EXP,FROUND) dut_f2i (fin, f2i);

function signed [MAN+EXP:0] ref_f2i;
	input [MAN+EXP:0] w;
	reg                    s;
	reg      [EXP-1:0]     ef, shf;
	reg      [MAN-1:0]     m;
	reg      [MAN+EXP:0]   mag;
	reg signed [EXP+1:0]   ew;
	begin
		s   = w[MAN+EXP];
		ef  = w[MAN+EXP-1:MAN];
		m   = w[MAN-1:0];
		shf = (ef[EXP-1]) ? -ef : ef;
		// native shift in each direction (the DUT reverses bits and shifts right)
		mag = (ef[EXP-1]) ? ({{(EXP+1){1'b0}}, m} >> shf) : ({{(EXP+1){1'b0}}, m} << shf);
		ew  = {{2{ef[EXP-1]}}, ef};
		if (FROUND != 0 && ew > $signed({{2{1'b0}}, EXP[EXP-1:0]}))
			ref_f2i = (s) ? {1'b1, {(MAN+EXP){1'b0}}} : {1'b0, {(MAN+EXP){1'b1}}};
		else
			ref_f2i = (s) ? -mag : mag;
	end
endfunction

// ------------------------------------------------------ float comparison ---

reg  [NUBITS-1:0] c1, c2;
wire [NUBITS-1:0] les, gre;

ula_fcmp #(NUBITS,MAN,EXP) dut_cmp (c1, c2, les, gre);

function real fval;
	input [NUBITS-1:0] w;
	reg                ws;
	reg signed [EXP:0] we;
	reg   [MAN-1:0]    wm;
	begin
		ws = w[MAN+EXP];
		we = $signed(w[MAN+EXP-1:MAN]);
		wm = w[MAN-1:0];
		fval = wm * (2.0 ** (we - MAN));
		if (ws) fval = -fval;
	end
endfunction

// one float of population `pop`: 0 = what the machine holds, 1 = arbitrary bits
task gen_float;
	output [NUBITS-1:0] w;
	reg                 s;
	reg signed [EXP:0]  e;
	reg   [MAN-1:0]     m;
	begin
		s = r16(0);
		m = rnd(0);
		e = $signed(r16(0)) % (EMAX - EMIN + 1);
		if (e < EMIN) e = e + (EMAX - EMIN + 1);
		if (e > EMAX) e = EMAX;
		if (pop == 0)
			case (r16(0) % 8)
				0      : begin m = 0;      e = EMIN; end                        // zero
				1, 2   : begin e = EMIN;   m = m >> (1 + (r16(0) % (MAN-1))); end // denormal, clamped exponent
				default: m[MAN-1] = 1'b1;                                        // normalized
			endcase
		w = {s, e[EXP-1:0], m};
	end
endtask

// ------------------------------------------------------------------- run ---

real v1, v2;

initial begin
	errs_shift = 0; errs_f2i = 0; errs_cmp = 0; info_cmp = 0;

	// shifts: random words, and every amount from 0 past the word width
	for (k = 0; k < N; k = k + 1) begin
		a  = rnd(0);
		b  = (k % 3 == 0) ? (k % (NUBITS + 4)) : rnd(0);
		op = 6'd43 + (k % 3);
		#1;
		if (sh !== ref_shift(op, a, b)) begin
			errs_shift = errs_shift + 1;
			if (errs_shift < 5) $display("  SHIFT op=%0d %h by %h -> %h expected %h", op, a, b, sh, ref_shift(op, a, b));
		end
	end

	// F2I: random words, with a sweep of the exponent field
	for (k = 0; k < N; k = k + 1) begin
		fin = rnd(0);
		if (k % 4 == 0) fin[MAN+EXP-1:MAN] = k % (1 << EXP);
		#1;
		if (f2i !== ref_f2i(fin)) begin
			errs_f2i = errs_f2i + 1;
			if (errs_f2i < 5) $display("  F2I %h -> %h expected %h", fin, f2i, ref_f2i(fin));
		end
	end

	// comparison: both populations
	for (pop = 0; pop < 2; pop = pop + 1)
		for (k = 0; k < N; k = k + 1) begin
			gen_float(c1);
			gen_float(c2);
			if (k %  9 == 0) c2 = c1;                                            // equal words
			if (k % 11 == 0) begin c2 = c1; c2[MAN+EXP] = ~c1[MAN+EXP]; end      // +x vs -x
			#1;
			v1 = fval(c1); v2 = fval(c2);
			if (les[0] !== (v1 < v2) || gre[0] !== (v1 > v2)) begin
				if (pop == 0) begin
					errs_cmp = errs_cmp + 1;
					if (errs_cmp < 5) $display("  FCMP %h vs %h -> les=%b gre=%b expected %b %b (%g, %g)",
					                           c1, c2, les[0], gre[0], v1 < v2, v1 > v2, v1, v2);
				end else info_cmp = info_cmp + 1;
			end
		end

	if (errs_shift + errs_f2i + errs_cmp == 0)
		$display("%0d/%0d/%0d FROUND=%0d: ok (shift %0d, f2i %0d, cmp %0d vectors; info: %0d of %0d out-of-format compares differ from the true value)",
		         NUBITS, MAN, EXP, FROUND, N, N, N, info_cmp, N);
	else
		$display("%0d/%0d/%0d FROUND=%0d: FAIL (shift %0d, f2i %0d, cmp %0d)",
		         NUBITS, MAN, EXP, FROUND, errs_shift, errs_f2i, errs_cmp);
	$finish;
end

endmodule
