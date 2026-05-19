/*
    Features unique to C+-

    - Directive  #INTERPOINT    : marks the return point for a reset on the itr pin

    - comp data type (for complex numbers): e.g. comp a = 3+4i;

    - StdLib      in(.)  : reads external data
    - StdLib     fin(.)  : reads external data (converting to float)
    - StdLib     out(.,.): writes outside the processor
    - StdLib    fout(.,.): writes outside the processor (converting to float)
    - StdLib    norm(.)  : function that divides the argument by the constant given by #NUGAIN (avoids using the ALU divider)
    - StdLib  sign(.,.)  : returns the second argument with the sign of the first (saves a lot of code, try it out yourself)
    - StdLib    pset(.)  : function that returns zero if the argument is negative (avoids if(x<0) x = 0;)
    - StdLib     abs(.)  : function that returns the absolute value (avoids if(x<0) x = -x;). For complex numbers, returns the magnitude
    - StdLib    copy(.,.): copies the value of the first argument into the second (no type checking)
    - StdLib    sqrt(.)  : returns the square root. Produces a float
    - StdLib    atan(.)  : returns the arctangent. Produces a float
    - StdLib     sin(.)  : returns the sine    of a number
    - StdLib     cos(.)  : returns the cosine  of a number
    - StdLib    real(.)  : returns the real part of a complex number
    - StdLib    imag(.)  : returns the imag part of a complex number
    - StdLib    fase(.)  : returns the phase    of a complex number
    - StdLib    mod2(.)  : squared magnitude    of a complex number
    - StdLib complex(.,.): creates a complex number from two reals

    - Operator >>> : right shift with twos-complement (shift preserving the sign)

    - Array initializable from a file. The array memory is filled at compile time. (e.g. int x[128] "values.txt";)
    - Array with reversed index. Used in FFT (e.g. x[j) = exp;) the bits of i are reversed.

    - Statements in Dirac notation (linear algebra):
      <a|b>             -> returns the inner product between vectors a and b
      a # |M|b>;        -> vector a receives the product of matrix M and vector b
      a # c|b>;         -> vector a receives c times vector b
      d # |a> + c|b>;   -> vector d receives the sum of vector a with vector b weighted by c
      A # |a><b|;       -> matrix A receives the outer product between vectors a and b
      A # |P| - |a><b|; -> matrix A receives the subtraction of matrix P by the outer product between a and b
      A # c|B|;         -> matrix A receives the product of constant c and matrix B
      A # c|I|;         -> matrix A receives the identity matrix weighted by c
      a # |0>;          -> zeros every element of vector a
      a # c|in(x)>;     -> fills vector a with input from port x weighted by c
      out(x,c|a>);      -> OUT on vector a weighted by c
      a # c -> |a>;     -> shift register
*/

%{

#include <stdlib.h>

#include "..\Headers\itr.h"         // interrupt handling
#include "..\Headers\oper.h"        // ALU operations
#include "..\Headers\stdlib.h"      // SAPHO standard library
#include "..\Headers\saltos.h"      // jump management (if/else while)
#include "..\Headers\global.h"      // global variables and functions
#include "..\Headers\macros.h"      // assembler macros
#include "..\Headers\funcoes.h"     // function creation and usage
#include "..\Headers\data_use.h"    // data usage
#include "..\Headers\variaveis.h"   // variable table
#include "..\Headers\diretivas.h"   // compilation directives
#include "..\Headers\data_declar.h" // data declaration
#include "..\Headers\data_assign.h" // data assignment
#include "..\Headers\array_index.h" // array index handling
#include "..\Headers\messages.h"    // PT/EN bilingual support
#include "..\Headers\args.h"        // command-line argument parsing

// required flex/bison variables ----------------------------------------------

int   yylex  (void);
void  yyerror(char const *s);

// Walker invocation shorthand. Producer rules just build the expression tree
// (no inline emit); every statement-level consumer of an `exp` value triggers
// codegen for that subtree by calling EE($N), which runs the walker and
// returns the result POD (type/id/node).
#define EE(NODE) ast_emit_expr(NODE)

%}

