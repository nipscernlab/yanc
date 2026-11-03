// ----------------------------------------------------------------------------
// data conversion and handling -----------------------------------------------
// ----------------------------------------------------------------------------

// data conversion ------------------------------------------------------------

#ifndef CMM_T2T_H
#define CMM_T2T_H

char* int_to_bin(int x, int w);                           // converts an integer to a binary string

#include "ast.h"   // expr

// helper functions for accessing terminal types ------------------------------

int  get_img_id (int id);                           // gets the id of the imaginary part
void get_cmp_cst(expr e, expr *er, expr *ei);       // generates exprs for a complex constant
void get_cmp_ets(expr e, expr *er, expr *ei);       // gets the exprs of a complex variable

#endif // CMM_T2T_H
