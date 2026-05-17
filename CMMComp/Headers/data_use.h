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

expr     num2exp(int id, int dtype);         // reduces a number       into expr
expr      id2exp(int id);                    // reduces an identifier  into expr
int    pplus2exp(int et);                    // reduces an i++         into exp (int et)
int  pplus1d2exp(int id, int et);            // reduces an x[i]++      into exp (int et)
int  pplus2d2exp(int id, int et1, int et2);  // reduces an x[i][j]++   into exp (int et)
