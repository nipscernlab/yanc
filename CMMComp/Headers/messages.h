// ----------------------------------------------------------------------------
// bilingual PT/EN support for cmmcomp messages -------------------------------
// ----------------------------------------------------------------------------
// usage:
//   printf(MSG_XXX, args...);   or   fprintf(stderr, MSG_XXX, args...);
// to see both string versions, search for MSG_XXX below.
// ----------------------------------------------------------------------------

#ifndef MESSAGES_H
#define MESSAGES_H

// 0 = portuguese (default), 1 = english
extern int lang_en;

// string-selection macro. takes PT and EN and chooses based on lang_en
#define M(pt, en) (lang_en ? (en) : (pt))

// scans argv for -en/-pt, sets lang_en and removes the flag from argv/argc
void parse_lang_flag(int *argc, char **argv);

// ============================================================================
// MESSAGE CATALOG ============================================================
// ============================================================================

// general errors (parser/main/limits) ---------------------------------------

#define MSG_ERR_SYNTAX \
    M("Erro de sintaxe na linha %d. Você é uma pessoa confusa!\n", \
      "Syntax error on line %d. You're a confused soul!\n")

#define MSG_ERR_OUT_OF_MEMORY \
    M("Erro: memória insuficiente pra alocar a tabela de símbolos!\n", \
      "Error: out of memory while growing the symbol table!\n")

#define MSG_ERR_NO_MAIN \
    M("Erro: cadê a função main()?\n", \
      "Error: yo, where's the main() function?\n")

// variable / number errors --------------------------------------------------

#define MSG_ERR_RESERVED_I \
    M("Erro na linha %d: símbolo 'i' é reservado para indicar a parte imaginária de uma constante complexa.\n", \
      "Error on line %d: the symbol 'i' is reserved for the imaginary part of a complex constant. Hands off!\n")

#define MSG_ERR_INT_MAX_OVERFLOW \
    M("Erro na linha %d: o maior número inteiro que pode ser representado é %d!\n", \
      "Error on line %d: the biggest integer this thing can hold is %d!\n")

#define MSG_ERR_FLOAT_MIN \
    M("Erro na linha %d: o menor número float que pode ser representado é %f!\n", \
      "Error on line %d: the smallest float this thing can hold is %f!\n")

#define MSG_ERR_FLOAT_MAX \
    M("Erro na linha %d: o maior número float que pode ser representado é %f!\n", \
      "Error on line %d: the biggest float this thing can hold is %f!\n")

// declaration / assignment / variable-use errors ---------------------------

#define MSG_ERR_VAR_EXISTS \
    M("Erro na linha %d: a variável '%s' já existe. Vai tomar um Ω³!\n", \
      "Error on line %d: the variable '%s' already exists. Go grab an Ω³!\n")

#define MSG_ERR_VAR_NOT_FOUND \
    M("Erro na linha %d: não tem essa variável '%s'!\n", \
      "Error on line %d: there's no variable '%s'!\n")

#define MSG_ERR_DECLARE_VAR_PLEASE \
    M("Erro na linha %d: se você declarar a variável '%s' eu agradeço.\n", \
      "Error on line %d: I'd really appreciate it if you declared the variable '%s'.\n")

#define MSG_ERR_DECL_VAR_PROPERLY \
    M("Erro na linha %d: mané, declara a variável '%s' direito!\n", \
      "Error on line %d: c'mon dude, declare the variable '%s' properly!\n")

#define MSG_ERR_DECL_FIRST \
    M("Erro na linha %d: tem que declarar '%s' primeiro!\n", \
      "Error on line %d: gotta declare '%s' first!\n")

#define MSG_ERR_MISSING_ARR_IDX \
    M("Erro na linha %d: cadê o índice de array da variável '%s'?\n", \
      "Error on line %d: where's the array index for the variable '%s'?\n")

#define MSG_ERR_ARRAY_NEEDS_IDX \
    M("Erro na linha %d: '%s' é um array. Cadê o índice?\n", \
      "Error on line %d: '%s' is an array. Where's the index, dude?\n")

