%{
    #include "../Headers/eval.h"
    #include "../Headers/messages.h"
    #include "../Headers/args.h"
%}

%option nounput noinput

INNUM  [-+]?[0-9]+
FLNUM  [-+]?(0|[1-9]+[0-9]*)\.?[0-9]*

VARIA   [a-zA-Z_][a-zA-Z0-9_]*
LABEL  @[a-zA-Z_][a-zA-Z0-9_]*
STRIN  \"(\\.|[^\\"])*\"

COMEN  "//"[^\n]*
EMPTY  [ \t]
LINHA  \n
CARES  [-+*()=#;:,\]\[{}<>~!%/&^|]

%%

"#PRNAME"  eval_direct( 1); // processor name
"#NUBITS"  eval_direct( 2); // ALU word width (bits)
"#NBMANT"  eval_direct( 3); // mantissa width (bits)
"#NBEXPO"  eval_direct( 4); // exponent width (bits)
"#NDSTAC"  ;                // must be recognized to avoid an error
"#SDEPTH"  ;                // same goes for everyone below
"#NUIOIN"  ;
"#NUIOOU"  ;
"#NUGAIN"  ;
"#FFTSIZ"  ;

"#array"   eval_direct(11); // regular array
"#arrays"  eval_direct(14); // array with initialization file

"#ITRAD"   eval_itrad (  ); // interrupt address point
"#TOAQUI"  eval_toaqui(  ); // #TOAQUI marker (PC compared against this address -> cheguei pin)

   "LOD"   eval_opcode(18); // loads data from memory
 "P_LOD"   eval_opcode(18); // PUSH + LOD
   "LDI"   eval_opcode(18); // LOD with indirect addressing (index in acc)
   "ILI"   eval_opcode(18); // LOD with reverse indirect addressing (FFT)

   "SET"   eval_opcode(18); // stores data in memory
   "SET_P" eval_opcode(18); // SET + POP
   "STI"   eval_opcode(18); // SET with indirect addressing (index on stack)
   "ISI"   eval_opcode(18); // SET with reverse indirect addressing (FFT)

   "PSH"   eval_opcode( 0); // pushes data onto the stack
   "POP"   eval_opcode( 0); // pops data from the stack

   "INN"   eval_opcode(20); // data input
 "F_INN"   eval_opcode(20); // floating-point data input
 "P_INN"   eval_opcode(20); // PUSH + INN
"PF_INN"   eval_opcode(20); // PUSH + F_INN
   "OUT"   eval_opcode(21); // data output

   "JMP"   eval_opcode(19); // unconditional jump
   "JIZ"   eval_opcode(19); // jump if acc is zero

   "CAL"   eval_opcode(19); // subroutine call
   "RET"   eval_opcode( 0); // subroutine return

   "ADD"   eval_opcode(18); // fixed-point addition with memory
 "S_ADD"   eval_opcode( 0); // fixed-point addition with stack
 "F_ADD"   eval_opcode(18); // floating-point addition with memory
"SF_ADD"   eval_opcode( 0); // floating-point addition with stack

   "MLT"   eval_opcode(18); // fixed-point multiplication with memory
 "S_MLT"   eval_opcode( 0); // fixed-point multiplication with stack
 "F_MLT"   eval_opcode(18); // floating-point multiplication with memory
"SF_MLT"   eval_opcode( 0); // floating-point multiplication with stack

   "DIV"   eval_opcode(18); // fixed-point division with memory
 "S_DIV"   eval_opcode( 0); // fixed-point division with stack
 "F_DIV"   eval_opcode(18); // floating-point division with memory
"SF_DIV"   eval_opcode( 0); // floating-point division with stack

   "MOD"   eval_opcode(18); // fixed-point remainder with memory
 "S_MOD"   eval_opcode( 0); // fixed-point remainder with stack

   "SGN"   eval_opcode(18); // fixed-point sign with memory
 "S_SGN"   eval_opcode( 0); // fixed-point sign with stack
 "F_SGN"   eval_opcode(18); // floating-point sign with memory
"SF_SGN"   eval_opcode( 0); // floating-point sign with stack

   "NEG"   eval_opcode( 0); // fixed-point negation with acc
   "NEG_M" eval_opcode(18); // fixed-point negation with memory
 "P_NEG_M" eval_opcode(18); // PUSH +   NEG_M
 "F_NEG"   eval_opcode( 0); // floating-point negation with acc
 "F_NEG_M" eval_opcode(18); // floating-point negation with memory
"PF_NEG_M" eval_opcode(18); // PUSH + F_NEG_M

   "ABS"   eval_opcode( 0); // fixed-point abs with acc
   "ABS_M" eval_opcode(18); // fixed-point abs with memory
 "P_ABS_M" eval_opcode(18); // PUSH +   ABS_M
 "F_ABS"   eval_opcode( 0); // floating-point abs with acc
 "F_ABS_M" eval_opcode(18); // floating-point abs with memory
"PF_ABS_M" eval_opcode(18); // PUSH + F_ABS_M

   "PST"   eval_opcode( 0); // clears if negative, fixed-point with acc
   "PST_M" eval_opcode(18); // clears if negative, fixed-point with memory
 "P_PST_M" eval_opcode(18); // PUSH +   PST_M
 "F_PST"   eval_opcode( 0); // clears if negative, floating-point with acc
 "F_PST_M" eval_opcode(18); // clears if negative, floating-point with memory
"PF_PST_M" eval_opcode(18); // PUSH + F_PST_M

   "NRM"   eval_opcode( 0); // divides by constant, fixed-point with acc
   "NRM_M" eval_opcode(18); // divides by constant, fixed-point with memory
 "P_NRM_M" eval_opcode(18); // PUSH +   NRM_M

   "I2F"   eval_opcode( 0); // int2float with acc
   "I2F_M" eval_opcode(18); // int2float with mem
 "P_I2F_M" eval_opcode(18); // PUSH + I2F_M

   "F2I"   eval_opcode( 0); // float2int with acc
   "F2I_M" eval_opcode(18); // float2int with mem
 "P_F2I_M" eval_opcode(18); // PUSH + F2I_M

   "AND"   eval_opcode(18); // and with memory
 "S_AND"   eval_opcode( 0); // and with stack
   "ORR"   eval_opcode(18); // or  with memory
 "S_ORR"   eval_opcode( 0); // or  with stack
   "XOR"   eval_opcode(18); // xor with memory
 "S_XOR"   eval_opcode( 0); // xor with stack

   "INV"   eval_opcode( 0); // not with acc
   "INV_M" eval_opcode(18); // not with mem
 "P_INV_M" eval_opcode(18); // PUSH + INV_M

   "LAN"   eval_opcode(18); // logical and with memory
 "S_LAN"   eval_opcode( 0); // logical and with stack
   "LOR"   eval_opcode(18); // logical or  with memory
 "S_LOR"   eval_opcode( 0); // logical or  with stack

   "LIN"   eval_opcode( 0); // logical inversion with acc
   "LIN_M" eval_opcode(18); // logical inversion with mem
 "P_LIN_M" eval_opcode(18); // PUSH + LIN_M

   "LES"   eval_opcode(18); // less than, fixed-point with memory
 "S_LES"   eval_opcode( 0); // less than, fixed-point with stack
 "F_LES"   eval_opcode(18); // less than, floating-point with memory
"SF_LES"   eval_opcode( 0); // less than, floating-point with stack

   "GRE"   eval_opcode(18); // greater than, fixed-point with memory
 "S_GRE"   eval_opcode( 0); // greater than, fixed-point with stack
 "F_GRE"   eval_opcode(18); // greater than, floating-point with memory
"SF_GRE"   eval_opcode( 0); // greater than, floating-point with stack

   "EQU"   eval_opcode(18); // equal to, with memory
 "S_EQU"   eval_opcode( 0); // equal to, with stack

   "SHL"   eval_opcode(18); // shift left  with memory
 "S_SHL"   eval_opcode( 0); // shift left  with stack
   "SHR"   eval_opcode(18); // shift right with memory
 "S_SHR"   eval_opcode( 0); // shift right with stack
   "SRS"   eval_opcode(18); // arithmetic shift right with memory
 "S_SRS"   eval_opcode( 0); // arithmetic shift right with stack

   /* special instructions */

   "NOP"   eval_opcode( 0); // no operation
 "F_ROT"   eval_opcode( 0); // nearest power-of-two square-root approximation (with ACC)
 "F_SU1"   eval_opcode(18); // floating-point subtraction with memory at input 1
 "F_SU2"   eval_opcode(18); // floating-point subtraction with memory at input 2
"SF_SU1"   eval_opcode( 0); // floating-point subtraction with stack at input 1
"SF_SU2"   eval_opcode( 0); // floating-point subtraction with stack at input 2

   /* scale-by-2^k and exponent-extract */

 "F_SCL"   eval_opcode(18); // scale float by 2^k, k from memory
"SF_SCL"   eval_opcode( 0); // scale float by 2^k, k from stack
   "XPO"   eval_opcode( 0); // base-2 exponent of float (acc) as int
 "XPO_M"   eval_opcode(18); // base-2 exponent of float (memory) as int

   /* base-less indirect addressing (runtime pointers / array params) */

   "LDA"   eval_opcode( 0); // acc = mem[acc]
   "STA"   eval_opcode( 0); // mem[stack_top] = acc, pop

   /* pseudo instructions */
   /* avoids indirect addressing */

   "LEA"   eval_opcode(24); // load-effective-address; counts as 1 var (the target) + 1 synthetic constant (the materialised address)

   "LOD_V" eval_opcode(22); //   LOD with constant offset
 "P_LOD_V" eval_opcode(22); // P_LOD with constant offset
   "SET_V" eval_opcode(22); //   SET with constant offset
   "ADD_V" eval_opcode(22); //   ADD with constant offset
 "F_ADD_V" eval_opcode(22); // F_ADD with constant offset
   "MLT_V" eval_opcode(22); //   MLT with constant offset
 "F_MLT_V" eval_opcode(22); // F_MLT with constant offset

{VARIA}    eval_opernd(yytext); // used on ALU instructions with variables (IDs)
{INNUM}    eval_opernd(yytext); // used on ALU instructions with an int constant
{FLNUM}    eval_opernd(yytext); // used on ALU instructions with a float constant

{STRIN}    eval_opernd(yytext); // used on instructions with strings (e.g. array initialization file)
{LABEL}    eval_label (yytext); // used on jump instructions

{COMEN}
{EMPTY}
{LINHA}
{CARES}

.;
%%

int main(int argc, char *argv[])
{
    parse_lang_flag(&argc, argv);   // processes -en/-pt flag (removes it from argv)

    cli_args a;
    cli_parse(argc, argv, &a);      // parses the named options (or exits with usage)

    yyin = fopen(a.input, "r"); // -i is the input .asm file
    if (yyin == NULL) { fprintf(stderr, MSG_ERR_CANT_OPEN_FILE, a.input); exit(EXIT_FAILURE); }
    eval_init   (a.temp_dir);   // -t is the directory holding the log file
    yylex();                    // run the lexer
    fclose(yyin);               // close the .asm file
    eval_finish();              // finalize the log file

    printf(MSG_OK_APP_DONE);

    return 0;
}

int yywrap(void) {return 1;}
