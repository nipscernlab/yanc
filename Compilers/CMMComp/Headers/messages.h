// ----------------------------------------------------------------------------
// bilingual PT/EN support for cmmcomp messages -------------------------------
// ----------------------------------------------------------------------------
// usage:
//   printf(MSG_XXX, args...);   or   fprintf(stderr, MSG_XXX, args...);
// to see both string versions, search for MSG_XXX below.
// ----------------------------------------------------------------------------

#ifndef MESSAGES_H
#define MESSAGES_H

// 1 = english (default), 0 = portuguese
extern int lang_en;

// string-selection macro. takes PT and EN and chooses based on lang_en
#define M(pt, en) (lang_en ? (en) : (pt))

// --lang=en|pt (and YANC_LANG): sets lang_en, takes the option out of argv
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

#define MSG_ERR_CANT_OPEN_FILE \
    M("Erro: não rolou de abrir o arquivo '%s'!!\n", \
      "Error: couldn't open the file '%s' for the life of me!!\n")

// variable / number errors --------------------------------------------------

#define MSG_ERR_RESERVED_I \
    M("Erro na linha %d: símbolo 'i' é reservado para indicar a parte imaginária de uma constante complexa.\n", \
      "Error on line %d: the symbol 'i' is reserved for the imaginary part of a complex constant. Hands off!\n")

#define MSG_ERR_NAME_TOO_LONG \
    M("Erro na linha %d: esse nome é comprido demais, o máximo é %d caracteres.\n", \
      "Error on line %d: that name is way too long, %d characters tops.\n")

#define MSG_ERR_NUGAIN_POW2 \
    M("Erro na linha %d: #NUGAIN %d não é potência de dois. norm(x) é x / NUGAIN em hardware: potência de dois vira um deslocamento; qualquer outro valor vira um divisor, o caminho crítico da ALU inteira.\n", \
      "Error on line %d: #NUGAIN %d is not a power of two. norm(x) is x / NUGAIN in hardware: a power of two becomes a shift; anything else becomes a divider, the critical path of the whole ALU.\n")

#define MSG_ERR_NUGAIN_POW2 \
    M("Erro na linha %d: #NUGAIN %d não é potência de dois. norm(x) é x / NUGAIN em hardware: potência de dois vira um deslocamento; qualquer outro valor vira um divisor, o caminho crítico da ALU inteira.\n", \
      "Error on line %d: #NUGAIN %d is not a power of two. norm(x) is x / NUGAIN in hardware: a power of two becomes a shift; anything else becomes a divider, the critical path of the whole ALU.\n")

#define MSG_ERR_FROUND_RANGE \
    M("Erro na linha %d: #FROUND só aceita 0 (legado), 1 (truncamento + saturação) ou 2 (arredondamento ao par).\n", \
      "Error on line %d: #FROUND only takes 0 (legacy), 1 (truncation + saturation) or 2 (round to nearest even).\n")

#define MSG_ERR_INT_MAX_OVERFLOW \
    M("Erro na linha %d: o maior número inteiro que pode ser representado é %d!\n", \
      "Error on line %d: the biggest integer this thing can hold is %d!\n")

#define MSG_WARN_ARR_SIZE_WORD \
    M("Atenção na linha %d: o array tem %d posições, mas um índice int de %d bits só chega até %d.\n", \
      "Heads up on line %d: the array has %d positions, but an int index of %d bits only reaches %d.\n")

#define MSG_ERR_OCTAL_DIGIT \
    M("Erro na linha %d: '%s' começa com 0, então é octal, e octal não tem 8 nem 9.\n", \
      "Error on line %d: '%s' starts with 0, so it is octal, and octal has no 8 or 9.\n")

#define MSG_ERR_FLOAT_MIN \
    M("Erro na linha %d: o menor número float que pode ser representado é %g!\n", \
      "Error on line %d: the smallest float this thing can hold is %g!\n")