#define MSG_ERR_NOT_ARRAY \
    M("Erro na linha %d: '%s' não é um array.\n", \
      "Error on line %d: '%s' isn't an array.\n")

#define MSG_ERR_NOT_ARRAY_HARSH \
    M("Erro na linha %d: '%s' não é array não, abensoado!\n", \
      "Error on line %d: '%s' ain't an array, mate!\n")

#define MSG_ERR_ARRAY_2D \
    M("Erro na linha %d: array '%s' tem duas dimensões!\n", \
      "Error on line %d: array '%s' has two dimensions!\n")

#define MSG_ERR_ARRAY_1D \
    M("Erro na linha %d: array '%s' tem uma dimensão só!\n", \
      "Error on line %d: array '%s' has just one dimension!\n")

#define MSG_ERR_INCR_COMPLEX \
    M("Erro na linha %d: o que você bebeu pra querer incrementar um número complexo?\n", \
      "Error on line %d: what were you smoking when you tried to increment a complex number?\n")

#define MSG_ERR_WRONG_USE \
    M("Erro na linha %d: não é assim que se usa '%s'!\n", \
      "Error on line %d: that's not how you use '%s'!\n")

// I/O errors and warnings ---------------------------------------------------

#define MSG_ERR_NO_IN_PORT \
    M("Erro na linha %d: não tem porta de entrada %s não!\n", \
      "Error on line %d: there's no input port %s, dude!\n")

#define MSG_ERR_NO_OUT_PORT \
    M("Erro na linha %d: não tem porta de saída %s não!\n", \
      "Error on line %d: there's no output port %s, dude!\n")

#define MSG_WARN_USE_FOUT \
    M("Atenção na linha %d: se não quiser esse warning, use 'fout'.\n", \
      "Heads up on line %d: if you don't want this warning, use 'fout'.\n")

#define MSG_WARN_USE_OUT \
    M("Atenção na linha %d: se não quiser esse warning, use 'out'.\n", \
      "Heads up on line %d: if you don't want this warning, use 'out'.\n")

// stdlib errors and warnings (special functions) ----------------------------

#define MSG_ERR_PICK_COMP_INFO \
    M("Erro na linha %d: primeiro seleciona qual informação desse número complexo você quer!\n", \
      "Error on line %d: first pick which info you want from this complex number!\n")

#define MSG_ERR_SIGN_COMPLEX \
    M("Erro na linha %d: não faz sentido o uso de sign(.,.) com números complexos!\n", \
      "Error on line %d: using sign(.,.) with complex numbers makes zero sense!\n")

#define MSG_ERR_PSET_COMPLEX \
    M("Erro na linha %d: não faz nenhum sentido usar a função 'pset(.)' com números complexos!\n", \
      "Error on line %d: using the 'pset(.)' function with complex numbers makes zero sense!\n")

#define MSG_ERR_NORM_NON_INT \
    M("Erro na linha %d: nada a ver! norm(.) é só pra inteiro!\n", \
      "Error on line %d: nope! norm(.) is integer-only!\n")

#define MSG_ERR_SQRT_COMPLEX \
    M("Erro na linha %d: não implementei raiz quadrada de número complexo ainda. Se vira!\n", \
      "Error on line %d: I haven't coded up complex square roots yet. You're on your own!\n")

#define MSG_ERR_REAL_ARG_COMP \
    M("Erro na linha %d: argumento da função real(.) tem que ser complexo!\n", \
      "Error on line %d: argument of real(.) has gotta be complex!\n")

#define MSG_ERR_IMAG_ARG_COMP \
    M("Erro na linha %d: argumento da função imag(.) tem que ser complexo!\n", \
      "Error on line %d: argument of imag(.) has gotta be complex!\n")

#define MSG_ERR_MOD2_ARG_COMP \
    M("Erro na linha %d: argumento da função mod2(.) tem que ser complexo!\n", \
      "Error on line %d: argument of mod2(.) has gotta be complex!\n")

#define MSG_ERR_FASE_ARG_COMP \
    M("Erro na linha %d: argumento da função fase(.) tem que ser complexo!\n", \
      "Error on line %d: argument of fase(.) has gotta be complex!\n")

