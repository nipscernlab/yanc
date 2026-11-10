// ****************************************************************************
// Helper circuits ************************************************************
// ****************************************************************************

// Simulation-visibility guard: the program-counter tap (pc_sim_val) feeds the
// waveform harness in the generated <proc>.v. It is compiled for Icarus
// (predefines __ICARUS__) and for Verilator when the user passes
// +define+YANC_TRACE, but never for synthesis. The stack's flags further down
// (fl_overflow and fl_max, registers that keep the worst case seen) are gated
// the same way (YANC_SIM_VIS), so they ride along with the harness; ula.v
// gates its rounding-error block with its own copy of this guard.
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

// program counter ------------------------------------------------------------

module pc
#(
	parameter NBITS = 8
)(
	 input                 clk , rst,
	 input     [NBITS-1:0] data,       // the address fetched this cycle
	output reg [NBITS-1:0] addr = 0

`ifdef YANC_SIM_VIS // --------------------------------------------------------
  , output     [NBITS-1:0] sim
`endif // ---------------------------------------------------------------------
);

wire [NBITS-1:0] um  = 1'b1;   // zero-extended; {{NBITS-1{..}}, ..} was a zero-width
                               // replication for a 2-word program (#167), as in stack
// The next address is the one fetched now + 1. It used to be computed again
// here, from a load flag and a value of its own (instr_fetch's pc_lval),
// a second copy of the fetch's choice that always gave the same address
// (#169): no jump, the PC itself; a jump, CAL or a waiting TOM/CAD, its
// target; RET, the stack's top; an interrupt, ITRADD.

// synchronous reset: FPGA-friendly (maps to the FF's dedicated sync clear, no
// recovery/removal timing on the reset net)
always @ (posedge clk) begin
	if (rst) addr <= 0;
	else     addr <= data + um;
end

`ifdef YANC_SIM_VIS // --------------------------------------------------------
assign sim = data;
`endif // ---------------------------------------------------------------------

endmodule

// instruction prefetch -------------------------------------------------------

module prefetch
#(
	parameter              MINSTW     = 8,
	parameter              NBOPCO     = 7,
	parameter              NBOPER     = 9,
	parameter [MINSTW-1:0] ITRADD     = 0,
	parameter              CAL        = 0,
	parameter              JIZ        = 0,
	parameter              JNZ        = 0,
	parameter              TOM        = 0,
	parameter              CAD        = 0
)(
	 input                        rst       ,
	 input                        tom_go    , // the partner took my value (TOM may go on)
	 input                        cad_go    , // the partner offers a value (CAD may go on)
	 input    [MINSTW       -1:0] self_addr , // address of the instruction now decoded (TOM/CAD wait)
	output                        pc_self   , // TOM/CAD waits: fetch self_addr again
	 input    [MINSTW       -1:0] pc_instr  ,
	output    [NBOPCO       -1:0] opcode    ,
	output    [NBOPER       -1:0] operand   ,
	 input    [NBOPCO+NBOPER-1:0] mem_instr ,
	output    [MINSTW       -1:0] instr_addr,
	 input                        is_um     ,
	output                        isp_push  ,
	output                        isp_pop   ,
	 input                        itr
);

// flow-control opcodes, as numbered in Compilers/common/isa.tsv (check_isa.py
// holds them to the table)
localparam [NBOPCO-1:0] OP_JMP = 16, OP_JIZ = 17, OP_JNZ = 18, OP_CAL = 19;
localparam [NBOPCO-1:0] OP_RET = 20, OP_TOM = 21, OP_CAD = 22;

wire wJMP;
wire wJMPZ;

// the conditional jumps test the same bit, is_um (the acc is not zero): JIZ
// jumps when it is clear, JNZ when it is set. Each is there only when the
// program uses it.
wire wJIZ, wJNZ;
generate if ((JIZ) != 0) begin : jiz_sel assign wJIZ = (opcode == OP_JIZ) & ~is_um; end
         else            begin : jiz_sel assign wJIZ = 1'b0;                       end endgenerate
generate if ((JNZ) != 0) begin : jnz_sel assign wJNZ = (opcode == OP_JNZ) &  is_um; end
         else            begin : jnz_sel assign wJNZ = 1'b0;                       end endgenerate

assign wJMPZ = (opcode == OP_JMP) | wJIZ | wJNZ;

// toma / cade (docs/design/toma-and-cade.md): a jump to itself until the partner's
// bit lets it through. The target is not the operand (free for the link
// number, docs/design/toma-and-cade-links.md): instr_fetch keeps the address of the
// instruction now decoded (self_addr) and the fetch takes it again.
wire wTOM, wCAD;
generate if ((TOM) != 0) begin : tom_sel assign wTOM = (opcode == OP_TOM) & ~tom_go; end
         else            begin : tom_sel assign wTOM = 1'b0;                        end endgenerate
generate if ((CAD) != 0) begin : cad_sel assign wCAD = (opcode == OP_CAD) & ~cad_go; end
         else            begin : cad_sel assign wCAD = 1'b0;                        end endgenerate

assign pc_self = wTOM | wCAD;
assign wJMP    = wJMPZ | pc_self;

wire pc_load;

generate if ((CAL) != 0) begin : call_ctrl

wire wCAL = (opcode == OP_CAL);
wire wRET = (opcode == OP_RET);

assign pc_load    =  wJMP | wCAL | wRET;
assign isp_push   =  wCAL;
assign isp_pop    =  wRET;

end else begin : call_ctrl

assign pc_load    =  wJMP;
assign isp_push   =  1'b0;                // no CAL: no instruction stack reads them
assign isp_pop    =  1'b0;

end endgenerate

assign opcode     =  mem_instr[NBOPCO+NBOPER-1:NBOPER];
assign operand    =  mem_instr[NBOPER       -1:     0];

// During rst the fetch address is 0, not the PC: instruction memory reads
// synchronously, so the word fetched on the reset edge is the first one
// executed afterwards. Fetching at the old PC let a JMP / JIZ / CAL sitting
// there run right after a mid-run reset and send the program back where it
// was. At boot the PC is already 0, so this makes every reset a boot.
// the jump target: the operand, or self_addr when TOM/CAD wait (only with
// TOM or CAD in the program)
wire [MINSTW-1:0] jmp_addr;
generate if ((TOM) != 0 || (CAD) != 0) begin : jmp_tgt assign jmp_addr = (pc_self) ? self_addr : operand[MINSTW-1:0]; end
         else                          begin : jmp_tgt assign jmp_addr =                          operand[MINSTW-1:0]; end endgenerate

generate if (ITRADD>0) begin : itr_fetch

