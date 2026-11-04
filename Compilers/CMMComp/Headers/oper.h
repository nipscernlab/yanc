#ifndef CMM_OPER_H
#define CMM_OPER_H

#include "ast.h"   // expr

// ----------------------------------------------------------------------------
// arithmetic operations ------------------------------------------------------
// ----------------------------------------------------------------------------

expr oper_neg (expr e);          //  -x
expr oper_add(expr e1, expr e2); // x+y
expr oper_sub(expr e1, expr e2); // x-y
expr oper_mult(expr e1, expr e2); // x*y
expr oper_div(expr e1, expr e2); // x/y
expr oper_mod (expr e1, expr e2); // x%y
void emit_sq_sum(expr er, expr ei); // er²+ei² in acc (|comp|²), acc-aware reorder

// ----------------------------------------------------------------------------
// comparison operations ------------------------------------------------------
// ----------------------------------------------------------------------------

expr oper_cmp (expr e1, expr e2, int type); // x>y, x<y, x==y
expr oper_ge(expr e1, expr e2);           // x>=y
expr oper_le(expr e1, expr e2);           // x<=y
expr oper_ne(expr e1, expr e2);           // x!=y

// ----------------------------------------------------------------------------
// logical operations (if else while) -----------------------------------------
// ----------------------------------------------------------------------------

expr oper_lin  (expr e);                     // !x
expr oper_logic(expr e1, expr e2, int type); // x&&y, x||y

// ----------------------------------------------------------------------------
// bitwise logical-gate operations (~ & |) ------------------------------------
// ----------------------------------------------------------------------------

expr oper_inv (expr e);                      // ~x
expr oper_bitw(expr e1, expr e2, int type);  // x&y, x|y, x^y

// ----------------------------------------------------------------------------
// bit-shift operations -------------------------------------------------------
// ----------------------------------------------------------------------------

expr oper_shift(expr e1, expr e2, int type); // x>>y, x<<y, x>>>y

#endif // CMM_OPER_H
