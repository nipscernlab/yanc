// ----------------------------------------------------------------------------
// routines and state variables for the function parser -----------------------
// ----------------------------------------------------------------------------

// global includes
#include <string.h>
#include <stdlib.h>

// local includes
#include "../Headers/ast.h"
#include "../Headers/t2t.h"
#include "../Headers/labels.h"
#include "../Headers/global.h"
#include "../Headers/data_use.h"
#include "../Headers/variaveis.h"
#include "../Headers/data_declar.h"
#include "../Headers/messages.h"

// ----------------------------------------------------------------------------
// global variable definitions ------------------------------------------------
// ----------------------------------------------------------------------------

int  fun_id;      // stores the id of the function being used
int  mainok  = 0; // main function status: 0 -> undefined, 1 -> resolved (how it will be called)
char fname [512]; // name of the function currently being parsed

// ----------------------------------------------------------------------------
// local variables ------------------------------------------------------------
// ----------------------------------------------------------------------------

int ret_ok;       // tells whether the function returned correctly
int fun_parse;    // stores the id of the function being parsed
int p_test;       // identifies parameters in function calls (similar to OFST but with value 10)

// Parameter ids collected by declar_par for the function currently being
// declared. func_body_begin moves these into a STMT_FUNC_HEADER node, then
// resets the count. Fixed-size: parameter lists never get large.
#define FUNC_MAX_PARAMS 16
static int func_params[FUNC_MAX_PARAMS];
static int func_nparams = 0;

// Set by declar_fun when it decides the current function needs a `JMP main`
// prologue (the first non-main function in source order). Consumed by
// func_body_begin when it builds the STMT_FUNC_HEADER, then resets.
static int func_jmp_main = 0;

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
    expr_node **t = realloc(arg_stack, (size_t)new_cap * sizeof(*t));
    if (!t) {fprintf(stderr, MSG_ERR_OUT_OF_MEMORY); exit(EXIT_FAILURE);}
    arg_stack     = t;
    arg_stack_cap = new_cap;
}

static void args_grow_marks(int needed)
{
    if (needed <= frame_marks_cap) return;
    int new_cap = frame_marks_cap ? frame_marks_cap * 2 : 8;
    while (new_cap < needed) new_cap *= 2;
    int *t = realloc(frame_marks, (size_t)new_cap * sizeof(*t));
    if (!t) {fprintf(stderr, MSG_ERR_OUT_OF_MEMORY); exit(EXIT_FAILURE);}
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
    expr_node **out = (n > 0) ? malloc((size_t)n * sizeof(*out)) : NULL;
    if (n > 0) memcpy(out, arg_stack + mark, (size_t)n * sizeof(*out));
    arg_stack_n = mark;
    *n_out = n;
    return out;
}

// ----------------------------------------------------------------------------
// helper functions -----------------------------------------------------------
// ----------------------------------------------------------------------------

// computes how many parameters a function has
int get_npar(int par)
{
    int t_fun = par;
    int n_par = 0;

    while (t_fun != 0) {t_fun = t_fun/10; n_par++;}

    return n_par;
}

