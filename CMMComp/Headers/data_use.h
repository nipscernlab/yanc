// ----------------------------------------------------------------------------
// routines for variable use (right-hand side) --------------------------------
// reduces terminals into exp -------------------------------------------------
// ----------------------------------------------------------------------------

// return value for exp (see the exp rule in the .y)
//   OFST    -> reduced integer
// 2*OFST    -> reduced float
// 3*OFST    -> reduced comp
//   OFTS+id -> int identifier
// 2*OFST+id -> float identifier
// 3*OFST+id -> comp identifier
// 5*OFST+id -> const comp identifier
#define OFST 1000000

int      num2exp(int id, int dtype);         // reduces a number       into exp
int       id2exp(int id);                    // reduces an identifier  into exp
int    pplus2exp(int et);                    // reduces an i++         into exp
int  pplus1d2exp(int id, int et);            // reduces an x[i]++      into exp
int  pplus2d2exp(int id, int et1, int et2);  // reduces an x[i][j]++   into exp
