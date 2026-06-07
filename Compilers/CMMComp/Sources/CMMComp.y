/*
    Features unique to C+-

    - Directive  #PRACA         : marks the return point for a reset on the itr pin
    - Directive  #TOAQUI        : marks an address that the cheguei pin reflects (PC == addr)

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
    - StdLib    sqrt(.)  : returns the square root (float; principal root for a complex argument)
    - StdLib    atan(.)  : returns the arctangent. Produces a float
    - StdLib     sin(.)  : returns the sine    of a number
    - StdLib     cos(.)  : returns the cosine  of a number
    - StdLib     tan(.)  : returns the tangent of a number (dedicated minimax, not sin/cos)
    - StdLib    cosh(.)  : hyperbolic cosine (e^x+e^-x)/2
    - StdLib    sinh(.)  : hyperbolic sine   (e^x-e^-x)/2
    - StdLib    tanh(.)  : hyperbolic tangent (e^2x-1)/(e^2x+1)
    - StdLib     exp(.)  : returns e^x (exponential). Produces a float
    - StdLib     log(.)  : returns the natural logarithm (ln). Produces a float
    - StdLib     pow(.,.): returns x^y. Const int exponent -> square-and-multiply; int var -> runtime loop; else exp(y*ln x)
    - StdLib   floor(.)  : largest integral float <= x   (returns a float)
    - StdLib    ceil(.)  : smallest integral float >= x  (returns a float)
    - StdLib   round(.)  : nearest integral float, ties away from zero (returns a float)
    - StdLib    real(.)  : returns the real part of a complex number
    - StdLib    imag(.)  : returns the imag part of a complex number
    - StdLib    fase(.)  : returns the phase    of a complex number
    - StdLib    mod2(.)  : squared magnitude    of a complex number
    - StdLib complex(.,.): creates a complex number from two reals
    - StdLib    conj(.)  : complex conjugate (a-bi); a real x becomes x+0i (always returns a comp)

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

#include "../Headers/itr.h"         // interrupt handling
#include "../Headers/oper.h"        // ALU operations
#include "../Headers/stdlib.h"      // SAPHO standard library
#include "../Headers/saltos.h"      // jump management (if/else while)
#include "../Headers/global.h"      // global variables and functions
#include "../Headers/macros.h"      // assembler macros
#include "../Headers/funcoes.h"     // function creation and usage
#include "../Headers/data_use.h"    // data usage
#include "../Headers/variaveis.h"   // variable table
#include "../Headers/diretivas.h"   // compilation directives
#include "../Headers/data_declar.h" // data declaration
#include "../Headers/data_assign.h" // data assignment
#include "../Headers/array_index.h" // array index handling
#include "../Headers/messages.h"    // PT/EN bilingual support
#include "../Headers/args.h"        // command-line argument parsing

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
    #include "../Headers/ast.h"
}

%union {
    int        ival;  // legacy: variable id, type code, INUM literal, etc.
    expr_node *eval;  // expression subtree carried by exp / terminal reductions
}

// tokens with no assignment --------------------------------------------------

%token PRNAME NUBITS NBMANT NBEXPO NDSTAC SDEPTH                       // directives
%token NUIOIN NUIOOU NUGAIN FFTSIZ ITRADD TOAQUI                       // directives
%token INN FIN OUT FOUT                                                // stdlib (I/O)
%token NRM PST ABS SGN COPY                                            // stdlib (special functions)
%token SQRT ATAN SIN COS TAN EXP LOG POW                               // stdlib (non-linear functions)
%token COSH SINH TANH                                                  // stdlib (hyperbolic)
%token FLOOR CEIL ROUND                                                // stdlib (rounding)
%token REAL IMAG COMP FASE MOD2 CONJ                                   // stdlib (complex numbers)
%token WHILE DO FOR IF THEN ELSE SWITCH CASE DEFAULT RET BREAK CONTINUE   // jumps
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

// par_list / par_items have no semantic value (declar_par appends ids to the
// deferred func_params[] array; func_body_begin consumes them).
//
// func_intro carries the function's v_table id up to funcao so func_ret can
// finalize the function without a mid-rule on `TYPE ID`.
%type <ival> func_intro

// expressions ride the bison stack as expr_node *. Each producer builds the
// subtree it just parsed; codegen happens when a statement-level consumer
// hits EE($N), which runs ast_emit_expr() over the subtree.
%type <eval> func_call
%type <eval> std_in std_fin
%type <eval> std_pst std_abs std_sign std_nrm
%type <eval> std_sqrt std_atan std_sin std_cos std_tan std_exp std_log std_pow
%type <eval> std_cosh std_sinh std_tanh
%type <eval> std_floor std_ceil std_round
%type <eval> std_real std_imag std_comp std_fase std_mod2 std_conj
%type <eval> exp terminal

%%

// Program and its elements ---------------------------------------------------
//
// Each prog_elements reduce appends a stmt_node to the global stmt_list
// opened by parse_init. The `fim : prog` end-action fires once parsing
// completes and runs the walker over the whole program. No codegen
// reaches f_asm before this point.

fim           : prog {prog_emit();}
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

dire_inter : ITRADD             {stmt_append(stmt_dire_inter ());} // interrupt start point (#PRACA)
           | TOAQUI             {stmt_append(stmt_dire_toaqui());} // PC tracker marker     (#TOAQUI -> cheguei pin)

// Variable declaration -------------------------------------------------------
         // list declaration (one or more uninitialized variables)
declar : TYPE id_list                               ';'
         // declaration of a variable with initialization
       | TYPE ID '=' exp ';'          {declar_var($2); stmt_append(stmt_assign($2, $4));}
         // array declaration with file initialization
       | TYPE ID '[' INUM ']'              STRING   ';' {declar_arr_1d($2,$4,$6    );}
       | TYPE ID '[' INUM ']' '[' INUM ']' STRING   ';' {declar_arr_2d($2,$4,$7,$9 );}
         // array declaration with Dirac-notation initialization (on demand)
       | TYPE ID '[' INUM ']' '#' '|' ID '|' ID BRA ';' {declar_Mv    ($2,$4,$8,$10);}
       | TYPE ID '[' INUM ']' '#'    exp '|' ID BRA ';' {declar_cv    ($2,$4,$7, $9);}

id_list : IID | id_list ',' IID

IID    : ID                           {declar_var   ($1         );}
       | ID '[' INUM ']'              {declar_arr_1d($1,$3   ,-1);}
       | ID '[' INUM ']' '[' INUM ']' {declar_arr_2d($1,$3,$6,-1);}

// Function declaration -------------------------------------------------------

funcao : func_intro par_list ')' '{' stmt_list '}' {func_ret($1);} // $1 = func_intro's ID; walker emits everything

// State-setup phase of a function declaration. Pulled into its own named
// non-terminal so the action is a clean end-action - no mid-rule sitting
// between TYPE ID '(' and par_list. declar_fun records fname / fun_parse /
// the JMP-main flag / the parameter collector reset; par_list / declar_par
// run afterwards and depend on that state being in place.
func_intro : TYPE ID '('                  {declar_fun($1,$2); $$ = $2;}
           ;

// Parameter list. par_list always reduces, even when empty - its end-action
// is the natural place to build the STMT_FUNC_HEADER (param count is final)
// and open the body's stmt_list collector. declar_par records each parameter
// id in the deferred func_params[] array; the actual SET / SET_P emits live
// in the STMT_FUNC_HEADER walker case. Arrays are not allowed as parameters.
par_list  : /* empty */                   {func_body_begin();}
          | par_items                     {func_body_begin();}
          ;