#define MSG_ERR_COMPLEX_OF_COMPLEX \
    M("Erro na linha %d: argumentos da função complex(.,.) não podem ser complexos!\n", \
      "Error on line %d: arguments of complex(.,.) can't themselves be complex!\n")

// linear algebra errors (Dirac notation) ------------------------------------

#define MSG_ERR_INNER_NEEDS_VECTORS \
    M("Erro na linha %d: o nome tá dizendo, produto vetorial é entre vetores!\n", \
      "Error on line %d: the name says it all, vector product is between vectors!\n")

#define MSG_ERR_VECTOR_SIZE_DIFF \
    M("Erro na linha %d: vetores de tamanhos diferentes? Vai estudar Álgebra Linear primeiro!\n", \
      "Error on line %d: vectors with different sizes? Go study Linear Algebra first!\n")

#define MSG_ERR_VECTOR_SIZE_DIFF2 \
    M("Erro na linha %d: os vetores têm tamanhos diferentes! Você é uma pessoa confusa.\n", \
      "Error on line %d: vectors have different sizes! You're a confused soul.\n")

#define MSG_ERR_TYPE_DIFF \
    M("Erro na linha %d: tipos de dados diferentes. Você é uma pessoa confusa!\n", \
      "Error on line %d: different data types. You're a confused soul!\n")

#define MSG_ERR_VARS_SAME_TYPE \
    M("Erro na linha %d: as variáveis têm que ser do mesmo tipo!\n", \
      "Error on line %d: variables gotta be the same type!\n")

#define MSG_ERR_NOT_IMPL_COMPLEX \
    M("Erro na linha %d: não implementei isso pra números complexos ainda. Se vira!\n", \
      "Error on line %d: haven't coded this up for complex numbers yet. You're on your own!\n")

#define MSG_ERR_NOT_A_VECTOR \
    M("Erro na linha %d: '%s' nem vetor é, abensoado!\n", \
      "Error on line %d: '%s' ain't even a vector, mate!\n")

#define MSG_ERR_NOT_A_MATRIX \
    M("Erro na linha %d: '%s' nem matriz é, abensoado!\n", \
      "Error on line %d: '%s' ain't even a matrix, mate!\n")

#define MSG_ERR_NOT_A_MATRIX2 \
    M("Erro na linha %d: '%s' não é uma matriz!\n", \
      "Error on line %d: '%s' isn't a matrix!\n")

#define MSG_ERR_MATRIX_ROW_MISMATCH \
    M("Erro na linha %d: o número de linhas de '%s' não bate com o tamanho de '%s'!\n", \
      "Error on line %d: row count of '%s' doesn't match size of '%s'!\n")

#define MSG_ERR_MATRIX_COL_MISMATCH \
    M("Erro na linha %d: o número de colunas de '%s' não bate com o tamanho de '%s'!\n", \
      "Error on line %d: column count of '%s' doesn't match size of '%s'!\n")

#define MSG_ERR_DIM_MISMATCH \
    M("Erro na linha %d: as dimensões não batem. Você é uma pessoa confusa!\n", \
      "Error on line %d: dimensions don't match. You're a confused soul!\n")

#define MSG_ERR_SHIFT_VEC_SELF \
    M("Erro na linha %d: só dá pra fazer shift de um vetor nele mesmo, abensoado!\n", \
      "Error on line %d: you can only shift a vector into itself, mate!\n")

// operator errors and warnings (oper.c) -------------------------------------

#define MSG_ERR_MOD_NON_INT \
    M("Erro na linha %d: qual o sentido de calcular o resto da divisão sem ser com número inteiro? Vai se tratar!\n", \
      "Error on line %d: what's the point of computing a remainder with non-integer numbers? Get a grip!\n")

#define MSG_WARN_CMP_INT_COMP \
    M("Atenção na linha %d: comparando int com comp? Vou pegar o módulo.\n", \
      "Heads up on line %d: comparing int with comp? Gonna use the magnitude.\n")

