// ----------------------------------------------------------------------------
// variable declaration -------------------------------------------------------
// ----------------------------------------------------------------------------

#include "ast.h"   // expr / expr_node / stmt_node

extern int type_tmp; // captures the type when a variable is declared (see c2asm.l)

// ----------------------------------------------------------------------------
// standard declarations ------------------------------------------------------
// ----------------------------------------------------------------------------
// Public entries do the parse-time v_table updates (so subsequent declarations
// can resolve this var) and defer the `#array` directive emit through a
// stmt_node so it lands in source order with the surrounding deferred stmts.

void declar_var   (int id);                                          // variable declaration
void declar_arr_1d(int id_var , int id_arg,           int id_fname); // 1D array declaration
void declar_arr_2d(int id_var , int id_x  , int id_y, int id_fname); // 2D array declaration

// Walker-time helpers used by the STMT_DECLAR_ARR_* walker cases in ast.c.
// Emit only the `#array` directive(s) (plus arr_size LOD/SET for 2D). The
// v_table is already populated by the parse-time half.
void declar_arr_1d_emit(int id_var, int id_arg, int id_fname);
void declar_arr_2d_emit(int id_var, int id_x, int id_y, int id_fname);

// ----------------------------------------------------------------------------
// array declarations initialized with Dirac notation -------------------------
// ----------------------------------------------------------------------------

void declar_Mv    (int id_name, int id_N  , int id_M, int id_v       ); // e.g. float a[4] # |B|a>;
void declar_cv    (int id_name, int id_N  , expr_node *c, int id_v   ); // e.g. float a[4] # c|a>;