assign instr_addr = (rst) ? {MINSTW{1'b0}} : (itr) ? ITRADD : (pc_load) ? jmp_addr : pc_instr;

end else begin : itr_fetch

assign instr_addr = (rst) ? {MINSTW{1'b0}} :                  (pc_load) ? jmp_addr : pc_instr;

end endgenerate

endmodule

// stack: the instruction (return-address) stack and the data stack ---------

module stack
#(
	parameter NADDR = 7,
	parameter DEPTH = 3,
	parameter NBITS = 8
)(
	 input             clk , rst,
	 input             push, pop,
	 input [NBITS-1:0] in,
	output [NBITS-1:0] out
);

// Constants

// (a 1-bit pointer made {NADDR-1{..}} a zero replication, which Vivado
// flags and stricter tools refuse; 1'b1 is zero-extended to NADDR bits)
wire [NADDR-1:0] zero = {NADDR{1'b0}};
wire [NADDR-1:0] um   = 1'b1;

// Memory

reg [NBITS-1:0] mem [DEPTH-1:0];

// Stack Pointer

reg  [NADDR-1:0] pointer = 0; // ideal for monitoring
wire [NADDR-1:0] pmaisum = pointer + um;
wire [NADDR-1:0] pmenoum = pointer - um;

always @ (posedge clk) begin // synchronous reset (FPGA-friendly)
	if      (rst ) pointer <= zero;
	else if (push) pointer <= pmaisum;
	else if (pop ) pointer <= pmenoum;
end

// Stack interface

always @ (posedge clk) if (push) mem[pointer] <= in;
assign                     out = mem[pmenoum];

// Flags
`ifdef YANC_SIM_VIS // --------------------------------------------------------

// The words the stack holds, counted apart from the pointer: a full stack of
// a power-of-two DEPTH has its pointer wrapped to 0, which the hardware reads
// right (the top is mem[pointer - 1]) but a monitor cannot tell from empty.
// One bit wider than the pointer, so DEPTH fits.
localparam [NADDR:0] FULL = DEPTH[NADDR:0];
reg  [NADDR:0] fl_cnt  = 0;
wire [NADDR:0] fl_nxt  = rst  ? {(NADDR+1){1'b0}} :
                         push ? fl_cnt + 1'b1      :
                         pop  ? fl_cnt - 1'b1      : fl_cnt;

reg            fl_overflow = 0;  // a push with DEPTH words held, or a pop with none
reg            fl_top;       // DEPTH words held now: the stack is full, which a
                             // program at its deepest point is, with no harm
reg  [NADDR:0] fl_max  = 0;  // the most words held
reg  [NADDR+1:0] pointeri;   // the words held, for the waveform: a sign bit on
                             // top, as gen_gtkw shows it as a signed number

// pointeri is a pure combinational copy -- no self-reference, so it is fine
// under Verilator.
always @ (*) pointeri = {1'b0, fl_cnt};
always @ (*) fl_top   = (fl_cnt == FULL);

// fl_overflow ("stack ever overflowed", sticky) and fl_max are STATE: an always
// @(*) that reads its own output is a latch / comb loop that Verilator flags
// as UNOPTFLAT and may mis-evaluate. They are registered on the clock, and
// fl_max on the count's NEXT value, so it changes in the same cycle as the
// pointer, in Icarus and Verilator alike. They never reset (stats of the run).
// fl_overflow rises on a real overflow (issue #145). Before, named fl_full,
// it rose when the stack became full, so asmcomp gave every stack a word
// more than the program needs to keep a correct program below it.
always @ (posedge clk) begin
	fl_cnt <= fl_nxt;
	if (!rst && ((push && fl_cnt == FULL) || (!push && pop && fl_cnt == {(NADDR+1){1'b0}}))) fl_overflow <= 1'b1;
	if (fl_nxt > fl_max) fl_max <= fl_nxt;
end

`endif // ---------------------------------------------------------------------

endmodule

// Instruction fetch ----------------------------------------------------------

module instr_fetch
#(
	parameter NBINST     = 8,
	parameter MINSTW     = 8,
	parameter ITRADD     = 0,
	parameter TOAQUIADDR = 0,
	parameter NBOPCO     = 7,
	parameter NBOPER     = 9,
	parameter SDEPTH     = 8,

	parameter CAL    = 0,
	parameter JIZ    = 0,
	parameter JNZ    = 0,
	parameter TOM    = 0,
	parameter CAD    = 0
)(
	input               clk, rst,
	input               itr,
	output              cheguei,
	input               tom_go, cad_go,

	input  [NBINST-1:0] instr,
	output [MINSTW-1:0] addr,

	input               acc,
	output [NBOPCO-1:0] opcode,
	output [NBOPER-1:0] operand

`ifdef YANC_SIM_VIS // --------------------------------------------------------
 , output [MINSTW-1:0] pc_sim_val
`endif // ---------------------------------------------------------------------
);

// Program Counter

wire [MINSTW-1:0] pc_addr;

// the PC follows the address fetched (addr, below): one choice of the next
// address, not two (#169)
`ifdef YANC_SIM_VIS // --------------------------------------------------------
pc #(MINSTW) pc (clk, rst, addr, pc_addr, pc_sim_val);
`else
pc #(MINSTW) pc (clk, rst, addr, pc_addr);
`endif // ---------------------------------------------------------------------

// Instruction prefetch

wire [NBINST-1:0] pf_instr = instr;
wire              pf_isp_push;
wire              pf_isp_pop;
wire [MINSTW-1:0] pf_addr;
wire              pf_self;
wire [MINSTW-1:0] self_addr;

// the address of the instruction now decoded, for a waiting TOM/CAD: the
// instruction memory reads synchronously, so it is the address presented in
// the previous cycle (only with TOM or CAD in the program)
generate
	if (TOM != 0 || CAD != 0) begin : self_blk
		reg [MINSTW-1:0] r_self = 0;
		always @ (posedge clk) r_self <= addr;
		assign self_addr = r_self;
	end else begin : self_blk
		assign self_addr = {MINSTW{1'b0}};
	end
endgenerate

prefetch #(.MINSTW(MINSTW),
           .NBOPCO(NBOPCO),
           .NBOPER(NBOPER),
           .ITRADD(ITRADD),
		   .CAL   (CAL   ),
		   .JIZ   (JIZ   ),
		   .JNZ   (JNZ   ),
		   .TOM   (TOM   ),
		   .CAD   (CAD   )) pf(rst, tom_go, cad_go, self_addr, pf_self, pc_addr, opcode, operand,
                               pf_instr, pf_addr,
                               acc,
                               pf_isp_push, pf_isp_pop,
                               itr);

// Instruction stack

// A #PRACA pulse empties the stack, as a reset does (#188): the pulse restarts
// the program at ITRADD, where asmcomp checks that both stacks are empty, so
// every word held when it came (the return addresses of the routines it left,
// and the push of a CAL in decode) is dropped. Without that, each pulse in a
// routine left its words behind and the pointer climbed past DEPTH. Only with
// #ITRAD; the data stack does the same (core, below).
wire stk_clr;
generate
	if (ITRADD > 0) begin : isp_clr assign stk_clr = rst | itr; end
	else            begin : isp_clr assign stk_clr = rst;       end
endgenerate

wire [MINSTW-1:0] stack_out;

generate
	if (CAL) begin : isp_blk
		stack #((SDEPTH > 1) ? $clog2(SDEPTH) : 1, SDEPTH, MINSTW) isp(clk, stk_clr, pf_isp_push, pf_isp_pop, pc_addr, stack_out);
	end
endgenerate

// External interface

// A RET takes the return address only when neither a reset nor an interrupt
// does: prefetch has already put them first in pf_addr (0 and ITRADD), and a
// RET here used to override both (#164, #166), so a reset or an interrupt
// pulse with a RET in decode went to the return address. The interrupt term
// exists only with #ITRAD.
wire ret_go;
generate
	if (ITRADD > 0) begin : ret_go_sel assign ret_go = pf_isp_pop & ~rst & ~itr; end
	else            begin : ret_go_sel assign ret_go = pf_isp_pop & ~rst;        end
endgenerate

generate
	if (CAL) begin : ret_addr
		assign addr = (ret_go) ? stack_out : pf_addr;
	end else begin : ret_addr
		assign addr =  pf_addr;
	end
endgenerate

// #TOAQUI marker: cheguei is 1 while the address fetched is TOAQUIADDR, i.e.
// the instruction at the marker is about to be executed. It compares addr,
// the address the instruction memory gets, after the return address of a
// RET: prefetch's own choice, which it compared before, is not the return
// address on a RET, so a marker reached through a return never raised it
// (#189). Nor is the PC register: it holds the address fetched in the
// previous cycle plus 1 (module pc), the next one in sequence.
generate
	if (TOAQUIADDR > 0) begin : toaqui_pin assign cheguei = (addr == TOAQUIADDR); end
	else                begin : toaqui_pin assign cheguei = 1'b0;                  end
endgenerate

endmodule

// Control of ALU input in1 ---------------------------------------------------

module ula_in1_ctrl
#(
	parameter NUBITS = 8
)(
	input               clk, rst, pop,
	input  [NUBITS-1:0] mem, stack,
	output [NUBITS-1:0] out
);

// stkr has no reset: popr, which picks it, is reset, so its value in and
// right after a reset is never read; a reset it does not need can keep a
// tool from packing it (#172)
reg popr;              always @ (posedge clk) if (rst) popr <= 1'b0; else popr <= pop;
reg [NUBITS-1:0] stkr; always @ (posedge clk) stkr <= stack;

assign out = (popr) ? stkr : mem;

endmodule

// Control of ALU input in2 ---------------------------------------------------

module ula_in2_ctrl
#(
	parameter NUBITS = 8
)(
	input               clk, rst,
	input               req_in,
	input  [NUBITS-1:0] acc, io_in,
	output [NUBITS-1:0] out
);

// ior has no reset, as stkr: req_inr, which picks it, is reset (#172)
reg               req_inr; always @ (posedge clk) if (rst) req_inr <= 1'b0; else req_inr <= req_in;
reg  [NUBITS-1:0] ior    ; always @ (posedge clk) ior <= io_in;

assign out = (req_inr) ? ior : acc;

endmodule

// Indirect addressing --------------------------------------------------------

module rel_addr
#(
	parameter MDATAW = 8,
	parameter FFTSIZ = 3,
	parameter USEFFT = 1
)(
	input               use_oft, fft,
	input  [MDATAW-1:0] offset,
	input  [MDATAW-1:0] addr,
	output [MDATAW-1:0] out
);

generate
	if (USEFFT) begin : fft_offset
		reg [FFTSIZ-1:0] aux;

		integer i;
		always @ (*) for (i = 0; i < FFTSIZ; i = i+1) aux[i] = offset[FFTSIZ-1-i];

		// the index's low FFTSIZ bits reversed (the FFT's order); the bits above
		// them, when the data memory has any, kept. A memory of exactly
		// 2^FFTSIZ words (an FFT array and nothing else) has none above:
		// offset[MDATAW-1:FFTSIZ] was then a reversed part-select that Icarus
		// refuses (#168). asmcomp refuses a smaller memory.
		wire [MDATAW-1:0] add;
		if (FFTSIZ < MDATAW) begin : keep_hi assign add = (fft) ? {offset[MDATAW-1:FFTSIZ], aux} : offset; end
		else                 begin : all_rev assign add = (fft) ? aux[MDATAW-1:0]                  : offset; end

		assign out = (use_oft) ?    add + addr: addr;
	end else begin : fft_offset
		assign out = (use_oft) ? offset + addr: addr;
	end
endgenerate

endmodule

// Memory-addressing control --------------------------------------------------

module mem_ctrl
#(
	parameter NUBITS = 8,
	parameter MDATAW = 8,
	parameter FFTSIZ = 3,

	parameter ISI    = 0,
	parameter ILI    = 0
)(
	input               sti, ldi, fft, wr,
	input  [NUBITS-1:0] ula,                      // the word a write stores
	input  [NUBITS-1:0] idx,                      // the read index: the ALU before the float normaliser
	input  [MDATAW-1:0] base_addr, stk_ofst,

	output              mem_wr,
	output [MDATAW-1:0] mem_addr_rd, mem_addr_wr,
	output [NUBITS-1:0] mem_data_wr
);

assign mem_data_wr = ula;
assign mem_wr      = wr;

// the address is always base + index: the operand field plus the acc (read)
// or plus the data-stack top (write). A pointer is dereferenced with a raw
// base of 0 in the operand field (LDI 0 / STI 0), or of the constant offset
// it is read at (LDI 3 = mem[acc + 3]). The FFT bit reversal of the index
// (x[k) in C+-) is built into the read path when the program uses ILI and
// into the write path when it uses ISI (cmm_fft_rd / cmm_fft_wr). The read
// index leaves the normaliser out (asmcomp puts a NOP between a normalised
// operation and an LDI/ILI right after it, as for JIZ).
rel_addr #(.MDATAW(MDATAW), .FFTSIZ(FFTSIZ), .USEFFT(ILI)) ra_rd(ldi, fft, idx[MDATAW-1:0], base_addr, mem_addr_rd);
rel_addr #(.MDATAW(MDATAW), .FFTSIZ(FFTSIZ), .USEFFT(ISI)) ra_wr(sti, fft, stk_ofst       , base_addr, mem_addr_wr);

endmodule

// I/O controller -------------------------------------------------------------

// The port number is the instruction's operand, all NBOPER bits of it. It used
// to come cut to the data-address width (MDATAW): a program with few
// variables and more ports lost the port number's top bits (out_en went x in
// simulation, out(2) could reach port 0 on a board). asmcomp sizes NBOPER to
// hold the highest port; the operand is zero-extended here as well, so no
// select falls outside it.

module io_ctrl
#(
	parameter NBOPER = 8,
	parameter NBIOIN = 8,
	parameter NBIOOU = 8,
	parameter    INN = 0,
	parameter  F_INN = 0,
	parameter  P_INN = 0,
	parameter PF_INN = 0,
	parameter    OUT = 0
)(
	input                   clk, rst,
	input                   req_in, out_en,
	input      [NBOPER-1:0] opnd,

	output                  en_in,
	output     [NBIOIN-1:0] addr_in,

	output                  en_out,
	output     [NBIOOU-1:0] addr_out
);

localparam AW = (NBOPER > NBIOIN) ? ((NBOPER > NBIOOU) ? NBOPER : NBIOOU)
                                  : ((NBIOIN > NBIOOU) ? NBIOIN : NBIOOU);
wire [AW-1:0] addr = opnd;                         // zero-extended when narrower

generate if ((INN | F_INN | P_INN | PF_INN) != 0) begin : en_in_sel   assign en_in   = req_in;           end else begin : en_in_sel   assign en_in   =         1'b0  ; end endgenerate
generate if ((INN | F_INN | P_INN | PF_INN) != 0) begin : addr_in_sel assign addr_in = addr[NBIOIN-1:0]; end else begin : addr_in_sel assign addr_in = {NBIOIN{1'b0}}; end endgenerate

// the output side only with OUT (#172): a program that only reads had both
// registers, addr_out loading the operand of every instruction. The port
// number now loads in OUT's cycle alone and holds until the next OUT.
generate if ((OUT) != 0) begin : out_side
	reg              en_r   = 1'b0;
	reg [NBIOOU-1:0] addr_r = 0;
	always @ (posedge clk) if (rst) en_r   <= 1'b0; else en_r   <= out_en;
	always @ (posedge clk) if (rst) addr_r <= 0;    else if (out_en) addr_r <= addr[NBIOOU-1:0];
	assign en_out   = en_r;
	assign addr_out = addr_r;
end else begin : out_side
	assign en_out   = 1'b0;
	assign addr_out = {NBIOOU{1'b0}};
end endgenerate

endmodule

// ****************************************************************************
// Main circuit ***************************************************************
// ****************************************************************************

module core
#(
	// -------------------------------------------------------------------------
	// Internal configuration parameters ---------------------------------------
	// -------------------------------------------------------------------------

	// data flow
	parameter  NBOPCO = 7,               // Number of opcode bits (do not change without updating the instr_decoder)
	parameter  NBOPER = 9,               // Operand width (bits)
	parameter  ITRADD     = 0,           // Interrupt address (PC jumps here while itr=1)
	parameter  TOAQUIADDR = 0,           // #TOAQUI marker address (cheguei is 1 while the address fetched is TOAQUIADDR)

	// memories
	// 6: $clog2 of processor.v's 64 words, the one default set (#185)
	parameter  MDATAW = 6,               // Number of address bits for data memory
	parameter  MINSTW = 6,               // Number of address bits for instruction memory
	parameter  NBINST = NBOPCO + NBOPER, // Instruction-memory width (bits)

	// -------------------------------------------------------------------------
	// User-configured parameters ----------------------------------------------
	// -------------------------------------------------------------------------

	// data flow
	parameter  NUBITS = 32,              // Data width (bits)
	parameter  NBMANT = 23,              // Mantissa width (bits)
	parameter  NBEXPO =  8,              // Exponent width (bits)
	// defaults: the one set named at the top of processor.v

	// memories
	parameter  SDEPTH = 128,             // Instruction stack depth
	parameter  DDEPTH = 128,             // Data stack depth

	// inputs and Outputs
	parameter  NBIOIN =  2,              // Number of IO address bits - input
	parameter  NBIOOU =  2,              // Number of IO address bits - output

	// arithmetic constants
	parameter  signed [NUBITS-1:0] NUGAIN = 128, // norm() divisor (NRM/NRM_M): a power of two, asmcomp enforces it
	parameter  FFTSIZ =  3,              // ILI size for bit reversal
	parameter  FROUND =  0,              // Float rounding level (#FROUND): 0 legacy, 1 truncation + saturation, 2 round to nearest even

	// links to partner processors (docs/design/toma-and-cade-links.md): link k is
	// built only where its bit is set; the opcode parameters TOM/CAD say
	// whether the instructions exist at all
	parameter  NLINK  =  1,              // number of link slots (highest link number used + 1)
	parameter  [NLINK-1:0] TOMS  = 0,    // bit k: the program has TOM k
	parameter  [NLINK-1:0] CADS  = 0,    // bit k: the program has CAD k
	parameter  [NLINK-1:0] LSYNC = 0,    // bit k: link k's partner runs on my clock (_sync forms): no synchronizer

	// -------------------------------------------------------------------------
	// Dynamically configured parameters ---------------------------------------
	// -------------------------------------------------------------------------

	// implements memory read/write
	parameter    LOD   = 0,
	parameter  P_LOD   = 0,
	
	parameter    LDI   = 0,
	parameter    ILI   = 0,
	
	parameter    SET   = 0,
	parameter    SET_P = 0,
	
	parameter    STI   = 0,
	parameter    ISI   = 0,

	// implements the data-stack interface
	parameter    PSH   = 0,
	parameter    POP   = 0,

	// implements I/O ports
	parameter    INN   = 0,
	parameter  F_INN   = 0,
	parameter  P_INN   = 0,
	parameter PF_INN   = 0,
	parameter    OUT   = 0,
	
	// implements jumps
	parameter    JIZ   = 0,
	parameter    JNZ   = 0,
	parameter    CAL   = 0,

	// the links to partner processors (docs/design/toma-and-cade-links.md)
	parameter    TOM   = 0,
	parameter    CAD   = 0,

	// two-parameter arithmetic operations
	parameter    ADD   = 0,
	parameter  S_ADD   = 0,
	parameter  F_ADD   = 0,
	parameter SF_ADD   = 0,

	parameter    MLT   = 0,
	parameter  S_MLT   = 0,
	parameter  F_MLT   = 0,
	parameter SF_MLT   = 0,

	parameter    DIV   = 0,
	parameter  S_DIV   = 0,
	parameter  F_DIV   = 0,
	parameter SF_DIV   = 0,

	parameter    MOD   = 0,
	parameter  S_MOD   = 0,

	parameter    SGN   = 0,
	parameter  S_SGN   = 0,
	parameter  F_SGN   = 0,
	parameter SF_SGN   = 0,

	// one-parameter arithmetic operations
	parameter    NEG   = 0,
	parameter    NEG_M = 0,
	parameter  P_NEG_M = 0,
	parameter  F_NEG   = 0,
	parameter  F_NEG_M = 0,
	parameter PF_NEG_M = 0,

	parameter    ABS   = 0,
	parameter    ABS_M = 0,
	parameter  P_ABS_M = 0,
	parameter  F_ABS   = 0,
	parameter  F_ABS_M = 0,
	parameter PF_ABS_M = 0,

	parameter    PST   = 0,
	parameter    PST_M = 0,
	parameter  P_PST_M = 0,
	parameter  F_PST   = 0,
	parameter  F_PST_M = 0,
	parameter PF_PST_M = 0,

	parameter    NRM   = 0,
	parameter    NRM_M = 0,
	parameter  P_NRM_M = 0,

	parameter    I2F   = 0,
	parameter    I2F_M = 0,
	parameter  P_I2F_M = 0,

	parameter    F2I   = 0,
	parameter    F2I_M = 0,
	parameter  P_F2I_M = 0,

	// two-parameter logical operations
	parameter    AND   = 0,
	parameter  S_AND   = 0,
	parameter    ORR   = 0,
	parameter  S_ORR   = 0,
	parameter    XOR   = 0,
	parameter  S_XOR   = 0,

	// one-parameter logical operations
	parameter    INV   = 0,
	parameter    INV_M = 0,
	parameter  P_INV_M = 0,

	// two-parameter conditional operations
	parameter    LAN   = 0,
	parameter  S_LAN   = 0,
	parameter    LOR   = 0,
	parameter  S_LOR   = 0,
	
	// one-parameter conditional operations
	parameter    LIN   = 0,
	parameter    LIN_M = 0,
	parameter  P_LIN_M = 0,

	// comparison operations
	parameter    LES   = 0,
	parameter  S_LES   = 0,
	parameter  F_LES   = 0,
	parameter SF_LES   = 0,

	parameter    GRE   = 0,
	parameter  S_GRE   = 0,
	parameter  F_GRE   = 0,
	parameter SF_GRE   = 0,

	parameter    EQU   = 0,
	parameter  S_EQU   = 0,

	// bit-shift operations
	parameter    SHL   = 0,
	parameter  S_SHL   = 0,

	parameter    SHR   = 0,
	parameter  S_SHR   = 0,

	parameter    SRS   = 0,
	parameter  S_SRS   = 0,

	// special operations
	parameter  F_ROT   = 0,   // nearest power-of-two square-root approximation (with ACC)
	parameter  F_SU1   = 0,   // floating-point subtraction at input 1
	parameter  F_SU2   = 0,   // floating-point subtraction at input 2
	parameter SF_SU1   = 0,   // floating-point subtraction at input 1 with stack
	parameter SF_SU2   = 0,   // floating-point subtraction at input 2 with stack
	parameter  F_SCL   = 0,   // scale float by 2^k, k from memory
	parameter SF_SCL   = 0,   // scale float by 2^k, k from stack
	parameter    XPO   = 0,   // base-2 exponent of float (acc) as int
	parameter  XPO_M   = 0    // base-2 exponent of float (memory) as int
)(
	input               clk, rst,

	input  [NBINST-1:0] instr,
	output [MINSTW-1:0] instr_addr,

	output              mem_wr,
	output [MDATAW-1:0] mem_addr_rd, mem_addr_wr,
	input  [NUBITS-1:0] mem_data_rd,
	output [NUBITS-1:0] mem_data_wr,

	input  [NUBITS-1:0] io_in,
	output [NBIOIN-1:0] addr_in,
	output [NBIOOU-1:0] addr_out,
	output              req_in,
	output              out_en,

	input               itr,
	output              cheguei,

	// the links to partner processors, one bit / word per link
	output [NLINK       -1:0] toma,       // my T per link: flipped when I offer a value
	output [NLINK       -1:0] cade,       // my C per link: flipped when I take one
	input  [NLINK       -1:0] valeu,      // the partner's C (TOM: has my value been taken?)
	input  [NLINK       -1:0] taqui,      // the partner's T (CAD: is a value offered?)
	input  [NLINK*NUBITS-1:0] cade_dado,  // the partners' toma_dado, link k at bits k*NUBITS (CAD k reads it)
	output [NUBITS      -1:0] toma_dado   // my offered value, the same for every link

`ifdef YANC_SIM_VIS // --------------------------------------------------------
 , output [MINSTW-1:0] pc_sim_val
`endif // ---------------------------------------------------------------------
);

// Instruction fetch ----------------------------------------------------------

wire              if_acc;
wire [NBOPCO-1:0] if_opcode;
wire [NBOPER-1:0] if_operand;
wire              tom_go, cad_go;

instr_fetch #(
	.NBINST     (NBINST     ),
	.MINSTW     (MINSTW     ),
	.ITRADD     (ITRADD     ),
	.TOAQUIADDR (TOAQUIADDR ),
	.NBOPCO     (NBOPCO     ),
	.NBOPER     (NBOPER     ),
	.SDEPTH     (SDEPTH     ),
	.CAL        (CAL        ),
	.JIZ        (JIZ        ),
	.JNZ        (JNZ        ),
	.TOM        (TOM        ),
	.CAD        (CAD        )) instr_fetch (.clk    (clk       ),
	                                .rst    (rst       ),
	                                .itr    (itr       ),
	                                .cheguei(cheguei   ),
	                                .tom_go (tom_go    ),
	                                .cad_go (cad_go    ),
	                                .instr  (instr     ),
	                                .addr   (instr_addr),
	                                .acc    (if_acc    ),
	                                .opcode (if_opcode ),
	                                .operand(if_operand)
	
`ifdef YANC_SIM_VIS // --------------------------------------------------------
                               , .pc_sim_val(pc_sim_val)
`endif // ---------------------------------------------------------------------
);

// toma / cade: the links to partner processors ------------------------------
// (docs/design/toma-and-cade.md, docs/design/toma-and-cade-links.md) On each link each side
// owns one bit: the writer flips T when it offers a value, the reader flips C
// when it takes it; a value waits while T != C. TOM k and CAD k (the operand is
// the link number) jump to themselves (prefetch) until *_go. Every opcode the
// prefetch shows is executed, so a looping TOM/CAD is seen here once per
// cycle. The bits are levels in flip-flops: two looping processors see each
// other's instruction one cycle per turn and could miss one-cycle pulses.
// Unless the program used a _sync form on link k (LSYNC[k]), the partner's
// bit enters through two flip-flops on my clock, and the writer holds its
// value still while T is seen: toma_dado is a register loaded when T flips,
// the accumulator itself without #ITRAD (#171, toma_reg below).
// Built only where needed: link k only with its TOMS/CADS bit; the operand is
// compared and the links multiplexed only with TOM (or CAD) on two or more.

localparam [NBOPCO-1:0] OP_TOM = 21, OP_CAD = 22;

function integer n_ones (input [NLINK-1:0] m); integer i; begin
	n_ones = 0; for (i = 0; i < NLINK; i = i + 1) n_ones = n_ones + m[i];
end endfunction

localparam N_TOM   = n_ones(TOMS);
localparam N_CAD   = n_ones(CADS);
localparam TOM_REG = ((TOMS & ~LSYNC) != 0);   // a synchronized TOM link: toma_dado is a register

wire [NLINK-1:0] tgo, cgo;     // per link: TOM k / CAD k may go on

// the link number: TOM k and CAD k carry it in their operand, and asmcomp
// makes NLINK the highest number used + 1, so its low LW bits are the whole
// number. The selects compare and index those bits, not the whole operand
// (#172).
localparam        LW  = (NLINK > 1) ? $clog2(NLINK) : 1;
wire     [LW-1:0] lnum = if_operand[LW-1:0];
wire [NLINK-1:0] toffer;       // per link: this cycle T flips
wire             tom_offer = |toffer;

genvar k;
generate for (k = 0; k < NLINK; k = k + 1) begin : lnk

	// TOM k --------------------------------------------------------------
	if (TOM != 0 && TOMS[k]) begin : t
		reg  rT = 1'b0;  // T
		reg  rW = 1'b0;  // offered, waiting to be taken: T flips once per TOM
		wire c_in;       // the partner's C, on my clock
		wire me;         // the TOM in decode is TOM k
		if (LSYNC[k]) begin : sync assign c_in = valeu[k]; end
		else begin : sync
			reg s1 = 1'b0, s2 = 1'b0;
			always @ (posedge clk) begin s1 <= valeu[k]; s2 <= s1; end
			assign c_in = s2;
		end
		if (N_TOM > 1) begin : sel assign me = (if_opcode == OP_TOM) & (lnum == k); end
		else           begin : sel assign me = (if_opcode == OP_TOM);                      end
		assign toffer[k] = me & ~rW;
		always @ (posedge clk) begin
			if (rst) begin
				rT <= 1'b0;
				rW <= 1'b0;
			end else if (me) begin
				if (!rW) begin
					rT <= ~rT;
					rW <= 1'b1;
				end else if (c_in == rT)
					rW <= 1'b0;
			end
		end
		assign toma[k] = rT;
		assign tgo [k] = rW & (c_in == rT);
`ifdef YANC_SIM_VIS // --------------------------------------------------------
		// A #PRACA pulse while this TOM waits leaves rW (and rD) as they are:
		// the first TOM k after the restart finds rW set, offers nothing new
		// and goes on once the partner takes the word offered before the
		// pulse, so the new word is lost (#188). The hardware cannot undo it
		// (the partner may already have seen T flip), so the simulation says
		// so; an exception code of its own is left to #36.
		if (ITRADD > 0) begin : itr_warn
			reg itr_q = 1'b0;
			always @ (posedge clk) begin
				itr_q <= itr;
				if (!rst && itr && !itr_q && rW)
					$display("Warning: a #PRACA pulse at %0t while toma(%0d) waits: the first toma(%0d) after the restart hands over the word offered before the pulse (#188)", $time, k, k);
			end
		end
`endif // ---------------------------------------------------------------------
	end else begin : t
		assign toffer[k] = 1'b0;
		assign toma  [k] = 1'b0;
		assign tgo   [k] = 1'b0;
	end

	// CAD k --------------------------------------------------------------
	if (CAD != 0 && CADS[k]) begin : c
		reg  rC = 1'b0;  // C
		wire t_in;       // the partner's T, on my clock
		wire me;         // the CAD in decode is CAD k
		if (LSYNC[k]) begin : sync assign t_in = taqui[k]; end
		else begin : sync
			reg s1 = 1'b0, s2 = 1'b0;
			always @ (posedge clk) begin s1 <= taqui[k]; s2 <= s1; end
			assign t_in = s2;
		end
		if (N_CAD > 1) begin : sel assign me = (if_opcode == OP_CAD) & (lnum == k); end
		else           begin : sel assign me = (if_opcode == OP_CAD);                      end
		always @ (posedge clk) begin
			if      (rst              ) rC <= 1'b0;
			else if (me && t_in != rC) rC <= ~rC;
		end
		assign cade[k] = rC;
		assign cgo [k] = (t_in != rC);
	end else begin : c
		assign cade[k] = 1'b0;
		assign cgo [k] = 1'b0;
	end

end endgenerate

// the go of the TOM / CAD in decode: with one link of the kind its bit alone
// (the others are 0), with more the operand picks it
generate
	if (N_TOM > 1) begin : tom_go_sel assign tom_go = tgo[lnum]; end
	else           begin : tom_go_sel assign tom_go = |tgo;            end
	if (N_CAD > 1) begin : cad_go_sel assign cad_go = cgo[lnum]; end
	else           begin : cad_go_sel assign cad_go = |cgo;            end
endgenerate

// the word CAD reads: with one CAD link that link's word, with more the
// operand picks it (the multiplexer a fan-in costs)
function integer first_one (input [NLINK-1:0] m); integer i; begin
	first_one = 0; for (i = NLINK-1; i >= 0; i = i - 1) if (m[i]) first_one = i;
end endfunction
localparam CAD_ONE = first_one(CADS);

wire [NUBITS-1:0] cad_word;
generate
	if (N_CAD > 1) begin : cad_word_sel assign cad_word = cade_dado[lnum*NUBITS +: NUBITS]; end
	else           begin : cad_word_sel assign cad_word = cade_dado[CAD_ONE   *NUBITS +: NUBITS]; end
endgenerate

// Instruction decoder --------------------------------------------------------

wire [NBOPCO-1:0] id_opcode  = if_opcode;

wire [       5:0] id_ula_op;
wire              id_dsp_push, id_dsp_pop;
wire              id_sti, id_ldi, id_fft, id_wr;
wire              id_req_in, id_out_en;

instr_dec #(.NBOPCO  ( NBOPCO ),
			   .LOD  (   LOD  ),
			 .P_LOD  ( P_LOD  ),
			   .LDI  (   LDI  ),
			   .ILI  (   ILI  ),
			   .SET  (   SET  ),
			   .SET_P(   SET_P),
			   .STI  (   STI  ),
			   .ISI  (   ISI  ),
			   .PSH  (   PSH  ),
			   .POP  (   POP  ),
			   .INN  (   INN  ),
			 .F_INN  ( F_INN  ),
			 .P_INN  ( P_INN  ),
			.PF_INN  (PF_INN  ),
			   .OUT  (   OUT  ),
			   .ADD  (   ADD  ),
			 .S_ADD  ( S_ADD  ),
			 .F_ADD  ( F_ADD  ),
			.SF_ADD  (SF_ADD  ),
			   .MLT  (   MLT  ),
			 .S_MLT  ( S_MLT  ),
			 .F_MLT  ( F_MLT  ),
			.SF_MLT  (SF_MLT  ),
			   .DIV  (   DIV  ),
			 .S_DIV  ( S_DIV  ),
			 .F_DIV  ( F_DIV  ),
			.SF_DIV  (SF_DIV  ),
			   .MOD  (   MOD  ),
			 .S_MOD  ( S_MOD  ),
			   .SGN  (   SGN  ),
			 .S_SGN  ( S_SGN  ),
			 .F_SGN  ( F_SGN  ),
			.SF_SGN  (SF_SGN  ),
			   .NEG  (   NEG  ),
			   .NEG_M(   NEG_M),
			 .P_NEG_M( P_NEG_M),
			 .F_NEG  ( F_NEG  ),
			 .F_NEG_M( F_NEG_M),
			.PF_NEG_M(PF_NEG_M),
			   .ABS  (   ABS  ),
			   .ABS_M(   ABS_M),
			 .P_ABS_M( P_ABS_M),
			 .F_ABS  ( F_ABS  ),
			 .F_ABS_M(F_ABS_M ),
			.PF_ABS_M(PF_ABS_M),
			   .PST  (   PST  ),
			   .PST_M(   PST_M),
			 .P_PST_M( P_PST_M),
			 .F_PST  ( F_PST  ),
			 .F_PST_M(F_PST_M ),
			.PF_PST_M(PF_PST_M),
			   .NRM  (   NRM  ),
			   .NRM_M(   NRM_M),
			 .P_NRM_M( P_NRM_M),
			   .I2F  (   I2F  ),
			   .I2F_M(   I2F_M),
			 .P_I2F_M( P_I2F_M),
			   .F2I  (   F2I  ),
			   .F2I_M(   F2I_M),
			 .P_F2I_M( P_F2I_M),
			   .AND  (   AND  ),
			 .S_AND  ( S_AND  ),
			   .ORR  (   ORR  ),
			 .S_ORR  ( S_ORR  ),
			   .XOR  (   XOR  ),
			 .S_XOR  ( S_XOR  ),
			   .INV  (   INV  ),
			   .INV_M(   INV_M),
			 .P_INV_M( P_INV_M),
			   .LAN  (   LAN  ),
			 .S_LAN  ( S_LAN  ),
			   .LOR  (   LOR  ),
			 .S_LOR  ( S_LOR  ),
			   .LIN  (   LIN  ),
			   .LIN_M(   LIN_M),
			 .P_LIN_M( P_LIN_M),
			   .LES  (   LES  ),
			 .S_LES  ( S_LES  ),
			 .F_LES  ( F_LES  ),
			.SF_LES  (SF_LES  ),
			   .GRE  (   GRE  ),
			 .S_GRE  ( S_GRE  ),
			 .F_GRE  ( F_GRE  ),
			.SF_GRE  (SF_GRE  ),
			   .EQU  (   EQU  ),
			 .S_EQU  ( S_EQU  ),
			   .SHL  (   SHL  ),
			 .S_SHL  ( S_SHL  ),
			   .SHR  (   SHR  ),
			 .S_SHR  ( S_SHR  ),
			   .SRS  (   SRS  ),
			 .S_SRS  ( S_SRS  ),
			 .F_ROT  ( F_ROT  ),
			 .F_SU1  ( F_SU1  ),
			 .F_SU2  ( F_SU2  ),
			.SF_SU1  (SF_SU1  ),
			.SF_SU2  (SF_SU2  ),
			 .F_SCL  ( F_SCL  ),
			.SF_SCL  (SF_SCL  ),
			   .XPO  (   XPO  ),
			 .XPO_M  ( XPO_M  )) id(clk, rst,
                                    id_opcode,
                                    id_dsp_push, id_dsp_pop,
                                    id_ula_op,
                                    id_wr,
                                    id_req_in, id_out_en,
                                    id_ldi, id_sti, id_fft);

// Data stack -----------------------------------------------------------------

wire              sp_push = id_dsp_push;
wire              sp_pop  = id_dsp_pop;
wire [NUBITS-1:0] sp_in, sp_data;

// a #PRACA pulse empties it, as the instruction stack (instr_fetch): the push
// or pop of the instruction in decode is dropped with the rest (#188)
wire              sp_clr;
generate
	if (ITRADD > 0) begin : sp_clr_sel assign sp_clr = rst | itr; end
	else            begin : sp_clr_sel assign sp_clr = rst;       end
endgenerate

stack #(.NADDR((DDEPTH > 1) ? $clog2(DDEPTH) : 1),   // a 1-word stack still has a 1-bit pointer
        .DEPTH(DDEPTH),
        .NBITS(NUBITS)) sp(clk, sp_clr, sp_push, sp_pop, sp_in, sp_data);

// ALU input controls ---------------------------------------------------------

wire [NUBITS-1:0] ula_data_in1;
wire [NUBITS-1:0] ula_data_in2;
wire [NUBITS-1:0] uic_acc;

// Accumulator register, declared up here (ahead of the uic_in2 generate that
// references it in its else arm) so the reference is backward. iverilog v13
// rejects use-before-declaration; v12 tolerated it.
reg signed [NUBITS-1:0] racc = 0;

// input in1
ula_in1_ctrl #(.NUBITS(NUBITS)) uic1 (clk, rst, id_dsp_pop, mem_data_rd, sp_data, ula_data_in1);

// input in2
// CAD reads the partner's out bus through the same path as INN, but raises no
// req_in outside: the choice between io_in and cade_dado is made in here, and
// only a program with both pays for it.
localparam HAS_INN = ((INN | P_INN | F_INN | PF_INN) != 0);
localparam HAS_CAD = (CAD != 0);
generate if (HAS_INN && HAS_CAD) begin : uic_in2
wire              is_cad = (id_opcode == OP_CAD);
wire [NUBITS-1:0] in_sel = (is_cad) ? cad_word : io_in;
ula_in2_ctrl #(.NUBITS(NUBITS)) uic2 (clk, rst, id_req_in | is_cad, uic_acc, in_sel, ula_data_in2);
end else if (HAS_CAD) begin : uic_in2   // cade() and no in(): the partner is the only input
ula_in2_ctrl #(.NUBITS(NUBITS)) uic2 (clk, rst, id_opcode == OP_CAD, uic_acc, cad_word, ula_data_in2);
end else if (HAS_INN) begin : uic_in2   // in() only: as before the link existed
ula_in2_ctrl #(.NUBITS(NUBITS)) uic2 (clk, rst, id_req_in , uic_acc, io_in, ula_data_in2);
end else begin : uic_in2 assign ula_data_in2 = racc; end
endgenerate

// Arithmetic Logic Unit ------------------------------------------------------

wire signed [NUBITS-1:0] ula_out;
wire        [NUBITS-1:0] ula_fast;   // ula_out without the normalised operations

ula #(.NUBITS (NUBITS ),
      .NBMANT (NBMANT ),
      .NBEXPO (NBEXPO ),
      .NUGAIN (NUGAIN ),
      .FROUND (FROUND ),
        .ADD  (  ADD   |  S_ADD  ),
	  .F_ADD  (F_ADD   | SF_ADD  ),
        .MLT  (  MLT   |  S_MLT  ),
      .F_MLT  (F_MLT   | SF_MLT  ),
        .DIV  (  DIV   |  S_DIV  ),
      .F_DIV  (F_DIV   | SF_DIV  ),
        .MOD  (  MOD   |  S_MOD  ),
        .SGN  (  SGN   |  S_SGN  ),
      .F_SGN  (F_SGN   | SF_SGN  ),
        .NEG  (  NEG             ),
        .NEG_M(  NEG_M |  P_NEG_M),
      .F_NEG  (F_NEG             ),
      .F_NEG_M(F_NEG_M | PF_NEG_M),
        .ABS  (  ABS             ),
        .ABS_M(  ABS_M |  P_ABS_M),
      .F_ABS  (F_ABS             ),
      .F_ABS_M(F_ABS_M | PF_ABS_M),
        .PST  (  PST             ),
        .PST_M(  PST_M |  P_PST_M),
      .F_PST  (F_PST             ),
      .F_PST_M(F_PST_M | PF_PST_M),
        .NRM  (  NRM             ),
        .NRM_M(  NRM_M |  P_NRM_M),
        .I2F  (  I2F   |  F_INN  | PF_INN),
        .I2F_M(  I2F_M |  P_I2F_M),
        .F2I  (  F2I             ),
        .F2I_M(  F2I_M |  P_F2I_M),
        .AND  (  AND   |  S_AND  ),
        .ORR  (  ORR   |  S_ORR  ),
        .XOR  (  XOR   |  S_XOR  ),
        .INV  (  INV             ),
        .INV_M(  INV_M |  P_INV_M),
        .LAN  (  LAN   |  S_LAN  ),
        .LOR  (  LOR   |  S_LOR  ),
        .LIN  (  LIN             ),
        .LIN_M(  LIN_M |  P_LIN_M),
        .LES  (  LES   |  S_LES  ),
      .F_LES  (F_LES   | SF_LES  ),
        .GRE  (  GRE   |  S_GRE  ),
      .F_GRE  (F_GRE   | SF_GRE  ),
        .EQU  (  EQU   |  S_EQU  ),
        .SHL  (  SHL   |  S_SHL  ),
        .SHR  (  SHR   |  S_SHR  ),
		.SRS  (  SRS   |  S_SRS  ),	
	  .F_ROT  (F_ROT             ),
	  .F_SU1  (F_SU1   | SF_SU1  ),
	  .F_SU2  (F_SU2   | SF_SU2  ),
	  .F_SCL  (F_SCL   | SF_SCL  ),
	  .XPO    (XPO              ),
	  .XPO_M  (XPO_M            )) ula (id_ula_op, ula_data_in1, ula_data_in2, ula_out, ula_fast, clk);

assign sp_in = ula_out;

// Accumulator ----------------------------------------------------------------
// (the racc reg itself is declared earlier, above the uic_in2 generate)

always @ (posedge clk) if (rst) racc <= 0; else racc <= ula_out; // synchronous reset (FPGA-friendly)

// toma_dado: the value the partners' CAD reads, one for every link (a writer
// is inside one TOM at a time and leaves it only once the value was taken).
// Only _sync links: the out bus itself (TOM keeps it there while it loops).
// Any synchronized TOM link: a register loaded in the cycle T flips, so it is
// already still when the partner sees T.
// Without an interrupt that register is the accumulator (#171): racc loads
// the same ula_out on the same edge, and holds it while TOM loops (its ALU
// operation passes the accumulator, and no input comes in); the writer
// leaves TOM only once the value was taken. An interrupt can leave TOM
// mid-wait and change the accumulator, so with #ITRAD rD stays.
generate if (TOM != 0 && TOM_REG && ITRADD == 0) begin : toma_reg
	assign toma_dado = racc;
end else if (TOM != 0 && TOM_REG) begin : toma_reg
	reg [NUBITS-1:0] rD = 0;
	always @ (posedge clk) if (tom_offer) rD <= ula_out;
	assign toma_dado = rD;
end else begin : toma_reg
	assign toma_dado = ula_out;
end endgenerate

assign uic_acc = racc;
// JIZ / JNZ branch decision: if_acc must reflect "accumulator is non-zero" across the
// WHOLE word, not just bit 0. Using ula_out[0] alone made `while (x)` / `if (x)`
// misbehave for any multi-bit value with a clear LSB (e.g. 2, 4, ...). The
// OR-reduction gives true C truthiness; JIZ jumps iff the word is exactly zero.
// It reads the ALU before the float normaliser (ula_fast): the normaliser is
// the longest path, and asmcomp puts a NOP between a normalised operation and
// a JIZ or JNZ right after it (asm_normnop), so the word tested is never one of them.
assign  if_acc = |ula_fast;

// Indirect Addressing --------------------------------------------------------

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
endgenerate

// I/O Control ----------------------------------------------------------------

generate if ((INN | F_INN | P_INN | PF_INN | OUT) != 0) begin : io_ports
io_ctrl #(.NBOPER(NBOPER),
          .NBIOIN(NBIOIN),
          .NBIOOU(NBIOOU),
		     .INN(   INN),
		   .F_INN( F_INN),
		   .P_INN( P_INN),
		  .PF_INN(PF_INN),
		     .OUT(   OUT)) io(clk, rst, id_req_in, id_out_en,
                              if_operand,
                              req_in, addr_in, out_en, addr_out);
end else begin : io_ports
assign req_in   = 1'b0;
assign out_en   = 1'b0;
assign addr_in  = {NBIOIN{1'b0}};
assign addr_out = {NBIOOU{1'b0}};
end endgenerate

endmodule
