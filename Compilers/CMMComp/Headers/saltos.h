// ----------------------------------------------------------------------------
// routines for jump implementation -------------------------------------------
// ----------------------------------------------------------------------------

#include "ast.h"   // expr / expr_node / stmt_node

// if/else --------------------------------------------------------------------
// if_stmt / if_fim hand the freshly built STMT_IF back to the grammar, which
// wraps it via stmt_append so it joins the enclosing body's stmt_list.

void       if_exp   (expr_node *cond);   // if (cond) opens its pending STMT_IF
stmt_node *if_stmt  (void);              // if without else (returns STMT_IF)
void       else_stmt(void);              // between then and else: switch body lists
stmt_node *if_fim   (void);              // if/else (returns the completed STMT_IF)

// while ----------------------------------------------------------------------

void       while_expp  (void);              // WHILE keyword: pending STMT_WHILE
void       while_expexp(expr_node *cond);   // cond + body-list open
stmt_node *while_stmt  (void);              // returns the STMT_WHILE
stmt_node *exec_break  (void);              // break; -> innermost loop or switch
stmt_node *exec_continue(void);             // continue; -> innermost loop

// do { body } while (cond); -- body runs once before the condition is tested.
void       do_open     (void);              // DO keyword: pending STMT_DO + body list
stmt_node *do_finish   (expr_node *cond);   // returns the STMT_DO

// for ------------------------------------------------------------------------
//
// for (init; cond; step) body desugars to: init; while(cond) { body; step; }
// for_init_set / for_step_set stash the init / step stmt_node in saltos.c
// statics; for_open consumes them into the pending while's then_body /
// else_body fields so nested fors don't trample each other.

void       for_init_set(stmt_node *init);   // for (init; ...) clause; NULL for empty
void       for_step_set(stmt_node *step);   // for (...; step) clause; NULL for empty
void       for_open    (expr_node *cond);   // FOR ( ... )  - pending STMT_WHILE + body-list open
stmt_node *for_finish  (void);              // returns the desugared STMT_WHILE

// switch/case ----------------------------------------------------------------

void       case_test   (int val_id, int val_type); // builds STMT_CASE_LABEL
void       defaut_test (void);                     // builds STMT_DEFAULT_LABEL
void       exec_switch (expr_node *cond);          // opens STMT_SWITCH
stmt_node *end_switch  (void);                     // returns the STMT_SWITCH
// break (in a loop OR a case) is built by exec_break above via the break-target
// stack -- there is no separate switch_break entry point any more.
