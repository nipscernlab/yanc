# Prototype (not for the repository): pointer registers with post-increment
# and a parallel store of the previous result, and a zero-overhead loop (RPT),
# each block generated only when the program uses it (NPTR > 0, RPT != 0).
P = 'C:/nipscern/yanc-dirac/.smoke/dirac-proto/proto/SAPHO/'

def patch(fn, reps):
    s = open(P + fn, encoding='utf-8', newline='').read()
    crlf = '\r\n' in s
    s = s.replace('\r\n', '\n')
    for a, b in reps:
        assert s.count(a) == 1, (fn, a[:70])
        s = s.replace(a, b)
    if crlf: s = s.replace('\n', '\r\n')
    open(P + fn, 'w', encoding='utf-8', newline='').write(s)

# ---------------------------------------------------------------- core.v ----
C = []

# pc: the sequential address comes from outside (the RPT loop may replace it)
C.append(("""	 input                 load,
	 input     [NBITS-1:0] data,
	output reg [NBITS-1:0] addr = 0""", """	 input                 load,
	 input     [NBITS-1:0] data,
	 input     [NBITS-1:0] seq,   // the next address when not loading: addr, or the RPT loop start
	output reg [NBITS-1:0] addr = 0"""))
C.append(("""wire [NBITS-1:0] val = (load) ? data : addr;""",
          """wire [NBITS-1:0] val = (load) ? data : seq;"""))

# instr_fetch: RPT parameter, the loop registers, the sequential address
C.append(("""	parameter CAL    = 0,
	parameter JIZ    = 0,
	parameter TOM    = 0,
	parameter CAD    = 0
)(
	input               clk, rst,
	input               itr,
	output              cheguei,
	input               tom_go, cad_go,""", """	parameter CAL    = 0,
	parameter JIZ    = 0,
	parameter TOM    = 0,
	parameter CAD    = 0,
	parameter RPT    = 0
)(
	input               clk, rst,
	input               itr,
	output              cheguei,
	input               tom_go, cad_go,"""))

C.append(("""`ifdef YANC_SIM_VIS // --------------------------------------------------------
pc #(MINSTW) pc (clk, rst, pc_load, pcl, pc_addr, pc_sim_val);
`else
pc #(MINSTW) pc (clk, rst, pc_load, pcl, pc_addr);
`endif // ---------------------------------------------------------------------
""", """// RPT L n (opcode 111): the L instructions after it run n times with no jump
// instruction. The loop registers hold the first address of the body (rs),
// the address after its last (re) and the turns left (rc); when the PC is
// about to fetch re with turns left, the fetch takes rs instead. Everything
// comes from registers, so the choice is ready before the decode of the
// instruction in flight decides a jump (pc_load stays the last select).
wire [MINSTW-1:0] pc_seq;   // the next sequential address

generate if (RPT) begin : rpt_blk
	localparam [NBOPCO-1:0] OP_RPT = 111;
	localparam CW = NBOPER - 4;                 // the count field
	reg  [MINSTW-1:0] rs = 0, re = 0;
	reg  [CW    -1:0] rc = 0;
	reg               ra = 1'b0;               // a loop is running
	wire              is_rpt = (opcode == OP_RPT);
	wire              wrap   = ra & (pc_addr == re);
	always @ (posedge clk) begin
		if (rst | itr) ra <= 1'b0;
		else if (is_rpt) begin
			rs <= pc_addr;
			re <= pc_addr + operand[NBOPER-2:NBOPER-4];
			rc <= operand[CW-1:0];
			ra <= (operand[CW-1:0] > 1);
		end else if (wrap) begin   // no jump inside the body (contract): pc_load not needed here
			rc <= rc - 1'b1;
			if (rc == 2) ra <= 1'b0;
		end
	end
	assign pc_seq = (wrap) ? rs : pc_addr;
end else begin : rpt_blk
	assign pc_seq = pc_addr;
end endgenerate

`ifdef YANC_SIM_VIS // --------------------------------------------------------
pc #(MINSTW) pc (clk, rst, pc_load, pcl, pc_seq, pc_addr, pc_sim_val);
`else
pc #(MINSTW) pc (clk, rst, pc_load, pcl, pc_seq, pc_addr);
`endif // ---------------------------------------------------------------------
"""))

