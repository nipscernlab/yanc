// ----------------------------------------------------------------------------
// CPPComp — diagnostics --------------------------------------------------------
// ----------------------------------------------------------------------------
// Every message a user sees is an MSG_ name below, in Portuguese and in English
// (AGENTS.md, rule 4 of the repository):
//   msg_error(line, MSG_XXX, args...);   or   fprintf(stderr, MSG_XXX, args...);
// The internal errors (msg_internal) stay in English only: they report a
// defect of cppcomp itself, read by whoever maintains it, not the program.
// ----------------------------------------------------------------------------

#ifndef CPPCOMP_MESSAGES_H
#define CPPCOMP_MESSAGES_H

#include "../../common/yanc_lang.h"     // yanc_lang_en, set by --lang and YANC_LANG

// the language of the messages: English unless --lang or YANC_LANG says pt
#define M(pt, en) (yanc_lang_en ? (en) : (pt))

void msg_set_file(const char *name);
void msg_error    (int line, const char *fmt, ...);    // prints and exits(1)
void msg_warning  (int line, const char *fmt, ...);
void msg_internal (const char *fmt, ...);              // compiler-internal bug; abort
int  msg_error_count(void);

// ============================================================================
// MESSAGE CATALOG ============================================================
// ============================================================================

// names in the assembly (codegen.c, claim) ------------------------------------

#define MSG_ERR_NAME_IS_MNEMONIC \
    M("%s: '%s' é uma instrução do assembly do processador; troque o nome", \
      "%s: '%s' is an instruction of the processor's assembly, rename it")

#define MSG_ERR_ASM_NAME_IS_MNEMONIC \
    M("%s vira '%s' no assembly, que é uma instrução do processador: troque o nome", \
      "%s is '%s' in the assembly, which is an instruction of the processor: rename it")

#define MSG_ERR_NAME_GLOBAL \
    M("%s: 'global' é o nome das variáveis globais no log da simulação; troque o nome da função", \
      "%s: 'global' names the global variables in the simulation log, rename the function")

#define MSG_ERR_NAME_RESERVED \
    M("%s: '%s' é reservado, é %s; troque o nome", \
      "%s: '%s' is reserved, it is %s; rename it")

#define MSG_ERR_NAME_CLASH \
    M("%s e %s viram os dois '%s' no assembly: troque o nome de um deles", \
      "%s and %s are both '%s' in the assembly: rename one of them")

// expressions and built-in calls (codegen.c) ----------------------------------

#define MSG_ERR_GENERIC_NO_MATCH \
    M("_Generic: nenhuma associação corresponde ao tipo de controle", \
      "_Generic: no association matches the controlling type")

#define MSG_ERR_UNDEFINED \
    M("'%s' não foi declarado", \
      "undefined '%s'")

#define MSG_ERR_SUBSCRIPT \
    M("índice aplicado a algo que não é array nem ponteiro", \
      "subscript on non-array/non-pointer")

#define MSG_ERR_UNDEFINED_FUNC \
    M("a função '%s' não foi declarada", \
      "undefined function '%s'")

#define MSG_ERR_DEREF \
    M("'*' aplicado a algo que não é ponteiro", \
      "deref on non-pointer")

#define MSG_ERR_DOT \
    M("'.' aplicado a algo que não é struct", \
      "'.' on non-struct")

#define MSG_ERR_NO_SUCH_FIELD \
    M("não existe o campo '%s'", \
      "no such field '%s'")

#define MSG_ERR_ARROW \
    M("'->' aplicado a algo que não é ponteiro para struct", \
      "'->' on non-pointer-to-struct")

#define MSG_ERR_NOT_LVALUE \
    M("a expressão não pode receber atribuição (não é um lvalue)", \
      "expression is not an lvalue")

#define MSG_ERR_NO_FIELD \
    M("não existe o campo '%s'", \
      "no field '%s'")

#define MSG_ERR_BAD_ARROW \
    M("alvo inválido para '->'", \
      "bad -> target")

#define MSG_ERR_LINK_NUMBER \
    M("%s(): o número do link vem primeiro e é um literal inteiro de 0 a %d", \
      "%s(): the link number comes first and is an integer literal from 0 to %d")