#define MSG_WARN_CMP_FLOAT_COMP \
    M("Atenção na linha %d: comparando float com comp? Vou pegar o módulo.\n", \
      "Heads up on line %d: comparing float with comp? Gonna use the magnitude.\n")

#define MSG_WARN_CMP_COMPLEX \
    M("Atenção na linha %d: comparação com número complexo? Vou usar o módulo.\n", \
      "Heads up on line %d: comparing with a complex number? Gonna use the magnitude.\n")

#define MSG_WARN_LOGIC_FLOAT \
    M("Atenção na linha %d: expressão lógica com float? Você é uma pessoa confusa!\n", \
      "Heads up on line %d: logical expression with a float? You're a confused soul!\n")

#define MSG_WARN_LOGIC_COMP \
    M("Atenção na linha %d: expressão lógica com comp? Sério? Vou arredondar a parte real!\n", \
      "Heads up on line %d: logical expression with a comp? Really? Gonna round the real part!\n")

#define MSG_ERR_LOGIC_NON_INT \
    M("Erro na linha %d: operação lógica, só entre números inteiros!\n", \
      "Error on line %d: logical operation, integers only!\n")

#define MSG_ERR_INV_NON_INT \
    M("Erro na linha %d: uso incorreto do operador '~'. Tem que passar tipo int. Viajou?\n", \
      "Error on line %d: bad use of the '~' operator. Gotta pass an int. You tripping?\n")

#define MSG_ERR_BITWISE_COMPLEX \
    M("Erro na linha %d: como você quer que eu faça operações bitwise com um número complexo? Viajou?\n", \
      "Error on line %d: how am I supposed to do bitwise ops with a complex number? You tripping?\n")

#define MSG_ERR_SHIFT_COMPLEX \
    M("Erro na linha %d: como você quer que eu desloque bits de um número complexo? Viajou?\n", \
      "Error on line %d: how am I supposed to shift bits of a complex number? You tripping?\n")

#define MSG_ERR_SHIFT_BY_COMP \
    M("Erro na linha %d: usando comp pra deslocar bits? Você é uma pessoa confusa!\n", \
      "Error on line %d: using comp to shift bits? You're a confused soul!\n")

#define MSG_WARN_SHIFT_BY_FLOAT \
    M("Atenção na linha %d: o segundo operando do shift tá em float. Aí você me quebra!\n", \
      "Heads up on line %d: the second operand of the shift is a float. Now you're killin' me!\n")

// type-conversion warnings (data_assign.c / funcoes.c) ----------------------

#define MSG_WARN_INT_RECV_FLOAT \
    M("Atenção na linha %d: variável '%s' é int, mas recebe float.\n", \
      "Heads up on line %d: variable '%s' is int, but getting a float.\n")

#define MSG_WARN_FLOAT_RECV_INT \
    M("Atenção na linha %d: variável '%s' é float, mas recebe int.\n", \
      "Heads up on line %d: variable '%s' is float, but getting an int.\n")

#define MSG_WARN_COMP_RECV_INT \
    M("Atenção na linha %d: variável '%s' é comp, mas recebe int.\n", \
      "Heads up on line %d: variable '%s' is comp, but getting an int.\n")

#define MSG_WARN_COMP_RECV_FLOAT \
    M("Atenção na linha %d: variável '%s' é comp, mas recebe float.\n", \
      "Heads up on line %d: variable '%s' is comp, but getting a float.\n")

#define MSG_WARN_ROUND_REAL \
    M("Atenção na linha %d: nessa conversão, eu vou arredondar a parte real hein!\n", \
      "Heads up on line %d: in this conversion I'm rounding the real part, just so you know!\n")

#define MSG_WARN_GRAB_REAL_ONLY \
    M("Atenção na linha %d: nessa conversão, eu vou pegar só a parte real hein!\n", \
      "Heads up on line %d: in this conversion I'm only grabbing the real part, just so you know!\n")

// function errors/warnings (funcoes.c) --------------------------------------

#define MSG_WARN_CONV_F2I_PARAM \
    M("Atenção na linha %d: convertendo float para int no parâmetro %d da função '%s'.\n", \
      "Heads up on line %d: converting float to int on parameter %d of function '%s'.\n")

