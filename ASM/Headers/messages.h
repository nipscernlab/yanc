// ----------------------------------------------------------------------------
// PT/EN bilingual support for asmcomp messages -------------------------------
// ----------------------------------------------------------------------------
// usage:
//   printf(MSG_XXX, args...);   or   fprintf(stderr, MSG_XXX, args...);
// to see both versions of the string, just search for MSG_XXX below.
// ----------------------------------------------------------------------------

#ifndef MESSAGES_H
#define MESSAGES_H

// 0 = Portuguese (default), 1 = English
extern int lang_en;

// string-selection macro. takes PT and EN and picks one based on lang_en
#define M(pt, en) (lang_en ? (en) : (pt))

// scans argv for -en/-pt, adjusts lang_en and removes the flag from argv/argc
void parse_lang_flag(int *argc, char **argv);

// ----------------------------------------------------------------------------
// message catalog ------------------------------------------------------------
// ----------------------------------------------------------------------------

// general errors ------------------------------------------------------------

#define MSG_ERR_OUT_OF_MEMORY \
    M("Erro: memória insuficiente pra alocar a tabela de símbolos!\n", \
      "Error: out of memory while growing the symbol table!\n")

#define MSG_ERR_FILE_WHERE \
    M("Erro: cadê o arquivo %s?\n", \
      "Error: where the heck is the file %s?\n")

#define MSG_ERR_CANT_OPEN_FILE \
    M("Erro: não rolou de abrir o arquivo '%s'!!\n", \
      "Error: couldn't open the file '%s' for the life of me!!\n")

#define MSG_ERR_FP_INCONSISTENT \
    M("Erro: inconsistência no ponto flutuante. Tem que ser NUBITS = NBMANT + NBEXPO + 1.\n", \
      "Error: floating-point setup doesn't add up. You gotta have NUBITS = NBMANT + NBEXPO + 1.\n")

// errors when reading array initialization files ----------------------------

#define MSG_ERR_EMPTY_LINE \
    M("Erro: a linha %d do arquivo '%s' tá vazia! Aí fica difícil.\n", \
      "Error: line %d of file '%s' is empty! Kinda hard to work with that.\n")

#define MSG_ERR_INVALID_INT \
    M("Erro: a linha %d do arquivo '%s' não é um inteiro válido!\n", \
      "Error: line %d of file '%s' isn't a valid integer!\n")

#define MSG_ERR_INT_OVER \
    M("Erro: a linha %d do arquivo '%s' é maior que o limite de %d!\n", \
      "Error: line %d of file '%s' busts the upper limit of %d!\n")

#define MSG_ERR_INT_UNDER \
    M("Erro: a linha %d do arquivo '%s' é menor que o limite de %d!\n", \
      "Error: line %d of file '%s' is below the lower limit of %d!\n")

#define MSG_ERR_INVALID_FLOAT \
    M("Erro: a linha %d do arquivo '%s' não é um float válido!\n", \
      "Error: line %d of file '%s' isn't a valid float!\n")

#define MSG_ERR_FLOAT_OVER \
    M("Erro: a linha %d do arquivo '%s' é maior que o limite de %f!\n", \
      "Error: line %d of file '%s' busts the upper limit of %f!\n")

#define MSG_ERR_FLOAT_UNDER \
    M("Erro: a linha %d do arquivo '%s' é menor que o limite de %f!\n", \
      "Error: line %d of file '%s' is below the lower limit of %f!\n")

#define MSG_ERR_BAD_FORMAT \
    M("Formato inválido!\n", \
      "Bad format!\n")

#define MSG_ERR_INVALID_COMP \
    M("Erro: a linha %d do arquivo '%s' não é um comp válido!\n", \
      "Error: line %d of file '%s' isn't a valid comp!\n")

#define MSG_ERR_COMP_REAL_OVER \
    M("Erro: parte real da linha %d do arquivo '%s' é maior que o limite de %f!\n", \
      "Error: real part of line %d of file '%s' busts the upper limit of %f!\n")

#define MSG_ERR_COMP_REAL_UNDER \
    M("Erro: parte real da linha %d do arquivo '%s' é menor que o limite de %f!\n", \
      "Error: real part of line %d of file '%s' is below the lower limit of %f!\n")

#define MSG_ERR_COMP_IMAG_OVER \
    M("Erro: parte imaginária da linha %d do arquivo '%s' é maior que o limite de %f!\n", \
      "Error: imaginary part of line %d of file '%s' busts the upper limit of %f!\n")

#define MSG_ERR_COMP_IMAG_UNDER \
    M("Erro: parte imaginária da linha %d do arquivo '%s' é menor que o limite de %f!\n", \
      "Error: imaginary part of line %d of file '%s' is below the lower limit of %f!\n")

