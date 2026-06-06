#include "ast.h"   // expr (for the migrated unary operators)

// ----------------------------------------------------------------------------
// arithmetic operations ------------------------------------------------------
// ----------------------------------------------------------------------------

expr oper_neg (expr e);          //  -x
expr oper_soma(expr e1, expr e2); // x+y
expr oper_subt(expr e1, expr e2); // x-y
expr oper_mult(expr e1, expr e2); // x*y
expr oper_divi(expr e1, expr e2); // x/y
expr oper_mod (expr e1, expr e2); // x%y
void emit_sq_sum(expr er, expr ei); // er²+ei² in acc (|comp|²), acc-aware reorder

// ----------------------------------------------------------------------------
// comparison operations ------------------------------------------------------
// ----------------------------------------------------------------------------

expr oper_cmp (expr e1, expr e2, int type); // x>y, x<y, x==y
expr oper_greq(expr e1, expr e2);           // x>=y
expr oper_leeq(expr e1, expr e2);           // x<=y
expr oper_dife(expr e1, expr e2);           // x!=y

// ----------------------------------------------------------------------------
// logical operations (if else while) -----------------------------------------
// ----------------------------------------------------------------------------

expr oper_lin  (expr e);                     // !x
expr oper_lanor(expr e1, expr e2, int type); // x&&y, x||y

// ----------------------------------------------------------------------------
// bitwise logical-gate operations (~ & |) ------------------------------------
// ----------------------------------------------------------------------------

expr oper_inv (expr e);                      // ~x
expr oper_bitw(expr e1, expr e2, int type);  // x&y, x|y, x^y

// ----------------------------------------------------------------------------
// bit-shift operations -------------------------------------------------------
// ----------------------------------------------------------------------------

expr oper_shift(expr e1, expr e2, int type); // x>>y, x<<y, x>>>y