#define MSG_WARN_CONV_C2I_PARAM \
    M("Atenção na linha %d: convertendo comp para int no parâmetro %d da função '%s'.\n", \
      "Heads up on line %d: converting comp to int on parameter %d of function '%s'.\n")

#define MSG_WARN_CONV_I2F_PARAM \
    M("Atenção na linha %d: convertendo int para float no parâmetro %d da função '%s'.\n", \
      "Heads up on line %d: converting int to float on parameter %d of function '%s'.\n")

#define MSG_WARN_CONV_C2F_PARAM \
    M("Atenção na linha %d: convertendo comp para float no parâmetro %d da função '%s'.\n", \
      "Heads up on line %d: converting comp to float on parameter %d of function '%s'.\n")

#define MSG_WARN_CONV_I2C_PARAM \
    M("Atenção na linha %d: convertendo int para comp no parâmetro %d da função '%s'.\n", \
      "Heads up on line %d: converting int to comp on parameter %d of function '%s'.\n")

#define MSG_WARN_CONV_F2C_PARAM \
    M("Atenção na linha %d: convertendo float para comp no parâmetro %d da função '%s'.\n", \
      "Heads up on line %d: converting float to comp on parameter %d of function '%s'.\n")

#define MSG_ERR_VOID_RETURN_VALUE \
    M("Erro na linha %d: valor de retorno em função void? viajou!\n", \
      "Error on line %d: a return value in a void function? You tripping!\n")

#define MSG_WARN_CONV_F2I_RETURN \
    M("Atenção na linha %d: vai converter float para int no retorno da função '%s'? Dá-lhe código!\n", \
      "Heads up on line %d: gonna convert float to int on the return of function '%s'? Code away then!\n")

#define MSG_WARN_RET_FLOAT_RECV_INT \
    M("Atenção na linha %d: retorno é float, mas recebe int.\n", \
      "Heads up on line %d: return is float, but getting an int.\n")

#define MSG_WARN_RET_COMP_RECV_INT \
    M("Atenção na linha %d: retorno da função é comp, mas recebe int.\n", \
      "Heads up on line %d: function return is comp, but getting an int.\n")

#define MSG_WARN_RET_COMP_RECV_FLOAT \
    M("Atenção na linha %d: retorno da função é comp, mas recebe float.\n", \
      "Heads up on line %d: function return is comp, but getting a float.\n")

#define MSG_ERR_FUNC_NO_RETURN \
    M("Erro na função %s: cadê o retorno pra essa função?\n", \
      "Error in function %s: where's the return for this function?\n")

#define MSG_ERR_NO_RETURN_VALUE \
    M("Erro na linha %d: cadê o valor de retorno da função?\n", \
      "Error on line %d: where's the function return value?\n")

#define MSG_ERR_FUNC_WHERE \
    M("Erro na linha %d: cadê essa função '%s'?\n", \
      "Error on line %d: where's this function '%s' supposed to be?\n")

#define MSG_ERR_FUNC_WHERE2 \
    M("Erro na linha %d: A função '%s' tá onde?\n", \
      "Error on line %d: function '%s' is where exactly?\n")

#define MSG_ERR_PARAM_COUNT \
    M("Erro na linha %d: olha lá direito quantos parâmetros tem a função '%s'.\n", \
      "Error on line %d: check again how many parameters the function '%s' actually takes.\n")

#define MSG_ERR_VOID_FUNC_USE \
    M("Erro na linha %d: olha lá a funcao '%s', você vai ver que ela nao retorna nada.\n", \
      "Error on line %d: take a look at function '%s', you'll see it doesn't return anything.\n")

#define MSG_ERR_PARAM_LIST_DIFF \
    M("Erro na linha %d: lista de parâmetros da função '%s' difere da original.\n", \
      "Error on line %d: parameter list of function '%s' doesn't match the original.\n")

// jump errors/warnings (saltos.c) -------------------------------------------

#define MSG_WARN_COND_FLOAT \
    M("Atenção na linha %d: expressão condicional dando float! Vou arredondar.\n", \
      "Heads up on line %d: conditional expression came out float! Gonna round it.\n")

