// ----------------------------------------------------------------------------
// data conversion and handling -----------------------------------------------
// ----------------------------------------------------------------------------

// data conversion ------------------------------------------------------------

#ifndef CMM_CONVERT_H
#define CMM_CONVERT_H

char* int_to_bin(int x, int w);                           // converts an integer to a binary string

#include "ast.h"   // expr

// helper functions for accessing terminal types ------------------------------

void get_cmp_parts(expr e, expr *er, expr *ei);
void load_as(int ty, expr e);                       // e into the acc as type ty (TY_INT, TY_FLOAT, TY_COMP)    // either of the two below, by the type of e
int  get_img_id (int id);                           // gets the id of the imaginary part
void get_cmp_cst(expr e, expr *er, expr *ei);       // generates exprs for a complex constant
void get_cmp_ets(expr e, expr *er, expr *ei);       // gets the exprs of a complex variable

#endif // CMM_CONVERT_H
