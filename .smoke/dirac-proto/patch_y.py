p = 'C:/nipscern/yanc-dirac/Compilers/CMMComp/Sources/CMMComp.y'
s = open(p, encoding='utf-8', newline='').read().replace('\r\n', '\n')
R = []

R.append(("""    - Statements in Dirac notation (linear algebra):
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
*/""", """    - Dirac notation (linear algebra), as expressions (docs/design/dirac-expressions.md):
      a chain of array names between bars, |a>, <a|, |M|, |M|a>, <a|M|b>, |A|B|x>:
      the first name is a bra when the chain opens with <, the last a ket when
      it closes with >, every other name a matrix. Juxtaposition is the product
      (c|a>, |a><b|, |A||B|), between + and * in precedence. |I|, |0> and
      |in(p)> take their size from the context. The shapes are checked
      (shapecheck_expr in ast.c).
      x = <a|b>;            -> a number
      a # expression;       -> the vector or matrix a receives the expression
      float a[N] # expr;    -> the same, in a declaration
      out(p, c|a>);         -> OUT of each element of a, times c
      a # c -> |a>;         -> shift register
      For now (step a) only the forms of the old rules are computed:
      |M|a>, c|a>, |a> + c|b>, |a><b|, |A| - |a><b|, c|A|, c|I|, |0>, c|in(p)>.
*/"""))

R.append(("""%token BRA KET EYE VZERO                                               // Dirac notation (linear algebra)""",
"""%token BRA KET EYE VZERO DBAR                                          // Dirac notation (BRA is the closing angle, KET the opening one; DBAR a bar before an array name)
%token <ival> KETIN                                                    // Dirac |in(p)>, the port as an INUM id"""))

R.append(("""// removes the conflict between if-with-else and if-without-else
%nonassoc THEN""", """// every conflict is an error of the build: the grammar has none
%expect 0

// removes the conflict between if-with-else and if-without-else
%nonassoc THEN"""))

R.append(("""%left '+' '-'
%left '*' '/' '%'""", """%left '+' '-'
%left JUXT DBAR KET EYE VZERO KETIN                                    // Dirac juxtaposition: (2*c)|a> and |b> + (c|a>)
%left '*' '/' '%'"""))

R.append(("""%type <eval> exp terminal""", """%type <eval> exp terminal
%type <eval> dirac dchain"""))

R.append(("""         // array declaration with Dirac-notation initialization (on demand)
       | TYPE ID '[' INUM ']' '#' '|' ID '|' ID BRA ';' {declar_Mv    ($2,$4,$8,$10);}
       | TYPE ID '[' INUM ']' '#'    exp '|' ID BRA ';' {declar_cv    ($2,$4,$7, $9);}""",
"""         // array declaration with a Dirac expression
       | TYPE ID '[' INUM ']' '#' exp ';' {declar_dirac($2,$4,$7);}"""))

R.append(("""         |      std_vout     // data output with Dirac notation
""", ""))

R.append(("""std_vout : OUT  '(' INUM ',' exp '|' ID BRA ')' ';' {stmt_append(stmt_vout($3, $5, $7));}  // data output with Dirac notation
""", ""))

R.append(("""           // linear algebra with Dirac notation (stdlib implemented as a virtual assign)
           | ID '#'     '|' ID '|' ID BRA ';'                    {stmt_append(stmt_dirac_Mv   ($1, $4, $6));}        // A # |B|a>
           | ID '#' exp '|' ID BRA ';'                           {stmt_append(stmt_dirac_cv   ($1, $3, $5));}        // a # c|b>
           | ID '#'     '|' ID BRA '+' exp '|' ID BRA ';'        {stmt_append(stmt_dirac_apcb ($1, $4, $7, $9));}    // a # |b> + c|d>
           | ID '#'     '|' ID BRA KET  ID '|' ';'               {stmt_append(stmt_dirac_vvt  ($1, $4, $7));}        // A # |a><b|
           | ID '#'     '|' ID '|' '-' '|' ID BRA KET ID '|' ';' {stmt_append(stmt_dirac_Mmvvt($1, $4, $8, $11));}   // A # B - |a><b|
           | ID '#' exp '|' ID '|' ';'                           {stmt_append(stmt_dirac_cM   ($1, $3, $5));}        // A # c|B|
           | ID '#' exp     EYE ';'                              {stmt_append(stmt_dirac_cI   ($1, $3));}            // A # c|I|
           | ID '#'         VZERO ';'                            {stmt_append(stmt_dirac_v0   ($1));}                // a # |0>
           | ID '#' exp '|' INN '(' INUM ')' BRA ';'             {stmt_append(stmt_dirac_cvin ($1, $3, $7));}        // a # |in(0)>
           | ID '#' exp '-' '>' '|' ID BRA ';'                   {stmt_append(stmt_dirac_shift($1, $3, $7));}        // a # c -> |a>""",
"""           // linear algebra with Dirac notation: a vector or matrix receives an expression
           | ID '#' exp ';'                                      {stmt_append(stmt_dirac      ($1, $3));}            // A # |B|a>, a # |b> + c|d>, ...
           | ID '#' exp '-' '>' DBAR ID BRA ';'                  {stmt_append(stmt_dirac_shift($1, $3, $7));}        // a # c -> |a>"""))

R.append(("""         // linear algebra with exp return (Dirac notation)
         | KET ID '|' ID BRA                  {$$ = expr_inner(expr_var(v_table[$2].type, $2),
                                                            expr_var(v_table[$4].type, $4));}
""", """         // Dirac notation: a term, and the product by juxtaposition
         | dirac                              {$$ = $1;}
         | exp dirac %prec JUXT               {$$ = expr_dirac_prod($1, $2);}
         // a bar and a name that is not an array, closed as a ket: say so
         // instead of a syntax error
         |     '|' ID BRA                     {dirac_not_vector($2); $$ = NULL;}
         | exp '|' ID BRA                     {dirac_not_vector($3); $$ = NULL;}

// a Dirac term: a chain of names between bars (dchain), |I|, |0> or |in(p)>
dirac    : dchain BRA                         {$$ = dirac_close($1, 1);}
         | dchain '|'                         {$$ = dirac_close($1, 0);}
         | EYE                                {$$ = expr_dirac_const(DIRAC_EYE);}
         | VZERO                              {$$ = expr_dirac_const(DIRAC_ZERO);}
         | KETIN                              {$$ = expr_dirac_in($1);}

dchain   : DBAR ID                            {$$ = dirac_open(0, $2);}
         | KET  ID                            {$$ = dirac_open(1, $2);}
         | dchain DBAR ID                     {$$ = dirac_push($1, $3);}
"""))

for a, b in R:
    assert s.count(a) == 1, a[:70]
    s = s.replace(a, b)
open(p, 'w', encoding='utf-8', newline='').write(s.replace('\n', '\r\n'))
print("ok")
