// ----------------------------------------------------------------------------
// relu, sigmoid, tanh, softmax and argmax on an array -------------------------
// ----------------------------------------------------------------------------
//
// docs/design/nn-functions.md. C± has no arrays as parameters, so a program
// cannot write these as functions of its own: the compiler takes the name of
// the array and calls a routine of Includes/ (Macros/ in a release), nn_*.asm,
// appended once to the program whatever the number of calls, as float_exp.asm
// is, and shared with cppcomp. A call is four words: the address of the
// array's first element in __nn_p, the address after its last in the acc,
// and the CAL. A matrix is its rows one after the other, so relu, sigmoid and
// tanh walk it as one vector of rows * cols elements.
//
// Measured on 2026-11-19 against loops built in place for each call (this
// file's first form): the routines cost more for one call and less from the
// second on, see the design note.

#include <stdio.h>
#include <stdlib.h>

#include "../Headers/ast.h"
#include "../Headers/global.h"
#include "../Headers/symbols.h"
#include "../Headers/macros.h"
#include "../Headers/messages.h"
#include "../Headers/diag.h"
#include "../Headers/acc.h"
#include "../Headers/nn.h"

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
// the calls -------------------------------------------------------------------
// ----------------------------------------------------------------------------

// __nn_p = &v[0]; acc = &v[0] + the number of elements; CAL __<mac>
static void call(const char *mac, int v)
{
    int n = v_table[v].size * (v_table[v].dims == AR_2D ? v_table[v].cols : 1);
    add_instr("LEA %s\n", v_table[v].name);
    add_instr("SET __nn_p\n");
    add_instr("ADD %d\n", n);
    add_instr("CAL __%s\n", mac);
    mac_add(mac);
    emit_peephole_reset();      // the routine changed the array and the acc
    v_table[v].used = 1;
}

void nn_emit(int fn, int v)
{
    int is_f = v_table[v].type == TY_FLOAT;
    switch (fn) {
        case NN_RELU:    call(is_f ? "nn_relu_f" : "nn_relu_i", v); break;
        case NN_SIGM:    call("nn_sigmoid", v); break;
        case NN_TANH:    call("nn_tanh",    v); break;
        default:         call("nn_softmax", v); break;
    }
    acc_live = 0;
}

// argmax(v) inside an expression: a live partial result, x + 1 in
// x + 1 + argmax(v), goes to the stack first, as for a call of a function;
// the routine leaves the index in the acc
expr nn_argmax_emit(int v)
{
    if (acc_live) add_instr("PSH\n");
    call(v_table[v].type == TY_FLOAT ? "nn_argmax_f" : "nn_argmax_i", v);
    acc_live = 1;
    return expr_make(TY_INT, 0);
}
