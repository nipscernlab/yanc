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
expr pplus2exp(int id)
{
    if (v_table[id].type > 2)
        {fprintf (stderr, MSG_ERR_INCR_COMPLEX, line_num+1); exit(EXIT_FAILURE);}

    // equivalent to taking x in the expression (x+1)
    expr e = id2exp(id);

    // now turn the 1 into an expr (same data type as x)
    if (find_var("1") == -1) add_var("1");
    int  lval = find_var("1");
    expr e1   = num2exp(lval, e.type);

    // then perform the addition
    expr ret = oper_soma(e, e1);

    // finally, assign back to id (ass_set still consumes int et)
    ass_set(id, ret);

    acc_ok = 1; // cannot free acc, since it is an exp

    return ret;
}

// ++ reduction into expr on a 1D array
expr pplus1d2exp(int id, expr ete)
{
    if (v_table[id].type > 2)
        {fprintf (stderr, MSG_ERR_INCR_COMPLEX, line_num+1); exit(EXIT_FAILURE);}

    // equivalent to taking x in the expression (x+1)
    expr e = arr_1d2exp(id, ete, 0);

    // now turn the 1 into an expr (same data type as x)
    if (find_var("1") == -1) add_var("1");
    int  lval = find_var("1");
    expr e1   = num2exp(lval, e.type);

    // then perform the addition
    expr ret = oper_soma(e, e1);

    // reload the array index and assign back to id (ass_array still int)
    arr_1d_index(id, ete);
    ass_array  (id, ret, 0);

    acc_ok = 1; // cannot free acc, since it is an exp

    return ret;
}

// ++ reduction into expr on a 2D array
expr pplus2d2exp(int id, expr e1, expr e2)
{
    if (v_table[id].type > 2)
        {fprintf (stderr, MSG_ERR_INCR_COMPLEX, line_num+1); exit(EXIT_FAILURE);}

    // equivalent to taking x in the expression (x+1)
    expr e = arr_2d2exp(id, e1, e2);

    // now turn the 1 into an expr (same data type as x)
    if (find_var("1") == -1) add_var("1");
    int  lval = find_var("1");
    expr eone = num2exp(lval, e.type);

    // then perform the addition
    expr ret = oper_soma(e, eone);

    // reload the array index and assign back to id (ass_array still int)
    arr_2d_index(id, e1, e2);
    ass_array   (id, ret, 0);

    acc_ok = 1; // cannot free acc, since it is an exp

    return ret;
}
