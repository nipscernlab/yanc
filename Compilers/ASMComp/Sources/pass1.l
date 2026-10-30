%{
    // asmcomp's first pass (it was the appcomp program): reads the .asm once
    // and writes app_log.txt, the directives, the address of every label,
    // the instruction and data-word counts, so the second pass (ASMComp.l)
    // resolves forward jumps and sizes the memories. Its own scanner,
    // prefixed `app` so it does not clash with the second pass's yylex.
    #include "../Headers/pass1.h"
    #include "../Headers/messages.h"
%}

%option nounput noinput noyywrap prefix="app"

INNUM  [-+]?[0-9]+
FLNUM  [-+]?(0|[1-9]+[0-9]*)\.?[0-9]*([eE][-+]?[0-9]+)?

VARIA   [a-zA-Z_][a-zA-Z0-9_]*
LABEL  @[a-zA-Z_][a-zA-Z0-9_]*
STRIN  \"(\\.|[^\\"])*\"

COMEN  "//"[^\n]*
EMPTY  [ \t]
LINHA  \n
CARES  [-+*()=#;:,\]\[{}<>~!%/&^|]

%%

"#PRNAME"  p1_direct( 1); // processor name
"#NUBITS"  p1_direct( 2); // ALU word width (bits)
"#NBMANT"  p1_direct( 3); // mantissa width (bits)
"#NBEXPO"  p1_direct( 4); // exponent width (bits)
"#NDSTAC"  ;                // must be recognized to avoid an error
"#SDEPTH"  ;                // same goes for everyone below
"#NUIOIN"  p1_direct(29); // number of input ports: the operand must hold the highest
"#NUIOOU"  p1_direct(29); // number of output ports: idem
"#NUGAIN"  ;
"#FFTSIZ"  ;
"#LKSYNC"  ;                // #LKSYNC k: read by asmcomp (link k on the processor clock)
"#FROUND"  p1_direct(25); // float rounding level (asmcomp needs it before encoding constants)

"#array"   p1_direct(11); // regular array
"#arrays"  p1_direct(14); // array with initialization file
"#arrayd"  p1_direct(31); // array with its initial values on the line

"#ITRAD"   p1_itrad (  ); // interrupt address point
"#TOAQUI"  p1_toaqui(  ); // #TOAQUI marker (PC compared against this address -> cheguei pin)
"#SHARE"   p1_direct(26); // #SHARE <name> <home>: <name> uses the data word of <home>

   "LOD"   p1_opcode(18); // loads data from memory
 "P_LOD"   p1_opcode(18); // PUSH + LOD
   "LDI"   p1_opcode(28); // LOD with indirect addressing (index in acc)
   "ILI"   p1_opcode(28); // LOD with reverse indirect addressing (FFT)

   "SET"   p1_opcode(18); // stores data in memory
   "SET_P" p1_opcode(18); // SET + POP
   "STI"   p1_opcode(28); // SET with indirect addressing (index on stack)
   "ISI"   p1_opcode(28); // SET with reverse indirect addressing (FFT)

   "PSH"   p1_opcode( 0); // pushes data onto the stack
   "POP"   p1_opcode( 0); // pops data from the stack

   "INN"   p1_opcode(20); // data input
 "F_INN"   p1_opcode(20); // floating-point data input
 "P_INN"   p1_opcode(20); // PUSH + INN
"PF_INN"   p1_opcode(20); // PUSH + F_INN
   "OUT"   p1_opcode(21); // data output

   "JMP"   p1_opcode(19); // unconditional jump
   "JIZ"   p1_opcode(19); // jump if acc is zero

   "CAL"   p1_opcode(19); // subroutine call
   "RET"   p1_opcode( 0); // subroutine return
   "TOM"   p1_opcode(20); // toma: offer acc on link <operand> (a number), wait until taken
   "CAD"   p1_opcode(20); // cade: wait until link <operand> (a number) offers, then take it

   "ADD"   p1_opcode(18); // fixed-point addition with memory
 "S_ADD"   p1_opcode( 0); // fixed-point addition with stack
 "F_ADD"   p1_opcode(18); // floating-point addition with memory
"SF_ADD"   p1_opcode( 0); // floating-point addition with stack

   "MLT"   p1_opcode(18); // fixed-point multiplication with memory
 "S_MLT"   p1_opcode( 0); // fixed-point multiplication with stack
 "F_MLT"   p1_opcode(18); // floating-point multiplication with memory
"SF_MLT"   p1_opcode( 0); // floating-point multiplication with stack

   "DIV"   p1_opcode(18); // fixed-point division with memory
 "S_DIV"   p1_opcode( 0); // fixed-point division with stack
 "F_DIV"   p1_opcode(18); // floating-point division with memory
"SF_DIV"   p1_opcode( 0); // floating-point division with stack

   "MOD"   p1_opcode(18); // fixed-point remainder with memory
 "S_MOD"   p1_opcode( 0); // fixed-point remainder with stack

   "SGN"   p1_opcode(18); // fixed-point sign with memory
 "S_SGN"   p1_opcode( 0); // fixed-point sign with stack
 "F_SGN"   p1_opcode(18); // floating-point sign with memory
"SF_SGN"   p1_opcode( 0); // floating-point sign with stack

   "NEG"   p1_opcode( 0); // fixed-point negation with acc
   "NEG_M" p1_opcode(18); // fixed-point negation with memory
 "P_NEG_M" p1_opcode(18); // PUSH +   NEG_M
 "F_NEG"   p1_opcode( 0); // floating-point negation with acc
 "F_NEG_M" p1_opcode(18); // floating-point negation with memory
"PF_NEG_M" p1_opcode(18); // PUSH + F_NEG_M

   "ABS"   p1_opcode( 0); // fixed-point abs with acc
   "ABS_M" p1_opcode(18); // fixed-point abs with memory
 "P_ABS_M" p1_opcode(18); // PUSH +   ABS_M
 "F_ABS"   p1_opcode( 0); // floating-point abs with acc
 "F_ABS_M" p1_opcode(18); // floating-point abs with memory
"PF_ABS_M" p1_opcode(18); // PUSH + F_ABS_M

   "PST"   p1_opcode( 0); // clears if negative, fixed-point with acc
   "PST_M" p1_opcode(18); // clears if negative, fixed-point with memory
 "P_PST_M" p1_opcode(18); // PUSH +   PST_M
 "F_PST"   p1_opcode( 0); // clears if negative, floating-point with acc
 "F_PST_M" p1_opcode(18); // clears if negative, floating-point with memory
"PF_PST_M" p1_opcode(18); // PUSH + F_PST_M

   "NRM"   p1_opcode( 0); // divides by constant, fixed-point with acc
   "NRM_M" p1_opcode(18); // divides by constant, fixed-point with memory
 "P_NRM_M" p1_opcode(18); // PUSH +   NRM_M

   "I2F"   p1_opcode( 0); // int2float with acc
   "I2F_M" p1_opcode(18); // int2float with mem
 "P_I2F_M" p1_opcode(18); // PUSH + I2F_M

   "F2I"   p1_opcode( 0); // float2int with acc
   "F2I_M" p1_opcode(18); // float2int with mem
 "P_F2I_M" p1_opcode(18); // PUSH + F2I_M

   "AND"   p1_opcode(18); // and with memory
 "S_AND"   p1_opcode( 0); // and with stack
   "ORR"   p1_opcode(18); // or  with memory
 "S_ORR"   p1_opcode( 0); // or  with stack
   "XOR"   p1_opcode(18); // xor with memory
 "S_XOR"   p1_opcode( 0); // xor with stack

   "INV"   p1_opcode( 0); // not with acc
   "INV_M" p1_opcode(18); // not with mem
 "P_INV_M" p1_opcode(18); // PUSH + INV_M

   "LAN"   p1_opcode(18); // logical and with memory
 "S_LAN"   p1_opcode( 0); // logical and with stack
   "LOR"   p1_opcode(18); // logical or  with memory
 "S_LOR"   p1_opcode( 0); // logical or  with stack

   "LIN"   p1_opcode( 0); // logical inversion with acc
   "LIN_M" p1_opcode(18); // logical inversion with mem
 "P_LIN_M" p1_opcode(18); // PUSH + LIN_M

   "LES"   p1_opcode(18); // less than, fixed-point with memory
 "S_LES"   p1_opcode( 0); // less than, fixed-point with stack
 "F_LES"   p1_opcode(18); // less than, floating-point with memory
"SF_LES"   p1_opcode( 0); // less than, floating-point with stack

   "GRE"   p1_opcode(18); // greater than, fixed-point with memory
 "S_GRE"   p1_opcode( 0); // greater than, fixed-point with stack
 "F_GRE"   p1_opcode(18); // greater than, floating-point with memory
"SF_GRE"   p1_opcode( 0); // greater than, floating-point with stack

   "EQU"   p1_opcode(18); // equal to, with memory
 "S_EQU"   p1_opcode( 0); // equal to, with stack

   "SHL"   p1_opcode(18); // shift left  with memory
 "S_SHL"   p1_opcode( 0); // shift left  with stack
   "SHR"   p1_opcode(18); // shift right with memory
 "S_SHR"   p1_opcode( 0); // shift right with stack
   "SRS"   p1_opcode(18); // arithmetic shift right with memory
 "S_SRS"   p1_opcode( 0); // arithmetic shift right with stack

   /* special instructions */

   "NOP"   p1_opcode( 0); // no operation
 "F_ROT"   p1_opcode( 0); // nearest power-of-two square-root approximation (with ACC)
 "F_SU1"   p1_opcode(18); // floating-point subtraction with memory at input 1
 "F_SU2"   p1_opcode(18); // floating-point subtraction with memory at input 2
"SF_SU1"   p1_opcode( 0); // floating-point subtraction with stack at input 1
"SF_SU2"   p1_opcode( 0); // floating-point subtraction with stack at input 2

   /* scale-by-2^k and exponent-extract */

 "F_SCL"   p1_opcode(18); // scale float by 2^k, k from memory
"SF_SCL"   p1_opcode( 0); // scale float by 2^k, k from stack
   "XPO"   p1_opcode( 0); // base-2 exponent of float (acc) as int
 "XPO_M"   p1_opcode(18); // base-2 exponent of float (memory) as int

   /* pseudo instructions */
   /* avoids indirect addressing */

   "LEA"   p1_opcode(24); // load-effective-address; counts as 1 var (the target) + 1 synthetic constant (the materialised address)

   /* the read of a division's result: the third word of `<div> x; NOP; <read>` */

   "QUO"   p1_opcode( 0); // integer quotient of the division four words before
   "REM"   p1_opcode( 0); // remainder of the division four words before
 "F_QUO"   p1_opcode( 0); // float quotient of the division three words before

   "LOD_V" p1_opcode(22); //   LOD with constant offset
 "P_LOD_V" p1_opcode(22); // P_LOD with constant offset
   "SET_V" p1_opcode(22); //   SET with constant offset
   "ADD_V" p1_opcode(22); //   ADD with constant offset
 "F_ADD_V" p1_opcode(22); // F_ADD with constant offset
   "MLT_V" p1_opcode(22); //   MLT with constant offset
 "F_MLT_V" p1_opcode(22); // F_MLT with constant offset
   "LDI_V" p1_opcode(22); //   LDI with constant offset
   "STI_V" p1_opcode(22); //   STI with constant offset

{VARIA}    p1_opernd(yytext); // used on ALU instructions with variables (IDs)
{INNUM}    p1_opernd(yytext); // used on ALU instructions with an int constant
{FLNUM}    p1_opernd(yytext); // used on ALU instructions with a float constant

{STRIN}    p1_opernd(yytext); // used on instructions with strings (e.g. array initialization file)
{LABEL}    p1_label (yytext); // used on jump instructions

{COMEN}
{EMPTY}
{LINHA}
{CARES}

.;
%%

void pass1_run(const char *asm_path, const char *dir, const char *name)
{
    yyin = fopen(asm_path, "r");
    if (yyin == NULL) { fprintf(stderr, MSG_ERR_CANT_OPEN_FILE, asm_path); exit(EXIT_FAILURE); }
    p1_init(dir, name);         // app_log.txt goes to the folder of the .asm
    yylex();
    fclose(yyin);
    p1_finish();
}