// Bison emits the YYSTYPE union into y.tab.h as well, so types referenced
// by %union must be declared before y.tab.h is included by the lexer.
%code requires {
    #include "..\Headers\ast.h"
}

%union {
    int        ival;  // legacy: variable id, type code, INUM literal, etc.
    expr_node *eval;  // expression subtree carried by exp / terminal reductions
}

// tokens with no assignment --------------------------------------------------

%token PRNAME NUBITS NBMANT NBEXPO NDSTAC SDEPTH                       // directives
%token NUIOIN NUIOOU NUGAIN FFTSIZ ITRADD                              // directives
%token INN FIN OUT FOUT                                                // stdlib (I/O)
%token NRM PST ABS SGN COPY                                            // stdlib (special functions)
%token SQRT ATAN SIN COS                                               // stdlib (non-linear functions)
%token REAL IMAG COMP FASE MOD2                                        // stdlib (complex numbers)
%token WHILE IF THEN ELSE SWITCH CASE DEFAULT RET BREAK                // jumps
%token SHIFTL SHIFTR SSHIFTR                                           // bit shift
%token GREQU LESEQ EQU DIF LAN LOR                                     // two-symbol logical operators
%token PPLUS                                                           // ++ operator. can be used both to reduce exp and for assignments
%token BRA KET EYE VZERO                                               // Dirac notation (linear algebra)

// terminal tokens ------------------------------------------------------------

%token <ival> TYPE ID STRING INUM FNUM CNUM                            // comes from the lexer with an associated value

// removes the conflict between if-with-else and if-without-else
%nonassoc THEN
%nonassoc ELSE

// important for a function parameter list
// the first parameter is the last to be parsed
%right ','

// lower in this list -> higher priority (matches C precedence)
%left LOR
%left LAN
%left '|'
%left '^'
%left '&'
%left EQU DIF
%left '>' '<' GREQU LESEQ
%left SHIFTL SHIFTR SSHIFTR
%left '+' '-'
%left '*' '/' '%'
%left '!' '~' PPLUS

// par_list still carries an int (the parameter id from declar_par)
%type <ival> par_list

// expressions ride the bison stack as expr_node *. Each producer builds the
// subtree it just parsed; codegen happens when a statement-level consumer
// hits EE($N), which runs ast_emit_expr() over the subtree.
%type <eval> func_call
%type <eval> std_in std_fin
%type <eval> std_pst std_abs std_sign std_nrm
%type <eval> std_sqrt std_atan std_sin std_cos
%type <eval> std_real std_imag std_comp std_fase std_mod2
%type <eval> exp terminal

%%

// Program and its elements ---------------------------------------------------

fim           : prog
prog          : prog_elements | prog prog_elements
prog_elements : direct | declar | funcao

// Compilation directives -----------------------------------------------------

direct : PRNAME   ID   {dire_exec("#PRNAME",$2, 1);} // processor name
       | NUBITS INUM   {dire_exec("#NUBITS",$2, 0);} // ALU word width
       | NBMANT INUM   {dire_exec("#NBMANT",$2, 3);} // mantissa width (bits)
       | NBEXPO INUM   {dire_exec("#NBEXPO",$2, 4);} // exponent width (bits)
       | NDSTAC INUM   {dire_exec("#NDSTAC",$2, 0);} // data stack depth
       | SDEPTH INUM   {dire_exec("#SDEPTH",$2, 0);} // subroutine stack depth
       | NUIOIN INUM   {dire_exec("#NUIOIN",$2, 7);} // number of input ports
       | NUIOOU INUM   {dire_exec("#NUIOOU",$2, 8);} // number of output ports
       | NUGAIN INUM   {dire_exec("#NUGAIN",$2, 0);} // division constant (norm(.))
       | FFTSIZ INUM   {dire_exec("#FFTSIZ",$2, 0);} // FFT size (2^FFTSIZ)

// Behavioral directives ------------------------------------------------------

dire_inter : ITRADD             {stmt_emit_inline(stmt_dire_inter());} // interrupt start point

// Variable declaration -------------------------------------------------------
         // list declaration (one or more uninitialized variables)