#define MSG_ERR_MISSING_LINES \
    M("Erro: tá faltando %d linhas no arquivo '%s'!\n", \
      "Error: you're %d lines short in file '%s'!\n")

// warning messages ----------------------------------------------------------

#define MSG_WARN_EXTRA_LINES \
    M("Atenção: tá sobrando %d linhas no arquivo '%s'!\n", \
      "Heads up: there's %d extra lines hanging around in file '%s'!\n")

#define MSG_WARN_UNUSED_IN_PORT \
    M("Atenção: porta de entrada %d não está sendo usada. Gastando hardware à toa!\n", \
      "Heads up: input port %d isn't being used. Burning hardware for nothing!\n")

#define MSG_WARN_UNUSED_OUT_PORT \
    M("Atenção: porta de saída %d não está sendo usada. Gastando hardware à toa!\n", \
      "Heads up: output port %d isn't being used. Burning hardware for nothing!\n")

// info messages for resource usage ------------------------------------------

#define MSG_INFO_FILL_ARRAY \
    M("Info: enchendo array com %d valores lidos do arquivo %s\n", \
      "Info: filling array with %d values read from file %s\n")

#define MSG_INFO_APPROX_ERR \
    M("Info: maior erro de aproximação no arquivo '%s' é %.14f na linha %d\n", \
      "Info: largest approximation error in file '%s' is %.14f on line %d\n")

#define MSG_INFO_APPROX_ERR_REAL \
    M("Info: maior erro de aproximação, na parte real, no arquivo '%s', é %.14f na linha %d\n", \
      "Info: largest approximation error, for real part, in file '%s', is %.14f on line %d\n")

#define MSG_INFO_APPROX_ERR_IMAG \
    M("Info: maior erro de aproximação, na parte imaginária, no arquivo '%s', é %.14f na linha %d\n", \
      "Info: largest approximation error, for imaginary part, in file '%s', is %.14f on line %d\n")

#define MSG_INFO_ISA_USAGE \
    M("Info: usando %d%% do Assembly Instruction Set\n", \
      "Info: using %d%% of the Assembly Instruction Set\n")

#define MSG_INFO_ULA_USAGE \
    M("Info: usando %d%% das operações da ULA\n", \
      "Info: using %d%% of the ULA operations\n")

// info messages for circuit additions (opcodes.c) ---------------------------

#define MSG_INFO_ARRAY_HANDLING \
    M("Info: adicionando suporte a array\n", \
      "Info: adding array handling\n")

#define MSG_INFO_BIT_REVERSE \
    M("Info: adicionando índice de array com bits invertidos (FFT)\n", \
      "Info: adding bit-reverse array index (FFT)\n")

#define MSG_INFO_INPUT_HANDLING \
    M("Info: adicionando suporte a entrada de dados\n", \
      "Info: adding data input handling\n")

#define MSG_INFO_OUTPUT_HANDLING \
    M("Info: adicionando suporte a saída de dados\n", \
      "Info: adding data output handling\n")

#define MSG_INFO_STACK_MEMORY \
    M("Info: adicionando pilha de memória para chamadas de função\n", \
      "Info: adding stack memory for function calls\n")

// info messages for ALU resources (opcodes.c) -------------------------------

#define MSG_INFO_INT2FLOAT \
    M("Info: recurso da ULA -> conversor de int para float\n", \
      "Info: ULA resource -> int to float converter\n")

#define MSG_INFO_FLOAT2INT \
    M("Info: recurso da ULA -> conversor de float para int\n", \
      "Info: ULA resource -> float to int converter\n")

#define MSG_INFO_INT_ADDER \
    M("Info: recurso da ULA -> somador inteiro\n", \
      "Info: ULA resource -> integer adder\n")

#define MSG_INFO_FLOAT_ADDER \
    M("Info: recurso da ULA -> somador em ponto flutuante\n", \
      "Info: ULA resource -> float-point adder\n")

#define MSG_INFO_INT_MULT \
    M("Info: recurso da ULA -> multiplicador inteiro\n", \
      "Info: ULA resource -> integer multiplier\n")

#define MSG_INFO_FLOAT_MULT \
    M("Info: recurso da ULA -> multiplicador em ponto flutuante\n", \
      "Info: ULA resource -> float-point multiplier\n")

#define MSG_INFO_INT_DIV \
    M("Info: recurso da ULA -> divisor inteiro (não recomendado em alta frequência)\n", \
      "Info: ULA resource -> integer divider (not recomended for high frequency opperation)\n")

#define MSG_INFO_FLOAT_DIV \
    M("Info: recurso da ULA -> divisor em ponto flutuante (não recomendado em alta frequência)\n", \
      "Info: ULA resource -> float-point divider (not recomended for high frequency opperation)\n")