C.append(("""		   .CAD   (CAD   )) pf(rst, tom_go, cad_go, self_addr, pf_self, pc_addr, opcode, operand,""",
          """		   .CAD   (CAD   )) pf(rst, tom_go, cad_go, self_addr, pf_self, pc_seq, opcode, operand,"""))

# mem_ctrl: a read base and a write base (a pointer may replace the operand)
C.append(("""	input  [MDATAW-1:0] base_addr, stk_ofst,""",
          """	input  [MDATAW-1:0] base_rd, base_wr, stk_ofst,"""))
C.append(("""rel_addr #(.MDATAW(MDATAW), .FFTSIZ(FFTSIZ), .USEFFT(ILI)) ra_rd(ldi, fft, idx[MDATAW-1:0], base_addr, mem_addr_rd);
rel_addr #(.MDATAW(MDATAW), .FFTSIZ(FFTSIZ), .USEFFT(ISI)) ra_wr(sti, fft, stk_ofst       , base_addr, mem_addr_wr);""",
"""rel_addr #(.MDATAW(MDATAW), .FFTSIZ(FFTSIZ), .USEFFT(ILI)) ra_rd(ldi, fft, idx[MDATAW-1:0], base_rd, mem_addr_rd);
rel_addr #(.MDATAW(MDATAW), .FFTSIZ(FFTSIZ), .USEFFT(ISI)) ra_wr(sti, fft, stk_ofst       , base_wr, mem_addr_wr);"""))

# core parameters
C.append(("""	parameter    XPO   = 0,   // base-2 exponent of float (acc) as int
	parameter  XPO_M   = 0    // base-2 exponent of float (memory) as int
)(""", """	parameter    XPO   = 0,   // base-2 exponent of float (acc) as int
	parameter  XPO_M   = 0,   // base-2 exponent of float (memory) as int

	// prototype: pointers and the zero-overhead loop
	parameter    NPTR  = 0,   // pointer registers the program uses (0..4), with SETP
	parameter    RPT   = 0    // RPT L n
)("""))

C.append(("""	.CAD        (CAD        )) instr_fetch (.clk    (clk       ),""",
          """	.CAD        (CAD        ),
	.RPT        (RPT        )) instr_fetch (.clk    (clk       ),"""))

