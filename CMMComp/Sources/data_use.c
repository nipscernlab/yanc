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
#include "..\Headers\t2t.h"
#include "..\Headers\oper.h"
#include "..\Headers\global.h"
#include "..\Headers\funcoes.h"
#include "..\Headers\data_use.h"
#include "..\Headers\variaveis.h"
#include "..\Headers\data_assign.h"
#include "..\Headers\array_index.h"
#include "..\Headers\messages.h"

// constant reduction into exp
// does not emit a load, just updates the variable state
int num2exp(int id, int dtype)
{
    v_used[id] = 1;
    v_isco[id] = 1;
    v_isar[id] = 0;
    v_type[id] = dtype;

    return dtype*OFST+id;
}

// ID reduction into exp
// does not emit a load yet, just checks and updates the variable state
int id2exp(int id)
{
    // test whether the variable has already been declared
    if (v_type[id] == 0)
        {fprintf (stderr, MSG_ERR_DECL_VAR_PROPERLY, line_num+1, rem_fname(v_name[id], fname)); exit(EXIT_FAILURE);}

    // if it is an array, the index is missing
    if (v_isar[id] > 0)
        {fprintf (stderr, MSG_ERR_MISSING_ARR_IDX, line_num+1, rem_fname(v_name[id], fname)); exit(EXIT_FAILURE);}

    v_used[id] = 1;

    return v_type[id]*OFST+id;
}

// ++ reduction into exp
int pplus2exp(int id)
{
    if (v_type[id] > 2)
        {fprintf (stderr, MSG_ERR_INCR_COMPLEX, line_num+1); exit(EXIT_FAILURE);}

    // equivalent to taking x in the expression (x+1)
    int et = id2exp(id);

    // now turn the 1 into an exp
    // first run the lexer on 1
    if (find_var("1") == -1) add_var("1");
    int lval = find_var("1");
    // decide whether it comes from INUM or FNUM
    int type = get_type(et);
    // then the parser
    int et1 = num2exp(lval,type);
    // then perform the addition
    int ret = oper_soma(et,et1);
    // finally, assign back to id
    ass_set(id, ret);

    acc_ok = 1; // cannot free acc, since it is an exp

    return ret;
}

// ++ reduction into exp on a 1D array
int pplus1d2exp(int id, int ete)
{
    if (v_type[id] > 2)
        {fprintf (stderr, MSG_ERR_INCR_COMPLEX, line_num+1); exit(EXIT_FAILURE);}

    // equivalent to taking x in the expression (x+1)
    int et = arr_1d2exp(id,ete,0);
    // now turn the 1 into an exp
    // first run the lexer on 1
    if (find_var("1") == -1) add_var("1");
    int lval = find_var("1");
    // decide whether it comes from INUM or FNUM
    int type = get_type(et);
    // then the parser
    int et1 = num2exp(lval,type);
    // then perform the addition
    int ret = oper_soma(et,et1);
    // reload the array index
    arr_1d_index(id, ete);
    // finally, assign back to id
    ass_array(id, ret, 0);

    acc_ok = 1; // cannot free acc, since it is an exp

    return ret;
}

// ++ reduction into exp on a 2D array
int pplus2d2exp(int id, int et1, int et2)
{
    if (v_type[id] > 2)
        {fprintf (stderr, MSG_ERR_INCR_COMPLEX, line_num+1); exit(EXIT_FAILURE);}

    // equivalent to taking x in the expression (x+1)
    int et = arr_2d2exp(id,et1,et2);
    // now turn the 1 into an exp
    // first run the lexer on 1
    if (find_var("1") == -1) add_var("1");
    int lval = find_var("1");
    // decide whether it comes from INUM or FNUM
    int type = get_type(et);
    // then the parser
    int etx = num2exp(lval,type);
    // then perform the addition
    int ret = oper_soma(et,etx);
    // reload the array index
    arr_2d_index(id, et1, et2);
    // finally, assign back to id
    ass_array(id, ret, 0);

    acc_ok = 1; // cannot free acc, since it is an exp

    return ret;
}
