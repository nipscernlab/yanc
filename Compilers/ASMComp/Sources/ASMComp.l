%{

#include "../Headers/eval.h"
#include "../Headers/hdl.h"
#include "../Headers/messages.h"
#include "../Headers/args.h"

#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <sys/stat.h>

/* Create a directory if it doesn't already exist. asmcomp writes its
 * Verilog/MIF outputs into <proc_dir>/Hardware and <proc_dir>/Simulation;
 * without these folders the subsequent fopen()s would crash with a NULL
 * FILE* deref. Silently tolerate "already exists" so reruns don't fail. */
static void ensure_dir(const char *path) {
#ifdef _WIN32
    /* mingw's <sys/stat.h> provides mkdir as a one-arg macro forwarding
     * to _mkdir; the mode is ignored on Windows. */
    if (mkdir(path) != 0 && errno != EEXIST) {
        fprintf(stderr, "asmcomp: warning: could not create %s (errno=%d)\n",
                path, errno);
    }
#else
    if (mkdir(path, 0755) != 0 && errno != EEXIST) {
        fprintf(stderr, "asmcomp: warning: could not create %s (errno=%d)\n",
                path, errno);
    }
#endif
}

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

"#PRNAME"  ;                                        // must be recognized to avoid an error
"#NUBITS"  ;                                        // same goes for everyone below
"#NBMANT"  ;
"#NBEXPO"  ;
"#NDSTAC"  eval_direct( 5);                         // data stack depth
"#SDEPTH"  eval_direct( 6);                         // subroutine stack depth
"#NUIOIN"  eval_direct( 7);                         // number of input ports
"#NUIOOU"  eval_direct( 8);                         // number of output ports
"#NUGAIN"  eval_direct( 9);                         // division constant for the norm() function
"#FFTSIZ"  eval_direct(10);                         // FFT size (bits)

"#array"   eval_direct(11);                         // regular array
"#arrays"  eval_direct(14);                         // array with initialization file

"#ITRAD"                  ;                         // interrupt address point
"#TOAQUI"                 ;                         // #TOAQUI marker (consumed by appcomp; ignored here)

   "LOD"   eval_opcode(  0,18, yytext,    "LOD"  ); // loads data from memory
 "P_LOD"   eval_opcode(  1,18, yytext,  "P_LOD"  ); // PUSH + LOD
   "LDI"   eval_opcode(  2,18, yytext,    "LDI"  ); // LOD with indirect addressing (index in acc)
   "ILI"   eval_opcode(  3,18, yytext,    "ILI"  ); // LOD with reverse indirect addressing (FFT)

   "SET"   eval_opcode(  4,18, yytext,    "SET"  ); // stores data in memory
   "SET_P" eval_opcode(  5,18, yytext,    "SET_P"); // SET + POP
   "STI"   eval_opcode(  6,18, yytext,    "STI"  ); // SET with indirect addressing (index on stack)
   "ISI"   eval_opcode(  7,18, yytext,    "ISI"  ); // SET with reverse indirect addressing (FFT)

   "PSH"   eval_opcode(  8, 0, yytext,    "PSH"  ); // pushes data onto the stack
   "POP"   eval_opcode(  9, 0, yytext,    "POP"  ); // pops data from the stack

   "INN"   eval_opcode( 10,20, yytext,    "INN"  ); // data input
 "F_INN"   eval_opcode( 11,20, yytext,  "F_INN"  ); // floating-point data input
 "P_INN"   eval_opcode( 12,20, yytext,  "P_INN"  ); // PUSH +   INN
"PF_INN"   eval_opcode( 13,20, yytext, "PF_INN"  ); // PUSH + F_INN
   "OUT"   eval_opcode( 14,21, yytext,    "OUT"  ); // data output

   "JMP"   eval_opcode( 15,19, yytext,    ""     ); // unconditional jump
   "JIZ"   eval_opcode( 16,19, yytext,    "JIZ"  ); // jump if acc is zero

   "CAL"   eval_opcode( 17,19, yytext,    "CAL"  ); // subroutine call
   "RET"   eval_opcode( 18, 0, yytext,    ""     ); // subroutine return

   "ADD"   eval_opcode( 19,18, yytext,    "ADD"  ); // fixed-point addition with memory
 "S_ADD"   eval_opcode( 20, 0, yytext,  "S_ADD"  ); // fixed-point addition with stack
 "F_ADD"   eval_opcode( 21,18, yytext,  "F_ADD"  ); // floating-point addition with memory
"SF_ADD"   eval_opcode( 22, 0, yytext, "SF_ADD"  ); // floating-point addition with stack

   "MLT"   eval_opcode( 23,18, yytext,    "MLT"  ); // fixed-point multiplication with memory
 "S_MLT"   eval_opcode( 24, 0, yytext,  "S_MLT"  ); // fixed-point multiplication with stack
 "F_MLT"   eval_opcode( 25,18, yytext,  "F_MLT"  ); // floating-point multiplication with memory
"SF_MLT"   eval_opcode( 26, 0, yytext, "SF_MLT"  ); // floating-point multiplication with stack

   "DIV"   eval_opcode( 27,18, yytext,    "DIV"  ); // fixed-point division with memory
 "S_DIV"   eval_opcode( 28, 0, yytext,  "S_DIV"  ); // fixed-point division with stack
 "F_DIV"   eval_opcode( 29,18, yytext,  "F_DIV"  ); // floating-point division with memory
"SF_DIV"   eval_opcode( 30, 0, yytext, "SF_DIV"  ); // floating-point division with stack

   "MOD"   eval_opcode( 31,18, yytext,    "MOD"  ); // fixed-point remainder with memory
 "S_MOD"   eval_opcode( 32, 0, yytext,  "S_MOD"  ); // fixed-point remainder with stack

   "SGN"   eval_opcode( 33,18, yytext,    "SGN"  ); // fixed-point sign with memory
 "S_SGN"   eval_opcode( 34, 0, yytext,  "S_SGN"  ); // fixed-point sign with stack
 "F_SGN"   eval_opcode( 35,18, yytext,  "F_SGN"  ); // floating-point sign with memory
"SF_SGN"   eval_opcode( 36, 0, yytext, "SF_SGN"  ); // floating-point sign with stack

   "NEG"   eval_opcode( 37, 0, yytext,    "NEG"  ); // fixed-point negation with acc
   "NEG_M" eval_opcode( 38,18, yytext,    "NEG_M"); // fixed-point negation with memory
 "P_NEG_M" eval_opcode( 39,18, yytext,  "P_NEG_M"); // PUSH +   NEG_M
 "F_NEG"   eval_opcode( 40, 0, yytext,  "F_NEG"  ); // floating-point negation with acc
 "F_NEG_M" eval_opcode( 41,18, yytext,  "F_NEG_M"); // floating-point negation with memory
"PF_NEG_M" eval_opcode( 42,18, yytext, "PF_NEG_M"); // PUSH + F_NEG_M

   "ABS"   eval_opcode( 43, 0, yytext,    "ABS"  ); // fixed-point abs with acc
   "ABS_M" eval_opcode( 44,18, yytext,    "ABS_M"); // fixed-point abs with memory
 "P_ABS_M" eval_opcode( 45,18, yytext,  "P_ABS_M"); // PUSH +   ABS_M
 "F_ABS"   eval_opcode( 46, 0, yytext,  "F_ABS"  ); // floating-point abs with acc
 "F_ABS_M" eval_opcode( 47,18, yytext,  "F_ABS_M"); // floating-point abs with memory
"PF_ABS_M" eval_opcode( 48,18, yytext, "PF_ABS_M"); // PUSH + F_ABS_M

   "PST"   eval_opcode( 49, 0, yytext,    "PST"  ); // clears if negative, fixed-point with acc
   "PST_M" eval_opcode( 50,18, yytext,    "PST_M"); // clears if negative, fixed-point with memory
 "P_PST_M" eval_opcode( 51,18, yytext,  "P_PST_M"); // PUSH +   PST_M
 "F_PST"   eval_opcode( 52, 0, yytext,  "F_PST"  ); // clears if negative, floating-point with acc
 "F_PST_M" eval_opcode( 53,18, yytext,  "F_PST_M"); // clears if negative, floating-point with memory
"PF_PST_M" eval_opcode( 54,18, yytext, "PF_PST_M"); // PUSH + F_PST_M

   "NRM"   eval_opcode( 55, 0, yytext,    "NRM"  ); // divides by constant, fixed-point with acc
   "NRM_M" eval_opcode( 56,18, yytext,    "NRM_M"); // divides by constant, fixed-point with memory
 "P_NRM_M" eval_opcode( 57,18, yytext,  "P_NRM_M"); // PUSH +   NRM_M

   "I2F"   eval_opcode( 58, 0, yytext,    "I2F"  ); // int2float with acc
   "I2F_M" eval_opcode( 59,18, yytext,    "I2F_M"); // int2float with mem
 "P_I2F_M" eval_opcode( 60,18, yytext,  "P_I2F_M"); // PUSH + I2F_M

   "F2I"   eval_opcode( 61, 0, yytext,    "F2I"  ); // float2int with acc
   "F2I_M" eval_opcode( 62,18, yytext,    "F2I_M"); // float2int with mem
 "P_F2I_M" eval_opcode( 63,18, yytext,  "P_F2I_M"); // PUSH + F2I_M

   "AND"   eval_opcode( 64,18, yytext,    "AND"  ); // and with memory
 "S_AND"   eval_opcode( 65, 0, yytext,  "S_AND"  ); // and with stack
   "ORR"   eval_opcode( 66,18, yytext,    "ORR"  ); // or  with memory
 "S_ORR"   eval_opcode( 67, 0, yytext,  "S_ORR"  ); // or  with stack
   "XOR"   eval_opcode( 68,18, yytext,    "XOR"  ); // xor with memory
 "S_XOR"   eval_opcode( 69, 0, yytext,  "S_XOR"  ); // xor with stack

   "INV"   eval_opcode( 70, 0, yytext,    "INV"  ); // not with acc
   "INV_M" eval_opcode( 71,18, yytext,    "INV_M"); // not with mem
 "P_INV_M" eval_opcode( 72,18, yytext,  "P_INV_M"); // PUSH + INV_M

   "LAN"   eval_opcode( 73,18, yytext,    "LAN"  ); // logical and with memory
 "S_LAN"   eval_opcode( 74, 0, yytext,  "S_LAN"  ); // logical and with stack
   "LOR"   eval_opcode( 75,18, yytext,    "LOR"  ); // logical or  with memory
 "S_LOR"   eval_opcode( 76, 0, yytext,  "S_LOR"  ); // logical or  with stack

   "LIN"   eval_opcode( 77, 0, yytext,    "LIN"  ); // logical inversion with acc
   "LIN_M" eval_opcode( 78,18, yytext,    "LIN_M"); // logical inversion with mem
 "P_LIN_M" eval_opcode( 79,18, yytext,  "P_LIN_M"); // PUSH + LIN_M

   "LES"   eval_opcode( 80,18, yytext,    "LES"  ); // less than, fixed-point with memory
 "S_LES"   eval_opcode( 81, 0, yytext,  "S_LES"  ); // less than, fixed-point with stack
 "F_LES"   eval_opcode( 82,18, yytext,  "F_LES"  ); // less than, floating-point with memory
"SF_LES"   eval_opcode( 83, 0, yytext, "SF_LES"  ); // less than, floating-point with stack

   "GRE"   eval_opcode( 84,18, yytext,    "GRE"  ); // greater than, fixed-point with memory
 "S_GRE"   eval_opcode( 85, 0, yytext,  "S_GRE"  ); // greater than, fixed-point with stack
 "F_GRE"   eval_opcode( 86,18, yytext,  "F_GRE"  ); // greater than, floating-point with memory
"SF_GRE"   eval_opcode( 87, 0, yytext, "SF_GRE"  ); // greater than, floating-point with stack

   "EQU"   eval_opcode( 88,18, yytext,    "EQU"  ); // equal to, with memory
 "S_EQU"   eval_opcode( 89, 0, yytext,  "S_EQU"  ); // equal to, with stack

   "SHL"   eval_opcode( 90,18, yytext,    "SHL"  ); // shift left  with memory
 "S_SHL"   eval_opcode( 91, 0, yytext,  "S_SHL"  ); // shift left  with stack
   "SHR"   eval_opcode( 92,18, yytext,    "SHR"  ); // shift right with memory
 "S_SHR"   eval_opcode( 93, 0, yytext,  "S_SHR"  ); // shift right with stack
   "SRS"   eval_opcode( 94,18, yytext,    "SRS"  ); // arithmetic shift right with memory
 "S_SRS"   eval_opcode( 95, 0, yytext,  "S_SRS"  ); // arithmetic shift right with stack

   /* special instructions */

   "NOP"   eval_opcode( 96, 0, yytext,    ""     ); // no operation
 "F_ROT"   eval_opcode( 97, 0, yytext,  "F_ROT"  ); // nearest power-of-two square-root approximation (with ACC)
 "F_SU1"   eval_opcode( 98,18, yytext,  "F_SU1"  ); // floating-point subtraction with memory at input 1
 "F_SU2"   eval_opcode( 99,18, yytext,  "F_SU2"  ); // floating-point subtraction with memory at input 2
"SF_SU1"   eval_opcode(100, 0, yytext, "SF_SU1"  ); // floating-point subtraction with stack at input 1
"SF_SU2"   eval_opcode(101, 0, yytext, "SF_SU2"  ); // floating-point subtraction with stack at input 2

   /* scale-by-2^k and exponent-extract */

 "F_SCL"   eval_opcode(104,18, yytext,  "F_SCL"  ); // scale float by 2^k, k from memory
"SF_SCL"   eval_opcode(105, 0, yytext, "SF_SCL"  ); // scale float by 2^k, k from stack
   "XPO"   eval_opcode(106, 0, yytext,    "XPO"  ); // base-2 exponent of float (acc) as int
 "XPO_M"   eval_opcode(107,18, yytext,  "XPO_M"  ); // base-2 exponent of float (memory) as int

   /* base-less indirect addressing (runtime pointers / array params) */

   "LDA"   eval_opcode(102, 0, yytext,    "LDA"  ); // acc = mem[acc]
   "STA"   eval_opcode(103, 0, yytext,    "STA"  ); // mem[stack_top] = acc, pop

   /* pseudo instructions */
   /* avoid indirect addressing */

   "LEA"   eval_opcode(  0,24, yytext,    "LOD"  ); // load-effective-address: acc = &var (pseudo, expands to LOD <const>)

   "LOD_V" eval_opcode( 0,22, yytext,    "LOD"  ); //   LOD with constant offset
 "P_LOD_V" eval_opcode( 1,22, yytext,  "P_LOD"  ); // P_LOD with constant offset
   "SET_V" eval_opcode( 4,22, yytext,    "SET"  ); //   SET with constant offset
   "ADD_V" eval_opcode(19,22, yytext,    "ADD"  ); //   ADD with constant offset
 "F_ADD_V" eval_opcode(21,22, yytext,  "F_ADD"  ); // F_ADD with constant offset
   "MLT_V" eval_opcode(23,22, yytext,    "MLT"  ); //   MLT with constant offset
 "F_MLT_V" eval_opcode(25,22, yytext,  "F_MLT"  ); // F_MLT with constant offset

{VARIA}    eval_opernd(yytext,0); // used on ALU instructions with variables (IDs)
{INNUM}    eval_opernd(yytext,1); // used on ALU instructions with an int constant
{FLNUM}    eval_opernd(yytext,2); // used on ALU instructions with a float constant

{STRIN}    eval_opernd(yytext,0); // used on instructions with strings (e.g. array initialization file)
{LABEL}                         ; // used on jump instructions

{COMEN}
{EMPTY}
{LINHA}
{CARES}

.;
%%

int main(int argc, char *argv[])
{
  parse_lang_flag(&argc, argv);              // processes -en/-pt flag (removes it from argv)

  cli_args a;
  cli_parse(argc, argv, &a);                 // parses the named options (or exits with usage)

  strcpy(proc_dir, a.proc_dir);              // processor folder path
  strcpy( hdl_dir, a.hdl_dir);               // HDL folder path
  strcpy( mac_dir, a.macros_dir);            // Macros folder path
  strcpy(temp_dir, a.temp_dir);              // Temp folder path

  /* Make sure the output subfolders exist before any fopen() targets them. */
  char outsub[2300];
  snprintf(outsub, sizeof(outsub), "%s/Hardware",   proc_dir); ensure_dir(outsub);
  snprintf(outsub, sizeof(outsub), "%s/Simulation", proc_dir); ensure_dir(outsub);

  yyin = fopen(a.input, "r");                // -i is the input .asm file
  if (yyin == NULL) { fprintf(stderr, MSG_ERR_CANT_OPEN_FILE, a.input); exit(EXIT_FAILURE); }
  eval_init(a.freq, a.clocks);               // initialize the state variables
  yylex();                                   // run the lexer
  fclose(yyin);                              // close the .asm file
  eval_finish();                             // finalize the assembler

  printf(MSG_OK_ASM_DONE);

  return 0;
}

int yywrap(void) {return 1;}
