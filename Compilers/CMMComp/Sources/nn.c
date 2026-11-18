// ----------------------------------------------------------------------------
// relu, sigmoid, tanh, softmax and argmax on an array -------------------------
// ----------------------------------------------------------------------------
//
// docs/design/nn-functions.md. C± has no arrays as parameters, so a program
// cannot write these as functions of its own: the compiler takes the name of
// the array and writes the loops a program would write by hand,
//
//     for (k = 0; k < N; k++) v[k] = pset(v[k]);
//
// as statement trees built at walker time (the same STMT_WHILE with its init
// and step that a for desugars into, so the loop gets the for's bottom test)
// and emitted by stmt_emit. Every node is built fresh where it is used: the
// walker caches what a node emitted, so one node cannot stand in two places.
//
// The loop counters and the running maximum and sum are variables of the
// compiler, __nn<stem><n>_<function>, one set per use, as dirac.c names its
// own: a call made inside a loop leaves the caller's alone.

#include <stdio.h>
#include <stdlib.h>

#include "../Headers/ast.h"
#include "../Headers/global.h"
#include "../Headers/symbols.h"
#include "../Headers/labels.h"
#include "../Headers/messages.h"
#include "../Headers/diag.h"
#include "../Headers/acc.h"
#include "../Headers/nn.h"
#include "../../common/xstr.h"

static int n_names = 0;   // numbers the variables of every use apart

const char *nn_name(int fn)
{
    switch (fn) {
        case NN_RELU:    return "relu";
        case NN_SIGM:    return "sigmoid";
        case NN_TANH:    return "tanh";
        case NN_SOFTMAX: return "softmax";
        default:         return "?";
    }
}

// ----------------------------------------------------------------------------
// parse-time checks -----------------------------------------------------------
// ----------------------------------------------------------------------------

static void declared(int v, int line)
{
    if (v_table[v].type == TY_NONE) fatal(MSG_ERR_DECL_FIRST, line, strip_fn_prefix(v_table[v].name));
}

void nn_check(int fn, int v, int line)
{
    const char *f = nn_name(fn);
    declared(v, line);
    char *name = strip_fn_prefix(v_table[v].name);
    if (v_table[v].dims == AR_NONE)
    {
        // a number: relu, sigmoid and tanh exist as expressions, softmax not
        if (fn == NN_SOFTMAX) fatal(MSG_ERR_NN_NOT_VECTOR, line, f, name);
        fatal(MSG_ERR_NN_NOT_ARRAY, line, f, name, name, f);
    }
    if (v_table[v].type > TY_FLOAT) fatal(MSG_ERR_FUNC_COMPLEX, line, f);
    if (fn == NN_SOFTMAX && v_table[v].dims != AR_1D) fatal(MSG_ERR_NN_NOT_VECTOR, line, f, name);
    if (fn != NN_RELU && v_table[v].type == TY_INT) fatal(MSG_ERR_NN_INT, line, f, name);
}

void nn_check_argmax(int v, int line)
{
    declared(v, line);
    char *name = strip_fn_prefix(v_table[v].name);
    if (v_table[v].dims != AR_1D) fatal(MSG_ERR_NN_NOT_VECTOR, line, "argmax", name);
    if (v_table[v].type > TY_FLOAT) fatal(MSG_ERR_FUNC_COMPLEX, line, "argmax");
}

// ----------------------------------------------------------------------------
// pieces of the trees ---------------------------------------------------------
// ----------------------------------------------------------------------------

// a scalar variable of the compiler, of type t
static int var(const char *stem, int t)
{
    char *name = xasprintf("__nn%s%d_%s", stem, n_names++, ectx.emit_fname);
    int id = find_var(name);
    if (id < 0) {add_var(name); id = find_var(name);}
    free(name);
    v_table[id].type = t;
    v_table[id].used = 1;
    return id;
}

static expr_node *ival(int k)
{
    char buf[24]; snprintf(buf, sizeof buf, "%d", k);
    return expr_lit(TY_INT, lex_inum(buf));
}

static expr_node *fval(const char *text) {return expr_lit(TY_FLOAT, lex_fnum(text));}

static expr_node *rd(int id) {return expr_var(v_table[id].type, id);}

// v[r] (c < 0) or v[r][c]: r and c are the counters
static expr_node *elem(int v, int r, int c)
{
    return expr_array_index(v_table[v].type, v, 0, rd(r), c < 0 ? NULL : rd(c));
}

static stmt_node *set_elem(int v, int r, int c, expr_node *rhs)
{
    return stmt_array_assign(v, rd(r), c < 0 ? NULL : rd(c), rhs, 0);
}

// a block of statements, as the parser makes one
static stmt_node *block(stmt_node *a, stmt_node *b, stmt_node *c)
{
    stmt_list_open();
    if (a) stmt_append(a);
    if (b) stmt_append(b);
    if (c) stmt_append(c);
    return stmt_list_close();
}