par_items : TYPE ID                       {declar_par($1,$2);}
          | par_items ',' TYPE ID         {declar_par($3,$4);}

// function and void returns
return_call : RET exp ';'                 {stmt_append(stmt_return($2  ));}
            | RET     ';'                 {stmt_append(stmt_return(NULL));}

// statement list in C --------------------------------------------------------

stmt_list: stmt_full | stmt_list stmt_full

// Every statement that can be written inside a function. There is no separate
// "statements allowed in a case" tier any more: a case body is an ordinary
// stmt_list, and `break` binds to the innermost enclosing loop OR switch via
// the break-target stack in saltos.c (see exec_break). That single change is
// what lets a switch nest inside a case, lets a `{ }` block live in a case, and
// makes a `break` inside an if-in-a-case correctly exit the switch.
stmt_full: '{' stmt_list '}' // statement block
         |        declar     // variable declarations
         |    assignment     // expression assignment to a variable
         |    while_stmt     // while loop
         |      do_while     // do { } while () loop
         |      for_stmt     // for loop (desugared to while + init/step)
         |  if_else_stmt     // if/else
         |       std_out     // stdlib data output
         |      std_fout     // stdlib data output (converting to float)
         |      std_vout     // data output with Dirac notation
         |      std_copy     // copies the value of the first argument into the second (no type checking)
         |     void_call     // subroutine call
         |   return_call     // function return
         |   switch_case     // switch/case
         |         break     // break; (innermost loop or switch)
         |      continue     // continue; (innermost loop)
         |    dire_inter     // interrupt point