#define MSG_WARN_COND_COMP \
    M("Atenção na linha %d: expressão condicional dando comp! Vou arredondar a parte real.\n", \
      "Heads up on line %d: conditional expression came out comp! Gonna round the real part.\n")

#define MSG_ERR_BREAK_LOST \
    M("Erro na linha %d: esse brake aí tá perdido!\n", \
      "Error on line %d: that break is just hanging out lost!\n")

#define MSG_ERR_NESTED_SWITCH \
    M("Erro na linha %d: um switch/case dentro de outro? Você é uma pessoa confusa!\n", \
      "Error on line %d: a switch/case inside another one? You're a confused soul, aren't you!\n")

#define MSG_WARN_CASE_FLOAT \
    M("Atenção na linha %d: índice do case dando float! Vou arredondar.\n", \
      "Heads up on line %d: case index came out float! Gonna round it.\n")

#define MSG_WARN_CASE_COMP \
    M("Atenção na linha %d: índice do case dando comp! Vou arredondar a parte real.\n", \
      "Heads up on line %d: case index came out comp! Gonna round the real part.\n")

// array-index warnings (array_index.c) --------------------------------------

#define MSG_WARN_IDX_FLOAT \
    M("Atenção na linha %d: tá vendo que o índice do array tá em ponto flutuante né? Vou arredondar!\n", \
      "Heads up on line %d: see how the array index came out a float? Gonna round it!\n")

#define MSG_WARN_IDX1_FLOAT \
    M("Atenção na linha %d: tá vendo que o primeiro índice do array tá em ponto flutuante né? Vou arredondar!\n", \
      "Heads up on line %d: see how the first array index came out a float? Gonna round it!\n")

#define MSG_WARN_IDX2_FLOAT \
    M("Atenção na linha %d: tá vendo que o segundo índice do array tá em ponto flutuante né? Vou arredondar!\n", \
      "Heads up on line %d: see how the second array index came out a float? Gonna round it!\n")

#define MSG_WARN_IDXS_FLOAT \
    M("Atenção na linha %d: tá vendo que os índices do array estão em ponto flutuante né? Vou arredondar!\n", \
      "Heads up on line %d: see how the array indices came out floats? Gonna round 'em!\n")

#define MSG_WARN_IDX_COMP_ROUND \
    M("Atenção na linha %d: índice de array complexo? Sério?! Vou arredondar a parte real.\n", \
      "Heads up on line %d: a complex array index? Seriously?! Gonna round the real part.\n")

#define MSG_WARN_IDX_COMP_GRAB \
    M("Atenção na linha %d: índice de array complexo? Sério?! Vou pegar a parte real.\n", \
      "Heads up on line %d: a complex array index? Seriously?! Gonna grab the real part.\n")

#define MSG_WARN_IDXS_MESS \
    M("Atenção na linha %d: esses índices do array estão uma bagunça. você é uma pessoa confusa!\n", \
      "Heads up on line %d: these array indices are all over the place. You're a confused soul!\n")

#define MSG_WARN_IDX_FLOAT_HEAVY \
    M("Atenção na linha %d: índice de array tá dando float. Vai gerar muito código pra arredondar!\n", \
      "Heads up on line %d: array index is coming out float. Gonna generate a ton of code to round it!\n")

// unused-variable errors/warnings (variaveis.c) -----------------------------

#define MSG_WARN_UNUSED_GLOBAL_VAR \
    M("Atenção: variável global '%s' não está sendo usada. Economize memória!\n", \
      "Heads up: global variable '%s' isn't being used. Save some memory, will ya!\n")

#define MSG_WARN_UNUSED_LOCAL_VAR \
    M("Atenção: variável '%s' na função '%s' não está sendo usada. Economize memória!\n", \
      "Heads up: variable '%s' in function '%s' isn't being used. Save some memory, will ya!\n")

#define MSG_WARN_UNUSED_FUNCTION \
    M("Atenção: função '%s' não está sendo usada. Economize memória!\n", \
      "Heads up: function '%s' isn't being used. Save some memory, will ya!\n")

