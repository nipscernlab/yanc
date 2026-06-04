// ----------------------------------------------------------------------------
// array index handling -------------------------------------------------------
// ----------------------------------------------------------------------------

/*
TODO:
1- usar instrucao virtual para evitar enderecamento idireto quando indice eh constante
*/

// global includes
#include  <stdio.h>
#include <string.h>
#include <stdlib.h>

// local includes
#include "../Headers/t2t.h"
#include "../Headers/global.h"
#include "../Headers/funcoes.h"
#include "../Headers/data_use.h"
#include "../Headers/variaveis.h"
#include "../Headers/messages.h"

// ----------------------------------------------------------------------------
// array in assignment, e.g. x[i] = y; ----------------------------------------
// ----------------------------------------------------------------------------

// loads the array index into the accumulator (1D array)
void arr_1d_index(int id, expr e)
{
    // ------------------------------------------------------------------------
    // argument check ---------------------------------------------------------
    // ------------------------------------------------------------------------

    // must check that it really is an array
    if (v_table[id].isar == 0)
        {fprintf (stderr, MSG_ERR_NOT_ARRAY_HARSH, line_num+1, rem_fname(v_table[id].name, fname)); exit(EXIT_FAILURE);}

    // must check that it really is a 1D array
    if (v_table[id].isar == 2)
        {fprintf (stderr, MSG_ERR_ARRAY_2D  , line_num+1, rem_fname(v_table[id].name, fname)); exit(EXIT_FAILURE);}

    // ------------------------------------------------------------------------
    // execute ----------------------------------------------------------------
    // ------------------------------------------------------------------------

    expr etr, eti;

    // when int in memory -------------------------------------------------

    if ((e.type == 1) && (e.id != 0))
    {
        add_instr("LOD %s\n", v_table[e.id].name);
    }

    // when int in acc ----------------------------------------------------

    if ((e.type == 1) && (e.id == 0))
    {
        // nothing to do
    }

    // when float in memory -----------------------------------------

    if ((e.type == 2) && (e.id != 0))
    {
        fprintf (stdout, MSG_WARN_IDX_FLOAT, line_num+1);

        add_instr("F2I_M %s\n", v_table[e.id].name);
    }

    // when float in acc --------------------------------------------------

    if ((e.type == 2) && (e.id == 0))
    {
        fprintf (stdout, MSG_WARN_IDX_FLOAT, line_num+1);

        add_instr("F2I\n");
    }

    // when comp const in memory ------------------------------------------

    if (e.type == 5)
    {
        fprintf (stdout, MSG_WARN_IDX_COMP_ROUND, line_num+1);

        get_cmp_cst(e, &etr, &eti);

        add_instr("F2I_M %s\n", v_table[etr.id].name);
    }

    // when comp in acc ---------------------------------------------------

    if ((e.type == 3) && (e.id == 0))
    {
        fprintf (stdout, MSG_WARN_IDX_COMP_ROUND, line_num+1);

        add_instr("POP\n");
        add_instr("F2I\n");
    }

    // when comp in memory ------------------------------------------------

    if ((e.type == 3) && (e.id != 0))
    {
        fprintf (stdout, MSG_WARN_IDX_COMP_ROUND, line_num+1);

        add_instr("F2I_M %s\n", v_table[e.id].name);
    }

    acc_ok = 1; // acc carregado
}