#define MSG_ERR_FLOAT_MAX \
    M("Erro na linha %d: o maior número float que pode ser representado é %g!\n", \
      "Error on line %d: the biggest float this thing can hold is %g!\n")

// declaration / assignment / variable-use errors ---------------------------

#define MSG_ERR_VAR_EXISTS \
    M("Erro na linha %d: a variável '%s' já existe. Vai tomar um Ω³!\n", \
      "Error on line %d: the variable '%s' already exists. Go grab an Ω³!\n")

// a declared name, as the .asm spells it, already belongs to something else,
// or the assembler would read it as an instruction (check_name, variaveis.c)
#define MSG_ERR_NAME_TAKEN \
    M("Erro na linha %d: %s vira '%s' no assembly, e esse nome já é %s. Troque um dos dois nomes.\n", \
      "Error on line %d: %s is '%s' in the assembly, and that name is already %s. Rename one of them.\n")

#define MSG_ERR_NAME_MNEMONIC \
    M("Erro na linha %d: %s vira '%s' no assembly, que é uma instrução do processador. Troque o nome.\n", \
      "Error on line %d: %s is '%s' in the assembly, which is an instruction of the processor. Rename it.\n")

#define MSG_ERR_NAME_IS_MNEMONIC \
    M("Erro na linha %d: %s: '%s' é uma instrução do processador. Troque o nome.\n", \
      "Error on line %d: %s: '%s' is an instruction of the processor. Rename it.\n")

#define MSG_ERR_NAME_RESERVED \
    M("Erro na linha %d: a função não pode se chamar '%s': %s.\n", \
      "Error on line %d: a function can't be called '%s': %s.\n")

#define MSG_ERR_NAME_UNDERSCORES \
    M("Erro na linha %d: '%s': nomes começando com '__' são do compilador.\n", \
      "Error on line %d: '%s': names starting with '__' belong to the compiler.\n")

#define MSG_NAME_GLOBAL     M("a global '%s'",              "the global '%s'")
#define MSG_NAME_LOCAL      M("a local '%s' de '%s()'",     "the local '%s' of '%s()'")
#define MSG_NAME_FUNC       M("a função '%s()'",            "the function '%s()'")
#define MSG_NAME_IMAG       M("a parte imaginária de %s",   "the imaginary part of %s")
#define MSG_NAME_FIM        M("o programa termina no rótulo @fim", "the program ends at the label @fim")
#define MSG_NAME_GLOBAL_LOG M("'global' marca as variáveis globais no log da simulação", \
                              "'global' marks the global variables in the simulation log")
#define MSG_NAME_NUM_INS    M("'num_ins' é uma chave do log da simulação", \
                              "'num_ins' is a key of the simulation log")

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

#define MSG_ERR_PRNAME_GONE \
    M("Erro na linha %d: #PRNAME não existe mais. O processador tem o nome do arquivo (filtro.cmm vira o processador filtro): apague a linha e, se quiser outro nome, renomeie o arquivo.\n", \
      "Error on line %d: #PRNAME is gone. The processor takes the name of the file (filtro.cmm becomes the processor filtro): delete the line and, for another name, rename the file.\n")

#define MSG_ERR_LINK_NUM \
    M("Erro na linha %d: o número do link é um inteiro de 0 a %d, e achei %s.\n", \
      "Error on line %d: the link number is an integer from 0 to %d, and I found %s.\n")

#define MSG_ERR_LINK_MIX \
    M("Erro na linha %d: o link %d já foi usado %s _sync. Um link tem um hardware só: ou o parceiro está no mesmo clock (toma_sync/cade_sync/fcade_sync em todo uso), ou não (toma/cade/fcade em todo uso).\n", \
      "Error on line %d: link %d was already used %s _sync. A link has one hardware: either the partner is on the same clock (toma_sync/cade_sync/fcade_sync in every use) or it is not (toma/cade/fcade in every use).\n")
#define MSG_LINK_WITH    M("com", "with")
#define MSG_LINK_WITHOUT M("sem", "without")