declar : TYPE id_list                               ';'
         // declaration of a variable with initialization
       | TYPE ID '=' exp ';'          {declar_var($2); stmt_emit_inline(stmt_assign($2, $4));}
         // array declaration with file initialization
       | TYPE ID '[' INUM ']'              STRING   ';' {declar_arr_1d($2,$4,$6    );}
       | TYPE ID '[' INUM ']' '[' INUM ']' STRING   ';' {declar_arr_2d($2,$4,$7,$9 );}
         // array declaration with Dirac-notation initialization (on demand)
       | TYPE ID '[' INUM ']' '#' '|' ID '|' ID BRA ';' {declar_Mv    ($2,$4,$8,$10);}
       | TYPE ID '[' INUM ']' '#'    exp '|' ID BRA ';' {declar_cv    ($2, $4, $7, $9);}

id_list : IID | id_list ',' IID

IID    : ID                           {declar_var   ($1         );}
       | ID '[' INUM ']'              {declar_arr_1d($1,$3   ,-1);}
       | ID '[' INUM ']' '[' INUM ']' {declar_arr_2d($1,$3,$6,-1);}

// Function declaration -------------------------------------------------------

funcao : TYPE ID  '('                     {declar_fun($1,$2);} // start of a function declaration
         par_list ')'                     {declar_fst($5   );} // sets the first parameter in the matching variable
         '{'                              {func_body_begin();} // open body capture for the AST walker
         stmt_list '}'                    {func_ret  ($2   );} // close body capture, emit AST, finalize
       | TYPE ID  '('  ')'                {declar_fun($1,$2);} // function without parameters
         '{'                              {func_body_begin();}
         stmt_list '}'                    {func_ret  ($2   );}

// parameter list in the declaration
// arrays are not yet allowed as function parameters
// returns the parameter id
par_list : TYPE ID                        {$$ = declar_par($1,$2);}
         | par_list ',' par_list          {        set_par($3   );} // pulls from the stack

// function and void returns
return_call : RET exp ';'                 {stmt_emit_inline(stmt_return($2  ));}
            | RET     ';'                 {stmt_emit_inline(stmt_return(NULL));}

// statement list in C --------------------------------------------------------

stmt_list: stmt_full | stmt_list stmt_full

// every statement that can be written inside a function
stmt_full: '{' stmt_list '}' // statement block
         |     stmt_case     // every statement type accepted inside a case
         |   switch_case     // switch case
         |         break     // break; inside a while
         |    dire_inter     // interrupt point

// statements that can be used inside a case:
stmt_case:        declar     // variable declarations
         |    assignment     // expression assignment to a variable
         |    while_stmt     // while loop
         |  if_else_stmt     // if/else
         |       std_out     // stdlib data output
         |      std_fout     // stdlib data output (converting to float)
         |      std_vout     // data output with Dirac notation
         |      std_copy     // copies the value of the first argument into the second (no type checking)
         |     void_call     // subroutine call
         |   return_call     // function return

// function calls -------------------------------------------------------------

void_call   : ID '(' exp_list ')' ';'  {stmt_emit_inline(vcall($1));}
func_call   : ID '(' exp_list ')'      {$$ = fcall($1);}

// Each call's arg frame opens at the FIRST reduction of its own exp_list
// (or at the empty alternative for f()), so nested calls each get their own
// frame without a mid-rule action. par_exp / par_listexp then record each
// arg's tree node onto the current top frame.
exp_list :                              {args_frame_push();}
         | exp                          {args_frame_push(); par_exp($1);}
         | exp_list ',' exp             {par_listexp($3);}

// Standard library -----------------------------------------------------------