#define MSG_ERR_LINK_MODE \
    M("o link %d já foi usado %s _sync: um link tem um só hardware, o parceiro está no meu clock (as formas _sync em todo uso) ou não está (toma/cade/fcade em todo uso)", \
      "link %d was already used %s _sync: a link has one hardware, the partner is on my clock (the _sync forms in every use) or it is not (toma/cade/fcade in every use)")

#define MSG_ERR_CADE_ONE_WORD \
    M("cade() traz uma palavra: só pode ir para uma variável de uma palavra", \
      "cade() brings one word: it can only go into a one-word variable")

#define MSG_ERR_ASSIGN_CONST \
    M("atribuição à constante '%s'", \
      "assignment to const '%s'")

#define MSG_ERR_CTOR_NON_CLASS \
    M("T(args) com um tipo que não é classe", \
      "T(args) on non-class type")

#define MSG_ERR_METHOD_NON_CLASS \
    M("chamada de método em algo que não é objeto de uma classe", \
      "method call on non-class value")

#define MSG_ERR_NO_METHOD \
    M("não existe o método '%s' na classe '%s'", \
      "no method '%s' in class '%s'")

#define MSG_ERR_IN_ARGS \
    M("in(porta) recebe 1 argumento", \
      "in(port) takes 1 arg")

#define MSG_ERR_IN_PORT \
    M("a porta de in() tem que ser um literal inteiro", \
      "in() port must be a literal int")

#define MSG_ERR_OUT_ARGS \
    M("out(porta,valor) recebe 2 argumentos", \
      "out(port,val) takes 2 args")

#define MSG_ERR_OUT_PORT \
    M("a porta de out() tem que ser um literal inteiro", \
      "out() port must be a literal int")

#define MSG_WARN_OUT_FLOAT \
    M("out() manda uma palavra inteira; este float sai como o seu padrão de bits cru (valor errado). Converta para int, por exemplo out(porta, (int)x)", \
      "out() sends an integer word; this float is output as its raw bit pattern (wrong value) -- cast to int, e.g. out(port, (int)x)")

#define MSG_ERR_LINK_ARGS2 \
    M("%s(link, valor) recebe 2 argumentos", \
      "%s(link, value) takes 2 args")

#define MSG_ERR_TOMA_ONE_WORD \
    M("toma() leva uma palavra: este valor ocupa %d", \
      "toma() carries one word: this value takes %d")

#define MSG_ERR_LINK_ARGS1 \
    M("%s(link) recebe 1 argumento, o número do link", \
      "%s(link) takes 1 arg, the link number")

#define MSG_ERR_MALLOC_ARGS \
    M("malloc(npalavras) recebe 1 argumento", \
      "malloc(nwords) takes 1 arg")

#define MSG_ERR_FREE_ARGS \
    M("free(ptr) recebe 1 argumento", \
      "free(ptr) takes 1 arg")

#define MSG_ERR_ZEROFILL_ARGS \
    M("__zerofill recebe 2 argumentos", \
      "__zerofill takes 2 args")

// initializers and statements (codegen.c) -------------------------------------

#define MSG_ERR_FIELD_DESIG_IN_ARRAY \
    M("designador de campo na inicialização de um array", \
      "field designator in array initializer")

#define MSG_ERR_ARRAY_DESIG_IN_STRUCT \
    M("designador de índice na inicialização de uma struct", \
      "array designator in struct initializer")

#define MSG_ERR_INIT_NO_FIELD \
    M("a inicialização cita o campo '%s', que não existe", \
      "no field '%s' in initializer")

#define MSG_ERR_TOO_MANY_INIT \
    M("valores demais na inicialização", \
      "too many initializers")

#define MSG_ERR_RECURSIVE_AGGREGATE \
    M("array ou struct local numa função recursiva não é suportado", \
      "array/struct local in a recursive function is not supported")

#define MSG_ERR_BREAK_OUTSIDE \
    M("break fora de laço ou switch", \
      "break outside of loop/switch")

#define MSG_ERR_CONTINUE_OUTSIDE \
    M("continue fora de laço", \
      "continue outside of loop")

#define MSG_ERR_RETURN_IN_VOID \
    M("return com valor numa função void", \
      "return value in void function")

#define MSG_ERR_CASE_OUTSIDE \
    M("case/default fora de switch", \
      "case/default outside switch")

#define MSG_ERR_TABLE_OPEN \
    M("cppcomp: não foi possível abrir %s\n", \
      "cppcomp: cannot open %s\n")