#define MSG_ERR_CADE_COMP \
    M("Erro na linha %d: cade() traz uma palavra só, e '%s' é complexo (duas palavras). Receba a parte real e a imaginária separadas.\n", \
      "Error on line %d: cade() brings one word, and '%s' is complex (two words). Take the real and the imaginary parts separately.\n")

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

#define MSG_ERR_FUNC_COMPLEX \
    M("Erro na linha %d: %s(.) não aceita comp.\n", \
      "Error on line %d: %s(.) doesn't take a comp.\n")

#define MSG_ERR_POW_COMPLEX \
    M("Erro na linha %d: elevar número complexo a uma potência? Tá viajando! Não fiz isso ainda, se vira!\n", \
      "Error on line %d: a complex number raised to a power? Dream on! Didn't build that yet, you're on your own!\n")

#define MSG_WARN_CONJ_REAL \
    M("Atenção na linha %d: conj de número real? É ele mesmo com zero imaginário, mas tá valendo!\n", \
      "Warning on line %d: conj of a real number? It's just itself with a zero imaginary part, but ok!\n")

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

#define MSG_ERR_NOT_IMPL_COMPLEX \
    M("Erro na linha %d: não implementei isso pra números complexos ainda. Se vira!\n", \
      "Error on line %d: haven't coded this up for complex numbers yet. You're on your own!\n")

#define MSG_ERR_NOT_A_VECTOR \
    M("Erro na linha %d: '%s' nem vetor é, abensoado!\n", \
      "Error on line %d: '%s' ain't even a vector, mate!\n")

#define MSG_ERR_NOT_A_MATRIX \
    M("Erro na linha %d: '%s' nem matriz é, abensoado!\n", \
      "Error on line %d: '%s' ain't even a matrix, mate!\n")

#define MSG_ERR_SHIFT_VEC_SELF \
    M("Erro na linha %d: só dá pra fazer shift de um vetor nele mesmo, abensoado!\n", \
      "Error on line %d: you can only shift a vector into itself, mate!\n")

// the shapes of the Dirac notation, as the messages below print them
#define MSG_SHAPE_NUMBER     M("um número", "a number")
#define MSG_SHAPE_VECTOR     M("um vetor de %d", "a vector of %d")
#define MSG_SHAPE_ROW        M("uma linha de %d", "a row of %d")
#define MSG_SHAPE_MATRIX     M("uma matriz %dx%d", "a %dx%d matrix")
#define MSG_SHAPE_ANY_VECTOR M("um vetor", "a vector")
#define MSG_SHAPE_ANY_MATRIX M("uma matriz", "a matrix")

#define MSG_ERR_DIRAC_SUM_SHAPE \
    M("Erro na linha %d: somar %s com %s? Não encaixa, as formas têm que ser iguais, abensoado!\n", \
      "Error on line %d: adding %s and %s? Those shapes don't fit, mate!\n")

#define MSG_ERR_DIRAC_PRODUCT_SHAPE \
    M("Erro na linha %d: %s vezes %s não rola: as colunas do primeiro têm que bater com as linhas do segundo!\n", \
      "Error on line %d: %s times %s? No way: the columns of the first must match the rows of the second!\n")

#define MSG_ERR_DIRAC_TARGET_SHAPE \
    M("Erro na linha %d: '%s' é %s e a expressão dá %s. Não cabe, né?\n", \
      "Error on line %d: '%s' is %s and the expression gives %s. That won't fit, pal!\n")

#define MSG_ERR_DIRAC_NOT_SCALAR \
    M("Erro na linha %d: aqui eu queria um número e você me deu %s!\n", \
      "Error on line %d: I wanted a number here and you gave me %s!\n")

#define MSG_ERR_DIRAC_OUT_MATRIX \
    M("Erro na linha %d: out() manda um número ou um vetor, %s inteira não dá!\n", \
      "Error on line %d: out() sends a number or a vector, not %s. Slice it up first!\n")

