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

// Monotonic switch id: a unique label namespace per switch (sw_case_<id>_* /
// switch_end_<id>). Per-switch state -- the running case count -- lives on the
// switch's stmt_node->id2; break routing lives on the break-target stack
// below. Nested switches therefore share no mutable global counter.
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
// break-target stack ---------------------------------------------------------
// ----------------------------------------------------------------------------
//
// Every loop (while / for) and every switch pushes one entry while its body is
// parsed; `break` (exec_break) resolves to the top, so it always exits the
// innermost enclosing loop OR switch -- the standard C rule, done in the
// semantic layer instead of via a grammar tier. A loop entry carries its while
// label and emits `JMP Lwh<id>end`; a switch entry carries its swit id and
// emits `JMP switch_end<id>`.

typedef struct { int is_switch; int id; } brk_target;
static brk_target *brk_stk  = NULL;
static int         brk_n    = 0;
static int         brk_cap  = 0;

static void brk_push(int is_switch, int id)
{
    if (brk_n + 1 > brk_cap)
    {
        int new_cap = brk_cap ? brk_cap * 2 : 8;
        brk_target *t = realloc(brk_stk, (size_t)new_cap * sizeof(*t));
        if (!t) {fprintf(stderr, MSG_ERR_OUT_OF_MEMORY); exit(EXIT_FAILURE);}
        brk_stk  = t;
        brk_cap  = new_cap;
    }
    brk_stk[brk_n].is_switch = is_switch;
    brk_stk[brk_n].id        = id;
    brk_n++;
}

static void brk_pop(void) {brk_n--;}

// ----------------------------------------------------------------------------
// continue-target stack ------------------------------------------------------
// ----------------------------------------------------------------------------
//
// Only loops push here (switches do not catch `continue`), so exec_continue
// resolves to the innermost enclosing LOOP. Each entry keeps the loop's
// stmt_node plus cont_lbl: a while-continue jumps to the loop top (re-test the
// condition), while a for- or do-while-continue jumps to the Lwh<n>cont label
// (the for runs its step there, the do-while re-evaluates its bottom test).
// When such a loop is actually targeted, exec_continue sets node->op so the
// walker emits that label -- loops without continue stay untouched.

typedef struct { stmt_node *node; int cont_lbl; } cont_target;
static cont_target *cont_stk = NULL;
static int          cont_n   = 0;
static int          cont_cap = 0;

static void cont_push(stmt_node *node, int cont_lbl)
{
    if (cont_n + 1 > cont_cap)
    {
        int new_cap = cont_cap ? cont_cap * 2 : 8;
        cont_target *t = realloc(cont_stk, (size_t)new_cap * sizeof(*t));
        if (!t) {fprintf(stderr, MSG_ERR_OUT_OF_MEMORY); exit(EXIT_FAILURE);}
        cont_stk = t;
        cont_cap = new_cap;
    }
    cont_stk[cont_n].node   = node;
    cont_stk[cont_n].cont_lbl = cont_lbl;
    cont_n++;
}

static void cont_pop(void) {cont_n--;}

// ----------------------------------------------------------------------------
// active-switch stack --------------------------------------------------------
// ----------------------------------------------------------------------------
//
// The switch currently collecting cases. case_test / defaut_test bump its
// running case count (stmt_node->id2) and stamp each label with its swit id;
// nested switches each get their own node, so the counts never collide.

static stmt_node **sw_stk  = NULL;
static int         sw_n    = 0;
static int         sw_cap  = 0;

static void sw_push(stmt_node *n)
{
    if (sw_n + 1 > sw_cap)
    {
        int new_cap = sw_cap ? sw_cap * 2 : 8;
        stmt_node **t = realloc(sw_stk, (size_t)new_cap * sizeof(*t));
        if (!t) {fprintf(stderr, MSG_ERR_OUT_OF_MEMORY); exit(EXIT_FAILURE);}
        sw_stk  = t;
        sw_cap  = new_cap;
    }
    sw_stk[sw_n++] = n;
}

static stmt_node *sw_top(void) {return sw_n ? sw_stk[sw_n - 1] : NULL;}
static void       sw_pop(void) {sw_n--;}

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
    brk_push(0, label);          // break inside the body exits this loop
    cont_push(partial, 0);       // continue jumps to the top (re-test cond)
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
    cont_pop();
    brk_pop();
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
//     while (cond) { body; <step> }
//
// The step is NOT glued onto the end of the body block -- it is kept on the
// while node's else_body and emitted by the STMT_WHILE walker right after the
// body (and after the optional Lwh<n>cont label). That is what makes `continue`
// in a for run the step before the next condition test. The emitted assembly is
// identical to gluing it into the body when there is no continue.
//
// for_init_clause / for_step_clause stash their stmt_node in these globals;
// for_open pulls them into the pending stmt_while's then_body (init) and
// else_body (step) slots, so nested fors each carry their own without colliding.

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
    brk_push(0, label);          // break inside the body exits this loop
    cont_push(partial, 1);       // continue jumps to the step label (Lwh<n>cont)
    stmt_list_open();

    for_saved_init = NULL;
    for_saved_step = NULL;
}