#define MSG_ERR_NO_MAIN \
    M("o programa não tem a função main()", \
      "program has no main() function")

// declarations (CPPComp.y) ----------------------------------------------------

#define MSG_WARN_BITS_IGNORED \
    M("'%s' pede %d bits, mas uma palavra do YANC tem %d: o tamanho pedido é ignorado e '%s' tem %d bits (%s)", \
      "'%s' asks for %d bits, but a YANC word has %d: the requested size is ignored and '%s' is a %d-bit %s")

#define MSG_ERR_LAMBDA_GLOBAL \
    M("uma lambda só é aceita como inicialização de uma global (chamada na hora, sem capturas)", \
      "a lambda is only supported as a global initializer (immediately-invoked, no captures)")

#define MSG_ERR_LAMBDA_RETURN \
    M("não dá para deduzir o tipo de retorno da lambda (retorne uma variável local ou um literal)", \
      "cannot deduce the lambda's return type (return a local variable or a literal)")

#define MSG_ERR_RANGE_FOR \
    M("for de intervalo sobre '%s': ele aceita um array ou um objeto com size() e operator[]", \
      "range-for over '%s': it takes an array or an object with size() and operator[]")

#define MSG_ERR_STATIC_ASSERT_CONST \
    M("_Static_assert exige uma expressão inteira constante", \
      "_Static_assert requires a constant integer expression")

#define MSG_ERR_STATIC_ASSERT_FAILED \
    M("a asserção estática falhou%s%s", \
      "static assertion failed%s%s")

#define MSG_ERR_REDEF_GLOBAL \
    M("a global '%s' foi definida de novo (se vem de um header, proteja-o com #ifndef ou #pragma once)", \
      "redefinition of global '%s' (if this comes from a header, guard it with #ifndef or #pragma once)")

#define MSG_ERR_UNKNOWN_TYPE \
    M("tipo desconhecido '%s'", \
      "unknown type '%s'")

#define MSG_ERR_NOT_CLASS_TEMPLATE \
    M("'%s' não é um template de classe", \
      "'%s' is not a class template")

#define MSG_ERR_UNKNOWN_BASE \
    M("classe base desconhecida '%s'", \
      "unknown base class '%s'")

#define MSG_ERR_MEMBER_ONE_INIT \
    M("o membro '%s' recebe um só valor de inicialização", \
      "member '%s' takes one initializer")

#define MSG_ERR_FIELD_SIZE_CONST \
    M("o tamanho de um campo array tem que ser constante", \
      "array field size must be constant")

#define MSG_ERR_ARRAY_SIZE_CONST \
    M("o tamanho do array tem que ser uma expressão constante", \
      "array size must be a constant expression")

#define MSG_ERR_TEMPLATE_ARG_CONST \
    M("o argumento do template tem que ser uma constante", \
      "template argument must be a constant")

#define MSG_ERR_TYPEDEF_INIT \
    M("typedef não pode ter valor de inicialização", \
      "typedef cannot have an initializer")

#define MSG_ERR_FPTR_ARRAY_INIT \
    M("um array de ponteiros para função é inicializado com { ... }", \
      "an array of function pointers takes a { ... } initializer")

#define MSG_ERR_REDECL_AS_FUNC \
    M("'%s' declarado de novo como função (já é uma variável)", \
      "'%s' redeclared as a function (already a variable)")

#define MSG_ERR_REDEF_FUNC \
    M("a função '%s' foi definida de novo%s", \
      "redefinition of function '%s'%s")

#define MSG_ERR_OUT_OF_CLASS_UNKNOWN \
    M("definição fora da classe para a classe desconhecida '%s'", \
      "out-of-class definition for unknown class '%s'")

#define MSG_ERR_NOT_CTOR \
    M("o nome qualificado '%s::%s' não é um construtor", \
      "qualified id '%s::%s' is not a constructor")

#define MSG_ERR_OUT_OF_CLASS_CTOR \
    M("construtor fora da classe para a classe desconhecida '%s'", \
      "out-of-class ctor for unknown class '%s'")

#define MSG_ERR_NOT_DTOR \
    M("o nome qualificado '%s::~%s' não é um destrutor", \
      "qualified id '%s::~%s' is not a destructor")

#define MSG_ERR_OUT_OF_CLASS_DTOR \
    M("destrutor fora da classe para a classe desconhecida '%s'", \
      "out-of-class dtor for unknown class '%s'")