#define MSG_INFO_DIRAC_TEMP \
    M("Info na linha %d: '%s' é lido numa posição e escrito em outra, então o resultado passa pela área temporária (%d palavras).\n", \
      "Info on line %d: '%s' is read at one spot and written at another, so the result takes a detour through the temporary area (%d words).\n")

#define MSG_INFO_DIRAC_TEMP_CHAIN \
    M("Info na linha %d: o produto do meio da cadeia vai pra área temporária (%d palavras), senão seria calculado de novo a cada elemento.\n", \
      "Info on line %d: the product in the middle of the chain goes to the temporary area (%d words), or it'd be computed all over again for every element.\n")

#define MSG_INFO_DIRAC_TEMP_TOTAL \
    M("Info: a área temporária do Dirac (__dirac_tmp) ocupa %d palavras da memória de dados. Pra economizar: escreva o resultado num vetor que não aparece do lado direito, ou calcule o produto do meio num vetor seu, numa linha antes. Quem usou:\n", \
      "Info: the Dirac temporary area (__dirac_tmp) takes %d words of data memory. To save them: write the result to an array that isn't on the right-hand side, or compute the middle product into an array of your own, on a line before. Who used it:\n")

#define MSG_INFO_DIRAC_TEMP_LINE \
    M("      linha %d: %d palavras\n", \
      "      line %d: %d words\n")

#define MSG_ERR_DIRAC_NOT_YET \
    M("Erro na linha %d: essa combinação de Dirac ainda tá no forno. Aguenta aí!\n", \
      "Error on line %d: this Dirac combo is still in the oven. Hang tight!\n")

// operator errors and warnings (oper.c) -------------------------------------

#define MSG_ERR_CMP_COMP_ORDER \
    M("Erro na linha %d: número complexo não tem ordem, então <, >, <= e >= não valem com comp. Compara mod2(z), real(z) ou imag(z).\n", \
      "Error on line %d: complex numbers have no order, so <, >, <= and >= don't work with a comp. Compare mod2(z), real(z) or imag(z) instead.\n")

#define MSG_ERR_MOD_NON_INT \
    M("Erro na linha %d: qual o sentido de calcular o resto da divisão sem ser com número inteiro? Vai se tratar!\n", \
      "Error on line %d: what's the point of computing a remainder with non-integer numbers? Get a grip!\n")

#define MSG_ERR_LOGIC_NON_INT \
    M("Erro na linha %d: operação lógica com um comp (só int e float)!\n", \
      "Error on line %d: logical operation with a comp (int and float only)!\n")

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

#define MSG_WARN_ARRAY_INI_LOCAL \
    M("Atenção na linha %d: o array local '%s' recebe os valores entre chaves uma vez só, quando a memória é carregada; de uma chamada para outra ele guarda o que a função escreveu nele (como um static em C).\n", \
      "Heads up on line %d: local array '%s' gets the values between braces only once, when the memory is loaded; from one call to the next it keeps what the function wrote into it (like a C static).\n")

#define MSG_ERR_ARRAY_INI_TYPE \
    M("Erro na linha %d: o array '%s' é de %s e não recebe o valor %s entre chaves.\n", \
      "Error on line %d: array '%s' holds %s and does not take the value %s between braces.\n")

#define MSG_ERR_ARRAY_INI_COUNT \
    M("Erro na linha %d: o array '%s' tem %d posições e a lista entre chaves vai até a posição %d.\n", \
      "Error on line %d: array '%s' has %d positions and the list between braces reaches position %d.\n")

#define MSG_ERR_ARRAY_INI_1D \
    M("Erro na linha %d: o array '%s' tem uma dimensão; chaves dentro da lista só separam as linhas de um array 2D.\n", \
      "Error on line %d: array '%s' has one dimension; braces inside the list only mark the rows of a 2D array.\n")

