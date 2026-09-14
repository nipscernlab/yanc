// ****************************************************************************
// Main multiplexer ***********************************************************
// ****************************************************************************

// Simulation-visibility guard: the rounding-error block at the bottom of this
// file (real-valued ULA taps + delta_int/delta_float) is waveform-only. Compile
// it for Icarus (predefines __ICARUS__) and for Verilator with +define+YANC_TRACE,
// never for synthesis. Verilog `ifdef has no OR, so fold both into YANC_SIM_VIS.
`ifdef __ICARUS__
 `ifndef YANC_SIM_VIS
  `define YANC_SIM_VIS
 `endif
`endif
`ifdef YANC_TRACE
 `ifndef YANC_SIM_VIS
  `define YANC_SIM_VIS
 `endif
`endif

// selects which operation goes to the output ---------------------------------

module ula_mux
#(
	 parameter NUBITS = 32
 )(
	 // operation index
	 input     [       5:0] op  ,
	 // outputs one of the inputs (in1 -> from memory, in2 -> from accumulator)
	 input     [NUBITS-1:0] in1 , in2,
	 // two-parameter arithmetic operations
	 input     [NUBITS-1:0] add ,
	 input     [NUBITS-1:0] mlt ,
	 input     [NUBITS-1:0] div ,
	 input     [NUBITS-1:0] mod ,                    // int-only
	 input     [NUBITS-1:0] sgn , fsgn,
	 // one-parameter arithmetic operations
	 input     [NUBITS-1:0] neg , negm, fneg, fnegm,
	 input     [NUBITS-1:0] abs , absm, fabs, fabsm,
	 input     [NUBITS-1:0] pst , pstm, fpst, fpstm,
	 input     [NUBITS-1:0] nrm , nrmm,              // int-only
	 input     [NUBITS-1:0] f2i , f2im,
	 // two-parameter logical operations
	 input     [NUBITS-1:0] ann , orr , cor ,        // and, or, xor
	 // one-parameter logical operations
	 input     [NUBITS-1:0] inv , invm,              // not
	 // two-parameter conditional operations
	 input     [NUBITS-1:0] lan , lor ,              // int-only
	 // one-parameter conditional operations
	 input     [NUBITS-1:0] lin , linm,              // int-only
	 // comparison operations
	 input     [NUBITS-1:0] les , fles,
	 input     [NUBITS-1:0] gre , fgre,
	 input     [NUBITS-1:0] equ ,                    // works for int and float
	 // bit-shift operations
	 input     [NUBITS-1:0] shl , shr , srs ,        // <<, >> and >>>
	 // operations from the normalization circuit
	 input     [NUBITS-1:0] smx ,                    // float-only with denorm.
	 // special operations (exponent surgery)
	 input     [NUBITS-1:0] fscl, xpo , xpom,        // F_SCL/SF_SCL, XPO, XPO_M
	 // output
	output reg [NUBITS-1:0] out
);

