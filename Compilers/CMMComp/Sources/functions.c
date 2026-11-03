// ----------------------------------------------------------------------------
// routines and state variables for the function parser -----------------------
// ----------------------------------------------------------------------------

// global includes
#include <string.h>
#include <stdlib.h>

// local includes
#include "../Headers/functions.h"
#include "../Headers/ast.h"
#include "../Headers/t2t.h"
#include "../Headers/labels.h"
#include "../Headers/global.h"
#include "../Headers/itr.h"
#include "../Headers/data_use.h"
#include "../Headers/symbols.h"
#include "../Headers/data_declar.h"
#include "../Headers/messages.h"
#include "../Headers/diag.h"
#include "../../common/xstr.h"

// ----------------------------------------------------------------------------
// global variable definitions ------------------------------------------------
// ----------------------------------------------------------------------------

int  fun_id;      // stores the id of the function being used
int  mainok  = 0; // 1 once a function named main is declared (check_var refuses a program without it)
const char *fname = ""; // name of the function currently being parsed (a v_table name, or "")

// ----------------------------------------------------------------------------
// local variables ------------------------------------------------------------
// ----------------------------------------------------------------------------

int ret_ok;       // tells whether the function returned correctly
int fun_parse;    // stores the id of the function being parsed

// Parameter ids collected by declare_par for the function currently being
// declared. func_body_begin moves these into a STMT_FUNC_HEADER node, then
// resets the count. Grown on demand: a function takes any number of them.
static int *func_params = NULL;
static int  func_nparams = 0;
static int  func_params_cap = 0;


// ----------------------------------------------------------------------------
// arg-stack frames for nested function calls --------------------------------
// ----------------------------------------------------------------------------
// Every '(' at a call site pushes a marker; every par_exp / par_listexp
// appends the arg's .node onto a global stack; the matching fcall / vcall
// drains the slice since its marker into the expr_func_call node (or frees
// the contents, for void calls).

static expr_node **arg_stack    = NULL;
static int arg_stack_n          = 0;
static int arg_stack_cap        = 0;
static int *frame_marks         = NULL;
static int frame_marks_n        = 0;
static int frame_marks_cap      = 0;

static void args_grow_stack(int needed)
{
    if (needed <= arg_stack_cap) return;
    int new_cap = arg_stack_cap ? arg_stack_cap * 2 : 8;
    while (new_cap < needed) new_cap *= 2;
    expr_node **t = xrealloc(arg_stack, (size_t)new_cap * sizeof(*t));
    arg_stack     = t;
    arg_stack_cap = new_cap;
}

static void args_grow_marks(int needed)
{
    if (needed <= frame_marks_cap) return;
    int new_cap = frame_marks_cap ? frame_marks_cap * 2 : 8;
    while (new_cap < needed) new_cap *= 2;
    int *t = xrealloc(frame_marks, (size_t)new_cap * sizeof(*t));
    frame_marks     = t;
    frame_marks_cap = new_cap;
}

void args_frame_push(void)
{
    args_grow_marks(frame_marks_n + 1);
    frame_marks[frame_marks_n++] = arg_stack_n;
}

static void args_push(expr_node *node)
{
    args_grow_stack(arg_stack_n + 1);
    arg_stack[arg_stack_n++] = node;
}

// pops the current frame and returns a heap-owned args[] array (caller
// owns both the array and the nodes). n_out is the arg count.
static expr_node **args_frame_pop(int *n_out)
{
    int mark = frame_marks[--frame_marks_n];
    int n    = arg_stack_n - mark;
    expr_node **out = (n > 0) ? xmalloc((size_t)n * sizeof(*out)) : NULL;
    if (n > 0) memcpy(out, arg_stack + mark, (size_t)n * sizeof(*out));
    arg_stack_n = mark;
    *n_out = n;
    return out;
}

// ----------------------------------------------------------------------------
// helper functions -----------------------------------------------------------
// ----------------------------------------------------------------------------

