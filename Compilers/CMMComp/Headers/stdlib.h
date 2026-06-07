// ----------------------------------------------------------------------------
// SAPHO standard library -----------------------------------------------------
// ----------------------------------------------------------------------------

// these routines should be called from the .y (parser)

#include "ast.h"   // expr

// ----------------------------------------------------------------------------
// input and output -----------------------------------------------------------
// ----------------------------------------------------------------------------

expr exec_in  (int port);          //  data input
expr exec_fin (int port);          //  data input (converting to float)
void exec_out (int port, expr e);  // data output
void exec_fout(int port, expr e);  // data output (converting to float)

// ----------------------------------------------------------------------------
// special functions that save code -------------------------------------------
// ----------------------------------------------------------------------------

expr exec_sign(expr e1, expr e2); // takes the sign of the first argument and applies it to the second
expr exec_abs (expr e);           // absolute value
expr exec_pst (expr e);           // clears if negative
expr exec_norm(expr e);           // division by constant
void exec_copy(expr e, int id);   // copies the value of the first argument into the second (no type checking)

// ----------------------------------------------------------------------------
// arithmetic functions -------------------------------------------------------
// ----------------------------------------------------------------------------

expr exec_sqrt(expr e);           // square root
expr exec_atan(expr e);           // arctangent
expr exec_sin (expr e);           // sine
expr exec_cos (expr e);           // cosine
expr exec_tan (expr e);           // tangent
expr exec_cosh(expr e);           // hyperbolic cosine
expr exec_sinh(expr e);           // hyperbolic sine
expr exec_tanh(expr e);           // hyperbolic tangent
expr exec_exp (expr e);           // exponential (e^x)
expr exec_log (expr e);           // natural logarithm (ln x)
expr exec_pow (expr ex, expr ey); // power x^y (const int exp: square-and-multiply; int var: loop; else exp(y*ln x))
expr exec_floor(expr e);          // largest integral float <= x
expr exec_ceil (expr e);          // smallest integral float >= x
expr exec_round(expr e);          // nearest integral float, ties away from zero

// ----------------------------------------------------------------------------
// special functions for complex numbers --------------------------------------
// ----------------------------------------------------------------------------

expr exec_real(expr e);           // returns the real part
expr exec_imag(expr e);           // returns the imaginary part
expr exec_mod2(expr e);           // squared magnitude (move to the parser)
expr exec_fase(expr e);           // phase in radians
expr exec_comp(expr er, expr ei); // combines two real numbers into a complex
expr exec_conj(expr e);           // complex conjugate (a-bi); real x -> x+0i; always comp

// ----------------------------------------------------------------------------
// special functions for vector work ------------------------------------------
// ----------------------------------------------------------------------------

expr exec_vtv  (int id1, int id2);                   // multiplication between two vectors
void exec_Mv   (int idy, int idM, int idv);          // matrix-vector multiplication
void exec_cv   (int idy, expr ec, int idv);          // multiplication of a vector by a constant
void exec_apcb (int idy, int ida, expr ec, int idb); // weighted sum into the second vector
void exec_vvt  (int idM, int ida, int idb);          // outer product between two vectors
void exec_Mmvvt(int idA, int idB, int ida, int idb); // matrix subtraction with outer product
void exec_cM   (int idA, expr ec, int idM);          // product of constant and matrix
void exec_cI   (int idM, expr ec);                   // generates the identity matrix
void exec_v0   (int idv);                            // generates a zero vector
void exec_cvin (int idv, expr ec, int idp);          // reads an input vector
void exec_vout (int port, expr ec, int idv);         // writes a vector to the output
void exec_shift(int ida, expr eb, int idc);          // shift register in vector a (a must equal c)
