// MAC variants of the array unit (docs/design/array-unit.md, section 7,
// step 2). A study, not SAPHO code: each variant is checked bit for bit
// against its model in mac.py (tb_mac.v) and fitted alone (fit.sh).
//
// Common interface: clr clears the sum (no term in flight); en presents a
// term a*b; three clocks after the last term, res is the sum rounded once to
// the SAPHO format at #FROUND FR (0 and 1 truncate the magnitude, 2 rounds to
// nearest, ties to even). The float word: [sign][exponent NBEXPO, two's
// complement][mantissa NBMANT, top bit written out]; value = m * 2^e; zero =
// the most negative exponent with m = 0.
//
//   stage 1  the product: mantissas multiplied, exponents added (mac_prod)
//   stage 2  the product prepared: placed in the long register (B, C) or
//            scaled and signed (A)
//   stage 3  the feedback: the accumulator
//   res      normalised and rounded from the accumulator (mac_round)

// stage 1: m1*m2 (2M bits), e1+e2, sign; a zero mantissa gives a zero product
module mac_prod #(parameter NB = 32, M = 23, E = 8) (
	input                    clk, en,
	input           [NB-1:0] a, b,
	output reg               v,
	output reg               s,
	output reg signed [E:0]  e,
	output reg     [2*M-1:0] m
);
wire signed [E-1:0] ea = a[NB-2:M], eb = b[NB-2:M];
always @ (posedge clk) begin
	v <= en;
	s <= a[NB-1] ^ b[NB-1];
	e <= ea + eb;
	m <= a[M-1:0] * b[M-1:0];
end
endmodule

// the rounding at the end: |acc| normalised to M bits, rounded, exponent lsb + shift
module mac_round #(parameter NB = 32, M = 23, E = 8, W = 64, XB = 12, FR = 2) (
	input  signed [W-1:0]  acc,
	input  signed [XB-1:0] lsb,          // the weight of the accumulator's bit 0
	output        [NB-1:0] res
);
wire          neg = acc[W-1];
wire [W-1:0]  mag = neg ? -acc : acc;    // -(-2^(W-1)) is 2^(W-1) read unsigned
integer k, n;
always @ (*) begin
	n = 0;
	for (k = 0; k < W; k = k + 1) if (mag[k]) n = k + 1;
end
wire signed [15:0] sh = n - M;
reg  [W:0]   low;
reg  [M:0]   q;
reg          up;
reg signed [XB+1:0] eo;
always @ (*) begin
	if (sh > 0) begin
		q   = mag >> sh;
		low = mag & (({{W{1'b0}}, 1'b1} << (sh - 1)) - 1);         // below the half bit
		up  = (FR == 2) && mag[sh - 1] && ((low != 0) || q[0]);
	end else begin
		q   = mag << (-sh);
		low = 0;
		up  = 1'b0;
	end
	q  = q + up;
	eo = lsb + sh;
	if (q[M]) begin q = q >> 1; eo = eo + 1; end
end
wire [E-1:0] ezero = {1'b1, {E-1{1'b0}}};
assign res = (mag == 0) ? {1'b0, ezero, {M{1'b0}}} : {neg, eo[E-1:0], q[M-1:0]};
endmodule