// checks whether argument k (from 0) passed to the function is ok
void par_check(expr e, int k)
{
    int t_cal = e.type;                  // type of the argument (1, 2 or 3)
    int t_fun = v_table[fun_id].param_types[k]; // type of the parameter in the function
    int index = k+1;                     // position of the parameter, for the messages

    char ld [10]; if (acc_ok == 0) strcpy(ld ,"LOD"  ); else strcpy(ld ,"P_LOD"  );
    char i2f[10]; if (acc_ok == 0) strcpy(i2f,"I2F_M"); else strcpy(i2f,"P_I2F_M");
    char f2i[10]; if (acc_ok == 0) strcpy(f2i,"F2I_M"); else strcpy(f2i,"P_F2I_M");

    // ------------------------------------------------------------------------
    // check every combination ------------------------------------------------
    // ------------------------------------------------------------------------

    expr etr, eti;

    // original is int and call is int var ------------------------------------

    if ((t_fun == 1) && (t_cal == 1) && (e.id != 0))
    {
        add_instr("%s %s\n", ld, v_table[e.id].name);
    }

    // original is int and call is int acc ------------------------------------

    if ((t_fun == 1) && (t_cal == 1) && (e.id == 0))
    {
        // nothing to do
    }

    // original is int and call is float var ----------------------------------

    if ((t_fun == 1) && (t_cal == 2) && (e.id != 0))
    {
        warn(MSG_WARN_CONV_F2I_PARAM, line_num+1, index, v_table[fun_id].name);

        add_instr("%s %s\n", f2i, v_table[e.id].name);
    }

    // original is int and call is float acc ----------------------------------

    if ((t_fun == 1) && (t_cal == 2) && (e.id == 0))
    {
        warn(MSG_WARN_CONV_F2I_PARAM, line_num+1, index, v_table[fun_id].name);

        add_instr("F2I\n");
    }

    // original is int and call is comp const ---------------------------------

    if ((t_fun == 1) && (t_cal == 5))
    {
        warn(MSG_WARN_CONV_C2I_PARAM, line_num+1, index, v_table[fun_id].name);

        get_cmp_cst(e,&etr,&eti);

        add_instr("%s %s\n", f2i, v_table[etr.id].name);
    }

    // original is int and call is comp var -----------------------------------

    if ((t_fun == 1) && (t_cal == 3) && (e.id != 0))
    {
        warn(MSG_WARN_CONV_C2I_PARAM, line_num+1, index, v_table[fun_id].name);

        add_instr("%s %s\n", f2i, v_table[e.id].name);
    }

    // original is int and call is comp acc -----------------------------------

    if ((t_fun == 1) && (t_cal == 3) && (e.id == 0))
    {
        warn(MSG_WARN_CONV_C2I_PARAM, line_num+1, index, v_table[fun_id].name);

        add_instr("POP\n");
        add_instr("F2I\n");
    }

    // original is float and call is int var ----------------------------------

    if ((t_fun == 2) && (t_cal == 1) && (e.id != 0))
    {
        warn(MSG_WARN_CONV_I2F_PARAM, line_num+1, index, v_table[fun_id].name);

        add_instr("%s %s\n", i2f, v_table[e.id].name);
    }

    // original is float and call is int acc ----------------------------------

    if ((t_fun == 2) && (t_cal == 1) && (e.id == 0))
    {
        warn(MSG_WARN_CONV_I2F_PARAM, line_num+1, index, v_table[fun_id].name);

        add_instr("I2F\n");
    }

    // original is float and call is float var --------------------------------

    if ((t_fun == 2) && (t_cal == 2) && (e.id != 0))
    {
        add_instr("%s %s\n", ld, v_table[e.id].name);
    }

    // original is float and call is float acc --------------------------------

    if ((t_fun == 2) && (t_cal == 2) && (e.id == 0))
    {
        // nothing to do
    }

    // original is float and call is comp const -------------------------------

    if ((t_fun == 2) && (t_cal == 5))
    {
        warn(MSG_WARN_CONV_C2F_PARAM, line_num+1, index, v_table[fun_id].name);

        get_cmp_cst(e,&etr,&eti);

        add_instr("%s %s\n", ld, v_table[etr.id].name);
    }

    // original is float and call is comp var ---------------------------------

    if ((t_fun == 2) && (t_cal == 3) && (e.id != 0))
    {
        warn(MSG_WARN_CONV_C2F_PARAM, line_num+1, index, v_table[fun_id].name);

        add_instr("%s %s\n", ld, v_table[e.id].name);
    }

    // original is float and call is comp acc ---------------------------------

    if ((t_fun == 2) && (t_cal == 3) && (e.id == 0))
    {
        warn(MSG_WARN_CONV_C2F_PARAM, line_num+1, index, v_table[fun_id].name);

        add_instr("POP\n");
    }

    // original is comp and call is int var -----------------------------------

    if ((t_fun == 3) && (t_cal == 1) && (e.id != 0))
    {
        warn(MSG_WARN_CONV_I2C_PARAM, line_num+1, index, v_table[fun_id].name);

        add_instr("%s %s\n", i2f, v_table[e.id].name);
        add_instr("P_LOD 0.0\n");
    }

    // original is comp and call is int acc -----------------------------------

    if ((t_fun == 3) && (t_cal == 1) && (e.id == 0))
    {
        warn(MSG_WARN_CONV_I2C_PARAM, line_num+1, index, v_table[fun_id].name);

        add_instr("I2F\n");
        add_instr("P_LOD 0.0\n");
    }

    // original is comp and call is float var ---------------------------------

    if ((t_fun == 3) && (t_cal == 2) && (e.id != 0))
    {
        warn(MSG_WARN_CONV_F2C_PARAM, line_num+1, index, v_table[fun_id].name);

        add_instr("%s %s\n", ld, v_table[e.id].name);
        add_instr("P_LOD 0.0\n");
    }

    // original is comp and call is float acc ---------------------------------

    if ((t_fun == 3) && (t_cal == 2) && (e.id == 0))
    {
        warn(MSG_WARN_CONV_F2C_PARAM, line_num+1, index, v_table[fun_id].name);

        add_instr("P_LOD 0.0\n");
    }

    // original is comp and call is comp const --------------------------------

    if ((t_fun == 3) && (t_cal == 5))
    {
        get_cmp_cst(e,&etr,&eti);

        add_instr("%s %s\n", ld, v_table[etr.id].name);
        add_instr("P_LOD %s\n",  v_table[eti.id].name);
    }

    // original is comp and call is comp var ----------------------------------

    if ((t_fun == 3) && (t_cal == 3) && (e.id != 0))
    {
        get_cmp_ets(e,&etr,&eti); // gets the extended IDs of the right side in memory

        add_instr("%s %s\n" , ld, v_table[etr.id].name);
        add_instr("P_LOD %s\n",     v_table[eti.id].name);
    }

    // original is comp and call is comp acc ----------------------------------

    if ((t_fun == 3) && (t_cal == 3) && (e.id == 0))
    {
        // nothing to do
    }
}