#define MSG_INFO_MODULO \
    M("Info: recurso da ULA -> cálculo de módulo (não recomendado em alta frequência)\n", \
      "Info: ULA resource -> modulo computation (not recomended for high frequency opperation)\n")

#define MSG_INFO_INT_SIGN \
    M("Info: recurso da ULA -> cálculo de sinal inteiro (standard library)\n", \
      "Info: ULA resource -> integer sign computation (standard library)\n")

#define MSG_INFO_FLOAT_SIGN \
    M("Info: recurso da ULA -> cálculo de sinal em ponto flutuante (standard library)\n", \
      "Info: ULA resource -> float-point sign computation (standard library)\n")

#define MSG_INFO_INT_NEG \
    M("Info: recurso da ULA -> operação de negativo inteiro\n", \
      "Info: ULA resource -> integer negative operation\n")

#define MSG_INFO_FLOAT_NEG \
    M("Info: recurso da ULA -> operação de negativo em ponto flutuante\n", \
      "Info: ULA resource -> float-point negative operation\n")

#define MSG_INFO_INT_ABS \
    M("Info: recurso da ULA -> operação de valor absoluto inteiro\n", \
      "Info: ULA resource -> integer absolute operation\n")

#define MSG_INFO_FLOAT_ABS \
    M("Info: recurso da ULA -> operação de valor absoluto em ponto flutuante\n", \
      "Info: ULA resource -> float-point absolute operation\n")

#define MSG_INFO_INT_PSET \
    M("Info: recurso da ULA -> cálculo de pset inteiro (standard library)\n", \
      "Info: ULA resource -> integer pset computation (standard library)\n")

#define MSG_INFO_FLOAT_PSET \
    M("Info: recurso da ULA -> cálculo de pset em ponto flutuante (standard library)\n", \
      "Info: ULA resource -> float-point pset computation (standard library)\n")

#define MSG_INFO_NORM \
    M("Info: recurso da ULA -> cálculo de normalização (standard library)\n", \
      "Info: ULA resource -> normalization computation (standard library)\n")

#define MSG_INFO_OP_AND \
    M("Info: recurso da ULA -> operador &\n", \
      "Info: ULA resource -> & operator\n")

#define MSG_INFO_OP_OR \
    M("Info: recurso da ULA -> operador |\n", \
      "Info: ULA resource -> | operator\n")

#define MSG_INFO_OP_XOR \
    M("Info: recurso da ULA -> operador ^\n", \
      "Info: ULA resource -> ^ operator\n")

#define MSG_INFO_OP_NOT \
    M("Info: recurso da ULA -> operador ~\n", \
      "Info: ULA resource -> ~ operator\n")

#define MSG_INFO_OP_LAND \
    M("Info: recurso da ULA -> operador &&\n", \
      "Info: ULA resource -> && operator\n")

#define MSG_INFO_OP_LOR \
    M("Info: recurso da ULA -> operador ||\n", \
      "Info: ULA resource -> || operator\n")

#define MSG_INFO_OP_LNOT \
    M("Info: recurso da ULA -> operador !\n", \
      "Info: ULA resource -> ! operator\n")

#define MSG_INFO_OP_LES_INT \
    M("Info: recurso da ULA -> operador < pra int\n", \
      "Info: ULA resource -> < operator for int\n")

#define MSG_INFO_OP_LES_FLOAT \
    M("Info: recurso da ULA -> operador < pra float\n", \
      "Info: ULA resource -> < operator for float\n")

#define MSG_INFO_OP_GRE_INT \
    M("Info: recurso da ULA -> operador > pra int\n", \
      "Info: ULA resource -> > operator for int\n")

#define MSG_INFO_OP_GRE_FLOAT \
    M("Info: recurso da ULA -> operador > pra float\n", \
      "Info: ULA resource -> > operator for float\n")

#define MSG_INFO_OP_EQU \
    M("Info: recurso da ULA -> operador ==\n", \
      "Info: ULA resource -> == operator\n")

#define MSG_INFO_OP_SHL \
    M("Info: recurso da ULA -> operador <<\n", \
      "Info: ULA resource -> << operator\n")

#define MSG_INFO_OP_SHR \
    M("Info: recurso da ULA -> operador >>\n", \
      "Info: ULA resource -> >> operator\n")

#define MSG_INFO_OP_SRS \
    M("Info: recurso da ULA -> operador >>>\n", \
      "Info: ULA resource -> >>> operator\n")

#define MSG_INFO_FLOAT_SQRT \
    M("Info: recurso da ULA -> aproximação de raiz quadrada para float\n", \
      "Info: ULA resource -> root-square approximation for float\n")

// success messages ----------------------------------------------------------

#define MSG_OK_ASM_DONE \
    M("Sucesso: o processador tá pronto pra uso.\n", \
      "Sweet: the processor is good to go.\n")

#endif
