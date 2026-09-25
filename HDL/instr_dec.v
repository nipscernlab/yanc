// ----------------------------------------------------------------------------
// instruction decoder -- opcode -> ALU operation and control lines ------------
// ----------------------------------------------------------------------------
//
// The decode table below is the whole decoder: one row per opcode, numbered as
// in Compilers/common/isa.tsv (Scripts/check_isa.py holds the two in step).
// Every row is guarded by the parameter of its mnemonic, which asmcomp sets to
// 1 only for the opcodes the program uses: an unused row is constant-false,
// so synthesis removes its comparator and the decoder grows with the program,
// like the rest of the ALU. Measured when this form replaced the hand-expanded
// one (Yosys, equivalence proven on 7 programs' parameter sets; Quartus
// sapho_all: 76 -> 31 ALUTs for the decoder, no RAM inferred). The table is a
// combinational case with the register after it: a case inside the clocked
// block, with constant outputs, is the pattern tools map to a ROM.

module instr_dec
#(
    parameter  NBOPCO  = 7,   // number of opcode bits
    parameter  MDATAW  = 8,   // number of address bits for data memory

    // one per mnemonic, set to 1 by asmcomp when the program uses it

    // memory and stack
    parameter LOD      = 0,
    parameter P_LOD    = 0,
    parameter LDI      = 0,
    parameter ILI      = 0,
    parameter SET      = 0,
    parameter SET_P    = 0,
    parameter STI      = 0,
    parameter ISI      = 0,
    parameter PSH      = 0,
    parameter POP      = 0,

    // I/O
    parameter INN      = 0,
    parameter F_INN    = 0,
    parameter P_INN    = 0,
    parameter PF_INN   = 0,
    parameter OUT      = 0,

    // int arithmetic
    parameter ADD      = 0,
    parameter S_ADD    = 0,
    parameter MLT      = 0,
    parameter S_MLT    = 0,
    parameter DIV      = 0,
    parameter S_DIV    = 0,
    parameter MOD      = 0,
    parameter S_MOD    = 0,
    parameter SGN      = 0,
    parameter S_SGN    = 0,

    // float arithmetic
    parameter F_ADD    = 0,
    parameter SF_ADD   = 0,
    parameter F_SU1    = 0,
    parameter F_SU2    = 0,
    parameter SF_SU1   = 0,
    parameter SF_SU2   = 0,
    parameter F_MLT    = 0,
    parameter SF_MLT   = 0,
    parameter F_DIV    = 0,
    parameter SF_DIV   = 0,
    parameter F_SGN    = 0,
    parameter SF_SGN   = 0,
    parameter F_SCL    = 0,
    parameter SF_SCL   = 0,

    // int unary
    parameter NEG      = 0,
    parameter NEG_M    = 0,
    parameter P_NEG_M  = 0,
    parameter ABS      = 0,
    parameter ABS_M    = 0,
    parameter P_ABS_M  = 0,
    parameter PST      = 0,
    parameter PST_M    = 0,
    parameter P_PST_M  = 0,
    parameter NRM      = 0,
    parameter NRM_M    = 0,
    parameter P_NRM_M  = 0,
    parameter INV      = 0,
    parameter INV_M    = 0,
    parameter P_INV_M  = 0,
    parameter LIN      = 0,
    parameter LIN_M    = 0,
    parameter P_LIN_M  = 0,

    // float unary
    parameter F_NEG    = 0,
    parameter F_NEG_M  = 0,
    parameter PF_NEG_M = 0,
    parameter F_ABS    = 0,
    parameter F_ABS_M  = 0,
    parameter PF_ABS_M = 0,
    parameter F_PST    = 0,
    parameter F_PST_M  = 0,
    parameter PF_PST_M = 0,
    parameter F_ROT    = 0,
    parameter XPO      = 0,
    parameter XPO_M    = 0,

    // conversions
    parameter I2F      = 0,
    parameter I2F_M    = 0,
    parameter P_I2F_M  = 0,
    parameter F2I      = 0,
    parameter F2I_M    = 0,
    parameter P_F2I_M  = 0,

    // logic
    parameter AND      = 0,
    parameter S_AND    = 0,
    parameter ORR      = 0,
    parameter S_ORR    = 0,
    parameter XOR      = 0,
    parameter S_XOR    = 0,
    parameter LAN      = 0,
    parameter S_LAN    = 0,
    parameter LOR      = 0,
    parameter S_LOR    = 0,

    // comparisons
    parameter LES      = 0,
    parameter S_LES    = 0,
    parameter GRE      = 0,
    parameter S_GRE    = 0,
    parameter EQU      = 0,
    parameter S_EQU    = 0,
    parameter F_LES    = 0,
    parameter SF_LES   = 0,
    parameter F_GRE    = 0,
    parameter SF_GRE   = 0,

    // shifts
    parameter SHL      = 0,
    parameter S_SHL    = 0,
    parameter SHR      = 0,
    parameter S_SHR    = 0,
    parameter SRS      = 0,
    parameter S_SRS    = 0
)(
    input                   clk, rst,
    input      [NBOPCO-1:0] opcode,

    output                  push, pop,

    output reg [       5:0] ula_op,

    output                  mem_wr,
    output                  req_in, out_en,
    output                  ldi, sti, fft
);