// ----------------------------------------------------------------------------
// declaration ----------------------------------------------------------------
// ----------------------------------------------------------------------------

// declares a function
// Records the function entry in v_table. No codegen happens here - the
// @label and the per-parameter
// SET / SET_P all live in the walker case for STMT_FUNC_HEADER and fire
// only when func_body_begin builds that node.
void declare_fun(int id1, int id2) // id1 -> type, id2 -> name index
{
    check_name(id2, 1);   // not a variable's or another function's name, not an instruction
    if (strcmp(v_table[id2].name, "main") == 0) mainok = 1; // the reset reaches main: it must exist

    fname = v_table[id2].name; // set the fname state to the function being analyzed
    v_table[id2].type = TY_FVOID + id1; // a function: TY_FVOID .. TY_FCOMP by its return type (TY_NONE .. TY_COMP)
    fun_parse    = id2; // set the fun_parse state to the function's name id
    ret_ok       = 0;   // set ret_ok to zero (function parsing starts)
    func_nparams = 0;   // reset param collection for the new function
}

// Records a parameter. Each declare_par call appends to func_params in
// declaration order; the emit (SET / SET_P) is deferred to the walker
// via STMT_FUNC_HEADER built in func_body_begin.
int declare_par(int type, int id)
{
    declare_var(id); // arrays cannot be passed as function parameters

    // store the data type of every parameter, in order
    symbol *f = &v_table[fun_parse];
    f->param_types = xrealloc(f->param_types, (size_t)(f->n_params+1) * sizeof(int));
    f->param_types[f->n_params++] = type;

    if (func_nparams == func_params_cap)
    {
        func_params_cap = func_params_cap ? func_params_cap*2 : 16;
        func_params = xrealloc(func_params, (size_t)func_params_cap * sizeof(int));
    }
    func_params[func_nparams++] = id;

    return id;
}

