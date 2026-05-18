/*
    Features unique to C+-

    - Directives #USEMAC #ENDMAC: select code chunks to be replaced by Macros optimized in assembly
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

%}

// Bison emits the YYSTYPE union into y.tab.h as well, so types referenced
// by %union must be declared before y.tab.h is included by the lexer.
%code requires {
    #include "..\Headers\ast.h"
}

%union {
    int  ival;     // legacy: variable id, type code, packed et, INUM literal, etc.
    expr eval;     // expression value carried by exp / terminal reductions
}

// tokens with no assignment --------------------------------------------------

%token PRNAME NUBITS NBMANT NBEXPO NDSTAC SDEPTH                       // directives
%token NUIOIN NUIOOU NUGAIN USEMAC ENDMAC FFTSIZ ITRADD                // directives
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

// every reduction that historically returned an "et" now produces an expr,
// and every consumer that historically took an "et" now takes an expr by
// value. Actions read directly with $$ / $N - the codegen surface no
// longer trafficks in packed int et at all.
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

       | USEMAC STRING INUM {mac_use($2,1,$3);}      // replaces a code section with an assembly macro (outside a function)
       | ENDMAC             {mac_end();}             // end point of the macro usage

// Behavioral directives ------------------------------------------------------

mac_use    : USEMAC STRING INUM {mac_use($2,0,$3);}  // uses a .asm macro in place of the compiler (inside a function)
mac_end    : ENDMAC             {mac_end();}         // macro usage end point
dire_inter : ITRADD             {dire_inter();}      // interrupt start point (used with the itr pin)

// Variable declaration -------------------------------------------------------
         // list declaration (one or more uninitialized variables)
declar : TYPE id_list                               ';'
         // declaration of a variable with initialization
       | TYPE ID '=' exp ';'          {declar_var($2); ass_set($2, $4);}
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
return_call : RET exp ';'                 {declar_ret($2, 1);}
            | RET     ';'                 {  void_ret(    );}

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
         |       mac_use     // declares the use of a macro until ENDMAC is found
         |       mac_end     // ends an assembler-macro call

// function calls -------------------------------------------------------------

// void function
void_call   : ID '('            {fun_id   = $1 ;} // fun_id -> id of the called function
              exp_list ')' ';'  {vcall     ($1);} // we can already emit the void call
// function with return value
func_call   : ID '('            {fun_id   = $1 ;}
              exp_list ')'      {$$ = fcall($1);} // emits the call and returns the final data type

// parameters need to be pushed onto the stack
// for each exp found, the resulting value is written to the stack with par_exp
// the first parameter stays in the accumulator (parsing goes from last to first)
// par_exp pushes parameters onto the stack and checks consistency
exp_list :                                              // may be empty (test it)
         | exp                              {par_exp    ($1);}  // first parameter
         | exp_list ',' exp                 {par_listexp($3);}  // remaining parameters

// Standard library -----------------------------------------------------------

std_out  : OUT  '(' INUM ',' exp ')' ';'            {exec_out ($3, $5);}            // data output
std_fout : FOUT '(' INUM ',' exp ')' ';'            {exec_fout($3, $5);}            // data output (converting to float)
std_in   : INN  '(' INUM ')'                   {$$ = exec_in  ($3);}                // data input
std_fin  : FIN  '(' INUM ')'                   {$$ = exec_fin ($3);}                // data input (converting to float)
std_pst  : PST  '(' exp  ')'                   {$$ = exec_pst ($3);}                // function pset(x)      -> clears if negative
std_abs  : ABS  '(' exp  ')'                   {$$ = exec_abs ($3);}                // function  abs(x)      -> absolute value of x
std_sign : SGN  '(' exp  ',' exp ')'           {$$ = exec_sign($3, $5);}            // function sign(x,y)    -> takes the sign of x and applies to y
std_nrm  : NRM  '(' exp  ')'                   {$$ = exec_norm($3);}                // function norm(x)      -> divides x by the NUGAIN constant
std_copy : COPY '(' exp  ',' ID  ')' ';'       {     exec_copy($3, $5);}            // function copy(x,y)    -> copies the value of x into y (no type checking)
std_sqrt : SQRT '(' exp  ')'                   {$$ = exec_sqrt($3);}                // function sqrt(x)      -> square root
std_atan : ATAN '(' exp  ')'                   {$$ = exec_atan($3);}                // function atan(x)      -> arctangent
std_sin  : SIN  '(' exp  ')'                   {$$ = exec_sin ($3);}                // function  sin(x)      -> sine    of x
std_cos  : COS  '(' exp  ')'                   {$$ = exec_cos ($3);}                // function  cos(x)      -> cosine  of x
std_real : REAL '(' exp  ')'                   {$$ = exec_real($3);}                // function real(x)      -> returns the real part of a comp
std_imag : IMAG '(' exp  ')'                   {$$ = exec_imag($3);}                // function imag(x)      -> returns the imag part of a comp
std_comp : COMP '(' exp  ',' exp ')'           {$$ = exec_comp($3, $5);}            // function complex(x,y) -> creates a comp from 2 reals
std_fase : FASE '(' exp  ')'                   {$$ = exec_fase($3);}                // function fase(x)      -> returns the phase of a comp
std_mod2 : MOD2 '(' exp  ')'                   {$$ = exec_mod2($3);}                // function mod2(x)      -> returns the squared magnitude of a comp
std_vout : OUT  '(' INUM ',' exp '|' ID BRA ')' ';' {exec_vout($3, $5, $7);}        // data output with Dirac notation

// if/else --------------------------------------------------------------------

if_else_stmt : if_exp stmt_full ELSE             {else_stmt(  );} // complete if/else
               stmt_full                         {if_fim   (  );}
             | if_exp stmt_full     %prec THEN   {if_stmt  (  );} // if without else
if_exp       : IF '(' exp ')'                    {if_exp   ($3);} // start (JIZ)

// switch/case ----------------------------------------------------------------

switch_case : SWITCH '(' exp ')'  {exec_switch($3);}
              '{' cases '}'       { end_switch(  );}

case_list   :           stmt_case
            | case_list stmt_case
            | case_list BREAK ';' {switch_break();} // case has its own break (different from while and for)

case        : CASE INUM ':'       {  case_test($2,1);} case_list
            | CASE FNUM ':'       {  case_test($2,2);} case_list
default     : DEFAULT   ':'       {defaut_test(    );} case_list

cases       : case | default | case cases

// while ----------------------------------------------------------------------

while_stmt : while_exp stmt_full           {while_stmt  (  );}
while_exp  : WHILE                         {while_expp  (  );}
            '(' exp ')'                    {while_expexp($4);}
break      : BREAK ';'                     {exec_break  (  );}

// assignments ----------------------------------------------------------------

           // standard assignment
assignment : ID  '=' exp ';'                          {ass_set($1, $3);}
           // increment
           | ID                          PPLUS ';'    {ass_pplus($1);}
           | ID  '[' exp ']'             PPLUS ';'    {ass_aplus($1, $3);}
           | ID  '[' exp ']' '[' exp ']' PPLUS ';'    {ass_apl2d($1, $3, $6);}
           // regular array
           | ID  '[' exp ']'  '='                     {arr_1d_index($1, $3);}
                     exp ';'                          {ass_array ($1, $7, 0);}
           // reversed array
           | ID  '[' exp ')'  '='                     {arr_1d_index($1, $3);}
                     exp ';'                          {ass_array ($1, $7, 1);}
           // 2D array (to be completed)
           | ID  '[' exp ']' '[' exp ']' '='          {arr_2d_index($1, $3, $6);}
                     exp ';'                          {ass_array   ($1, $10, 0);}
           // linear algebra with Dirac notation (stdlib implemented as a virtual assign)
           | ID '#'     '|' ID '|' ID BRA ';'                    {exec_Mv   ($1,$4,$6);}                       // A # |B|a>
           | ID '#' exp '|' ID BRA ';'                           {exec_cv   ($1, $3, $5);}                     // a # c|b>
           | ID '#'     '|' ID BRA '+' exp '|' ID BRA ';'        {exec_apcb ($1, $4, $7, $9);}                 // a # |b> + c|d>
           | ID '#'     '|' ID BRA KET  ID '|' ';'               {exec_vvt  ($1, $4, $7);}                     // A # |a><b|
           | ID '#'     '|' ID '|' '-' '|' ID BRA KET ID '|' ';' {exec_Mmvvt($1, $4, $8, $11);}                // A # B - |a><b|
           | ID '#' exp '|' ID '|' ';'                           {exec_cM   ($1, $3, $5);}                     // A # c|B|
           | ID '#' exp     EYE ';'                              {exec_cI   ($1, $3);}                         // A # c|I|
           | ID '#'         VZERO ';'                            {exec_v0   ($1);}                             // a # |0>
           | ID '#' exp '|' INN '(' INUM ')' BRA ';'             {exec_cvin ($1, $3, $7);}                     // a # |in(0)>
           | ID '#' exp '-' '>' '|' ID BRA ';'                   {exec_shift($1, $3, $7);}                     // a # c -> |a>

// expressions ----------------------------------------------------------------

exp:       terminal                           {$$ = $1;}
         // arrays
         | ID '[' exp ']'                     {$$ = arr_1d2exp($1, $3, 0);}
         | ID '[' exp ')'                     {$$ = arr_1d2exp($1, $3, 1);}
         | ID '[' exp ']' '[' exp ']'         {$$ = arr_2d2exp($1, $3, $6);}
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
         |    '-' exp                         {$$ = oper_neg($2);}
         |    '!' exp                         {$$ = oper_lin($2);}
         |    '~' exp                         {$$ = oper_inv($2);}
         | ID                         PPLUS   {$$ = pplus2exp  ($1);}
         | ID '[' exp ']'             PPLUS   {$$ = pplus1d2exp($1, $3);}
         | ID '[' exp ']' '[' exp ']' PPLUS   {$$ = pplus2d2exp($1, $3, $6);}
         // shift operators
         | exp  SHIFTL exp                    {$$ = oper_shift($1, $3, 0);}
         | exp  SHIFTR exp                    {$$ = oper_shift($1, $3, 1);}
         | exp SSHIFTR exp                    {$$ = oper_shift($1, $3, 2);}
         // bitwise operators
         | exp   '&'   exp                    {$$ = oper_bitw ($1, $3, 0);}
         | exp   '|'   exp                    {$$ = oper_bitw ($1, $3, 1);}
         | exp   '^'   exp                    {$$ = oper_bitw ($1, $3, 2);}
         // arithmetic operators
         | exp   '%'   exp                    {$$ = oper_mod ($1, $3);}
         | exp   '+'   exp                    {$$ = oper_soma($1, $3);}
         | exp   '-'   exp                    {$$ = oper_subt($1, $3);}
         | exp   '*'   exp                    {$$ = oper_mult($1, $3);}
         | exp   '/'   exp                    {$$ = oper_divi($1, $3);}
         // true/false operators
         | exp  LAN    exp                    {$$ = oper_lanor($1, $3, 0);}
         | exp  LOR    exp                    {$$ = oper_lanor($1, $3, 1);}
         | exp   '<'   exp                    {$$ = oper_cmp  ($1, $3, 0);}
         | exp   '>'   exp                    {$$ = oper_cmp  ($1, $3, 1);}
         | exp  EQU    exp                    {$$ = oper_cmp  ($1, $3, 2);}
         | exp  GREQU  exp                    {$$ = oper_greq ($1, $3);}
         | exp  LESEQ  exp                    {$$ = oper_leeq ($1, $3);}
         | exp  DIF    exp                    {$$ = oper_dife ($1, $3);}
         // linear algebra with exp return (Dirac notation)
         | KET ID '|' ID BRA                  {$$ = exec_vtv($2, $4);}

// terminals used in reductions for expressions -------------------------------

         // constants
terminal : INUM                               {$$ = num2exp($1, 1); $$.node = expr_lit(1, $1);}
         | FNUM                               {$$ = num2exp($1, 2);}
         | CNUM                               {$$ = num2exp($1, 5);}
         // variables
         | ID                                 {$$ =  id2exp($1);}

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
