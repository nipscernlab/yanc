// ----------------------------------------------------------------------------
// array index handling -------------------------------------------------------
// ----------------------------------------------------------------------------

#include "ast.h"   // expr

// array used in assignments (left-hand side) ---------------------------------

void arr_1d_index(int id, expr e);              // gets the index of a 1D array
void arr_2d_index(int id, expr e1, expr e2);    // gets the index of a 2D array

// array used in expressions (right-hand side) --------------------------------

expr arr_1d2exp  (int id, expr e , int fft);    // turns a 1D array into an expr
expr arr_2d2exp  (int id, expr e1, expr e2);    // turns a 2D array into an expr
expr arr_1d2exp_const(int id, int k);           // arr[const] read, direct LOD_V (no indirect addressing)