// when the return keyword is found
void declare_ret(expr e, int ret)
{
    // check whether it really is a function, or void by mistake
    if (v_table[fun_parse].type == TY_FVOID)
        fatal(MSG_ERR_VOID_RETURN_VALUE, line_num+1);

    // ------------------------------------------------------------------------
    // check every combination ------------------------------------------------
    // ------------------------------------------------------------------------

    expr etr, eti;
    int left_type = v_table[fun_parse].type;

    // int with int var
    if ((left_type == 7) && (e.type == TY_INT) && (e.id!=0))
    {
        add_instr("LOD %s\n", v_table[e.id].name);
    }

    // int with int acc
    if ((left_type == 7) && (e.type == TY_INT) && (e.id==0))
    {
        // nothing to do
    }

    // int with float var
    if ((left_type == 7) && (e.type == TY_FLOAT) && (e.id!=0))
    {
        warn(MSG_WARN_CONV_F2I_RETURN, line_num+1, v_table[fun_parse].name);

        add_instr("F2I_M %s\n", v_table[e.id].name);
    }

    // int with float acc
    if ((left_type == 7) && (e.type == TY_FLOAT) && (e.id==0))
    {
        warn(MSG_WARN_CONV_F2I_RETURN, line_num+1, v_table[fun_parse].name);
        add_instr("F2I\n");
    }

    // int with comp const
    if ((left_type == 7) && (e.type == TY_CCONST))
    {
        warn(MSG_WARN_ROUND_REAL, line_num+1);

        get_cmp_cst(e,&etr,&eti);

        add_instr("F2I_M %s\n", v_table[etr.id].name);
    }

    // int with comp var
    if ((left_type == 7) && (e.type == TY_COMP) && (e.id!=0))
    {
        warn(MSG_WARN_ROUND_REAL, line_num+1);

        get_cmp_ets(e,&etr,&eti);

        add_instr("F2I_M %s\n", v_table[etr.id].name);
    }

    // int with comp acc
    if ((left_type == 7) && (e.type == TY_COMP) && (e.id==0))
    {
        warn(MSG_WARN_ROUND_REAL, line_num+1);

        add_instr("POP\n");
        add_instr("F2I\n");
    }

    // float with int var
    if ((left_type == 8) && (e.type == TY_INT) && (e.id!=0))
    {
        warn(MSG_WARN_RET_FLOAT_RECV_INT, line_num+1);

        add_instr("I2F_M %s\n", v_table[e.id].name);
    }

    // float with int acc
    if ((left_type == 8) && (e.type == TY_INT) && (e.id==0))
    {
        warn(MSG_WARN_RET_FLOAT_RECV_INT, line_num+1);

        add_instr("I2F\n");
    }

    // float with float var
    if ((left_type == 8) && (e.type == TY_FLOAT) && (e.id!=0))
    {
        add_instr("LOD %s\n", v_table[e.id].name);
    }

    // float with float acc
    if ((left_type == 8) && (e.type == TY_FLOAT) && (e.id==0))
    {
        // nothing to do
    }

    // float with comp const
    if ((left_type == 8) && (e.type == TY_CCONST))
    {
        warn(MSG_WARN_GRAB_REAL_ONLY, line_num+1);

        get_cmp_cst(e,&etr,&eti);

        add_instr("LOD %s\n", v_table[etr.id].name);
    }

    // float with comp var
    if ((left_type == 8) && (e.type == TY_COMP) && (e.id!=0))
    {
        warn(MSG_WARN_GRAB_REAL_ONLY, line_num+1);

        get_cmp_ets(e,&etr,&eti);

        add_instr("LOD %s\n", v_table[etr.id].name);
    }

    // float with comp acc
    if ((left_type == 8) && (e.type == TY_COMP) && (e.id==0))
    {
        warn(MSG_WARN_GRAB_REAL_ONLY, line_num+1);

        add_instr("POP\n");
    }

    // comp with int var
    if ((left_type == 9) && (e.type == TY_INT) && (e.id!=0))
    {
        warn(MSG_WARN_RET_COMP_RECV_INT, line_num+1);

        add_instr("I2F_M %s\n", v_table[e.id].name);
        add_instr("P_LOD 0.0\n");
    }

    // comp with int acc
    if ((left_type == 9) && (e.type == TY_INT) && (e.id==0))
    {
        warn(MSG_WARN_RET_COMP_RECV_INT, line_num+1);

        add_instr("I2F\n");
        add_instr("P_LOD 0.0\n");
    }

    // comp with float var
    if ((left_type == 9) && (e.type == TY_FLOAT) && (e.id!=0))
    {
        warn(MSG_WARN_RET_COMP_RECV_FLOAT, line_num+1);

        add_instr("LOD %s\n", v_table[e.id].name);
        add_instr("P_LOD 0.0\n");
    }

    // comp with float acc
    if ((left_type == 9) && (e.type == TY_FLOAT) && (e.id==0))
    {
        warn(MSG_WARN_RET_COMP_RECV_FLOAT, line_num+1);

        add_instr("P_LOD 0.0\n");
    }

    // comp with comp const
    if ((left_type == 9) && (e.type == TY_CCONST))
    {
        get_cmp_cst(e,&etr,&eti);

        add_instr("LOD %s\n"  , v_table[etr.id].name);
        add_instr("P_LOD %s\n", v_table[eti.id].name);
    }

    // comp with comp var
    if ((left_type == 9) && (e.type == TY_COMP) && (e.id!=0))
    {
        get_cmp_ets(e,&etr,&eti);

        add_instr("LOD %s\n"  , v_table[etr.id].name);
        add_instr("P_LOD %s\n", v_table[eti.id].name);
    }

    // comp with comp acc
    if ((left_type == 9) && (e.type == TY_COMP) && (e.id==0))
    {
        // nothing to do
    }

    // ------------------------------------------------------------------------
    // finalize ---------------------------------------------------------------
    // ------------------------------------------------------------------------

    if (ret == 0) return;

    add_instr("RET\n");

    acc_ok = 0; // even with an exp in the acc, release it to start another function
    ret_ok = 1; // the return keyword appeared properly in the function
}

