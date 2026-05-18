// ----------------------------------------------------------------------------
// routines for jump implementation -------------------------------------------
// ----------------------------------------------------------------------------

#include "ast.h"   // expr / expr_node / stmt_node

// if/else --------------------------------------------------------------------
// if_stmt / if_fim hand the freshly built STMT_IF back to the grammar, which
// wraps it via stmt_emit_inline so it joins the enclosing body's stmt_list.

void       if_exp   (expr_node *cond);   // if (cond) opens its pending STMT_IF
stmt_node *if_stmt  (void);              // if without else (returns STMT_IF)
void       else_stmt(void);              // between then and else: switch body lists
stmt_node *if_fim   (void);              // if/else (returns the completed STMT_IF)

// while ----------------------------------------------------------------------

void       while_expp  (void);              // WHILE keyword: pending STMT_WHILE
void       while_expexp(expr_node *cond);   // cond + body-list open
stmt_node *while_stmt  (void);              // returns the STMT_WHILE
stmt_node *exec_break  (void);              // STMT_BREAK_WHILE for break;

// switch/case ----------------------------------------------------------------

void       case_test   (int val_id, int val_type); // builds STMT_CASE_LABEL
void       defaut_test (void);                     // builds STMT_DEFAULT_LABEL
void       switch_break(void);                     // builds STMT_SWITCH_BREAK
void       exec_switch (expr_node *cond);          // opens STMT_SWITCH
stmt_node *end_switch  (void);                     // returns the STMT_SWITCH
