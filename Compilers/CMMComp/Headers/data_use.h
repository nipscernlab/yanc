// ----------------------------------------------------------------------------
// routines for variable use (right-hand side) --------------------------------
// reduces terminals into expr ------------------------------------------------
// ----------------------------------------------------------------------------

#include "ast.h"   // expr

expr    num2exp(int id, int dtype);          // reduces a number       into expr
expr     id2exp(int id);                     // reduces an identifier  into expr
expr   pplus2exp(int id);                    // reduces an i++         into expr
expr pplus1d2exp(int id, expr ete);          // reduces an x[i]++      into expr
expr pplus2d2exp(int id, expr e1, expr e2);  // reduces an x[i][j]++   into expr
