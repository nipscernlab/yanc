// ----------------------------------------------------------------------------
// array index handling -------------------------------------------------------
// ----------------------------------------------------------------------------

// global includes
#include  <stdio.h>
#include <string.h>
#include <stdlib.h>

// local includes
#include "../Headers/array_index.h"
#include "../Headers/convert.h"
#include "../Headers/global.h"
#include "../Headers/functions.h"
#include "../Headers/data_use.h"
#include "../Headers/symbols.h"
#include "../Headers/messages.h"
#include "../Headers/diag.h"

// ----------------------------------------------------------------------------
// array in assignment, e.g. x[i] = y; ----------------------------------------
// ----------------------------------------------------------------------------

// the index e as an int in the acc, the conversion it needs on the way: an
// int is loaded, a float converted (F2I, F2I_M) with the warning wfloat, a
// comp by its real part (POP of a comp in the acc) with
// MSG_WARN_IDX_COMP_ROUND. push: the P_ forms, the acc holding a value to keep
static void load_index(expr e, int push, const char *wfloat)
{
    const char *ld  = push ? "P_LOD"   : "LOD";
    const char *f2i = push ? "P_F2I_M" : "F2I_M";
    if (e.type == TY_INT)
    {
        if (e.id != 0) add_instr("%s %s\n", ld, v_table[e.id].name);
        return;
    }
    if (e.type == TY_FLOAT)
    {
        warn(wfloat, pctx.line_num+1);
        if (e.id != 0) add_instr("%s %s\n", f2i, v_table[e.id].name);
        else           add_instr("F2I\n");
        return;
    }
    warn(MSG_WARN_IDX_COMP_ROUND, pctx.line_num+1);
    if (e.type == TY_CCONST)
    {
        expr etr, eti;
        get_cmp_cst(e, &etr, &eti);
        add_instr("%s %s\n", f2i, v_table[etr.id].name);
    }
    else if (e.id == 0) {add_instr("POP\n"); add_instr("F2I\n");}
    else                add_instr("%s %s\n", f2i, v_table[e.id].name);
}

// loads the array index into the accumulator (1D array)
void arr_1d_index(int id, expr e)
{
    // ------------------------------------------------------------------------
    // argument check ---------------------------------------------------------
    // ------------------------------------------------------------------------

    // must check that it really is an array
    if (v_table[id].dims == AR_NONE)
        fatal(MSG_ERR_NOT_ARRAY_HARSH, pctx.line_num+1, strip_fn_prefix(v_table[id].name));

    // must check that it really is a 1D array
    if (v_table[id].dims == AR_2D)
        fatal(MSG_ERR_ARRAY_2D  , pctx.line_num+1, strip_fn_prefix(v_table[id].name));

    // ------------------------------------------------------------------------
    // execute ----------------------------------------------------------------
    // ------------------------------------------------------------------------

    load_index(e, 0, MSG_WARN_IDX_FLOAT);

    acc_live = 1; // acc carregado
}