stmt_node *for_finish()
{
    stmt_node *body    = stmt_list_close();
    cont_pop();
    brk_pop();
    pop_while();
    stmt_node *partial = pending_pop();

    stmt_node *init = partial->then_body;
    partial->then_body = NULL;
    // The step stays on partial->else_body; the STMT_WHILE walker emits it after
    // the body (where `continue` lands), so the step always runs.
    partial->body = body;

    // Insert init BEFORE the while into the parent stmt_list. The grammar's
    // rule end-action appends the returned while right after, so the final
    // sequence on the parent list is: init then while.
    if (init) stmt_append(init);

    return partial;
}

// ----------------------------------------------------------------------------
// do/while -------------------------------------------------------------------
// ----------------------------------------------------------------------------
//
// do { body } while (cond);  -- the body runs first and the condition is tested
// at the bottom, so the body always executes at least once. It shares the
// while label namespace (push_while), so break (-> Lwh<n>end) and continue (->
// Lwh<n>cont) ride the same stacks as while/for; cont_lbl=1 because a continue
// re-evaluates the bottom test, landing at the Lwh<n>cont label.

void do_open()
{
    int label = push_while();
    stmt_node *partial = stmt_do(label, NULL, NULL);
    pending_push(partial);
    brk_push(0, label);          // break exits this loop
    cont_push(partial, 1);       // continue lands at Lwh<n>cont (re-test cond)
    stmt_list_open();            // accumulate the body
}

stmt_node *do_finish(expr_node *cond)
{
    stmt_node *body    = stmt_list_close();
    cont_pop();
    brk_pop();
    pop_while();
    stmt_node *partial = pending_pop();
    partial->cond = cond;
    partial->body = body;
    return partial;
}

stmt_node *exec_break()
{
    if (brk_n == 0)  // break with no enclosing loop or switch
    {
        fprintf(stderr, MSG_ERR_BREAK_LOST, line_num+1);
        exit(EXIT_FAILURE);
    }
    brk_target t = brk_stk[brk_n - 1];  // innermost
    return t.is_switch ? stmt_switch_break(t.id) : stmt_break_while(t.id);
}

stmt_node *exec_continue()
{
    if (cont_n == 0)  // continue with no enclosing loop
    {
        fprintf(stderr, MSG_ERR_CONTINUE_LOST, line_num+1);
        exit(EXIT_FAILURE);
    }
    cont_target t = cont_stk[cont_n - 1];  // innermost loop
    // for / do-while need the Lwh<n>cont label emitted at the continue landing
    // point; flag the loop node so its walker drops that label.
    if (t.cont_lbl) t.node->op = 1;
    return stmt_continue(t.node->id, t.cont_lbl);
}

// ----------------------------------------------------------------------------
// switch/case ----------------------------------------------------------------
// ----------------------------------------------------------------------------

void exec_switch(expr_node *cond)
{
    // Pre-declare the implicit switch_exp variable so case_test (at parse
    // time) can record its case_idx referencing it; the walker fills in
    // v_type at emit time once it knows the cond's evaluated type. The variable
    // is shared across all switches, including nested ones, and stays correct:
    // the walker emits all of a switch's compares in a dispatch block before any
    // case body runs, so an inner switch (inside a body) reusing switch_exp can
    // never corrupt an enclosing compare -- the enclosing dispatch is already
    // done by then.
    if (find_var("switch_exp") == -1) add_var("switch_exp");

    swit_cnt++;
    stmt_node *partial = stmt_switch(swit_cnt, /*case_max=*/0, cond, NULL);
    pending_push(partial);
    sw_push(partial);       // active switch for case_test / defaut_test
    brk_push(1, swit_cnt);  // break inside a case exits this switch
    stmt_list_open();       // accumulate body (case labels, stmts, breaks, defaults)
}

void case_test(int val_id, int val_type)
{
    stmt_node *sw  = sw_top();
    stmt_node *top = stmt_list_top();
    if (sw && top)
    {
        sw->id2++;  // running case count (becomes case_max at end_switch)
        stmt_block_push(top, stmt_case_label(sw->id, sw->id2, val_id, val_type));
    }
}

void defaut_test()
{
    stmt_node *sw  = sw_top();
    stmt_node *top = stmt_list_top();
    if (sw && top)
    {
        sw->id2++;
        stmt_block_push(top, stmt_default_label(sw->id, sw->id2));
    }
}

stmt_node *end_switch()
{
    stmt_node *body    = stmt_list_close();
    brk_pop();
    sw_pop();
    stmt_node *partial = pending_pop();
    partial->body      = body;
    // partial->id2 already holds the running case count (case_max) for the
    // trailing @sw_case_<id>_<max+1> / @switch_end label the walker emits.
    return partial;
}
