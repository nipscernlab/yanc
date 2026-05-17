// ----------------------------------------------------------------------------
// routines for variable use (right-hand side) --------------------------------
// reduces terminals into exp -------------------------------------------------
// ----------------------------------------------------------------------------

// Legacy packing for the int-based "et" carrier still used by oper_*,
// data_assign_*, exec_*, arr_*, etc. that have not been migrated to expr
// yet. The bridges expr_to_et / expr_of_et in ast.h round-trip across it.
//   OFST    -> reduced integer
// 2*OFST    -> reduced float
// 3*OFST    -> reduced comp
//   OFST+id -> int identifier
// 2*OFST+id -> float identifier
// 3*OFST+id -> comp identifier
// 5*OFST+id -> const comp identifier
#define OFST 1000000

#include "ast.h"   // expr

expr    num2exp(int id, int dtype);          // reduces a number       into expr
expr     id2exp(int id);                     // reduces an identifier  into expr
expr   pplus2exp(int id);                    // reduces an i++         into expr
expr pplus1d2exp(int id, expr ete);          // reduces an x[i]++      into expr
expr pplus2d2exp(int id, expr e1, expr e2);  // reduces an x[i][j]++   into expr