// checks whether the argument passed to the function is ok
void par_check(expr e)
{
    // get the original number of parameters
    int n_par = get_npar(v_table[fun_id].fpar);

    // get the type and position of the current parameter being called
    int  t_cal = p_test; // will hold the parameter type (0, 1, 2 or 3)
    int  aux   = p_test;
    int id_cal = n_par ;
    int  index = 1;      // will hold the parameter position
    while (aux > 10)
    {
           aux = aux   / 10;
         t_cal = t_cal % 10;
        id_cal--;
         index++;
    }

    // get the type of the current parameter in the original function
    int t_fun = v_table[fun_id].fpar;
    int i;
    for (i = 1; i < id_cal; i++) t_fun = t_fun/10;
    t_fun = t_fun % 10;

    char ld [10]; if (acc_ok == 0) strcpy(ld ,"LOD"  ); else strcpy(ld ,"P_LOD"  );
    char i2f[10]; if (acc_ok == 0) strcpy(i2f,"I2F_M"); else strcpy(i2f,"P_I2F_M");

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
        fprintf(stdout, MSG_WARN_CONV_F2I_PARAM, line_num+1, index, v_table[fun_id].name);

        add_instr("%s %s\n", ld, v_table[e.id].name);
        add_instr("F2I\n");
    }

    // original is int and call is float acc ----------------------------------

    if ((t_fun == 1) && (t_cal == 2) && (e.id == 0))
    {
        fprintf(stdout, MSG_WARN_CONV_F2I_PARAM, line_num+1, index, v_table[fun_id].name);

        add_instr("F2I\n");
    }

    // original is int and call is comp const ---------------------------------

    if ((t_fun == 1) && (t_cal == 5))
    {
        fprintf(stdout, MSG_WARN_CONV_C2I_PARAM, line_num+1, index, v_table[fun_id].name);

        get_cmp_cst(e,&etr,&eti);

        add_instr("%s %s\n", ld, v_table[etr.id].name);
        add_instr("F2I\n");
    }

    // original is int and call is comp var -----------------------------------

    if ((t_fun == 1) && (t_cal == 3) && (e.id != 0))
    {
        fprintf(stdout, MSG_WARN_CONV_C2I_PARAM, line_num+1, index, v_table[fun_id].name);

        add_instr("%s %s\n", ld, v_table[e.id].name);
        add_instr("F2I\n");
    }

    // original is int and call is comp acc -----------------------------------

    if ((t_fun == 1) && (t_cal == 3) && (e.id == 0))
    {
        fprintf(stdout, MSG_WARN_CONV_C2I_PARAM, line_num+1, index, v_table[fun_id].name);

        add_instr("POP\n");
        add_instr("F2I\n");
    }

    // original is float and call is int var ----------------------------------

    if ((t_fun == 2) && (t_cal == 1) && (e.id != 0))
    {
        fprintf(stdout, MSG_WARN_CONV_I2F_PARAM, line_num+1, index, v_table[fun_id].name);

        add_instr("%s %s\n", i2f, v_table[e.id].name);
    }

    // original is float and call is int acc ----------------------------------

    if ((t_fun == 2) && (t_cal == 1) && (e.id == 0))
    {
        fprintf(stdout, MSG_WARN_CONV_I2F_PARAM, line_num+1, index, v_table[fun_id].name);

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
        fprintf(stdout, MSG_WARN_CONV_C2F_PARAM, line_num+1, index, v_table[fun_id].name);

        get_cmp_cst(e,&etr,&eti);

        add_instr("%s %s\n", ld, v_table[etr.id].name);
    }

    // original is float and call is comp var ---------------------------------

    if ((t_fun == 2) && (t_cal == 3) && (e.id != 0))
    {
        fprintf(stdout, MSG_WARN_CONV_C2F_PARAM, line_num+1, index, v_table[fun_id].name);

        add_instr("%s %s\n", ld, v_table[e.id].name);
    }

    // original is float and call is comp acc ---------------------------------

    if ((t_fun == 2) && (t_cal == 3) && (e.id == 0))
    {
        fprintf(stdout, MSG_WARN_CONV_C2F_PARAM, line_num+1, index, v_table[fun_id].name);

        add_instr("POP\n");
    }

    // original is comp and call is int var -----------------------------------

    if ((t_fun == 3) && (t_cal == 1) && (e.id != 0))
    {
        fprintf(stdout, MSG_WARN_CONV_I2C_PARAM, line_num+1, index, v_table[fun_id].name);

        add_instr("%s %s\n", i2f, v_table[e.id].name);
        add_instr("P_LOD 0.0\n");
    }

    // original is comp and call is int acc -----------------------------------

    if ((t_fun == 3) && (t_cal == 1) && (e.id == 0))
    {
        fprintf(stdout, MSG_WARN_CONV_I2C_PARAM, line_num+1, index, v_table[fun_id].name);

        add_instr("I2F\n");
        add_instr("P_LOD 0.0\n");
    }

    // original is comp and call is float var ---------------------------------

    if ((t_fun == 3) && (t_cal == 2) && (e.id != 0))
    {
        fprintf(stdout, MSG_WARN_CONV_F2C_PARAM, line_num+1, index, v_table[fun_id].name);

        add_instr("%s %s\n", ld, v_table[e.id].name);
        add_instr("P_LOD 0.0\n");
    }

    // original is comp and call is float acc ---------------------------------

    if ((t_fun == 3) && (t_cal == 2) && (e.id == 0))
    {
        fprintf(stdout, MSG_WARN_CONV_F2C_PARAM, line_num+1, index, v_table[fun_id].name);

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
// Records the function entry in v_table + decides whether a `JMP main`
// prologue will be needed once this function gets emitted. No codegen
// happens here - the @label, the optional JMP main and the per-parameter
// SET / SET_P all live in the walker case for STMT_FUNC_HEADER and fire
// only when func_body_begin builds that node.
void declar_fun(int id1, int id2) // id1 -> type, id2 -> name index
{
    int is_main = (strcmp(v_table[id2].name, "main") == 0);
    if (mainok == 0)
    {
        func_jmp_main = !is_main; // walker emits JMP main before the @label
        mainok        = 1;        // first function resolves the main question
    }
    else
    {
        func_jmp_main = 0;
    }

    strcpy(fname, v_table[id2].name); // set the fname state to the function being analyzed
    v_table[id2].type = id1+6       ; // v_type becomes function (void, int, float, comp) -> (6, 7, 8, 9)
    fun_parse    = id2; // set the fun_parse state to the function's name id
    ret_ok       = 0;   // set ret_ok to zero (function parsing starts)
    func_nparams = 0;   // reset param collection for the new function
}

// Records a parameter. Each declar_par call appends to func_params in
// declaration order; the emit (SET / SET_P) is deferred to the walker
// via STMT_FUNC_HEADER built in func_body_begin.
int declar_par(int type, int id)
{
    declar_var(id); // arrays cannot be passed as function parameters

    // store info about every parameter's data type in a single number
    v_table[fun_parse].fpar = v_table[fun_parse].fpar*10 + type;

    if (func_nparams < FUNC_MAX_PARAMS)
        func_params[func_nparams++] = id;

    return id;
}

// when the return keyword is found
void declar_ret(expr e, int ret)
{
    // check whether it really is a function, or void by mistake
    if (v_table[fun_parse].type == 6)
        {fprintf (stderr, MSG_ERR_VOID_RETURN_VALUE, line_num+1); exit(EXIT_FAILURE);}

    // test whether it is inside an if/else
    //if ((get_if() > 0) && (v_table[fun_parse].type != 6))
        //fprintf(stdout, "Heads up on line %d: using return inside if/else can break if you forget it somewhere!\n", line_num+1);

    // ------------------------------------------------------------------------
    // check every combination ------------------------------------------------
    // ------------------------------------------------------------------------

    expr etr, eti;
    int left_type = v_table[fun_parse].type;

    // int with int var
    if ((left_type == 7) && (e.type == 1) && (e.id!=0))
    {
        add_instr("LOD %s\n", v_table[e.id].name);
    }

    // int with int acc
    if ((left_type == 7) && (e.type == 1) && (e.id==0))
    {
        // nothing to do
    }

    // int with float var
    if ((left_type == 7) && (e.type == 2) && (e.id!=0))
    {
        fprintf(stdout, MSG_WARN_CONV_F2I_RETURN, line_num+1, v_table[fun_parse].name);

        add_instr("F2I_M %s\n", v_table[e.id].name);
    }

    // int with float acc
    if ((left_type == 7) && (e.type == 2) && (e.id==0))
    {
        fprintf(stdout, MSG_WARN_CONV_F2I_RETURN, line_num+1, v_table[fun_parse].name);
        add_instr("F2I\n");
    }

    // int with comp const
    if ((left_type == 7) && (e.type == 5))
    {
        fprintf (stdout, MSG_WARN_ROUND_REAL, line_num+1);

        get_cmp_cst(e,&etr,&eti);

        add_instr("F2I_M %s\n", v_table[etr.id].name);
    }

    // int with comp var
    if ((left_type == 7) && (e.type == 3) && (e.id!=0))
    {
        fprintf (stdout, MSG_WARN_ROUND_REAL, line_num+1);

        get_cmp_ets(e,&etr,&eti);

        add_instr("F2I_M %s\n", v_table[etr.id].name);
    }

    // int with comp acc
    if ((left_type == 7) && (e.type == 3) && (e.id==0))
    {
        fprintf (stdout, MSG_WARN_ROUND_REAL, line_num+1);

        add_instr("POP\n");
        add_instr("F2I\n");
    }

    // float with int var
    if ((left_type == 8) && (e.type == 1) && (e.id!=0))
    {
        fprintf(stdout, MSG_WARN_RET_FLOAT_RECV_INT, line_num+1);

        add_instr("I2F_M %s\n", v_table[e.id].name);
    }

    // float with int acc
    if ((left_type == 8) && (e.type == 1) && (e.id==0))
    {
        fprintf(stdout, MSG_WARN_RET_FLOAT_RECV_INT, line_num+1);

        add_instr("I2F\n");
    }

    // float with float var
    if ((left_type == 8) && (e.type == 2) && (e.id!=0))
    {
        add_instr("LOD %s\n", v_table[e.id].name);
    }

    // float with float acc
    if ((left_type == 8) && (e.type == 2) && (e.id==0))
    {
        // nothing to do
    }

    // float with comp const
    if ((left_type == 8) && (e.type == 5))
    {
        fprintf (stdout, MSG_WARN_GRAB_REAL_ONLY, line_num+1);

        get_cmp_cst(e,&etr,&eti);

        add_instr("LOD %s\n", v_table[etr.id].name);
    }

    // float with comp var
    if ((left_type == 8) && (e.type == 3) && (e.id!=0))
    {
        fprintf (stdout, MSG_WARN_GRAB_REAL_ONLY, line_num+1);

        get_cmp_ets(e,&etr,&eti);

        add_instr("LOD %s\n", v_table[etr.id].name);
    }

    // float with comp acc
    if ((left_type == 8) && (e.type == 3) && (e.id==0))
    {
        fprintf (stdout, MSG_WARN_GRAB_REAL_ONLY, line_num+1);

        add_instr("POP\n");
    }

    // comp with int var
    if ((left_type == 9) && (e.type == 1) && (e.id!=0))
    {
        fprintf(stdout, MSG_WARN_RET_COMP_RECV_INT, line_num+1);

        add_instr("I2F_M %s\n", v_table[e.id].name);
        add_instr("P_LOD 0.0\n");
    }

    // comp with int acc
    if ((left_type == 9) && (e.type == 1) && (e.id==0))
    {
        fprintf(stdout, MSG_WARN_RET_COMP_RECV_INT, line_num+1);

        add_instr("I2F\n");
        add_instr("P_LOD 0.0\n");
    }

    // comp with float var
    if ((left_type == 9) && (e.type == 2) && (e.id!=0))
    {
        fprintf(stdout, MSG_WARN_RET_COMP_RECV_FLOAT, line_num+1);

        add_instr("LOD %s\n", v_table[e.id].name);
        add_instr("P_LOD 0.0\n");
    }

    // comp with float acc
    if ((left_type == 9) && (e.type == 2) && (e.id==0))
    {
        fprintf(stdout, MSG_WARN_RET_COMP_RECV_FLOAT, line_num+1);

        add_instr("P_LOD 0.0\n");
    }

    // comp with comp const
    if ((left_type == 9) && (e.type == 5))
    {
        get_cmp_cst(e,&etr,&eti);

        add_instr("LOD %s\n"  , v_table[etr.id].name);
        add_instr("P_LOD %s\n", v_table[eti.id].name);
    }

    // comp with comp var
    if ((left_type == 9) && (e.type == 3) && (e.id!=0))
    {
        get_cmp_ets(e,&etr,&eti);

        add_instr("LOD %s\n"  , v_table[etr.id].name);
        add_instr("P_LOD %s\n", v_table[eti.id].name);
    }

    // comp with comp acc
    if ((left_type == 9) && (e.type == 3) && (e.id==0))
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
// STMT_FUNC_HEADER on top of it so the walker emits @label / JMP main /
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
        params = malloc(sizeof(int) * func_nparams);
        if (!params) {fprintf(stderr, MSG_ERR_OUT_OF_MEMORY); exit(EXIT_FAILURE);}
        memcpy(params, func_params, sizeof(int) * func_nparams);
    }
    stmt_append(stmt_func_header(fun_parse, func_jmp_main,
                                      params, func_nparams));
    func_nparams = 0;
    func_jmp_main = 0;
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
    strcpy(fname, "");
}

