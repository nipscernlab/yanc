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

// ----------------------------------------------------------------------------
// comparison operations ------------------------------------------------------
// ----------------------------------------------------------------------------

int oper_cmp  (int et1, int et2, int type); // x>y, x<y, x==y
int oper_greq (int et1, int et2);           // x>=y
int oper_leeq (int et1, int et2);           // x<=y
int oper_dife (int et1, int et2);           // x!=y

// ----------------------------------------------------------------------------
// logical operations (if else while) -----------------------------------------
// ----------------------------------------------------------------------------

expr oper_lin  (expr e);                    // !x
int oper_lanor(int et1, int et2, int type); // x&&y, x||y

// ----------------------------------------------------------------------------
// bitwise logical-gate operations (~ & |) ------------------------------------
// ----------------------------------------------------------------------------

expr oper_inv  (expr e);                    // ~x
int oper_bitw (int et1, int et2, int type); // x&y, x|y, x^y

// ----------------------------------------------------------------------------
// bit-shift operations -------------------------------------------------------
// ----------------------------------------------------------------------------

int oper_shift(int et1, int et2, int type); // x>>y, x<<y, x>>>y
