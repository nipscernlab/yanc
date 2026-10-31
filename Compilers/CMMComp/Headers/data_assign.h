// ----------------------------------------------------------------------------
// assignment instructions (left-hand side) -----------------------------------
// ----------------------------------------------------------------------------

#include "ast.h"   // expr

void ass_set  (int id, expr e);           // standard assignment
void ass_array(int id, expr e , int fft); // array assignment
extern int ass_idx_reload;                 // an index variable ass_array may read again (0: none; ast.c)
void ass_array_const(int id, int k, int dims, expr e); // arr[const] or m[const][const] = rhs, direct SET_V (no indirect addressing)
void ass_pplus(int id);                   // i++ assignment
void ass_aplus(int id, expr e);           // i++ assignment for a 1D array
void ass_apl2d(int id, expr e1, expr e2); // i++ assignment for a 2D array