// the lexer (CPPComp.l) -------------------------------------------------------

#define MSG_ERR_RESERVED_DUNDER \
    M("'%s': um nome com '__' é reservado (o C++ os guarda para o compilador, e o YANC os usa no assembly)", \
      "'%s': a name with '__' in it is reserved (C++ keeps them for the compiler, and YANC uses them in the assembly)")

#define MSG_ERR_PRAGMA_PRNAME \
    M("#pragma yanc prname não existe mais: o processador leva o nome do arquivo (filtro.cpp vira o processador filtro); apague a linha e, para outro nome, renomeie o arquivo", \
      "#pragma yanc prname is gone: the processor takes the name of the file (filtro.cpp becomes the processor filtro); delete the line and, for another name, rename the file")

#define MSG_ERR_PRAGMA_FFTSIZ \
    M("#pragma yanc fftsiz não faz nada em C++ (o endereçamento bit-reverso é só do C±); apague a linha", \
      "#pragma yanc fftsiz does nothing in C++ (the bit-reversed addressing is C± only); delete the line")

#define MSG_ERR_PRAGMA_ITRADD \
    M("#pragma yanc itradd não faz nada em C++ (interrupção é só do C±); apague a linha", \
      "#pragma yanc itradd does nothing in C++ (interrupts are C± only); delete the line")

#define MSG_WARN_PRAGMA_UNKNOWN \
    M("ajuste desconhecido de #pragma yanc: '%s'", \
      "unknown #pragma yanc setting '%s'")

#define MSG_ERR_OCTAL \
    M("'%s': um número que começa com 0 é octal, e não tem 8 nem 9", \
      "'%s': a number starting with 0 is octal, and has no 8 or 9")

#define MSG_ERR_BAD_CHAR \
    M("caractere inesperado '%c' (0x%02x)", \
      "unexpected character '%c' (0x%02x)")

// the command line and the files (main.c) -------------------------------------

#define MSG_ERR_UNKNOWN_OPTION \
    M("cppcomp: opção desconhecida '%s'\n", \
      "cppcomp: unknown option '%s'\n")

#define MSG_ERR_FILE_NAME \
    M("cppcomp: o arquivo '%s' dá nome ao processador, e '%s' %s; renomeie o arquivo\n", \
      "cppcomp: the file '%s' names the processor, and '%s' %s; rename the file\n")

#define MSG_ERR_PARSE_FAILED \
    M("cppcomp: a análise do programa falhou\n", \
      "cppcomp: parse failed\n")

#define MSG_ERR_CANT_WRITE \
    M("cppcomp: não foi possível abrir '%s' para escrita\n", \
      "cppcomp: cannot open '%s' for writing\n")

// the preprocessor (cpppp.c) --------------------------------------------------

#define MSG_ERR_INCLUDE_DEEP \
    M("cppcomp: #include aninhado fundo demais\n", \
      "cppcomp: include too deep\n")

#define MSG_ERR_CANT_OPEN \
    M("cppcomp: não foi possível abrir '%s'\n", \
      "cppcomp: cannot open '%s'\n")

#define MSG_ERR_INCLUDE_QUOTE \
    M("cppcomp: não achei o #include \"%s\"\n", \
      "cppcomp: can't find #include \"%s\"\n")

#define MSG_ERR_INCLUDE_ANGLE \
    M("cppcomp: não achei o #include <%s>\n", \
      "cppcomp: can't find #include <%s>\n")

#define MSG_ERR_IFDEF_DEEP \
    M("cppcomp: #ifdef aninhado fundo demais\n", \
      "cppcomp: #ifdef nesting too deep\n")

#define MSG_ERR_IF_DEEP \
    M("cppcomp: #if aninhado fundo demais\n", \
      "cppcomp: #if nesting too deep\n")

#define MSG_ERR_STRAY_ELIF \
    M("cppcomp: #elif sem #if\n", \
      "cppcomp: stray #elif\n")

#define MSG_ERR_STRAY_ELSE \
    M("cppcomp: #else sem #if\n", \
      "cppcomp: stray #else\n")

#define MSG_ERR_STRAY_ENDIF \
    M("cppcomp: #endif sem #if\n", \
      "cppcomp: stray #endif\n")