// loads the array index into the accumulator (2D array)
void arr_2d_index(int id, expr e1, expr e2)
{
    // ------------------------------------------------------------------------
    // argument check ---------------------------------------------------------
    // ------------------------------------------------------------------------

    // must check that it really is an array
    if (v_table[id].dims == AR_NONE)
        fatal(MSG_ERR_NOT_ARRAY_HARSH, pctx.line_num+1, strip_fn_prefix(v_table[id].name));

    // must check that it is not a 1D array
    if (v_table[id].dims == AR_1D)
        fatal(MSG_ERR_ARRAY_1D, pctx.line_num+1, strip_fn_prefix(v_table[id].name));

    // ------------------------------------------------------------------------
    // execute ----------------------------------------------------------------
    // ------------------------------------------------------------------------

    expr etr, eti;

    // int in acc and int in acc
    if ((e1.type == TY_INT) && (e1.id == 0) && (e2.type == TY_INT) && (e2.id == 0))
    {
        add_instr("SET_P __aux_var\n");
        add_instr("MLT   %d\n", v_table[id].cols);
        add_instr("ADD   __aux_var\n");
    }

    // int in acc and int in memory
    if ((e1.type == TY_INT) && (e1.id == 0) && (e2.type == TY_INT) && (e2.id != 0))
    {
        add_instr("MLT %d\n", v_table[id].cols);
        add_instr("ADD %s\n",  v_table[e2.id].name);
    }

    // int in acc and float in acc
    if ((e1.type == TY_INT) && (e1.id == 0) && (e2.type == TY_FLOAT) && (e2.id == 0))
    {
        warn(MSG_WARN_IDX2_FLOAT, pctx.line_num+1);

        add_instr("F2I\n");
        add_instr("SET_P __aux_var\n");
        add_instr("MLT   %d\n", v_table[id].cols);
        add_instr("ADD   __aux_var\n");
    }

    // int in acc and float in memory
    if ((e1.type == TY_INT) && (e1.id == 0) && (e2.type == TY_FLOAT) && (e2.id != 0))
    {
        warn(MSG_WARN_IDX2_FLOAT, pctx.line_num+1);

        add_instr("MLT     %d\n", v_table[id].cols);
        add_instr("P_F2I_M %s\n" , v_table[e2.id].name);
        add_instr("S_ADD\n");
    }

    // int in acc and comp const in memory
    if ((e1.type == TY_INT) && (e1.id == 0) && (e2.type == TY_CCONST) && (e2.id != 0))
    {
        warn(MSG_WARN_IDX_COMP_GRAB, pctx.line_num+1);

        get_cmp_cst(e2, &etr, &eti);

        add_instr("MLT     %d\n", v_table[id].cols);
        add_instr("P_F2I_M %s\n" , v_table[etr.id].name);
        add_instr("S_ADD\n");
    }

    // int in acc and comp in acc
    if ((e1.type == TY_INT) && (e1.id == 0) && (e2.type == TY_COMP) && (e2.id == 0))
    {
        warn(MSG_WARN_IDX_COMP_GRAB, pctx.line_num+1);

        add_instr("SET_P __aux_var\n");
        add_instr("F2I\n");
        add_instr("SET_P __aux_var\n");
        add_instr("MLT   %d\n", v_table[id].cols);
        add_instr("ADD   __aux_var\n");
    }

    // int in acc and comp in memory
    if ((e1.type == TY_INT) && (e1.id == 0) && (e2.type == TY_COMP) && (e2.id != 0))
    {
        warn(MSG_WARN_IDX_COMP_GRAB, pctx.line_num+1);

        add_instr("MLT     %d\n", v_table[id].cols);
        add_instr("P_F2I_M %s\n" , v_table[e2.id].name);
        add_instr("S_ADD\n");
    }

    // int in memory and int in acc
    if ((e1.type == TY_INT) && (e1.id != 0) && (e2.type == TY_INT) && (e2.id == 0))
    {
        add_instr("P_LOD %s\n",  v_table[e1.id].name);
        add_instr("MLT   %d\n", v_table[id].cols);
        add_instr("S_ADD\n");
    }

    // int in memory and int in memory
    if ((e1.type == TY_INT) && (e1.id != 0) && (e2.type == TY_INT) && (e2.id != 0))
    {
        add_instr("LOD  %s\n",  v_table[e1.id].name);
        add_instr("MLT  %d\n", v_table[id].cols);
        add_instr("ADD  %s\n",  v_table[e2.id].name);
    }

    // int in memory and float in acc
    if ((e1.type == TY_INT) && (e1.id != 0) && (e2.type == TY_FLOAT) && (e2.id == 0))
    {
        warn(MSG_WARN_IDX2_FLOAT, pctx.line_num+1);

        add_instr("F2I\n");
        add_instr("P_LOD %s\n",  v_table[e1.id].name);
        add_instr("MLT   %d\n", v_table[id].cols);
        add_instr("S_ADD\n");
    }

    // int in memory and float in memory
    if ((e1.type == TY_INT) && (e1.id != 0) && (e2.type == TY_FLOAT) && (e2.id != 0))
    {
        warn(MSG_WARN_IDX2_FLOAT, pctx.line_num+1);

        add_instr("LOD     %s\n",  v_table[e1.id].name);
        add_instr("MLT     %d\n", v_table[id].cols);
        add_instr("P_F2I_M %s\n",  v_table[e2.id].name);
        add_instr("S_ADD\n");
    }

    // int in memory and comp const in memory
    if ((e1.type == TY_INT) && (e1.id != 0) && (e2.type == TY_CCONST) && (e2.id != 0))
    {
        warn(MSG_WARN_IDX_COMP_GRAB, pctx.line_num+1);

        get_cmp_cst(e2, &etr, &eti);

        add_instr("LOD     %s\n",  v_table[e1.id].name);
        add_instr("MLT     %d\n", v_table[id].cols);
        add_instr("P_F2I_M %s\n",  v_table[etr.id].name);
        add_instr("S_ADD\n");
    }

    // int in memory and comp in acc
    if ((e1.type == TY_INT) && (e1.id != 0) && (e2.type == TY_COMP) && (e2.id == 0))
    {
        warn(MSG_WARN_IDX_COMP_GRAB, pctx.line_num+1);

        add_instr("SET_P __aux_var\n");
        add_instr("F2I\n");
        add_instr("P_LOD %s\n", v_table[e1.id].name);
        add_instr("MLT   %d\n", v_table[id].cols);
        add_instr("S_ADD\n");
    }

    // int in memory and comp in memory
    if ((e1.type == TY_INT) && (e1.id != 0) && (e2.type == TY_COMP) && (e2.id != 0))
    {
        warn(MSG_WARN_IDX_COMP_GRAB, pctx.line_num+1);

        add_instr("LOD     %s\n",  v_table[e1.id].name);
        add_instr("MLT     %d\n", v_table[id].cols);
        add_instr("P_F2I_M %s\n",  v_table[e2.id].name);
        add_instr("S_ADD\n");
    }

    // float in acc and int in acc
    if ((e1.type == TY_FLOAT) && (e1.id == 0) && (e2.type == TY_INT) && (e2.id == 0))
    {
        warn(MSG_WARN_IDX1_FLOAT, pctx.line_num+1);

        add_instr("SET_P __aux_var\n");
        add_instr("F2I\n");
        add_instr("MLT   %d\n", v_table[id].cols);
        add_instr("ADD   __aux_var\n");
    }

    // float in acc and int in memory
    if ((e1.type == TY_FLOAT) && (e1.id == 0) && (e2.type == TY_INT) && (e2.id != 0))
    {
        warn(MSG_WARN_IDX1_FLOAT, pctx.line_num+1);

        add_instr("F2I\n");
        add_instr("MLT %d\n", v_table[id].cols);
        add_instr("ADD %s\n",  v_table[e2.id].name);
    }

    // float in acc and float in acc
    if ((e1.type == TY_FLOAT) && (e1.id == 0) && (e2.type == TY_FLOAT) && (e2.id == 0))
    {
        warn(MSG_WARN_IDXS_FLOAT, pctx.line_num+1);

        add_instr("F2I\n");
        add_instr("SET_P __aux_var\n");
        add_instr("F2I\n");
        add_instr("MLT   %d\n", v_table[id].cols);
        add_instr("ADD   __aux_var\n");
    }

    // float in acc and float in memory
    if ((e1.type == TY_FLOAT) && (e1.id == 0) && (e2.type == TY_FLOAT) && (e2.id != 0))
    {
        warn(MSG_WARN_IDXS_MESS, pctx.line_num+1);

        add_instr("F2I\n");
        add_instr("MLT     %d\n", v_table[id].cols);
        add_instr("P_F2I_M %s\n" , v_table[e2.id].name);
        add_instr("S_ADD\n");
    }

    // float in acc and comp const in memory
    if ((e1.type == TY_FLOAT) && (e1.id == 0) && (e2.type == TY_CCONST) && (e2.id != 0))
    {
        warn(MSG_WARN_IDXS_MESS, pctx.line_num+1);

        get_cmp_cst(e2, &etr, &eti);

        add_instr("F2I\n");
        add_instr("MLT     %d\n", v_table[id].cols);
        add_instr("P_F2I_M %s\n" , v_table[etr.id].name);
        add_instr("S_ADD\n");
    }

    // float in acc and comp in acc
    if ((e1.type == TY_FLOAT) && (e1.id == 0) && (e2.type == TY_COMP) && (e2.id == 0))
    {
        warn(MSG_WARN_IDXS_MESS, pctx.line_num+1);

        add_instr("SET_P __aux_var\n");
        add_instr("F2I\n");
        add_instr("SET_P __aux_var\n");
        add_instr("F2I\n");
        add_instr("MLT   %d\n", v_table[id].cols);
        add_instr("ADD   __aux_var\n");
    }

    // float in acc and comp in memory
    if ((e1.type == TY_FLOAT) && (e1.id == 0) && (e2.type == TY_COMP) && (e2.id != 0))
    {
        warn(MSG_WARN_IDXS_MESS, pctx.line_num+1);

        add_instr("F2I\n");
        add_instr("MLT     %d\n", v_table[id].cols);
        add_instr("P_F2I_M %s\n" , v_table[e2.id].name);
        add_instr("S_ADD\n");
    }

    // float in memory and int in acc
    if ((e1.type == TY_FLOAT) && (e1.id != 0) && (e2.type == TY_INT) && (e2.id == 0))
    {
        warn(MSG_WARN_IDX1_FLOAT, pctx.line_num+1);

        add_instr("P_F2I_M %s\n" , v_table[e1.id].name);
        add_instr("MLT     %d\n", v_table[id].cols);
        add_instr("S_ADD\n");
    }

    // float in memory and int in memory
    if ((e1.type == TY_FLOAT) && (e1.id != 0) && (e2.type == TY_INT) && (e2.id != 0))
    {
        warn(MSG_WARN_IDX1_FLOAT, pctx.line_num+1);

        add_instr("F2I_M  %s\n",  v_table[e1.id].name);
        add_instr("MLT    %d\n", v_table[id].cols);
        add_instr("ADD    %s\n",  v_table[e2.id].name);
    }

    // float in memory and float in acc
    if ((e1.type == TY_FLOAT) && (e1.id != 0) && (e2.type == TY_FLOAT) && (e2.id == 0))
    {
        warn(MSG_WARN_IDXS_FLOAT, pctx.line_num+1);

        add_instr("F2I\n");
        add_instr("P_F2I_M %s\n",  v_table[e1.id].name);
        add_instr("MLT     %d\n", v_table[id].cols);
        add_instr("S_ADD\n");
    }

    // float in memory and float in memory
    if ((e1.type == TY_FLOAT) && (e1.id != 0) && (e2.type == TY_FLOAT) && (e2.id != 0))
    {
        warn(MSG_WARN_IDXS_FLOAT, pctx.line_num+1);

        add_instr("F2I_M   %s\n",  v_table[e1.id].name);
        add_instr("MLT     %d\n", v_table[id].cols);
        add_instr("P_F2I_M %s\n",  v_table[e2.id].name);
        add_instr("S_ADD\n");
    }

    // float in memory and comp const in memory
    if ((e1.type == TY_FLOAT) && (e1.id != 0) && (e2.type == TY_CCONST) && (e2.id != 0))
    {
        warn(MSG_WARN_IDXS_MESS, pctx.line_num+1);

        get_cmp_cst(e2, &etr, &eti);

        add_instr("F2I_M   %s\n",  v_table[e1.id].name);
        add_instr("MLT     %d\n", v_table[id].cols);
        add_instr("P_F2I_M %s\n",  v_table[etr.id].name);
        add_instr("S_ADD\n");
    }

    // float in memory and comp in acc
    if ((e1.type == TY_FLOAT) && (e1.id != 0) && (e2.type == TY_COMP) && (e2.id == 0))
    {
        warn(MSG_WARN_IDXS_MESS, pctx.line_num+1);

        add_instr("SET_P __aux_var\n");
        add_instr("F2I\n");
        add_instr("P_F2I_M %s\n", v_table[e1.id].name);
        add_instr("MLT   %d\n", v_table[id].cols);
        add_instr("S_ADD\n");
    }

    // float in memory and comp in memory
    if ((e1.type == TY_FLOAT) && (e1.id != 0) && (e2.type == TY_COMP) && (e2.id != 0))
    {
        warn(MSG_WARN_IDXS_MESS, pctx.line_num+1);

        add_instr("F2I_M   %s\n",  v_table[e1.id].name);
        add_instr("MLT     %d\n", v_table[id].cols);
        add_instr("P_F2I_M %s\n",  v_table[e2.id].name);
        add_instr("S_ADD\n");
    }

    // comp const in memory and int in acc
    if ((e1.type == TY_CCONST) && (e1.id != 0) && (e2.type == TY_INT) && (e2.id == 0))
    {
        warn(MSG_WARN_IDX_COMP_GRAB, pctx.line_num+1);

        get_cmp_cst(e1, &etr, &eti);

        add_instr("P_F2I_M %s\n", v_table[etr.id].name);
        add_instr("MLT   %d\n", v_table[id].cols);
        add_instr("S_ADD\n");
    }

    // comp const in memory and int in memory
    if ((e1.type == TY_CCONST) && (e1.id != 0) && (e2.type == TY_INT) && (e2.id != 0))
    {
        warn(MSG_WARN_IDX_COMP_GRAB, pctx.line_num+1);

        get_cmp_cst(e1, &etr, &eti);

        add_instr("F2I_M %s\n" , v_table[etr.id].name);
        add_instr("MLT   %d\n", v_table[id].cols);
        add_instr("ADD   %s\n",  v_table[e2.id].name);
    }

    // comp const in memory and float in acc
    if ((e1.type == TY_CCONST) && (e1.id != 0) && (e2.type == TY_FLOAT) && (e2.id == 0))
    {
        warn(MSG_WARN_IDXS_MESS, pctx.line_num+1);

        get_cmp_cst(e1, &etr, &eti);

        add_instr("F2I\n");
        add_instr("P_F2I_M %s\n", v_table[etr.id].name);
        add_instr("MLT   %d\n", v_table[id].cols);
        add_instr("S_ADD\n");
    }

    // comp const in memory and float in memory
    if ((e1.type == TY_CCONST) && (e1.id != 0) && (e2.type == TY_FLOAT) && (e2.id != 0))
    {
        warn(MSG_WARN_IDXS_MESS, pctx.line_num+1);

        get_cmp_cst(e1, &etr, &eti);

        add_instr("F2I_M   %s\n" , v_table[etr.id].name);
        add_instr("MLT     %d\n", v_table[id].cols);
        add_instr("P_F2I_M %s\n" , v_table[e2.id].name);
        add_instr("S_ADD\n");
    }

    // comp const in memory and comp const in memory
    if ((e1.type == TY_CCONST) && (e1.id != 0) && (e2.type == TY_CCONST) && (e2.id != 0))
    {
        warn(MSG_WARN_IDXS_MESS, pctx.line_num+1);

        get_cmp_cst(e1, &etr, &eti);
        add_instr("F2I_M   %s\n" , v_table[etr.id].name);
        add_instr("MLT     %d\n", v_table[id].cols);

        get_cmp_cst(e2, &etr, &eti);
        add_instr("P_F2I_M %s\n" , v_table[etr.id].name);
        add_instr("S_ADD\n");
    }

    // comp const in memory and comp in acc
    if ((e1.type == TY_CCONST) && (e1.id != 0) && (e2.type == TY_COMP) && (e2.id == 0))
    {
        warn(MSG_WARN_IDXS_MESS, pctx.line_num+1);

        get_cmp_cst(e1, &etr, &eti);

        add_instr("SET_P __aux_var\n");
        add_instr("F2I\n");
        add_instr("P_F2I_M %s\n", v_table[etr.id].name);
        add_instr("MLT   %d\n", v_table[id].cols);
        add_instr("S_ADD\n");
    }

    // comp const in memory and comp in memory
    if ((e1.type == TY_CCONST) && (e1.id != 0) && (e2.type == TY_COMP) && (e2.id != 0))
    {
        warn(MSG_WARN_IDXS_MESS, pctx.line_num+1);

        get_cmp_cst(e1, &etr, &eti);

        add_instr("F2I_M   %s\n" , v_table[etr.id].name);
        add_instr("MLT     %d\n", v_table[id].cols);
        add_instr("P_F2I_M %s\n" , v_table[e2.id].name);
        add_instr("S_ADD\n");
    }

    // comp in acc and int in acc
    if ((e1.type == TY_COMP) && (e1.id == 0) && (e2.type == TY_INT) && (e2.id == 0))
    {
        warn(MSG_WARN_IDX_COMP_GRAB, pctx.line_num+1);

        add_instr("SET_P __aux_var\n");
        add_instr("POP\n");
        add_instr("F2I\n");
        add_instr("MLT   %d\n", v_table[id].cols);
        add_instr("ADD   __aux_var\n");
    }

    // comp in acc and int in memory
    if ((e1.type == TY_COMP) && (e1.id == 0) && (e2.type == TY_INT) && (e2.id != 0))
    {
        warn(MSG_WARN_IDX_COMP_GRAB, pctx.line_num+1);

        add_instr("POP\n");
        add_instr("F2I\n");
        add_instr("MLT %d\n", v_table[id].cols);
        add_instr("ADD %s\n",  v_table[e2.id].name);
    }

    // comp in acc and float in acc
    if ((e1.type == TY_COMP) && (e1.id == 0) && (e2.type == TY_FLOAT) && (e2.id == 0))
    {
        warn(MSG_WARN_IDXS_MESS, pctx.line_num+1);

        add_instr("F2I\n");
        add_instr("SET_P __aux_var\n");
        add_instr("POP\n");
        add_instr("F2I\n");
        add_instr("MLT   %d\n", v_table[id].cols);
        add_instr("ADD   __aux_var\n");
    }

    // comp in acc and float in memory
    if ((e1.type == TY_COMP) && (e1.id == 0) && (e2.type == TY_FLOAT) && (e2.id != 0))
    {
        warn(MSG_WARN_IDXS_MESS, pctx.line_num+1);

        add_instr("POP\n");
        add_instr("F2I\n");
        add_instr("MLT %d\n"   , v_table[id].cols);
        add_instr("P_F2I_M %s\n", v_table[e2.id].name);
        add_instr("S_ADD\n");
    }

    // comp in acc and comp const in memory
    if ((e1.type == TY_COMP) && (e1.id == 0) && (e2.type == TY_CCONST) && (e2.id != 0))
    {
        warn(MSG_WARN_IDXS_MESS, pctx.line_num+1);

        get_cmp_cst(e2, &etr, &eti);

        add_instr("POP\n");
        add_instr("F2I\n");
        add_instr("MLT     %d\n", v_table[id].cols);
        add_instr("P_F2I_M %s\n" , v_table[etr.id].name);
        add_instr("S_ADD\n");
    }

    // comp in acc and comp in acc
    if ((e1.type == TY_COMP) && (e1.id == 0) && (e2.type == TY_COMP) && (e2.id == 0))
    {
        warn(MSG_WARN_IDXS_MESS, pctx.line_num+1);

        add_instr("SET_P __aux_var\n");
        add_instr("F2I\n");
        add_instr("SET_P __aux_var\n");
        add_instr("POP\n");
        add_instr("F2I\n");
        add_instr("MLT   %d\n", v_table[id].cols);
        add_instr("ADD   __aux_var\n");
    }

    // comp in acc and comp in memory
    if ((e1.type == TY_COMP) && (e1.id == 0) && (e2.type == TY_COMP) && (e2.id != 0))
    {
        warn(MSG_WARN_IDXS_MESS, pctx.line_num+1);

        add_instr("POP\n");
        add_instr("F2I\n");
        add_instr("MLT     %d\n", v_table[id].cols);
        add_instr("P_F2I_M %s\n" , v_table[e2.id].name);
        add_instr("S_ADD\n");
    }

    // comp in memory and int in acc
    if ((e1.type == TY_COMP) && (e1.id != 0) && (e2.type == TY_INT) && (e2.id == 0))
    {
        warn(MSG_WARN_IDX_COMP_GRAB, pctx.line_num+1);

        add_instr("P_F2I_M %s\n",  v_table[e1.id].name);
        add_instr("MLT     %d\n", v_table[id].cols);
        add_instr("S_ADD\n");
    }

    // comp in memory and int in memory
    if ((e1.type == TY_COMP) && (e1.id != 0) && (e2.type == TY_INT) && (e2.id != 0))
    {
        warn(MSG_WARN_IDX_COMP_GRAB, pctx.line_num+1);

        add_instr("F2I_M %s\n",  v_table[e1.id].name);
        add_instr("MLT   %d\n", v_table[id].cols);
        add_instr("ADD   %s\n",  v_table[e2.id].name);
    }

    // comp in memory and float in acc
    if ((e1.type == TY_COMP) && (e1.id != 0) && (e2.type == TY_FLOAT) && (e2.id == 0))
    {
        warn(MSG_WARN_IDXS_MESS, pctx.line_num+1);

        add_instr("F2I\n");
        add_instr("P_F2I_M %s\n",  v_table[e1.id].name);
        add_instr("MLT     %d\n", v_table[id].cols);
        add_instr("S_ADD\n");
    }

    // comp in memory and float in memory
    if ((e1.type == TY_COMP) && (e1.id != 0) && (e2.type == TY_FLOAT) && (e2.id != 0))
    {
        warn(MSG_WARN_IDXS_MESS, pctx.line_num+1);

        add_instr("F2I_M   %s\n",  v_table[e1.id].name);
        add_instr("MLT     %d\n", v_table[id].cols);
        add_instr("P_F2I_M %s\n",  v_table[e2.id].name);
        add_instr("S_ADD\n");
    }

    // comp in memory and comp const in memory
    if ((e1.type == TY_COMP) && (e1.id != 0) && (e2.type == TY_CCONST) && (e2.id != 0))
    {
        warn(MSG_WARN_IDXS_MESS, pctx.line_num+1);

        get_cmp_cst(e2, &etr, &eti);

        add_instr("F2I_M   %s\n" , v_table[e1.id].name);
        add_instr("MLT     %d\n", v_table[id].cols);
        add_instr("P_F2I_M %s\n" , v_table[etr.id].name);
        add_instr("S_ADD\n");
    }

    // comp in memory and comp in acc
    if ((e1.type == TY_COMP) && (e1.id != 0) && (e2.type == TY_COMP) && (e2.id == 0))
    {
        warn(MSG_WARN_IDXS_MESS, pctx.line_num+1);

        add_instr("SET_P __aux_var\n");
        add_instr("F2I\n");
        add_instr("P_F2I_M %s\n", v_table[e1.id].name);
        add_instr("MLT   %d\n", v_table[id].cols);
        add_instr("S_ADD\n");
    }

    // comp in memory and comp in memory
    if ((e1.type == TY_COMP) && (e1.id != 0) && (e2.type == TY_COMP) && (e2.id != 0))
    {
        warn(MSG_WARN_IDXS_MESS, pctx.line_num+1);

        add_instr("F2I_M   %s\n" , v_table[e1.id].name);
        add_instr("MLT     %d\n", v_table[id].cols);
        add_instr("P_F2I_M %s\n" , v_table[e2.id].name);
        add_instr("S_ADD\n");
    }

    acc_live = 1; // acc carregado
}

