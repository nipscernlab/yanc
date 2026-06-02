// ----------------------------------------------------------------------------
// routines for jump implementation -------------------------------------------
// ----------------------------------------------------------------------------
//
// Each compound statement (if / while / switch) is built incrementally as the
// parser sees its parts. We push a partial stmt_node onto a pending stack at
// the opening token, fill in the body / else / etc. as the inner statements
// reduce, and pop the completed node at the closing reduce. The grammar then
// hands the node to stmt_append, which appends it to the enclosing
// stmt_list. The walker emits the cond, labels and body at body-close time.

#include <stdlib.h>

#include "../Headers/ast.h"
#include "../Headers/labels.h"
#include "../Headers/global.h"
#include "../Headers/variaveis.h"
#include "../Headers/messages.h"

// switch/case state variables
int switching = 0;
int case_cnt  = 0;
int swit_cnt  = 0;

// ----------------------------------------------------------------------------
// pending compound-statement stack -------------------------------------------
// ----------------------------------------------------------------------------

static stmt_node **pending_stack     = NULL;
static int         pending_stack_n   = 0;
static int         pending_stack_cap = 0;

static void pending_push(stmt_node *n)
{
    if (pending_stack_n + 1 > pending_stack_cap)
    {
        int new_cap = pending_stack_cap ? pending_stack_cap * 2 : 8;
        stmt_node **t = realloc(pending_stack, (size_t)new_cap * sizeof(*t));
        if (!t) {fprintf(stderr, MSG_ERR_OUT_OF_MEMORY); exit(EXIT_FAILURE);}
        pending_stack     = t;
        pending_stack_cap = new_cap;
    }
    pending_stack[pending_stack_n++] = n;
}

static stmt_node *pending_pop(void)
{
    return pending_stack[--pending_stack_n];
}

// ----------------------------------------------------------------------------
// if/else --------------------------------------------------------------------
// ----------------------------------------------------------------------------

void if_exp(expr_node *cond)
{
    int label = push_if();
    stmt_node *partial = stmt_if(label, cond, NULL, NULL);
    pending_push(partial);
    stmt_list_open();  // accumulate the then-body
}

stmt_node *if_stmt()
{
    stmt_node *then_body = stmt_list_close();
    pop_if();
    stmt_node *partial   = pending_pop();
    partial->then_body   = then_body;
    return partial;
}

void else_stmt()
{
    stmt_node *then_body = stmt_list_close();
    stmt_node *partial   = pending_pop();
    partial->then_body   = then_body;
    pending_push(partial);  // keep open until if_fim
    stmt_list_open();       // accumulate the else-body
}

stmt_node *if_fim()
{
    stmt_node *else_body = stmt_list_close();
    pop_if();
    stmt_node *partial   = pending_pop();
    partial->else_body   = else_body;
    return partial;
}

// ----------------------------------------------------------------------------
// while ----------------------------------------------------------------------
// ----------------------------------------------------------------------------

void while_expp()
{
    int label = push_while();
    stmt_node *partial = stmt_while(label, NULL, NULL);
    pending_push(partial);
    // cond comes in while_expexp; body list opens there too
}

void while_expexp(expr_node *cond)
{
    stmt_node *partial = pending_pop();
    partial->cond      = cond;
    pending_push(partial);
    stmt_list_open();  // accumulate the body
}

stmt_node *while_stmt()
{
    stmt_node *body    = stmt_list_close();
    pop_while();
    stmt_node *partial = pending_pop();
    partial->body      = body;
    return partial;
}

// ----------------------------------------------------------------------------
// for ------------------------------------------------------------------------
// ----------------------------------------------------------------------------
//
// for (init; cond; step) body
//   desugars at parse time into:
//     init;
//     while (cond) { body; step; }
//
// for_init_clause / for_step_clause stash their stmt_node in these globals;
// for_open pulls them into the pending stmt_while's then_body / else_body
// slots (those fields are unused for while), so nested fors each carry
// their own init/step without colliding.

static stmt_node *for_saved_init = NULL;
static stmt_node *for_saved_step = NULL;

void for_init_set(stmt_node *init) {for_saved_init = init;}
void for_step_set(stmt_node *step) {for_saved_step = step;}

void for_open(expr_node *cond)
{
    int label = push_while();
    stmt_node *partial = stmt_while(label, cond, NULL);
    // Park init/step on the pending while so a nested for can reuse the
    // globals freely. for_finish reads them back.
    partial->then_body = for_saved_init;
    partial->else_body = for_saved_step;
    pending_push(partial);
    stmt_list_open();

    for_saved_init = NULL;
    for_saved_step = NULL;
}

stmt_node *for_finish()
{
    stmt_node *body    = stmt_list_close();
    pop_while();
    stmt_node *partial = pending_pop();

    stmt_node *init = partial->then_body;
    stmt_node *step = partial->else_body;
    partial->then_body = NULL;
    partial->else_body = NULL;

    if (step) stmt_block_push(body, step);
    partial->body = body;

    // Insert init BEFORE the while into the parent stmt_list. The grammar's
    // rule end-action appends the returned while right after, so the final
    // sequence on the parent list is: init then while.
    if (init) stmt_append(init);

    return partial;
}

stmt_node *exec_break()
{
    if (get_while() == 0)
    {
        fprintf(stderr, MSG_ERR_BREAK_LOST, line_num+1);
        exit(EXIT_FAILURE);
    }
    return stmt_break_while(get_while());
}

// ----------------------------------------------------------------------------
// switch/case ----------------------------------------------------------------
// ----------------------------------------------------------------------------

void exec_switch(expr_node *cond)
{
    if (switching == 1)
    {
        fprintf(stderr, MSG_ERR_NESTED_SWITCH, line_num+1);
        exit(EXIT_FAILURE);
    }

    // pre-declare the implicit switch_exp variable so case_test (at parse
    // time) can record its case_idx referencing it; the walker fills in
    // v_type at emit time once it knows the cond's evaluated type.
    if (find_var("switch_exp") == -1) add_var("switch_exp");

    swit_cnt++;
    case_cnt  = 0;
    switching = 1;

    stmt_node *partial = stmt_switch(swit_cnt, /*case_max=*/0, cond, NULL);
    pending_push(partial);
    stmt_list_open();  // accumulate body (case labels, stmts, breaks, defaults)
}

void case_test(int val_id, int val_type)
{
    case_cnt++;
    stmt_node *top = stmt_list_top();
    if (top) stmt_block_push(top, stmt_case_label(swit_cnt, case_cnt, val_id, val_type));
}

void defaut_test()
{
    case_cnt++;
    stmt_node *top = stmt_list_top();
    if (top) stmt_block_push(top, stmt_default_label(swit_cnt, case_cnt));
}

void switch_break()
{
    stmt_node *top = stmt_list_top();
    if (top) stmt_block_push(top, stmt_switch_break(swit_cnt));
}

stmt_node *end_switch()
{
    stmt_node *body    = stmt_list_close();
    stmt_node *partial = pending_pop();
    partial->body      = body;
    partial->id2       = case_cnt;  // case_max for the trailing label
    switching = 0;
    return partial;
}
