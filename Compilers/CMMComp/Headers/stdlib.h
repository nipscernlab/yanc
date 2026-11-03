// ----------------------------------------------------------------------------
// SAPHO standard library -----------------------------------------------------
// ----------------------------------------------------------------------------

// these routines should be called from the .y (parser)

#ifndef CMM_STDLIB_H
#define CMM_STDLIB_H

#include "ast.h"   // expr

// ----------------------------------------------------------------------------
// input and output -----------------------------------------------------------
// ----------------------------------------------------------------------------

expr builtin_in  (int port);          //  data input
expr builtin_fin (int port);          //  data input (converting to float)
void builtin_out (int port, expr e);  // data output
void builtin_fout(int port, expr e);  // data output (converting to float)
int  link_use(int id, int sync);   // the link number of a toma/cade (id: its INUM); records plain / _sync
void link_sync_emit(void);             // #LKSYNC k for every link used with a _sync form
void builtin_toma(expr e, int k);     // toma(k, e): hands e over link k (docs/design/toma-and-cade-links.md)
expr builtin_cade(int dest, int k);   // x = cade(k): takes link k's word for x, as x's type
expr builtin_cade_expr(int type, int k); // cade(k) (1, int) / fcade(k) (2, float) inside an expression

// ----------------------------------------------------------------------------
// special functions that save code -------------------------------------------
// ----------------------------------------------------------------------------

expr builtin_sign(expr e1, expr e2); // takes the sign of the first argument and applies it to the second
expr builtin_abs (expr e);           // absolute value
expr builtin_pst (expr e);           // clears if negative
expr builtin_norm(expr e);           // division by constant
void builtin_copy(expr e, int id);   // copies the value of the first argument into the second (no type checking)

// ----------------------------------------------------------------------------
// arithmetic functions -------------------------------------------------------
// ----------------------------------------------------------------------------

expr builtin_sqrt(expr e);           // square root
expr builtin_atan(expr e);           // arctangent
expr builtin_sin (expr e);           // sine
expr builtin_cos (expr e);           // cosine
expr builtin_tan (expr e);           // tangent
expr builtin_cosh(expr e);           // hyperbolic cosine
expr builtin_sinh(expr e);           // hyperbolic sine
expr builtin_tanh(expr e);           // hyperbolic tangent
expr builtin_exp (expr e);           // exponential (e^x)
expr builtin_log (expr e);           // natural logarithm (ln x)
expr builtin_pow (expr ex, expr ey); // power x^y (const int exp: square-and-multiply; int var: loop; else exp(y*ln x))
expr builtin_floor(expr e);          // largest integral float <= x
expr builtin_ceil (expr e);          // smallest integral float >= x
expr builtin_round(expr e);          // nearest integral float, ties away from zero

// ----------------------------------------------------------------------------
// special functions for complex numbers --------------------------------------
// ----------------------------------------------------------------------------

expr builtin_real(expr e);           // returns the real part
expr builtin_imag(expr e);           // returns the imaginary part
expr builtin_mod2(expr e);           // squared magnitude (move to the parser)
expr builtin_fase(expr e);           // phase in radians
expr builtin_comp(expr er, expr ei); // combines two real numbers into a complex
expr builtin_conj(expr e);           // complex conjugate (a-bi); real x -> x+0i; always comp

// ----------------------------------------------------------------------------
// the Dirac shift register (the other forms: dirac.c) -------------------------
// ----------------------------------------------------------------------------

void builtin_shift(int ida, expr eb, int idc, int later); // shift register in vector a (a must equal c)
void builtin_shift_in(int ida, expr e);                   // later: the value, computed after the shift, into a[0]

#endif // CMM_STDLIB_H