#define MSG_ERR_ARRAY_INI_ROWS \
    M("Erro na linha %d: no array '%s', as chaves de dentro separam as linhas de um array 2D: todas as linhas entre chaves, ou nenhuma, cada uma com até %d valores.\n", \
      "Error on line %d: in array '%s', inner braces mark the rows of a 2D array: every row between braces, or none, each with up to %d values.\n")

#define MSG_WARN_ARRAY_FILE_LOCAL \
    M("Atenção na linha %d: o array local '%s' é inicializado pelo arquivo %s uma vez só, quando a memória é carregada; de uma chamada para outra ele guarda o que a função escreveu nele (como um static em C).\n", \
      "Heads up on line %d: local array '%s' is initialized from file %s only once, when the memory is loaded; from one call to the next it keeps what the function wrote into it (like a C static).\n")

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
    M("Atenção na linha %d: nessa conversão, eu vou truncar a parte real hein!\n", \
      "Heads up on line %d: in this conversion I'm truncating the real part, just so you know!\n")

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
    M("Erro na função %s: tem caminho que chega no fim sem passar por um return, e aí a execução cai direto na função seguinte. Bota um return no fim.\n", \
      "Error in function %s: some path gets to the end without a return, and then it'd run straight into the next function. Put a return at the end.\n")

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

#define MSG_ERR_BREAK_LOST \
    M("Erro na linha %d: esse brake aí tá perdido!\n", \
      "Error on line %d: that break is just hanging out lost!\n")

#define MSG_ERR_CONTINUE_LOST \
    M("Erro na linha %d: continue fora de um loop não vai a lugar nenhum!\n", \
      "Error on line %d: a continue outside a loop goes nowhere!\n")

#define MSG_WARN_CASE_FLOAT \
    M("Atenção na linha %d: índice do case dando float! Vou truncar.\n", \
      "Heads up on line %d: case index came out float! Gonna truncate it.\n")

#define MSG_WARN_CASE_COMP \
    M("Atenção na linha %d: índice do case dando comp! Vou truncar a parte real.\n", \
      "Heads up on line %d: case index came out comp! Gonna truncate the real part.\n")

// array-index warnings (array_index.c) --------------------------------------

#define MSG_WARN_IDX_FLOAT \
    M("Atenção na linha %d: tá vendo que o índice do array tá em ponto flutuante né? Vou truncar!\n", \
      "Heads up on line %d: see how the array index came out a float? Gonna truncate it!\n")

#define MSG_WARN_IDX1_FLOAT \
    M("Atenção na linha %d: tá vendo que o primeiro índice do array tá em ponto flutuante né? Vou truncar!\n", \
      "Heads up on line %d: see how the first array index came out a float? Gonna truncate it!\n")

#define MSG_WARN_IDX2_FLOAT \
    M("Atenção na linha %d: tá vendo que o segundo índice do array tá em ponto flutuante né? Vou truncar!\n", \
      "Heads up on line %d: see how the second array index came out a float? Gonna truncate it!\n")

#define MSG_WARN_IDXS_FLOAT \
    M("Atenção na linha %d: tá vendo que os índices do array estão em ponto flutuante né? Vou truncar!\n", \
      "Heads up on line %d: see how the array indices came out floats? Gonna truncate 'em!\n")

#define MSG_WARN_IDX_COMP_ROUND \
    M("Atenção na linha %d: índice de array complexo? Sério?! Vou truncar a parte real.\n", \
      "Heads up on line %d: a complex array index? Seriously?! Gonna truncate the real part.\n")

#define MSG_WARN_IDX_COMP_GRAB \
    M("Atenção na linha %d: índice de array complexo? Sério?! Vou pegar a parte real.\n", \
      "Heads up on line %d: a complex array index? Seriously?! Gonna grab the real part.\n")

#define MSG_WARN_IDXS_MESS \
    M("Atenção na linha %d: esses índices do array estão uma bagunça. você é uma pessoa confusa!\n", \
      "Heads up on line %d: these array indices are all over the place. You're a confused soul!\n")

