// ----------------------------------------------------------------------------
// assignment instructions (left-hand side) -----------------------------------
// ----------------------------------------------------------------------------

#ifndef CMM_DATA_ASSIGN_H
#define CMM_DATA_ASSIGN_H

#include "ast.h"   // expr

void assign_set  (int id, expr e);           // standard assignment
void assign_array(int id, expr e , int fft); // array assignment
void assign_array_const(int id, int k, int dims, expr e); // arr[const] or m[const][const] = rhs, direct SET_V (no indirect addressing)
void assign_pplus(int id, int dec);                   // i++ (i-- with dec) assignment
void assign_aplus(int id, expr e, int dec);           // i++ (i--) assignment for a 1D array
void assign_apl2d(int id, expr e1, expr e2, int dec); // i++ (i--) assignment for a 2D array

#endif // CMM_DATA_ASSIGN_H