# the pointers, and the memory addresses through them
C.append(("""// Indirect Addressing --------------------------------------------------------

wire [MDATAW-1:0] rf;

generate
	if (STI | LDI | ILI | ISI) begin : mem_access
		mem_ctrl #(.NUBITS(NUBITS),
		           .MDATAW(MDATAW),
		           .FFTSIZ(FFTSIZ),
		           .ILI(ILI),.ISI(ISI)) ac(id_sti, id_ldi, id_fft, id_wr,
		                                   ula_out, ula_fast,
		                                   if_operand[MDATAW-1:0], sp_data[MDATAW-1:0],
		                                   mem_wr, mem_addr_rd, mem_addr_wr, mem_data_wr);
	end else begin : mem_access
		assign mem_wr      = id_wr;
		assign mem_addr_rd = if_operand[MDATAW-1:0];
		assign mem_addr_wr = if_operand[MDATAW-1:0];
		assign mem_data_wr = ula_out;
	end
endgenerate""", """// Pointers (prototype) -------------------------------------------------------
// With NPTR > 0 the top bit of the operand of a memory instruction selects the
// pointer mode (asmcomp widens the operand so that no other operand reaches
// that bit). The fields: [1:0] k, the pointer of the instruction's own memory
// access; [3:2] its change after the access (01 +1, 10 -1); [4] st, also store
// the previous result (ula_out, what a SET stores) through pointer [6:5] j,
// changed by [8:7]. SETP k a (opcode 110) loads pointer k with the address a,
// an immediate. Every select reads registers and the instruction word, never
// the ALU: the read and write bases come before the indirect adder of LDI/STI.

localparam [NBOPCO-1:0] OP_SETP = 110;

wire [MDATAW-1:0] base_rd, base_wr;
wire              ptr_st;

generate if (NPTR > 0) begin : ptr_blk
	wire             is_setp = (if_opcode == OP_SETP);
	wire             ptr_on  = if_operand[NBOPER-1] & ~is_setp;
	wire [1:0]       k  = if_operand[1:0], mk = if_operand[3:2];
	wire             st = if_operand[4];
	wire [1:0]       j  = if_operand[6:5], mj = if_operand[8:7];
	wire [1:0]       sk = if_operand[MDATAW+1:MDATAW];  // SETP: which pointer

	wire [MDATAW-1:0] pk_term [0:3];
	wire [MDATAW-1:0] pj_term [0:3];

	genvar i;
	for (i = 0; i < NPTR; i = i + 1) begin : preg
		wire step_k = ptr_on & (k == i) & (mk != 2'b00);
		wire step_j = ptr_on & st & (j == i) & (mj != 2'b00);
		wire down   = step_k ? mk[1] : mj[1];
		reg [MDATAW-1:0] r = {MDATAW{1'b0}};
		always @ (posedge clk) begin
			if      (rst)                    r <= {MDATAW{1'b0}};
			else if (is_setp && sk == i)     r <= if_operand[MDATAW-1:0];
			else if (step_k | step_j)        r <= (down) ? r - 1'b1 : r + 1'b1;
		end
		// a mux as an OR of guarded terms: no input at don't care
		assign pk_term[i] = (k == i) ? r : {MDATAW{1'b0}};
		assign pj_term[i] = (j == i) ? r : {MDATAW{1'b0}};
	end
	for (i = NPTR; i < 4; i = i + 1) begin : pnone   // pointers the program does not use
		assign pk_term[i] = {MDATAW{1'b0}};
		assign pj_term[i] = {MDATAW{1'b0}};
	end

	wire [MDATAW-1:0] pk = pk_term[0] | pk_term[1] | pk_term[2] | pk_term[3];
	wire [MDATAW-1:0] pj = pj_term[0] | pj_term[1] | pj_term[2] | pj_term[3];

	assign ptr_st  = ptr_on & st;
	assign base_rd = (ptr_on) ? pk : if_operand[MDATAW-1:0];
	assign base_wr = (ptr_st) ? pj : base_rd;
end else begin : ptr_blk
	assign ptr_st  = 1'b0;
	assign base_rd = if_operand[MDATAW-1:0];
	assign base_wr = if_operand[MDATAW-1:0];
end endgenerate

// Indirect Addressing --------------------------------------------------------

wire [MDATAW-1:0] rf;

generate
	if (STI | LDI | ILI | ISI) begin : mem_access
		mem_ctrl #(.NUBITS(NUBITS),
		           .MDATAW(MDATAW),
		           .FFTSIZ(FFTSIZ),
		           .ILI(ILI),.ISI(ISI)) ac(id_sti, id_ldi, id_fft, id_wr | ptr_st,
		                                   ula_out, ula_fast,
		                                   base_rd, base_wr, sp_data[MDATAW-1:0],
		                                   mem_wr, mem_addr_rd, mem_addr_wr, mem_data_wr);
	end else begin : mem_access
		assign mem_wr      = id_wr | ptr_st;
		assign mem_addr_rd = base_rd;
		assign mem_addr_wr = base_wr;
		assign mem_data_wr = ula_out;
	end
endgenerate"""))

patch('core.v', C)

# ----------------------------------------------------------- processor.v ----
patch('processor.v', [
("""	parameter  XPO_M   = 0     // base-2 exponent of float (memory) as int""",
 """	parameter  XPO_M   = 0,    // base-2 exponent of float (memory) as int
	parameter  NPTR    = 0,    // prototype: pointer registers
	parameter  RPT     = 0     // prototype: zero-overhead loop"""),
("""	   .XPO_M  ( XPO_M  )) core(clk, rst,""",
 """	   .XPO_M  ( XPO_M  ),
	   .NPTR   ( NPTR   ),
	   .RPT    ( RPT    )) core(clk, rst,"""),
])
print("ok")