// return without exp (return;)
void void_ret()
{
    // check whether it really is void, or a non-void by mistake
    if (v_table[fun_parse].type != 6)
        {fprintf (stderr, MSG_ERR_NO_RETURN_VALUE, line_num+1); exit(EXIT_FAILURE);}

    // if main, use JMP fim instead of RET. Reads the function name from
    // v_table via fun_parse (set by the STMT_FUNC walker before walking
    // the body) - the parse-time `fname` is "" by now since walker time
    // happens after parse_end of every function.
    if (strcmp(v_table[fun_parse].name, "main") == 0)
         add_sinst(-3, "@fim JMP fim\n");
    else add_instr(             "RET\n");
}

// ----------------------------------------------------------------------------
// usage ----------------------------------------------------------------------
// ----------------------------------------------------------------------------

// Each arg's exp_list reduce just records the arg's tree node on the current
// call's frame. The emit (subtree + par_check stage + CAL) happens later when
// the walker reaches the EXPR_FUNC_CALL node at consumer-EE / vcall time.
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
    if  (v_table[id].type < 6)
    {
        fprintf(stderr, MSG_ERR_FUNC_WHERE, line_num+1, rem_fname(v_table[id].name, fname));
        exit(EXIT_FAILURE);
    }

    int n; expr_node **a = args_frame_pop(&n);

    if (n != get_npar(v_table[id].fpar))
    {
        fprintf(stderr, MSG_ERR_PARAM_COUNT, line_num+1, rem_fname(v_table[id].name, fname));
        exit(EXIT_FAILURE);
    }

    v_table[id].used = 1;

    // type=0 for void: walker leaves acc_ok=0 after CAL
    return stmt_void_call(expr_func_call(0, id, a, n));
}

// value-returning call: pre-validates and packages the call as an
// EXPR_FUNC_CALL node, hands the node up to whichever consumer reduces next.
// No emit at parse time - the walker emits when the consumer calls EE($N).
expr_node *fcall(int id)
{
    if (v_table[id].type == 6)
    {
        fprintf (stderr, MSG_ERR_VOID_FUNC_USE, line_num+1, v_table[id].name);
        exit(EXIT_FAILURE);
    }
    else if (v_table[id].type < 6)
    {
        fprintf (stderr, MSG_ERR_FUNC_WHERE2, line_num+1, rem_fname(v_table[id].name, fname));
        exit(EXIT_FAILURE);
    }

    int n; expr_node **a = args_frame_pop(&n);

    if (n != get_npar(v_table[id].fpar))
    {
        fprintf(stderr, MSG_ERR_PARAM_LIST_DIFF, line_num+1, v_table[id].name);
        exit(EXIT_FAILURE);
    }

    v_table[id].used = 1;

    return expr_func_call(v_table[id].type-6, id, a, n);
}
