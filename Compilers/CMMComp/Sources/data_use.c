// ----------------------------------------------------------------------------
// routines for exp reduction -------------------------------------------------
// ----------------------------------------------------------------------------

/*
TODO:
1- revisit the ++ increment operators
*/

// global includes
#include  <stdio.h>
#include <stdlib.h>

// local includes
#include "../Headers/t2t.h"
#include "../Headers/oper.h"
#include "../Headers/global.h"
#include "../Headers/funcoes.h"
#include "../Headers/data_use.h"
#include "../Headers/variaveis.h"
#include "../Headers/data_assign.h"
#include "../Headers/array_index.h"
#include "../Headers/messages.h"

// constant reduction into expr
// does not emit a load, just updates the variable state
expr num2exp(int id, int dtype)
{
    v_table[id].used          = 1;
    v_table[id].isco    = 1;
    v_table[id].isar          = 0;
    v_table[id].type          = dtype;

    return expr_make(dtype, id);
}

// ID reduction into expr
// does not emit a load yet, just checks and updates the variable state
expr id2exp(int id)
{
    // test whether the variable has already been declared
    if (v_table[id].type == 0)
        {fprintf (stderr, MSG_ERR_DECL_VAR_PROPERLY, line_num+1, rem_fname(v_table[id].name, fname)); exit(EXIT_FAILURE);}

    // if it is an array, the index is missing
    if (v_table[id].isar > 0)
        {fprintf (stderr, MSG_ERR_MISSING_ARR_IDX, line_num+1, rem_fname(v_table[id].name, fname)); exit(EXIT_FAILURE);}

    v_table[id].used = 1;

    return expr_make(v_table[id].type, id);
}

// ++ reduction into expr
expr pplus2exp(int id, int old)
{
    if (v_table[id].type > 2)
        {fprintf (stderr, MSG_ERR_INCR_COMPLEX, line_num+1); exit(EXIT_FAILURE);}

    // equivalent to taking x in the expression (x+1)
    expr e = id2exp(id);

    // the old value is wanted: load x now and keep a copy on the stack, which
    // SET_P (SET + POP) brings back after the store; a live operand in the
    // acc goes to the stack first, as everywhere else (P_LOD)
    if (old)
    {
        add_instr(acc_ok == 1 ? "P_LOD %s\n" : "LOD %s\n", v_table[id].name);
        add_instr("PSH\n");
        e = expr_make(e.type, 0);   // x in the acc
        acc_ok = 1;
    }

    // now turn the 1 into an expr (same data type as x): the name is the
    // assembler operand, so a float x needs "1.0" -- "1" would be assembled
    // as the int 1, whose bits the float adder reads as an unnormalised number
    char *one = (e.type == 2) ? "1.0" : "1";
    if (find_var(one) == -1) add_var(one);
    int  lval = find_var(one);
    expr e1   = num2exp(lval, e.type);

    // then perform the addition
    expr ret = oper_soma(e, e1);

    // finally, assign back to id; in an expression the value of `x++` is the
    // old x, which SET_P pops back into the acc after the store (id2exp above
    // made ass_set's checks: declared, not an array)
    if (old) add_instr("SET_P %s\n", v_table[id].name);
    else     ass_set(id, ret);

    acc_ok = 1; // cannot free acc, since it is an exp

    return ret;
}

// ++ reduction into expr on a 1D array
expr pplus1d2exp(int id, expr ete, int old)
{
    if (v_table[id].type > 2)
        {fprintf (stderr, MSG_ERR_INCR_COMPLEX, line_num+1); exit(EXIT_FAILURE);}

    // a live operand in the acc (e.g. the left side of `y + x[i]++`) goes to
    // the stack before the index load overwrites it; an index that is itself
    // in the acc had its own subtree do that (as arr_1d2exp's P_LOD does)
    if (acc_ok == 1 && ete.id != 0) add_instr("PSH\n");
    // the index goes to the acc and then onto the stack, where STI will take
    // it; PSH leaves the acc as it was, so LDI still sees the index
    arr_1d_index(id, ete);
    add_instr("PSH\n");
    add_instr("LDI %s\n", v_table[id].name);
    acc_ok = 1;                                // x[i] in the acc
    expr e = expr_make(v_table[id].type, 0);
    // a float element whose old value is wanted: keep a copy (the index sits
    // on the stack top for STI, so the stack cannot hold it)
    if (old && e.type == 2) add_instr("SET aux_var\n");

    // now turn the 1 into an expr (same data type as x): the name is the
    // assembler operand, so a float x needs "1.0" -- "1" would be assembled
    // as the int 1, whose bits the float adder reads as an unnormalised number
    char *one = (e.type == 2) ? "1.0" : "1";
    if (find_var(one) == -1) add_var(one);
    int  lval = find_var(one);
    expr e1   = num2exp(lval, e.type);

    // then perform the addition
    expr ret = oper_soma(e, e1);

    // assign back: the value is in the acc, the index on the stack
    ass_array(id, ret, 0);

    // the value of `x[i]++` in an expression is the old element
    if (old) add_instr(e.type == 2 ? "LOD aux_var\n" : "ADD -1\n");

    acc_ok = 1; // cannot free acc, since it is an exp

    return ret;
}

// ++ reduction into expr on a 2D array
expr pplus2d2exp(int id, expr e1, expr e2, int old)
{
    if (v_table[id].type > 2)
        {fprintf (stderr, MSG_ERR_INCR_COMPLEX, line_num+1); exit(EXIT_FAILURE);}

    // a live operand in the acc goes to the stack first (see 1-D)
    if (acc_ok == 1 && e1.id != 0 && e2.id != 0) add_instr("PSH\n");
    // the linear index goes to the acc and then onto the stack (see 1-D)
    arr_2d_index(id, e1, e2);
    add_instr("PSH\n");
    add_instr("LDI %s\n", v_table[id].name);
    acc_ok = 1;                                // x[i][j] in the acc
    expr e = expr_make(v_table[id].type, 0);
    if (old && e.type == 2) add_instr("SET aux_var\n");   // see 1-D

    // now turn the 1 into an expr (same data type as x): "1.0" for a float
    // (see pplus2exp)
    char *one = (e.type == 2) ? "1.0" : "1";
    if (find_var(one) == -1) add_var(one);
    int  lval = find_var(one);
    expr eone = num2exp(lval, e.type);

    // then perform the addition
    expr ret = oper_soma(e, eone);

    // assign back: the value is in the acc, the index on the stack
    ass_array(id, ret, 0);

    // the value of `x[i][j]++` in an expression is the old element
    if (old) add_instr(e.type == 2 ? "LOD aux_var\n" : "ADD -1\n");

    acc_ok = 1; // cannot free acc, since it is an exp

    return ret;
}