// fires at the opening '{' of a function body. Opens the stmt_list that
// collects every statement reduced inside the body, then drops a
// STMT_FUNC_HEADER on top of it so the walker emits @label /
// per-parameter SET[_P] before the body's stmts. The matching '}' (in
// func_ret) closes the list and walks it through stmt_emit, producing the
// function body's assembly all at once.
void func_body_begin(void)
{
    stmt_list_open();
    emit_peephole_reset();

    // Hand the collected param ids over to the header node; reset the
    // collector for the next function.
    int *params = NULL;
    if (func_nparams > 0)
    {
        params = xmalloc(sizeof(int) * func_nparams);
        memcpy(params, func_params, sizeof(int) * func_nparams);
    }
    stmt_append(stmt_func_header(fun_parse, params, func_nparams));
    func_nparams = 0;
}

// end of the parsing for a function declaration. Closes the body stmt_list
// and hands it up wrapped in a STMT_FUNC. Codegen for the function (walking
// the body, validating return-reached for non-void, emitting JMP fim / RET)
// happens later in the STMT_FUNC walker case, fired from the global `fim`
// walk - by then the body has been walked and ret_ok reflects whether a
// `return` statement actually fired.
void func_ret(int id) // id -> id of the current function
{
    stmt_node *body = stmt_list_close();
    stmt_append(stmt_func(id, body));

    // env variable fname becomes empty (left a function)
    fname = "";
}

// return without exp (return;)
void void_ret(void)
{
    // check whether it really is void, or a non-void by mistake
    if (v_table[fun_parse].type != TY_FVOID)
        fatal(MSG_ERR_NO_RETURN_VALUE, line_num+1);

    // if main, jump to the @fim that the end of main emits, where the
    // testbench stops the simulation. With a #PRACA, return; waits in place
    // for the interrupt instead (an @fim of its own, which the testbench does
    // not stop on), as ZeroCross and ProcDTW do. Reads the function name from
    // v_table via fun_parse (set by the STMT_FUNC walker before walking
    // the body) - the parse-time `fname` is "" by now since walker time
    // happens after parse_end of every function.
    if (strcmp(v_table[fun_parse].name, "main") != 0)
         add_instr(             "RET\n");
    else if (itr_seen)
         add_sinst(-3, "@fim JMP fim\n");
    else add_instr(             "JMP fim\n");
}

// ----------------------------------------------------------------------------
// usage ----------------------------------------------------------------------
// ----------------------------------------------------------------------------

// Each arg's exp_list reduce just records the arg's tree node on the current
// call's frame. The emit (subtree + par_check stage + CAL) happens later when
// the walker reaches the EXPR_FUNC_CALL node (or the void call's statement).
void par_exp(expr_node *n)
{
    args_push(n);
}

void par_listexp(expr_node *n)
{
    args_push(n);
}