// ----------------------------------------------------------------------------
// array in expressions, e.g. x = y[i]; ---------------------------------------
// ----------------------------------------------------------------------------

// 1D array read with a COMPILE-TIME-CONSTANT index, e.g. x = y[3];
// Reads arr[k] straight into the accumulator with the LOD_V pseudo-instruction
// (the assembler bakes the offset into a plain LOD at arr_base+k), so there is
// no index to materialise and no indirect LDI -- one fewer instruction than
// arr_1d2exp. P_LOD_V when the accumulator already holds a live value (this
// read is an operand of a larger expression), mirroring arr_1d2exp's LOD/P_LOD
// choice. A comp array loads the real half, then pushes it and loads the imag
// half (the comp-in-acc layout). The walker routes 1D-forward reads here, and
// 2D ones whose two indexes are constant (dims 2: k = row * columns + column,
// the matrix being stored row by row).
expr arr_1d2exp_const(int id, int k, int dims)
{
    // the same checks and use mark as arr_1d2exp
    if (v_table[id].type == TY_NONE)
        fatal(MSG_ERR_DECL_VAR_PROPERLY, pctx.line_num+1, strip_fn_prefix(v_table[id].name));
    if (v_table[id].dims == AR_NONE)
        fatal(MSG_ERR_NOT_ARRAY_HARSH, pctx.line_num+1, strip_fn_prefix(v_table[id].name));
    if (v_table[id].dims == AR_2D && dims == 1)
        fatal(MSG_ERR_ARRAY_2D  , pctx.line_num+1, strip_fn_prefix(v_table[id].name));
    v_table[id].used = 1;

    if (acc_live) add_instr("P_LOD_V %s %d\n", v_table[id].name, k);
    else        add_instr(  "LOD_V %s %d\n", v_table[id].name, k);
    if (v_table[id].type == TY_COMP) add_instr("P_LOD_V %s_i %d\n", v_table[id].name, k);
    acc_live = 1;  // value now in the accumulator
    return expr_make(v_table[id].type, 0);
}

// turns a 1D array into an exp
// the fft parameter tells whether to use the reversed index
expr arr_1d2exp(int id, expr e, int fft)
{
    // consistency checks -----------------------------------------------------

    // test whether the variable has been declared
    if (v_table[id].type == TY_NONE)
        fatal(MSG_ERR_DECL_VAR_PROPERLY, pctx.line_num+1, strip_fn_prefix(v_table[id].name));

    // must check that it really is an array
    if (v_table[id].dims == AR_NONE)
        fatal(MSG_ERR_NOT_ARRAY_HARSH, pctx.line_num+1, strip_fn_prefix(v_table[id].name));

    // must check that it really is a 1D array
    if (v_table[id].dims == AR_2D)
        fatal(MSG_ERR_ARRAY_2D  , pctx.line_num+1, strip_fn_prefix(v_table[id].name));

    // prepare the LOD commands ----------------------------------------------

    char ldv[10]; if (fft    == 1) strcpy(ldv,"ILI"    ); else strcpy(ldv,"LDI"  );
    char ldi[10]; if (acc_live == 1) strcpy(ldi,"P_LOD"  ); else strcpy(ldi,"LOD"  );

    // ------------------------------------------------------------------------
    // write the instructions -------------------------------------------------
    // ------------------------------------------------------------------------

    int  type = v_table[id].type;

    // left int/float ---------------------------------------------------------

    if (type < 3)
    {
        load_index(e, acc_live, MSG_WARN_IDX_FLOAT_HEAVY);
        add_instr("%s %s\n", ldv, v_table[id].name);
    }

    // left comp --------------------------------------------------------------

    // an int index in memory is loaded twice; any other goes through __aux_var
    else if ((e.type == TY_INT) && (e.id != 0))
    {
        add_instr("%s %s\n"    , ldi, v_table[e.id].name);
        add_instr("%s %s\n"    , ldv, v_table[id].name);
        add_instr("P_LOD %s\n" ,      v_table[e.id].name);
        add_instr("%s %s_i\n"  , ldv, v_table[id].name);
    }
    else
    {
        load_index(e, acc_live, MSG_WARN_IDX_FLOAT_HEAVY);
        add_instr("SET   __aux_var\n");
        add_instr("%s %s\n"  , ldv, v_table[id].name);
        add_instr("P_LOD __aux_var\n");
        add_instr("%s %s_i\n", ldv, v_table[id].name);
    }

    acc_live     = 1;
    v_table[id].used = 1;

    return expr_make(v_table[id].type, 0);
}