#define MSG_ERR_TOO_MANY_I \
    M("cppcomp: pastas -I demais\n", \
      "cppcomp: too many -I dirs\n")

// pieces put into the messages above: who owns a name in the assembly ---------

#define MSG_WHO_LOCAL \
    M("a variável local '%s' de '%s'", \
      "the local '%s' of '%s'")

#define MSG_WHO_FUNC \
    M("a função '%s'", \
      "the function '%s'")

#define MSG_WHO_OVERLOAD \
    M("uma sobrecarga de '%s'", \
      "an overload of '%s'")

#define MSG_WHO_GLOBAL \
    M("a variável global '%s'", \
      "the global '%s'")

#define MSG_WHO_MALLOC \
    M("a rotina de alocação do heap", \
      "the heap's allocation routine")

#define MSG_WHO_FREE \
    M("a rotina de liberação do heap", \
      "the heap's release routine")

#define MSG_WHO_FIM \
    M("o rótulo onde o programa termina", \
      "the label where the program ends")

// pieces: single words and clauses --------------------------------------------

#define MSG_WORD_WITH \
    M("com", \
      "with")

#define MSG_WORD_WITHOUT \
    M("sem", \
      "without")

#define MSG_WORD_FLOAT \
    M("float", \
      "float")

#define MSG_WORD_INTEGER \
    M("inteiro", \
      "integer")

#define MSG_WORD_ONE_MAIN \
    M(" (um programa tem exatamente um main)", \
      " (a program has exactly one main)")

// pieces: why a file name cannot name the processor (main.c) ------------------

#define MSG_WHY_CHARS \
    M("só pode ter letras, dígitos e '_'", \
      "can only have letters, digits and '_'")

#define MSG_WHY_DIGIT \
    M("não pode começar com dígito", \
      "cannot start with a digit")

#define MSG_WHY_DUNDER \
    M("não pode começar com '__' (os nomes do compilador)", \
      "cannot start with '__' (the compiler's names)")

#define MSG_WHY_KEYWORD \
    M("é uma palavra reservada do Verilog", \
      "is a Verilog keyword")

#define MSG_WORD_ERROR \
    M("erro", \
      "error")

#define MSG_WORD_WARNING \
    M("atenção", \
      "warning")

// bison's two texts (yyerror) -------------------------------------------------

#define MSG_ERR_SYNTAX \
    M("erro de sintaxe", \
      "syntax error")

#define MSG_ERR_PARSER_MEMORY \
    M("o analisador ficou sem memória", \
      "memory exhausted")

// the prefix after file:line
#define MSG_PREFIX_ERROR   M("erro: ",    "error: ")
#define MSG_PREFIX_WARNING M("atenção: ", "warning: ")

// -h / --help
#define MSG_CLI_USAGE \
    M("cppcomp %s — compilador C++ para o processador SAPHO\n" \
      "uso: cppcomp [opções] <arquivo.cpp>\n" \
      "  <arquivo.cpp>           o fonte; o processador leva o nome dele\n" \
      "  -o, --output   <pasta>  pasta de saída (padrão: ./<nome>): <nome>.asm e\n" \
      "                          as tabelas que o asmcomp e a forma de onda leem\n" \
      "  -I <pasta>              acrescenta uma pasta de #include (pode repetir); a\n" \
      "                          pasta do fonte vem primeiro, a Header ao lado de bin/ por último\n" \
      "  -E                      imprime o fonte pré-processado e para\n" \
      "  --lang=en|pt            a língua das mensagens (também YANC_LANG); padrão en\n" \
      "  -h, --help              esta ajuda\n" \
      "  -V, --version           mostra a versão e sai\n", \
      "cppcomp %s — C++ compiler for the SAPHO processor\n" \
      "usage: cppcomp [options] <file.cpp>\n" \
      "  <file.cpp>              the source; the processor takes its name\n" \
      "  -o, --output   <dir>    output folder (default: ./<name>): <name>.asm and\n" \
      "                          the tables asmcomp and the waveform read\n" \
      "  -I <dir>                add an #include folder (repeatable); the source's\n" \
      "                          folder comes first, the Header folder next to bin/ last\n" \
      "  -E                      print the preprocessed source and stop\n" \
      "  --lang=en|pt            the language of the messages (also YANC_LANG); default en\n" \
      "  -h, --help              this help\n" \
      "  -V, --version           show version and exit\n")

#endif