always @ (*) case (op)
	6'd0   : out =   in2 ;   //   NOP
	6'd1   : out =   in1 ;   //   LOD

	6'd2   : out =   add ;   //   ADD
	6'd3   : out =   smx ;   // F_ADD

	6'd4   : out =   mlt ;   //   MLT
	6'd5   : out =   smx ;   // F_MLT

	6'd6   : out =   div ;   //   DIV
	6'd7   : out =   smx ;   // F_DIV

	6'd8   : out =   mod ;   //   MOD

	6'd9   : out =   sgn ;   //   SGN
	6'd10  : out =  fsgn ;   // F_SGN

	6'd11  : out =   neg ;   //   NEG
	6'd12  : out =   negm;   //   NEG_M
	6'd13  : out =  fneg ;   // F_NEG
	6'd14  : out =  fnegm;   // F_NEG_M

	6'd15  : out =   abs ;   //   ABS
	6'd16  : out =   absm;   //   ABS_M
	6'd17  : out =  fabs ;   // F_ABS
	6'd18  : out =  fabsm;   // F_ABS_M

	6'd19  : out =   pst ;   //   PST
	6'd20  : out =   pstm;   //   PST_M
	6'd21  : out =  fpst ;   // F_PST
	6'd22  : out =  fpstm;   // F_PST_M

	6'd23  : out =   nrm ;   //   NRM
	6'd24  : out =   nrmm;   //   NRM_M

	6'd25  : out =   smx ;   //   I2F
	6'd26  : out =   smx ;   //   I2F_M

	6'd27  : out =   f2i ;   //   F2I
	6'd28  : out =   f2im;   //   F2I_M

	6'd29  : out =   ann ;   //   AND
	6'd30  : out =   orr ;   //   ORR
	6'd31  : out =   cor ;   //   XOR

	6'd32  : out =   inv ;   //   INV
	6'd33  : out =   invm;   //   INV_M

	6'd34  : out =   lan ;   //   LAN
	6'd35  : out =   lor ;   //   LOR

	6'd36  : out =   lin ;   //   LIN
	6'd37  : out =   linm;   //   LIN_M

	6'd38  : out =   les ;   //   LES
	6'd39  : out =  fles ;   // F_LES

	6'd40  : out =   gre ;   //   GRE
	6'd41  : out =  fgre ;   // F_GRE

	6'd42  : out =   equ ;   //   EQU

	6'd43  : out =   shl ;   //   SHL
	6'd44  : out =   shr ;   //   SHR
	6'd45  : out =   srs ;   //   SRS

	6'd46  : out =   smx ;   // F_ROT
	6'd47  : out =   smx ;   // F_SU1
	6'd48  : out =   smx ;   // F_SU2

	6'd49  : out =  fscl ;   // F_SCL / SF_SCL
	6'd50  : out =   xpo ;   // XPO
	6'd51  : out =  xpom ;   // XPO_M

	default: out = {NUBITS{1'bx}};
endcase

endmodule

// ****************************************************************************
// Auxiliary circuits for floating-point operations ***************************
// ****************************************************************************

// equalizes the exponent of two numbers --------------------------------------
// for operations that require mantissas at the same order of magnitude. e.g., F_ADD

module ula_denorm
#(
	parameter MAN = 23,
	parameter EXP =  8,
	parameter G   =  0                                  // extra low bits kept below the mantissa (3 = guard/round/sticky, FROUND 2)
)(
	 input                            neg1, neg2,       // invert the sign
	 input            [MAN+EXP :0]     in1,  in2,
	output reg signed [EXP-1   :0]   e_out,
	output reg signed [MAN+G   :0] sm1_out, sm2_out
);

localparam W = MAN+G;                                   // mantissa + extra bits

// unpack the registered inputs -----------------------------------------------

wire                  s1_in = (neg1) ? ~in1[MAN+EXP] : in1[MAN+EXP];
wire                  s2_in = (neg2) ? ~in2[MAN+EXP] : in2[MAN+EXP];
wire signed [EXP-1:0] e1_in = in1[MAN+EXP-1:MAN];
wire signed [EXP-1:0] e2_in = in2[MAN+EXP-1:MAN];
wire        [MAN-1:0] m1_in = in1[MAN    -1:0  ];
wire        [MAN-1:0] m2_in = in2[MAN    -1:0  ];

// compute the shift ----------------------------------------------------------
// the smaller exponent shifts to match the larger ----------------------------

wire signed [EXP:0] eme    =  e1_in-e2_in;                           // subtraction e1-e2
wire                ege    =  eme     [EXP];                         // save the sign of the subtraction
wire        [EXP:0] shift2 = (ege) ?  {EXP+1{1'b0}} : eme;           // shift of input 2
wire        [EXP:0] shift1 = (ege) ? -eme           : {EXP+1{1'b0}}; // shift of input 1

// take the final exponent ----------------------------------------------------

always @ (*) e_out = (ege) ? e2_in : e1_in; // the final exponent is the larger

// right-shift the mantissa with smaller exp ----------------------------------

// the mantissa is widened by G zero bits before the shift, so what the shift
// pushes out lands in the extra bits instead of being lost (G = 0: plain shift)
wire [W-1:0] m1_ext, m2_ext;
wire [W-1:0] m1_sh  = m1_ext >> shift1;
wire [W-1:0] m2_sh  = m2_ext >> shift2;
wire [W-1:0] m1_out, m2_out;

generate if (G != 0) begin : sticky
	assign m1_ext = {m1_in, {G{1'b0}}};
	assign m2_ext = {m2_in, {G{1'b0}}};
	// the lowest extra bit also collects every bit shifted past it (sticky)
	wire st1 = (shift1 >= W) ? (m1_in != {MAN{1'b0}}) : ((m1_ext << (W - shift1)) != {W{1'b0}});
	wire st2 = (shift2 >= W) ? (m2_in != {MAN{1'b0}}) : ((m2_ext << (W - shift2)) != {W{1'b0}});
	assign m1_out = {m1_sh[W-1:1], m1_sh[0] | st1};
	assign m2_out = {m2_sh[W-1:1], m2_sh[0] | st2};
end else begin : plain
	assign m1_ext = m1_in;
	assign m2_ext = m2_in;
	assign m1_out = m1_sh;
	assign m2_out = m2_sh;
end endgenerate

// compute the signed mantissas -----------------------------------------------

// zero-pad m{1,2}_out (W bits) to match sm{1,2}_out's W+1-bit signed width
always @ (*) sm1_out = (s1_in) ? -{1'b0, m1_out} : {1'b0, m1_out};
always @ (*) sm2_out = (s2_in) ? -{1'b0, m2_out} : {1'b0, m2_out};

endmodule

// multiplexer of the normalization module ------------------------------------
// assists the normalization circuit of a floating-point number ---------------

module ula_nmux
#(
	parameter NCOMP = 2,
	parameter NBITS = 8
)(
	input  [NCOMP-1:0]   A,   B,
	input  [NBITS-1:0] in1, in2,
	output [NBITS-1:0] out
);

assign out = (A==B) ? in1 : in2;

endmodule

// normalization of a floating-point number -----------------------------------
// left-shift until the most significant bit of the mantissa is 1 -------------

// The operators hand in an exponent 2 bits wider than the format, so an
// overflow/underflow is still visible here. FROUND selects what to do with it:
//   0 - legacy: the exponent wraps modulo 2^EXP, zero keeps the operand sign
//   1 - saturate on overflow, flush to canonical zero on underflow (and on zero)
//   2 - as 1, plus round to nearest even on the G extra bits of the mantissa

module ula_norm
#(
	parameter MAN    = 23,
	parameter EXP    =  8,
	parameter FROUND =  0,
	parameter G      =  0                  // extra low bits carried by the mantissa (3 when FROUND = 2)
)(
	 input [MAN+EXP+G+2:0] in,             // {s, e[EXP+1:0], m[MAN+G-1:0]}
	output [MAN+EXP    :0] out
);

localparam W = MAN+G;

wire                    sig = in[MAN+EXP+G+2  ];
wire signed [EXP+1  :0] exp = in[MAN+EXP+G+1:W];
wire        [W-1    :0] man = in[W        -1:0];

wire [EXP-1:0] w [W-1:0];

// normalize the W-bit word (the LSB of the mantissa field keeps weight 2^exp)
wire        [EXP-1:0] sh    =  w[W-2];
wire        [W-1  :0] nrm   =  man << sh;
wire signed [EXP+1:0] e_nrm =  exp - {{2{1'b0}}, sh};

localparam signed [EXP+1:0] EMAX  =  (1 <<< (EXP-1)) - 1;   // largest exponent
localparam signed [EXP+1:0] EZERO = -(1 <<< (EXP-1));       // exponent of the zero encoding

wire                  out_s;
wire signed [EXP-1:0] out_e;
wire        [MAN-1:0] out_m;

generate if (FROUND == 0) begin : legacy
	assign out_s = sig;
	assign out_e = (man == {W{1'b0}}) ? EZERO[EXP-1:0] : e_nrm[EXP-1:0];
	assign out_m = nrm[W-1:G];
end else begin : ranged
	// round to nearest even on the extra bits (nothing to do when G = 0)
	wire        [MAN-1:0] mnt = nrm[W-1:G];
	wire                  inc;
	if (G != 0) begin : rne
		wire grd = nrm[G-1];                                          // first bit below the mantissa
		wire stk = |nrm[G-2:0];                                       // anything below it
		assign inc = grd & (stk | mnt[0]);
	end else begin : trunc
		assign inc = 1'b0;
	end
	wire        [MAN  :0] rnd   = {1'b0, mnt} + {{MAN{1'b0}}, inc};
	wire                  rc    = rnd[MAN];                           // carry out: mantissa was all ones
	wire        [MAN-1:0] m_rnd = (rc) ? {1'b1, {MAN-1{1'b0}}} : rnd[MAN-1:0];
	wire signed [EXP+1:0] e_rnd = e_nrm + {{EXP+1{1'b0}}, rc};
	// canonical zero on a zero or underflowed result, saturate on overflow
	wire                  zero  = (man == {W{1'b0}}) | (e_rnd < EZERO);
	wire                  ovf   = (e_rnd > EMAX);
	assign out_s = (zero) ? 1'b0 : sig;
	assign out_e = (zero) ? EZERO[EXP-1:0] : (ovf) ? EMAX[EXP-1:0] : e_rnd[EXP-1:0];
	assign out_m = (zero) ? {MAN{1'b0}}   : (ovf) ? {MAN{1'b1}}   : m_rnd;
end endgenerate

ula_nmux #(1, EXP) mm1 (man[W-1], 1'b0, {{EXP-1{1'b0}}, {1'b1}}, {EXP{1'b0}}, w[0]);

genvar i;

generate
	for (i = 1; i < W-1; i = i+1) begin : norm
		ula_nmux #(i+1, EXP) mm (man[W-1:W-1-i], {i+1{1'b0}}, i[EXP-1:0] + {{EXP-1{1'b0}}, {1'b1}}, w[i-1], w[i]);
	end
endgenerate

assign out = {out_s, out_e, out_m};

endmodule

// multiplexer of operations that require normalization -----------------------

module norm_mux
#(
	parameter NUBITS = 32,
	parameter NBMANT = 23,
	parameter NBEXPO =  8,
	parameter FROUND =  0,
	parameter G      =  0
)(
	 input [         5:0] op  ,
	 input [NUBITS+G+1:0] fadd,           // operator outputs carry a 2-bit-wider exponent and G extra mantissa bits
	 input [NUBITS+G+1:0] fmlt,
	 input [NUBITS+G+1:0] fdiv,
	 input [NUBITS+G+1:0] i2f , i2fm,
	 input [NUBITS+G+1:0] frot,
	output [NUBITS  -1:0] out
);

// input multiplexer
reg [NUBITS+G+1:0] imux_out;

always @ (*) case (op)
	6'd3   : imux_out =  fadd ;   // F_ADD
	6'd5   : imux_out =  fmlt ;   // F_MLT
	6'd7   : imux_out =  fdiv ;   // F_DIV
	6'd25  : imux_out =   i2f ;   //   I2F
	6'd26  : imux_out =   i2fm;   //   I2F_M
	6'd46  : imux_out =  frot ;   // F_ROT
	6'd47  : imux_out =  fadd ;   // F_SU1 (uses the same addition circuit)
	6'd48  : imux_out =  fadd ;   // F_SU2 (uses the same addition circuit)
	default: imux_out = {NUBITS+G+2{1'bx}};
endcase

// perform the normalization
ula_norm #(NBMANT,NBEXPO,FROUND,G) ula_norm (imux_out, out);

endmodule

// ****************************************************************************
// Two-parameter arithmetic operations ****************************************
// ****************************************************************************

// ADD - fixed-point addition -------------------------------------------------

module ula_add
#(
	parameter NUBITS = 32
)(
	 input signed [NUBITS-1:0] in1, in2,
	output signed [NUBITS-1:0] out
);

assign out = in1 + in2;

endmodule

// F_ADD - floating-point addition --------------------------------------------

module ula_fadd
#(
	parameter MAN    = 23,
	parameter EXP    =  8,
	parameter FROUND =  0,
	parameter G      =  0
)(
	input  signed [EXP-1     :0] e_in,
	input  signed [MAN+G     :0] sm1_in, sm2_in,              // already comes in with one extra sign bit
	output        [MAN+EXP+G+2:0] out                         // exponent 2 bits wider (see ula_norm)
);

localparam W = MAN+G;

wire signed [W+1:0] soma  =  sm1_in + sm2_in;                 // add one more bit to avoid overflow
wire signed [W+1:0] m     = (soma[W+1]) ? -soma : soma;       // compute abs() of the mantissa
wire                carry =  m[W];                            // the sum outgrew the mantissa

wire                  s_out  = soma[W+1];
wire signed [EXP+1:0] e_wide = {{2{e_in[EXP-1]}}, e_in};
wire signed [EXP+1:0] e_out;
wire        [W-1  :0] m_out;

generate if (FROUND == 0) begin : legacy
	// always shift right by one: the LSB is lost even when there was no carry
	assign e_out = e_wide + {{EXP+1{1'b0}}, 1'b1};
	assign m_out = m[W:1];
end else if (G == 0) begin : keep_lsb
	// shift (and bump the exponent) only when the sum actually carried out
	assign e_out = e_wide + {{EXP+1{1'b0}}, carry};
	assign m_out = (carry) ? m[W:1] : m[W-1:0];
end else begin : keep_lsb_sticky
	// same, folding the bit that falls off the shift into the sticky bit
	assign e_out = e_wide + {{EXP+1{1'b0}}, carry};
	assign m_out = (carry) ? {m[W:2], m[1] | m[0]} : m[W-1:0];
end endgenerate

assign out = {s_out, e_out, m_out};

endmodule

// MLT - fixed-point multiplication -------------------------------------------

module ula_mlt
#(
	parameter NUBITS = 32
)(
	 input signed [NUBITS-1:0] in1, in2,
	output signed [NUBITS-1:0] out
);

assign out = in1 * in2;

endmodule

// F_MLT - floating-point multiplication --------------------------------------

module ula_fmlt
#(
	parameter MAN    = 23,
	parameter EXP    =  8,
	parameter FROUND =  0,
	parameter G      =  0
)(
	 input     [MAN+EXP :0] in1, in2,
	output     [MAN+EXP+G+2:0] out                            // exponent 2 bits wider (see ula_norm)
);

localparam W = MAN+G;
localparam signed [EXP+1:0] EZERO = -(1 <<< (EXP-1));

// separate the parts of the input signals ------------------------------------

wire                  s1 = in1[MAN+EXP      ];
wire                  s2 = in2[MAN+EXP      ];
wire signed [EXP+1:0] e1 = {{2{in1[MAN+EXP-1]}}, in1[MAN+EXP-1:MAN]};
wire signed [EXP+1:0] e2 = {{2{in2[MAN+EXP-1]}}, in2[MAN+EXP-1:MAN]};
wire        [MAN-1:0] m1 = in1[MAN    -1:0  ];
wire        [MAN-1:0] m2 = in2[MAN    -1:0  ];

// compute the sign -----------------------------------------------------------

wire s_mlt = (s1 != s2);

// compute the mantissa value -------------------------------------------------

wire [2*MAN-1:0] mult  = m1 * m2;
wire             top   = mult[2*MAN-1];                       // does the product occupy the top bit?

// compute the exponent value (for a product that occupies the top bit) -------

wire signed [EXP+1:0] e_top = e1 + e2 + MAN[EXP+1:0];

// finalize -------------------------------------------------------------------

wire                  s_out;
wire signed [EXP+1:0] e_out;
wire        [W-1  :0] m_out;

generate if (FROUND == 0) begin : legacy
	// top MAN bits of the product; a zero product is encoded here directly
	wire [MAN-1:0] m_top = mult[2*MAN-1:MAN];
	wire           nz    = (m_top != {MAN{1'b0}});
	assign s_out = (nz) ? s_mlt : 1'b0;
	assign e_out = (nz) ? e_top : EZERO;
	assign m_out = m_top;
end else if (G == 0) begin : keep_lsb
	// when the top bit is 0 take the slice one bit lower (and the exponent one less)
	assign s_out = s_mlt;
	assign e_out = e_top - {{EXP+1{1'b0}}, ~top};
	assign m_out = (top) ? mult[2*MAN-1:MAN] : mult[2*MAN-2:MAN-1];
end else begin : keep_lsb_sticky
	// same, with two more product bits and the OR of the rest as sticky (G = 3)
	assign s_out = s_mlt;
	assign e_out = e_top - {{EXP+1{1'b0}}, ~top};
	assign m_out = (top) ? {mult[2*MAN-1:MAN-2], |mult[MAN-3:0]} : {mult[2*MAN-2:MAN-3], |mult[MAN-4:0]};
end endgenerate

assign out = {s_out, e_out, m_out};

endmodule

// DIV - fixed-point division -------------------------------------------------

module ula_div
#(
	parameter NUBITS = 32
)(
	 input signed [NUBITS-1:0] in1, in2,
	output signed [NUBITS-1:0] out
);

assign out = in1 / in2;

endmodule

// F_DIV - floating-point division --------------------------------------------

module ula_fdiv
#(
	parameter MAN    = 23,
	parameter EXP    =  8,
	parameter FROUND =  0,
	parameter G      =  0
)(
	 input [MAN+EXP :0] in1, in2,
	output [MAN+EXP+G+2:0] out                                // exponent 2 bits wider (see ula_norm)
);

localparam W = MAN+G;

wire                  s1 = in1[MAN+EXP      ];
wire                  s2 = in2[MAN+EXP      ];
wire signed [EXP+1:0] e1 = {{2{in1[MAN+EXP-1]}}, in1[MAN+EXP-1:MAN]};
wire signed [EXP+1:0] e2 = {{2{in2[MAN+EXP-1]}}, in2[MAN+EXP-1:MAN]};
wire        [MAN-1:0] m1 = in1[MAN    -1:0  ];
wire        [MAN-1:0] m2 = in2[MAN    -1:0  ];

wire                  s_out = (s1 != s2);
wire signed [EXP+1:0] e_dif = e1 - e2 - MAN[EXP+1:0];
wire signed [EXP+1:0] e_out;
wire        [W-1  :0] m_out;

generate if (FROUND == 0) begin : legacy
	// MAN quotient bits; when m1 < m2 the top one is 0 and the LSB is lost
	wire [2*MAN-2:0] m1_ext = {m1, {MAN-1{1'b0}}};
	wire [2*MAN-2:0] div    =  m1_ext / m2;
	assign e_out = e_dif + {{EXP+1{1'b0}}, 1'b1};
	assign m_out = div[MAN-1:0];
end else if (G == 0) begin : keep_lsb
	// one more quotient bit, so the slice can follow the top bit
	wire [2*MAN-1:0] m1_ext = {m1, {MAN{1'b0}}};
	wire [2*MAN-1:0] div    =  m1_ext / m2;                   // < 2^(MAN+1) for a normalized divisor
	wire             top    =  div[MAN];
	assign e_out = e_dif + {{EXP+1{1'b0}}, top};
	assign m_out = (top) ? div[MAN:1] : div[MAN-1:0];
end else begin : keep_lsb_sticky
	// three more quotient bits: guard, round and a partial sticky (an exact
	// sticky would need the remainder, i.e. a second divider)
	wire [2*MAN+2:0] m1_ext = {m1, {MAN+3{1'b0}}};
	wire [2*MAN+2:0] div    =  m1_ext / m2;                   // < 2^(MAN+4) for a normalized divisor
	wire             top    =  div[MAN+3];
	assign e_out = e_dif + {{EXP+1{1'b0}}, top};
	assign m_out = (top) ? {div[MAN+3:2], div[1] | div[0]} : div[MAN+2:0];
end endgenerate

assign out = {s_out, e_out, m_out};

endmodule

// MOD - division remainder ---------------------------------------------------

module ula_mod
#(
	parameter NUBITS = 32
)(
	 input signed [NUBITS-1:0] in1, in2,
	output signed [NUBITS-1:0] out
);

assign out = in1 % in2;

endmodule

// SGN - takes the sign of the first argument ---------------------------------

module ula_sgn
#(
	parameter NUBITS = 32
)(
	 input signed [NUBITS-1:0] in1, in2,
	output signed [NUBITS-1:0] out
);

assign out =  (in1[NUBITS-1] == in2[NUBITS-1]) ? in2 : -in2;

endmodule

// F_SGN - takes the sign of the first argument in float ----------------------

module ula_fsgn
#(
	parameter MAN    = 23,
	parameter EXP    =  8,
	parameter FROUND =  0
)(
	 input [MAN+EXP:0] in1, in2,
	output [MAN+EXP:0] out
);

wire signed [EXP-1:0] e_out = in2[EXP+MAN-1:MAN];
wire        [MAN-1:0] m_out = in2[MAN    -1:  0];
wire                  zero  = (FROUND != 0) & (m_out == {MAN{1'b0}}); // FROUND >= 1: zero stays canonical (+0)
wire                  s_out = (zero) ? 1'b0 : in1[EXP+MAN];

assign out = {s_out, e_out, m_out};

endmodule

// ****************************************************************************
// One-parameter arithmetic operations ****************************************
// ****************************************************************************

// NEG - negation of an integer -----------------------------------------------

module ula_neg
#(
	parameter NUBITS = 32
)(
	 input signed [NUBITS-1:0] in,
	output signed [NUBITS-1:0] out
);

assign out = -in;

endmodule

// F_NEG - negation of a floating-point number --------------------------------

module ula_fneg
#(
	parameter MAN    = 23,
	parameter EXP    =  8,
	parameter FROUND =  0
)(
	 input [MAN+EXP:0] in,
	output [MAN+EXP:0] out
);

wire                  s_in = in[MAN+EXP      ];
wire signed [EXP-1:0] e_in = in[MAN+EXP-1:MAN];
wire        [MAN-1:0] m_in = in[MAN    -1:0  ];

wire                  zero  = (FROUND != 0) & (m_in == {MAN{1'b0}}); // FROUND >= 1: zero stays canonical (+0)
wire                  s_out = (zero) ? 1'b0 : ~s_in;
wire signed [EXP-1:0] e_out =  e_in;
wire        [MAN-1:0] m_out =  m_in;

assign out = {s_out, e_out, m_out};

endmodule

// ABS - absolute value of an integer -----------------------------------------

module ula_abs
#(
	parameter NUBITS = 32
)(
	 input [NUBITS-1:0] in,
	output [NUBITS-1:0] out
);

assign out = (in[NUBITS-1]) ? -in : in;

endmodule

// F_ABS - absolute value of a floating-point number --------------------------

module ula_fabs
#(
	parameter MAN = 23,
	parameter EXP = 8
)(
	 input [MAN+EXP:0] in,
	output [MAN+EXP:0] out
);

wire                  s_out = 0;
wire signed [EXP-1:0] e_out = in[EXP+MAN-1:MAN];
wire        [MAN-1:0] m_out = in[MAN    -1:  0];

assign out = {s_out, e_out, m_out};

endmodule

// PST - zero if negative -----------------------------------------------------

module ula_pst
#(
	parameter NUBITS = 32
)(
	 input [NUBITS-1:0] in,
	output [NUBITS-1:0] out
);

assign out = (in[NUBITS-1]) ? {NUBITS{1'b0}} : in;

endmodule

// F_PST - zero if negative for float -----------------------------------------

module ula_fpst
#(
	parameter MAN = 23,
	parameter EXP = 8
)(
	 input [MAN+EXP:0] in,
	output [MAN+EXP:0] out
);

assign out = (in[MAN+EXP]) ? {1'b0, 1'b1, {MAN+EXP-1{1'b0}}} : in;

endmodule

// NRM - division by a constant -----------------------------------------------

module ula_nrm
#(
	parameter         NUBITS = 32,
	parameter signed  NUGAIN =  1
)(
	 input    signed [NUBITS-1:0] in,
	output    signed [NUBITS-1:0] out
);

assign out = in/NUGAIN;

endmodule

// I2F - converts int to float ------------------------------------------------

module ula_i2f
#(
	parameter MAN = 23,
	parameter EXP =  8,
	parameter G   =  0
)(
	input  signed [MAN-1      :0] in,
	output        [MAN+EXP+G+2:0] out                         // exponent 2 bits wider, G extra mantissa bits (see ula_norm)
);

wire                  i2f_s = in[MAN-1];
wire signed [EXP+1:0] i2f_e = 0;
wire        [MAN-1:0] i2f_m = (i2f_s) ? -in : in;
wire        [MAN+G-1:0] i2f_x;                               // exact: the G extra bits are zero

generate if (G != 0) begin : ext assign i2f_x = {i2f_m, {G{1'b0}}}; end
         else            begin : nox assign i2f_x = i2f_m;             end endgenerate

assign out = {i2f_s, i2f_e, i2f_x};

endmodule

// F2I - converts float to int ------------------------------------------------

module ula_f2i
#(
	parameter MAN = 23,
	parameter EXP =  8
)(
	input             [MAN+EXP :0] in,
	output reg signed [MAN+EXP :0] out
);

wire           s = in[MAN+EXP      ];
wire [EXP-1:0] e = in[MAN+EXP-1:MAN];
wire [MAN-1:0] m = in[MAN    -1:  0];

wire        [EXP-1:0] shift = (e[EXP-1]) ? -e : e;
// shift the magnitude (logical), then apply the sign — truncates toward zero,
// as C/IEEE float->int conversion requires (a signed >>> would floor instead).
// m_ext widens m (MAN bits) to the final mag width (MAN+EXP+1) so the shift
// operates at the destination width — Verilator otherwise warns WIDTHEXPAND.
wire        [MAN+EXP:0] m_ext = {{(EXP+1){1'b0}}, m};
wire        [MAN+EXP:0] mag   = (e[EXP-1]) ? (m_ext >> shift) : (m_ext << shift);

always @ (*) out  = (s) ? -mag : mag;

endmodule

// ****************************************************************************
// Two-parameter logical operations *******************************************
// ****************************************************************************

// AND - bitwise AND (&) ------------------------------------------------------

module ula_and
#(
	parameter NUBITS = 32
)(
	 input [NUBITS-1:0] in1, in2,
	output [NUBITS-1:0] out
);

assign out = in1 & in2;

endmodule

// ORR - bitwise OR (|) -------------------------------------------------------

module ula_or
#(
	parameter NUBITS = 32
)(
	 input [NUBITS-1:0] in1, in2,
	output [NUBITS-1:0] out
);

assign out = in1 | in2;

endmodule

// XOR - bitwise XOR (^) ------------------------------------------------------

module ula_xor
#(
	parameter NUBITS = 32
)(
	 input [NUBITS-1:0] in1, in2,
	output [NUBITS-1:0] out
);

assign out = (in1 ^ in2);

endmodule

// ****************************************************************************
// One-parameter logical operations *******************************************
// ****************************************************************************

// INV - bitwise inversion (~) ------------------------------------------------

module ula_inv
#(
	parameter NUBITS = 32
)(
	 input signed [NUBITS-1:0] in,
	output signed [NUBITS-1:0] out
);

assign out = ~in;

endmodule

// ****************************************************************************
// Two-parameter conditional operations ***************************************
// ****************************************************************************

// LAN - if one of the conditions is zero -> outputs zero (&&) ----------------

module ula_lan
#(
	parameter  NUBITS = 32
)(
	 input    [NUBITS-1:0] in1, in2,
	output    [NUBITS-1:0] out
);

assign out = ((in1 == {NUBITS{1'b0}}) || (in2 == {NUBITS{1'b0}})) ? {NUBITS{1'b0}} : {{NUBITS-1{1'b0}}, 1'b1};

endmodule

// LOR - if one of the conditions is one -> outputs one (||) ------------------

module ula_lor
#(
	parameter  NUBITS = 32
)(
	 input    [NUBITS-1:0] in1, in2,
	output    [NUBITS-1:0] out
);

assign out = ((in1 == {NUBITS{1'b0}}) && (in2 == {NUBITS{1'b0}})) ? {NUBITS{1'b0}} : {{NUBITS-1{1'b0}}, 1'b1};

endmodule

// ****************************************************************************
// One-parameter conditional operations ***************************************
// ****************************************************************************

// LIN - inverts the condition ------------------------------------------------

module ula_lin
#(
	parameter NUBITS = 32
)(
	 input   [NUBITS-1:0] in,
	output   [NUBITS-1:0] out
);

assign out = (in  == {NUBITS{1'b0}}) ? {{NUBITS-1{1'b0}}, 1'b1} : {NUBITS{1'b0}};

endmodule

// ****************************************************************************
// Comparison operations ******************************************************
// ****************************************************************************

// LES - less than ------------------------------------------------------------

module ula_les
#(
	parameter NUBITS = 32
)(
	 input signed [NUBITS-1:0] in1, in2,
	output        [NUBITS-1:0] out
);

// zero-extend the 1-bit comparison result to NUBITS so Verilator does not
// warn WIDTHEXPAND on the ASSIGNW. Semantics are unchanged: false -> all
// zeros, true -> ...0001.
assign out = {{(NUBITS-1){1'b0}}, (in1 < in2)};

endmodule

// FLES - floating-point less than --------------------------------------------

module ula_fles
#(
	parameter NUBITS = 32,
	parameter NBMANT = 23
)(
	 input signed [NBMANT  :0] in1, in2,
	output        [NUBITS-1:0] out
);

assign out = {{(NUBITS-1){1'b0}}, (in1 < in2)};

endmodule

// GRE - greater than ---------------------------------------------------------

module ula_gre
#(
	parameter NUBITS = 32
)(
	 input signed [NUBITS-1:0] in1, in2,
	output        [NUBITS-1:0] out
);

assign out = {{(NUBITS-1){1'b0}}, (in1 > in2)};

endmodule

// FGRE - floating-point greater than -----------------------------------------

module ula_fgre
#(
	parameter NUBITS = 32,
	parameter NBMANT = 23
)(
	 input signed [NBMANT  :0] in1, in2,
	output        [NUBITS-1:0] out
);

assign out = {{(NUBITS-1){1'b0}}, (in1 > in2)};

endmodule

// EQU - equal to -------------------------------------------------------------

module ula_equ
#(
	parameter NUBITS = 32
)(
	 input   [NUBITS-1:0] in1, in2,
	output   [NUBITS-1:0] out
);

assign out = {{(NUBITS-1){1'b0}}, (in1 == in2)};

endmodule

// ****************************************************************************
// Bit-shift operations *******************************************************
// ****************************************************************************

// SHL - left shift -----------------------------------------------------------

module ula_shl
#(
	parameter   NUBITS = 32
)(
	 input     [NUBITS-1:0] in1, in2,
	output     [NUBITS-1:0] out
);

assign out = in1 << in2;

endmodule

// SHR - right shift ----------------------------------------------------------

module ula_shr
#(
	parameter   NUBITS = 32
)(
	 input     [NUBITS-1:0] in1, in2,
	output     [NUBITS-1:0] out
);

assign out = in1 >> in2;

endmodule

// SRS - arithmetic right shift -----------------------------------------------

module ula_srs
#(
	parameter      NUBITS = 32
)(
	 input signed [NUBITS-1:0] in1,
	 input        [NUBITS-1:0] in2,
	output signed [NUBITS-1:0] out
);

assign out = in1 >>> in2;

endmodule

// ****************************************************************************
// Special operations *********************************************************
// ****************************************************************************

// F_ROT - nearest power of 2 to the square root ------------------------------

module ula_frot
#(
	parameter MAN = 23,
	parameter EXP =  8,
	parameter G   =  0
)(
	 input [MAN+EXP    :0] in,
	output [MAN+EXP+G+2:0] out                                // exponent 2 bits wider, G extra mantissa bits (see ula_norm)
);

wire                  s_in  = in[MAN+EXP      ];
wire signed [EXP-1:0] e_in  = in[MAN+EXP-1:MAN];
wire        [MAN-1:0] m_in  = in[MAN    -1:0  ];

wire                  s_out = s_in;
wire signed [EXP-1:0] e_hlf = (e_in+(MAN-1))/2;
wire signed [EXP+1:0] e_out = {{2{e_hlf[EXP-1]}}, e_hlf};
wire        [MAN-1:0] m_one = 1;
wire        [MAN+G-1:0] m_out;                               // exact: the G extra bits are zero

generate if (G != 0) begin : ext assign m_out = {m_one, {G{1'b0}}}; end
         else            begin : nox assign m_out = m_one;             end endgenerate

assign out = {s_out, e_out, m_out};

endmodule

// F_SCL - scale a float by a power of two: out = in2 * 2^in1 -----------------
// in1 is a signed integer k; the exponent field gets k added to it (value =
// mantissa*2^e), saturating to the EXP-bit range. The mantissa is unchanged, so
// a normalized input stays normalized (no re-normalization needed).

module ula_scl
#(
	parameter MAN = 23,
	parameter EXP =  8
)(
	 input  signed [MAN+EXP:0] in1,                 // k (signed integer)
	 input         [MAN+EXP:0] in2,                 // x (normalized float)
	output         [MAN+EXP:0] out
);

wire                  s = in2[MAN+EXP      ];
wire signed [EXP-1:0] e = in2[MAN+EXP-1:MAN];
wire        [MAN-1:0] m = in2[MAN    -1:0  ];

wire signed [MAN+EXP:0] esum = $signed(e) + in1;            // e + k (wide signed)
localparam signed [MAN+EXP:0] EMAX =  (1 <<< (EXP-1)) - 1;  //  127 for EXP=8
localparam signed [MAN+EXP:0] EMIN = -(1 <<< (EXP-1));      // -128 for EXP=8
wire signed [EXP-1:0] e_out = (esum > EMAX) ? EMAX[EXP-1:0] :
                              (esum < EMIN) ? EMIN[EXP-1:0] : esum[EXP-1:0];

assign out = {s, e_out, m};

endmodule

// XPO - base-2 exponent of a float as a signed int ---------------------------
// value = mantissa_int * 2^e with mantissa_int in [2^(MAN-1),2^MAN) when
// normalized, so floor(log2|x|) = e + (MAN-1). out is that integer (0 if x==0).

module ula_xpo
#(
	parameter MAN = 23,
	parameter EXP =  8
)(
	 input         [MAN+EXP:0] in,
	output signed [MAN+EXP:0] out
);

wire signed [EXP-1:0]   e    = in[MAN+EXP-1:MAN];
wire                    nz   = |in[MAN+EXP-1:0];            // nonzero magnitude
wire signed [MAN+EXP:0] bias = MAN-1;                       // 22, full-width signed

// compute the signed sum in its OWN signed wire first (like ula_scl's esum) so e
// is sign-extended; doing it inside the ?: would pick up the unsigned else branch.
wire signed [MAN+EXP:0] xval = $signed(e) + bias;          // e + (MAN-1)

assign out = nz ? xval : {(MAN+EXP+1){1'b0}};

endmodule

// ****************************************************************************
// Main Circuit ***************************************************************
// ****************************************************************************

module ula
#(
	// General
	parameter                     NUBITS = 32,
	parameter                     NBMANT = 23,
	parameter                     NBEXPO =  8,
	parameter signed [NUBITS-1:0] NUGAIN = 64,
	parameter                     NBOPCO =  7,
	// float rounding level (#FROUND): 0 legacy (truncate, exponent wraps, bit-identical
	// to the original datapath), 1 keep the LSB before normalization + saturate/flush
	// + canonical zero, 2 as 1 + round to nearest even (guard/round/sticky bits)
	parameter                     FROUND =  0,

	// two-parameter arithmetic operations
	parameter   ADD   = 0,
	parameter F_ADD   = 0,
	parameter   MLT   = 0,
	parameter F_MLT   = 0,
	parameter   DIV   = 0,
	parameter F_DIV   = 0,
	parameter   MOD   = 0,
	parameter   SGN   = 0,
	parameter F_SGN   = 0,

	// one-parameter arithmetic operations
	parameter   NEG   = 0,
	parameter   NEG_M = 0,
	parameter F_NEG   = 0,
	parameter F_NEG_M = 0,
	parameter   ABS   = 0,
	parameter   ABS_M = 0,
	parameter F_ABS   = 0,
	parameter F_ABS_M = 0,
	parameter   PST   = 0,
	parameter   PST_M = 0,
	parameter F_PST   = 0,
	parameter F_PST_M = 0,
	parameter   NRM   = 0,
	parameter   NRM_M = 0,
	parameter   I2F   = 0,
	parameter   I2F_M = 0,
	parameter   F2I   = 0,
	parameter   F2I_M = 0,

	// two-parameter logical operations
	parameter   AND   = 0,
	parameter   ORR   = 0,
	parameter   XOR   = 0,

	// one-parameter logical operations
	parameter   INV   = 0,
	parameter   INV_M = 0,

	// two-parameter conditional operations
	parameter   LAN   = 0,
	parameter   LOR   = 0,

	// one-parameter conditional operations
	parameter   LIN   = 0,
	parameter   LIN_M = 0,

	// comparison operations
	parameter   LES   = 0,
	parameter F_LES   = 0,
	parameter   GRE   = 0,
	parameter F_GRE   = 0,
	parameter   EQU   = 0,

	// bit-shift operations
	parameter   SHL   = 0,
	parameter   SHR   = 0,
	parameter   SRS   = 0,

	// special operations
	parameter F_ROT   = 0,
	parameter F_SU1   = 0,
	parameter F_SU2   = 0,
	parameter F_SCL   = 0,
	parameter XPO     = 0,
	parameter XPO_M   = 0)
(
	input         [       5:0] op,
	input  signed [NUBITS-1:0] in1, in2,
	output signed [NUBITS-1:0] out
);

// floating-point rounding level ----------------------------------------------

// FROUND 2 carries G = 3 extra bits (guard, round, sticky) below the mantissa
// from the denormalizer / operators up to the normalizer, which rounds them off.
// The operators also hand the normalizer an exponent 2 bits wider than the
// format (NUBITS+G+2 wires), so overflow/underflow can be detected there.
localparam G = (FROUND == 2) ? 3 : 0;

// floating-point denormalization circuit -------------------------------------

wire signed [NBEXPO-1:0] e_out;                       // normalized exponent
wire signed [NBMANT+G:0] sm1_out, sm2_out;            // normalized mantissas (+ G extra bits)
// F_SU1/F_SU2 are 32-bit parameters; cast to 1 bit with `!= 0` so the AND
// with the 1-bit op-equality stays 1 bit and matches the 1-bit su{1,2} LHS.
wire					 su1 = (F_SU1 != 0) & (op == 6'd47); // invert sign of in1 for F_SU1
wire                     su2 = (F_SU2 != 0) & (op == 6'd48); // invert sign of in2 for F_SU2

generate if ((F_ADD | F_SU1 | F_SU2 | F_GRE | F_LES) != 0) begin : op_denorm ula_denorm #(NBMANT,NBEXPO,G) denorm(su1, su2, in1, in2, e_out, sm1_out, sm2_out); end endgenerate

// ADD ------------------------------------------------------------------------

wire signed [NUBITS-1:0] add;

generate if ((ADD) != 0) begin : op_add ula_add #(NUBITS) my_add(in1, in2, add); end else begin : op_add assign add = {NUBITS{1'bx}}; end endgenerate

// F_ADD ----------------------------------------------------------------------

wire signed [NUBITS+G+1:0] fadd;

generate if ((F_ADD | F_SU1 | F_SU2) != 0) begin : op_fadd ula_fadd #(NBMANT,NBEXPO,FROUND,G) my_fadd(e_out, sm1_out, sm2_out, fadd); end else begin : op_fadd assign fadd = {NUBITS+G+2{1'bx}}; end endgenerate

// MLT ------------------------------------------------------------------------

wire signed [NUBITS-1:0] mlt;

generate if ((MLT) != 0) begin : op_mlt ula_mlt #(NUBITS) my_mlt(in1, in2, mlt); end else begin : op_mlt assign mlt = {NUBITS{1'bx}}; end endgenerate

// F_MLT ----------------------------------------------------------------------

wire signed [NUBITS+G+1:0] fmlt;

generate if ((F_MLT) != 0) begin : op_fmlt ula_fmlt #(NBMANT,NBEXPO,FROUND,G) my_fmlt(in1 ,in2 , fmlt); end else begin : op_fmlt assign fmlt = {NUBITS+G+2{1'bx}}; end endgenerate

// DIV ------------------------------------------------------------------------

wire signed [NUBITS-1:0] div;

generate if ((DIV) != 0) begin : op_div ula_div #(NUBITS) my_div(in1, in2, div); end else begin : op_div assign div = {NUBITS{1'bx}}; end endgenerate

// F_DIV ----------------------------------------------------------------------

wire signed [NUBITS+G+1:0] fdiv;

generate if ((F_DIV) != 0) begin : op_fdiv ula_fdiv #(NBMANT,NBEXPO,FROUND,G) my_fdiv(in1, in2, fdiv); end else begin : op_fdiv assign fdiv = {NUBITS+G+2{1'bx}}; end endgenerate

// MOD ------------------------------------------------------------------------

wire signed [NUBITS-1:0] mod;

generate if ((MOD) != 0) begin : op_mod ula_mod #(NUBITS) my_mod(in1, in2, mod); end else begin : op_mod assign mod = {NUBITS{1'bx}}; end endgenerate

// SGN ------------------------------------------------------------------------

wire signed [NUBITS-1:0] sgn;

generate if ((SGN) != 0) begin : op_sgn ula_sgn #(NUBITS) my_sgn(in1, in2, sgn); end else begin : op_sgn assign sgn = {NUBITS{1'bx}}; end endgenerate

// F_SGN ----------------------------------------------------------------------

wire signed [NUBITS-1:0] fsgn;

generate if ((F_SGN) != 0) begin : op_fsgn ula_fsgn #(NBMANT,NBEXPO,FROUND) my_fsgn(in1, in2, fsgn); end else begin : op_fsgn assign fsgn = {NUBITS{1'bx}}; end endgenerate

// NEG ------------------------------------------------------------------------

wire signed [NUBITS-1:0] neg;

generate if ((NEG) != 0) begin : op_neg ula_neg #(NUBITS) my_neg(in2, neg); end else begin : op_neg assign neg = {NUBITS{1'bx}}; end endgenerate

// NEG_M ----------------------------------------------------------------------

wire signed [NUBITS-1:0] negm;

generate if ((NEG_M) != 0) begin : op_negm ula_neg #(NUBITS) my_negm(in1, negm ); end else begin : op_negm assign negm = {NUBITS{1'bx}}; end endgenerate

// F_NEG ----------------------------------------------------------------------

wire signed [NUBITS-1:0] fneg;

generate if ((F_NEG) != 0) begin : op_fneg ula_fneg #(NBMANT,NBEXPO,FROUND) my_fneg(in2, fneg); end else begin : op_fneg assign fneg = {NUBITS{1'bx}}; end endgenerate

// F_NEG_M --------------------------------------------------------------------

wire signed [NUBITS-1:0] fnegm;

generate if ((F_NEG_M) != 0) begin : op_fnegm ula_fneg #(NBMANT,NBEXPO,FROUND) my_fnegm(in1, fnegm); end else begin : op_fnegm assign fnegm = {NUBITS{1'bx}}; end endgenerate

// ABS ------------------------------------------------------------------------

wire signed [NUBITS-1:0] abs;

generate if ((ABS) != 0) begin : op_abs ula_abs #(NUBITS) my_abs(in2, abs); end else begin : op_abs assign abs = {NUBITS{1'bx}}; end endgenerate

// ABS_M ----------------------------------------------------------------------

wire signed [NUBITS-1:0] absm;

generate if ((ABS_M) != 0) begin : op_absm ula_abs #(NUBITS) my_absm(in1, absm); end else begin : op_absm assign absm = {NUBITS{1'bx}}; end endgenerate

// F_ABS ----------------------------------------------------------------------

wire signed [NUBITS-1:0] fabs;

generate if ((F_ABS) != 0) begin : op_fabs ula_fabs #(NBMANT,NBEXPO) my_fabs(in2, fabs); end else begin : op_fabs assign fabs = {NUBITS{1'bx}}; end endgenerate

// F_ABS_M --------------------------------------------------------------------

wire signed [NUBITS-1:0] fabsm;

generate if ((F_ABS_M) != 0) begin : op_fabsm ula_fabs #(NBMANT,NBEXPO) my_fabsm(in1, fabsm); end else begin : op_fabsm assign fabsm = {NUBITS{1'bx}}; end endgenerate

// PST ------------------------------------------------------------------------

wire signed [NUBITS-1:0] pst;

generate if ((PST) != 0) begin : op_pst ula_pst #(NUBITS) my_pst(in2, pst); end else begin : op_pst assign pst = {NUBITS{1'bx}}; end endgenerate

// PST_M ----------------------------------------------------------------------

wire signed [NUBITS-1:0] pstm;

generate if ((PST_M) != 0) begin : op_pstm ula_pst #(NUBITS) my_pstm(in1, pstm); end else begin : op_pstm assign pstm = {NUBITS{1'bx}}; end endgenerate

// F_PST ----------------------------------------------------------------------

wire signed [NUBITS-1:0] fpst;

generate if ((F_PST) != 0) begin : op_fpst ula_fpst #(NBMANT,NBEXPO) my_fpst(in2, fpst); end else begin : op_fpst assign fpst = {NUBITS{1'bx}}; end endgenerate

// F_PST_M --------------------------------------------------------------------

wire signed [NUBITS-1:0] fpstm;

generate if ((F_PST_M) != 0) begin : op_fpstm ula_fpst #(NBMANT,NBEXPO) my_fpstm(in1, fpstm); end else begin : op_fpstm assign fpstm = {NUBITS{1'bx}}; end endgenerate

// NRM ------------------------------------------------------------------------

wire signed [NUBITS-1:0] nrm;

generate if ((NRM) != 0) begin : op_nrm ula_nrm #(NUBITS,NUGAIN) my_nrm(in2, nrm); end else begin : op_nrm assign nrm = {NUBITS{1'bx}}; end endgenerate

// NRM_M ----------------------------------------------------------------------

wire signed [NUBITS-1:0] nrmm;

generate if ((NRM_M) != 0) begin : op_nrmm ula_nrm #(NUBITS,NUGAIN) my_nrmm(in1, nrmm); end else begin : op_nrmm assign nrmm = {NUBITS{1'bx}}; end endgenerate

// I2F ------------------------------------------------------------------------

wire signed [NUBITS+G+1:0] i2f;

generate if ((I2F) != 0) begin : op_i2f ula_i2f #(NBMANT,NBEXPO,G) my_i2f (in2[NBMANT-1:0], i2f); end else begin : op_i2f assign i2f = {NUBITS+G+2{1'bx}}; end endgenerate

// I2F_M ----------------------------------------------------------------------

wire signed [NUBITS+G+1:0] i2fm;

generate if ((I2F_M) != 0) begin : op_i2fm ula_i2f #(NBMANT,NBEXPO,G) my_i2fm(in1[NBMANT-1:0], i2fm); end else begin : op_i2fm assign i2fm = {NUBITS+G+2{1'bx}}; end endgenerate

// F2I ------------------------------------------------------------------------

wire signed [NUBITS-1:0] f2i;

generate if ((F2I) != 0) begin : op_f2i ula_f2i #(NBMANT,NBEXPO) my_f2i (in2, f2i); end else begin : op_f2i assign f2i = {NUBITS{1'bx}}; end endgenerate

// F2I_M ----------------------------------------------------------------------

wire signed [NUBITS-1:0] f2im;

generate if ((F2I_M) != 0) begin : op_f2im ula_f2i #(NBMANT,NBEXPO) my_f2im (in1, f2im); end else begin : op_f2im assign f2im = {NUBITS{1'bx}}; end endgenerate

// AND ------------------------------------------------------------------------

wire signed [NUBITS-1:0] ann;

generate if ((AND) != 0) begin : op_ann ula_and #(NUBITS) my_and(in1, in2, ann); end else begin : op_ann assign ann = {NUBITS{1'bx}}; end endgenerate

// ORR ------------------------------------------------------------------------

wire signed [NUBITS-1:0] orr;

generate if ((ORR) != 0) begin : op_orr ula_or #(NUBITS) my_orr(in1, in2, orr); end else begin : op_orr assign orr = {NUBITS{1'bx}}; end endgenerate

// XOR ------------------------------------------------------------------------

wire signed [NUBITS-1:0] cor;

generate if ((XOR) != 0) begin : op_cor ula_xor #(NUBITS) my_xor(in1, in2, cor); end else begin : op_cor assign cor = {NUBITS{1'bx}}; end endgenerate

// INV ------------------------------------------------------------------------

wire signed [NUBITS-1:0] inv;

generate if ((INV) != 0) begin : op_inv ula_inv #(NUBITS) my_inv (in2, inv); end else begin : op_inv assign inv = {NUBITS{1'bx}}; end endgenerate

// INV_M ----------------------------------------------------------------------

wire signed [NUBITS-1:0] invm;

generate if ((INV_M) != 0) begin : op_invm ula_inv #(NUBITS) my_invm(in1, invm); end else begin : op_invm assign invm = {NUBITS{1'bx}}; end endgenerate

// LAN ------------------------------------------------------------------------

wire signed [NUBITS-1:0] lan;

generate if ((LAN) != 0) begin : op_lan ula_lan #(NUBITS) my_lan(in1, in2, lan); end else begin : op_lan assign lan = {NUBITS{1'bx}}; end endgenerate

// LOR ------------------------------------------------------------------------

wire signed [NUBITS-1:0] lor;

generate if ((LOR) != 0) begin : op_lor ula_lor #(NUBITS) my_lor(in1, in2, lor); end else begin : op_lor assign lor = {NUBITS{1'bx}}; end endgenerate

// LIN ------------------------------------------------------------------------

wire signed [NUBITS-1:0] lin;

generate if ((LIN) != 0) begin : op_lin ula_lin #(NUBITS) my_lin(in2, lin); end else begin : op_lin assign lin = {NUBITS{1'bx}}; end endgenerate

// LIN_M ----------------------------------------------------------------------

wire signed [NUBITS-1:0] linm;

generate if ((LIN_M) != 0) begin : op_linm ula_lin #(NUBITS) my_linm(in1, linm); end else begin : op_linm assign linm = {NUBITS{1'bx}}; end endgenerate

// LES ------------------------------------------------------------------------

wire signed [NUBITS-1:0] les;

generate if ((LES) != 0) begin : op_les ula_les #(NUBITS) my_les(in1, in2, les); end else begin : op_les assign les = {NUBITS{1'bx}}; end endgenerate

// F_LES ----------------------------------------------------------------------

wire signed [NUBITS-1:0] fles;

generate if ((F_LES) != 0) begin : op_fles ula_fles #(NUBITS,NBMANT+G) my_fles(sm1_out, sm2_out, fles); end else begin : op_fles assign fles = {NUBITS{1'bx}}; end endgenerate

// GRE ------------------------------------------------------------------------

wire signed [NUBITS-1:0] gre;

generate if ((GRE) != 0) begin : op_gre ula_gre #(NUBITS) my_gre(in1, in2, gre); end else begin : op_gre assign gre = {NUBITS{1'bx}}; end endgenerate

// F_GRE ----------------------------------------------------------------------

wire signed [NUBITS-1:0] fgre;

generate if ((F_GRE) != 0) begin : op_fgre ula_fgre #(NUBITS,NBMANT+G) my_fgre(sm1_out, sm2_out, fgre); end else begin : op_fgre assign fgre = {NUBITS{1'bx}}; end endgenerate

// EQU ------------------------------------------------------------------------

wire signed [NUBITS-1:0] equ;

generate if ((EQU) != 0) begin : op_equ ula_equ #(NUBITS) my_equ(in1, in2, equ); end else begin : op_equ assign equ = {NUBITS{1'bx}}; end endgenerate

// SHR ------------------------------------------------------------------------

wire signed [NUBITS-1:0] shr;

generate if ((SHR) != 0) begin : op_shr ula_shr #(NUBITS) my_shr(in1, in2, shr); end else begin : op_shr assign shr = {NUBITS{1'bx}}; end endgenerate

// SHL ------------------------------------------------------------------------

wire signed [NUBITS-1:0] shl;

generate if ((SHL) != 0) begin : op_shl ula_shl #(NUBITS) my_shl(in1, in2, shl); end else begin : op_shl assign shl = {NUBITS{1'bx}}; end endgenerate

// SRS ------------------------------------------------------------------------

wire signed [NUBITS-1:0] srs;

generate if ((SRS) != 0) begin : op_srs ula_srs #(NUBITS) my_srs(in1, in2, srs); end else begin : op_srs assign srs = {NUBITS{1'bx}}; end endgenerate

// F_ROT ----------------------------------------------------------------------

wire signed [NUBITS+G+1:0] frot;

generate if ((F_ROT) != 0) begin : op_frot ula_frot #(NBMANT,NBEXPO,G) my_frot(in2, frot); end else begin : op_frot assign frot = {NUBITS+G+2{1'bx}}; end endgenerate

// F_SCL / XPO / XPO_M --------------------------------------------------------
// XPO reads in2 (acc), XPO_M reads in1 (memory) -- mirrors F2I/F2I_M.

wire signed [NUBITS-1:0] fscl;

generate if ((F_SCL) != 0) begin : op_fscl ula_scl #(NBMANT,NBEXPO) my_scl(in1, in2, fscl); end else begin : op_fscl assign fscl = {NUBITS{1'bx}}; end endgenerate

wire signed [NUBITS-1:0] xpo;

generate if ((XPO) != 0) begin : op_xpo ula_xpo #(NBMANT,NBEXPO) my_xpo(in2, xpo); end else begin : op_xpo assign xpo = {NUBITS{1'bx}}; end endgenerate

wire signed [NUBITS-1:0] xpom;

generate if ((XPO_M) != 0) begin : op_xpom ula_xpo #(NBMANT,NBEXPO) my_xpom(in1, xpom); end else begin : op_xpom assign xpom = {NUBITS{1'bx}}; end endgenerate

// denormalization mux --------------------------------------------------------

wire signed [NUBITS-1:0] smx;

generate if ((I2F | I2F_M | F_ADD | F_SU1 | F_SU2 | F_MLT | F_DIV | F_ROT) != 0) begin : op_smx norm_mux #(NUBITS,NBMANT,NBEXPO,FROUND,G) norm_mux(op, fadd, fmlt, fdiv, i2f, i2fm, frot, smx); end else begin : op_smx assign smx = {NUBITS{1'bx}}; end endgenerate

// main mux -------------------------------------------------------------------

ula_mux #(NUBITS) ula_mux (.op (op ),
                           .in1(in1),.in2 (in2 ),
                           .add(add),
                           .mlt(mlt),
                           .div(div),
                           .mod(mod),
                           .sgn(sgn),.fsgn(fsgn),
                           .neg(neg),.negm(negm),.fneg(fneg),.fnegm(fnegm),
                           .abs(abs),.absm(absm),.fabs(fabs),.fabsm(fabsm),
                           .pst(pst),.pstm(pstm),.fpst(fpst),.fpstm(fpstm),
                           .nrm(nrm),.nrmm(nrmm),
                           .f2i(f2i),.f2im(f2im),
                           .ann(ann),
                           .orr(orr),
                           .cor(cor),
                           .inv(inv),.invm(invm),
                           .lan(lan),
                           .lor(lor),
                           .lin(lin),.linm(linm),
                           .les(les),.fles(fles),
                           .gre(gre),.fgre(fgre),
                           .equ(equ),
                           .shl(shl),
                           .shr(shr),
                           .srs(srs),
                           .smx(smx),
                           .fscl(fscl),.xpo(xpo),.xpom(xpom),
                           .out(out));

// ----------------------------------------------------------------------------
// flags (simulation) ---------------------------------------------------------
// ----------------------------------------------------------------------------

`ifdef YANC_SIM_VIS // --------------------------------------------------------

// get the signed mantissa for inputs and output ------------------------------

// signed mantissa = +/- the NBMANT-bit magnitude. Width is NBMANT+1 (sign +
// magnitude) so the assign is symmetric -- a 32-bit integer here would make the
// RHS narrower than the LHS and Verilator flags WIDTHEXPAND.
reg signed [NBMANT:0] sm_in1; always @ (*) sm_in1 = (in1[NBMANT+NBEXPO]) ? -$signed({1'b0, in1[NBMANT-1:0]}) : $signed({1'b0, in1[NBMANT-1:0]}); // signed mantissa of in1
reg signed [NBMANT:0] sm_in2; always @ (*) sm_in2 = (in2[NBMANT+NBEXPO]) ? -$signed({1'b0, in2[NBMANT-1:0]}) : $signed({1'b0, in2[NBMANT-1:0]}); // signed mantissa of in2
reg signed [NBMANT:0] sm_out; always @ (*) sm_out = (out[NBMANT+NBEXPO]) ? -$signed({1'b0, out[NBMANT-1:0]}) : $signed({1'b0, out[NBMANT-1:0]}); // signed mantissa of out

// get the exponent of inputs and output --------------------------------------

// exponent = NBEXPO-bit signed field; size the reg to match so the assign is
// symmetric (a 32-bit integer would be wider than the RHS -> WIDTHEXPAND).
reg signed [NBEXPO-1:0] e_in1; always @ (*) e_in1 = $signed(in1[NBMANT+NBEXPO-1:NBMANT]); // exponent of in1
reg signed [NBEXPO-1:0] e_in2; always @ (*) e_in2 = $signed(in2[NBMANT+NBEXPO-1:NBMANT]); // exponent of in2
reg signed [NBEXPO-1:0] e_ouu; always @ (*) e_ouu = $signed(out[NBMANT+NBEXPO-1:NBMANT]); // exponent of out

// get the real values of inputs and output -----------------------------------

real r_in1; always @ (*) r_in1 = sm_in1*$pow(2.0,e_in1); // real value of in1
real r_in2; always @ (*) r_in2 = sm_in2*$pow(2.0,e_in2); // real value of in2
real r_out; always @ (*) r_out = sm_out*$pow(2.0,e_ouu); // real value of out

// compute rounding error for float -------------------------------------------

real delta_float;

always @ (*) begin
	case (op)
		3      : delta_float = (r_in1 + r_in2) - r_out; // addition
		5      : delta_float = (r_in1 * r_in2) - r_out; // multiplication
		7      : delta_float = (r_in1 / r_in2) - r_out; // division
		47	   : delta_float = (r_in2 - r_in1) - r_out; // subtraction
		48	   : delta_float = (r_in1 - r_in2) - r_out; // subtraction
		default: delta_float = 0;
	endcase
end

// compute rounding error for int ---------------------------------------------

real in1r; always @ (*) in1r = in1;
real in2r; always @ (*) in2r = in2;

real val_add; always @ (*) val_add = in1r + in2r;
real val_mlt; always @ (*) val_mlt = in1r * in2r;
real val_div; always @ (*) val_div = in1r / in2r;
// integer remainder: compute it on the integer inputs (same value as the real
// modulo for integer operands) -- a real `%` makes Verilator implicitly convert
// real->int (REALCVT) and is non-standard Verilog anyway.
real val_mod; always @ (*) val_mod = in1 % in2;

real delta_int;

always @ (*) begin
	case (op)
		2      : delta_int = val_add - out; // addition
		4      : delta_int = val_mlt - out; // multiplication
		6      : delta_int = val_div - out; // division
		8      : delta_int = val_mod - out; // remainder
		25     : delta_int = in2   - r_out; // int to float with acc
		26	   : delta_int = in1   - r_out; // int to float with mem
		27     : delta_int = r_in2   - out; // float to int with acc
		28     : delta_int = r_in1   - out; // float to int with mem
		default: delta_int = 0;
	endcase
end

`endif // ---------------------------------------------------------------------

endmodule