// for (k = from; k < to; k++) body, as for_open / for_finish build it: the
// init stays on then_body as a reference, so that the walker sees the entry
// test holds and tests at the bottom only
static stmt_node *loop(int k, int from, int to, stmt_node *body)
{
    stmt_node *init = stmt_assign(k, ival(from));
    stmt_node *w    = stmt_while(push_while(), expr_binop(OP_LT, rd(k), ival(to)));
    w->then_body = init;
    w->else_body = stmt_pplus(k, NULL, NULL, 0);
    w->body      = body;
    return block(init, w, NULL);
}

// if (v[k] > m) {m = v[k]; then}
static stmt_node *if_greater(int v, int k, int m, stmt_node *then)
{
    stmt_node *s = stmt_if(push_if(), expr_binop(OP_GT, elem(v, k, -1), rd(m)));
    s->then_body = block(stmt_assign(m, elem(v, k, -1)), then, NULL);
    return s;
}

// the function of fn on one element
static expr_node *apply(int fn, expr_node *x)
{
    switch (fn) {
        case NN_RELU: return expr_stdlib(OP_STD_PST,  0, x, NULL);
        case NN_SIGM: return expr_stdlib(OP_STD_SIGM, 0, x, NULL);
        default:      return expr_stdlib(OP_STD_TANH, 0, x, NULL);
    }
}

// ----------------------------------------------------------------------------
// the statements --------------------------------------------------------------
// ----------------------------------------------------------------------------

// v[k] = f(v[k]) on every element: one loop for a vector, two for a matrix
static stmt_node *elementwise(int fn, int v)
{
    int r = var("k", TY_INT);
    if (v_table[v].dims == AR_1D)
        return loop(r, 0, v_table[v].size, block(set_elem(v, r, -1, apply(fn, elem(v, r, -1))), NULL, NULL));
    int c = var("k", TY_INT);
    stmt_node *inner = loop(c, 0, v_table[v].cols, block(set_elem(v, r, c, apply(fn, elem(v, r, c))), NULL, NULL));
    return loop(r, 0, v_table[v].size, inner);
}

// m = v[0]; for (k = 1; k < N; k++) if (v[k] > m) {m = v[k]; then(k)}
// with then(k) = b = k when b >= 0
static stmt_node *running_max(int v, int m, int b)
{
    int k = var("k", TY_INT);
    stmt_node *then = (b >= 0) ? stmt_assign(b, rd(k)) : NULL;
    stmt_node *first = stmt_assign(m, expr_array_index(v_table[v].type, v, 0, ival(0), NULL));
    return block(first, loop(k, 1, v_table[v].size, block(if_greater(v, k, m, then), NULL, NULL)), NULL);
}

// m = max(v); s = 0.0; for k: {v[k] = exp(v[k] - m); s = s + v[k];}
// s = 1.0 / s; for k: v[k] = v[k] * s
//
// The maximum goes first because the SAPHO float has no infinity: exp of a
// large element passes the largest float, and with every element at or below
// 0 after the subtraction, each exp is at most 1 and the sum at most N. One
// division, then products: a divider for one word, not one per element.
static stmt_node *softmax(int v)
{
    int n = v_table[v].size;
    int m = var("m", TY_FLOAT), s = var("s", TY_FLOAT);
    int k1 = var("k", TY_INT), k2 = var("k", TY_INT);

    stmt_node *e = set_elem(v, k1, -1,
                            expr_stdlib(OP_STD_EXP, 0, expr_binop(OP_SUB, elem(v, k1, -1), rd(m)), NULL));
    stmt_node *acc = stmt_assign(s, expr_binop(OP_ADD, rd(s), elem(v, k1, -1)));
    stmt_node *pass1 = block(stmt_assign(s, fval("0.0")), loop(k1, 0, n, block(e, acc, NULL)), NULL);

    stmt_node *inv = stmt_assign(s, expr_binop(OP_DIV, fval("1.0"), rd(s)));
    stmt_node *pass2 = loop(k2, 0, n, block(set_elem(v, k2, -1, expr_binop(OP_MUL, elem(v, k2, -1), rd(s))), NULL, NULL));

    stmt_list_open();
    stmt_append(running_max(v, m, -1));
    stmt_append(pass1);
    stmt_append(inv);
    stmt_append(pass2);
    return stmt_list_close();
}

void nn_emit(int fn, int v)
{
    stmt_emit(fn == NN_SOFTMAX ? softmax(v) : elementwise(fn, v));
    v_table[v].used = 1;
}

// b = 0; m = v[0]; for (k = 1; k < N; k++) if (v[k] > m) {m = v[k]; b = k;}
// The first of several equal elements wins: only a larger one moves b.
expr nn_argmax_emit(int v)
{
    // a live partial result, x + 1 in x + 1 + argmax(v), goes to the stack,
    // as for a call: the loops take the acc
    if (acc_live) add_instr("PSH\n");
    acc_live = 0;

    int b = var("b", TY_INT), m = var("m", v_table[v].type);
    stmt_emit(block(stmt_assign(b, ival(0)), running_max(v, m, b), NULL));

    add_instr("LOD %s\n", v_table[b].name);
    acc_live = 1;
    v_table[v].used = 1;
    return expr_make(TY_INT, 0);
}