// function calls -------------------------------------------------------------

void_call   : ID '(' exp_list ')' ';'  {stmt_append(vcall($1));}
func_call   : ID '(' exp_list ')'      {$$ = fcall($1);}

// Each call's arg frame opens at the FIRST reduction of its own exp_list
// (or at the empty alternative for f()), so nested calls each get their own
// frame without a mid-rule action. par_exp / par_listexp then record each
// arg's tree node onto the current top frame.
exp_list :                              {args_frame_push();}
         | exp                          {args_frame_push(); par_exp($1);}
         | exp_list ',' exp             {par_listexp($3);}

// Standard library -----------------------------------------------------------

std_out  : OUT  '(' INUM ',' exp ')' ';'            {stmt_append(stmt_out($3, $5, 0));}  // data output
std_fout : FOUT '(' INUM ',' exp ')' ';'            {stmt_append(stmt_out($3, $5, 1));}  // data output (converting to float)
std_in   : INN  '(' INUM ')'                   {$$ = expr_stdlib(OP_STD_IN, $3, NULL, NULL);}  // data input
std_fin  : FIN  '(' INUM ')'                   {$$ = expr_stdlib(OP_STD_FIN, $3, NULL, NULL);}  // float input
std_pst  : PST  '(' exp  ')'                   {$$ = expr_stdlib(OP_STD_PST, 0,  $3,   NULL);}  // clears if negative
std_abs  : ABS  '(' exp  ')'                   {$$ = expr_stdlib(OP_STD_ABS, 0,  $3,   NULL);}  // |x|
std_sign : SGN  '(' exp  ',' exp ')'           {$$ = expr_stdlib(OP_STD_SIGN, 0,  $3,   $5  );}  // y with sign of x
std_nrm  : NRM  '(' exp  ')'                   {$$ = expr_stdlib(OP_STD_NRM, 0,  $3,   NULL);}  // x / NUGAIN
std_copy : COPY '(' exp  ',' ID  ')' ';'       {stmt_append(stmt_copy($3, $5));}                // void: copies x into y
std_sqrt : SQRT '(' exp  ')'                   {$$ = expr_stdlib(OP_STD_SQRT, 0,  $3,   NULL);}  // sqrt(x)
std_atan : ATAN '(' exp  ')'                   {$$ = expr_stdlib(OP_STD_ATAN, 0,  $3,   NULL);}  // atan(x)
std_sin  : SIN  '(' exp  ')'                   {$$ = expr_stdlib(OP_STD_SIN, 0,  $3,   NULL);}  // sin(x)
std_cos  : COS  '(' exp  ')'                   {$$ = expr_stdlib(OP_STD_COS, 0,  $3,   NULL);}  // cos(x)
std_tan  : TAN  '(' exp  ')'                   {$$ = expr_stdlib(OP_STD_TAN, 0,  $3,   NULL);}  // tan(x)
std_cosh : COSH '(' exp  ')'                   {$$ = expr_stdlib(OP_STD_COSH, 0,  $3,   NULL);}  // cosh(x)
std_sinh : SINH '(' exp  ')'                   {$$ = expr_stdlib(OP_STD_SINH, 0,  $3,   NULL);}  // sinh(x)
std_tanh : TANH '(' exp  ')'                   {$$ = expr_stdlib(OP_STD_TANH, 0,  $3,   NULL);}  // tanh(x)
std_exp  : EXP  '(' exp  ')'                   {$$ = expr_stdlib(OP_STD_EXP, 0,  $3,   NULL);}  // exp(x)
std_log  : LOG  '(' exp  ')'                   {$$ = expr_stdlib(OP_STD_LOG, 0,  $3,   NULL);}  // log(x)
std_pow  : POW  '(' exp  ',' exp ')'           {$$ = expr_stdlib(OP_STD_POW, 0,  $3,   $5  );}  // pow(x, y) = x^y
std_floor: FLOOR '(' exp ')'                   {$$ = expr_stdlib(OP_STD_FLOOR, 0,  $3,   NULL);}  // floor(x)
std_ceil : CEIL '(' exp  ')'                   {$$ = expr_stdlib(OP_STD_CEIL, 0,  $3,   NULL);}  // ceil(x)
std_round: ROUND '(' exp ')'                   {$$ = expr_stdlib(OP_STD_ROUND, 0,  $3,   NULL);}  // round(x)
std_real : REAL '(' exp  ')'                   {$$ = expr_stdlib(OP_STD_REAL, 0,  $3,   NULL);}  // real(comp)
std_imag : IMAG '(' exp  ')'                   {$$ = expr_stdlib(OP_STD_IMAG, 0,  $3,   NULL);}  // imag(comp)
std_comp : COMP '(' exp  ',' exp ')'           {$$ = expr_stdlib(OP_STD_COMP, 0,  $3,   $5  );}  // complex(x, y)
std_fase : FASE '(' exp  ')'                   {$$ = expr_stdlib(OP_STD_FASE, 0,  $3,   NULL);}  // phase(comp)
std_mod2 : MOD2 '(' exp  ')'                   {$$ = expr_stdlib(OP_STD_MOD2, 0,  $3,   NULL);}  // |comp|^2
std_conj : CONJ '(' exp  ')'                   {$$ = expr_stdlib(OP_STD_CONJ, 0,  $3,   NULL);}  // conjugate
std_vout : OUT  '(' INUM ',' exp '|' ID BRA ')' ';' {stmt_append(stmt_vout($3, $5, $7));}  // data output with Dirac notation