// {ula_op, push, pop, mem_wr, req_in, out_en, ldi, sti, fft}
reg [13:0] ctl;

always @ (*) begin
    ctl = 14'd0;                                    // pass-acc, no push, no pop, no write
    case (opcode)
    //                        ula    push pop  wr   in   out  ldi  sti  fft
    // memory and stack
    7'd1  : if (LOD     ) ctl = {6'd1 , 1'b0, 1'b0, 1'b0, 1'b0, 1'b0, 1'b0, 1'b0, 1'b0};
    7'd2  : if (P_LOD   ) ctl = {6'd1 , 1'b1, 1'b0, 1'b0, 1'b0, 1'b0, 1'b0, 1'b0, 1'b0};
    7'd3  : if (LDI     ) ctl = {6'd1 , 1'b0, 1'b0, 1'b0, 1'b0, 1'b0, 1'b1, 1'b0, 1'b0};
    7'd4  : if (ILI     ) ctl = {6'd1 , 1'b0, 1'b0, 1'b0, 1'b0, 1'b0, 1'b1, 1'b0, 1'b1};
    7'd5  : if (SET     ) ctl = {6'd0 , 1'b0, 1'b0, 1'b1, 1'b0, 1'b0, 1'b0, 1'b0, 1'b0};
    7'd6  : if (SET_P   ) ctl = {6'd1 , 1'b0, 1'b1, 1'b1, 1'b0, 1'b0, 1'b0, 1'b0, 1'b0};
    7'd7  : if (STI     ) ctl = {6'd0 , 1'b0, 1'b1, 1'b1, 1'b0, 1'b0, 1'b0, 1'b1, 1'b0};
    7'd8  : if (ISI     ) ctl = {6'd0 , 1'b0, 1'b1, 1'b1, 1'b0, 1'b0, 1'b0, 1'b1, 1'b1};
    7'd9  : if (PSH     ) ctl = {6'd0 , 1'b1, 1'b0, 1'b0, 1'b0, 1'b0, 1'b0, 1'b0, 1'b0};
    7'd10 : if (POP     ) ctl = {6'd1 , 1'b0, 1'b1, 1'b0, 1'b0, 1'b0, 1'b0, 1'b0, 1'b0};
    // I/O
    7'd11 : if (INN     ) ctl = {6'd0 , 1'b0, 1'b0, 1'b0, 1'b1, 1'b0, 1'b0, 1'b0, 1'b0};
    7'd12 : if (F_INN   ) ctl = {6'd25, 1'b0, 1'b0, 1'b0, 1'b1, 1'b0, 1'b0, 1'b0, 1'b0};
    7'd13 : if (P_INN   ) ctl = {6'd0 , 1'b1, 1'b0, 1'b0, 1'b1, 1'b0, 1'b0, 1'b0, 1'b0};
    7'd14 : if (PF_INN  ) ctl = {6'd25, 1'b1, 1'b0, 1'b0, 1'b1, 1'b0, 1'b0, 1'b0, 1'b0};
    7'd15 : if (OUT     ) ctl = {6'd0 , 1'b0, 1'b0, 1'b0, 1'b0, 1'b1, 1'b0, 1'b0, 1'b0};
    // int arithmetic
    7'd20 : if (ADD     ) ctl = {6'd2 , 1'b0, 1'b0, 1'b0, 1'b0, 1'b0, 1'b0, 1'b0, 1'b0};
    7'd21 : if (S_ADD   ) ctl = {6'd2 , 1'b0, 1'b1, 1'b0, 1'b0, 1'b0, 1'b0, 1'b0, 1'b0};
    7'd22 : if (MLT     ) ctl = {6'd4 , 1'b0, 1'b0, 1'b0, 1'b0, 1'b0, 1'b0, 1'b0, 1'b0};
    7'd23 : if (S_MLT   ) ctl = {6'd4 , 1'b0, 1'b1, 1'b0, 1'b0, 1'b0, 1'b0, 1'b0, 1'b0};
    7'd24 : if (DIV     ) ctl = {6'd6 , 1'b0, 1'b0, 1'b0, 1'b0, 1'b0, 1'b0, 1'b0, 1'b0};
    7'd25 : if (S_DIV   ) ctl = {6'd6 , 1'b0, 1'b1, 1'b0, 1'b0, 1'b0, 1'b0, 1'b0, 1'b0};
    7'd26 : if (MOD     ) ctl = {6'd8 , 1'b0, 1'b0, 1'b0, 1'b0, 1'b0, 1'b0, 1'b0, 1'b0};
    7'd27 : if (S_MOD   ) ctl = {6'd8 , 1'b0, 1'b1, 1'b0, 1'b0, 1'b0, 1'b0, 1'b0, 1'b0};
    7'd28 : if (SGN     ) ctl = {6'd9 , 1'b0, 1'b0, 1'b0, 1'b0, 1'b0, 1'b0, 1'b0, 1'b0};
    7'd29 : if (S_SGN   ) ctl = {6'd9 , 1'b0, 1'b1, 1'b0, 1'b0, 1'b0, 1'b0, 1'b0, 1'b0};
    // float arithmetic
    7'd30 : if (F_ADD   ) ctl = {6'd3 , 1'b0, 1'b0, 1'b0, 1'b0, 1'b0, 1'b0, 1'b0, 1'b0};
    7'd31 : if (SF_ADD  ) ctl = {6'd3 , 1'b0, 1'b1, 1'b0, 1'b0, 1'b0, 1'b0, 1'b0, 1'b0};
    7'd32 : if (F_SU1   ) ctl = {6'd47, 1'b0, 1'b0, 1'b0, 1'b0, 1'b0, 1'b0, 1'b0, 1'b0};
    7'd33 : if (F_SU2   ) ctl = {6'd48, 1'b0, 1'b0, 1'b0, 1'b0, 1'b0, 1'b0, 1'b0, 1'b0};
    7'd34 : if (SF_SU1  ) ctl = {6'd47, 1'b0, 1'b1, 1'b0, 1'b0, 1'b0, 1'b0, 1'b0, 1'b0};
    7'd35 : if (SF_SU2  ) ctl = {6'd48, 1'b0, 1'b1, 1'b0, 1'b0, 1'b0, 1'b0, 1'b0, 1'b0};
    7'd36 : if (F_MLT   ) ctl = {6'd5 , 1'b0, 1'b0, 1'b0, 1'b0, 1'b0, 1'b0, 1'b0, 1'b0};
    7'd37 : if (SF_MLT  ) ctl = {6'd5 , 1'b0, 1'b1, 1'b0, 1'b0, 1'b0, 1'b0, 1'b0, 1'b0};
    7'd38 : if (F_DIV   ) ctl = {6'd7 , 1'b0, 1'b0, 1'b0, 1'b0, 1'b0, 1'b0, 1'b0, 1'b0};
    7'd39 : if (SF_DIV  ) ctl = {6'd7 , 1'b0, 1'b1, 1'b0, 1'b0, 1'b0, 1'b0, 1'b0, 1'b0};
    7'd40 : if (F_SGN   ) ctl = {6'd10, 1'b0, 1'b0, 1'b0, 1'b0, 1'b0, 1'b0, 1'b0, 1'b0};
    7'd41 : if (SF_SGN  ) ctl = {6'd10, 1'b0, 1'b1, 1'b0, 1'b0, 1'b0, 1'b0, 1'b0, 1'b0};
    7'd42 : if (F_SCL   ) ctl = {6'd49, 1'b0, 1'b0, 1'b0, 1'b0, 1'b0, 1'b0, 1'b0, 1'b0};
    7'd43 : if (SF_SCL  ) ctl = {6'd49, 1'b0, 1'b1, 1'b0, 1'b0, 1'b0, 1'b0, 1'b0, 1'b0};
    // int unary
    7'd44 : if (NEG     ) ctl = {6'd11, 1'b0, 1'b0, 1'b0, 1'b0, 1'b0, 1'b0, 1'b0, 1'b0};
    7'd45 : if (NEG_M   ) ctl = {6'd12, 1'b0, 1'b0, 1'b0, 1'b0, 1'b0, 1'b0, 1'b0, 1'b0};
    7'd46 : if (P_NEG_M ) ctl = {6'd12, 1'b1, 1'b0, 1'b0, 1'b0, 1'b0, 1'b0, 1'b0, 1'b0};
    7'd47 : if (ABS     ) ctl = {6'd15, 1'b0, 1'b0, 1'b0, 1'b0, 1'b0, 1'b0, 1'b0, 1'b0};
    7'd48 : if (ABS_M   ) ctl = {6'd16, 1'b0, 1'b0, 1'b0, 1'b0, 1'b0, 1'b0, 1'b0, 1'b0};
    7'd49 : if (P_ABS_M ) ctl = {6'd16, 1'b1, 1'b0, 1'b0, 1'b0, 1'b0, 1'b0, 1'b0, 1'b0};
    7'd50 : if (PST     ) ctl = {6'd19, 1'b0, 1'b0, 1'b0, 1'b0, 1'b0, 1'b0, 1'b0, 1'b0};
    7'd51 : if (PST_M   ) ctl = {6'd20, 1'b0, 1'b0, 1'b0, 1'b0, 1'b0, 1'b0, 1'b0, 1'b0};
    7'd52 : if (P_PST_M ) ctl = {6'd20, 1'b1, 1'b0, 1'b0, 1'b0, 1'b0, 1'b0, 1'b0, 1'b0};
    7'd53 : if (NRM     ) ctl = {6'd23, 1'b0, 1'b0, 1'b0, 1'b0, 1'b0, 1'b0, 1'b0, 1'b0};
    7'd54 : if (NRM_M   ) ctl = {6'd24, 1'b0, 1'b0, 1'b0, 1'b0, 1'b0, 1'b0, 1'b0, 1'b0};
    7'd55 : if (P_NRM_M ) ctl = {6'd24, 1'b1, 1'b0, 1'b0, 1'b0, 1'b0, 1'b0, 1'b0, 1'b0};
    7'd56 : if (INV     ) ctl = {6'd32, 1'b0, 1'b0, 1'b0, 1'b0, 1'b0, 1'b0, 1'b0, 1'b0};
    7'd57 : if (INV_M   ) ctl = {6'd33, 1'b0, 1'b0, 1'b0, 1'b0, 1'b0, 1'b0, 1'b0, 1'b0};
    7'd58 : if (P_INV_M ) ctl = {6'd33, 1'b1, 1'b0, 1'b0, 1'b0, 1'b0, 1'b0, 1'b0, 1'b0};
    7'd59 : if (LIN     ) ctl = {6'd36, 1'b0, 1'b0, 1'b0, 1'b0, 1'b0, 1'b0, 1'b0, 1'b0};
    7'd60 : if (LIN_M   ) ctl = {6'd37, 1'b0, 1'b0, 1'b0, 1'b0, 1'b0, 1'b0, 1'b0, 1'b0};
    7'd61 : if (P_LIN_M ) ctl = {6'd37, 1'b1, 1'b0, 1'b0, 1'b0, 1'b0, 1'b0, 1'b0, 1'b0};
    // float unary
    7'd62 : if (F_NEG   ) ctl = {6'd13, 1'b0, 1'b0, 1'b0, 1'b0, 1'b0, 1'b0, 1'b0, 1'b0};
    7'd63 : if (F_NEG_M ) ctl = {6'd14, 1'b0, 1'b0, 1'b0, 1'b0, 1'b0, 1'b0, 1'b0, 1'b0};
    7'd64 : if (PF_NEG_M) ctl = {6'd14, 1'b1, 1'b0, 1'b0, 1'b0, 1'b0, 1'b0, 1'b0, 1'b0};
    7'd65 : if (F_ABS   ) ctl = {6'd17, 1'b0, 1'b0, 1'b0, 1'b0, 1'b0, 1'b0, 1'b0, 1'b0};
    7'd66 : if (F_ABS_M ) ctl = {6'd18, 1'b0, 1'b0, 1'b0, 1'b0, 1'b0, 1'b0, 1'b0, 1'b0};
    7'd67 : if (PF_ABS_M) ctl = {6'd18, 1'b1, 1'b0, 1'b0, 1'b0, 1'b0, 1'b0, 1'b0, 1'b0};
    7'd68 : if (F_PST   ) ctl = {6'd21, 1'b0, 1'b0, 1'b0, 1'b0, 1'b0, 1'b0, 1'b0, 1'b0};
    7'd69 : if (F_PST_M ) ctl = {6'd22, 1'b0, 1'b0, 1'b0, 1'b0, 1'b0, 1'b0, 1'b0, 1'b0};
    7'd70 : if (PF_PST_M) ctl = {6'd22, 1'b1, 1'b0, 1'b0, 1'b0, 1'b0, 1'b0, 1'b0, 1'b0};
    7'd71 : if (F_ROT   ) ctl = {6'd46, 1'b0, 1'b0, 1'b0, 1'b0, 1'b0, 1'b0, 1'b0, 1'b0};
    7'd72 : if (XPO     ) ctl = {6'd50, 1'b0, 1'b0, 1'b0, 1'b0, 1'b0, 1'b0, 1'b0, 1'b0};
    7'd73 : if (XPO_M   ) ctl = {6'd51, 1'b0, 1'b0, 1'b0, 1'b0, 1'b0, 1'b0, 1'b0, 1'b0};
    // conversions
    7'd74 : if (I2F     ) ctl = {6'd25, 1'b0, 1'b0, 1'b0, 1'b0, 1'b0, 1'b0, 1'b0, 1'b0};
    7'd75 : if (I2F_M   ) ctl = {6'd26, 1'b0, 1'b0, 1'b0, 1'b0, 1'b0, 1'b0, 1'b0, 1'b0};
    7'd76 : if (P_I2F_M ) ctl = {6'd26, 1'b1, 1'b0, 1'b0, 1'b0, 1'b0, 1'b0, 1'b0, 1'b0};
    7'd77 : if (F2I     ) ctl = {6'd27, 1'b0, 1'b0, 1'b0, 1'b0, 1'b0, 1'b0, 1'b0, 1'b0};
    7'd78 : if (F2I_M   ) ctl = {6'd28, 1'b0, 1'b0, 1'b0, 1'b0, 1'b0, 1'b0, 1'b0, 1'b0};
    7'd79 : if (P_F2I_M ) ctl = {6'd28, 1'b1, 1'b0, 1'b0, 1'b0, 1'b0, 1'b0, 1'b0, 1'b0};
    // logic
    7'd80 : if (AND     ) ctl = {6'd29, 1'b0, 1'b0, 1'b0, 1'b0, 1'b0, 1'b0, 1'b0, 1'b0};
    7'd81 : if (S_AND   ) ctl = {6'd29, 1'b0, 1'b1, 1'b0, 1'b0, 1'b0, 1'b0, 1'b0, 1'b0};
    7'd82 : if (ORR     ) ctl = {6'd30, 1'b0, 1'b0, 1'b0, 1'b0, 1'b0, 1'b0, 1'b0, 1'b0};
    7'd83 : if (S_ORR   ) ctl = {6'd30, 1'b0, 1'b1, 1'b0, 1'b0, 1'b0, 1'b0, 1'b0, 1'b0};
    7'd84 : if (XOR     ) ctl = {6'd31, 1'b0, 1'b0, 1'b0, 1'b0, 1'b0, 1'b0, 1'b0, 1'b0};
    7'd85 : if (S_XOR   ) ctl = {6'd31, 1'b0, 1'b1, 1'b0, 1'b0, 1'b0, 1'b0, 1'b0, 1'b0};
    7'd86 : if (LAN     ) ctl = {6'd34, 1'b0, 1'b0, 1'b0, 1'b0, 1'b0, 1'b0, 1'b0, 1'b0};
    7'd87 : if (S_LAN   ) ctl = {6'd34, 1'b0, 1'b1, 1'b0, 1'b0, 1'b0, 1'b0, 1'b0, 1'b0};
    7'd88 : if (LOR     ) ctl = {6'd35, 1'b0, 1'b0, 1'b0, 1'b0, 1'b0, 1'b0, 1'b0, 1'b0};
    7'd89 : if (S_LOR   ) ctl = {6'd35, 1'b0, 1'b1, 1'b0, 1'b0, 1'b0, 1'b0, 1'b0, 1'b0};
    // comparisons
    7'd90 : if (LES     ) ctl = {6'd38, 1'b0, 1'b0, 1'b0, 1'b0, 1'b0, 1'b0, 1'b0, 1'b0};
    7'd91 : if (S_LES   ) ctl = {6'd38, 1'b0, 1'b1, 1'b0, 1'b0, 1'b0, 1'b0, 1'b0, 1'b0};
    7'd92 : if (GRE     ) ctl = {6'd40, 1'b0, 1'b0, 1'b0, 1'b0, 1'b0, 1'b0, 1'b0, 1'b0};
    7'd93 : if (S_GRE   ) ctl = {6'd40, 1'b0, 1'b1, 1'b0, 1'b0, 1'b0, 1'b0, 1'b0, 1'b0};
    7'd94 : if (EQU     ) ctl = {6'd42, 1'b0, 1'b0, 1'b0, 1'b0, 1'b0, 1'b0, 1'b0, 1'b0};
    7'd95 : if (S_EQU   ) ctl = {6'd42, 1'b0, 1'b1, 1'b0, 1'b0, 1'b0, 1'b0, 1'b0, 1'b0};
    7'd96 : if (F_LES   ) ctl = {6'd39, 1'b0, 1'b0, 1'b0, 1'b0, 1'b0, 1'b0, 1'b0, 1'b0};
    7'd97 : if (SF_LES  ) ctl = {6'd39, 1'b0, 1'b1, 1'b0, 1'b0, 1'b0, 1'b0, 1'b0, 1'b0};
    7'd98 : if (F_GRE   ) ctl = {6'd41, 1'b0, 1'b0, 1'b0, 1'b0, 1'b0, 1'b0, 1'b0, 1'b0};
    7'd99 : if (SF_GRE  ) ctl = {6'd41, 1'b0, 1'b1, 1'b0, 1'b0, 1'b0, 1'b0, 1'b0, 1'b0};
    // shifts
    7'd100: if (SHL     ) ctl = {6'd43, 1'b0, 1'b0, 1'b0, 1'b0, 1'b0, 1'b0, 1'b0, 1'b0};
    7'd101: if (S_SHL   ) ctl = {6'd43, 1'b0, 1'b1, 1'b0, 1'b0, 1'b0, 1'b0, 1'b0, 1'b0};
    7'd102: if (SHR     ) ctl = {6'd44, 1'b0, 1'b0, 1'b0, 1'b0, 1'b0, 1'b0, 1'b0, 1'b0};
    7'd103: if (S_SHR   ) ctl = {6'd44, 1'b0, 1'b1, 1'b0, 1'b0, 1'b0, 1'b0, 1'b0, 1'b0};
    7'd104: if (SRS     ) ctl = {6'd45, 1'b0, 1'b0, 1'b0, 1'b0, 1'b0, 1'b0, 1'b0, 1'b0};
    7'd105: if (S_SRS   ) ctl = {6'd45, 1'b0, 1'b1, 1'b0, 1'b0, 1'b0, 1'b0, 1'b0, 1'b0};
    default: ;
    endcase
end

assign {push, pop, mem_wr, req_in, out_en, ldi, sti, fft} = ctl[7:0];

// the ALU operation is registered; a synchronous reset to 0 = pass-acc is a
// harmless NOP for the ALU while the rest of the pipeline is being reset
always @ (posedge clk) if (rst) ula_op <= 6'd0; else ula_op <= ctl[13:8];

endmodule
