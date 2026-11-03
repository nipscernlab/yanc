// ----------------------------------------------------------------------------
// assignment instructions (left-hand side) -----------------------------------
// ----------------------------------------------------------------------------

#ifndef CMM_DATA_ASSIGN_H
#define CMM_DATA_ASSIGN_H

#include "ast.h"   // expr

void assign_set  (int id, expr e);           // standard assignment
void assign_array(int id, expr e , int fft); // array assignment
extern int assign_idx_reload;                 // an index variable assign_array may read again (0: none; ast.c)
void assign_array_const(int id, int k, int dims, expr e); // arr[const] or m[const][const] = rhs, direct SET_V (no indirect addressing)
void assign_pplus(int id);                   // i++ assignment
void assign_aplus(int id, expr e);           // i++ assignment for a 1D array
void assign_apl2d(int id, expr e1, expr e2); // i++ assignment for a 2D array

#endif // CMM_DATA_ASSIGN_H
