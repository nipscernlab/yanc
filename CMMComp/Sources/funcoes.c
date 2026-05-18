// ----------------------------------------------------------------------------
// routines and state variables for the function parser -----------------------
// ----------------------------------------------------------------------------

// global includes
#include <string.h>
#include <stdlib.h>

// local includes
#include "..\Headers\ast.h"
#include "..\Headers\emit.h"
#include "..\Headers\t2t.h"
#include "..\Headers\labels.h"
#include "..\Headers\global.h"
#include "..\Headers\data_use.h"
#include "..\Headers\variaveis.h"
#include "..\Headers\data_declar.h"
#include "..\Headers\messages.h"

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
    int n_par = get_npar(v_fpar[fun_id]);

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
    int t_fun = v_fpar[fun_id];
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
        add_instr("%s %s\n", ld, v_name[e.id]);
    }

    // original is int and call is int acc ------------------------------------

    if ((t_fun == 1) && (t_cal == 1) && (e.id == 0))
    {
        // nothing to do
    }

    // original is int and call is float var ----------------------------------

    if ((t_fun == 1) && (t_cal == 2) && (e.id != 0))
    {
        fprintf(stdout, MSG_WARN_CONV_F2I_PARAM, line_num+1, index, v_name[fun_id]);

        add_instr("%s %s\n", ld, v_name[e.id]);
        add_instr("F2I\n");
    }

    // original is int and call is float acc ----------------------------------

    if ((t_fun == 1) && (t_cal == 2) && (e.id == 0))
    {
        fprintf(stdout, MSG_WARN_CONV_F2I_PARAM, line_num+1, index, v_name[fun_id]);

        add_instr("F2I\n");
    }

    // original is int and call is comp const ---------------------------------

    if ((t_fun == 1) && (t_cal == 5))
    {
        fprintf(stdout, MSG_WARN_CONV_C2I_PARAM, line_num+1, index, v_name[fun_id]);

        get_cmp_cst(e,&etr,&eti);

        add_instr("%s %s\n", ld, v_name[etr.id]);
        add_instr("F2I\n");
    }

    // original is int and call is comp var -----------------------------------

    if ((t_fun == 1) && (t_cal == 3) && (e.id != 0))
    {
        fprintf(stdout, MSG_WARN_CONV_C2I_PARAM, line_num+1, index, v_name[fun_id]);

        add_instr("%s %s\n", ld, v_name[e.id]);
        add_instr("F2I\n");
    }

    // original is int and call is comp acc -----------------------------------

    if ((t_fun == 1) && (t_cal == 3) && (e.id == 0))
    {
        fprintf(stdout, MSG_WARN_CONV_C2I_PARAM, line_num+1, index, v_name[fun_id]);

        add_instr("POP\n");
        add_instr("F2I\n");
    }

    // original is float and call is int var ----------------------------------

    if ((t_fun == 2) && (t_cal == 1) && (e.id != 0))
    {
        fprintf(stdout, MSG_WARN_CONV_I2F_PARAM, line_num+1, index, v_name[fun_id]);

        add_instr("%s %s\n", i2f, v_name[e.id]);
    }

    // original is float and call is int acc ----------------------------------

    if ((t_fun == 2) && (t_cal == 1) && (e.id == 0))
    {
        fprintf(stdout, MSG_WARN_CONV_I2F_PARAM, line_num+1, index, v_name[fun_id]);

        add_instr("I2F\n");
    }

    // original is float and call is float var --------------------------------

    if ((t_fun == 2) && (t_cal == 2) && (e.id != 0))
    {
        add_instr("%s %s\n", ld, v_name[e.id]);
    }

    // original is float and call is float acc --------------------------------

    if ((t_fun == 2) && (t_cal == 2) && (e.id == 0))
    {
        // nothing to do
    }

    // original is float and call is comp const -------------------------------

    if ((t_fun == 2) && (t_cal == 5))
    {
        fprintf(stdout, MSG_WARN_CONV_C2F_PARAM, line_num+1, index, v_name[fun_id]);

        get_cmp_cst(e,&etr,&eti);

        add_instr("%s %s\n", ld, v_name[etr.id]);
    }

    // original is float and call is comp var ---------------------------------

    if ((t_fun == 2) && (t_cal == 3) && (e.id != 0))
    {
        fprintf(stdout, MSG_WARN_CONV_C2F_PARAM, line_num+1, index, v_name[fun_id]);

        add_instr("%s %s\n", ld, v_name[e.id]);
    }

    // original is float and call is comp acc ---------------------------------

    if ((t_fun == 2) && (t_cal == 3) && (e.id == 0))
    {
        fprintf(stdout, MSG_WARN_CONV_C2F_PARAM, line_num+1, index, v_name[fun_id]);

        add_instr("POP\n");
    }

    // original is comp and call is int var -----------------------------------

    if ((t_fun == 3) && (t_cal == 1) && (e.id != 0))
    {
        fprintf(stdout, MSG_WARN_CONV_I2C_PARAM, line_num+1, index, v_name[fun_id]);

        add_instr("%s %s\n", i2f, v_name[e.id]);
        add_instr("P_LOD 0.0\n");
    }

    // original is comp and call is int acc -----------------------------------

    if ((t_fun == 3) && (t_cal == 1) && (e.id == 0))
    {
        fprintf(stdout, MSG_WARN_CONV_I2C_PARAM, line_num+1, index, v_name[fun_id]);

        add_instr("I2F\n");
        add_instr("P_LOD 0.0\n");
    }

    // original is comp and call is float var ---------------------------------

    if ((t_fun == 3) && (t_cal == 2) && (e.id != 0))
    {
        fprintf(stdout, MSG_WARN_CONV_F2C_PARAM, line_num+1, index, v_name[fun_id]);

        add_instr("%s %s\n", ld, v_name[e.id]);
        add_instr("P_LOD 0.0\n");
    }

    // original is comp and call is float acc ---------------------------------

    if ((t_fun == 3) && (t_cal == 2) && (e.id == 0))
    {
        fprintf(stdout, MSG_WARN_CONV_F2C_PARAM, line_num+1, index, v_name[fun_id]);

        add_instr("P_LOD 0.0\n");
    }

    // original is comp and call is comp const --------------------------------

    if ((t_fun == 3) && (t_cal == 5))
    {
        get_cmp_cst(e,&etr,&eti);

        add_instr("%s %s\n", ld, v_name[etr.id]);
        add_instr("P_LOD %s\n",  v_name[eti.id]);
    }

    // original is comp and call is comp var ----------------------------------

    if ((t_fun == 3) && (t_cal == 3) && (e.id != 0))
    {
        get_cmp_ets(e,&etr,&eti); // gets the extended IDs of the right side in memory

        add_instr("%s %s\n" , ld, v_name[etr.id]);
        add_instr("P_LOD %s\n",     v_name[eti.id]);
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
void declar_fun(int id1, int id2) //id1 -> type, id2 -> name index
{
    // enter this if the first declared function is not main
    // in that case a JMP to main is needed first
    // because main must be the first function after reset
    if ((mainok == 0) && (strcmp(v_name[id2], "main") != 0))
    {
        add_sinst(-2, "JMP main\n");

        mainok = 1; // main-function question resolved
    }
    // enter this if the first declared function is main
    // no JMP needed here
    // just record that the question is resolved in mainok
    else if ((mainok == 0) && (strcmp(v_name[id2], "main") == 0))
    {
        mainok = 1; // defined how the main function will be used
    }

    add_sinst(0, "@%s ", v_name[id2]);

    strcpy(fname, v_name[id2]); // set the fname state to the function being analyzed
    v_type[id2] = id1+6       ; // v_type becomes function (void, int, float, comp) -> (6, 7, 8, 9)
    fun_parse   = id2         ; // set the fun_parse state to the function's name id
    ret_ok      = 0           ; // set ret_ok to zero (function parsing starts)
}

// picks up the first parameter
void declar_fst(int id)
{
    // if comp ...
    if (v_type[id] > 2)
    {
        // first take the img from the stack
        int idi = get_img_id(id);
        add_instr("SET_P %s\n", v_name[idi]);
    }

    // the first function parameter uses SET (since it is the last to be called)
    // the remaining ones (if any) use SET_P in another function
    add_instr("SET %s\n", v_name[id]);
}

// picks up the second parameter and onwards
int declar_par(int type, int id)
{
    declar_var(id); // arrays cannot be passed as function parameters

    // store info about every parameter's data type in a single number
    v_fpar[fun_parse] = v_fpar[fun_parse]*10 + type;

    return id;
}

// emits SET_P on each parameter as we walk through them
void set_par(int id)
{
    // if comp
    if (v_type[id] > 2)
    {
        int idi = get_img_id(id);
        add_instr("SET_P %s\n", v_name[idi]);
    }
        add_instr("SET_P %s\n", v_name[id] );
}

// when the return keyword is found
void declar_ret(expr e, int ret)
{
    // check whether it really is a function, or void by mistake
    if (v_type[fun_parse] == 6)
        {fprintf (stderr, MSG_ERR_VOID_RETURN_VALUE, line_num+1); exit(EXIT_FAILURE);}

    // test whether it is inside an if/else
    //if ((get_if() > 0) && (v_type[fun_parse] != 6))
        //fprintf(stdout, "Heads up on line %d: using return inside if/else can break if you forget it somewhere!\n", line_num+1);

    // ------------------------------------------------------------------------
    // check every combination ------------------------------------------------
    // ------------------------------------------------------------------------

    expr etr, eti;
    int left_type = v_type[fun_parse];

    // int with int var
    if ((left_type == 7) && (e.type == 1) && (e.id!=0))
    {
        add_instr("LOD %s\n", v_name[e.id]);
    }

    // int with int acc
    if ((left_type == 7) && (e.type == 1) && (e.id==0))
    {
        // nothing to do
    }

    // int with float var
    if ((left_type == 7) && (e.type == 2) && (e.id!=0))
    {
        fprintf(stdout, MSG_WARN_CONV_F2I_RETURN, line_num+1, v_name[fun_parse]);

        add_instr("F2I_M %s\n", v_name[e.id]);
    }

    // int with float acc
    if ((left_type == 7) && (e.type == 2) && (e.id==0))
    {
        fprintf(stdout, MSG_WARN_CONV_F2I_RETURN, line_num+1, v_name[fun_parse]);
        add_instr("F2I\n");
    }

    // int with comp const
    if ((left_type == 7) && (e.type == 5))
    {
        fprintf (stdout, MSG_WARN_ROUND_REAL, line_num+1);

        get_cmp_cst(e,&etr,&eti);

        add_instr("F2I_M %s\n", v_name[etr.id]);
    }

    // int with comp var
    if ((left_type == 7) && (e.type == 3) && (e.id!=0))
    {
        fprintf (stdout, MSG_WARN_ROUND_REAL, line_num+1);

        get_cmp_ets(e,&etr,&eti);

        add_instr("F2I_M %s\n", v_name[etr.id]);
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

        add_instr("I2F_M %s\n", v_name[e.id]);
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
        add_instr("LOD %s\n", v_name[e.id]);
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

        add_instr("LOD %s\n", v_name[etr.id]);
    }

    // float with comp var
    if ((left_type == 8) && (e.type == 3) && (e.id!=0))
    {
        fprintf (stdout, MSG_WARN_GRAB_REAL_ONLY, line_num+1);

        get_cmp_ets(e,&etr,&eti);

        add_instr("LOD %s\n", v_name[etr.id]);
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

        add_instr("I2F_M %s\n", v_name[e.id]);
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

        add_instr("LOD %s\n", v_name[e.id]);
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

        add_instr("LOD %s\n"  , v_name[etr.id]);
        add_instr("P_LOD %s\n", v_name[eti.id]);
    }

    // comp with comp var
    if ((left_type == 9) && (e.type == 3) && (e.id!=0))
    {
        get_cmp_ets(e,&etr,&eti);

        add_instr("LOD %s\n"  , v_name[etr.id]);
        add_instr("P_LOD %s\n", v_name[eti.id]);
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

// fires at the opening '{' of a function body. Starts capturing every
// emission until the matching '}' so the body becomes one AST_RAW leaf
// inside an AST_BLOCK, ready for the ast_emit walker.
void func_body_begin(void)
{
    emit_push_capture();
}

// end of the parsing for a function declaration
void func_ret(int id) // id -> id of the current function
{
    // pop the body capture and replay it through the AST walker. The body
    // is still a single opaque chunk for now (one AST_RAW kid); future
    // migrations can split it into per-statement kids without touching
    // this code path.
    char     *body_text = emit_pop_capture();
    ast_node *body_raw  = ast_raw(body_text);
    free(body_text);

    ast_node *block = ast_block();
    ast_block_push(block, body_raw);
    ast_emit(block);
    ast_free(block);

    // check whether the function had the return x; instruction
    if ((v_type[id] != 6) && (ret_ok == 0))
        {fprintf (stderr, MSG_ERR_FUNC_NO_RETURN, v_name[id]); exit(EXIT_FAILURE);}

    // if it is the main function, emit a JMP fim
    if (strcmp(v_name[id], "main") == 0)
    {
        add_sinst(-3, "@fim JMP fim\n");

        v_used[id] = 1; // main was used (avoid the warning of main declared but not used)
    }
    else if (v_type[id] == 6) {add_instr("RET\n");} // void type still needs a RET

    // env variable fname becomes empty (left a function)
    strcpy(fname, "");
}

// return without exp (return;)
void void_ret()
{
    // check whether it really is void, or a non-void by mistake
    if (v_type[fun_parse] != 6)
        {fprintf (stderr, MSG_ERR_NO_RETURN_VALUE, line_num+1); exit(EXIT_FAILURE);}

    // if main, use JMP fim instead of RET
    if ((strcmp(fname, "main") == 0))
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

// void-call statement: pre-validates type / arity, builds an EXPR_FUNC_CALL
// with type=0 (void), drives the walker (which emits args + par_check + CAL),
// then frees the throwaway node.
void vcall(int id)
{
    if  (v_type[id] < 6)
    {
        fprintf(stderr, MSG_ERR_FUNC_WHERE, line_num+1, rem_fname(v_name[id], fname));
        exit(EXIT_FAILURE);
    }

    int n; expr_node **a = args_frame_pop(&n);

    if (n != get_npar(v_fpar[id]))
    {
        fprintf(stderr, MSG_ERR_PARAM_COUNT, line_num+1, rem_fname(v_name[id], fname));
        exit(EXIT_FAILURE);
    }

    v_used[id] = 1;

    // type=0 for void: walker leaves acc_ok=0 after CAL
    expr_node *node = expr_func_call(0, id, a, n);
    ast_emit_expr(node);
    expr_free(node);
}

// value-returning call: pre-validates and packages the call as an
// EXPR_FUNC_CALL node, hands the node up to whichever consumer reduces next.
// No emit at parse time - the walker emits when the consumer calls EE($N).
expr_node *fcall(int id)
{
    if (v_type[id] == 6)
    {
        fprintf (stderr, MSG_ERR_VOID_FUNC_USE, line_num+1, v_name[id]);
        exit(EXIT_FAILURE);
    }
    else if (v_type[id] < 6)
    {
        fprintf (stderr, MSG_ERR_FUNC_WHERE2, line_num+1, rem_fname(v_name[id], fname));
        exit(EXIT_FAILURE);
    }

    int n; expr_node **a = args_frame_pop(&n);

    if (n != get_npar(v_fpar[id]))
    {
        fprintf(stderr, MSG_ERR_PARAM_LIST_DIFF, line_num+1, v_name[id]);
        exit(EXIT_FAILURE);
    }

    v_used[id] = 1;

    return expr_func_call(v_type[id]-6, id, a, n);
}