std_out  : OUT  '(' INUM ',' exp ')' ';'            {stmt_emit_inline(stmt_out($3, $5, 0));}  // data output
std_fout : FOUT '(' INUM ',' exp ')' ';'            {stmt_emit_inline(stmt_out($3, $5, 1));}  // data output (converting to float)
std_in   : INN  '(' INUM ')'                   {$$ = expr_stdlib(OP_STD_IN, $3, NULL, NULL);}  // data input
std_fin  : FIN  '(' INUM ')'                   {$$ = expr_stdlib(OP_STD_FIN, $3, NULL, NULL);}  // float input
std_pst  : PST  '(' exp  ')'                   {$$ = expr_stdlib(OP_STD_PST, 0,  $3,   NULL);}  // clears if negative
std_abs  : ABS  '(' exp  ')'                   {$$ = expr_stdlib(OP_STD_ABS, 0,  $3,   NULL);}  // |x|
std_sign : SGN  '(' exp  ',' exp ')'           {$$ = expr_stdlib(OP_STD_SIGN, 0,  $3,   $5  );}  // y with sign of x
std_nrm  : NRM  '(' exp  ')'                   {$$ = expr_stdlib(OP_STD_NRM, 0,  $3,   NULL);}  // x / NUGAIN
std_copy : COPY '(' exp  ',' ID  ')' ';'       {stmt_emit_inline(stmt_copy($3, $5));}                // void: copies x into y
std_sqrt : SQRT '(' exp  ')'                   {$$ = expr_stdlib(OP_STD_SQRT, 0,  $3,   NULL);}  // sqrt(x)
std_atan : ATAN '(' exp  ')'                   {$$ = expr_stdlib(OP_STD_ATAN, 0,  $3,   NULL);}  // atan(x)
std_sin  : SIN  '(' exp  ')'                   {$$ = expr_stdlib(OP_STD_SIN, 0,  $3,   NULL);}  // sin(x)
std_cos  : COS  '(' exp  ')'                   {$$ = expr_stdlib(OP_STD_COS, 0,  $3,   NULL);}  // cos(x)
std_real : REAL '(' exp  ')'                   {$$ = expr_stdlib(OP_STD_REAL, 0,  $3,   NULL);}  // real(comp)
std_imag : IMAG '(' exp  ')'                   {$$ = expr_stdlib(OP_STD_IMAG, 0,  $3,   NULL);}  // imag(comp)
std_comp : COMP '(' exp  ',' exp ')'           {$$ = expr_stdlib(OP_STD_COMP, 0,  $3,   $5  );}  // complex(x, y)
std_fase : FASE '(' exp  ')'                   {$$ = expr_stdlib(OP_STD_FASE, 0,  $3,   NULL);}  // phase(comp)
std_mod2 : MOD2 '(' exp  ')'                   {$$ = expr_stdlib(OP_STD_MOD2, 0,  $3,   NULL);}  // |comp|^2
std_vout : OUT  '(' INUM ',' exp '|' ID BRA ')' ';' {stmt_emit_inline(stmt_vout($3, $5, $7));}  // data output with Dirac notation

// if/else --------------------------------------------------------------------

if_else_stmt : if_exp stmt_full ELSE             {else_stmt(  );} // complete if/else
               stmt_full                         {stmt_emit_inline(if_fim ());}
             | if_exp stmt_full     %prec THEN   {stmt_emit_inline(if_stmt());} // if without else
if_exp       : IF '(' exp ')'                    {if_exp   ($3);} // build pending STMT_IF

// switch/case ----------------------------------------------------------------

switch_case : SWITCH '(' exp ')'  {exec_switch($3);}
              '{' cases '}'       {stmt_emit_inline(end_switch());}

case_list   :           stmt_case
            | case_list stmt_case
            | case_list BREAK ';' {switch_break();} // case has its own break (different from while and for)

case        : CASE INUM ':'       {  case_test($2,1);} case_list
            | CASE FNUM ':'       {  case_test($2,2);} case_list
default     : DEFAULT   ':'       {defaut_test(    );} case_list

cases       : case | default | case cases

// while ----------------------------------------------------------------------

while_stmt : while_exp stmt_full           {stmt_emit_inline(while_stmt());}
while_exp  : WHILE                         {while_expp  (  );}
            '(' exp ')'                    {while_expexp($4);}
break      : BREAK ';'                     {stmt_emit_inline(exec_break());}

// assignments ----------------------------------------------------------------

           // standard assignment