// if/else --------------------------------------------------------------------

if_else_stmt : if_exp stmt_full else_intro stmt_full {stmt_append(if_fim());}  // complete if/else
             | if_exp stmt_full %prec THEN           {stmt_append(if_stmt());} // if without else
if_exp       : IF '(' exp ')'                        {if_exp($3);}                  // build pending STMT_IF
else_intro   : ELSE                                  {else_stmt();}                 // close then-body, open else-body

// switch/case ----------------------------------------------------------------

switch_case   : switch_intro '{' sw_body '}' {stmt_append(end_switch());}
switch_intro  : SWITCH '(' exp ')'           {exec_switch($3);}

// The switch body is a flat sequence of case labels, the default label and
// statements, in any order -- the C model (a label is just a marker, not a
// container). case_test / defaut_test append a CASE_LABEL / DEFAULT_LABEL node;
// the codegen (STMT_SWITCH walker) builds a dispatch block from the labels and
// lays the bodies out contiguously, so a case without `break` falls through to
// the next (real C fall-through) and an empty case stacks onto the following
// one. `break` is an ordinary stmt_full, resolved to this switch by the
// break-target stack.
sw_body       : /* empty */
              | sw_body case_label
              | sw_body default_label
              | sw_body stmt_full

case_label    : CASE INUM ':' {case_test($2,1);}
              | CASE FNUM ':' {case_test($2,2);}

default_label : DEFAULT   ':' {defaut_test();}

// while ----------------------------------------------------------------------

while_stmt  : while_exp stmt_full {stmt_append(while_stmt());}
while_exp   : while_intro '(' exp ')' {while_expexp($3);}
while_intro : WHILE               {while_expp();}
break      : BREAK ';'                     {stmt_append(exec_break());}
continue   : CONTINUE ';'                  {stmt_append(exec_continue());}

// do { body } while (cond);  -- do_intro opens the loop (labels/break/continue
// stacks + body list) before the body, so break/continue inside bind correctly;
// do_finish takes the bottom-tested condition and returns the STMT_DO.
do_while    : do_intro stmt_full WHILE '(' exp ')' ';' {stmt_append(do_finish($5));}
do_intro    : DO                          {do_open();}

// for -----------------------------------------------------------------------
//
// for (init; cond; step) body  desugars at parse-time into:
//     init;
//     while (cond) { body; step; }
//
// for_init_clause / for_step_clause stash their stmt_node into static state
// in saltos.c (for_init_set / for_step_set). for_intro reads the cond and
// pulls the stashed init/step into the pending stmt_while's then_body /
// else_body slots so nested fors can each carry their own. for_finish
// closes the body block, appends step at its tail, attaches the body to
// the partial while, appends init to the parent stmt_list, and returns
// the while_node which the rule's end-action appends right after init.