// macros / interrupcao -------------------------------------------------------

#define MSG_ERR_NESTED_MACRO \
    M("Erro na linha %d: tá chamando uma macro dentro da outra. você é uma pessoa confusa!\n", \
      "Error on line %d: you're calling a macro inside another one. You're a confused soul, aren't you!\n")

#define MSG_ERR_MACRO_NOT_FOUND \
    M("Erro na linha %d: cadê a macro %s? Tinha que estar na pasta Software!\n", \
      "Error on line %d: where's the macro %s? Was supposed to be in the Software folder!\n")

#define MSG_ERR_MACRO_NO_START \
    M("Erro na linha %d: não estou achando o começo da macro\n", \
      "Error on line %d: can't find the start of the macro\n")

#define MSG_ERR_DUP_INTERRUPT \
    M("Erro na linha %d: já tem uma interrupção rolando em outro ponto antes desse!\n", \
      "Error on line %d: there's already an interrupt going on somewhere else before this one!\n")

// info messages -------------------------------------------------------------

#define MSG_INFO_INTERRUPT_DIRECTIVE \
    M("Info: diretiva de interrupção encontrada na linha %d\n", \
      "Info: interruption directive found at line %d\n")

#define MSG_INFO_INS_GENERATED \
    M("Info: %d instruções assembly geradas\n", \
      "Info: %d assembly instructions generated\n")

#define MSG_INFO_CONST_APPROX \
    M("Info: constante %s na linha %d aproximada para %.14f (erro = %.14f)\n", \
      "Info: constant %s on line %d aproximated to %.14f (error = %.14f)\n")

#define MSG_INFO_USER_MACRO \
    M("Info: substituindo código C± pela macro do usuário %s na linha %d\n", \
      "Info: replacing C± code by user macro %s at line %d\n")

#define MSG_INFO_SQRT_MACRO \
    M("Info: adicionando macro assembly para cálculo de raiz quadrada\n", \
      "Info: adding assembly macro for root square computation\n")

#define MSG_INFO_ATAN_MACRO \
    M("Info: adicionando macro assembly para cálculo de arco-tangente\n", \
      "Info: adding assembly macro for arc-tangent computation\n")

#define MSG_INFO_SIN_MACRO \
    M("Info: adicionando macro assembly para cálculo de seno\n", \
      "Info: adding assembly macro for sin computation\n")

#define MSG_INFO_ARRAY_FILE_INIT \
    M("Info: inicialização de array com arquivo %s para variável '%s' na linha %d\n", \
      "Info: array initialization with file %s for variable '%s' at line %d\n")

// Dirac-notation info messages (stdlib.c) -----------------------------------

#define MSG_INFO_DIRAC_INNER \
    M("Info: notação de Dirac para Produto Interno detectada na linha %d\n", \
      "Info: Dirac notation for Inner Product detected at line %d\n")

#define MSG_INFO_DIRAC_MV \
    M("Info: notação de Dirac para multiplicação Matriz-Vetor detectada na linha %d\n", \
      "Info: Dirac notation for Matrix-Vector multiplication detected at line %d\n")

#define MSG_INFO_DIRAC_CV \
    M("Info: notação de Dirac para multiplicação Constante-Vetor detectada na linha %d\n", \
      "Info: Dirac notation for Constant-Vector multiplication detected at line %d\n")

#define MSG_INFO_DIRAC_VECTOR_SUM \
    M("Info: notação de Dirac para Soma de Vetores detectada na linha %d\n", \
      "Info: Dirac notation for Vectors Sum detected at line %d\n")

#define MSG_INFO_DIRAC_OUTER \
    M("Info: notação de Dirac para Produto Externo detectada na linha %d\n", \
      "Info: Dirac notation for Outer Product detected at line %d\n")

#define MSG_INFO_DIRAC_MATRIX_SUM \
    M("Info: notação de Dirac para Soma de Matrizes detectada na linha %d\n", \
      "Info: Dirac notation for Matrix Sum detected at line %d\n")

