// ----------------------------------------------------------------------------
// routines for variable use (right-hand side) --------------------------------
// reduces terminals into expr ------------------------------------------------
// ----------------------------------------------------------------------------

#ifndef CMM_DATA_USE_H
#define CMM_DATA_USE_H

#include "ast.h"   // expr

expr    num2exp(int id, int dtype);          // reduces a number       into expr
expr     id2exp(int id);                     // reduces an identifier  into expr
// old = 1: postfix ++ inside an expression, whose value is the one BEFORE the
// increment (C); old = 0: the statement `x++;`, which needs no value
expr   pplus2exp(int id, int old, int dec);           // reduces an i++ (i-- with dec)    into expr
expr pplus1d2exp(int id, expr ete, int old, int dec); // reduces an x[i]++ (x[i]--)       into expr
expr pplus2d2exp(int id, expr e1, expr e2, int old, int dec); // reduces an x[i][j]++ (x[i][j]--) into expr

#endif // CMM_DATA_USE_H
