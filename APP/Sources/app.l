%{
    #include "..\Headers\eval.h"
    #include "..\Headers\messages.h"
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

"#PRNAME"  eval_direct( 1); // nome do processador
"#NUBITS"  eval_direct( 2); // num de bits da ULA
"#NBMANT"  eval_direct( 3); // num de bits da mantissa
"#NBEXPO"  eval_direct( 4); // num de bits do expoente
"#NDSTAC"  ;                // precisa ser identificado pra nao dar erro
"#SDEPTH"  ;                // mesma coisa pra todos ai embaixo
"#NUIOIN"  ;
"#NUIOOU"  ;
"#NUGAIN"  ;
"#FFTSIZ"  ;

"#array"   eval_direct(11); // array normal
"#arrays"  eval_direct(14); // array com arquivo de inicializacao

"#ITRAD"   eval_itrad (  ); // ponto de endereco da interrupcao

   "LOD"   eval_opcode(18); // carrega dado da memoria
 "P_LOD"   eval_opcode(18); // PUSH + LOD
   "LDI"   eval_opcode(18); // LOD com enderecamento indireto (indice no acc)
   "ILI"   eval_opcode(18); // LOD com enderecamento indireto invertido (FFT)

   "SET"   eval_opcode(18); // salva dado na memoria
   "SET_P" eval_opcode(18); // SET + POP
   "STI"   eval_opcode(18); // SET com enderecamento indireto (indice na pilha)
   "ISI"   eval_opcode(18); // SET com enderecamento indireto invertido (FFT)

   "PSH"   eval_opcode( 0); // xuxa dado na pilha
   "POP"   eval_opcode( 0); // retira dado da pilha

   "INN"   eval_opcode(20); // entrada de dados
 "F_INN"   eval_opcode(20); // entrada de dados em ponto flutuante
 "P_INN"   eval_opcode(20); // PUSH + INN
"PF_INN"   eval_opcode(20); // PUSH + F_INN
   "OUT"   eval_opcode(21); // saida de dados

   "JMP"   eval_opcode(19); // salto incondicional
   "JIZ"   eval_opcode(19); // salta se acc for zero

   "CAL"   eval_opcode(19); // chamada de subrotina
   "RET"   eval_opcode( 0); // retorno de subrotina

   "ADD"   eval_opcode(18); // adicao em ponto fixo com memoria
 "S_ADD"   eval_opcode( 0); // adicao em ponto fixo com pilha
 "F_ADD"   eval_opcode(18); // adicao em ponto flut com memoria
"SF_ADD"   eval_opcode( 0); // adicao em ponto flut com pilha

   "MLT"   eval_opcode(18); // multip em ponto fixo com memoria
 "S_MLT"   eval_opcode( 0); // multip em ponto fixo com pilha
 "F_MLT"   eval_opcode(18); // multip em ponto flut com memoria
"SF_MLT"   eval_opcode( 0); // multip em ponto flut com pilha

   "DIV"   eval_opcode(18); // divisa em ponto fixo com memoria
 "S_DIV"   eval_opcode( 0); // divisa em ponto fixo com pilha
 "F_DIV"   eval_opcode(18); // divisa em ponto flut com memoria
"SF_DIV"   eval_opcode( 0); // divisa em ponto flut com pilha

   "MOD"   eval_opcode(18); // resto da divisao em ponto fixo com memoria
 "S_MOD"   eval_opcode( 0); // resto da divisao em ponto fixo com pilha

   "SGN"   eval_opcode(18); // pega sinal em ponto fixo com memoria
 "S_SGN"   eval_opcode( 0); // pega sinal em ponto fixo com pilha
 "F_SGN"   eval_opcode(18); // pega sinal em ponto flut com memoria
"SF_SGN"   eval_opcode( 0); // pega sinal em ponto flut com pilha

   "NEG"   eval_opcode( 0); // negacao em ponto fixo com acc
   "NEG_M" eval_opcode(18); // negacao em ponto fixo com memoria
 "P_NEG_M" eval_opcode(18); // PUSH +   NEG_M
 "F_NEG"   eval_opcode( 0); // negacao em ponto flut com acc
 "F_NEG_M" eval_opcode(18); // negacao em ponto flut com memoria
"PF_NEG_M" eval_opcode(18); // PUSH + F_NEG_M

   "ABS"   eval_opcode( 0); // abs em ponto fixo com acc
   "ABS_M" eval_opcode(18); // abs em ponto fixo com memoria
 "P_ABS_M" eval_opcode(18); // PUSH +   ABS_M
 "F_ABS"   eval_opcode( 0); // abs em ponto flut com acc
 "F_ABS_M" eval_opcode(18); // abs em ponto flut com memoria
"PF_ABS_M" eval_opcode(18); // PUSH + F_ABS_M

   "PST"   eval_opcode( 0); // zera negativo em ponto fixo com acc
   "PST_M" eval_opcode(18); // zera negativo em ponto fixo com memoria
 "P_PST_M" eval_opcode(18); // PUSH +   PST_M
 "F_PST"   eval_opcode( 0); // zera negativo em ponto flut com acc
 "F_PST_M" eval_opcode(18); // zera negativo em ponto flut com memoria
"PF_PST_M" eval_opcode(18); // PUSH + F_PST_M

   "NRM"   eval_opcode( 0); // divide por constante em ponto fixo com acc
   "NRM_M" eval_opcode(18); // divide por constante em ponto fixo com memoria
 "P_NRM_M" eval_opcode(18); // PUSH +   NRM_M

   "I2F"   eval_opcode( 0); // int2float com acc
   "I2F_M" eval_opcode(18); // int2float com mem
 "P_I2F_M" eval_opcode(18); // PUSH + I2F_M

   "F2I"   eval_opcode( 0); // float2int com acc
   "F2I_M" eval_opcode(18); // float2int com mem
 "P_F2I_M" eval_opcode(18); // PUSH + F2I_M

   "AND"   eval_opcode(18); // and com memoria
 "S_AND"   eval_opcode( 0); // and com pilha
   "ORR"   eval_opcode(18); // or  com memoria
 "S_ORR"   eval_opcode( 0); // or  cop pilha
   "XOR"   eval_opcode(18); // xor com memoria
 "S_XOR"   eval_opcode( 0); // xor com pilha
   
   "INV"   eval_opcode( 0); // not com acc
   "INV_M" eval_opcode(18); // not com mem
 "P_INV_M" eval_opcode(18); // PUSH + INV_M

   "LAN"   eval_opcode(18); // and logico com memoria
 "S_LAN"   eval_opcode( 0); // and logico com pilha
   "LOR"   eval_opcode(18); // or  logico com memoria
 "S_LOR"   eval_opcode( 0); // or  logico com pilha

   "LIN"   eval_opcode( 0); // inversao logica com acc
   "LIN_M" eval_opcode(18); // inversao logica com mem
 "P_LIN_M" eval_opcode(18); // PUSH + LIN_M

   "LES"   eval_opcode(18); // menor que em ponto fixo com memoria
 "S_LES"   eval_opcode( 0); // menor que em ponto fixo com pilha
 "F_LES"   eval_opcode(18); // menor que em ponto flut com memoria
"SF_LES"   eval_opcode( 0); // menor que em ponto flut com pilha

   "GRE"   eval_opcode(18); // maior que em ponto fixo com memoria
 "S_GRE"   eval_opcode( 0); // maior que em ponto fixo com pilha
 "F_GRE"   eval_opcode(18); // maior que em ponto flut com memoria
"SF_GRE"   eval_opcode( 0); // maior que em ponto flut com pilha

   "EQU"   eval_opcode(18); // igual a com memoria
 "S_EQU"   eval_opcode( 0); // igual a com pilha

   "SHL"   eval_opcode(18); // shift left  com memoria
 "S_SHL"   eval_opcode( 0); // shift left  com pilha
   "SHR"   eval_opcode(18); // shift right com memoria
 "S_SHR"   eval_opcode( 0); // shift right com pilha
   "SRS"   eval_opcode(18); // shift right aritmetico com memoria
 "S_SRS"   eval_opcode( 0); // shift right aritmetico com pilha

   /* instrucoes especiais */

   "NOP"   eval_opcode( 0); // no operation
 "F_ROT"   eval_opcode( 0); // potencia de 2 mais proxima da raiz (com ACC)
 "F_SU1"   eval_opcode(18); // subtracao de ponto flutuante com memoria na entrada 1
 "F_SU2"   eval_opcode(18); // subtracao de ponto flutuante com memoria na entrada 2
"SF_SU1"   eval_opcode( 0); // subtracao de ponto flutuante com pilha na entrada 1
"SF_SU2"   eval_opcode( 0); // subtracao de ponto flutuante com pilha na entrada 2

   /* pseudo instrucoes */
   /* evita enderecamento indireto */
   
   "LOD_V" eval_opcode(22); //   LOD com offset constante
 "P_LOD_V" eval_opcode(22); // P_LOD com offset constante
   "SET_V" eval_opcode(22); //   SET com offset constante
   "ADD_V" eval_opcode(22); //   ADD com offset constante
 "F_ADD_V" eval_opcode(22); // F_ADD com offset constante
   "MLT_V" eval_opcode(22); //   MLT com offset constante
 "F_MLT_V" eval_opcode(22); // F_MLT com offset constante

{VARIA}    eval_opernd(yytext); // usado em instrucoes da ula com variaveis (IDs)
{INNUM}    eval_opernd(yytext); // usado em instrucoes da ula com constante int
{FLNUM}    eval_opernd(yytext); // usado em instrucoes da ula com constante float

{STRIN}    eval_opernd(yytext); // usado em instrucoes com strings (ex: arquivo de incializacao de array)
{LABEL}    eval_label (yytext); // usado em instrucoes de salto

{COMEN}
{EMPTY}
{LINHA}
{CARES}

.;
%%

int main(int argc, char *argv[])
{
    parse_lang_flag(&argc, argv);   // processa flag -en/-pt (remove de argv)

	  yyin = fopen(argv[1], "r"); // primeiro argumento eh o arquivo .asm de entrada
    eval_init   (argv[2]);      // segundo  argumento eh o caminho pro arquivo de log
    yylex();                    // executa o lexer
	  fclose(yyin);               // fecha o arquivo .asm
    eval_finish();              // finaliza o arquivo de log

    printf(MSG_OK_APP_DONE);

    return 0;
}

int yywrap(void) {return 1;}