#define MSG_WARN_IDX_FLOAT_HEAVY \
    M("Atenção na linha %d: índice de array tá dando float. Vai gerar muito código pra truncar!\n", \
      "Heads up on line %d: array index is coming out float. Gonna generate a ton of code to truncate it!\n")

// unused-variable errors/warnings (variaveis.c) -----------------------------

// a variable never read is not in the program (asm_unread), nor in the waveform
#define MSG_WARN_UNUSED_GLOBAL_VAR \
    M("Atenção: variável global '%s' não é lida em lugar nenhum, então saiu do programa e da forma de onda (#WAVE %s a mantém).\n", \
      "Heads up: global variable '%s' is never read, so it's out of the program and the waveform (#WAVE %s keeps it).\n")

#define MSG_WARN_UNUSED_LOCAL_VAR \
    M("Atenção: variável '%s' na função '%s' não é lida em lugar nenhum, então saiu do programa e da forma de onda (#WAVE %s, dentro da função, a mantém).\n", \
      "Heads up: variable '%s' in function '%s' is never read, so it's out of the program and the waveform (#WAVE %s, in the function, keeps it).\n")

#define MSG_WARN_UNUSED_FUNCTION \
    M("Atenção: função '%s' não está sendo usada. Economize memória!\n", \
      "Heads up: function '%s' isn't being used. Save some memory, will ya!\n")

#define MSG_ERR_RECURSION \
    M("Erro na linha %d: a função '%s' chama ela mesma (%s). Recursão não rola em C±. Quer recursão mesmo? Usa o compilador C++ do SAPHO.\n", \
      "Error on line %d: function '%s' calls itself (%s). Recursion doesn't fly in C±. Really want recursion? Use SAPHO's C++ compiler.\n")

// interrupcao ----------------------------------------------------------------

#define MSG_ERR_DUP_INTERRUPT \
    M("Erro na linha %d: já tem uma interrupção rolando em outro ponto antes desse!\n", \
      "Error on line %d: there's already an interrupt going on somewhere else before this one!\n")

#define MSG_ERR_DUP_TOAQUI \
    M("Erro na linha %d: #TOAQUI repetido (só pode um)\n", \
      "Error on line %d: duplicate #TOAQUI marker (only one allowed)\n")

#define MSG_ERR_WAVE_NOT_VAR \
    M("Erro na linha %d: #WAVE pede uma variável, e '%s' não é uma\n", \
      "Error on line %d: #WAVE takes a variable, and '%s' isn't one\n")

#define MSG_INFO_TOAQUI \
    M("Info na linha %d: marcador #TOAQUI registrado (aciona o pino cheguei)\n", \
      "Info on line %d: #TOAQUI marker registered (drives the cheguei pin)\n")

// info messages -------------------------------------------------------------

#define MSG_INFO_INTERRUPT_DIRECTIVE \
    M("Info: diretiva de interrupção encontrada na linha %d\n", \
      "Info: interruption directive found at line %d\n")

#define MSG_INFO_INS_GENERATED \
    M("Info: %d instruções assembly geradas\n", \
      "Info: %d assembly instructions generated\n")

#define MSG_WARN_CONST_FLUSHED \
    M("Atenção na linha %d: a constante %s é menor que o menor float normal (%g) e vira 0 com #FROUND %d. Use #FROUND 0 ou aumente #NBEXPO.\n", \
      "Heads up on line %d: constant %s is below the smallest normal float (%g) and becomes 0 at #FROUND %d. Use #FROUND 0 or raise #NBEXPO.\n")

#define MSG_INFO_CONST_APPROX \
    M("Info: constante %s na linha %d aproximada para %.17g (erro = %.3g)\n", \
      "Info: constant %s on line %d approximated to %.17g (error = %.3g)\n")

#define MSG_INFO_SQRT_MACRO \
    M("Info: adicionando macro assembly para cálculo de raiz quadrada\n", \
      "Info: adding assembly macro for root square computation\n")