// void-call statement: pre-validates type / arity, pops this call's frame,
// and packages the args into an EXPR_FUNC_CALL (type=0). The grammar wraps
// the returned stmt_node into the enclosing body via stmt_append; the
// walker emits par_check + CAL at body-close time. Popping the frame here
// (instead of inside the walker) keeps each call's args bound at parse time
// - waiting until walker time would draw frames in LIFO order while the
// walker visits the calls in source order, swapping arg lists.
stmt_node *vcall(int id)
{
    if  (v_table[id].type < TY_FVOID)
    {
        fatal(MSG_ERR_FUNC_WHERE, line_num+1, strip_fn_prefix(v_table[id].name));
    }

    int n; expr_node **a = args_frame_pop(&n);

    if (n != v_table[id].n_params)
    {
        fatal(MSG_ERR_PARAM_COUNT, line_num+1, strip_fn_prefix(v_table[id].name));
    }

    v_table[id].used = 1;

    // type=0 for void: walker leaves acc_ok=0 after CAL
    return stmt_void_call(expr_func_call(0, id, a, n));
}

// value-returning call: pre-validates and packages the call as an
// EXPR_FUNC_CALL node, hands the node up to whichever consumer reduces next.
// No emit at parse time - the walker emits it with its statement.
expr_node *fcall(int id)
{
    if (v_table[id].type == TY_FVOID)
    {
        fatal(MSG_ERR_VOID_FUNC_USE, line_num+1, v_table[id].name);
    }
    else if (v_table[id].type < TY_FVOID)
    {
        fatal(MSG_ERR_FUNC_WHERE2, line_num+1, strip_fn_prefix(v_table[id].name));
    }

    int n; expr_node **a = args_frame_pop(&n);

    if (n != v_table[id].n_params)
    {
        fatal(MSG_ERR_PARAM_LIST_DIFF, line_num+1, v_table[id].name);
    }

    v_table[id].used = 1;

    return expr_func_call(v_table[id].type - TY_FVOID, id, a, n);
}

// ----------------------------------------------------------------------------
// recursion check ------------------------------------------------------------
// ----------------------------------------------------------------------------
// C+- gives every local a fixed data address, so a function that is still
// running when it is called again gets its values overwritten (a recursive
// factorial printed 1 1 1). The walker records every CAL as an edge, and a
// depth-first walk over them stops the build on the first cycle it finds:
// direct (fact -> fact), or through other functions (a -> b -> a), which C+-
// cannot spell today (a call needs its callee defined above it).

static int *ce_from = NULL, *ce_to = NULL, *ce_line = NULL, ce_n = 0, ce_cap = 0;

void fun_call_edge(int caller, int callee, int line)
{
    if (caller < 0 || callee < 0) return;          // a call outside any function body
    for (int k = 0; k < ce_n; k++) if (ce_from[k] == caller && ce_to[k] == callee) return;
    if (ce_n == ce_cap)
    {
        ce_cap  = ce_cap ? 2 * ce_cap : 64;
        ce_from = xrealloc(ce_from, ce_cap * sizeof(int));
        ce_to   = xrealloc(ce_to  , ce_cap * sizeof(int));
        ce_line = xrealloc(ce_line, ce_cap * sizeof(int));
    }
    ce_from[ce_n] = caller; ce_to[ce_n] = callee; ce_line[ce_n] = line; ce_n++;
}

static int rec_dfs(int f, char *state, int *path, int depth)
{
    state[f] = 1;                                  // on the current path
    path[depth] = f;
    for (int k = 0; k < ce_n; k++)
    {
        if (ce_from[k] != f) continue;
        int g = ce_to[k];
        if (state[g] == 1)                         // back to a function still open
        {
            int at = 0; while (path[at] != g) at++;
            char *chain = xstrdup("");             // f -> g -> ... -> f, of any length
            for (int i = at; i <= depth; i++)
            {
                char *c = xasprintf("%s%s -> ", chain, v_table[path[i]].name);
                free(chain); chain = c;
            }
            fatal(MSG_ERR_RECURSION, ce_line[k], v_table[g].name, xasprintf("%s%s", chain, v_table[g].name));
        }
        if (state[g] == 0 && rec_dfs(g, state, path, depth + 1)) return 1;
    }
    state[f] = 2;                                  // done, no cycle through it
    return 0;
}

void check_rec(void)
{
    int top = 0;
    for (int k = 0; k < ce_n; k++)
    {
        if (ce_from[k] > top) top = ce_from[k];
        if (ce_to[k]   > top) top = ce_to[k];
    }
    char *state = xcalloc(top + 1, 1);
    int  *path  = xmalloc((ce_n + 2) * sizeof(int));
    for (int k = 0; k < ce_n; k++)
        if (state[ce_from[k]] == 0) rec_dfs(ce_from[k], state, path, 0);
    free(state); free(path);
}