// C: full-range long accumulator (Kulisch). LSB weight 2^(2*emin) = 2^-(2^E).
module mac_c #(parameter NB = 32, M = 23, E = 8, LGN = 10, FR = 2) (
	input           clk, clr, en,
	input  [NB-1:0] a, b,
	output [NB-1:0] res
);
localparam W  = 2*M + (1 << (E+1)) - 1 + LGN;
localparam XB = E + 3;
wire v1, s1; wire signed [E:0] e1; wire [2*M-1:0] m1;
mac_prod #(NB, M, E) p (clk, en, a, b, v1, s1, e1, m1);
wire [E+1:0] off = e1 + (1 << E);                    // 0 .. 2^(E+1)-2
wire [W-1:0] placed = {{W-2*M{1'b0}}, m1} << off;
reg  v2; reg signed [W-1:0] p2;
always @ (posedge clk) begin v2 <= v1; p2 <= s1 ? -placed : placed; end
reg signed [W-1:0] acc;
always @ (posedge clk) if (clr) acc <= 0; else if (v2) acc <= acc + p2;
wire signed [XB-1:0] lsb = -(1 << E);
mac_round #(NB, M, E, W, XB, FR) r (acc, lsb, res);
endmodule

// B: long fixed-point accumulator of W bits, its LSB weight 2^lsb set from
// outside (a register the program would write). Products below the window
// are cut (toward zero); a sum above it wraps.
module mac_b #(parameter NB = 32, M = 23, E = 8, W = 64, XB = 12, FR = 2) (
	input                  clk, clr, en,
	input         [NB-1:0] a, b,
	input  signed [XB-1:0] lsb,
	output        [NB-1:0] res
);
wire v1, s1; wire signed [E:0] e1; wire [2*M-1:0] m1;
mac_prod #(NB, M, E) p (clk, en, a, b, v1, s1, e1, m1);
wire signed [XB:0] off = e1 - lsb;
wire [W+2*M-1:0] wide = {{W{1'b0}}, m1};
wire [W-1:0] placed = (off >= 0) ? wide << off : wide >> (-off);
reg  v2; reg signed [W-1:0] p2;
always @ (posedge clk) begin v2 <= v1; p2 <= s1 ? -placed : placed; end
reg signed [W-1:0] acc;
always @ (posedge clk) if (clr) acc <= 0; else if (v2) acc <= acc + p2;
mac_round #(NB, M, E, W, XB, FR) r (acc, lsb, res);
endmodule

// A: lazy accumulator, a W-bit mantissa in two's complement (W+1 bits with
// the sign, range [-2^W, 2^W)) and its own exponent. In the feedback: the
// exponents compared, the smaller operand shifted right (arithmetic, so
// toward minus infinity), the add, one shift right when the sum leaves the
// range. Never normalised to the left; a zero accumulator takes the product.
module mac_a #(parameter NB = 32, M = 23, E = 8, W = 54, FR = 2) (
	input           clk, clr, en,
	input  [NB-1:0] a, b,
	output [NB-1:0] res
);
localparam XB = E + 4;
wire v1, s1; wire signed [E:0] e1; wire [2*M-1:0] m1;
mac_prod #(NB, M, E) p (clk, en, a, b, v1, s1, e1, m1);
wire [W:0] up = {{W+1-2*M{1'b0}}, m1} << (W - 2*M);  // the product at the top of W bits
reg  v2; reg signed [W:0] p2; reg signed [XB-1:0] pe2;
// a zero product is skipped: its exponent (from the zero's) must not move the accumulator's
always @ (posedge clk) begin v2 <= v1 && (m1 != 0); p2 <= s1 ? -up : up; pe2 <= e1 - (W - 2*M); end
reg signed [W:0] acc; reg signed [XB-1:0] ea;
wire signed [XB:0] d = pe2 - ea;
// every operand of the ?: signed, or the whole expression is unsigned and >>> shifts in zeros
wire signed [W:0] vs = (d >= 0) ? ((d > W) ? $signed({W+1{acc[W]}}) : acc >>> d) : acc;
wire signed [W:0] ps = (d >= 0) ? p2 : ((-d > W) ? $signed({W+1{p2[W]}}) : p2 >>> (-d));
wire signed [XB-1:0] en_ = (d >= 0) ? pe2 : ea;
wire signed [W+1:0] sum = vs + ps;
always @ (posedge clk)
	if (clr) begin acc <= 0; ea <= 0; end
	else if (v2) begin
		if (acc == 0)                 begin acc <= p2;        ea <= pe2;     end
		else if (sum[W+1] != sum[W])  begin acc <= sum[W+1:1]; ea <= en_ + 1; end
		else                          begin acc <= sum[W:0];   ea <= en_;     end
	end
mac_round #(NB, M, E, W+1, XB, FR) r (acc, ea, res);
endmodule

// The block as fitted: inputs and output registered, so every timed path
// starts and ends at a register. VAR: 0 = A, 1 = B, 2 = C.
module mac_fit #(parameter VAR = 2, NB = 32, M = 23, E = 8, W = 64, LGN = 10, XB = 12, FR = 2) (
	input                  clk, clr, en,
	input         [NB-1:0] a, b,
	input  signed [XB-1:0] lsb,
	output reg    [NB-1:0] res
);
reg clr_r, en_r; reg [NB-1:0] a_r, b_r; reg signed [XB-1:0] lsb_r;
always @ (posedge clk) begin clr_r <= clr; en_r <= en; a_r <= a; b_r <= b; lsb_r <= lsb; end
wire [NB-1:0] r;
generate
	if (VAR == 0)      begin : va mac_a #(NB, M, E, W, FR)      u (clk, clr_r, en_r, a_r, b_r, r); end
	else if (VAR == 1) begin : vb mac_b #(NB, M, E, W, XB, FR)  u (clk, clr_r, en_r, a_r, b_r, lsb_r, r); end
	else               begin : vc mac_c #(NB, M, E, LGN, FR)    u (clk, clr_r, en_r, a_r, b_r, r); end
endgenerate
always @ (posedge clk) res <= r;
endmodule