#define MSG_INFO_ATAN_MACRO \
    M("Info: adicionando macro assembly para cálculo de arco-tangente\n", \
      "Info: adding assembly macro for arc-tangent computation\n")

#define MSG_INFO_SIN_MACRO \
    M("Info: adicionando macro assembly para cálculo de seno\n", \
      "Info: adding assembly macro for sin computation\n")

#define MSG_INFO_TAN_MACRO \
    M("Info: adicionando macro assembly para cálculo de tangente\n", \
      "Info: adding assembly macro for tangent computation\n")

#define MSG_INFO_EXP_MACRO \
    M("Info: adicionando macro assembly para cálculo de exponencial\n", \
      "Info: adding assembly macro for exponential computation\n")

#define MSG_INFO_LOG_MACRO \
    M("Info: adicionando macro assembly para cálculo de logaritmo natural\n", \
      "Info: adding assembly macro for natural-logarithm computation\n")

#define MSG_INFO_ARRAY_FILE_INIT \
    M("Info: inicialização de array com arquivo %s para variável '%s' na linha %d\n", \
      "Info: array initialization with file %s for variable '%s' at line %d\n")

// Dirac-notation info messages (stdlib.c) -----------------------------------

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

#define MSG_ERR_PROC_NAME \
    M("Erro: o arquivo '%s' dá nome ao processador, e '%s' %s. Renomeia o arquivo.\n", \
      "Error: the file '%s' names the processor, and '%s' %s. Rename the file.\n")
#define MSG_PROC_NAME_CHARS   M("só pode ter letras, dígitos e '_'", "can only have letters, digits and '_'")
#define MSG_PROC_NAME_DIGIT   M("não pode começar com dígito", "can't start with a digit")
#define MSG_PROC_NAME_UNDERSC M("não pode começar com '__' (é do compilador)", "can't start with '__' (that's the compiler's)")
#define MSG_PROC_NAME_KEYWORD M("é palavra reservada do Verilog", "is a Verilog keyword")

#define MSG_ERR_NO_TOOL_DIR \
    M("Erro: cadê a pasta %s? Ela tem que estar ao lado da pasta bin/ do cmmcomp. Instala o YANC de novo.\n", \
      "Error: where's the %s folder? It has to sit next to cmmcomp's bin/ folder. Install YANC again.\n")

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
      "  cmmcomp [opções] <arq.cmm>\n" \
      "\n" \
      "O processador tem o nome do arquivo (<nome>.cmm). Escreve na pasta de saída o\n" \
      "<nome>.asm e as tabelas (prog_log.txt, pc_<nome>_mem.txt, src_lines.txt) que o\n" \
      "asmcomp e a forma de onda usam.\n" \
      "\n" \
      "Opções:\n" \
      "  -o, --output <dir>      pasta de saída (padrão: ./<nome>)\n" \
      "  --wave-arrays           cada elemento de vetor vira um sinal no GTKWave\n" \
      "  --lang=en|pt            idioma das mensagens (padrão: en, ou YANC_LANG)\n" \
      "  -h, --help              mostra esta ajuda e sai\n" \
      "  -V, --version           mostra a versão e sai\n", \
      "cmmcomp - YANC C± front-end compiler (%s)\n" \
      "\n" \
      "Usage:\n" \
      "  cmmcomp [options] <file.cmm>\n" \
      "\n" \
      "The processor takes the name of the file (<name>.cmm). Writes to the output\n" \
      "folder the <name>.asm and the tables (prog_log.txt, pc_<name>_mem.txt,\n" \
      "src_lines.txt) that asmcomp and the waveform use.\n" \
      "\n" \
      "Options:\n" \
      "  -o, --output <dir>      output folder (default: ./<name>)\n" \
      "  --wave-arrays           each array element as its own GTKWave signal\n" \
      "  --lang=en|pt            message language (default: en, or YANC_LANG)\n" \
      "  -h, --help              show this help and exit\n" \
      "  -V, --version           show version and exit\n")

#endif