// transforma array 2D em exp
expr arr_2d2exp(int id, expr e1, expr e2)
{
    // consistency checks -----------------------------------------------------

    // test whether the variable has been declared
    if (v_table[id].type == TY_NONE)
        fatal(MSG_ERR_DECL_VAR_PROPERLY, pctx.line_num+1, strip_fn_prefix(v_table[id].name));

    // must check that it really is a 2D array
    if (v_table[id].dims == AR_NONE)
        fatal(MSG_ERR_NOT_ARRAY_HARSH, pctx.line_num+1, strip_fn_prefix(v_table[id].name));

    // must check that it is not a 1D array
    if (v_table[id].dims == AR_1D)
        fatal(MSG_ERR_ARRAY_1D , pctx.line_num+1, strip_fn_prefix(v_table[id].name));

    // prepare the LOD commands -----------------------------------------------

    char ldv[10];                  strcpy(ldv,"LDI"    ); // no 2D FFT yet
    char ldi[10]; if (acc_live == 1) strcpy(ldi,"P_LOD"  ); else strcpy(ldi,"LOD"  );
    char f2i[10]; if (acc_live == 1) strcpy(f2i,"P_F2I_M"); else strcpy(f2i,"F2I_M");

    // ------------------------------------------------------------------------
    // write the instructions -------------------------------------------------
    // ------------------------------------------------------------------------

    expr  etr, eti;
    int  type = v_table[id].type;

    // left int/float ---------------------------------------------------------

    // an int column index in the acc does not reach here: the walker reads the
    // element through one int expression, row * row length + column (ast.c)

    if (type < 3)
    {
        // int in acc and int in memory
        if ((e1.type == TY_INT) && (e1.id == 0) && (e2.type == TY_INT) && (e2.id != 0))
        {
            add_instr("MLT %d\n", v_table[id].cols);
            add_instr("ADD %s\n" , v_table[e2.id].name);
            add_instr( "%s %s\n" , ldv   , v_table[id].name);
        }

        // int in acc and float in acc
        if ((e1.type == TY_INT) && (e1.id == 0) && (e2.type == TY_FLOAT) && (e2.id == 0))
        {
            warn(MSG_WARN_IDX2_FLOAT, pctx.line_num+1);

            add_instr("F2I\n");
            add_instr("SET_P __aux_var\n");
            add_instr("MLT   %d\n", v_table[id].cols);
            add_instr("ADD   __aux_var\n");
            add_instr( "%s   %s\n", ldv    , v_table[id].name);
        }

        // int in acc and float in memory
        if ((e1.type == TY_INT) && (e1.id == 0) && (e2.type == TY_FLOAT) && (e2.id != 0))
        {
            warn(MSG_WARN_IDX2_FLOAT, pctx.line_num+1);

            add_instr("MLT     %d\n", v_table[id].cols);
            add_instr("P_F2I_M %s\n" , v_table[e2.id].name);
            add_instr("S_ADD\n");
            add_instr( "%s     %s\n", ldv    , v_table[id].name);
        }

        // int in acc and comp const in memory
        if ((e1.type == TY_INT) && (e1.id == 0) && (e2.type == TY_CCONST) && (e2.id != 0))
        {
            warn(MSG_WARN_IDX_COMP_GRAB, pctx.line_num+1);

            get_cmp_cst(e2, &etr, &eti);

            add_instr("MLT     %d\n", v_table[id].cols);
            add_instr("P_F2I_M %s\n" , v_table[etr.id].name);
            add_instr("S_ADD\n");
            add_instr( "%s     %s\n", ldv    , v_table[id].name);
        }

        // int in acc and comp in acc
        if ((e1.type == TY_INT) && (e1.id == 0) && (e2.type == TY_COMP) && (e2.id == 0))
        {
            warn(MSG_WARN_IDX_COMP_GRAB, pctx.line_num+1);

            add_instr("SET_P __aux_var\n");
            add_instr("F2I\n");
            add_instr("SET_P __aux_var\n");
            add_instr("MLT   %d\n", v_table[id].cols);
            add_instr("ADD   __aux_var\n");
            add_instr( "%s   %s\n", ldv    , v_table[id].name);
        }

        // int in acc and comp in memory
        if ((e1.type == TY_INT) && (e1.id == 0) && (e2.type == TY_COMP) && (e2.id != 0))
        {
            warn(MSG_WARN_IDX_COMP_GRAB, pctx.line_num+1);

            add_instr("MLT     %d\n", v_table[id].cols);
            add_instr("P_F2I_M %s\n" , v_table[e2.id].name);
            add_instr("S_ADD\n");
            add_instr( "%s     %s\n", ldv    , v_table[id].name);
        }

        // int in memory and int in memory
        if ((e1.type == TY_INT) && (e1.id != 0) && (e2.type == TY_INT) && (e2.id != 0))
        {
            add_instr( "%s %s\n", ldi, v_table[e1.id].name);
            add_instr("MLT %d\n"    , v_table[id].cols);
            add_instr("ADD %s\n"     , v_table[e2.id].name);
            add_instr( "%s %s\n", ldv        , v_table[id].name);
        }

        // int in memory and float in acc
        if ((e1.type == TY_INT) && (e1.id != 0) && (e2.type == TY_FLOAT) && (e2.id == 0))
        {
            warn(MSG_WARN_IDX2_FLOAT, pctx.line_num+1);

            add_instr("F2I\n");
            add_instr("P_LOD %s\n" , v_table[e1.id].name);
            add_instr("MLT   %d\n", v_table[id].cols);
            add_instr("S_ADD\n");
            add_instr( "%s   %s\n", ldv    , v_table[id].name);
        }

        // int in memory and float in memory
        if ((e1.type == TY_INT) && (e1.id != 0) && (e2.type == TY_FLOAT) && (e2.id != 0))
        {
            warn(MSG_WARN_IDX2_FLOAT, pctx.line_num+1);

            add_instr( "%s     %s\n", ldi, v_table[e1.id].name);
            add_instr("MLT     %d\n"    , v_table[id].cols);
            add_instr("P_F2I_M %s\n"     , v_table[e2.id].name);
            add_instr("S_ADD\n");
            add_instr( "%s     %s\n", ldv        , v_table[id].name);
        }

        // int in memory and comp const in memory
        if ((e1.type == TY_INT) && (e1.id != 0) && (e2.type == TY_CCONST) && (e2.id != 0))
        {
            warn(MSG_WARN_IDX_COMP_GRAB, pctx.line_num+1);

            get_cmp_cst(e2, &etr, &eti);

            add_instr( "%s     %s\n", ldi, v_table[e1.id].name);
            add_instr("MLT     %d\n"    , v_table[id].cols);
            add_instr("P_F2I_M %s\n"     , v_table[etr.id].name);
            add_instr("S_ADD\n");
            add_instr( "%s     %s\n", ldv        , v_table[id].name);
        }

        // int in memory and comp in acc
        if ((e1.type == TY_INT) && (e1.id != 0) && (e2.type == TY_COMP) && (e2.id == 0))
        {
            warn(MSG_WARN_IDX_COMP_GRAB, pctx.line_num+1);

            add_instr("SET_P __aux_var\n");
            add_instr("F2I\n");
            add_instr("P_LOD %s\n", v_table[e1.id].name);
            add_instr("MLT   %d\n", v_table[id].cols);
            add_instr("S_ADD\n");
            add_instr( "%s   %s\n", ldv    , v_table[id].name);
        }

        // int in memory and comp in memory
        if ((e1.type == TY_INT) && (e1.id != 0) && (e2.type == TY_COMP) && (e2.id != 0))
        {
            warn(MSG_WARN_IDX_COMP_GRAB, pctx.line_num+1);

            add_instr( "%s     %s\n", ldi, v_table[e1.id].name);
            add_instr("MLT     %d\n"    , v_table[id].cols);
            add_instr("P_F2I_M %s\n"     , v_table[e2.id].name);
            add_instr("S_ADD\n");
            add_instr( "%s     %s\n", ldv        , v_table[id].name);
        }

        // float in acc and int in acc
        if ((e1.type == TY_FLOAT) && (e1.id == 0) && (e2.type == TY_INT) && (e2.id == 0))
        {
            warn(MSG_WARN_IDX1_FLOAT, pctx.line_num+1);

            add_instr("SET_P __aux_var\n");
            add_instr("F2I\n");
            add_instr("MLT   %d\n", v_table[id].cols);
            add_instr("ADD   __aux_var\n");
            add_instr( "%s   %s\n", ldv    , v_table[id].name);
        }

        // float in acc and int in memory
        if ((e1.type == TY_FLOAT) && (e1.id == 0) && (e2.type == TY_INT) && (e2.id != 0))
        {
            warn(MSG_WARN_IDX1_FLOAT, pctx.line_num+1);

            add_instr("F2I\n");
            add_instr("MLT %d\n", v_table[id].cols);
            add_instr("ADD %s\n" , v_table[e2.id].name);
            add_instr( "%s %s\n" , ldv   , v_table[id].name);
        }

        // float in acc and float in acc
        if ((e1.type == TY_FLOAT) && (e1.id == 0) && (e2.type == TY_FLOAT) && (e2.id == 0))
        {
            warn(MSG_WARN_IDXS_FLOAT, pctx.line_num+1);

            add_instr("F2I\n");
            add_instr("SET_P __aux_var\n");
            add_instr("F2I\n");
            add_instr("MLT   %d\n", v_table[id].cols);
            add_instr("ADD   __aux_var\n");
            add_instr( "%s   %s\n", ldv    , v_table[id].name);
        }

        // float in acc and float in memory
        if ((e1.type == TY_FLOAT) && (e1.id == 0) && (e2.type == TY_FLOAT) && (e2.id != 0))
        {
            warn(MSG_WARN_IDXS_MESS, pctx.line_num+1);

            add_instr("F2I\n");
            add_instr("MLT     %d\n", v_table[id].cols);
            add_instr("P_F2I_M %s\n" , v_table[e2.id].name);
            add_instr("S_ADD\n");
            add_instr( "%s     %s\n" , ldv   , v_table[id].name);
        }

        // float in acc and comp const in memory
        if ((e1.type == TY_FLOAT) && (e1.id == 0) && (e2.type == TY_CCONST) && (e2.id != 0))
        {
            warn(MSG_WARN_IDXS_MESS, pctx.line_num+1);

            get_cmp_cst(e2, &etr, &eti);

            add_instr("F2I\n");
            add_instr("MLT %d\n"   , v_table[id].cols);
            add_instr("P_F2I_M %s\n", v_table[etr.id].name);
            add_instr("S_ADD\n");
            add_instr("%s %s\n", ldv        , v_table[id].name);
        }

        // float in acc and comp in acc
        if ((e1.type == TY_FLOAT) && (e1.id == 0) && (e2.type == TY_COMP) && (e2.id == 0))
        {
            warn(MSG_WARN_IDXS_MESS, pctx.line_num+1);

            add_instr("SET_P __aux_var\n");
            add_instr("F2I\n");
            add_instr("SET_P __aux_var\n");
            add_instr("F2I\n");
            add_instr("MLT   %d\n", v_table[id].cols);
            add_instr("ADD   __aux_var\n");
            add_instr( "%s   %s\n", ldv    , v_table[id].name);
        }

        // float in acc and comp in memory
        if ((e1.type == TY_FLOAT) && (e1.id == 0) && (e2.type == TY_COMP) && (e2.id != 0))
        {
            warn(MSG_WARN_IDXS_MESS, pctx.line_num+1);

            add_instr("F2I\n");
            add_instr("MLT     %d\n", v_table[id].cols);
            add_instr("P_F2I_M %s\n",  v_table[e2.id].name);
            add_instr("S_ADD\n");
            add_instr( "%s     %s\n", ldv,     v_table[id].name);
        }

        // float in memory and int in acc
        if ((e1.type == TY_FLOAT) && (e1.id != 0) && (e2.type == TY_INT) && (e2.id == 0))
        {
            warn(MSG_WARN_IDX1_FLOAT, pctx.line_num+1);

            add_instr("P_F2I_M %s\n", v_table[e1.id].name);
            add_instr("MLT   %d\n", v_table[id].cols);
            add_instr("S_ADD\n");
            add_instr( "%s   %s\n" , ldv   , v_table[id].name);
        }

        // float in memory and int in memory
        if ((e1.type == TY_FLOAT) && (e1.id != 0) && (e2.type == TY_INT) && (e2.id != 0))
        {
            warn(MSG_WARN_IDX1_FLOAT, pctx.line_num+1);

            add_instr( "%s %s\n", f2i, v_table[e1.id].name);
            add_instr("MLT %d\n"    , v_table[id].cols);
            add_instr("ADD %s\n",      v_table[e2.id].name);
            add_instr( "%s %s\n", ldv        , v_table[id].name);
        }

        // float in memory and float in acc
        if ((e1.type == TY_FLOAT) && (e1.id != 0) && (e2.type == TY_FLOAT) && (e2.id == 0))
        {
            warn(MSG_WARN_IDXS_FLOAT, pctx.line_num+1);

            add_instr("F2I\n");
            add_instr("P_F2I_M %s\n", v_table[e1.id].name);
            add_instr("MLT   %d\n", v_table[id].cols);
            add_instr("S_ADD\n");
            add_instr( "%s   %s\n", ldv    , v_table[id].name);
        }

        // float in memory and float in memory
        if ((e1.type == TY_FLOAT) && (e1.id != 0) && (e2.type == TY_FLOAT) && (e2.id != 0))
        {
            warn(MSG_WARN_IDXS_MESS, pctx.line_num+1);

            add_instr( "%s     %s\n", f2i, v_table[e1.id].name);
            add_instr("MLT     %d\n"    , v_table[id].cols);
            add_instr("P_F2I_M %s\n",      v_table[e2.id].name);
            add_instr("S_ADD\n");
            add_instr( "%s     %s\n", ldv        , v_table[id].name);
        }

        // float in memory and comp const in memory
        if ((e1.type == TY_FLOAT) && (e1.id != 0) && (e2.type == TY_CCONST) && (e2.id != 0))
        {
            warn(MSG_WARN_IDXS_MESS, pctx.line_num+1);

            get_cmp_cst(e2, &etr, &eti);

            add_instr( "%s     %s\n", f2i, v_table[e1.id].name);
            add_instr("MLT     %d\n"    , v_table[id].cols);
            add_instr("P_F2I_M %s\n",      v_table[etr.id].name);
            add_instr("S_ADD\n");
            add_instr( "%s     %s\n", ldv        , v_table[id].name);
        }

        // float in memory and comp in acc
        if ((e1.type == TY_FLOAT) && (e1.id != 0) && (e2.type == TY_COMP) && (e2.id == 0))
        {
            warn(MSG_WARN_IDXS_MESS, pctx.line_num+1);

            add_instr("SET_P __aux_var\n");
            add_instr("F2I\n");
            add_instr("SET   __aux_var\n");
            add_instr("F2I_M %s \n",   v_table[e1.id].name);
            add_instr("MLT   %d\n", v_table[id].cols);
            add_instr("ADD   __aux_var\n");
            add_instr( "%s   %s\n", ldv     , v_table[id].name);
        }

        // float in memory and comp in memory
        if ((e1.type == TY_FLOAT) && (e1.id != 0) && (e2.type == TY_COMP) && (e2.id != 0))
        {
            warn(MSG_WARN_IDXS_MESS, pctx.line_num+1);

            add_instr( "%s     %s\n", f2i, v_table[e1.id].name);
            add_instr("MLT     %d\n"    , v_table[id].cols);
            add_instr("P_F2I_M %s\n",      v_table[e2.id].name);
            add_instr("S_ADD\n");
            add_instr( "%s     %s\n", ldv        , v_table[id].name);
        }

        // comp const in memory and int in acc
        if ((e1.type == TY_CCONST) && (e1.id != 0) && (e2.type == TY_INT) && (e2.id == 0))
        {
            warn(MSG_WARN_IDX_COMP_GRAB, pctx.line_num+1);

            get_cmp_cst(e1, &etr, &eti);

            add_instr("P_F2I_M %s\n", v_table[etr.id].name);
            add_instr("MLT   %d\n", v_table[id].cols);
            add_instr("S_ADD\n");
            add_instr( "%s   %s\n", ldv    , v_table[id].name);
        }

        // comp const in memory and int in memory
        if ((e1.type == TY_CCONST) && (e1.id != 0) && (e2.type == TY_INT) && (e2.id != 0))
        {
            warn(MSG_WARN_IDX_COMP_GRAB, pctx.line_num+1);

            get_cmp_cst(e1, &etr, &eti);

            add_instr( "%s %s\n", f2i, v_table[etr.id].name);
            add_instr("MLT %d\n"    , v_table[id].cols);
            add_instr("ADD %s\n",      v_table[e2.id].name);
            add_instr( "%s %s\n", ldv        , v_table[id].name);
        }

        // comp const in memory and float in acc
        if ((e1.type == TY_CCONST) && (e1.id != 0) && (e2.type == TY_FLOAT) && (e2.id == 0))
        {
            warn(MSG_WARN_IDXS_MESS, pctx.line_num+1);

            get_cmp_cst(e1, &etr, &eti);

            add_instr("F2I\n");
            add_instr("P_F2I_M %s\n", v_table[etr.id].name);
            add_instr("MLT   %d\n", v_table[id].cols);
            add_instr("S_ADD\n");
            add_instr( "%s   %s\n", ldv    , v_table[id].name);
        }

        // comp const in memory and float in memory
        if ((e1.type == TY_CCONST) && (e1.id != 0) && (e2.type == TY_FLOAT) && (e2.id != 0))
        {
            warn(MSG_WARN_IDXS_MESS, pctx.line_num+1);

            get_cmp_cst(e1, &etr, &eti);

            add_instr( "%s     %s\n", f2i, v_table[etr.id].name);
            add_instr("MLT     %d\n"    , v_table[id].cols);
            add_instr("P_F2I_M %s\n",      v_table[e2.id].name);
            add_instr("S_ADD\n");
            add_instr( "%s     %s\n", ldv        , v_table[id].name);
        }

        // comp const in memory and comp const in memory
        if ((e1.type == TY_CCONST) && (e1.id != 0) && (e2.type == TY_CCONST) && (e2.id != 0))
        {
            warn(MSG_WARN_IDXS_MESS, pctx.line_num+1);

            get_cmp_cst(e1, &etr, &eti);
            add_instr("%s %s\n", f2i, v_table[etr.id].name);
            add_instr("MLT %d\n"   , v_table[id].cols);

            get_cmp_cst(e2, &etr, &eti);
            add_instr("P_F2I_M %s\n", v_table[etr.id].name);
            add_instr("S_ADD\n");
            add_instr( "%s     %s\n", ldv   , v_table[id].name);
        }

        // comp const in memory and comp in acc
        if ((e1.type == TY_CCONST) && (e1.id != 0) && (e2.type == TY_COMP) && (e2.id == 0))
        {
            warn(MSG_WARN_IDXS_MESS, pctx.line_num+1);

            get_cmp_cst(e1, &etr, &eti);

            add_instr("SET_P __aux_var\n");
            add_instr("F2I\n");
            add_instr("P_F2I_M %s\n", v_table[etr.id].name);
            add_instr("MLT   %d\n", v_table[id].cols);
            add_instr("S_ADD\n");
            add_instr( "%s   %s\n", ldv    , v_table[id].name);
        }

        // comp const in memory and comp in memory
        if ((e1.type == TY_CCONST) && (e1.id != 0) && (e2.type == TY_COMP) && (e2.id != 0))
        {
            warn(MSG_WARN_IDXS_MESS, pctx.line_num+1);

            get_cmp_cst(e1, &etr, &eti);

            add_instr( "%s     %s\n", f2i, v_table[etr.id].name);
            add_instr("MLT     %d\n"    , v_table[id].cols);
            add_instr("P_F2I_M %s\n",      v_table[e2.id].name);
            add_instr("S_ADD\n");
            add_instr( "%s     %s\n", ldv        , v_table[id].name);
        }

        // comp in acc and int in acc
        if ((e1.type == TY_COMP) && (e1.id == 0) && (e2.type == TY_INT) && (e2.id == 0))
        {
            warn(MSG_WARN_IDX_COMP_GRAB, pctx.line_num+1);

            add_instr("SET_P __aux_var\n");
            add_instr("POP\n");
            add_instr("F2I\n");
            add_instr("MLT   %d\n", v_table[id].cols);
            add_instr("ADD   __aux_var\n");
            add_instr( "%s   %s\n", ldv    , v_table[id].name);
        }

        // comp in acc and int in memory
        if ((e1.type == TY_COMP) && (e1.id == 0) && (e2.type == TY_INT) && (e2.id != 0))
        {
            warn(MSG_WARN_IDX_COMP_GRAB, pctx.line_num+1);

            add_instr("POP\n");
            add_instr("F2I\n");
            add_instr("MLT %d\n", v_table[id].cols);
            add_instr("ADD %s\n",  v_table[e2.id].name);
            add_instr( "%s %s\n",  ldv   , v_table[id].name);
        }

        // comp in acc and float in acc
        if ((e1.type == TY_COMP) && (e1.id == 0) && (e2.type == TY_FLOAT) && (e2.id == 0))
        {
            warn(MSG_WARN_IDXS_MESS, pctx.line_num+1);

            add_instr("F2I\n");
            add_instr("SET_P __aux_var\n");
            add_instr("POP\n");
            add_instr("F2I\n");
            add_instr("MLT  %d\n", v_table[id].cols);
            add_instr("ADD  __aux_var\n");
            add_instr( "%s  %s\n", ldv    , v_table[id].name);
        }

        // comp in acc and float in memory
        if ((e1.type == TY_COMP) && (e1.id == 0) && (e2.type == TY_FLOAT) && (e2.id != 0))
        {
            warn(MSG_WARN_IDXS_MESS, pctx.line_num+1);

            add_instr("POP\n");
            add_instr("F2I\n");
            add_instr("MLT     %d\n", v_table[id].cols);
            add_instr("P_F2I_M %s\n",  v_table[e2.id].name);
            add_instr("S_ADD\n");
            add_instr( "%s     %s\n",  ldv   , v_table[id].name);
        }

        // comp in acc and comp const in memory
        if ((e1.type == TY_COMP) && (e1.id == 0) && (e2.type == TY_CCONST) && (e2.id != 0))
        {
            warn(MSG_WARN_IDXS_MESS, pctx.line_num+1);

            get_cmp_cst(e2, &etr, &eti);

            add_instr("POP\n");
            add_instr("F2I\n");
            add_instr("MLT     %d\n", v_table[id].cols);
            add_instr("P_F2I_M %s\n",  v_table[etr.id].name);
            add_instr("S_ADD\n");
            add_instr( "%s     %s\n",  ldv   , v_table[id].name);
        }

        // comp in acc and comp in acc
        if ((e1.type == TY_COMP) && (e1.id == 0) && (e2.type == TY_COMP) && (e2.id == 0))
        {
            warn(MSG_WARN_IDXS_MESS, pctx.line_num+1);

            add_instr("POP\n");
            add_instr("F2I\n");
            add_instr("SET_P __aux_var\n");
            add_instr("POP\n");
            add_instr("F2I\n");
            add_instr("MLT   %d\n", v_table[id].cols);
            add_instr("ADD   __aux_var\n");
            add_instr( "%s   %s\n", ldv    , v_table[id].name);
        }

        // comp in acc and comp in memory
        if ((e1.type == TY_COMP) && (e1.id == 0) && (e2.type == TY_COMP) && (e2.id != 0))
        {
            warn(MSG_WARN_IDXS_MESS, pctx.line_num+1);

            add_instr("POP\n");
            add_instr("F2I\n");
            add_instr("MLT     %d\n", v_table[id].cols);
            add_instr("P_F2I_M %s\n",  v_table[e2.id].name);
            add_instr("S_ADD\n");
            add_instr( "%s     %s\n",  ldv   , v_table[id].name);
        }

        // comp in memory and int in acc
        if ((e1.type == TY_COMP) && (e1.id != 0) && (e2.type == TY_INT) && (e2.id == 0))
        {
            warn(MSG_WARN_IDX_COMP_GRAB, pctx.line_num+1);

            add_instr("P_F2I_M %s\n",  v_table[e1.id].name);
            add_instr("MLT     %d\n", v_table[id].cols);
            add_instr("S_ADD\n");
            add_instr( "%s     %s\n",  ldv   , v_table[id].name);
        }

        // comp in memory and int in memory
        if ((e1.type == TY_COMP) && (e1.id != 0) && (e2.type == TY_INT) && (e2.id != 0))
        {
            warn(MSG_WARN_IDX_COMP_GRAB, pctx.line_num+1);

            add_instr( "%s %s\n", f2i, v_table[e1.id].name);
            add_instr("MLT %d\n"    , v_table[id].cols);
            add_instr("ADD %s\n",      v_table[e2.id].name);
            add_instr( "%s %s\n", ldv        , v_table[id].name);
        }

        // comp in memory and float in acc
        if ((e1.type == TY_COMP) && (e1.id != 0) && (e2.type == TY_FLOAT) && (e2.id == 0))
        {
            warn(MSG_WARN_IDXS_MESS, pctx.line_num+1);

            add_instr("F2I\n");
            add_instr("P_F2I_M %s\n",  v_table[e1.id].name);
            add_instr("MLT     %d\n", v_table[id].cols);
            add_instr("S_ADD\n");
            add_instr( "%s     %s\n",  ldv   , v_table[id].name);
        }

        // comp in memory and float in memory
        if ((e1.type == TY_COMP) && (e1.id != 0) && (e2.type == TY_FLOAT) && (e2.id != 0))
        {
            warn(MSG_WARN_IDXS_MESS, pctx.line_num+1);

            add_instr("%s %s\n", f2i,  v_table[e1.id].name);
            add_instr("MLT     %d\n", v_table[id].cols);
            add_instr("P_F2I_M %s\n",  v_table[e2.id].name);
            add_instr("S_ADD\n");
            add_instr( "%s     %s\n",  ldv   , v_table[id].name);
        }

        // comp in memory and comp const in memory
        if ((e1.type == TY_COMP) && (e1.id != 0) && (e2.type == TY_CCONST) && (e2.id != 0))
        {
            warn(MSG_WARN_IDXS_MESS, pctx.line_num+1);

            get_cmp_cst(e2, &etr, &eti);

            add_instr( "%s     %s\n", f2i, v_table[e1.id].name);
            add_instr("MLT     %d\n"    , v_table[id].cols);
            add_instr("P_F2I_M %s\n",      v_table[etr.id].name);
            add_instr("S_ADD\n");
            add_instr( "%s     %s\n", ldv        , v_table[id].name);
        }

        // comp in memory and comp in acc
        if ((e1.type == TY_COMP) && (e1.id != 0) && (e2.type == TY_COMP) && (e2.id == 0))
        {
            warn(MSG_WARN_IDXS_MESS, pctx.line_num+1);

            add_instr("SET_P __aux_var\n");
            add_instr("F2I\n");
            add_instr("P_F2I_M %s\n", v_table[e1.id].name);
            add_instr("MLT   %d\n", v_table[id].cols);
            add_instr("S_ADD\n");
            add_instr( "%s   %s\n",  ldv   , v_table[id].name);
        }

        // comp in memory and comp in memory
        if ((e1.type == TY_COMP) && (e1.id != 0) && (e2.type == TY_COMP) && (e2.id != 0))
        {
            warn(MSG_WARN_IDXS_MESS, pctx.line_num+1);

            add_instr("%s      %s\n", f2i, v_table[e1.id].name);
            add_instr("MLT     %d\n"    , v_table[id].cols);
            add_instr("P_F2I_M %s\n",      v_table[e2.id].name);
            add_instr("S_ADD\n");
            add_instr ("%s     %s\n", ldv        , v_table[id].name);
        }
    }

    // left comp --------------------------------------------------------------

    else
    {
        // int in acc and int in memory
        if ((e1.type == TY_INT) && (e1.id == 0) && (e2.type == TY_INT) && (e2.id != 0))
        {
            add_instr("MLT   %d\n" , v_table[id].cols);
            add_instr("ADD   %s\n"  , v_table[e2.id].name);
            add_instr("SET   __aux_var\n");
            add_instr( "%s   %s\n"  , ldv   , v_table[id].name);
            add_instr("P_LOD __aux_var\n");
            add_instr( "%s   %s_i\n", ldv   , v_table[id].name);
        }

        // int in acc and float in acc
        if ((e1.type == TY_INT) && (e1.id == 0) && (e2.type == TY_FLOAT) && (e2.id == 0))
        {
            warn(MSG_WARN_IDX2_FLOAT, pctx.line_num+1);

            add_instr("F2I\n");
            add_instr("SET_P __aux_var\n");
            add_instr("MLT   %d\n", v_table[id].cols);
            add_instr("ADD   __aux_var\n");
            add_instr("SET   __aux_var\n");
            add_instr( "%s   %s\n"  , ldv  , v_table[id].name);
            add_instr("P_LOD __aux_var\n");
            add_instr( "%s   %s_i\n", ldv  , v_table[id].name);
        }

        // int in acc and float in memory
        if ((e1.type == TY_INT) && (e1.id == 0) && (e2.type == TY_FLOAT) && (e2.id != 0))
        {
            warn(MSG_WARN_IDX2_FLOAT, pctx.line_num+1);

            add_instr("MLT     %d\n", v_table[id].cols);
            add_instr("P_F2I_M %s\n" , v_table[e2.id].name);
            add_instr("S_ADD\n");
            add_instr("SET     __aux_var\n");
            add_instr( "%s     %s\n"  , ldv  , v_table[id].name);
            add_instr("P_LOD   __aux_var\n");
            add_instr( "%s     %s_i\n", ldv  , v_table[id].name);
        }

        // int in acc and comp const in memory
        if ((e1.type == TY_INT) && (e1.id == 0) && (e2.type == TY_CCONST) && (e2.id != 0))
        {
            warn(MSG_WARN_IDX_COMP_GRAB, pctx.line_num+1);

            get_cmp_cst(e2, &etr, &eti);

            add_instr("MLT     %d\n", v_table[id].cols);
            add_instr("P_F2I_M %s\n" , v_table[etr.id].name);
            add_instr("S_ADD\n");
            add_instr("SET     __aux_var\n");
            add_instr( "%s     %s\n"  , ldv  , v_table[id].name);
            add_instr("P_LOD   __aux_var\n");
            add_instr( "%s     %s_i\n", ldv  , v_table[id].name);
        }

        // int in acc and comp in acc
        if ((e1.type == TY_INT) && (e1.id == 0) && (e2.type == TY_COMP) && (e2.id == 0))
        {
            warn(MSG_WARN_IDX_COMP_GRAB, pctx.line_num+1);

            add_instr("SET_P __aux_var\n");
            add_instr("F2I\n");
            add_instr("SET_P __aux_var\n");
            add_instr("MLT   %d\n", v_table[id].cols);
            add_instr("ADD   __aux_var\n");
            add_instr("SET   __aux_var\n");
            add_instr( "%s   %s\n"  , ldv  , v_table[id].name);
            add_instr("P_LOD __aux_var\n");
            add_instr( "%s   %s_i\n", ldv  , v_table[id].name);
        }

        // int in acc and comp in memory
        if ((e1.type == TY_INT) && (e1.id == 0) && (e2.type == TY_COMP) && (e2.id != 0))
        {
            warn(MSG_WARN_IDX_COMP_GRAB, pctx.line_num+1);

            add_instr("MLT     %d\n", v_table[id].cols);
            add_instr("P_F2I_M %s\n",  v_table[e2.id].name);
            add_instr("S_ADD\n");
            add_instr("SET     __aux_var\n");
            add_instr( "%s     %s\n"  , ldv  , v_table[id].name);
            add_instr("P_LOD   __aux_var\n");
            add_instr( "%s     %s_i\n", ldv  , v_table[id].name);
        }

        // int in memory and int in memory
        if ((e1.type == TY_INT) && (e1.id != 0) && (e2.type == TY_INT) && (e2.id != 0))
        {
            add_instr( "%s   %s\n", ldi, v_table[e1.id].name);
            add_instr("MLT   %d\n"    , v_table[id].cols);
            add_instr("ADD   %s\n",      v_table[e2.id].name);
            add_instr("SET   __aux_var\n");
            add_instr( "%s   %s\n"  , ldv      , v_table[id].name);
            add_instr("P_LOD __aux_var\n");
            add_instr( "%s   %s_i\n", ldv      , v_table[id].name);
        }

        // int in memory and float in acc
        if ((e1.type == TY_INT) && (e1.id != 0) && (e2.type == TY_FLOAT) && (e2.id == 0))
        {
            warn(MSG_WARN_IDX2_FLOAT, pctx.line_num+1);

            add_instr("F2I\n");
            add_instr("P_LOD %s\n",  v_table[e1.id].name);
            add_instr("MLT   %d\n", v_table[id].cols);
            add_instr("S_ADD\n");
            add_instr("SET   __aux_var\n");
            add_instr( "%s   %s\n"  , ldv  , v_table[id].name);
            add_instr("P_LOD __aux_var\n");
            add_instr( "%s   %s_i\n", ldv  , v_table[id].name);
        }

        // int in memory and float in memory
        if ((e1.type == TY_INT) && (e1.id != 0) && (e2.type == TY_FLOAT) && (e2.id != 0))
        {
            warn(MSG_WARN_IDX2_FLOAT, pctx.line_num+1);

            add_instr( "%s     %s\n", ldi, v_table[e1.id].name);
            add_instr("MLT     %d\n"    , v_table[id].cols);
            add_instr("P_F2I_M %s\n"     , v_table[e2.id].name);
            add_instr("S_ADD\n");
            add_instr("SET     __aux_var\n");
            add_instr( "%s     %s\n"  , ldv      , v_table[id].name);
            add_instr("P_LOD   __aux_var\n");
            add_instr( "%s     %s_i\n", ldv      , v_table[id].name);
        }

        // int in memory and comp const in memory
        if ((e1.type == TY_INT) && (e1.id != 0) && (e2.type == TY_CCONST) && (e2.id != 0))
        {
            warn(MSG_WARN_IDX_COMP_GRAB, pctx.line_num+1);

            get_cmp_cst(e2, &etr, &eti);

            add_instr( "%s     %s\n", ldi, v_table[e1.id].name);
            add_instr("MLT     %d\n"    , v_table[id].cols);
            add_instr("P_F2I_M %s\n"     , v_table[etr.id].name);
            add_instr("S_ADD\n");
            add_instr("SET     __aux_var\n");
            add_instr( "%s     %s\n"  , ldv      , v_table[id].name);
            add_instr("P_LOD   __aux_var\n");
            add_instr( "%s     %s_i\n", ldv      , v_table[id].name);
        }

        // int in memory and comp in acc
        if ((e1.type == TY_INT) && (e1.id != 0) && (e2.type == TY_COMP) && (e2.id == 0))
        {
            warn(MSG_WARN_IDX_COMP_GRAB, pctx.line_num+1);

            add_instr("SET_P __aux_var\n");
            add_instr("F2I\n");
            add_instr("P_LOD %s\n", v_table[e1.id].name);
            add_instr("MLT   %d\n" , v_table[id].cols);
            add_instr("S_ADD\n");
            add_instr("SET   __aux_var\n");
            add_instr( "%s   %s\n"  , ldv   , v_table[id].name);
            add_instr("P_LOD __aux_var\n");
            add_instr( "%s   %s_i\n", ldv   , v_table[id].name);
        }

        // int in memory and comp in memory
        if ((e1.type == TY_INT) && (e1.id != 0) && (e2.type == TY_COMP) && (e2.id != 0))
        {
            warn(MSG_WARN_IDX_COMP_GRAB, pctx.line_num+1);

            add_instr( "%s     %s\n", ldi, v_table[e1.id].name);
            add_instr("MLT     %d\n"    , v_table[id].cols);
            add_instr("P_F2I_M %s\n"     , v_table[e2.id].name);
            add_instr("S_ADD\n");
            add_instr("SET     __aux_var\n");
            add_instr( "%s     %s\n"  , ldv      , v_table[id].name);
            add_instr("P_LOD   __aux_var\n");
            add_instr( "%s     %s_i\n", ldv      , v_table[id].name);
        }

        // float in acc and int in acc
        if ((e1.type == TY_FLOAT) && (e1.id == 0) && (e2.type == TY_INT) && (e2.id == 0))
        {
            warn(MSG_WARN_IDX1_FLOAT, pctx.line_num+1);

            add_instr("SET_P __aux_var\n");
            add_instr("F2I\n");
            add_instr("MLT   %d\n", v_table[id].cols);
            add_instr("ADD   __aux_var\n");
            add_instr("SET   __aux_var\n");
            add_instr( "%s   %s\n"  , ldv  , v_table[id].name);
            add_instr("P_LOD __aux_var\n");
            add_instr( "%s   %s_i\n", ldv  , v_table[id].name);
        }

        // float in acc and int in memory
        if ((e1.type == TY_FLOAT) && (e1.id == 0) && (e2.type == TY_INT) && (e2.id != 0))
        {
            warn(MSG_WARN_IDX1_FLOAT, pctx.line_num+1);

            add_instr("F2I\n");
            add_instr("MLT   %d\n", v_table[id].cols);
            add_instr("ADD   %s\n" , v_table[e2.id].name);
            add_instr("SET   __aux_var\n");
            add_instr( "%s   %s\n"  , ldv  , v_table[id].name);
            add_instr("P_LOD __aux_var\n");
            add_instr( "%s   %s_i\n", ldv  , v_table[id].name);
        }

        // float in acc and float in acc
        if ((e1.type == TY_FLOAT) && (e1.id == 0) && (e2.type == TY_FLOAT) && (e2.id == 0))
        {
            warn(MSG_WARN_IDXS_FLOAT, pctx.line_num+1);

            add_instr("F2I\n");
            add_instr("SET_P __aux_var\n");
            add_instr("F2I\n");
            add_instr("MLT   %d\n", v_table[id].cols);
            add_instr("ADD   __aux_var\n");
            add_instr("SET   __aux_var\n");
            add_instr( "%s   %s\n"  , ldv  , v_table[id].name);
            add_instr("P_LOD __aux_var\n");
            add_instr( "%s   %s_i\n", ldv  , v_table[id].name);
        }

        // float in acc and float in memory
        if ((e1.type == TY_FLOAT) && (e1.id == 0) && (e2.type == TY_FLOAT) && (e2.id != 0))
        {
            warn(MSG_WARN_IDXS_MESS, pctx.line_num+1);

            add_instr("F2I\n");
            add_instr("MLT     %d\n", v_table[id].cols);
            add_instr("P_F2I_M %s\n" , v_table[e2.id].name);
            add_instr("S_ADD\n");
            add_instr("SET     __aux_var\n");
            add_instr( "%s     %s\n"  , ldv  , v_table[id].name);
            add_instr("P_LOD   __aux_var\n");
            add_instr( "%s     %s_i\n", ldv  , v_table[id].name);
        }

        // float in acc and comp const in memory
        if ((e1.type == TY_FLOAT) && (e1.id == 0) && (e2.type == TY_CCONST) && (e2.id != 0))
        {
            warn(MSG_WARN_IDXS_MESS, pctx.line_num+1);

            get_cmp_cst(e2, &etr, &eti);

            add_instr("F2I\n");
            add_instr("MLT     %d\n", v_table[id].cols);
            add_instr("P_F2I_M %s\n" , v_table[etr.id].name);
            add_instr("S_ADD\n");
            add_instr("SET     __aux_var\n");
            add_instr( "%s     %s\n"  , ldv  , v_table[id].name);
            add_instr("P_LOD   __aux_var\n");
            add_instr( "%s     %s_i\n", ldv  , v_table[id].name);
        }

        // float in acc and comp in acc
        if ((e1.type == TY_FLOAT) && (e1.id == 0) && (e2.type == TY_COMP) && (e2.id == 0))
        {
            warn(MSG_WARN_IDXS_MESS, pctx.line_num+1);

            add_instr("SET_P __aux_var\n");
            add_instr("F2I\n");
            add_instr("SET_P __aux_var\n");
            add_instr("F2I\n");
            add_instr("MLT   %d\n", v_table[id].cols);
            add_instr("ADD   __aux_var\n");
            add_instr("SET   __aux_var\n");
            add_instr( "%s   %s\n"  , ldv  , v_table[id].name);
            add_instr("P_LOD __aux_var\n");
            add_instr( "%s   %s_i\n", ldv  , v_table[id].name);
        }

        // float in acc and comp in memory
        if ((e1.type == TY_FLOAT) && (e1.id == 0) && (e2.type == TY_COMP) && (e2.id != 0))
        {
            warn(MSG_WARN_IDXS_MESS, pctx.line_num+1);

            add_instr("F2I\n");
            add_instr("MLT     %d\n", v_table[id].cols);
            add_instr("P_F2I_M %s\n" , v_table[e2.id].name);
            add_instr("S_ADD\n");
            add_instr("SET     __aux_var\n");
            add_instr( "%s     %s\n"  , ldv  , v_table[id].name);
            add_instr("P_LOD   __aux_var\n");
            add_instr( "%s     %s_i\n", ldv  , v_table[id].name);
        }

        // float in memory and int in acc
        if ((e1.type == TY_FLOAT) && (e1.id != 0) && (e2.type == TY_INT) && (e2.id == 0))
        {
            warn(MSG_WARN_IDX1_FLOAT, pctx.line_num+1);

            add_instr("P_F2I_M %s\n", v_table[e1.id].name);
            add_instr("MLT   %d\n", v_table[id].cols);
            add_instr("S_ADD\n");
            add_instr("SET   __aux_var\n");
            add_instr( "%s   %s\n"  , ldv  , v_table[id].name);
            add_instr("P_LOD __aux_var\n");
            add_instr( "%s   %s_i\n", ldv  , v_table[id].name);
        }

        // float in memory and int in memory
        if ((e1.type == TY_FLOAT) && (e1.id != 0) && (e2.type == TY_INT) && (e2.id != 0))
        {
            warn(MSG_WARN_IDX_COMP_GRAB, pctx.line_num+1);

            add_instr("%s    %s\n", f2i, v_table[e1.id].name);
            add_instr("MLT   %d\n"    , v_table[id].cols);
            add_instr("ADD   %s\n"     , v_table[e2.id].name);
            add_instr("SET   __aux_var\n");
            add_instr( "%s   %s\n"  , ldv      , v_table[id].name);
            add_instr("P_LOD __aux_var\n");
            add_instr( "%s   %s_i\n", ldv      , v_table[id].name);
        }

        // float in memory and float in acc
        if ((e1.type == TY_FLOAT) && (e1.id != 0) && (e2.type == TY_FLOAT) && (e2.id == 0))
        {
            warn(MSG_WARN_IDXS_MESS, pctx.line_num+1);

            add_instr("F2I\n");
            add_instr("P_F2I_M %s\n", v_table[e1.id].name);
            add_instr("MLT   %d\n", v_table[id].cols);
            add_instr("S_ADD\n");
            add_instr("SET   __aux_var\n");
            add_instr( "%s   %s\n"  , ldv  , v_table[id].name);
            add_instr("P_LOD __aux_var\n");
            add_instr( "%s   %s_i\n", ldv  , v_table[id].name);
        }

        // float in memory and float in memory
        if ((e1.type == TY_FLOAT) && (e1.id != 0) && (e2.type == TY_FLOAT) && (e2.id != 0))
        {
            warn(MSG_WARN_IDXS_MESS, pctx.line_num+1);

            add_instr( "%s     %s\n", f2i, v_table[e1.id].name);
            add_instr("MLT     %d\n"    , v_table[id].cols);
            add_instr("P_F2I_M %s\n"     , v_table[e2.id].name);
            add_instr("S_ADD\n");
            add_instr("SET     __aux_var\n");
            add_instr("%s      %s\n"  , ldv      , v_table[id].name);
            add_instr("P_LOD   __aux_var\n");
            add_instr( "%s     %s_i\n", ldv      , v_table[id].name);
        }

        // float in memory and comp const in memory
        if ((e1.type == TY_FLOAT) && (e1.id != 0) && (e2.type == TY_CCONST) && (e2.id != 0))
        {
            warn(MSG_WARN_IDXS_MESS, pctx.line_num+1);

            add_instr( "%s %s\n", f2i, v_table[e1.id].name);
            add_instr("MLT %d\n"    , v_table[id].cols);

            get_cmp_cst(e2, &etr, &eti);

            add_instr("P_F2I_M %s\n", v_table[etr.id].name);
            add_instr("S_ADD\n");
            add_instr("SET     __aux_var\n");
            add_instr( "%s     %s\n"  , ldv , v_table[id].name);
            add_instr("P_LOD   __aux_var\n");
            add_instr( "%s     %s_i\n", ldv , v_table[id].name);
        }

        // float in memory and comp in acc
        if ((e1.type == TY_FLOAT) && (e1.id != 0) && (e2.type == TY_COMP) && (e2.id == 0))
        {
            warn(MSG_WARN_IDXS_MESS, pctx.line_num+1);

            add_instr("SET_P __aux_var\n");
            add_instr("F2I\n");
            add_instr("P_F2I_M %s\n", v_table[e1.id].name);
            add_instr("MLT   %d\n", v_table[id].cols);
            add_instr("S_ADD\n");
            add_instr("SET   __aux_var\n");
            add_instr( "%s   %s\n"  , ldv  , v_table[id].name);
            add_instr("P_LOD __aux_var\n");
            add_instr( "%s   %s_i\n", ldv  , v_table[id].name);
        }

        // float in memory and comp in memory
        if ((e1.type == TY_FLOAT) && (e1.id != 0) && (e2.type == TY_COMP) && (e2.id != 0))
        {
            warn(MSG_WARN_IDXS_MESS, pctx.line_num+1);

            add_instr( "%s     %s\n", f2i, v_table[e1.id].name);
            add_instr("MLT     %d\n"    , v_table[id].cols);
            add_instr("P_F2I_M %s\n"     , v_table[e2.id].name);
            add_instr("S_ADD\n");
            add_instr("SET     __aux_var\n");
            add_instr( "%s     %s\n"  , ldv      , v_table[id].name);
            add_instr("P_LOD   __aux_var\n");
            add_instr( "%s     %s_i\n", ldv      , v_table[id].name);
        }

        // comp const in memory and int in acc
        if ((e1.type == TY_CCONST) && (e1.id != 0) && (e2.type == TY_INT) && (e2.id == 0))
        {
            warn(MSG_WARN_IDX_COMP_GRAB, pctx.line_num+1);

            get_cmp_cst(e1, &etr, &eti);

            add_instr("P_F2I_M %s\n", v_table[etr.id].name);
            add_instr("MLT   %d\n", v_table[id].cols);
            add_instr("S_ADD\n");
            add_instr("SET   __aux_var\n");
            add_instr( "%s   %s\n"  , ldv  , v_table[id].name);
            add_instr("P_LOD __aux_var\n");
            add_instr( "%s   %s_i\n", ldv  , v_table[id].name);
        }

        // comp const in memory and int in memory
        if ((e1.type == TY_CCONST) && (e1.id != 0) && (e2.type == TY_INT) && (e2.id != 0))
        {
            warn(MSG_WARN_IDX_COMP_GRAB, pctx.line_num+1);

            get_cmp_cst(e1, &etr, &eti);

            add_instr( "%s   %s\n", f2i, v_table[etr.id].name);
            add_instr("MLT   %d\n"    , v_table[id].cols);
            add_instr("ADD   %s\n",      v_table[e2.id].name);
            add_instr("SET   __aux_var\n");
            add_instr( "%s   %s\n"  , ldv      , v_table[id].name);
            add_instr("P_LOD __aux_var\n");
            add_instr( "%s   %s_i\n", ldv      , v_table[id].name);
        }

        // comp const in memory and float in acc
        if ((e1.type == TY_CCONST) && (e1.id != 0) && (e2.type == TY_FLOAT) && (e2.id == 0))
        {
            warn(MSG_WARN_IDXS_MESS, pctx.line_num+1);

            get_cmp_cst(e1, &etr, &eti);

            add_instr("F2I\n");
            add_instr("P_F2I_M %s\n", v_table[etr.id].name);
            add_instr("MLT   %d\n", v_table[id].cols);
            add_instr("S_ADD\n");
            add_instr("SET   __aux_var\n");
            add_instr( "%s   %s\n"  , ldv  , v_table[id].name);
            add_instr("P_LOD __aux_var\n");
            add_instr( "%s   %s_i\n", ldv  , v_table[id].name);
        }

        // comp const in memory and float in memory
        if ((e1.type == TY_CCONST) && (e1.id != 0) && (e2.type == TY_FLOAT) && (e2.id != 0))
        {
            warn(MSG_WARN_IDXS_MESS, pctx.line_num+1);

            get_cmp_cst(e1, &etr, &eti);

            add_instr( "%s     %s\n", f2i, v_table[etr.id].name);
            add_instr("MLT     %d\n"    , v_table[id].cols);
            add_instr("P_F2I_M %s\n",      v_table[e2.id].name);
            add_instr("S_ADD\n");
            add_instr("SET     __aux_var\n");
            add_instr( "%s     %s\n"  , ldv      , v_table[id].name);
            add_instr("P_LOD   __aux_var\n");
            add_instr( "%s     %s_i\n", ldv      , v_table[id].name);
        }

        // comp const in memory and comp const in memory
        if ((e1.type == TY_CCONST) && (e1.id != 0) && (e2.type == TY_CCONST) && (e2.id != 0))
        {
            warn(MSG_WARN_IDXS_MESS, pctx.line_num+1);

            get_cmp_cst(e1, &etr, &eti);
            add_instr( "%s %s\n", f2i, v_table[etr.id].name);
            add_instr("MLT %d\n"    , v_table[id].cols);

            get_cmp_cst(e2, &etr, &eti);
            add_instr("P_F2I_M %s\n", v_table[etr.id].name);
            add_instr("S_ADD\n");
            add_instr("SET     __aux_var\n");
            add_instr( "%s     %s\n"  , ldv , v_table[id].name);
            add_instr("P_LOD   __aux_var\n");
            add_instr( "%s     %s_i\n", ldv , v_table[id].name);
        }

        // comp const in memory and comp in acc
        if ((e1.type == TY_CCONST) && (e1.id != 0) && (e2.type == TY_COMP) && (e2.id == 0))
        {
            warn(MSG_WARN_IDXS_MESS, pctx.line_num+1);

            get_cmp_cst(e1, &etr, &eti);

            add_instr("SET_P __aux_var\n");
            add_instr("F2I\n");
            add_instr("P_F2I_M %s\n", v_table[etr.id].name);
            add_instr("MLT   %d\n", v_table[id].cols);
            add_instr("S_ADD\n");
            add_instr("SET   __aux_var\n");
            add_instr( "%s   %s\n"  , ldv  , v_table[id].name);
            add_instr("P_LOD __aux_var\n");
            add_instr( "%s   %s_i\n", ldv  , v_table[id].name);
        }

        // comp const in memory and comp in memory
        if ((e1.type == TY_CCONST) && (e1.id != 0) && (e2.type == TY_COMP) && (e2.id != 0))
        {
            warn(MSG_WARN_IDXS_MESS, pctx.line_num+1);

            get_cmp_cst(e1, &etr, &eti);

            add_instr("%s      %s\n", f2i, v_table[etr.id].name);
            add_instr("MLT     %d\n"    , v_table[id].cols);
            add_instr("P_F2I_M %s\n"     , v_table[e2.id].name);
            add_instr("S_ADD\n");
            add_instr("SET     __aux_var\n");
            add_instr( "%s     %s\n"  , ldv      , v_table[id].name);
            add_instr("P_LOD   __aux_var\n");
            add_instr( "%s     %s_i\n", ldv      , v_table[id].name);
        }

        // comp in acc and int in acc
        if ((e1.type == TY_COMP) && (e1.id == 0) && (e2.type == TY_INT) && (e2.id == 0))
        {
            warn(MSG_WARN_IDX_COMP_GRAB, pctx.line_num+1);

            add_instr("SET_P __aux_var\n");
            add_instr("POP\n");
            add_instr("F2I\n");
            add_instr("MLT   %d\n", v_table[id].cols);
            add_instr("ADD   __aux_var\n");
            add_instr("SET   __aux_var\n");
            add_instr( "%s   %s\n"  , ldv  , v_table[id].name);
            add_instr("P_LOD __aux_var\n");
            add_instr( "%s   %s_i\n", ldv  , v_table[id].name);
        }

        // comp in acc and int in memory
        if ((e1.type == TY_COMP) && (e1.id == 0) && (e2.type == TY_INT) && (e2.id != 0))
        {
            warn(MSG_WARN_IDX_COMP_GRAB, pctx.line_num+1);

            add_instr("SET_P __aux_var\n");
            add_instr("F2I\n");
            add_instr("MLT   %d\n", v_table[id].cols);
            add_instr("ADD   %s\n" , v_table[e2.id].name);
            add_instr("SET   __aux_var\n");
            add_instr( "%s   %s\n"  , ldv  , v_table[id].name);
            add_instr("P_LOD __aux_var\n");
            add_instr( "%s   %s_i\n", ldv  , v_table[id].name);
        }

        // comp in acc and float in acc
        if ((e1.type == TY_COMP) && (e1.id == 0) && (e2.type == TY_FLOAT) && (e2.id == 0))
        {
            warn(MSG_WARN_IDXS_MESS, pctx.line_num+1);

            add_instr("F2I\n");
            add_instr("SET_P __aux_var\n");
            add_instr("POP\n");
            add_instr("F2I\n");
            add_instr("MLT   %d\n", v_table[id].cols);
            add_instr("ADD   __aux_var\n");
            add_instr("SET   __aux_var\n");
            add_instr("%s    %s\n"  , ldv  , v_table[id].name);
            add_instr("P_LOD __aux_var\n");
            add_instr("%s    %s_i\n", ldv  , v_table[id].name);
        }

        // comp in acc and float in memory
        if ((e1.type == TY_COMP) && (e1.id == 0) && (e2.type == TY_FLOAT) && (e2.id != 0))
        {
            warn(MSG_WARN_IDXS_MESS, pctx.line_num+1);

            add_instr("SET_P   __aux_var\n");
            add_instr("F2I\n");
            add_instr("MLT     %d\n", v_table[id].cols);
            add_instr("P_F2I_M %s\n" , v_table[e2.id].name);
            add_instr("S_ADD\n");
            add_instr("SET     __aux_var\n");
            add_instr ("%s     %s\n"  , ldv  , v_table[id].name);
            add_instr("P_LOD   __aux_var\n");
            add_instr( "%s     %s_i\n", ldv  , v_table[id].name);
        }

        // comp in acc and comp const in memory
        if ((e1.type == TY_COMP) && (e1.id == 0) && (e2.type == TY_CCONST) && (e2.id != 0))
        {
            warn(MSG_WARN_IDXS_MESS, pctx.line_num+1);

            get_cmp_cst(e2, &etr, &eti);

            add_instr("SET_P   __aux_var\n");
            add_instr("F2I\n");
            add_instr("MLT     %d\n", v_table[id].cols);
            add_instr("P_F2I_M %s\n" , v_table[etr.id].name);
            add_instr("S_ADD\n");
            add_instr("SET     __aux_var\n");
            add_instr( "%s     %s\n"  , ldv  , v_table[id].name);
            add_instr("P_LOD   __aux_var\n");
            add_instr( "%s     %s_i\n", ldv  , v_table[id].name);
        }

        // comp in acc and comp in acc
        if ((e1.type == TY_COMP) && (e1.id == 0) && (e2.type == TY_COMP) && (e2.id == 0))
        {
            warn(MSG_WARN_IDXS_MESS, pctx.line_num+1);

            add_instr("SET_P __aux_var\n");
            add_instr("F2I\n");
            add_instr("SET_P __aux_var\n");
            add_instr("POP\n");
            add_instr("F2I\n");
            add_instr("MLT   %d\n", v_table[id].cols);
            add_instr("ADD   __aux_var\n");
            add_instr("SET   __aux_var\n");
            add_instr( "%s   %s\n"  , ldv  , v_table[id].name);
            add_instr("P_LOD __aux_var\n");
            add_instr( "%s   %s_i\n", ldv  , v_table[id].name);
        }

        // comp in acc and comp in memory
        if ((e1.type == TY_COMP) && (e1.id == 0) && (e2.type == TY_COMP) && (e2.id != 0))
        {
            warn(MSG_WARN_IDXS_MESS, pctx.line_num+1);

            add_instr("SET_P   __aux_var\n");
            add_instr("F2I\n");
            add_instr("MLT     %d\n", v_table[id].cols);
            add_instr("P_F2I_M %s\n" , v_table[e2.id].name);
            add_instr("S_ADD\n");
            add_instr("SET     __aux_var\n");
            add_instr( "%s     %s\n"  , ldv  , v_table[id].name);
            add_instr("P_LOD   __aux_var\n");
            add_instr( "%s     %s_i\n", ldv  , v_table[id].name);
        }

        // comp in memory and int in acc
        if ((e1.type == TY_COMP) && (e1.id != 0) && (e2.type == TY_INT) && (e2.id == 0))
        {
            warn(MSG_WARN_IDX_COMP_GRAB, pctx.line_num+1);

            add_instr("P_F2I_M %s\n" , v_table[e1.id].name);
            add_instr("MLT     %d\n", v_table[id].cols);
            add_instr("S_ADD\n");
            add_instr("SET     __aux_var\n");
            add_instr("%s      %s\n"  , ldv  , v_table[id].name);
            add_instr("P_LOD   __aux_var\n");
            add_instr("%s      %s_i\n", ldv  , v_table[id].name);
        }

        // comp in memory and int in memory
        if ((e1.type == TY_COMP) && (e1.id != 0) && (e2.type == TY_INT) && (e2.id != 0))
        {
            warn(MSG_WARN_IDX_COMP_GRAB, pctx.line_num+1);

            add_instr("%s    %s\n", f2i, v_table[e1.id].name);
            add_instr("MLT   %d\n"    , v_table[id].cols);
            add_instr("ADD   %s\n",      v_table[e2.id].name);
            add_instr("SET   __aux_var\n");
            add_instr( "%s   %s\n"  , ldv      , v_table[id].name);
            add_instr("P_LOD __aux_var\n");
            add_instr( "%s   %s_i\n", ldv      , v_table[id].name);
        }

        // comp in memory and float in acc
        if ((e1.type == TY_COMP) && (e1.id != 0) && (e2.type == TY_FLOAT) && (e2.id == 0))
        {
            warn(MSG_WARN_IDXS_MESS, pctx.line_num+1);

            add_instr("F2I\n");
            add_instr("P_F2I_M %s\n" , v_table[e1.id].name);
            add_instr("MLT     %d\n", v_table[id].cols);
            add_instr("S_ADD\n");
            add_instr("SET     __aux_var\n");
            add_instr( "%s     %s\n"  , ldv  , v_table[id].name);
            add_instr("P_LOD   __aux_var\n");
            add_instr( "%s     %s_i\n", ldv  , v_table[id].name);
        }

        // comp in memory and float in memory
        if ((e1.type == TY_COMP) && (e1.id != 0) && (e2.type == TY_FLOAT) && (e2.id != 0))
        {
            warn(MSG_WARN_IDXS_MESS, pctx.line_num+1);

            add_instr( "%s     %s\n", f2i, v_table[e1.id].name);
            add_instr("MLT     %d\n"    , v_table[id].cols);
            add_instr("P_F2I_M %s\n",      v_table[e2.id].name);
            add_instr("S_ADD\n");
            add_instr("SET     __aux_var\n");
            add_instr( "%s     %s\n"  , ldv      , v_table[id].name);
            add_instr("P_LOD   __aux_var\n");
            add_instr( "%s     %s_i\n", ldv      , v_table[id].name);
        }

        // comp in memory and comp const in memory
        if ((e1.type == TY_COMP) && (e1.id != 0) && (e2.type == TY_CCONST) && (e2.id != 0))
        {
            warn(MSG_WARN_IDXS_MESS, pctx.line_num+1);

            get_cmp_cst(e2, &etr, &eti);

            add_instr( "%s     %s\n", f2i, v_table[e1.id].name);
            add_instr("MLT     %d\n"    , v_table[id].cols);
            add_instr("P_F2I_M %s\n",      v_table[etr.id].name);
            add_instr("S_ADD\n");
            add_instr("SET     __aux_var\n");
            add_instr( "%s     %s\n"  , ldv      , v_table[id].name);
            add_instr("P_LOD   __aux_var\n");
            add_instr( "%s     %s_i\n", ldv      , v_table[id].name);
        }

        // comp in memory and comp in acc
        if ((e1.type == TY_COMP) && (e1.id != 0) && (e2.type == TY_COMP) && (e2.id == 0))
        {
            warn(MSG_WARN_IDXS_MESS, pctx.line_num+1);

            add_instr("SET_P __aux_var\n");
            add_instr("F2I\n");
            add_instr("P_F2I_M %s\n", v_table[e1.id].name);
            add_instr("MLT   %d\n", v_table[id].cols);
            add_instr("S_ADD\n");
            add_instr("SET   __aux_var\n");
            add_instr( "%s   %s\n", ldv    , v_table[id].name);
            add_instr("P_LOD __aux_var\n");
            add_instr( "%s %s_i\n", ldv    , v_table[id].name);
        }

        // comp in memory and comp in memory
        if ((e1.type == TY_COMP) && (e1.id != 0) && (e2.type == TY_COMP) && (e2.id != 0))
        {
            warn(MSG_WARN_IDXS_MESS, pctx.line_num+1);

            add_instr(" %s     %s\n", f2i, v_table[e1.id].name);
            add_instr("MLT     %d\n"    , v_table[id].cols);
            add_instr("P_F2I_M %s\n"     , v_table[e2.id].name);
            add_instr("S_ADD\n");
            add_instr("SET     __aux_var\n");
            add_instr(" %s     %s\n"  , ldv      , v_table[id].name);
            add_instr("P_LOD   __aux_var\n");
            add_instr(" %s     %s_i\n", ldv      , v_table[id].name);
        }
    }

    acc_live     = 1;
    v_table[id].used = 1;

    return expr_make(v_table[id].type, 0);
}