for_stmt        : for_intro stmt_full                  {stmt_append(for_finish());}
for_intro       : FOR '(' for_init_clause ';' exp ';' for_step_clause ')' {for_open($5);}

for_init_clause : /* empty */                          {for_init_set(NULL);}
                | ID  '=' exp                          {for_init_set(stmt_assign($1, $3));}
                | TYPE ID '=' exp                      {declar_var($2); for_init_set(stmt_assign($2, $4));}

for_step_clause : /* empty */                          {for_step_set(NULL);}
                | ID  '=' exp                          {for_step_set(stmt_assign($1, $3));}
                | ID PPLUS                             {for_step_set(stmt_pplus($1, NULL, NULL));}

// assignments ----------------------------------------------------------------

           // standard assignment
assignment : ID  '=' exp ';'                          {stmt_append(stmt_assign($1, $3));}
           // increment
           | ID                          PPLUS ';'    {stmt_append(stmt_pplus($1, NULL, NULL));}
           | ID  '[' exp ']'             PPLUS ';'    {stmt_append(stmt_pplus($1, $3,   NULL));}
           | ID  '[' exp ']' '[' exp ']' PPLUS ';'    {stmt_append(stmt_pplus($1, $3,   $6  ));}
           // regular array
           | ID  '[' exp ']'  '='     exp ';'         {stmt_append(stmt_array_assign($1, $3, NULL, $6, 0));}
           // reversed array
           | ID  '[' exp ')'  '='     exp ';'         {stmt_append(stmt_array_assign($1, $3, NULL, $6, 1));}
           // 2D array
           | ID  '[' exp ']' '[' exp ']' '=' exp ';'  {stmt_append(stmt_array_assign($1, $3, $6,   $9, 0));}
           // linear algebra with Dirac notation (stdlib implemented as a virtual assign)
           | ID '#'     '|' ID '|' ID BRA ';'                    {stmt_append(stmt_dirac_Mv   ($1, $4, $6));}        // A # |B|a>
           | ID '#' exp '|' ID BRA ';'                           {stmt_append(stmt_dirac_cv   ($1, $3, $5));}        // a # c|b>
           | ID '#'     '|' ID BRA '+' exp '|' ID BRA ';'        {stmt_append(stmt_dirac_apcb ($1, $4, $7, $9));}    // a # |b> + c|d>
           | ID '#'     '|' ID BRA KET  ID '|' ';'               {stmt_append(stmt_dirac_vvt  ($1, $4, $7));}        // A # |a><b|
           | ID '#'     '|' ID '|' '-' '|' ID BRA KET ID '|' ';' {stmt_append(stmt_dirac_Mmvvt($1, $4, $8, $11));}   // A # B - |a><b|
           | ID '#' exp '|' ID '|' ';'                           {stmt_append(stmt_dirac_cM   ($1, $3, $5));}        // A # c|B|
           | ID '#' exp     EYE ';'                              {stmt_append(stmt_dirac_cI   ($1, $3));}            // A # c|I|
           | ID '#'         VZERO ';'                            {stmt_append(stmt_dirac_v0   ($1));}                // a # |0>
           | ID '#' exp '|' INN '(' INUM ')' BRA ';'             {stmt_append(stmt_dirac_cvin ($1, $3, $7));}        // a # |in(0)>
           | ID '#' exp '-' '>' '|' ID BRA ';'                   {stmt_append(stmt_dirac_shift($1, $3, $7));}        // a # c -> |a>

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
         | std_tan                            {$$ = $1;}
         | std_cosh                           {$$ = $1;}
         | std_sinh                           {$$ = $1;}
         | std_tanh                           {$$ = $1;}
         | std_exp                            {$$ = $1;}
         | std_log                            {$$ = $1;}
         | std_pow                            {$$ = $1;}
         | std_floor                          {$$ = $1;}
         | std_ceil                           {$$ = $1;}
         | std_round                          {$$ = $1;}
         | std_real                           {$$ = $1;}
         | std_imag                           {$$ = $1;}
         | std_comp                           {$$ = $1;}
         | std_fase                           {$$ = $1;}
         | std_mod2                           {$$ = $1;}
         | std_conj                           {$$ = $1;}
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
               a.array_sim ? "1" : "0"); // initializes the parser and global variables
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