assignment : ID  '=' exp ';'                          {stmt_emit_inline(stmt_assign($1, $3));}
           // increment
           | ID                          PPLUS ';'    {stmt_emit_inline(stmt_pplus($1, NULL, NULL));}
           | ID  '[' exp ']'             PPLUS ';'    {stmt_emit_inline(stmt_pplus($1, $3,   NULL));}
           | ID  '[' exp ']' '[' exp ']' PPLUS ';'    {stmt_emit_inline(stmt_pplus($1, $3,   $6  ));}
           // regular array
           | ID  '[' exp ']'  '='     exp ';'         {stmt_emit_inline(stmt_array_assign($1, $3, NULL, $6, 0));}
           // reversed array
           | ID  '[' exp ')'  '='     exp ';'         {stmt_emit_inline(stmt_array_assign($1, $3, NULL, $6, 1));}
           // 2D array
           | ID  '[' exp ']' '[' exp ']' '=' exp ';'  {stmt_emit_inline(stmt_array_assign($1, $3, $6,   $9, 0));}
           // linear algebra with Dirac notation (stdlib implemented as a virtual assign)
           | ID '#'     '|' ID '|' ID BRA ';'                    {stmt_emit_inline(stmt_dirac_Mv   ($1, $4, $6));}        // A # |B|a>
           | ID '#' exp '|' ID BRA ';'                           {stmt_emit_inline(stmt_dirac_cv   ($1, $3, $5));}        // a # c|b>
           | ID '#'     '|' ID BRA '+' exp '|' ID BRA ';'        {stmt_emit_inline(stmt_dirac_apcb ($1, $4, $7, $9));}    // a # |b> + c|d>
           | ID '#'     '|' ID BRA KET  ID '|' ';'               {stmt_emit_inline(stmt_dirac_vvt  ($1, $4, $7));}        // A # |a><b|
           | ID '#'     '|' ID '|' '-' '|' ID BRA KET ID '|' ';' {stmt_emit_inline(stmt_dirac_Mmvvt($1, $4, $8, $11));}   // A # B - |a><b|
           | ID '#' exp '|' ID '|' ';'                           {stmt_emit_inline(stmt_dirac_cM   ($1, $3, $5));}        // A # c|B|
           | ID '#' exp     EYE ';'                              {stmt_emit_inline(stmt_dirac_cI   ($1, $3));}            // A # c|I|
           | ID '#'         VZERO ';'                            {stmt_emit_inline(stmt_dirac_v0   ($1));}                // a # |0>
           | ID '#' exp '|' INN '(' INUM ')' BRA ';'             {stmt_emit_inline(stmt_dirac_cvin ($1, $3, $7));}        // a # |in(0)>
           | ID '#' exp '-' '>' '|' ID BRA ';'                   {stmt_emit_inline(stmt_dirac_shift($1, $3, $7));}        // a # c -> |a>

// expressions ----------------------------------------------------------------