// loads the array index into the accumulator (2D array)
void arr_2d_index(int id, expr e1, expr e2)
{
    // ------------------------------------------------------------------------
    // argument check ---------------------------------------------------------
    // ------------------------------------------------------------------------

    // must check that it really is an array
    if (v_table[id].isar == 0)
        {fprintf (stderr, MSG_ERR_NOT_ARRAY_HARSH, line_num+1, rem_fname(v_table[id].name, fname)); exit(EXIT_FAILURE);}

    // must check that it is not a 1D array
    if (v_table[id].isar == 1)
        {fprintf (stderr, MSG_ERR_ARRAY_1D, line_num+1, rem_fname(v_table[id].name, fname)); exit(EXIT_FAILURE);}

    // ------------------------------------------------------------------------
    // execute ----------------------------------------------------------------
    // ------------------------------------------------------------------------

    expr etr, eti;

    // int in acc and int in acc
    if ((e1.type == 1) && (e1.id == 0) && (e2.type == 1) && (e2.id == 0))
    {
        add_instr("SET_P aux_var\n");
        add_instr("MLT   %s_arr_size\n", v_table[id].name);
        add_instr("ADD   aux_var\n");
    }

    // int in acc and int in memory
    if ((e1.type == 1) && (e1.id == 0) && (e2.type == 1) && (e2.id != 0))
    {
        add_instr("MLT %s_arr_size\n", v_table[id].name);
        add_instr("ADD %s\n",  v_table[e2.id].name);
    }

    // int in acc and float in acc
    if ((e1.type == 1) && (e1.id == 0) && (e2.type == 2) && (e2.id == 0))
    {
        fprintf (stdout, MSG_WARN_IDX2_FLOAT, line_num+1);

        add_instr("F2I\n");
        add_instr("SET_P aux_var\n");
        add_instr("MLT   %s_arr_size\n", v_table[id].name);
        add_instr("ADD   aux_var\n");
    }

    // int in acc and float in memory
    if ((e1.type == 1) && (e1.id == 0) && (e2.type == 2) && (e2.id != 0))
    {
        fprintf (stdout, MSG_WARN_IDX2_FLOAT, line_num+1);

        add_instr("MLT     %s_arr_size\n", v_table[id].name);
        add_instr("P_F2I_M %s\n" , v_table[e2.id].name);
        add_instr("S_ADD\n");
    }

    // int in acc and comp const in memory
    if ((e1.type == 1) && (e1.id == 0) && (e2.type == 5) && (e2.id != 0))
    {
        fprintf (stdout, MSG_WARN_IDX_COMP_GRAB, line_num+1);

        get_cmp_cst(e2, &etr, &eti);

        add_instr("MLT     %s_arr_size\n", v_table[id].name);
        add_instr("P_F2I_M %s\n" , v_table[etr.id].name);
        add_instr("S_ADD\n");
    }

    // int in acc and comp in acc
    if ((e1.type == 1) && (e1.id == 0) && (e2.type == 3) && (e2.id == 0))
    {
        fprintf (stdout, MSG_WARN_IDX_COMP_GRAB, line_num+1);

        add_instr("SET_P aux_var\n");
        add_instr("F2I\n");
        add_instr("SET_P aux_var\n");
        add_instr("MLT   %s_arr_size\n", v_table[id].name);
        add_instr("ADD   aux_var\n");
    }

    // int in acc and comp in memory
    if ((e1.type == 1) && (e1.id == 0) && (e2.type == 3) && (e2.id != 0))
    {
        fprintf (stdout, MSG_WARN_IDX_COMP_GRAB, line_num+1);

        add_instr("MLT     %s_arr_size\n", v_table[id].name);
        add_instr("P_F2I_M %s\n" , v_table[e2.id].name);
        add_instr("S_ADD\n");
    }

    // int in memory and int in acc
    if ((e1.type == 1) && (e1.id != 0) && (e2.type == 1) && (e2.id == 0))
    {
        add_instr("P_LOD %s\n",  v_table[e1.id].name);
        add_instr("MLT   %s_arr_size\n", v_table[id].name);
        add_instr("S_ADD\n");
    }

    // int in memory and int in memory
    if ((e1.type == 1) && (e1.id != 0) && (e2.type == 1) && (e2.id != 0))
    {
        add_instr("LOD  %s\n",  v_table[e1.id].name);
        add_instr("MLT  %s_arr_size\n", v_table[id].name);
        add_instr("ADD  %s\n",  v_table[e2.id].name);
    }

    // int in memory and float in acc
    if ((e1.type == 1) && (e1.id != 0) && (e2.type == 2) && (e2.id == 0))
    {
        fprintf (stdout, MSG_WARN_IDX2_FLOAT, line_num+1);

        add_instr("F2I\n");
        add_instr("P_LOD %s\n",  v_table[e1.id].name);
        add_instr("MLT   %s_arr_size\n", v_table[id].name);
        add_instr("S_ADD\n");
    }

    // int in memory and float in memory
    if ((e1.type == 1) && (e1.id != 0) && (e2.type == 2) && (e2.id != 0))
    {
        fprintf (stdout, MSG_WARN_IDX2_FLOAT, line_num+1);

        add_instr("LOD     %s\n",  v_table[e1.id].name);
        add_instr("MLT     %s_arr_size\n", v_table[id].name);
        add_instr("P_F2I_M %s\n",  v_table[e2.id].name);
        add_instr("S_ADD\n");
    }

    // int in memory and comp const in memory
    if ((e1.type == 1) && (e1.id != 0) && (e2.type == 5) && (e2.id != 0))
    {
        fprintf(stdout, MSG_WARN_IDX_COMP_GRAB, line_num+1);

        get_cmp_cst(e2, &etr, &eti);

        add_instr("LOD     %s\n",  v_table[e1.id].name);
        add_instr("MLT     %s_arr_size\n", v_table[id].name);
        add_instr("P_F2I_M %s\n",  v_table[etr.id].name);
        add_instr("S_ADD\n");
    }

    // int in memory and comp in acc
    if ((e1.type == 1) && (e1.id != 0) && (e2.type == 3) && (e2.id == 0))
    {
        fprintf(stdout, MSG_WARN_IDX_COMP_GRAB, line_num+1);

        add_instr("SET_P aux_var\n");
        add_instr("F2I\n");
        add_instr("SET   aux_var\n");
        add_instr("LOD   %s\n",  v_table[e1.id].name);
        add_instr("MLT   %s_arr_size\n", v_table[id].name);
        add_instr("ADD   aux_var\n");
    }

    // int in memory and comp in memory
    if ((e1.type == 1) && (e1.id != 0) && (e2.type == 3) && (e2.id != 0))
    {
        fprintf(stdout, MSG_WARN_IDX_COMP_GRAB, line_num+1);

        add_instr("LOD     %s\n",  v_table[e1.id].name);
        add_instr("MLT     %s_arr_size\n", v_table[id].name);
        add_instr("P_F2I_M %s\n",  v_table[e2.id].name);
        add_instr("S_ADD\n");
    }

    // float in acc and int in acc
    if ((e1.type == 2) && (e1.id == 0) && (e2.type == 1) && (e2.id == 0))
    {
        fprintf(stdout, MSG_WARN_IDX1_FLOAT, line_num+1);

        add_instr("SET_P aux_var\n");
        add_instr("F2I\n");
        add_instr("MLT   %s_arr_size\n", v_table[id].name);
        add_instr("ADD   aux_var\n");
    }

    // float in acc and int in memory
    if ((e1.type == 2) && (e1.id == 0) && (e2.type == 1) && (e2.id != 0))
    {
        fprintf(stdout, MSG_WARN_IDX1_FLOAT, line_num+1);

        add_instr("F2I\n");
        add_instr("MLT %s_arr_size\n", v_table[id].name);
        add_instr("ADD %s\n",  v_table[e2.id].name);
    }

    // float in acc and float in acc
    if ((e1.type == 2) && (e1.id == 0) && (e2.type == 2) && (e2.id == 0))
    {
        fprintf(stdout, MSG_WARN_IDXS_FLOAT, line_num+1);

        add_instr("F2I\n");
        add_instr("SET_P aux_var\n");
        add_instr("F2I\n");
        add_instr("MLT   %s_arr_size\n", v_table[id].name);
        add_instr("ADD   aux_var\n");
    }

    // float in acc and float in memory
    if ((e1.type == 2) && (e1.id == 0) && (e2.type == 2) && (e2.id != 0))
    {
        fprintf(stdout, MSG_WARN_IDXS_MESS, line_num+1);

        add_instr("F2I\n");
        add_instr("MLT     %s_arr_size\n", v_table[id].name);
        add_instr("P_F2I_M %s\n" , v_table[e2.id].name);
        add_instr("S_ADD\n");
    }

    // float in acc and comp const in memory
    if ((e1.type == 2) && (e1.id == 0) && (e2.type == 5) && (e2.id != 0))
    {
        fprintf(stdout, MSG_WARN_IDXS_MESS, line_num+1);

        get_cmp_cst(e2, &etr, &eti);

        add_instr("F2I\n");
        add_instr("MLT     %s_arr_size\n", v_table[id].name);
        add_instr("P_F2I_M %s\n" , v_table[etr.id].name);
        add_instr("S_ADD\n");
    }

    // float in acc and comp in acc
    if ((e1.type == 2) && (e1.id == 0) && (e2.type == 3) && (e2.id == 0))
    {
        fprintf (stdout, MSG_WARN_IDXS_MESS, line_num+1);

        add_instr("SET_P aux_var\n");
        add_instr("F2I\n");
        add_instr("SET_P aux_var\n");
        add_instr("F2I\n");
        add_instr("MLT   %s_arr_size\n", v_table[id].name);
        add_instr("ADD   aux_var\n");
    }

    // float in acc and comp in memory
    if ((e1.type == 2) && (e1.id == 0) && (e2.type == 3) && (e2.id != 0))
    {
        fprintf (stdout, MSG_WARN_IDXS_MESS, line_num+1);

        add_instr("F2I\n");
        add_instr("MLT     %s_arr_size\n", v_table[id].name);
        add_instr("P_F2I_M %s\n" , v_table[e2.id].name);
        add_instr("S_ADD\n");
    }

    // float in memory and int in acc
    if ((e1.type == 2) && (e1.id != 0) && (e2.type == 1) && (e2.id == 0))
    {
        fprintf(stdout, MSG_WARN_IDX1_FLOAT, line_num+1);

        add_instr("P_F2I_M %s\n" , v_table[e1.id].name);
        add_instr("MLT     %s_arr_size\n", v_table[id].name);
        add_instr("S_ADD\n");
    }

    // float in memory and int in memory
    if ((e1.type == 2) && (e1.id != 0) && (e2.type == 1) && (e2.id != 0))
    {
        fprintf(stdout, MSG_WARN_IDX1_FLOAT, line_num+1);

        add_instr("F2I_M  %s\n",  v_table[e1.id].name);
        add_instr("MLT    %s_arr_size\n", v_table[id].name);
        add_instr("ADD    %s\n",  v_table[e2.id].name);
    }

    // float in memory and float in acc
    if ((e1.type == 2) && (e1.id != 0) && (e2.type == 2) && (e2.id == 0))
    {
        fprintf (stdout, MSG_WARN_IDXS_FLOAT, line_num+1);

        add_instr("F2I\n");
        add_instr("P_F2I_M %s\n",  v_table[e1.id].name);
        add_instr("MLT     %s_arr_size\n", v_table[id].name);
        add_instr("S_ADD\n");
    }

    // float in memory and float in memory
    if ((e1.type == 2) && (e1.id != 0) && (e2.type == 2) && (e2.id != 0))
    {
        fprintf (stdout, MSG_WARN_IDXS_FLOAT, line_num+1);

        add_instr("F2I_M   %s\n",  v_table[e1.id].name);
        add_instr("MLT     %s_arr_size\n", v_table[id].name);
        add_instr("P_F2I_M %s\n",  v_table[e2.id].name);
        add_instr("S_ADD\n");
    }

    // float in memory and comp const in memory
    if ((e1.type == 2) && (e1.id != 0) && (e2.type == 5) && (e2.id != 0))
    {
        fprintf(stdout, MSG_WARN_IDXS_MESS, line_num+1);

        get_cmp_cst(e2, &etr, &eti);

        add_instr("F2I_M   %s\n",  v_table[e1.id].name);
        add_instr("MLT     %s_arr_size\n", v_table[id].name);
        add_instr("P_F2I_M %s\n",  v_table[etr.id].name);
        add_instr("S_ADD\n");
    }

    // float in memory and comp in acc
    if ((e1.type == 2) && (e1.id != 0) && (e2.type == 3) && (e2.id == 0))
    {
        fprintf(stdout, MSG_WARN_IDXS_MESS, line_num+1);

        add_instr("SET_P aux_var\n");
        add_instr("F2I\n");
        add_instr("SET   aux_var\n");
        add_instr("F2I_M %s\n",  v_table[e1.id].name);
        add_instr("MLT   %s_arr_size\n", v_table[id].name);
        add_instr("ADD   aux_var\n");
    }

    // float in memory and comp in memory
    if ((e1.type == 2) && (e1.id != 0) && (e2.type == 3) && (e2.id != 0))
    {
        fprintf(stdout, MSG_WARN_IDXS_MESS, line_num+1);

        add_instr("F2I_M   %s\n",  v_table[e1.id].name);
        add_instr("MLT     %s_arr_size\n", v_table[id].name);
        add_instr("P_F2I_M %s\n",  v_table[e2.id].name);
        add_instr("S_ADD\n");
    }

    // comp const in memory and int in acc
    if ((e1.type == 5) && (e1.id != 0) && (e2.type == 1) && (e2.id == 0))
    {
        fprintf (stdout, MSG_WARN_IDX_COMP_GRAB, line_num+1);

        get_cmp_cst(e1, &etr, &eti);

        add_instr("F2I\n");
        add_instr("SET   aux_var\n");
        add_instr("F2I_M %s\n" , v_table[etr.id].name);
        add_instr("MLT   %s_arr_size\n", v_table[id].name);
        add_instr("ADD   aux_var\n");
    }

    // comp const in memory and int in memory
    if ((e1.type == 5) && (e1.id != 0) && (e2.type == 1) && (e2.id != 0))
    {
        fprintf(stdout, MSG_WARN_IDX_COMP_GRAB, line_num+1);

        get_cmp_cst(e1, &etr, &eti);

        add_instr("F2I_M %s\n" , v_table[etr.id].name);
        add_instr("MLT   %s_arr_size\n", v_table[id].name);
        add_instr("ADD   %s\n",  v_table[e2.id].name);
    }

    // comp const in memory and float in acc
    if ((e1.type == 5) && (e1.id != 0) && (e2.type == 2) && (e2.id == 0))
    {
        fprintf(stdout, MSG_WARN_IDXS_MESS, line_num+1);

        get_cmp_cst(e1, &etr, &eti);

        add_instr("F2I\n");
        add_instr("SET   aux_var\n");
        add_instr("F2I_M %s\n" , v_table[etr.id].name);
        add_instr("MLT   %s_arr_size\n", v_table[id].name);
        add_instr("ADD   aux_var\n");
    }

    // comp const in memory and float in memory
    if ((e1.type == 5) && (e1.id != 0) && (e2.type == 2) && (e2.id != 0))
    {
        fprintf(stdout, MSG_WARN_IDXS_MESS, line_num+1);

        get_cmp_cst(e1, &etr, &eti);

        add_instr("F2I_M   %s\n" , v_table[etr.id].name);
        add_instr("MLT     %s_arr_size\n", v_table[id].name);
        add_instr("P_F2I_M %s\n" , v_table[e2.id].name);
        add_instr("S_ADD\n");
    }

    // comp const in memory and comp const in memory
    if ((e1.type == 5) && (e1.id != 0) && (e2.type == 5) && (e2.id != 0))
    {
        fprintf (stdout, MSG_WARN_IDXS_MESS, line_num+1);

        get_cmp_cst(e1, &etr, &eti);
        add_instr("F2I_M   %s\n" , v_table[etr.id].name);
        add_instr("MLT     %s_arr_size\n", v_table[id].name);

        get_cmp_cst(e2, &etr, &eti);
        add_instr("P_F2I_M %s\n" , v_table[etr.id].name);
        add_instr("S_ADD\n");
    }

    // comp const in memory and comp in acc
    if ((e1.type == 5) && (e1.id != 0) && (e2.type == 3) && (e2.id == 0))
    {
        fprintf(stdout, MSG_WARN_IDXS_MESS, line_num+1);

        get_cmp_cst(e1, &etr, &eti);

        add_instr("SET_P aux_var\n");
        add_instr("F2I\n");
        add_instr("SET   aux_var\n");
        add_instr("F2I_M %s\n" , v_table[etr.id].name);
        add_instr("MLT   %s_arr_size\n", v_table[id].name);
        add_instr("ADD   aux_var\n");
    }

    // comp const in memory and comp in memory
    if ((e1.type == 5) && (e1.id != 0) && (e2.type == 3) && (e2.id != 0))
    {
        fprintf(stdout, MSG_WARN_IDXS_MESS, line_num+1);

        get_cmp_cst(e1, &etr, &eti);

        add_instr("F2I_M   %s\n" , v_table[etr.id].name);
        add_instr("MLT     %s_arr_size\n", v_table[id].name);
        add_instr("P_F2I_M %s\n" , v_table[e2.id].name);
        add_instr("S_ADD\n");
    }

    // comp in acc and int in acc
    if ((e1.type == 3) && (e1.id == 0) && (e2.type == 1) && (e2.id == 0))
    {
        fprintf(stdout, MSG_WARN_IDX_COMP_GRAB, line_num+1);

        add_instr("SET_P aux_var\n");
        add_instr("POP\n");
        add_instr("F2I\n");
        add_instr("MLT   %s_arr_size\n", v_table[id].name);
        add_instr("ADD   aux_var\n");
    }

    // comp in acc and int in memory
    if ((e1.type == 3) && (e1.id == 0) && (e2.type == 1) && (e2.id != 0))
    {
        fprintf(stdout, MSG_WARN_IDX_COMP_GRAB, line_num+1);

        add_instr("POP\n");
        add_instr("F2I\n");
        add_instr("MLT %s_arr_size\n", v_table[id].name);
        add_instr("ADD %s\n",  v_table[e2.id].name);
    }

    // comp in acc and float in acc
    if ((e1.type == 3) && (e1.id == 0) && (e2.type == 2) && (e2.id == 0))
    {
        fprintf (stdout, MSG_WARN_IDXS_MESS, line_num+1);

        add_instr("F2I\n");
        add_instr("SET_P aux_var\n");
        add_instr("POP\n");
        add_instr("F2I\n");
        add_instr("MLT   %s_arr_size\n", v_table[id].name);
        add_instr("ADD   aux_var\n");
    }

    // comp in acc and float in memory
    if ((e1.type == 3) && (e1.id == 0) && (e2.type == 2) && (e2.id != 0))
    {
        fprintf(stdout, MSG_WARN_IDXS_MESS, line_num+1);

        add_instr("POP\n");
        add_instr("F2I\n");
        add_instr("MLT %s_arr_size\n"   , v_table[id].name);
        add_instr("P_F2I_M %s\n", v_table[e2.id].name);
        add_instr("S_ADD\n");
    }

    // comp in acc and comp const in memory
    if ((e1.type == 3) && (e1.id == 0) && (e2.type == 5) && (e2.id != 0))
    {
        fprintf(stdout, MSG_WARN_IDXS_MESS, line_num+1);

        get_cmp_cst(e2, &etr, &eti);

        add_instr("POP\n");
        add_instr("F2I\n");
        add_instr("MLT     %s_arr_size\n", v_table[id].name);
        add_instr("P_F2I_M %s\n" , v_table[etr.id].name);
        add_instr("S_ADD\n");
    }

    // comp in acc and comp in acc
    if ((e1.type == 3) && (e1.id == 0) && (e2.type == 3) && (e2.id == 0))
    {
        fprintf(stdout, MSG_WARN_IDXS_MESS, line_num+1);

        add_instr("SET_P aux_var\n");
        add_instr("F2I\n");
        add_instr("SET_P aux_var\n");
        add_instr("POP\n");
        add_instr("F2I\n");
        add_instr("MLT   %s_arr_size\n", v_table[id].name);
        add_instr("ADD   aux_var\n");
    }

    // comp in acc and comp in memory
    if ((e1.type == 3) && (e1.id == 0) && (e2.type == 3) && (e2.id != 0))
    {
        fprintf (stdout, MSG_WARN_IDXS_MESS, line_num+1);

        add_instr("POP\n");
        add_instr("F2I\n");
        add_instr("MLT     %s_arr_size\n", v_table[id].name);
        add_instr("P_F2I_M %s\n" , v_table[e2.id].name);
        add_instr("S_ADD\n");
    }

    // comp in memory and int in acc
    if ((e1.type == 3) && (e1.id != 0) && (e2.type == 1) && (e2.id == 0))
    {
        fprintf(stdout, MSG_WARN_IDX_COMP_GRAB, line_num+1);

        add_instr("P_F2I_M %s\n",  v_table[e1.id].name);
        add_instr("MLT     %s_arr_size\n", v_table[id].name);
        add_instr("S_ADD\n");
    }

    // comp in memory and int in memory
    if ((e1.type == 3) && (e1.id != 0) && (e2.type == 1) && (e2.id != 0))
    {
        fprintf(stdout, MSG_WARN_IDX_COMP_GRAB, line_num+1);

        add_instr("F2I_M %s\n",  v_table[e1.id].name);
        add_instr("MLT   %s_arr_size\n", v_table[id].name);
        add_instr("ADD   %s\n",  v_table[e2.id].name);
    }

    // comp in memory and float in acc
    if ((e1.type == 3) && (e1.id != 0) && (e2.type == 2) && (e2.id == 0))
    {
        fprintf(stdout, MSG_WARN_IDXS_MESS, line_num+1);

        add_instr("F2I\n");
        add_instr("P_F2I_M %s\n",  v_table[e1.id].name);
        add_instr("MLT     %s_arr_size\n", v_table[id].name);
        add_instr("S_ADD\n");
    }

    // comp in memory and float in memory
    if ((e1.type == 3) && (e1.id != 0) && (e2.type == 2) && (e2.id != 0))
    {
        fprintf(stdout, MSG_WARN_IDXS_MESS, line_num+1);

        add_instr("F2I_M   %s\n",  v_table[e1.id].name);
        add_instr("MLT     %s_arr_size\n", v_table[id].name);
        add_instr("P_F2I_M %s\n",  v_table[e2.id].name);
        add_instr("S_ADD\n");
    }

    // comp in memory and comp const in memory
    if ((e1.type == 3) && (e1.id != 0) && (e2.type == 5) && (e2.id != 0))
    {
        fprintf (stdout, MSG_WARN_IDXS_MESS, line_num+1);

        get_cmp_cst(e2, &etr, &eti);

        add_instr("F2I_M   %s\n" , v_table[e1.id].name);
        add_instr("MLT     %s_arr_size\n", v_table[id].name);
        add_instr("P_F2I_M %s\n" , v_table[etr.id].name);
        add_instr("S_ADD\n");
    }

    // comp in memory and comp in acc
    if ((e1.type == 3) && (e1.id != 0) && (e2.type == 3) && (e2.id == 0))
    {
        fprintf (stdout, MSG_WARN_IDXS_MESS, line_num+1);

        add_instr("SET_P aux_var\n");
        add_instr("F2I\n");
        add_instr("SET   aux_var\n");
        add_instr("F2I_M %s\n" , v_table[e1.id].name);
        add_instr("MLT   %s_arr_size\n", v_table[id].name);
        add_instr("ADD   aux_var\n");
    }

    // comp in memory and comp in memory
    if ((e1.type == 3) && (e1.id != 0) && (e2.type == 3) && (e2.id != 0))
    {
        fprintf (stdout, MSG_WARN_IDXS_MESS, line_num+1);

        add_instr("F2I_M   %s\n" , v_table[e1.id].name);
        add_instr("MLT     %s_arr_size\n", v_table[id].name);
        add_instr("P_F2I_M %s\n" , v_table[e2.id].name);
        add_instr("S_ADD\n");
    }

    acc_ok = 1; // acc carregado
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
// choice. The walker routes only int-array / 1D-forward reads here.
expr arr_1d2exp_const(int id, int k)
{
    if (acc_ok) add_instr("P_LOD_V %s %d\n", v_table[id].name, k);
    else        add_instr(  "LOD_V %s %d\n", v_table[id].name, k);
    acc_ok = 1;  // value now in the accumulator
    return expr_make(v_table[id].type, 0);
}

// turns a 1D array into an exp
// the fft parameter tells whether to use the reversed index
expr arr_1d2exp(int id, expr e, int fft)
{
    // consistency checks -----------------------------------------------------

    // test whether the variable has been declared
    if (v_table[id].type == 0)
        {fprintf (stderr, MSG_ERR_DECL_VAR_PROPERLY, line_num+1, rem_fname(v_table[id].name, fname)); exit(EXIT_FAILURE);}

    // must check that it really is an array
    if (v_table[id].isar == 0)
        {fprintf (stderr, MSG_ERR_NOT_ARRAY_HARSH, line_num+1, rem_fname(v_table[id].name, fname)); exit(EXIT_FAILURE);}

    // must check that it really is a 1D array
    if (v_table[id].isar == 2)
        {fprintf (stderr, MSG_ERR_ARRAY_2D  , line_num+1, rem_fname(v_table[id].name, fname)); exit(EXIT_FAILURE);}

    // prepare the LOD commands ----------------------------------------------

    char ldv[10]; if (fft    == 1) strcpy(ldv,"ILI"    ); else strcpy(ldv,"LDI"  );
    char ldi[10]; if (acc_ok == 1) strcpy(ldi,"P_LOD"  ); else strcpy(ldi,"LOD"  );
    char f2i[10]; if (acc_ok == 1) strcpy(f2i,"P_F2I_M"); else strcpy(f2i,"F2I_M");

    // ------------------------------------------------------------------------
    // write the instructions -------------------------------------------------
    // ------------------------------------------------------------------------

    expr  etr, eti;
    int  type = v_table[id].type;

    // left int/float ---------------------------------------------------------

    if (type < 3)
    {
        // int in acc
        if ((e.type == 1) && (e.id == 0))
        {
            add_instr("%s %s\n", ldv, v_table[id].name);
        }

        // int in memory
        if ((e.type == 1) && (e.id != 0))
        {
            add_instr("%s %s\n", ldi, v_table[e.id].name);
            add_instr("%s %s\n", ldv, v_table[id].name);
        }

        // float in acc
        if ((e.type == 2) && (e.id == 0))
        {
            fprintf (stdout, MSG_WARN_IDX_FLOAT_HEAVY, line_num+1);

            add_instr("F2I\n");
            add_instr("%s %s\n", ldv, v_table[id].name);
        }

        // float in memory
        if ((e.type == 2) && (e.id != 0))
        {
            fprintf (stdout, MSG_WARN_IDX_FLOAT_HEAVY, line_num+1);

            add_instr("%s %s\n", f2i, v_table[e.id].name);
            add_instr("%s %s\n", ldv, v_table[id].name);
        }

        // comp const in memory
        if ((e.type == 5) && (e.id != 0))
        {
            fprintf (stdout, MSG_WARN_IDX_COMP_ROUND, line_num+1);

            get_cmp_cst(e, &etr, &eti);

            add_instr("%s %s\n", f2i, v_table[etr.id].name);
            add_instr("%s %s\n", ldv, v_table[id].name);
        }

        // comp in acc
        if ((e.type == 3) && (e.id == 0))
        {
            fprintf (stdout, MSG_WARN_IDX_COMP_ROUND, line_num+1);

            add_instr("POP\n");
            add_instr("F2I\n");
            add_instr("%s %s\n", ldv, v_table[id].name);
        }

        // comp in memory
        if ((e.type == 3) && (e.id != 0))
        {
            fprintf (stdout, MSG_WARN_IDX_COMP_ROUND, line_num+1);

            add_instr("%s %s\n", f2i, v_table[e.id].name);
            add_instr("%s %s\n", ldv, v_table[id].name);
        }
    }

    // left comp --------------------------------------------------------------

    else
    {
        // int in acc
        if ((e.type == 1) && (e.id == 0))
        {
            add_instr("SET   aux_var\n");
            add_instr("%s %s\n"  , ldv, v_table[id].name);
            add_instr("P_LOD aux_var\n");
            add_instr("%s %s_i\n", ldv, v_table[id].name);
        }

        // int in memory
        if ((e.type == 1) && (e.id != 0))
        {
            add_instr("%s %s\n"    , ldi, v_table[e.id].name);
            add_instr("%s %s\n"    , ldv, v_table[id].name);
            add_instr("P_LOD %s\n" ,      v_table[e.id].name);
            add_instr("%s %s_i\n"  , ldv, v_table[id].name);
        }

        // float in acc
        if ((e.type == 2) && (e.id == 0))
        {
            fprintf (stdout, MSG_WARN_IDX_FLOAT_HEAVY, line_num+1);

            add_instr("F2I\n");
            add_instr("SET   aux_var\n");
            add_instr("%s %s\n"  , ldv, v_table[id].name);
            add_instr("P_LOD aux_var\n");
            add_instr("%s %s_i\n", ldv, v_table[id].name);
        }

        // float in memory
        if ((e.type == 2) && (e.id != 0))
        {
            fprintf (stdout, MSG_WARN_IDX_FLOAT_HEAVY, line_num+1);

            add_instr("%s %s\n"  , f2i, v_table[e.id].name);
            add_instr("SET   aux_var\n");
            add_instr("%s %s\n"  , ldv, v_table[id].name);
            add_instr("P_LOD aux_var\n");
            add_instr("%s %s_i\n", ldv, v_table[id].name);
        }

        // comp const in memory
        if ((e.type == 5) && (e.id != 0))
        {
            fprintf (stdout, MSG_WARN_IDX_COMP_ROUND, line_num+1);

            get_cmp_cst(e, &etr, &eti);

            add_instr("%s %s\n"  , f2i, v_table[etr.id].name);
            add_instr("SET aux_var\n");
            add_instr("%s %s\n"  , ldv, v_table[id].name);
            add_instr("P_LOD aux_var\n");
            add_instr("%s %s_i\n", ldv, v_table[id].name);
        }

        // comp in acc
        if ((e.type == 3) && (e.id == 0))
        {
            fprintf (stdout, MSG_WARN_IDX_COMP_ROUND, line_num+1);

            add_instr("POP\n");
            add_instr("F2I\n");
            add_instr("SET   aux_var\n");
            add_instr("%s %s\n"  , ldv, v_table[id].name);
            add_instr("P_LOD aux_var\n");
            add_instr("%s %s_i\n", ldv, v_table[id].name);
        }

        // comp in memory
        if ((e.type == 3) && (e.id != 0))
        {
            fprintf (stdout, MSG_WARN_IDX_COMP_ROUND, line_num+1);

            add_instr("%s %s\n"  , f2i, v_table[e.id].name);
            add_instr("SET   aux_var\n");
            add_instr("%s %s\n"  , ldv, v_table[id].name);
            add_instr("P_LOD aux_var\n");
            add_instr("%s %s_i\n", ldv, v_table[id].name);
        }
    }

    acc_ok     = 1;
    v_table[id].used = 1;

    return expr_make(v_table[id].type, 0);
}

// transforma array 2D em exp
expr arr_2d2exp(int id, expr e1, expr e2)
{
    // consistency checks -----------------------------------------------------

    // test whether the variable has been declared
    if (v_table[id].type == 0)
        {fprintf (stderr, MSG_ERR_DECL_VAR_PROPERLY, line_num+1, rem_fname(v_table[id].name, fname)); exit(EXIT_FAILURE);}

    // must check that it really is a 2D array
    if (v_table[id].isar == 0)
        {fprintf (stderr, MSG_ERR_NOT_ARRAY_HARSH, line_num+1, rem_fname(v_table[id].name, fname)); exit(EXIT_FAILURE);}

    // must check that it is not a 1D array
    if (v_table[id].isar == 1)
        {fprintf (stderr, MSG_ERR_ARRAY_1D , line_num+1, rem_fname(v_table[id].name, fname)); exit(EXIT_FAILURE);}

    // prepare the LOD commands -----------------------------------------------

    char ldv[10];                  strcpy(ldv,"LDI"    ); // no 2D FFT yet
    char ldi[10]; if (acc_ok == 1) strcpy(ldi,"P_LOD"  ); else strcpy(ldi,"LOD"  );
    char f2i[10]; if (acc_ok == 1) strcpy(f2i,"P_F2I_M"); else strcpy(f2i,"F2I_M");

    // ------------------------------------------------------------------------
    // write the instructions -------------------------------------------------
    // ------------------------------------------------------------------------

    expr  etr, eti;
    int  type = v_table[id].type;

    // left int/float ---------------------------------------------------------

    if (type < 3)
    {
        // int in acc and int in acc
        if ((e1.type == 1) && (e1.id == 0) && (e2.type == 1) && (e2.id == 0))
        {
            add_instr("SET_P aux_var\n");
            add_instr("MLT   %s_arr_size\n", v_table[id].name);
            add_instr("ADD   aux_var\n");
            add_instr( "%s   %s\n", ldv    , v_table[id].name);
        }

        // int in acc and int in memory
        if ((e1.type == 1) && (e1.id == 0) && (e2.type == 1) && (e2.id != 0))
        {
            add_instr("MLT %s_arr_size\n", v_table[id].name);
            add_instr("ADD %s\n" , v_table[e2.id].name);
            add_instr( "%s %s\n" , ldv   , v_table[id].name);
        }

        // int in acc and float in acc
        if ((e1.type == 1) && (e1.id == 0) && (e2.type == 2) && (e2.id == 0))
        {
            fprintf (stdout, MSG_WARN_IDX2_FLOAT, line_num+1);

            add_instr("F2I\n");
            add_instr("SET_P aux_var\n");
            add_instr("MLT   %s_arr_size\n", v_table[id].name);
            add_instr("ADD   aux_var\n");
            add_instr( "%s   %s\n", ldv    , v_table[id].name);
        }

        // int in acc and float in memory
        if ((e1.type == 1) && (e1.id == 0) && (e2.type == 2) && (e2.id != 0))
        {
            fprintf (stdout, MSG_WARN_IDX2_FLOAT, line_num+1);

            add_instr("MLT     %s_arr_size\n", v_table[id].name);
            add_instr("P_F2I_M %s\n" , v_table[e2.id].name);
            add_instr("S_ADD\n");
            add_instr( "%s     %s\n", ldv    , v_table[id].name);
        }

        // int in acc and comp const in memory
        if ((e1.type == 1) && (e1.id == 0) && (e2.type == 5) && (e2.id != 0))
        {
            fprintf (stdout, MSG_WARN_IDX_COMP_GRAB, line_num+1);

            get_cmp_cst(e2, &etr, &eti);

            add_instr("MLT     %s_arr_size\n", v_table[id].name);
            add_instr("P_F2I_M %s\n" , v_table[etr.id].name);
            add_instr("S_ADD\n");
            add_instr( "%s     %s\n", ldv    , v_table[id].name);
        }

        // int in acc and comp in acc
        if ((e1.type == 1) && (e1.id == 0) && (e2.type == 3) && (e2.id == 0))
        {
            fprintf (stdout, MSG_WARN_IDX_COMP_GRAB, line_num+1);

            add_instr("SET_P aux_var\n");
            add_instr("F2I\n");
            add_instr("SET_P aux_var\n");
            add_instr("MLT   %s_arr_size\n", v_table[id].name);
            add_instr("ADD   aux_var\n");
            add_instr( "%s   %s\n", ldv    , v_table[id].name);
        }

        // int in acc and comp in memory
        if ((e1.type == 1) && (e1.id == 0) && (e2.type == 3) && (e2.id != 0))
        {
            fprintf (stdout, MSG_WARN_IDX_COMP_GRAB, line_num+1);

            add_instr("MLT     %s_arr_size\n", v_table[id].name);
            add_instr("P_F2I_M %s\n" , v_table[e2.id].name);
            add_instr("S_ADD\n");
            add_instr( "%s     %s\n", ldv    , v_table[id].name);
        }

        // int in memory and int in acc
        if ((e1.type == 1) && (e1.id != 0) && (e2.type == 1) && (e2.id == 0))
        {
            add_instr("P_LOD %s\n" , v_table[e1.id].name);
            add_instr("MLT   %s_arr_size\n", v_table[id].name);
            add_instr("S_ADD\n");
            add_instr( "%s   %s\n", ldv    , v_table[id].name);
        }

        // int in memory and int in memory
        if ((e1.type == 1) && (e1.id != 0) && (e2.type == 1) && (e2.id != 0))
        {
            add_instr( "%s %s\n", ldi, v_table[e1.id].name);
            add_instr("MLT %s_arr_size\n"    , v_table[id].name);
            add_instr("ADD %s\n"     , v_table[e2.id].name);
            add_instr( "%s %s\n", ldv        , v_table[id].name);
        }

        // int in memory and float in acc
        if ((e1.type == 1) && (e1.id != 0) && (e2.type == 2) && (e2.id == 0))
        {
            fprintf (stdout, MSG_WARN_IDX2_FLOAT, line_num+1);

            add_instr("F2I\n");
            add_instr("P_LOD %s\n" , v_table[e1.id].name);
            add_instr("MLT   %s_arr_size\n", v_table[id].name);
            add_instr("S_ADD\n");
            add_instr( "%s   %s\n", ldv    , v_table[id].name);
        }

        // int in memory and float in memory
        if ((e1.type == 1) && (e1.id != 0) && (e2.type == 2) && (e2.id != 0))
        {
            fprintf (stdout, MSG_WARN_IDX2_FLOAT, line_num+1);

            add_instr( "%s     %s\n", ldi, v_table[e1.id].name);
            add_instr("MLT     %s_arr_size\n"    , v_table[id].name);
            add_instr("P_F2I_M %s\n"     , v_table[e2.id].name);
            add_instr("S_ADD\n");
            add_instr( "%s     %s\n", ldv        , v_table[id].name);
        }

        // int in memory and comp const in memory
        if ((e1.type == 1) && (e1.id != 0) && (e2.type == 5) && (e2.id != 0))
        {
            fprintf (stdout, MSG_WARN_IDX_COMP_GRAB, line_num+1);

            get_cmp_cst(e2, &etr, &eti);

            add_instr( "%s     %s\n", ldi, v_table[e1.id].name);
            add_instr("MLT     %s_arr_size\n"    , v_table[id].name);
            add_instr("P_F2I_M %s\n"     , v_table[etr.id].name);
            add_instr("S_ADD\n");
            add_instr( "%s     %s\n", ldv        , v_table[id].name);
        }

        // int in memory and comp in acc
        if ((e1.type == 1) && (e1.id != 0) && (e2.type == 3) && (e2.id == 0))
        {
            fprintf (stdout, MSG_WARN_IDX_COMP_GRAB, line_num+1);

            add_instr("SET_P aux_var\n");
            add_instr("F2I\n");
            add_instr("SET   aux_var\n");
            add_instr("LOD   %s\n",  v_table[e1.id].name);
            add_instr("MLT   %s_arr_size\n", v_table[id].name);
            add_instr("ADD   aux_var\n");
            add_instr( "%s   %s\n", ldv    , v_table[id].name);
        }

        // int in memory and comp in memory
        if ((e1.type == 1) && (e1.id != 0) && (e2.type == 3) && (e2.id != 0))
        {
            fprintf (stdout, MSG_WARN_IDX_COMP_GRAB, line_num+1);

            add_instr( "%s     %s\n", ldi, v_table[e1.id].name);
            add_instr("MLT     %s_arr_size\n"    , v_table[id].name);
            add_instr("P_F2I_M %s\n"     , v_table[e2.id].name);
            add_instr("S_ADD\n");
            add_instr( "%s     %s\n", ldv        , v_table[id].name);
        }

        // float in acc and int in acc
        if ((e1.type == 2) && (e1.id == 0) && (e2.type == 1) && (e2.id == 0))
        {
            fprintf (stdout, MSG_WARN_IDX1_FLOAT, line_num+1);

            add_instr("SET_P aux_var\n");
            add_instr("F2I\n");
            add_instr("MLT   %s_arr_size\n", v_table[id].name);
            add_instr("ADD   aux_var\n");
            add_instr( "%s   %s\n", ldv    , v_table[id].name);
        }

        // float in acc and int in memory
        if ((e1.type == 2) && (e1.id == 0) && (e2.type == 1) && (e2.id != 0))
        {
            fprintf (stdout, MSG_WARN_IDX1_FLOAT, line_num+1);

            add_instr("F2I\n");
            add_instr("MLT %s_arr_size\n", v_table[id].name);
            add_instr("ADD %s\n" , v_table[e2.id].name);
            add_instr( "%s %s\n" , ldv   , v_table[id].name);
        }

        // float in acc and float in acc
        if ((e1.type == 2) && (e1.id == 0) && (e2.type == 2) && (e2.id == 0))
        {
            fprintf (stdout, MSG_WARN_IDXS_FLOAT, line_num+1);

            add_instr("F2I\n");
            add_instr("SET_P aux_var\n");
            add_instr("F2I\n");
            add_instr("MLT   %s_arr_size\n", v_table[id].name);
            add_instr("ADD   aux_var\n");
            add_instr( "%s   %s\n", ldv    , v_table[id].name);
        }

        // float in acc and float in memory
        if ((e1.type == 2) && (e1.id == 0) && (e2.type == 2) && (e2.id != 0))
        {
            fprintf (stdout, MSG_WARN_IDXS_MESS, line_num+1);

            add_instr("F2I\n");
            add_instr("MLT     %s_arr_size\n", v_table[id].name);
            add_instr("P_F2I_M %s\n" , v_table[e2.id].name);
            add_instr("S_ADD\n");
            add_instr( "%s     %s\n" , ldv   , v_table[id].name);
        }

        // float in acc and comp const in memory
        if ((e1.type == 2) && (e1.id == 0) && (e2.type == 5) && (e2.id != 0))
        {
            fprintf (stdout, MSG_WARN_IDXS_MESS, line_num+1);

            get_cmp_cst(e2, &etr, &eti);

            add_instr("F2I\n");
            add_instr("MLT %s_arr_size\n"   , v_table[id].name);
            add_instr("P_F2I_M %s\n", v_table[etr.id].name);
            add_instr("S_ADD\n");
            add_instr("%s %s\n", ldv        , v_table[id].name);
        }

        // float in acc and comp in acc
        if ((e1.type == 2) && (e1.id == 0) && (e2.type == 3) && (e2.id == 0))
        {
            fprintf (stdout, MSG_WARN_IDXS_MESS, line_num+1);

            add_instr("SET_P aux_var\n");
            add_instr("F2I\n");
            add_instr("SET_P aux_var\n");
            add_instr("F2I\n");
            add_instr("MLT   %s_arr_size\n", v_table[id].name);
            add_instr("ADD   aux_var\n");
            add_instr( "%s   %s\n", ldv    , v_table[id].name);
        }

        // float in acc and comp in memory
        if ((e1.type == 2) && (e1.id == 0) && (e2.type == 3) && (e2.id != 0))
        {
            fprintf (stdout, MSG_WARN_IDXS_MESS, line_num+1);

            add_instr("F2I\n");
            add_instr("MLT     %s_arr_size\n", v_table[id].name);
            add_instr("P_F2I_M %s\n",  v_table[e2.id].name);
            add_instr("S_ADD\n");
            add_instr( "%s     %s\n", ldv,     v_table[id].name);
        }

        // float in memory and int in acc
        if ((e1.type == 2) && (e1.id != 0) && (e2.type == 1) && (e2.id == 0))
        {
            fprintf (stdout, MSG_WARN_IDX1_FLOAT, line_num+1);

            add_instr("SET   aux_var\n");
            add_instr("F2I_M %s\n" ,   v_table[e1.id].name);
            add_instr("MLT   %s_arr_size\n", v_table[id].name);
            add_instr("ADD   aux_var\n");
            add_instr( "%s   %s\n" , ldv   , v_table[id].name);
        }

        // float in memory and int in memory
        if ((e1.type == 2) && (e1.id != 0) && (e2.type == 1) && (e2.id != 0))
        {
            fprintf (stdout, MSG_WARN_IDX1_FLOAT, line_num+1);

            add_instr( "%s %s\n", f2i, v_table[e1.id].name);
            add_instr("MLT %s_arr_size\n"    , v_table[id].name);
            add_instr("ADD %s\n",      v_table[e2.id].name);
            add_instr( "%s %s\n", ldv        , v_table[id].name);
        }

        // float in memory and float in acc
        if ((e1.type == 2) && (e1.id != 0) && (e2.type == 2) && (e2.id == 0))
        {
            fprintf (stdout, MSG_WARN_IDXS_FLOAT, line_num+1);

            add_instr("F2I\n");
            add_instr("SET   aux_var\n");
            add_instr("F2I_M %s\n",  v_table[e1.id].name);
            add_instr("MLT   %s_arr_size\n", v_table[id].name);
            add_instr("ADD   aux_var\n");
            add_instr( "%s   %s\n", ldv    , v_table[id].name);
        }

        // float in memory and float in memory
        if ((e1.type == 2) && (e1.id != 0) && (e2.type == 2) && (e2.id != 0))
        {
            fprintf (stdout, MSG_WARN_IDXS_MESS, line_num+1);

            add_instr( "%s     %s\n", f2i, v_table[e1.id].name);
            add_instr("MLT     %s_arr_size\n"    , v_table[id].name);
            add_instr("P_F2I_M %s\n",      v_table[e2.id].name);
            add_instr("S_ADD\n");
            add_instr( "%s     %s\n", ldv        , v_table[id].name);
        }

        // float in memory and comp const in memory
        if ((e1.type == 2) && (e1.id != 0) && (e2.type == 5) && (e2.id != 0))
        {
            fprintf (stdout, MSG_WARN_IDXS_MESS, line_num+1);

            get_cmp_cst(e2, &etr, &eti);

            add_instr( "%s     %s\n", f2i, v_table[e1.id].name);
            add_instr("MLT     %s_arr_size\n"    , v_table[id].name);
            add_instr("P_F2I_M %s\n",      v_table[etr.id].name);
            add_instr("S_ADD\n");
            add_instr( "%s     %s\n", ldv        , v_table[id].name);
        }

        // float in memory and comp in acc
        if ((e1.type == 2) && (e1.id != 0) && (e2.type == 3) && (e2.id == 0))
        {
            fprintf (stdout, MSG_WARN_IDXS_MESS, line_num+1);

            add_instr("SET_P aux_var\n");
            add_instr("F2I\n");
            add_instr("SET   aux_var\n");
            add_instr("F2I_M %s \n",   v_table[e1.id].name);
            add_instr("MLT   %s_arr_size\n", v_table[id].name);
            add_instr("ADD   aux_var\n");
            add_instr( "%s   %s\n", ldv     , v_table[id].name);
        }

        // float in memory and comp in memory
        if ((e1.type == 2) && (e1.id != 0) && (e2.type == 3) && (e2.id != 0))
        {
            fprintf (stdout, MSG_WARN_IDXS_MESS, line_num+1);

            add_instr( "%s     %s\n", f2i, v_table[e1.id].name);
            add_instr("MLT     %s_arr_size\n"    , v_table[id].name);
            add_instr("P_F2I_M %s\n",      v_table[e2.id].name);
            add_instr("S_ADD\n");
            add_instr( "%s     %s\n", ldv        , v_table[id].name);
        }

        // comp const in memory and int in acc
        if ((e1.type == 5) && (e1.id != 0) && (e2.type == 1) && (e2.id == 0))
        {
            fprintf (stdout, MSG_WARN_IDX_COMP_GRAB, line_num+1);

            get_cmp_cst(e1, &etr, &eti);

            add_instr("F2I\n");
            add_instr("SET   aux_var\n");
            add_instr("F2I_M %s\n",  v_table[etr.id].name);
            add_instr("MLT   %s_arr_size\n", v_table[id].name);
            add_instr("ADD   aux_var\n");
            add_instr( "%s   %s\n", ldv    , v_table[id].name);
        }

        // comp const in memory and int in memory
        if ((e1.type == 5) && (e1.id != 0) && (e2.type == 1) && (e2.id != 0))
        {
            fprintf (stdout, MSG_WARN_IDX_COMP_GRAB, line_num+1);

            get_cmp_cst(e1, &etr, &eti);

            add_instr( "%s %s\n", f2i, v_table[etr.id].name);
            add_instr("MLT %s_arr_size\n"    , v_table[id].name);
            add_instr("ADD %s\n",      v_table[e2.id].name);
            add_instr( "%s %s\n", ldv        , v_table[id].name);
        }

        // comp const in memory and float in acc
        if ((e1.type == 5) && (e1.id != 0) && (e2.type == 2) && (e2.id == 0))
        {
            fprintf (stdout, MSG_WARN_IDXS_MESS, line_num+1);

            get_cmp_cst(e1, &etr, &eti);

            add_instr("F2I\n");
            add_instr("SET   aux_var\n");
            add_instr("F2I_M %s\n",  v_table[etr.id].name);
            add_instr("MLT   %s_arr_size\n", v_table[id].name);
            add_instr("ADD   aux_var\n");
            add_instr( "%s   %s\n", ldv    , v_table[id].name);
        }

        // comp const in memory and float in memory
        if ((e1.type == 5) && (e1.id != 0) && (e2.type == 2) && (e2.id != 0))
        {
            fprintf (stdout, MSG_WARN_IDXS_MESS, line_num+1);

            get_cmp_cst(e1, &etr, &eti);

            add_instr( "%s     %s\n", f2i, v_table[etr.id].name);
            add_instr("MLT     %s_arr_size\n"    , v_table[id].name);
            add_instr("P_F2I_M %s\n",      v_table[e2.id].name);
            add_instr("S_ADD\n");
            add_instr( "%s     %s\n", ldv        , v_table[id].name);
        }

        // comp const in memory and comp const in memory
        if ((e1.type == 5) && (e1.id != 0) && (e2.type == 5) && (e2.id != 0))
        {
            fprintf (stdout, MSG_WARN_IDXS_MESS, line_num+1);

            get_cmp_cst(e1, &etr, &eti);
            add_instr("%s %s\n", f2i, v_table[etr.id].name);
            add_instr("MLT %s_arr_size\n"   , v_table[id].name);

            get_cmp_cst(e2, &etr, &eti);
            add_instr("P_F2I_M %s\n", v_table[etr.id].name);
            add_instr("S_ADD\n");
            add_instr( "%s     %s\n", ldv   , v_table[id].name);
        }

        // comp const in memory and comp in acc
        if ((e1.type == 5) && (e1.id != 0) && (e2.type == 3) && (e2.id == 0))
        {
            fprintf (stdout, MSG_WARN_IDXS_MESS, line_num+1);

            get_cmp_cst(e1, &etr, &eti);

            add_instr("SET_P aux_var\n");
            add_instr("F2I\n");
            add_instr("SET   aux_var\n");
            add_instr("F2I_M %s\n",  v_table[etr.id].name);
            add_instr("MLT   %s_arr_size\n", v_table[id].name);
            add_instr("ADD   aux_var\n");
            add_instr( "%s   %s\n", ldv    , v_table[id].name);
        }

        // comp const in memory and comp in memory
        if ((e1.type == 5) && (e1.id != 0) && (e2.type == 3) && (e2.id != 0))
        {
            fprintf (stdout, MSG_WARN_IDXS_MESS, line_num+1);

            get_cmp_cst(e1, &etr, &eti);

            add_instr( "%s     %s\n", f2i, v_table[etr.id].name);
            add_instr("MLT     %s_arr_size\n"    , v_table[id].name);
            add_instr("P_F2I_M %s\n",      v_table[e2.id].name);
            add_instr("S_ADD\n");
            add_instr( "%s     %s\n", ldv        , v_table[id].name);
        }

        // comp in acc and int in acc
        if ((e1.type == 3) && (e1.id == 0) && (e2.type == 1) && (e2.id == 0))
        {
            fprintf (stdout, MSG_WARN_IDX_COMP_GRAB, line_num+1);

            add_instr("SET_P aux_var\n");
            add_instr("POP\n");
            add_instr("F2I\n");
            add_instr("MLT   %s_arr_size\n", v_table[id].name);
            add_instr("ADD   aux_var\n");
            add_instr( "%s   %s\n", ldv    , v_table[id].name);
        }

        // comp in acc and int in memory
        if ((e1.type == 3) && (e1.id == 0) && (e2.type == 1) && (e2.id != 0))
        {
            fprintf (stdout, MSG_WARN_IDX_COMP_GRAB, line_num+1);

            add_instr("POP\n");
            add_instr("F2I\n");
            add_instr("MLT %s_arr_size\n", v_table[id].name);
            add_instr("ADD %s\n",  v_table[e2.id].name);
            add_instr( "%s %s\n",  ldv   , v_table[id].name);
        }

        // comp in acc and float in acc
        if ((e1.type == 3) && (e1.id == 0) && (e2.type == 2) && (e2.id == 0))
        {
            fprintf (stdout, MSG_WARN_IDXS_MESS, line_num+1);

            add_instr("F2I\n");
            add_instr("SET_P aux_var\n");
            add_instr("POP\n");
            add_instr("F2I\n");
            add_instr("MLT  %s_arr_size\n", v_table[id].name);
            add_instr("ADD  aux_var\n");
            add_instr( "%s  %s\n", ldv    , v_table[id].name);
        }

        // comp in acc and float in memory
        if ((e1.type == 3) && (e1.id == 0) && (e2.type == 2) && (e2.id != 0))
        {
            fprintf (stdout, MSG_WARN_IDXS_MESS, line_num+1);

            add_instr("POP\n");
            add_instr("F2I\n");
            add_instr("MLT     %s_arr_size\n", v_table[id].name);
            add_instr("P_F2I_M %s\n",  v_table[e2.id].name);
            add_instr("S_ADD\n");
            add_instr( "%s     %s\n",  ldv   , v_table[id].name);
        }

        // comp in acc and comp const in memory
        if ((e1.type == 3) && (e1.id == 0) && (e2.type == 5) && (e2.id != 0))
        {
            fprintf (stdout, MSG_WARN_IDXS_MESS, line_num+1);

            get_cmp_cst(e2, &etr, &eti);

            add_instr("POP\n");
            add_instr("F2I\n");
            add_instr("MLT     %s_arr_size\n", v_table[id].name);
            add_instr("P_F2I_M %s\n",  v_table[etr.id].name);
            add_instr("S_ADD\n");
            add_instr( "%s     %s\n",  ldv   , v_table[id].name);
        }

        // comp in acc and comp in acc
        if ((e1.type == 3) && (e1.id == 0) && (e2.type == 3) && (e2.id == 0))
        {
            fprintf (stdout, MSG_WARN_IDXS_MESS, line_num+1);

            add_instr("POP\n");
            add_instr("F2I\n");
            add_instr("SET_P aux_var\n");
            add_instr("POP\n");
            add_instr("F2I\n");
            add_instr("MLT   %s_arr_size\n", v_table[id].name);
            add_instr("ADD   aux_var\n");
            add_instr( "%s   %s\n", ldv    , v_table[id].name);
        }

        // comp in acc and comp in memory
        if ((e1.type == 3) && (e1.id == 0) && (e2.type == 3) && (e2.id != 0))
        {
            fprintf (stdout, MSG_WARN_IDXS_MESS, line_num+1);

            add_instr("POP\n");
            add_instr("F2I\n");
            add_instr("MLT     %s_arr_size\n", v_table[id].name);
            add_instr("P_F2I_M %s\n",  v_table[e2.id].name);
            add_instr("S_ADD\n");
            add_instr( "%s     %s\n",  ldv   , v_table[id].name);
        }

        // comp in memory and int in acc
        if ((e1.type == 3) && (e1.id != 0) && (e2.type == 1) && (e2.id == 0))
        {
            fprintf (stdout, MSG_WARN_IDX_COMP_GRAB, line_num+1);

            add_instr("P_F2I_M %s\n",  v_table[e1.id].name);
            add_instr("MLT     %s_arr_size\n", v_table[id].name);
            add_instr("S_ADD\n");
            add_instr( "%s     %s\n",  ldv   , v_table[id].name);
        }

        // comp in memory and int in memory
        if ((e1.type == 3) && (e1.id != 0) && (e2.type == 1) && (e2.id != 0))
        {
            fprintf (stdout, MSG_WARN_IDX_COMP_GRAB, line_num+1);

            add_instr( "%s %s\n", f2i, v_table[e1.id].name);
            add_instr("MLT %s_arr_size\n"    , v_table[id].name);
            add_instr("ADD %s\n",      v_table[e2.id].name);
            add_instr( "%s %s\n", ldv        , v_table[id].name);
        }

        // comp in memory and float in acc
        if ((e1.type == 3) && (e1.id != 0) && (e2.type == 2) && (e2.id == 0))
        {
            fprintf (stdout, MSG_WARN_IDXS_MESS, line_num+1);

            add_instr("F2I\n");
            add_instr("P_F2I_M %s\n",  v_table[e1.id].name);
            add_instr("MLT     %s_arr_size\n", v_table[id].name);
            add_instr("S_ADD\n");
            add_instr( "%s     %s\n",  ldv   , v_table[id].name);
        }

        // comp in memory and float in memory
        if ((e1.type == 3) && (e1.id != 0) && (e2.type == 2) && (e2.id != 0))
        {
            fprintf (stdout, MSG_WARN_IDXS_MESS, line_num+1);

            add_instr("%s %s\n", f2i,  v_table[e1.id].name);
            add_instr("MLT     %s_arr_size\n", v_table[id].name);
            add_instr("P_F2I_M %s\n",  v_table[e2.id].name);
            add_instr("S_ADD\n");
            add_instr( "%s     %s\n",  ldv   , v_table[id].name);
        }

        // comp in memory and comp const in memory
        if ((e1.type == 3) && (e1.id != 0) && (e2.type == 5) && (e2.id != 0))
        {
            fprintf (stdout, MSG_WARN_IDXS_MESS, line_num+1);

            get_cmp_cst(e2, &etr, &eti);

            add_instr( "%s     %s\n", f2i, v_table[e1.id].name);
            add_instr("MLT     %s_arr_size\n"    , v_table[id].name);
            add_instr("P_F2I_M %s\n",      v_table[etr.id].name);
            add_instr("S_ADD\n");
            add_instr( "%s     %s\n", ldv        , v_table[id].name);
        }

        // comp in memory and comp in acc
        if ((e1.type == 3) && (e1.id != 0) && (e2.type == 3) && (e2.id == 0))
        {
            fprintf (stdout, MSG_WARN_IDXS_MESS, line_num+1);

            add_instr("SET_P aux_var\n");
            add_instr("F2I\n");
            add_instr("SET   aux_var\n");
            add_instr("F2I_M %s\n",  v_table[e1.id].name);
            add_instr("MLT   %s_arr_size\n", v_table[id].name);
            add_instr("ADD   aux_var\n");
            add_instr( "%s   %s\n",  ldv   , v_table[id].name);
        }

        // comp in memory and comp in memory
        if ((e1.type == 3) && (e1.id != 0) && (e2.type == 3) && (e2.id != 0))
        {
            fprintf (stdout, MSG_WARN_IDXS_MESS, line_num+1);

            add_instr("%s      %s\n", f2i, v_table[e1.id].name);
            add_instr("MLT     %s_arr_size\n"    , v_table[id].name);
            add_instr("P_F2I_M %s\n",      v_table[e2.id].name);
            add_instr("S_ADD\n");
            add_instr ("%s     %s\n", ldv        , v_table[id].name);
        }
    }

    // left comp --------------------------------------------------------------

    else
    {
        // int in acc and int in acc
        if ((e1.type == 1) && (e1.id == 0) && (e2.type == 1) && (e2.id == 0))
        {
            add_instr("SET_P aux_var\n");
            add_instr("MLT   %s_arr_size\n", v_table[id].name);
            add_instr("ADD   aux_var\n");
            add_instr("SET   aux_var\n");
            add_instr( "%s   %s\n"  , ldv  , v_table[id].name);
            add_instr("P_LOD aux_var\n");
            add_instr( "%s   %s_i\n", ldv  , v_table[id].name);
        }

        // int in acc and int in memory
        if ((e1.type == 1) && (e1.id == 0) && (e2.type == 1) && (e2.id != 0))
        {
            add_instr("MLT   %s_arr_size\n" , v_table[id].name);
            add_instr("ADD   %s\n"  , v_table[e2.id].name);
            add_instr("SET   aux_var\n");
            add_instr( "%s   %s\n"  , ldv   , v_table[id].name);
            add_instr("P_LOD aux_var\n");
            add_instr( "%s   %s_i\n", ldv   , v_table[id].name);
        }

        // int in acc and float in acc
        if ((e1.type == 1) && (e1.id == 0) && (e2.type == 2) && (e2.id == 0))
        {
            fprintf (stdout, MSG_WARN_IDX2_FLOAT, line_num+1);

            add_instr("F2I\n");
            add_instr("SET_P aux_var\n");
            add_instr("MLT   %s_arr_size\n", v_table[id].name);
            add_instr("ADD   aux_var\n");
            add_instr("SET   aux_var\n");
            add_instr( "%s   %s\n"  , ldv  , v_table[id].name);
            add_instr("P_LOD aux_var\n");
            add_instr( "%s   %s_i\n", ldv  , v_table[id].name);
        }

        // int in acc and float in memory
        if ((e1.type == 1) && (e1.id == 0) && (e2.type == 2) && (e2.id != 0))
        {
            fprintf (stdout, MSG_WARN_IDX2_FLOAT, line_num+1);

            add_instr("MLT     %s_arr_size\n", v_table[id].name);
            add_instr("P_F2I_M %s\n" , v_table[e2.id].name);
            add_instr("S_ADD\n");
            add_instr("SET     aux_var\n");
            add_instr( "%s     %s\n"  , ldv  , v_table[id].name);
            add_instr("P_LOD   aux_var\n");
            add_instr( "%s     %s_i\n", ldv  , v_table[id].name);
        }

        // int in acc and comp const in memory
        if ((e1.type == 1) && (e1.id == 0) && (e2.type == 5) && (e2.id != 0))
        {
            fprintf (stdout, MSG_WARN_IDX_COMP_GRAB, line_num+1);

            get_cmp_cst(e2, &etr, &eti);

            add_instr("MLT     %s_arr_size\n", v_table[id].name);
            add_instr("P_F2I_M %s\n" , v_table[etr.id].name);
            add_instr("S_ADD\n");
            add_instr("SET     aux_var\n");
            add_instr( "%s     %s\n"  , ldv  , v_table[id].name);
            add_instr("P_LOD   aux_var\n");
            add_instr( "%s     %s_i\n", ldv  , v_table[id].name);
        }

        // int in acc and comp in acc
        if ((e1.type == 1) && (e1.id == 0) && (e2.type == 3) && (e2.id == 0))
        {
            fprintf (stdout, MSG_WARN_IDX_COMP_GRAB, line_num+1);

            add_instr("SET_P aux_var\n");
            add_instr("F2I\n");
            add_instr("SET_P aux_var\n");
            add_instr("MLT   %s_arr_size\n", v_table[id].name);
            add_instr("ADD   aux_var\n");
            add_instr("SET   aux_var\n");
            add_instr( "%s   %s\n"  , ldv  , v_table[id].name);
            add_instr("P_LOD aux_var\n");
            add_instr( "%s   %s_i\n", ldv  , v_table[id].name);
        }

        // int in acc and comp in memory
        if ((e1.type == 1) && (e1.id == 0) && (e2.type == 3) && (e2.id != 0))
        {
            fprintf (stdout, MSG_WARN_IDX_COMP_GRAB, line_num+1);

            add_instr("MLT     %s_arr_size\n", v_table[id].name);
            add_instr("P_F2I_M %s\n",  v_table[e2.id].name);
            add_instr("S_ADD\n");
            add_instr("SET     aux_var\n");
            add_instr( "%s     %s\n"  , ldv  , v_table[id].name);
            add_instr("P_LOD   aux_var\n");
            add_instr( "%s     %s_i\n", ldv  , v_table[id].name);
        }

        // int in memory and int in acc
        if ((e1.type == 1) && (e1.id != 0) && (e2.type == 1) && (e2.id == 0))
        {
            add_instr("P_LOD %s\n",  v_table[e1.id].name);
            add_instr("MLT   %s_arr_size\n", v_table[id].name);
            add_instr("S_ADD\n");
            add_instr("SET   aux_var\n");
            add_instr( "%s   %s\n"  , ldv  , v_table[id].name);
            add_instr("P_LOD aux_var\n");
            add_instr( "%s   %s_i\n", ldv  , v_table[id].name);
        }

        // int in memory and int in memory
        if ((e1.type == 1) && (e1.id != 0) && (e2.type == 1) && (e2.id != 0))
        {
            add_instr( "%s   %s\n", ldi, v_table[e1.id].name);
            add_instr("MLT   %s_arr_size\n"    , v_table[id].name);
            add_instr("ADD   %s\n",      v_table[e2.id].name);
            add_instr("SET   aux_var\n");
            add_instr( "%s   %s\n"  , ldv      , v_table[id].name);
            add_instr("P_LOD aux_var\n");
            add_instr( "%s   %s_i\n", ldv      , v_table[id].name);
        }

        // int in memory and float in acc
        if ((e1.type == 1) && (e1.id != 0) && (e2.type == 2) && (e2.id == 0))
        {
            fprintf (stdout, MSG_WARN_IDX2_FLOAT, line_num+1);

            add_instr("F2I\n");
            add_instr("P_LOD %s\n",  v_table[e1.id].name);
            add_instr("MLT   %s_arr_size\n", v_table[id].name);
            add_instr("S_ADD\n");
            add_instr("SET   aux_var\n");
            add_instr( "%s   %s\n"  , ldv  , v_table[id].name);
            add_instr("P_LOD aux_var\n");
            add_instr( "%s   %s_i\n", ldv  , v_table[id].name);
        }

        // int in memory and float in memory
        if ((e1.type == 1) && (e1.id != 0) && (e2.type == 2) && (e2.id != 0))
        {
            fprintf (stdout, MSG_WARN_IDX2_FLOAT, line_num+1);

            add_instr( "%s     %s\n", ldi, v_table[e1.id].name);
            add_instr("MLT     %s_arr_size\n"    , v_table[id].name);
            add_instr("P_F2I_M %s\n"     , v_table[e2.id].name);
            add_instr("S_ADD\n");
            add_instr("SET     aux_var\n");
            add_instr( "%s     %s\n"  , ldv      , v_table[id].name);
            add_instr("P_LOD   aux_var\n");
            add_instr( "%s     %s_i\n", ldv      , v_table[id].name);
        }

        // int in memory and comp const in memory
        if ((e1.type == 1) && (e1.id != 0) && (e2.type == 5) && (e2.id != 0))
        {
            fprintf (stdout, MSG_WARN_IDX_COMP_GRAB, line_num+1);

            get_cmp_cst(e2, &etr, &eti);

            add_instr( "%s     %s\n", ldi, v_table[e1.id].name);
            add_instr("MLT     %s_arr_size\n"    , v_table[id].name);
            add_instr("P_F2I_M %s\n"     , v_table[etr.id].name);
            add_instr("S_ADD\n");
            add_instr("SET     aux_var\n");
            add_instr( "%s     %s\n"  , ldv      , v_table[id].name);
            add_instr("P_LOD   aux_var\n");
            add_instr( "%s     %s_i\n", ldv      , v_table[id].name);
        }

        // int in memory and comp in acc
        if ((e1.type == 1) && (e1.id != 0) && (e2.type == 3) && (e2.id == 0))
        {
            fprintf (stdout, MSG_WARN_IDX_COMP_GRAB, line_num+1);

            add_instr("SET_P aux_var\n");
            add_instr("F2I\n");
            add_instr("SET   aux_var\n");
            add_instr("LOD   %s\n",   v_table[e1.id].name);
            add_instr("MLT   %s_arr_size\n" , v_table[id].name);
            add_instr("ADD   aux_var\n");
            add_instr("SET   aux_var\n");
            add_instr( "%s   %s\n"  , ldv   , v_table[id].name);
            add_instr("P_LOD aux_var\n");
            add_instr( "%s   %s_i\n", ldv   , v_table[id].name);
        }

        // int in memory and comp in memory
        if ((e1.type == 1) && (e1.id != 0) && (e2.type == 3) && (e2.id != 0))
        {
            fprintf (stdout, MSG_WARN_IDX_COMP_GRAB, line_num+1);

            add_instr( "%s     %s\n", ldi, v_table[e1.id].name);
            add_instr("MLT     %s_arr_size\n"    , v_table[id].name);
            add_instr("P_F2I_M %s\n"     , v_table[e2.id].name);
            add_instr("S_ADD\n");
            add_instr("SET     aux_var\n");
            add_instr( "%s     %s\n"  , ldv      , v_table[id].name);
            add_instr("P_LOD   aux_var\n");
            add_instr( "%s     %s_i\n", ldv      , v_table[id].name);
        }

        // float in acc and int in acc
        if ((e1.type == 2) && (e1.id == 0) && (e2.type == 1) && (e2.id == 0))
        {
            fprintf (stdout, MSG_WARN_IDX1_FLOAT, line_num+1);

            add_instr("SET_P aux_var\n");
            add_instr("F2I\n");
            add_instr("MLT   %s_arr_size\n", v_table[id].name);
            add_instr("ADD   aux_var\n");
            add_instr("SET   aux_var\n");
            add_instr( "%s   %s\n"  , ldv  , v_table[id].name);
            add_instr("P_LOD aux_var\n");
            add_instr( "%s   %s_i\n", ldv  , v_table[id].name);
        }

        // float in acc and int in memory
        if ((e1.type == 2) && (e1.id == 0) && (e2.type == 1) && (e2.id != 0))
        {
            fprintf (stdout, MSG_WARN_IDX1_FLOAT, line_num+1);

            add_instr("F2I\n");
            add_instr("MLT   %s_arr_size\n", v_table[id].name);
            add_instr("ADD   %s\n" , v_table[e2.id].name);
            add_instr("SET   aux_var\n");
            add_instr( "%s   %s\n"  , ldv  , v_table[id].name);
            add_instr("P_LOD aux_var\n");
            add_instr( "%s   %s_i\n", ldv  , v_table[id].name);
        }

        // float in acc and float in acc
        if ((e1.type == 2) && (e1.id == 0) && (e2.type == 2) && (e2.id == 0))
        {
            fprintf (stdout, MSG_WARN_IDXS_FLOAT, line_num+1);

            add_instr("F2I\n");
            add_instr("SET_P aux_var\n");
            add_instr("F2I\n");
            add_instr("MLT   %s_arr_size\n", v_table[id].name);
            add_instr("ADD   aux_var\n");
            add_instr("SET   aux_var\n");
            add_instr( "%s   %s\n"  , ldv  , v_table[id].name);
            add_instr("P_LOD aux_var\n");
            add_instr( "%s   %s_i\n", ldv  , v_table[id].name);
        }

        // float in acc and float in memory
        if ((e1.type == 2) && (e1.id == 0) && (e2.type == 2) && (e2.id != 0))
        {
            fprintf (stdout, MSG_WARN_IDXS_MESS, line_num+1);

            add_instr("F2I\n");
            add_instr("MLT     %s_arr_size\n", v_table[id].name);
            add_instr("P_F2I_M %s\n" , v_table[e2.id].name);
            add_instr("S_ADD\n");
            add_instr("SET     aux_var\n");
            add_instr( "%s     %s\n"  , ldv  , v_table[id].name);
            add_instr("P_LOD   aux_var\n");
            add_instr( "%s     %s_i\n", ldv  , v_table[id].name);
        }

        // float in acc and comp const in memory
        if ((e1.type == 2) && (e1.id == 0) && (e2.type == 5) && (e2.id != 0))
        {
            fprintf (stdout, MSG_WARN_IDXS_MESS, line_num+1);

            get_cmp_cst(e2, &etr, &eti);

            add_instr("F2I\n");
            add_instr("MLT     %s_arr_size\n", v_table[id].name);
            add_instr("P_F2I_M %s\n" , v_table[etr.id].name);
            add_instr("S_ADD\n");
            add_instr("SET     aux_var\n");
            add_instr( "%s     %s\n"  , ldv  , v_table[id].name);
            add_instr("P_LOD   aux_var\n");
            add_instr( "%s     %s_i\n", ldv  , v_table[id].name);
        }

        // float in acc and comp in acc
        if ((e1.type == 2) && (e1.id == 0) && (e2.type == 3) && (e2.id == 0))
        {
            fprintf (stdout, MSG_WARN_IDXS_MESS, line_num+1);

            add_instr("SET_P aux_var\n");
            add_instr("F2I\n");
            add_instr("SET_P aux_var\n");
            add_instr("F2I\n");
            add_instr("MLT   %s_arr_size\n", v_table[id].name);
            add_instr("ADD   aux_var\n");
            add_instr("SET   aux_var\n");
            add_instr( "%s   %s\n"  , ldv  , v_table[id].name);
            add_instr("P_LOD aux_var\n");
            add_instr( "%s   %s_i\n", ldv  , v_table[id].name);
        }

        // float in acc and comp in memory
        if ((e1.type == 2) && (e1.id == 0) && (e2.type == 3) && (e2.id != 0))
        {
            fprintf (stdout, MSG_WARN_IDXS_MESS, line_num+1);

            add_instr("F2I\n");
            add_instr("MLT     %s_arr_size\n", v_table[id].name);
            add_instr("P_F2I_M %s\n" , v_table[e2.id].name);
            add_instr("S_ADD\n");
            add_instr("SET     aux_var\n");
            add_instr( "%s     %s\n"  , ldv  , v_table[id].name);
            add_instr("P_LOD   aux_var\n");
            add_instr( "%s     %s_i\n", ldv  , v_table[id].name);
        }

        // float in memory and int in acc
        if ((e1.type == 2) && (e1.id != 1) && (e2.type == 1) && (e2.id == 0))
        {
            fprintf (stdout, MSG_WARN_IDX_COMP_GRAB, line_num+1);

            add_instr("F2I\n");
            add_instr("SET   aux_var\n");
            add_instr("F2I_M %s\n" , v_table[e1.id].name);
            add_instr("MLT   %s_arr_size\n", v_table[id].name);
            add_instr("ADD   aux_var\n");
            add_instr("SET   aux_var\n");
            add_instr( "%s   %s\n"  , ldv  , v_table[id].name);
            add_instr("P_LOD aux_var\n");
            add_instr( "%s   %s_i\n", ldv  , v_table[id].name);
        }

        // float in memory and int in memory
        if ((e1.type == 2) && (e1.id != 0) && (e2.type == 1) && (e2.id != 0))
        {
            fprintf (stdout, MSG_WARN_IDX_COMP_GRAB, line_num+1);

            add_instr("%s    %s\n", f2i, v_table[e1.id].name);
            add_instr("MLT   %s_arr_size\n"    , v_table[id].name);
            add_instr("ADD   %s\n"     , v_table[e2.id].name);
            add_instr("SET   aux_var\n");
            add_instr( "%s   %s\n"  , ldv      , v_table[id].name);
            add_instr("P_LOD aux_var\n");
            add_instr( "%s   %s_i\n", ldv      , v_table[id].name);
        }

        // float in memory and float in acc
        if ((e1.type == 2) && (e1.id != 0) && (e2.type == 2) && (e2.id == 0))
        {
            fprintf (stdout, MSG_WARN_IDXS_MESS, line_num+1);

            add_instr("F2I\n");
            add_instr("SET   aux_var\n");
            add_instr("F2I_M %s\n" , v_table[e1.id].name);
            add_instr("MLT   %s_arr_size\n", v_table[id].name);
            add_instr("ADD   aux_var\n");
            add_instr("SET   aux_var\n");
            add_instr( "%s   %s\n"  , ldv  , v_table[id].name);
            add_instr("P_LOD aux_var\n");
            add_instr( "%s   %s_i\n", ldv  , v_table[id].name);
        }

        // float in memory and float in memory
        if ((e1.type == 2) && (e1.id != 0) && (e2.type == 2) && (e2.id != 0))
        {
            fprintf (stdout, MSG_WARN_IDXS_MESS, line_num+1);

            add_instr( "%s     %s\n", f2i, v_table[e1.id].name);
            add_instr("MLT     %s_arr_size\n"    , v_table[id].name);
            add_instr("P_F2I_M %s\n"     , v_table[e2.id].name);
            add_instr("S_ADD\n");
            add_instr("SET     aux_var\n");
            add_instr("%s      %s\n"  , ldv      , v_table[id].name);
            add_instr("P_LOD   aux_var\n");
            add_instr( "%s     %s_i\n", ldv      , v_table[id].name);
        }

        // float in memory and comp const in memory
        if ((e1.type == 2) && (e1.id != 0) && (e2.type == 5) && (e2.id != 0))
        {
            fprintf (stdout, MSG_WARN_IDXS_MESS, line_num+1);

            add_instr( "%s %s\n", f2i, v_table[e1.id].name);
            add_instr("MLT %s_arr_size\n"    , v_table[id].name);

            get_cmp_cst(e2, &etr, &eti);

            add_instr("P_F2I_M %s\n", v_table[etr.id].name);
            add_instr("S_ADD\n");
            add_instr("SET     aux_var\n");
            add_instr( "%s     %s\n"  , ldv , v_table[id].name);
            add_instr("P_LOD   aux_var\n");
            add_instr( "%s     %s_i\n", ldv , v_table[id].name);
        }

        // float in memory and comp in acc
        if ((e1.type == 2) && (e1.id != 0) && (e2.type == 3) && (e2.id == 0))
        {
            fprintf (stdout, MSG_WARN_IDXS_MESS, line_num+1);

            add_instr("SET_P aux_var\n");
            add_instr("F2I\n");
            add_instr("SET   aux_var\n");
            add_instr("F2I_M %s\n" , v_table[e1.id].name);
            add_instr("MLT   %s_arr_size\n", v_table[id].name);
            add_instr("ADD   aux_var\n");
            add_instr("SET   aux_var\n");
            add_instr( "%s   %s\n"  , ldv  , v_table[id].name);
            add_instr("P_LOD aux_var\n");
            add_instr( "%s   %s_i\n", ldv  , v_table[id].name);
        }

        // float in memory and comp in memory
        if ((e1.type == 2) && (e1.id != 0) && (e2.type == 3) && (e2.id != 0))
        {
            fprintf (stdout, MSG_WARN_IDXS_MESS, line_num+1);

            add_instr( "%s     %s\n", f2i, v_table[e1.id].name);
            add_instr("MLT     %s_arr_size\n"    , v_table[id].name);
            add_instr("P_F2I_M %s\n"     , v_table[e2.id].name);
            add_instr("S_ADD\n");
            add_instr("SET     aux_var\n");
            add_instr( "%s     %s\n"  , ldv      , v_table[id].name);
            add_instr("P_LOD   aux_var\n");
            add_instr( "%s     %s_i\n", ldv      , v_table[id].name);
        }

        // comp const in memory and int in acc
        if ((e1.type == 5) && (e1.id != 0) && (e2.type == 1) && (e2.id == 0))
        {
            fprintf (stdout, MSG_WARN_IDX_COMP_GRAB, line_num+1);

            get_cmp_cst(e1, &etr, &eti);

            add_instr("F2I\n");
            add_instr("SET   aux_var\n");
            add_instr("F2I_M %s\n" , v_table[etr.id].name);
            add_instr("MLT   %s_arr_size\n", v_table[id].name);
            add_instr("ADD   aux_var\n");
            add_instr("SET   aux_var\n");
            add_instr( "%s   %s\n"  , ldv  , v_table[id].name);
            add_instr("P_LOD aux_var\n");
            add_instr( "%s   %s_i\n", ldv  , v_table[id].name);
        }

        // comp const in memory and int in memory
        if ((e1.type == 5) && (e1.id != 0) && (e2.type == 1) && (e2.id != 0))
        {
            fprintf (stdout, MSG_WARN_IDX_COMP_GRAB, line_num+1);

            get_cmp_cst(e1, &etr, &eti);

            add_instr( "%s   %s\n", f2i, v_table[etr.id].name);
            add_instr("MLT   %s_arr_size\n"    , v_table[id].name);
            add_instr("ADD   %s\n",      v_table[e2.id].name);
            add_instr("SET   aux_var\n");
            add_instr( "%s   %s\n"  , ldv      , v_table[id].name);
            add_instr("P_LOD aux_var\n");
            add_instr( "%s   %s_i\n", ldv      , v_table[id].name);
        }

        // comp const in memory and float in acc
        if ((e1.type == 5) && (e1.id != 0) && (e2.type == 2) && (e2.id == 0))
        {
            fprintf (stdout, MSG_WARN_IDXS_MESS, line_num+1);

            get_cmp_cst(e1, &etr, &eti);

            add_instr("F2I\n");
            add_instr("SET   aux_var\n");
            add_instr("F2I_M %s\n",  v_table[etr.id].name);
            add_instr("MLT   %s_arr_size\n", v_table[id].name);
            add_instr("ADD   aux_var\n");
            add_instr("SET   aux_var\n");
            add_instr( "%s   %s\n"  , ldv  , v_table[id].name);
            add_instr("P_LOD aux_var\n");
            add_instr( "%s   %s_i\n", ldv  , v_table[id].name);
        }

        // comp const in memory and float in memory
        if ((e1.type == 5) && (e1.id != 0) && (e2.type == 2) && (e2.id != 0))
        {
            fprintf (stdout, MSG_WARN_IDXS_MESS, line_num+1);

            get_cmp_cst(e1, &etr, &eti);

            add_instr( "%s     %s\n", f2i, v_table[etr.id].name);
            add_instr("MLT     %s_arr_size\n"    , v_table[id].name);
            add_instr("P_F2I_M %s\n",      v_table[e2.id].name);
            add_instr("S_ADD\n");
            add_instr("SET     aux_var\n");
            add_instr( "%s     %s\n"  , ldv      , v_table[id].name);
            add_instr("P_LOD   aux_var\n");
            add_instr( "%s     %s_i\n", ldv      , v_table[id].name);
        }

        // comp const in memory and comp const in memory
        if ((e1.type == 5) && (e1.id != 0) && (e2.type == 5) && (e2.id != 0))
        {
            fprintf (stdout, MSG_WARN_IDXS_MESS, line_num+1);

            get_cmp_cst(e1, &etr, &eti);
            add_instr( "%s %s\n", f2i, v_table[etr.id].name);
            add_instr("MLT %s_arr_size\n"    , v_table[id].name);

            get_cmp_cst(e2, &etr, &eti);
            add_instr("P_F2I_M %s\n", v_table[etr.id].name);
            add_instr("S_ADD\n");
            add_instr("SET     aux_var\n");
            add_instr( "%s     %s\n"  , ldv , v_table[id].name);
            add_instr("P_LOD   aux_var\n");
            add_instr( "%s     %s_i\n", ldv , v_table[id].name);
        }

        // comp const in memory and comp in acc
        if ((e1.type == 5) && (e1.id != 0) && (e2.type == 3) && (e2.id == 0))
        {
            fprintf (stdout, MSG_WARN_IDXS_MESS, line_num+1);

            get_cmp_cst(e1, &etr, &eti);

            add_instr("SET_P aux_var\n");
            add_instr("F2I\n");
            add_instr("SET   aux_var\n");
            add_instr("F2I_M %s\n",  v_table[etr.id].name);
            add_instr("MLT   %s_arr_size\n", v_table[id].name);
            add_instr("ADD   aux_var\n");
            add_instr("SET   aux_var\n");
            add_instr( "%s   %s\n"  , ldv  , v_table[id].name);
            add_instr("P_LOD aux_var\n");
            add_instr( "%s   %s_i\n", ldv  , v_table[id].name);
        }

        // comp const in memory and comp in memory
        if ((e1.type == 5) && (e1.id != 0) && (e2.type == 3) && (e2.id != 0))
        {
            fprintf (stdout, MSG_WARN_IDXS_MESS, line_num+1);

            get_cmp_cst(e1, &etr, &eti);

            add_instr("%s      %s\n", f2i, v_table[etr.id].name);
            add_instr("MLT     %s_arr_size\n"    , v_table[id].name);
            add_instr("P_F2I_M %s\n"     , v_table[e2.id].name);
            add_instr("S_ADD\n");
            add_instr("SET     aux_var\n");
            add_instr( "%s     %s\n"  , ldv      , v_table[id].name);
            add_instr("P_LOD   aux_var\n");
            add_instr( "%s     %s_i\n", ldv      , v_table[id].name);
        }

        // comp in acc and int in acc
        if ((e1.type == 3) && (e1.id == 0) && (e2.type == 1) && (e2.id == 0))
        {
            fprintf (stdout, MSG_WARN_IDX_COMP_GRAB, line_num+1);

            add_instr("SET_P aux_var\n");
            add_instr("POP\n");
            add_instr("F2I\n");
            add_instr("MLT   %s_arr_size\n", v_table[id].name);
            add_instr("ADD   aux_var\n");
            add_instr("SET   aux_var\n");
            add_instr( "%s   %s\n"  , ldv  , v_table[id].name);
            add_instr("P_LOD aux_var\n");
            add_instr( "%s   %s_i\n", ldv  , v_table[id].name);
        }

        // comp in acc and int in memory
        if ((e1.type == 3) && (e1.id == 0) && (e2.type == 1) && (e2.id != 0))
        {
            fprintf (stdout, MSG_WARN_IDX_COMP_GRAB, line_num+1);

            add_instr("SET_P aux_var\n");
            add_instr("F2I\n");
            add_instr("MLT   %s_arr_size\n", v_table[id].name);
            add_instr("ADD   %s\n" , v_table[e2.id].name);
            add_instr("SET   aux_var\n");
            add_instr( "%s   %s\n"  , ldv  , v_table[id].name);
            add_instr("P_LOD aux_var\n");
            add_instr( "%s   %s_i\n", ldv  , v_table[id].name);
        }

        // comp in acc and float in acc
        if ((e1.type == 3) && (e1.id == 0) && (e2.type == 2) && (e2.id == 0))
        {
            fprintf (stdout, MSG_WARN_IDXS_MESS, line_num+1);

            add_instr("F2I\n");
            add_instr("SET_P aux_var\n");
            add_instr("POP\n");
            add_instr("F2I\n");
            add_instr("MLT   %s_arr_size\n", v_table[id].name);
            add_instr("ADD   aux_var\n");
            add_instr("SET   aux_var\n");
            add_instr("%s    %s\n"  , ldv  , v_table[id].name);
            add_instr("P_LOD aux_var\n");
            add_instr("%s    %s_i\n", ldv  , v_table[id].name);
        }

        // comp in acc and float in memory
        if ((e1.type == 3) && (e1.id == 0) && (e2.type == 2) && (e2.id != 0))
        {
            fprintf (stdout, MSG_WARN_IDXS_MESS, line_num+1);

            add_instr("SET_P   aux_var\n");
            add_instr("F2I\n");
            add_instr("MLT     %s_arr_size\n", v_table[id].name);
            add_instr("P_F2I_M %s\n" , v_table[e2.id].name);
            add_instr("S_ADD\n");
            add_instr("SET     aux_var\n");
            add_instr ("%s     %s\n"  , ldv  , v_table[id].name);
            add_instr("P_LOD   aux_var\n");
            add_instr( "%s     %s_i\n", ldv  , v_table[id].name);
        }

        // comp in acc and comp const in memory
        if ((e1.type == 3) && (e1.id == 0) && (e2.type == 5) && (e2.id != 0))
        {
            fprintf (stdout, MSG_WARN_IDXS_MESS, line_num+1);

            get_cmp_cst(e2, &etr, &eti);

            add_instr("SET_P   aux_var\n");
            add_instr("F2I\n");
            add_instr("MLT     %s_arr_size\n", v_table[id].name);
            add_instr("P_F2I_M %s\n" , v_table[etr.id].name);
            add_instr("S_ADD\n");
            add_instr("SET     aux_var\n");
            add_instr( "%s     %s\n"  , ldv  , v_table[id].name);
            add_instr("P_LOD   aux_var\n");
            add_instr( "%s     %s_i\n", ldv  , v_table[id].name);
        }

        // comp in acc and comp in acc
        if ((e1.type == 3) && (e1.id == 0) && (e2.type == 3) && (e2.id == 0))
        {
            fprintf (stdout, MSG_WARN_IDXS_MESS, line_num+1);

            add_instr("SET_P aux_var\n");
            add_instr("F2I\n");
            add_instr("SET_P aux_var\n");
            add_instr("POP\n");
            add_instr("F2I\n");
            add_instr("MLT   %s_arr_size\n", v_table[id].name);
            add_instr("ADD   aux_var\n");
            add_instr("SET   aux_var\n");
            add_instr( "%s   %s\n"  , ldv  , v_table[id].name);
            add_instr("P_LOD aux_var\n");
            add_instr( "%s   %s_i\n", ldv  , v_table[id].name);
        }

        // comp in acc and comp in memory
        if ((e1.type == 3) && (e1.id == 0) && (e2.type == 3) && (e2.id != 0))
        {
            fprintf (stdout, MSG_WARN_IDXS_MESS, line_num+1);

            add_instr("SET_P   aux_var\n");
            add_instr("F2I\n");
            add_instr("MLT     %s_arr_size\n", v_table[id].name);
            add_instr("P_F2I_M %s\n" , v_table[e2.id].name);
            add_instr("S_ADD\n");
            add_instr("SET     aux_var\n");
            add_instr( "%s     %s\n"  , ldv  , v_table[id].name);
            add_instr("P_LOD   aux_var\n");
            add_instr( "%s     %s_i\n", ldv  , v_table[id].name);
        }

        // comp in memory and int in acc
        if ((e1.type == 3) && (e1.id != 0) && (e2.type == 1) && (e2.id == 0))
        {
            fprintf (stdout, MSG_WARN_IDX_COMP_GRAB, line_num+1);

            add_instr("P_F2I_M %s\n" , v_table[e1.id].name);
            add_instr("MLT     %s_arr_size\n", v_table[id].name);
            add_instr("S_ADD\n");
            add_instr("SET     aux_var\n");
            add_instr("%s      %s\n"  , ldv  , v_table[id].name);
            add_instr("P_LOD   aux_var\n");
            add_instr("%s      %s_i\n", ldv  , v_table[id].name);
        }

        // comp in memory and int in memory
        if ((e1.type == 3) && (e1.id != 0) && (e2.type == 1) && (e2.id != 0))
        {
            fprintf (stdout, MSG_WARN_IDX_COMP_GRAB, line_num+1);

            add_instr("%s    %s\n", f2i, v_table[e1.id].name);
            add_instr("MLT   %s_arr_size\n"    , v_table[id].name);
            add_instr("ADD   %s\n",      v_table[e2.id].name);
            add_instr("SET   aux_var\n");
            add_instr( "%s   %s\n"  , ldv      , v_table[id].name);
            add_instr("P_LOD aux_var\n");
            add_instr( "%s   %s_i\n", ldv      , v_table[id].name);
        }

        // comp in memory and float in acc
        if ((e1.type == 3) && (e1.id != 0) && (e2.type == 2) && (e2.id == 0))
        {
            fprintf (stdout, MSG_WARN_IDXS_MESS, line_num+1);

            add_instr("F2I\n");
            add_instr("P_F2I_M %s\n" , v_table[e1.id].name);
            add_instr("MLT     %s_arr_size\n", v_table[id].name);
            add_instr("S_ADD\n");
            add_instr("SET     aux_var\n");
            add_instr( "%s     %s\n"  , ldv  , v_table[id].name);
            add_instr("P_LOD   aux_var\n");
            add_instr( "%s     %s_i\n", ldv  , v_table[id].name);
        }

        // comp in memory and float in memory
        if ((e1.type == 3) && (e1.id != 0) && (e2.type == 2) && (e2.id != 0))
        {
            fprintf (stdout, MSG_WARN_IDXS_MESS, line_num+1);

            add_instr( "%s     %s\n", f2i, v_table[e1.id].name);
            add_instr("MLT     %s_arr_size\n"    , v_table[id].name);
            add_instr("P_F2I_M %s\n",      v_table[e2.id].name);
            add_instr("S_ADD\n");
            add_instr("SET     aux_var\n");
            add_instr( "%s     %s\n"  , ldv      , v_table[id].name);
            add_instr("P_LOD   aux_var\n");
            add_instr( "%s     %s_i\n", ldv      , v_table[id].name);
        }

        // comp in memory and comp const in memory
        if ((e1.type == 3) && (e1.id != 0) && (e2.type == 5) && (e2.id != 0))
        {
            fprintf (stdout, MSG_WARN_IDXS_MESS, line_num+1);

            get_cmp_cst(e2, &etr, &eti);

            add_instr( "%s     %s\n", f2i, v_table[e1.id].name);
            add_instr("MLT     %s_arr_size\n"    , v_table[id].name);
            add_instr("P_F2I_M %s\n",      v_table[etr.id].name);
            add_instr("S_ADD\n");
            add_instr("SET     aux_var\n");
            add_instr( "%s     %s\n"  , ldv      , v_table[id].name);
            add_instr("P_LOD   aux_var\n");
            add_instr( "%s     %s_i\n", ldv      , v_table[id].name);
        }

        // comp in memory and comp in acc
        if ((e1.type == 3) && (e1.id != 0) && (e2.type == 3) && (e2.id == 0))
        {
            fprintf (stdout, MSG_WARN_IDXS_MESS, line_num+1);

            add_instr("SET_P aux_var\n");
            add_instr("F2I\n");
            add_instr("SET   aux_var\n");
            add_instr("F2I_M %s\n" , v_table[e1.id].name);
            add_instr("MLT   %s_arr_size\n", v_table[id].name);
            add_instr("ADD   aux_var\n");
            add_instr("SET   aux_var\n");
            add_instr( "%s   %s\n", ldv    , v_table[id].name);
            add_instr("P_LOD aux_var\n");
            add_instr( "%s %s_i\n", ldv    , v_table[id].name);
        }

        // comp in memory and comp in memory
        if ((e1.type == 3) && (e1.id != 0) && (e2.type == 3) && (e2.id != 0))
        {
            fprintf (stdout, MSG_WARN_IDXS_MESS, line_num+1);

            add_instr(" %s     %s\n", f2i, v_table[e1.id].name);
            add_instr("MLT     %s_arr_size\n"    , v_table[id].name);
            add_instr("P_F2I_M %s\n"     , v_table[e2.id].name);
            add_instr("S_ADD\n");
            add_instr("SET     aux_var\n");
            add_instr(" %s     %s\n"  , ldv      , v_table[id].name);
            add_instr("P_LOD   aux_var\n");
            add_instr(" %s     %s_i\n", ldv      , v_table[id].name);
        }
    }

    acc_ok     = 1;
    v_table[id].used = 1;

    return expr_make(v_table[id].type, 0);
}