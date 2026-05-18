// ----------------------------------------------------------------------------
// routines for jump implementation -------------------------------------------
// ----------------------------------------------------------------------------

#include "ast.h"   // expr

// if/else --------------------------------------------------------------------
// if_stmt / if_fim return the freshly built AST node; the grammar wraps it
// in stmt_emit_inline(stmt_ast_wrap(...)) so codegen flows through stmt_emit.

void       if_exp (expr e);          // if start
ast_node *if_stmt();                 // if without else (returns its AST_IF)
void     else_stmt();                // before the else stmts
ast_node *if_fim ();                 // end of if/else (returns its AST_IF)

// while ----------------------------------------------------------------------

void       while_expp();             // the while keyword alone - emits a label here
void       while_expexp(expr e);     // evaluates exp and emits a JIZ to decide whether to enter or not
ast_node *while_stmt();              // end of while (returns its AST_WHILE)
ast_node *exec_break();              // break; inside a while (returns its AST_BREAK)

// switch/case ----------------------------------------------------------------

void       case_test (int id, int type); // tests whether this is the correct case
void     defaut_test ();             // tests whether this is the default
void     switch_break();             // a switch/case break was found
void     exec_switch (expr e);       // switch/case start
ast_node *end_switch ();             // switch/case end (returns its AST_SWITCH)