#define MSG_INFO_DIRAC_CM \
    M("Info: notação de Dirac para multiplicação Constante-Matriz detectada na linha %d\n", \
      "Info: Dirac notation for Constant-Matrix multiplication detected at line %d\n")

#define MSG_INFO_DIRAC_ZERO_VECTOR \
    M("Info: notação de Dirac para zerar conteúdo de vetor detectada na linha %d\n", \
      "Info: Dirac notation for zeroing vector contents detected at line %d\n")

#define MSG_INFO_DIRAC_SET_VECTOR \
    M("Info: notação de Dirac para setar conteúdo de vetor detectada na linha %d\n", \
      "Info: Dirac notation for setting vector contents detected at line %d\n")

#define MSG_INFO_DIRAC_FLUSH_VECTOR \
    M("Info: notação de Dirac para flush de vetor detectada na linha %d\n", \
      "Info: Dirac notation for flushing vector contents detected at line %d\n")

#define MSG_INFO_DIRAC_SHIFT \
    M("Info: notação de Dirac para shift register no vetor %s detectada na linha %d\n", \
      "Info: Dirac notation for shift register in vector %s detected at line %d\n")

// sucessos ------------------------------------------------------------------

#define MSG_OK_CMM_DONE \
    M("Sucesso: compilou! Agora é só descobrir por que não funciona.\n", \
      "Sweet: it compiled! Now you just gotta figure out why it doesn't work.\n")

// command-line interface ----------------------------------------------------

#define MSG_ERR_CLI_NEEDS_VALUE \
    M("Erro: a opção '%s' precisa de um valor.\n", \
      "Error: option '%s' needs a value.\n")

#define MSG_ERR_CLI_UNKNOWN_OPT \
    M("Erro: opção desconhecida '%s'.\n", \
      "Error: unknown option '%s'.\n")

#define MSG_ERR_CLI_MISSING \
    M("Erro: faltou a opção obrigatória '%s'.\n", \
      "Error: missing required option '%s'.\n")

#define MSG_CLI_VERSION \
    M("cmmcomp (YANC) %s\n", \
      "cmmcomp (YANC) %s\n")

#define MSG_CLI_HELP \
    M("cmmcomp - compilador front-end C± do YANC (%s)\n" \
      "\n" \
      "Uso:\n" \
      "  cmmcomp [opções] -i <arq.cmm> -n <nome> -p <dir> -m <dir> -t <dir>\n" \
      "\n" \
      "Opções obrigatórias:\n" \
      "  -i, --input <arq>       arquivo-fonte C± (dentro de <dir-proc>/Software)\n" \
      "  -n, --name <nome>       nome do processador (base do .asm gerado)\n" \
      "  -p, --proc-dir <dir>    diretório do processador\n" \
      "  -m, --macros-dir <dir>  diretório de Macros\n" \
      "  -t, --temp-dir <dir>    diretório Temp\n" \
      "\n" \
      "Outras opções:\n" \
      "  -P, --project           modo projeto (array inicializado pela simulação)\n" \
      "  -en, -pt                idioma das mensagens (padrão: -pt)\n" \
      "  -h, --help              mostra esta ajuda e sai\n" \
      "  -V, --version           mostra a versão e sai\n", \
      "cmmcomp - YANC C± front-end compiler (%s)\n" \
      "\n" \
      "Usage:\n" \
      "  cmmcomp [options] -i <file.cmm> -n <name> -p <dir> -m <dir> -t <dir>\n" \
      "\n" \
      "Required options:\n" \
      "  -i, --input <file>      C± source file (inside <proc-dir>/Software)\n" \
      "  -n, --name <name>       processor name (base name of the .asm output)\n" \
      "  -p, --proc-dir <dir>    processor directory\n" \
      "  -m, --macros-dir <dir>  Macros directory\n" \
      "  -t, --temp-dir <dir>    Temp directory\n" \
      "\n" \
      "Other options:\n" \
      "  -P, --project           project mode (array initialised by simulation)\n" \
      "  -en, -pt                diagnostic message language (default: -pt)\n" \
      "  -h, --help              show this help and exit\n" \
      "  -V, --version           show version and exit\n")

#endif
