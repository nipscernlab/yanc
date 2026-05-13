%{

#include "..\Headers\eval.h"
#include "..\Headers\hdl.h"
#include "..\Headers\messages.h"

#include <string.h>

%}

INNUM  [-+]?[0-9]+
FLNUM  [-+]?(0|[1-9]+[0-9]*)\.?[0-9]*

VARIA   [a-zA-Z][a-zA-Z0-9_]*
LABEL  @[a-zA-Z][a-zA-Z0-9_]*
STRIN  \"(\\.|[^\\"])*\"

COMEN  "//"[^\n]*
EMPTY  [ \t]
LINHA  \n
CARES  [-+*()=#;:,\]\[{}<>~!%/&^|]

%%

"#PRNAME"  ;                                        // precisa ser identificado pra nao dar erro
"#NUBITS"  ;                                        // mesma coisa pra todos ai embaixo
"#NBMANT"  ;
"#NBEXPO"  ;
"#NDSTAC"  eval_direct( 5);                         // tamanho da pilha de dados
"#SDEPTH"  eval_direct( 6);                         // tamanho da pilha de subrotinas
"#NUIOIN"  eval_direct( 7);                         // quantidade de portas de entrada
"#NUIOOU"  eval_direct( 8);                         // quantidade de portas de saida
"#NUGAIN"  eval_direct( 9);                         // constante de divisao da funcao norm()
"#FFTSIZ"  eval_direct(10);                         // tamanho da fft (em bits)

"#array"   eval_direct(11);                         // array normal
"#arrays"  eval_direct(14);                         // array com arquivo de inicializacao

"#ITRAD"                  ;                         // ponto de endereco da interrupcao

   "LOD"   eval_opcode(  0,18, yytext,    "LOD"  ); // carrega dado da memoria
 "P_LOD"   eval_opcode(  1,18, yytext,  "P_LOD"  ); // PUSH + LOD
   "LDI"   eval_opcode(  2,18, yytext,    "LDI"  ); // LOD com enderecamento indireto (indice no acc)
   "ILI"   eval_opcode(  3,18, yytext,    "ILI"  ); // LOD com enderecamento indireto invertido (FFT)

   "SET"   eval_opcode(  4,18, yytext,    "SET"  ); // salva dado na memoria
   "SET_P" eval_opcode(  5,18, yytext,    "SET_P"); // SET + POP
   "STI"   eval_opcode(  6,18, yytext,    "STI"  ); // SET com enderecamento indireto (indice na pilha)
   "ISI"   eval_opcode(  7,18, yytext,    "ISI"  ); // SET com enderecamento indireto invertido (FFT)

   "PSH"   eval_opcode(  8, 0, yytext,    "PSH"  ); // xuxa dado na pilha
   "POP"   eval_opcode(  9, 0, yytext,    "POP"  ); // retira dado da pilha

   "INN"   eval_opcode( 10,20, yytext,    "INN"  ); // entrada de dados
 "F_INN"   eval_opcode( 11,20, yytext,  "F_INN"  ); // entrada de dados em ponto flutuante
 "P_INN"   eval_opcode( 12,20, yytext,  "P_INN"  ); // PUSH +   INN
"PF_INN"   eval_opcode( 13,20, yytext, "PF_INN"  ); // PUSH + F_INN
   "OUT"   eval_opcode( 14,21, yytext,    "OUT"  ); // saida   de dados

   "JMP"   eval_opcode( 15,19, yytext,    ""     ); // salto incondicional
   "JIZ"   eval_opcode( 16,19, yytext,    "JIZ"  ); // salta se acc for zero

   "CAL"   eval_opcode( 17,19, yytext,    "CAL"  ); // chamada de subrotina
   "RET"   eval_opcode( 18, 0, yytext,    ""     ); // retorno de subrotina

   "ADD"   eval_opcode( 19,18, yytext,    "ADD"  ); // adicao em ponto fixo com memoria
 "S_ADD"   eval_opcode( 20, 0, yytext,  "S_ADD"  ); // adicao em ponto fixo com pilha
 "F_ADD"   eval_opcode( 21,18, yytext,  "F_ADD"  ); // adicao em ponto flut com memoria
"SF_ADD"   eval_opcode( 22, 0, yytext, "SF_ADD"  ); // adicao em ponto flut com pilha

   "MLT"   eval_opcode( 23,18, yytext,    "MLT"  ); // multip em ponto fixo com memoria
 "S_MLT"   eval_opcode( 24, 0, yytext,  "S_MLT"  ); // multip em ponto fixo com pilha
 "F_MLT"   eval_opcode( 25,18, yytext,  "F_MLT"  ); // multip em ponto flut com memoria
"SF_MLT"   eval_opcode( 26, 0, yytext, "SF_MLT"  ); // multip em ponto flut com pilha

   "DIV"   eval_opcode( 27,18, yytext,    "DIV"  ); // divisa em ponto fixo com memoria
 "S_DIV"   eval_opcode( 28, 0, yytext,  "S_DIV"  ); // divisa em ponto fixo com pilha
 "F_DIV"   eval_opcode( 29,18, yytext,  "F_DIV"  ); // divisa em ponto flut com memoria
"SF_DIV"   eval_opcode( 30, 0, yytext, "SF_DIV"  ); // divisa em ponto flut com pilha

   "MOD"   eval_opcode( 31,18, yytext,    "MOD"  ); // resto da divisao em ponto fixo com memoria
 "S_MOD"   eval_opcode( 32, 0, yytext,  "S_MOD"  ); // resto da divisao em ponto fixo com pilha

   "SGN"   eval_opcode( 33,18, yytext,    "SGN"  ); // pega sinal em ponto fixo com memoria
 "S_SGN"   eval_opcode( 34, 0, yytext,  "S_SGN"  ); // pega sinal em ponto fixo com pilha
 "F_SGN"   eval_opcode( 35,18, yytext,  "F_SGN"  ); // pega sinal em ponto flut com memoria
"SF_SGN"   eval_opcode( 36, 0, yytext, "SF_SGN"  ); // pega sinal em ponto flut com pilha

   "NEG"   eval_opcode( 37, 0, yytext,    "NEG"  ); // negacao em ponto fixo com acc
   "NEG_M" eval_opcode( 38,18, yytext,    "NEG_M"); // negacao em ponto fixo com memoria
 "P_NEG_M" eval_opcode( 39,18, yytext,  "P_NEG_M"); // PUSH +   NEG_M
 "F_NEG"   eval_opcode( 40, 0, yytext,  "F_NEG"  ); // negacao em ponto flut com acc
 "F_NEG_M" eval_opcode( 41,18, yytext,  "F_NEG_M"); // negacao em ponto flut com memoria
"PF_NEG_M" eval_opcode( 42,18, yytext, "PF_NEG_M"); // PUSH + F_NEG_M

   "ABS"   eval_opcode( 43, 0, yytext,    "ABS"  ); // abs em ponto fixo com acc
   "ABS_M" eval_opcode( 44,18, yytext,    "ABS_M"); // abs em ponto fixo com memoria
 "P_ABS_M" eval_opcode( 45,18, yytext,  "P_ABS_M"); // PUSH +   ABS_M
 "F_ABS"   eval_opcode( 46, 0, yytext,  "F_ABS"  ); // abs em ponto flut com acc
 "F_ABS_M" eval_opcode( 47,18, yytext,  "F_ABS_M"); // abs em ponto flut com memoria
"PF_ABS_M" eval_opcode( 48,18, yytext, "PF_ABS_M"); // PUSH + F_ABS_M

   "PST"   eval_opcode( 49, 0, yytext,    "PST"  ); // zera negativo em ponto fixo com acc
   "PST_M" eval_opcode( 50,18, yytext,    "PST_M"); // zera negativo em ponto fixo com memoria
 "P_PST_M" eval_opcode( 51,18, yytext,  "P_PST_M"); // PUSH +   PST_M
 "F_PST"   eval_opcode( 52, 0, yytext,  "F_PST"  ); // zera negativo em ponto flut com acc
 "F_PST_M" eval_opcode( 53,18, yytext,  "F_PST_M"); // zera negativo em ponto flut com memoria
"PF_PST_M" eval_opcode( 54,18, yytext, "PF_PST_M"); // PUSH + F_PST_M

   "NRM"   eval_opcode( 55, 0, yytext,    "NRM"  ); // divide por constante em ponto fixo com acc
   "NRM_M" eval_opcode( 56,18, yytext,    "NRM_M"); // divide por constante em ponto fixo com memoria
 "P_NRM_M" eval_opcode( 57,18, yytext,  "P_NRM_M"); // PUSH +   NRM_M

   "I2F"   eval_opcode( 58, 0, yytext,    "I2F"  ); // int2float com acc
   "I2F_M" eval_opcode( 59,18, yytext,    "I2F_M"); // int2float com mem
 "P_I2F_M" eval_opcode( 60,18, yytext,  "P_I2F_M"); // PUSH + I2F_M

   "F2I"   eval_opcode( 61, 0, yytext,    "F2I"  ); // float2int com acc
   "F2I_M" eval_opcode( 62,18, yytext,    "F2I_M"); // float2int com mem
 "P_F2I_M" eval_opcode( 63,18, yytext,  "P_F2I_M"); // PUSH + F2I_M

   "AND"   eval_opcode( 64,18, yytext,    "AND"  ); // and com memoria
 "S_AND"   eval_opcode( 65, 0, yytext,  "S_AND"  ); // and com pilha
   "ORR"   eval_opcode( 66,18, yytext,    "ORR"  ); // or  com memoria
 "S_ORR"   eval_opcode( 67, 0, yytext,  "S_ORR"  ); // or  cop pilha
   "XOR"   eval_opcode( 68,18, yytext,    "XOR"  ); // xor com memoria
 "S_XOR"   eval_opcode( 69, 0, yytext,  "S_XOR"  ); // xor com pilha
   
   "INV"   eval_opcode( 70, 0, yytext,    "INV"  ); // not com acc
   "INV_M" eval_opcode( 71,18, yytext,    "INV_M"); // not com mem
 "P_INV_M" eval_opcode( 72,18, yytext,  "P_INV_M"); // PUSH + INV_M

   "LAN"   eval_opcode( 73,18, yytext,    "LAN"  ); // and logico com memoria
 "S_LAN"   eval_opcode( 74, 0, yytext,  "S_LAN"  ); // and logico com pilha
   "LOR"   eval_opcode( 75,18, yytext,    "LOR"  ); // or  logico com memoria
 "S_LOR"   eval_opcode( 76, 0, yytext,  "S_LOR"  ); // or  logico com pilha

   "LIN"   eval_opcode( 77, 0, yytext,    "LIN"  ); // inversao logica com acc
   "LIN_M" eval_opcode( 78,18, yytext,    "LIN_M"); // inversao logica com mem
 "P_LIN_M" eval_opcode( 79,18, yytext,  "P_LIN_M"); // PUSH + LIN_M

   "LES"   eval_opcode( 80,18, yytext,    "LES"  ); // menor que em ponto fixo com memoria
 "S_LES"   eval_opcode( 81, 0, yytext,  "S_LES"  ); // menor que em ponto fixo com pilha
 "F_LES"   eval_opcode( 82,18, yytext,  "F_LES"  ); // menor que em ponto flut com memoria
"SF_LES"   eval_opcode( 83, 0, yytext, "SF_LES"  ); // menor que em ponto flut com pilha

   "GRE"   eval_opcode( 84,18, yytext,    "GRE"  ); // maior que em ponto fixo com memoria
 "S_GRE"   eval_opcode( 85, 0, yytext,  "S_GRE"  ); // maior que em ponto fixo com pilha
 "F_GRE"   eval_opcode( 86,18, yytext,  "F_GRE"  ); // maior que em ponto flut com memoria
"SF_GRE"   eval_opcode( 87, 0, yytext, "SF_GRE"  ); // maior que em ponto flut com pilha

   "EQU"   eval_opcode( 88,18, yytext,    "EQU"  ); // igual a com memoria
 "S_EQU"   eval_opcode( 89, 0, yytext,  "S_EQU"  ); // igual a com pilha

   "SHL"   eval_opcode( 90,18, yytext,    "SHL"  ); // shift left  com memoria
 "S_SHL"   eval_opcode( 91, 0, yytext,  "S_SHL"  ); // shift left  com pilha
   "SHR"   eval_opcode( 92,18, yytext,    "SHR"  ); // shift right com memoria
 "S_SHR"   eval_opcode( 93, 0, yytext,  "S_SHR"  ); // shift right com pilha
   "SRS"   eval_opcode( 94,18, yytext,    "SRS"  ); // shift right aritmetico com memoria
 "S_SRS"   eval_opcode( 95, 0, yytext,  "S_SRS"  ); // shift right aritmetico com pilha

   /* instrucoes especiais */

   "NOP"   eval_opcode( 96, 0, yytext,    ""     ); // no operation
 "F_ROT"   eval_opcode( 97, 0, yytext,  "F_ROT"  ); // potencia de 2 mais proxima da raiz (com ACC)
 "F_SU1"   eval_opcode( 98,18, yytext,  "F_SU1"  ); // subtracao de ponto flutuante com memoria na entrada 1
 "F_SU2"   eval_opcode( 99,18, yytext,  "F_SU2"  ); // subtracao de ponto flutuante com memoria na entrada 2
"SF_SU1"   eval_opcode(100, 0, yytext, "SF_SU1"  ); // subtracao de ponto flutuante com pilha na entrada 1
"SF_SU2"   eval_opcode(101, 0, yytext, "SF_SU2"  ); // subtracao de ponto flutuante com pilha na entrada 2

   /* pseudo instrucoes */
   /* evita enderecamento indireto */

   "LOD_V" eval_opcode( 0,22, yytext,    "LOD"  ); //   LOD com offset constante
 "P_LOD_V" eval_opcode( 1,22, yytext,  "P_LOD"  ); // P_LOD com offset constante
   "SET_V" eval_opcode( 4,22, yytext,    "SET"  ); //   SET com offset constante
   "ADD_V" eval_opcode(19,22, yytext,    "ADD"  ); //   ADD com offset constante
 "F_ADD_V" eval_opcode(21,22, yytext,  "F_ADD"  ); // F_ADD com offset constante
   "MLT_V" eval_opcode(23,22, yytext,    "MLT"  ); //   MLT com offset constante
 "F_MLT_V" eval_opcode(25,22, yytext,  "F_MLT"  ); // F_MLT com offset constante

{VARIA}    eval_opernd(yytext,0); // usado em instrucoes da ula com variaveis (IDs)
{INNUM}    eval_opernd(yytext,1); // usado em instrucoes da ula com constante int
{FLNUM}    eval_opernd(yytext,2); // usado em instrucoes da ula com constante float

{STRIN}    eval_opernd(yytext,0); // usado em instrucoes com strings (ex: arquivo de incializacao de array)
{LABEL}                         ; // usado em instrucoes de salto

{COMEN}
{EMPTY}
{LINHA}
{CARES}

.;
%%

int main(int argc, char *argv[])
{
  parse_lang_flag(&argc, argv);                         // processa flag -en/-pt (remove de argv)

	strcpy(proc_dir,argv[2]);                             // path da pasta do projeto
	strcpy( hdl_dir,argv[3]);                             // path da pasta HDL
  strcpy( mac_dir,argv[4]);                             // path da pasta Macros
	strcpy(temp_dir,argv[5]);                             // path da pasta Temp

  yyin = fopen(argv[1], "r");                           // primeiro argumento eh o arquivo .asm de entrada
	eval_init(atoi(argv[6]),atoi(argv[7]),atoi(argv[8])); // inicializa as variaveis de estado
  yylex();                                              // executa o lexer
	fclose(yyin);                                         // fecha o arquivo .asm
	eval_finish();                                        // executa finalizacao do assembler

  printf(MSG_OK_ASM_DONE);

  return 0;
}

int yywrap(void) {return 1;}