exp:       terminal                           {$$ = $1;}
         // arrays
         | ID '[' exp ']'                     {$$ = expr_array_index(v_table[$1].type, $1, 0, $3, NULL);}
         | ID '[' exp ')'                     {$$ = expr_array_index(v_table[$1].type, $1, 1, $3, NULL);}
         | ID '[' exp ']' '[' exp ']'         {$$ = expr_array_index(v_table[$1].type, $1, 0, $3, $6  );}
         // std library that returns values
         | std_in                             {$$ = $1;}
         | std_fin                            {$$ = $1;}
         | std_pst                            {$$ = $1;}
         | std_abs                            {$$ = $1;}
         | std_sign                           {$$ = $1;}
         | std_nrm                            {$$ = $1;}
         | std_sqrt                           {$$ = $1;}
         | std_atan                           {$$ = $1;}
         | std_sin                            {$$ = $1;}
         | std_cos                            {$$ = $1;}
         | std_real                           {$$ = $1;}
         | std_imag                           {$$ = $1;}
         | std_comp                           {$$ = $1;}
         | std_fase                           {$$ = $1;}
         | std_mod2                           {$$ = $1;}
         // function call
         | func_call                          {$$ = $1;}
         // null operators
         |    '(' exp ')'                     {$$ = $2;}
         |    '+' exp                         {$$ = $2;}
         // unary operators
         |    '-' exp                         {$$ = expr_unop(OP_NEG, $2);}
         |    '!' exp                         {$$ = expr_unop(OP_LIN, $2);}
         |    '~' exp                         {$$ = expr_unop(OP_INV, $2);}
         | ID                         PPLUS   {$$ = expr_pplus(v_table[$1].type, $1, NULL, NULL);}
         | ID '[' exp ']'             PPLUS   {$$ = expr_pplus(v_table[$1].type, $1, $3,   NULL);}
         | ID '[' exp ']' '[' exp ']' PPLUS   {$$ = expr_pplus(v_table[$1].type, $1, $3,   $6  );}
         // shift operators
         | exp  SHIFTL exp                    {$$ = expr_binop(OP_SHL, $1, $3);}
         | exp  SHIFTR exp                    {$$ = expr_binop(OP_SHR, $1, $3);}
         | exp SSHIFTR exp                    {$$ = expr_binop(OP_SSHR, $1, $3);}
         // bitwise operators
         | exp   '&'   exp                    {$$ = expr_binop(OP_AND, $1, $3);}
         | exp   '|'   exp                    {$$ = expr_binop(OP_OR, $1, $3);}
         | exp   '^'   exp                    {$$ = expr_binop(OP_XOR, $1, $3);}
         // arithmetic operators
         | exp   '%'   exp                    {$$ = expr_binop(OP_MOD, $1, $3);}
         | exp   '+'   exp                    {$$ = expr_binop(OP_ADD, $1, $3);}
         | exp   '-'   exp                    {$$ = expr_binop(OP_SUB, $1, $3);}
         | exp   '*'   exp                    {$$ = expr_binop(OP_MUL, $1, $3);}
         | exp   '/'   exp                    {$$ = expr_binop(OP_DIV, $1, $3);}
         // true/false operators
         | exp  LAN    exp                    {$$ = expr_binop(OP_LAN, $1, $3);}
         | exp  LOR    exp                    {$$ = expr_binop(OP_LOR, $1, $3);}
         | exp   '<'   exp                    {$$ = expr_binop(OP_LT, $1, $3);}
         | exp   '>'   exp                    {$$ = expr_binop(OP_GT, $1, $3);}
         | exp  EQU    exp                    {$$ = expr_binop(OP_EQ, $1, $3);}
         | exp  GREQU  exp                    {$$ = expr_binop(OP_GE, $1, $3);}
         | exp  LESEQ  exp                    {$$ = expr_binop(OP_LE, $1, $3);}
         | exp  DIF    exp                    {$$ = expr_binop(OP_NE, $1, $3);}
         // linear algebra with exp return (Dirac notation)
         | KET ID '|' ID BRA                  {$$ = expr_inner(expr_var(v_table[$2].type, $2),
                                                            expr_var(v_table[$4].type, $4));}

// terminals used in reductions for expressions -------------------------------
// Pure tree-construction: no emit, no walker call. The expression's emit
// runs in one batch when a statement-level consumer calls EE($N), which
// invokes the walker on the root and emits the entire subtree.

         // constants
terminal : INUM                               {$$ = expr_lit(1, $1);}
         | FNUM                               {$$ = expr_lit(2, $1);}
         | CNUM                               {$$ = expr_lit(5, $1);}
         // variables
         | ID                                 {$$ = expr_var(v_table[$1].type, $1);}

%%

// program entry point
int main(int argc, char *argv[])
{
    parse_lang_flag(&argc, argv);   // processes -en/-pt flag (removes it from argv)

    cli_args a;
    cli_parse(argc, argv, &a);      // parses the named options (or exits with usage)

    parse_init(a.input, a.name, a.proc_dir, a.macros_dir, a.temp_dir,
               a.project ? "1" : "0"); // initializes the parser and global variables
    yyparse   ();                       // here the magic happens!!
    parse_end (a.name, a.proc_dir);     // finalizes the parser

    // final message
    printf(MSG_OK_CMM_DONE);

    return 0;
}

// bison syntax error
void yyerror (char const *s)
{
    fprintf (stderr, MSG_ERR_SYNTAX, line_num+1);
    exit(EXIT_FAILURE);
}
