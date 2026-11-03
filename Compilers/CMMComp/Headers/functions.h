// ----------------------------------------------------------------------------
// routines and state variables for the function parser -----------------------
// ----------------------------------------------------------------------------

#ifndef CMM_FUNCTIONS_H
#define CMM_FUNCTIONS_H

#include "ast.h"   // expr
#include "context.h"

// state variables ------------------------------------------------------------

extern int  fun_id;                 // stores the id of the function being used

// type-checks argument k (from 0) against the expected parameter and emits
// LOD/SET to stage it for the upcoming CAL. Read by the EXPR_FUNC_CALL walker.
void par_check(expr e, int k);

// call graph: every CAL the walker emits is an edge caller -> callee, and
// check_rec() stops the build on a cycle -- C+- locals live at fixed
// addresses, so a recursive call runs over its caller's values
void fun_call_edge(int caller, int callee, int line);
void check_rec(void);

// declaration ----------------------------------------------------------------

void  declare_fun(int id1, int id2); // type and name (state only - emit is deferred)
int   declare_par(int   t, int id ); // appends to the deferred STMT_FUNC_HEADER param list
void  declare_ret(expr e , int ret); // a return keyword was found
void func_body_begin(void);         // fires at the function's '{', builds STMT_FUNC_HEADER
void    func_ret(int id);           // end/return of the function
void    void_ret(void);           // end of a void function

// usage ----------------------------------------------------------------------

void       par_exp    (expr_node *n); // record first  arg's node on the frame
void       par_listexp(expr_node *n); // record next-N arg's node on the frame
stmt_node *vcall      (int id);       // void user call: packages args into a STMT_VOID_CALL
expr_node *fcall      (int id);       // value user call: builds and returns node

// argument-stack frames for nested function calls. args_frame_push() is
// called from the grammar mid-rule action right after 'ID ('; par_exp /
// par_listexp push each arg's node onto the frame; fcall / vcall drain the
// frame when ')' is reduced.
void  args_frame_push(void);

#endif // CMM_FUNCTIONS_H
