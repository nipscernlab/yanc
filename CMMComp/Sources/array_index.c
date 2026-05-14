// ----------------------------------------------------------------------------
// array index handling -------------------------------------------------------
// ----------------------------------------------------------------------------

/*
TODO:
1- usar instrucao virtual para evitar enderecamento idireto quando indice eh constante
2- AST vai economizar muito codigo em array 2D
*/

// global includes
#include  <stdio.h>
#include <string.h>
#include <stdlib.h>

// local includes
#include "..\Headers\t2t.h"
#include "..\Headers\global.h"
#include "..\Headers\funcoes.h"
#include "..\Headers\data_use.h"
#include "..\Headers\variaveis.h"
#include "..\Headers\messages.h"

// ----------------------------------------------------------------------------
// array in assignment, e.g. x[i] = y; ----------------------------------------
// ----------------------------------------------------------------------------

// loads the array index into the accumulator (1D array)
void arr_1d_index(int id, int et)
{
    // ------------------------------------------------------------------------
    // argument check ---------------------------------------------------------
    // ------------------------------------------------------------------------

    // must check that it really is an array
    if (v_isar[id] == 0)
        {fprintf (stderr, MSG_ERR_NOT_ARRAY_HARSH, line_num+1, rem_fname(v_name[id], fname)); exit(EXIT_FAILURE);}

    // must check that it really is a 1D array
    if (v_isar[id] == 2)
        {fprintf (stderr, MSG_ERR_ARRAY_2D  , line_num+1, rem_fname(v_name[id], fname)); exit(EXIT_FAILURE);}

    // ------------------------------------------------------------------------
    // execute ----------------------------------------------------------------
    // ------------------------------------------------------------------------

    int etr, eti;

    // when int in memory -------------------------------------------------

    if ((get_type(et) == 1) && (et % OFST != 0))
    {
        add_instr("LOD %s\n", v_name[et % OFST]);
    }

    // when int in acc ----------------------------------------------------

    if ((get_type(et) == 1) && (et % OFST == 0))
    {
        // nothing to do
    }

    // when float in memory -----------------------------------------

    if ((get_type(et) == 2) && (et % OFST != 0))
    {
        fprintf (stdout, MSG_WARN_IDX_FLOAT, line_num+1);

        add_instr("F2I_M %s\n", v_name[et % OFST]);
    }

    // when float in acc --------------------------------------------------

    if ((get_type(et) == 2) && (et % OFST == 0))
    {
        fprintf (stdout, MSG_WARN_IDX_FLOAT, line_num+1);

        add_instr("F2I\n");
    }

    // when comp const in memory ------------------------------------------

    if (get_type(et) == 5)
    {
        fprintf (stdout, MSG_WARN_IDX_COMP_ROUND, line_num+1);

        get_cmp_cst(et, &etr, &eti);

        add_instr("F2I_M %s\n", v_name[etr % OFST]);
    }

    // when comp in acc ---------------------------------------------------

    if ((get_type(et) == 3) && (et % OFST == 0))
    {
        fprintf (stdout, MSG_WARN_IDX_COMP_ROUND, line_num+1);

        add_instr("POP\n");
        add_instr("F2I\n");
    }

    // when comp in memory ------------------------------------------------

    if ((get_type(et) == 3) && (et % OFST != 0))
    {
        fprintf (stdout, MSG_WARN_IDX_COMP_ROUND, line_num+1);

        add_instr("F2I_M %s\n", v_name[et % OFST]);
    }

    acc_ok = 1; // acc carregado
}

// loads the array index into the accumulator (2D array)
void arr_2d_index(int id, int et1, int et2)
{
    // ------------------------------------------------------------------------
    // argument check ---------------------------------------------------------
    // ------------------------------------------------------------------------

    // must check that it really is an array
    if (v_isar[id] == 0)
        {fprintf (stderr, MSG_ERR_NOT_ARRAY_HARSH, line_num+1, rem_fname(v_name[id], fname)); exit(EXIT_FAILURE);}

    // must check that it is not a 1D array
    if (v_isar[id] == 1)
        {fprintf (stderr, MSG_ERR_ARRAY_1D, line_num+1, rem_fname(v_name[id], fname)); exit(EXIT_FAILURE);}

    // ------------------------------------------------------------------------
    // execute ----------------------------------------------------------------
    // ------------------------------------------------------------------------

    int etr, eti;

    // int in acc and int in acc
    if ((get_type(et1) == 1) && (et1 % OFST == 0) && (get_type(et2) == 1) && (et2 % OFST == 0))
    {
        add_instr("SET_P aux_var\n");
        add_instr("MLT   %s_arr_size\n", v_name[id]);
        add_instr("ADD   aux_var\n");
    }

    // int in acc and int in memory
    if ((get_type(et1) == 1) && (et1 % OFST == 0) && (get_type(et2) == 1) && (et2 % OFST != 0))
    {
        add_instr("MLT %s_arr_size\n", v_name[id]);
        add_instr("ADD %s\n",  v_name[et2 % OFST]);
    }

    // int in acc and float in acc
    if ((get_type(et1) == 1) && (et1 % OFST == 0) && (get_type(et2) == 2) && (et2 % OFST == 0))
    {
        fprintf (stdout, MSG_WARN_IDX2_FLOAT, line_num+1);

        add_instr("F2I\n");
        add_instr("SET_P aux_var\n");
        add_instr("MLT   %s_arr_size\n", v_name[id]);
        add_instr("ADD   aux_var\n");
    }

    // int in acc and float in memory
    if ((get_type(et1) == 1) && (et1 % OFST == 0) && (get_type(et2) == 2) && (et2 % OFST != 0))
    {
        fprintf (stdout, MSG_WARN_IDX2_FLOAT, line_num+1);

        add_instr("MLT     %s_arr_size\n", v_name[id]);
        add_instr("P_F2I_M %s\n" , v_name[et2 % OFST]);
        add_instr("S_ADD\n");
    }

    // int in acc and comp const in memory
    if ((get_type(et1) == 1) && (et1 % OFST == 0) && (get_type(et2) == 5) && (et2 % OFST != 0))
    {
        fprintf (stdout, MSG_WARN_IDX_COMP_GRAB, line_num+1);

        get_cmp_cst(et2, &etr, &eti);

        add_instr("MLT     %s_arr_size\n", v_name[id]);
        add_instr("P_F2I_M %s\n" , v_name[etr % OFST]);
        add_instr("S_ADD\n");
    }

    // int in acc and comp in acc
    if ((get_type(et1) == 1) && (et1 % OFST == 0) && (get_type(et2) == 3) && (et2 % OFST == 0))
    {
        fprintf (stdout, MSG_WARN_IDX_COMP_GRAB, line_num+1);

        add_instr("SET_P aux_var\n");
        add_instr("F2I\n");
        add_instr("SET_P aux_var\n");
        add_instr("MLT   %s_arr_size\n", v_name[id]);
        add_instr("ADD   aux_var\n");
    }

    // int in acc and comp in memory
    if ((get_type(et1) == 1) && (et1 % OFST == 0) && (get_type(et2) == 3) && (et2 % OFST != 0))
    {
        fprintf (stdout, MSG_WARN_IDX_COMP_GRAB, line_num+1);

        add_instr("MLT     %s_arr_size\n", v_name[id]);
        add_instr("P_F2I_M %s\n" , v_name[et2 % OFST]);
        add_instr("S_ADD\n");
    }

    // int in memory and int in acc
    if ((get_type(et1) == 1) && (et1 % OFST != 0) && (get_type(et2) == 1) && (et2 % OFST == 0))
    {
        add_instr("P_LOD %s\n",  v_name[et1 % OFST]);
        add_instr("MLT   %s_arr_size\n", v_name[id]);
        add_instr("S_ADD\n");
    }

    // int in memory and int in memory
    if ((get_type(et1) == 1) && (et1 % OFST != 0) && (get_type(et2) == 1) && (et2 % OFST != 0))
    {
        add_instr("LOD  %s\n",  v_name[et1 % OFST]);
        add_instr("MLT  %s_arr_size\n", v_name[id]);
        add_instr("ADD  %s\n",  v_name[et2 % OFST]);
    }

    // int in memory and float in acc
    if ((get_type(et1) == 1) && (et1 % OFST != 0) && (get_type(et2) == 2) && (et2 % OFST == 0))
    {
        fprintf (stdout, MSG_WARN_IDX2_FLOAT, line_num+1);

        add_instr("F2I\n");
        add_instr("P_LOD %s\n",  v_name[et1 % OFST]);
        add_instr("MLT   %s_arr_size\n", v_name[id]);
        add_instr("S_ADD\n");
    }

    // int in memory and float in memory
    if ((get_type(et1) == 1) && (et1 % OFST != 0) && (get_type(et2) == 2) && (et2 % OFST != 0))
    {
        fprintf (stdout, MSG_WARN_IDX2_FLOAT, line_num+1);

        add_instr("LOD     %s\n",  v_name[et1 % OFST]);
        add_instr("MLT     %s_arr_size\n", v_name[id]);
        add_instr("P_F2I_M %s\n",  v_name[et2 % OFST]);
        add_instr("S_ADD\n");
    }

    // int in memory and comp const in memory
    if ((get_type(et1) == 1) && (et1 % OFST != 0) && (get_type(et2) == 5) && (et2 % OFST != 0))
    {
        fprintf(stdout, MSG_WARN_IDX_COMP_GRAB, line_num+1);

        get_cmp_cst(et2, &etr, &eti);

        add_instr("LOD     %s\n",  v_name[et1 % OFST]);
        add_instr("MLT     %s_arr_size\n", v_name[id]);
        add_instr("P_F2I_M %s\n",  v_name[etr % OFST]);
        add_instr("S_ADD\n");
    }

    // int in memory and comp in acc
    if ((get_type(et1) == 1) && (et1 % OFST != 0) && (get_type(et2) == 3) && (et2 % OFST == 0))
    {
        fprintf(stdout, MSG_WARN_IDX_COMP_GRAB, line_num+1);

        add_instr("SET_P aux_var\n");
        add_instr("F2I\n");
        add_instr("SET   aux_var\n");
        add_instr("LOD   %s\n",  v_name[et1 % OFST]);
        add_instr("MLT   %s_arr_size\n", v_name[id]);
        add_instr("ADD   aux_var\n");
    }

    // int in memory and comp in memory
    if ((get_type(et1) == 1) && (et1 % OFST != 0) && (get_type(et2) == 3) && (et2 % OFST != 0))
    {
        fprintf(stdout, MSG_WARN_IDX_COMP_GRAB, line_num+1);

        add_instr("LOD     %s\n",  v_name[et1 % OFST]);
        add_instr("MLT     %s_arr_size\n", v_name[id]);
        add_instr("P_F2I_M %s\n",  v_name[et2 % OFST]);
        add_instr("S_ADD\n");
    }

    // float in acc and int in acc
    if ((get_type(et1) == 2) && (et1 % OFST == 0) && (get_type(et2) == 1) && (et2 % OFST == 0))
    {
        fprintf(stdout, MSG_WARN_IDX1_FLOAT, line_num+1);

        add_instr("SET_P aux_var\n");
        add_instr("F2I\n");
        add_instr("MLT   %s_arr_size\n", v_name[id]);
        add_instr("ADD   aux_var\n");
    }

    // float in acc and int in memory
    if ((get_type(et1) == 2) && (et1 % OFST == 0) && (get_type(et2) == 1) && (et2 % OFST != 0))
    {
        fprintf(stdout, MSG_WARN_IDX1_FLOAT, line_num+1);

        add_instr("F2I\n");
        add_instr("MLT %s_arr_size\n", v_name[id]);
        add_instr("ADD %s\n",  v_name[et2 % OFST]);
    }

    // float in acc and float in acc
    if ((get_type(et1) == 2) && (et1 % OFST == 0) && (get_type(et2) == 2) && (et2 % OFST == 0))
    {
        fprintf(stdout, MSG_WARN_IDXS_FLOAT, line_num+1);

        add_instr("F2I\n");
        add_instr("SET_P aux_var\n");
        add_instr("F2I\n");
        add_instr("MLT   %s_arr_size\n", v_name[id]);
        add_instr("ADD   aux_var\n");
    }

    // float in acc and float in memory
    if ((get_type(et1) == 2) && (et1 % OFST == 0) && (get_type(et2) == 2) && (et2 % OFST != 0))
    {
        fprintf(stdout, MSG_WARN_IDXS_MESS, line_num+1);

        add_instr("F2I\n");
        add_instr("MLT     %s_arr_size\n", v_name[id]);
        add_instr("P_F2I_M %s\n" , v_name[et2 % OFST]);
        add_instr("S_ADD\n");
    }

    // float in acc and comp const in memory
    if ((get_type(et1) == 2) && (et1 % OFST == 0) && (get_type(et2) == 5) && (et2 % OFST != 0))
    {
        fprintf(stdout, MSG_WARN_IDXS_MESS, line_num+1);

        get_cmp_cst(et2, &etr, &eti);

        add_instr("F2I\n");
        add_instr("MLT     %s_arr_size\n", v_name[id]);
        add_instr("P_F2I_M %s\n" , v_name[etr % OFST]);
        add_instr("S_ADD\n");
    }

    // float in acc and comp in acc
    if ((get_type(et1) == 2) && (et1 % OFST == 0) && (get_type(et2) == 3) && (et2 % OFST == 0))
    {
        fprintf (stdout, MSG_WARN_IDXS_MESS, line_num+1);

        add_instr("SET_P aux_var\n");
        add_instr("F2I\n");
        add_instr("SET_P aux_var\n");
        add_instr("F2I\n");
        add_instr("MLT   %s_arr_size\n", v_name[id]);
        add_instr("ADD   aux_var\n");
    }

    // float in acc and comp in memory
    if ((get_type(et1) == 2) && (et1 % OFST == 0) && (get_type(et2) == 3) && (et2 % OFST != 0))
    {
        fprintf (stdout, MSG_WARN_IDXS_MESS, line_num+1);

        add_instr("F2I\n");
        add_instr("MLT     %s_arr_size\n", v_name[id]);
        add_instr("P_F2I_M %s\n" , v_name[et2 % OFST]);
        add_instr("S_ADD\n");
    }

    // float in memory and int in acc
    if ((get_type(et1) == 2) && (et1 % OFST != 0) && (get_type(et2) == 1) && (et2 % OFST == 0))
    {
        fprintf(stdout, MSG_WARN_IDX1_FLOAT, line_num+1);

        add_instr("P_F2I_M %s\n" , v_name[et1 % OFST]);
        add_instr("MLT     %s_arr_size\n", v_name[id]);
        add_instr("S_ADD\n");
    }

    // float in memory and int in memory
    if ((get_type(et1) == 2) && (et1 % OFST != 0) && (get_type(et2) == 1) && (et2 % OFST != 0))
    {
        fprintf(stdout, MSG_WARN_IDX1_FLOAT, line_num+1);

        add_instr("F2I_M  %s\n",  v_name[et1 % OFST]);
        add_instr("MLT    %s_arr_size\n", v_name[id]);
        add_instr("ADD    %s\n",  v_name[et2 % OFST]);
    }

    // float in memory and float in acc
    if ((get_type(et1) == 2) && (et1 % OFST != 0) && (get_type(et2) == 2) && (et2 % OFST == 0))
    {
        fprintf (stdout, MSG_WARN_IDXS_FLOAT, line_num+1);

        add_instr("F2I\n");
        add_instr("P_F2I_M %s\n",  v_name[et1 % OFST]);
        add_instr("MLT     %s_arr_size\n", v_name[id]);
        add_instr("S_ADD\n");
    }

    // float in memory and float in memory
    if ((get_type(et1) == 2) && (et1 % OFST != 0) && (get_type(et2) == 2) && (et2 % OFST != 0))
    {
        fprintf (stdout, MSG_WARN_IDXS_FLOAT, line_num+1);

        add_instr("F2I_M   %s\n",  v_name[et1 % OFST]);
        add_instr("MLT     %s_arr_size\n", v_name[id]);
        add_instr("P_F2I_M %s\n",  v_name[et2 % OFST]);
        add_instr("S_ADD\n");
    }

    // float in memory and comp const in memory
    if ((get_type(et1) == 2) && (et1 % OFST != 0) && (get_type(et2) == 5) && (et2 % OFST != 0))
    {
        fprintf(stdout, MSG_WARN_IDXS_MESS, line_num+1);

        get_cmp_cst(et2, &etr, &eti);

        add_instr("F2I_M   %s\n",  v_name[et1 % OFST]);
        add_instr("MLT     %s_arr_size\n", v_name[id]);
        add_instr("P_F2I_M %s\n",  v_name[etr % OFST]);
        add_instr("S_ADD\n");
    }

    // float in memory and comp in acc
    if ((get_type(et1) == 2) && (et1 % OFST != 0) && (get_type(et2) == 3) && (et2 % OFST == 0))
    {
        fprintf(stdout, MSG_WARN_IDXS_MESS, line_num+1);

        add_instr("SET_P aux_var\n");
        add_instr("F2I\n");
        add_instr("SET   aux_var\n");
        add_instr("F2I_M %s\n",  v_name[et1 % OFST]);
        add_instr("MLT   %s_arr_size\n", v_name[id]);
        add_instr("ADD   aux_var\n");
    }

    // float in memory and comp in memory
    if ((get_type(et1) == 2) && (et1 % OFST != 0) && (get_type(et2) == 3) && (et2 % OFST != 0))
    {
        fprintf(stdout, MSG_WARN_IDXS_MESS, line_num+1);

        add_instr("F2I_M   %s\n",  v_name[et1 % OFST]);
        add_instr("MLT     %s_arr_size\n", v_name[id]);
        add_instr("P_F2I_M %s\n",  v_name[et2 % OFST]);
        add_instr("S_ADD\n");
    }

    // comp const in memory and int in acc
    if ((get_type(et1) == 5) && (et1 % OFST != 0) && (get_type(et2) == 1) && (et2 % OFST == 0))
    {
        fprintf (stdout, MSG_WARN_IDX_COMP_GRAB, line_num+1);

        get_cmp_cst(et1, &etr, &eti);

        add_instr("F2I\n");
        add_instr("SET   aux_var\n");
        add_instr("F2I_M %s\n" , v_name[etr % OFST]);
        add_instr("MLT   %s_arr_size\n", v_name[id]);
        add_instr("ADD   aux_var\n");
    }

    // comp const in memory and int in memory
    if ((get_type(et1) == 5) && (et1 % OFST != 0) && (get_type(et2) == 1) && (et2 % OFST != 0))
    {
        fprintf(stdout, MSG_WARN_IDX_COMP_GRAB, line_num+1);

        get_cmp_cst(et1, &etr, &eti);

        add_instr("F2I_M %s\n" , v_name[etr % OFST]);
        add_instr("MLT   %s_arr_size\n", v_name[id]);
        add_instr("ADD   %s\n",  v_name[et2 % OFST]);
    }

    // comp const in memory and float in acc
    if ((get_type(et1) == 5) && (et1 % OFST != 0) && (get_type(et2) == 2) && (et2 % OFST == 0))
    {
        fprintf(stdout, MSG_WARN_IDXS_MESS, line_num+1);

        get_cmp_cst(et1, &etr, &eti);

        add_instr("F2I\n");
        add_instr("SET   aux_var\n");
        add_instr("F2I_M %s\n" , v_name[etr % OFST]);
        add_instr("MLT   %s_arr_size\n", v_name[id]);
        add_instr("ADD   aux_var\n");
    }

    // comp const in memory and float in memory
    if ((get_type(et1) == 5) && (et1 % OFST != 0) && (get_type(et2) == 2) && (et2 % OFST != 0))
    {
        fprintf(stdout, MSG_WARN_IDXS_MESS, line_num+1);

        get_cmp_cst(et1, &etr, &eti);

        add_instr("F2I_M   %s\n" , v_name[etr % OFST]);
        add_instr("MLT     %s_arr_size\n", v_name[id]);
        add_instr("P_F2I_M %s\n" , v_name[et2 % OFST]);
        add_instr("S_ADD\n");
    }

    // comp const in memory and comp const in memory
    if ((get_type(et1) == 5) && (et1 % OFST != 0) && (get_type(et2) == 5) && (et2 % OFST != 0))
    {
        fprintf (stdout, MSG_WARN_IDXS_MESS, line_num+1);

        get_cmp_cst(et1, &etr, &eti);
        add_instr("F2I_M   %s\n" , v_name[etr % OFST]);
        add_instr("MLT     %s_arr_size\n", v_name[id]);

        get_cmp_cst(et2, &etr, &eti);
        add_instr("P_F2I_M %s\n" , v_name[etr % OFST]);
        add_instr("S_ADD\n");
    }

    // comp const in memory and comp in acc
    if ((get_type(et1) == 5) && (et1 % OFST != 0) && (get_type(et2) == 3) && (et2 % OFST == 0))
    {
        fprintf(stdout, MSG_WARN_IDXS_MESS, line_num+1);

        get_cmp_cst(et1, &etr, &eti);

        add_instr("SET_P aux_var\n");
        add_instr("F2I\n");
        add_instr("SET   aux_var\n");
        add_instr("F2I_M %s\n" , v_name[etr % OFST]);
        add_instr("MLT   %s_arr_size\n", v_name[id]);
        add_instr("ADD   aux_var\n");
    }

    // comp const in memory and comp in memory
    if ((get_type(et1) == 5) && (et1 % OFST != 0) && (get_type(et2) == 3) && (et2 % OFST != 0))
    {
        fprintf(stdout, MSG_WARN_IDXS_MESS, line_num+1);

        get_cmp_cst(et1, &etr, &eti);

        add_instr("F2I_M   %s\n" , v_name[etr % OFST]);
        add_instr("MLT     %s_arr_size\n", v_name[id]);
        add_instr("P_F2I_M %s\n" , v_name[et2 % OFST]);
        add_instr("S_ADD\n");
    }

    // comp in acc and int in acc
    if ((get_type(et1) == 3) && (et1 % OFST == 0) && (get_type(et2) == 1) && (et2 % OFST == 0))
    {
        fprintf(stdout, MSG_WARN_IDX_COMP_GRAB, line_num+1);

        add_instr("SET_P aux_var\n");
        add_instr("POP\n");
        add_instr("F2I\n");
        add_instr("MLT   %s_arr_size\n", v_name[id]);
        add_instr("ADD   aux_var\n");
    }

    // comp in acc and int in memory
    if ((get_type(et1) == 3) && (et1 % OFST == 0) && (get_type(et2) == 1) && (et2 % OFST != 0))
    {
        fprintf(stdout, MSG_WARN_IDX_COMP_GRAB, line_num+1);

        add_instr("POP\n");
        add_instr("F2I\n");
        add_instr("MLT %s_arr_size\n", v_name[id]);
        add_instr("ADD %s\n",  v_name[et2 % OFST]);
    }

    // comp in acc and float in acc
    if ((get_type(et1) == 3) && (et1 % OFST == 0) && (get_type(et2) == 2) && (et2 % OFST == 0))
    {
        fprintf (stdout, MSG_WARN_IDXS_MESS, line_num+1);

        add_instr("F2I\n");
        add_instr("SET_P aux_var\n");
        add_instr("POP\n");
        add_instr("F2I\n");
        add_instr("MLT   %s_arr_size\n", v_name[id]);
        add_instr("ADD   aux_var\n");
    }

    // comp in acc and float in memory
    if ((get_type(et1) == 3) && (et1 % OFST == 0) && (get_type(et2) == 2) && (et2 % OFST != 0))
    {
        fprintf(stdout, MSG_WARN_IDXS_MESS, line_num+1);

        add_instr("POP\n");
        add_instr("F2I\n");
        add_instr("MLT %s_arr_size\n"   , v_name[id]);
        add_instr("P_F2I_M %s\n", v_name[et2 % OFST]);
        add_instr("S_ADD\n");
    }

    // comp in acc and comp const in memory
    if ((get_type(et1) == 3) && (et1 % OFST == 0) && (get_type(et2) == 5) && (et2 % OFST != 0))
    {
        fprintf(stdout, MSG_WARN_IDXS_MESS, line_num+1);

        get_cmp_cst(et2, &etr, &eti);

        add_instr("POP\n");
        add_instr("F2I\n");
        add_instr("MLT     %s_arr_size\n", v_name[id]);
        add_instr("P_F2I_M %s\n" , v_name[etr % OFST]);
        add_instr("S_ADD\n");
    }

    // comp in acc and comp in acc
    if ((get_type(et1) == 3) && (et1 % OFST == 0) && (get_type(et2) == 3) && (et2 % OFST == 0))
    {
        fprintf(stdout, MSG_WARN_IDXS_MESS, line_num+1);

        add_instr("SET_P aux_var\n");
        add_instr("F2I\n");
        add_instr("SET_P aux_var\n");
        add_instr("POP\n");
        add_instr("F2I\n");
        add_instr("MLT   %s_arr_size\n", v_name[id]);
        add_instr("ADD   aux_var\n");
    }

    // comp in acc and comp in memory
    if ((get_type(et1) == 3) && (et1 % OFST == 0) && (get_type(et2) == 3) && (et2 % OFST != 0))
    {
        fprintf (stdout, MSG_WARN_IDXS_MESS, line_num+1);

        add_instr("POP\n");
        add_instr("F2I\n");
        add_instr("MLT     %s_arr_size\n", v_name[id]);
        add_instr("P_F2I_M %s\n" , v_name[et2 % OFST]);
        add_instr("S_ADD\n");
    }

    // comp in memory and int in acc
    if ((get_type(et1) == 3) && (et1 % OFST != 0) && (get_type(et2) == 1) && (et2 % OFST == 0))
    {
        fprintf(stdout, MSG_WARN_IDX_COMP_GRAB, line_num+1);

        add_instr("P_F2I_M %s\n",  v_name[et1 % OFST]);
        add_instr("MLT     %s_arr_size\n", v_name[id]);
        add_instr("S_ADD\n");
    }

    // comp in memory and int in memory
    if ((get_type(et1) == 3) && (et1 % OFST != 0) && (get_type(et2) == 1) && (et2 % OFST != 0))
    {
        fprintf(stdout, MSG_WARN_IDX_COMP_GRAB, line_num+1);

        add_instr("F2I_M %s\n",  v_name[et1 % OFST]);
        add_instr("MLT   %s_arr_size\n", v_name[id]);
        add_instr("ADD   %s\n",  v_name[et2 % OFST]);
    }

    // comp in memory and float in acc
    if ((get_type(et1) == 3) && (et1 % OFST != 0) && (get_type(et2) == 2) && (et2 % OFST == 0))
    {
        fprintf(stdout, MSG_WARN_IDXS_MESS, line_num+1);

        add_instr("F2I\n");
        add_instr("P_F2I_M %s\n",  v_name[et1 % OFST]);
        add_instr("MLT     %s_arr_size\n", v_name[id]);
        add_instr("S_ADD\n");
    }

    // comp in memory and float in memory
    if ((get_type(et1) == 3) && (et1 % OFST != 0) && (get_type(et2) == 2) && (et2 % OFST != 0))
    {
        fprintf(stdout, MSG_WARN_IDXS_MESS, line_num+1);

        add_instr("F2I_M   %s\n",  v_name[et1 % OFST]);
        add_instr("MLT     %s_arr_size\n", v_name[id]);
        add_instr("P_F2I_M %s\n",  v_name[et2 % OFST]);
        add_instr("S_ADD\n");
    }

    // comp in memory and comp const in memory
    if ((get_type(et1) == 3) && (et1 % OFST != 0) && (get_type(et2) == 5) && (et2 % OFST != 0))
    {
        fprintf (stdout, MSG_WARN_IDXS_MESS, line_num+1);

        get_cmp_cst(et2, &etr, &eti);

        add_instr("F2I_M   %s\n" , v_name[et1 % OFST]);
        add_instr("MLT     %s_arr_size\n", v_name[id]);
        add_instr("P_F2I_M %s\n" , v_name[etr % OFST]);
        add_instr("S_ADD\n");
    }

    // comp in memory and comp in acc
    if ((get_type(et1) == 3) && (et1 % OFST != 0) && (get_type(et2) == 3) && (et2 % OFST == 0))
    {
        fprintf (stdout, MSG_WARN_IDXS_MESS, line_num+1);

        add_instr("SET_P aux_var\n");
        add_instr("F2I\n");
        add_instr("SET   aux_var\n");
        add_instr("F2I_M %s\n" , v_name[et1 % OFST]);
        add_instr("MLT   %s_arr_size\n", v_name[id]);
        add_instr("ADD   aux_var\n");
    }

    // comp in memory and comp in memory
    if ((get_type(et1) == 3) && (et1 % OFST != 0) && (get_type(et2) == 3) && (et2 % OFST != 0))
    {
        fprintf (stdout, MSG_WARN_IDXS_MESS, line_num+1);

        add_instr("F2I_M   %s\n" , v_name[et1 % OFST]);
        add_instr("MLT     %s_arr_size\n", v_name[id]);
        add_instr("P_F2I_M %s\n" , v_name[et2 % OFST]);
        add_instr("S_ADD\n");
    }

    acc_ok = 1; // acc carregado
}

// ----------------------------------------------------------------------------
// array in expressions, e.g. x = y[i]; ---------------------------------------
// ----------------------------------------------------------------------------

// turns a 1D array into an exp
// the fft parameter tells whether to use the reversed index
int arr_1d2exp(int id, int et, int fft)
{
    // consistency checks -----------------------------------------------------

    // test whether the variable has been declared
    if (v_type[id] == 0)
        {fprintf (stderr, MSG_ERR_DECL_VAR_PROPERLY, line_num+1, rem_fname(v_name[id], fname)); exit(EXIT_FAILURE);}

    // must check that it really is an array
    if (v_isar[id] == 0)
        {fprintf (stderr, MSG_ERR_NOT_ARRAY_HARSH, line_num+1, rem_fname(v_name[id], fname)); exit(EXIT_FAILURE);}

    // must check that it really is a 1D array
    if (v_isar[id] == 2)
        {fprintf (stderr, MSG_ERR_ARRAY_2D  , line_num+1, rem_fname(v_name[id], fname)); exit(EXIT_FAILURE);}

    // prepare the LOD commands ----------------------------------------------

    char ldv[10]; if (fft    == 1) strcpy(ldv,"ILI"    ); else strcpy(ldv,"LDI"  );
    char ldi[10]; if (acc_ok == 1) strcpy(ldi,"P_LOD"  ); else strcpy(ldi,"LOD"  );
    char f2i[10]; if (acc_ok == 1) strcpy(f2i,"P_F2I_M"); else strcpy(f2i,"F2I_M");

    // ------------------------------------------------------------------------
    // write the instructions -------------------------------------------------
    // ------------------------------------------------------------------------

    int  etr, eti;
    int  type = v_type[id];

    // left int/float ---------------------------------------------------------

    if (type < 3)
    {
        // int in acc
        if ((get_type(et) == 1) && (et % OFST == 0))
        {
            add_instr("%s %s\n", ldv, v_name[id]);
        }

        // int in memory
        if ((get_type(et) == 1) && (et % OFST != 0))
        {
            add_instr("%s %s\n", ldi, v_name[et % OFST]);
            add_instr("%s %s\n", ldv, v_name[id]);
        }

        // float in acc
        if ((get_type(et) == 2) && (et % OFST == 0))
        {
            fprintf (stdout, MSG_WARN_IDX_FLOAT_HEAVY, line_num+1);

            add_instr("F2I\n");
            add_instr("%s %s\n", ldv, v_name[id]);
        }

        // float in memory
        if ((get_type(et) == 2) && (et % OFST != 0))
        {
            fprintf (stdout, MSG_WARN_IDX_FLOAT_HEAVY, line_num+1);

            add_instr("%s %s\n", f2i, v_name[et % OFST]);
            add_instr("%s %s\n", ldv, v_name[id]);
        }

        // comp const in memory
        if ((get_type(et) == 5) && (et % OFST != 0))
        {
            fprintf (stdout, MSG_WARN_IDX_COMP_ROUND, line_num+1);

            get_cmp_cst(et, &etr, &eti);

            add_instr("%s %s\n", f2i, v_name[etr % OFST]);
            add_instr("%s %s\n", ldv, v_name[id]);
        }

        // comp in acc
        if ((get_type(et) == 3) && (et % OFST == 0))
        {
            fprintf (stdout, MSG_WARN_IDX_COMP_ROUND, line_num+1);

            add_instr("POP\n");
            add_instr("F2I\n");
            add_instr("%s %s\n", ldv, v_name[id]);
        }

        // comp in memory
        if ((get_type(et) == 3) && (et % OFST != 0))
        {
            fprintf (stdout, MSG_WARN_IDX_COMP_ROUND, line_num+1);

            add_instr("%s %s\n", f2i, v_name[et%OFST]);
            add_instr("%s %s\n", ldv, v_name[id]);
        }
    }

    // left comp --------------------------------------------------------------

    else
    {
        // int in acc
        if ((get_type(et) == 1) && (et % OFST == 0))
        {
            add_instr("SET   aux_var\n");
            add_instr("%s %s\n"  , ldv, v_name[id]);
            add_instr("P_LOD aux_var\n");
            add_instr("%s %s_i\n", ldv, v_name[id]);
        }

        // int in memory
        if ((get_type(et) == 1) && (et % OFST != 0))
        {
            add_instr("%s %s\n"    , ldi, v_name[et % OFST]);
            add_instr("%s %s\n"    , ldv, v_name[id]);
            add_instr("P_LOD %s\n" ,      v_name[et % OFST]);
            add_instr("%s %s_i\n"  , ldv, v_name[id]);
        }

        // float in acc
        if ((get_type(et) == 2) && (et % OFST == 0))
        {
            fprintf (stdout, MSG_WARN_IDX_FLOAT_HEAVY, line_num+1);

            add_instr("F2I\n");
            add_instr("SET   aux_var\n");
            add_instr("%s %s\n"  , ldv, v_name[id]);
            add_instr("P_LOD aux_var\n");
            add_instr("%s %s_i\n", ldv, v_name[id]);
        }

        // float in memory
        if ((get_type(et) == 2) && (et % OFST != 0))
        {
            fprintf (stdout, MSG_WARN_IDX_FLOAT_HEAVY, line_num+1);

            add_instr("%s %s\n"  , f2i, v_name[et % OFST]);
            add_instr("SET   aux_var\n");
            add_instr("%s %s\n"  , ldv, v_name[id]);
            add_instr("P_LOD aux_var\n");
            add_instr("%s %s_i\n", ldv, v_name[id]);
        }

        // comp const in memory
        if ((get_type(et) == 5) && (et % OFST != 0))
        {
            fprintf (stdout, MSG_WARN_IDX_COMP_ROUND, line_num+1);

            get_cmp_cst(et, &etr, &eti);

            add_instr("%s %s\n"  , f2i, v_name[etr % OFST]);
            add_instr("SET aux_var\n");
            add_instr("%s %s\n"  , ldv, v_name[id]);
            add_instr("P_LOD aux_var\n");
            add_instr("%s %s_i\n", ldv, v_name[id]);
        }

        // comp in acc
        if ((get_type(et) == 3) && (et % OFST == 0))
        {
            fprintf (stdout, MSG_WARN_IDX_COMP_ROUND, line_num+1);

            add_instr("POP\n");
            add_instr("F2I\n");
            add_instr("SET   aux_var\n");
            add_instr("%s %s\n"  , ldv, v_name[id]);
            add_instr("P_LOD aux_var\n");
            add_instr("%s %s_i\n", ldv, v_name[id]);
        }

        // comp in memory
        if ((get_type(et) == 3) && (et % OFST != 0))
        {
            fprintf (stdout, MSG_WARN_IDX_COMP_ROUND, line_num+1);

            add_instr("%s %s\n"  , f2i, v_name[et%OFST]);
            add_instr("SET   aux_var\n");
            add_instr("%s %s\n"  , ldv, v_name[id]);
            add_instr("P_LOD aux_var\n");
            add_instr("%s %s_i\n", ldv, v_name[id]);
        }
    }

    acc_ok     = 1;
    v_used[id] = 1;

    return v_type[id]*OFST;
}

// transforma array 2D em exp
int arr_2d2exp(int id, int et1, int et2)
{
    // consistency checks -----------------------------------------------------

    // test whether the variable has been declared
    if (v_type[id] == 0)
        {fprintf (stderr, MSG_ERR_DECL_VAR_PROPERLY, line_num+1, rem_fname(v_name[id], fname)); exit(EXIT_FAILURE);}

    // must check that it really is a 2D array
    if (v_isar[id] == 0)
        {fprintf (stderr, MSG_ERR_NOT_ARRAY_HARSH, line_num+1, rem_fname(v_name[id], fname)); exit(EXIT_FAILURE);}

    // must check that it is not a 1D array
    if (v_isar[id] == 1)
        {fprintf (stderr, MSG_ERR_ARRAY_1D , line_num+1, rem_fname(v_name[id], fname)); exit(EXIT_FAILURE);}

    // prepare the LOD commands -----------------------------------------------

    char ldv[10];                  strcpy(ldv,"LDI"    ); // no 2D FFT yet
    char ldi[10]; if (acc_ok == 1) strcpy(ldi,"P_LOD"  ); else strcpy(ldi,"LOD"  );
    char f2i[10]; if (acc_ok == 1) strcpy(f2i,"P_F2I_M"); else strcpy(f2i,"F2I_M");

    // ------------------------------------------------------------------------
    // write the instructions -------------------------------------------------
    // ------------------------------------------------------------------------

    int  etr, eti;
    int  type = v_type[id];

    // left int/float ---------------------------------------------------------

    if (type < 3)
    {
        // int in acc and int in acc
        if ((get_type(et1) == 1) && (et1 % OFST == 0) && (get_type(et2) == 1) && (et2 % OFST == 0))
        {
            add_instr("SET_P aux_var\n");
            add_instr("MLT   %s_arr_size\n", v_name[id]);
            add_instr("ADD   aux_var\n");
            add_instr( "%s   %s\n", ldv    , v_name[id]);
        }

        // int in acc and int in memory
        if ((get_type(et1) == 1) && (et1 % OFST == 0) && (get_type(et2) == 1) && (et2 % OFST != 0))
        {
            add_instr("MLT %s_arr_size\n", v_name[id]);
            add_instr("ADD %s\n" , v_name[et2 % OFST]);
            add_instr( "%s %s\n" , ldv   , v_name[id]);
        }

        // int in acc and float in acc
        if ((get_type(et1) == 1) && (et1 % OFST == 0) && (get_type(et2) == 2) && (et2 % OFST == 0))
        {
            fprintf (stdout, MSG_WARN_IDX2_FLOAT, line_num+1);

            add_instr("F2I\n");
            add_instr("SET_P aux_var\n");
            add_instr("MLT   %s_arr_size\n", v_name[id]);
            add_instr("ADD   aux_var\n");
            add_instr( "%s   %s\n", ldv    , v_name[id]);
        }

        // int in acc and float in memory
        if ((get_type(et1) == 1) && (et1 % OFST == 0) && (get_type(et2) == 2) && (et2 % OFST != 0))
        {
            fprintf (stdout, MSG_WARN_IDX2_FLOAT, line_num+1);

            add_instr("MLT     %s_arr_size\n", v_name[id]);
            add_instr("P_F2I_M %s\n" , v_name[et2 % OFST]);
            add_instr("S_ADD\n");
            add_instr( "%s     %s\n", ldv    , v_name[id]);
        }

        // int in acc and comp const in memory
        if ((get_type(et1) == 1) && (et1 % OFST == 0) && (get_type(et2) == 5) && (et2 % OFST != 0))
        {
            fprintf (stdout, MSG_WARN_IDX_COMP_GRAB, line_num+1);

            get_cmp_cst(et2, &etr, &eti);

            add_instr("MLT     %s_arr_size\n", v_name[id]);
            add_instr("P_F2I_M %s\n" , v_name[etr % OFST]);
            add_instr("S_ADD\n");
            add_instr( "%s     %s\n", ldv    , v_name[id]);
        }

        // int in acc and comp in acc
        if ((get_type(et1) == 1) && (et1 % OFST == 0) && (get_type(et2) == 3) && (et2 % OFST == 0))
        {
            fprintf (stdout, MSG_WARN_IDX_COMP_GRAB, line_num+1);

            add_instr("SET_P aux_var\n");
            add_instr("F2I\n");
            add_instr("SET_P aux_var\n");
            add_instr("MLT   %s_arr_size\n", v_name[id]);
            add_instr("ADD   aux_var\n");
            add_instr( "%s   %s\n", ldv    , v_name[id]);
        }

        // int in acc and comp in memory
        if ((get_type(et1) == 1) && (et1 % OFST == 0) && (get_type(et2) == 3) && (et2 % OFST != 0))
        {
            fprintf (stdout, MSG_WARN_IDX_COMP_GRAB, line_num+1);

            add_instr("MLT     %s_arr_size\n", v_name[id]);
            add_instr("P_F2I_M %s\n" , v_name[et2 % OFST]);
            add_instr("S_ADD\n");
            add_instr( "%s     %s\n", ldv    , v_name[id]);
        }

        // int in memory and int in acc
        if ((get_type(et1) == 1) && (et1 % OFST != 0) && (get_type(et2) == 1) && (et2 % OFST == 0))
        {
            add_instr("P_LOD %s\n" , v_name[et1 % OFST]);
            add_instr("MLT   %s_arr_size\n", v_name[id]);
            add_instr("S_ADD\n");
            add_instr( "%s   %s\n", ldv    , v_name[id]);
        }

        // int in memory and int in memory
        if ((get_type(et1) == 1) && (et1 % OFST != 0) && (get_type(et2) == 1) && (et2 % OFST != 0))
        {
            add_instr( "%s %s\n", ldi, v_name[et1 % OFST]);
            add_instr("MLT %s_arr_size\n"    , v_name[id]);
            add_instr("ADD %s\n"     , v_name[et2 % OFST]);
            add_instr( "%s %s\n", ldv        , v_name[id]);
        }

        // int in memory and float in acc
        if ((get_type(et1) == 1) && (et1 % OFST != 0) && (get_type(et2) == 2) && (et2 % OFST == 0))
        {
            fprintf (stdout, MSG_WARN_IDX2_FLOAT, line_num+1);

            add_instr("F2I\n");
            add_instr("P_LOD %s\n" , v_name[et1 % OFST]);
            add_instr("MLT   %s_arr_size\n", v_name[id]);
            add_instr("S_ADD\n");
            add_instr( "%s   %s\n", ldv    , v_name[id]);
        }

        // int in memory and float in memory
        if ((get_type(et1) == 1) && (et1 % OFST != 0) && (get_type(et2) == 2) && (et2 % OFST != 0))
        {
            fprintf (stdout, MSG_WARN_IDX2_FLOAT, line_num+1);

            add_instr( "%s     %s\n", ldi, v_name[et1 % OFST]);
            add_instr("MLT     %s_arr_size\n"    , v_name[id]);
            add_instr("P_F2I_M %s\n"     , v_name[et2 % OFST]);
            add_instr("S_ADD\n");
            add_instr( "%s     %s\n", ldv        , v_name[id]);
        }

        // int in memory and comp const in memory
        if ((get_type(et1) == 1) && (et1 % OFST != 0) && (get_type(et2) == 5) && (et2 % OFST != 0))
        {
            fprintf (stdout, MSG_WARN_IDX_COMP_GRAB, line_num+1);

            get_cmp_cst(et2, &etr, &eti);

            add_instr( "%s     %s\n", ldi, v_name[et1 % OFST]);
            add_instr("MLT     %s_arr_size\n"    , v_name[id]);
            add_instr("P_F2I_M %s\n"     , v_name[etr % OFST]);
            add_instr("S_ADD\n");
            add_instr( "%s     %s\n", ldv        , v_name[id]);
        }

        // int in memory and comp in acc
        if ((get_type(et1) == 1) && (et1 % OFST != 0) && (get_type(et2) == 3) && (et2 % OFST == 0))
        {
            fprintf (stdout, MSG_WARN_IDX_COMP_GRAB, line_num+1);

            add_instr("SET_P aux_var\n");
            add_instr("F2I\n");
            add_instr("SET   aux_var\n");
            add_instr("LOD   %s\n",  v_name[et1 % OFST]);
            add_instr("MLT   %s_arr_size\n", v_name[id]);
            add_instr("ADD   aux_var\n");
            add_instr( "%s   %s\n", ldv    , v_name[id]);
        }

        // int in memory and comp in memory
        if ((get_type(et1) == 1) && (et1 % OFST != 0) && (get_type(et2) == 3) && (et2 % OFST != 0))
        {
            fprintf (stdout, MSG_WARN_IDX_COMP_GRAB, line_num+1);

            add_instr( "%s     %s\n", ldi, v_name[et1 % OFST]);
            add_instr("MLT     %s_arr_size\n"    , v_name[id]);
            add_instr("P_F2I_M %s\n"     , v_name[et2 % OFST]);
            add_instr("S_ADD\n");
            add_instr( "%s     %s\n", ldv        , v_name[id]);
        }

        // float in acc and int in acc
        if ((get_type(et1) == 2) && (et1 % OFST == 0) && (get_type(et2) == 1) && (et2 % OFST == 0))
        {
            fprintf (stdout, MSG_WARN_IDX1_FLOAT, line_num+1);

            add_instr("SET_P aux_var\n");
            add_instr("F2I\n");
            add_instr("MLT   %s_arr_size\n", v_name[id]);
            add_instr("ADD   aux_var\n");
            add_instr( "%s   %s\n", ldv    , v_name[id]);
        }

        // float in acc and int in memory
        if ((get_type(et1) == 2) && (et1 % OFST == 0) && (get_type(et2) == 1) && (et2 % OFST != 0))
        {
            fprintf (stdout, MSG_WARN_IDX1_FLOAT, line_num+1);

            add_instr("F2I\n");
            add_instr("MLT %s_arr_size\n", v_name[id]);
            add_instr("ADD %s\n" , v_name[et2 % OFST]);
            add_instr( "%s %s\n" , ldv   , v_name[id]);
        }

        // float in acc and float in acc
        if ((get_type(et1) == 2) && (et1 % OFST == 0) && (get_type(et2) == 2) && (et2 % OFST == 0))
        {
            fprintf (stdout, MSG_WARN_IDXS_FLOAT, line_num+1);

            add_instr("F2I\n");
            add_instr("SET_P aux_var\n");
            add_instr("F2I\n");
            add_instr("MLT   %s_arr_size\n", v_name[id]);
            add_instr("ADD   aux_var\n");
            add_instr( "%s   %s\n", ldv    , v_name[id]);
        }

        // float in acc and float in memory
        if ((get_type(et1) == 2) && (et1 % OFST == 0) && (get_type(et2) == 2) && (et2 % OFST != 0))
        {
            fprintf (stdout, MSG_WARN_IDXS_MESS, line_num+1);

            add_instr("F2I\n");
            add_instr("MLT     %s_arr_size\n", v_name[id]);
            add_instr("P_F2I_M %s\n" , v_name[et2 % OFST]);
            add_instr("S_ADD\n");
            add_instr( "%s     %s\n" , ldv   , v_name[id]);
        }

        // float in acc and comp const in memory
        if ((get_type(et1) == 2) && (et1 % OFST == 0) && (get_type(et2) == 5) && (et2 % OFST != 0))
        {
            fprintf (stdout, MSG_WARN_IDXS_MESS, line_num+1);

            get_cmp_cst(et2, &etr, &eti);

            add_instr("F2I\n");
            add_instr("MLT %s_arr_size\n"   , v_name[id]);
            add_instr("P_F2I_M %s\n", v_name[etr % OFST]);
            add_instr("S_ADD\n");
            add_instr("%s %s\n", ldv        , v_name[id]);
        }

        // float in acc and comp in acc
        if ((get_type(et1) == 2) && (et1 % OFST == 0) && (get_type(et2) == 3) && (et2 % OFST == 0))
        {
            fprintf (stdout, MSG_WARN_IDXS_MESS, line_num+1);

            add_instr("SET_P aux_var\n");
            add_instr("F2I\n");
            add_instr("SET_P aux_var\n");
            add_instr("F2I\n");
            add_instr("MLT   %s_arr_size\n", v_name[id]);
            add_instr("ADD   aux_var\n");
            add_instr( "%s   %s\n", ldv    , v_name[id]);
        }

        // float in acc and comp in memory
        if ((get_type(et1) == 2) && (et1 % OFST == 0) && (get_type(et2) == 3) && (et2 % OFST != 0))
        {
            fprintf (stdout, MSG_WARN_IDXS_MESS, line_num+1);

            add_instr("F2I\n");
            add_instr("MLT     %s_arr_size\n", v_name[id]);
            add_instr("P_F2I_M %s\n",  v_name[et2 % OFST]);
            add_instr("S_ADD\n");
            add_instr( "%s     %s\n", ldv,     v_name[id]);
        }

        // float in memory and int in acc
        if ((get_type(et1) == 2) && (et1 % OFST != 0) && (get_type(et2) == 1) && (et2 % OFST == 0))
        {
            fprintf (stdout, MSG_WARN_IDX1_FLOAT, line_num+1);

            add_instr("SET   aux_var\n");
            add_instr("F2I_M %s\n" ,   v_name[et1%OFST]);
            add_instr("MLT   %s_arr_size\n", v_name[id]);
            add_instr("ADD   aux_var\n");
            add_instr( "%s   %s\n" , ldv   , v_name[id]);
        }

        // float in memory and int in memory
        if ((get_type(et1) == 2) && (et1 % OFST != 0) && (get_type(et2) == 1) && (et2 % OFST != 0))
        {
            fprintf (stdout, MSG_WARN_IDX1_FLOAT, line_num+1);

            add_instr( "%s %s\n", f2i, v_name[et1 % OFST]);
            add_instr("MLT %s_arr_size\n"    , v_name[id]);
            add_instr("ADD %s\n",      v_name[et2 % OFST]);
            add_instr( "%s %s\n", ldv        , v_name[id]);
        }

        // float in memory and float in acc
        if ((get_type(et1) == 2) && (et1 % OFST != 0) && (get_type(et2) == 2) && (et2 % OFST == 0))
        {
            fprintf (stdout, MSG_WARN_IDXS_FLOAT, line_num+1);

            add_instr("F2I\n");
            add_instr("SET   aux_var\n");
            add_instr("F2I_M %s\n",  v_name[et1 % OFST]);
            add_instr("MLT   %s_arr_size\n", v_name[id]);
            add_instr("ADD   aux_var\n");
            add_instr( "%s   %s\n", ldv    , v_name[id]);
        }

        // float in memory and float in memory
        if ((get_type(et1) == 2) && (et1 % OFST != 0) && (get_type(et2) == 2) && (et2 % OFST != 0))
        {
            fprintf (stdout, MSG_WARN_IDXS_MESS, line_num+1);

            add_instr( "%s     %s\n", f2i, v_name[et1 % OFST]);
            add_instr("MLT     %s_arr_size\n"    , v_name[id]);
            add_instr("P_F2I_M %s\n",      v_name[et2 % OFST]);
            add_instr("S_ADD\n");
            add_instr( "%s     %s\n", ldv        , v_name[id]);
        }

        // float in memory and comp const in memory
        if ((get_type(et1) == 2) && (et1 % OFST != 0) && (get_type(et2) == 5) && (et2 % OFST != 0))
        {
            fprintf (stdout, MSG_WARN_IDXS_MESS, line_num+1);

            get_cmp_cst(et2, &etr, &eti);

            add_instr( "%s     %s\n", f2i, v_name[et1 % OFST]);
            add_instr("MLT     %s_arr_size\n"    , v_name[id]);
            add_instr("P_F2I_M %s\n",      v_name[etr % OFST]);
            add_instr("S_ADD\n");
            add_instr( "%s     %s\n", ldv        , v_name[id]);
        }

        // float in memory and comp in acc
        if ((get_type(et1) == 2) && (et1 % OFST != 0) && (get_type(et2) == 3) && (et2 % OFST == 0))
        {
            fprintf (stdout, MSG_WARN_IDXS_MESS, line_num+1);

            add_instr("SET_P aux_var\n");
            add_instr("F2I\n");
            add_instr("SET   aux_var\n");
            add_instr("F2I_M %s \n",   v_name[et1%OFST]);
            add_instr("MLT   %s_arr_size\n", v_name[id]);
            add_instr("ADD   aux_var\n");
            add_instr( "%s   %s\n", ldv     , v_name[id]);
        }

        // float in memory and comp in memory
        if ((get_type(et1) == 2) && (et1 % OFST != 0) && (get_type(et2) == 3) && (et2 % OFST != 0))
        {
            fprintf (stdout, MSG_WARN_IDXS_MESS, line_num+1);

            add_instr( "%s     %s\n", f2i, v_name[et1 % OFST]);
            add_instr("MLT     %s_arr_size\n"    , v_name[id]);
            add_instr("P_F2I_M %s\n",      v_name[et2 % OFST]);
            add_instr("S_ADD\n");
            add_instr( "%s     %s\n", ldv        , v_name[id]);
        }

        // comp const in memory and int in acc
        if ((get_type(et1) == 5) && (et1 % OFST != 0) && (get_type(et2) == 1) && (et2 % OFST == 0))
        {
            fprintf (stdout, MSG_WARN_IDX_COMP_GRAB, line_num+1);

            get_cmp_cst(et1, &etr, &eti);

            add_instr("F2I\n");
            add_instr("SET   aux_var\n");
            add_instr("F2I_M %s\n",  v_name[etr % OFST]);
            add_instr("MLT   %s_arr_size\n", v_name[id]);
            add_instr("ADD   aux_var\n");
            add_instr( "%s   %s\n", ldv    , v_name[id]);
        }

        // comp const in memory and int in memory
        if ((get_type(et1) == 5) && (et1 % OFST != 0) && (get_type(et2) == 1) && (et2 % OFST != 0))
        {
            fprintf (stdout, MSG_WARN_IDX_COMP_GRAB, line_num+1);

            get_cmp_cst(et1, &etr, &eti);

            add_instr( "%s %s\n", f2i, v_name[etr % OFST]);
            add_instr("MLT %s_arr_size\n"    , v_name[id]);
            add_instr("ADD %s\n",      v_name[et2 % OFST]);
            add_instr( "%s %s\n", ldv        , v_name[id]);
        }

        // comp const in memory and float in acc
        if ((get_type(et1) == 5) && (et1 % OFST != 0) && (get_type(et2) == 2) && (et2 % OFST == 0))
        {
            fprintf (stdout, MSG_WARN_IDXS_MESS, line_num+1);

            get_cmp_cst(et1, &etr, &eti);

            add_instr("F2I\n");
            add_instr("SET   aux_var\n");
            add_instr("F2I_M %s\n",  v_name[etr % OFST]);
            add_instr("MLT   %s_arr_size\n", v_name[id]);
            add_instr("ADD   aux_var\n");
            add_instr( "%s   %s\n", ldv    , v_name[id]);
        }

        // comp const in memory and float in memory
        if ((get_type(et1) == 5) && (et1 % OFST != 0) && (get_type(et2) == 2) && (et2 % OFST != 0))
        {
            fprintf (stdout, MSG_WARN_IDXS_MESS, line_num+1);

            get_cmp_cst(et1, &etr, &eti);

            add_instr( "%s     %s\n", f2i, v_name[etr % OFST]);
            add_instr("MLT     %s_arr_size\n"    , v_name[id]);
            add_instr("P_F2I_M %s\n",      v_name[et2 % OFST]);
            add_instr("S_ADD\n");
            add_instr( "%s     %s\n", ldv        , v_name[id]);
        }

        // comp const in memory and comp const in memory
        if ((get_type(et1) == 5) && (et1 % OFST != 0) && (get_type(et2) == 5) && (et2 % OFST != 0))
        {
            fprintf (stdout, MSG_WARN_IDXS_MESS, line_num+1);

            get_cmp_cst(et1, &etr, &eti);
            add_instr("%s %s\n", f2i, v_name[etr % OFST]);
            add_instr("MLT %s_arr_size\n"   , v_name[id]);

            get_cmp_cst(et2, &etr, &eti);
            add_instr("P_F2I_M %s\n", v_name[etr % OFST]);
            add_instr("S_ADD\n");
            add_instr( "%s     %s\n", ldv   , v_name[id]);
        }

        // comp const in memory and comp in acc
        if ((get_type(et1) == 5) && (et1 % OFST != 0) && (get_type(et2) == 3) && (et2 % OFST == 0))
        {
            fprintf (stdout, MSG_WARN_IDXS_MESS, line_num+1);

            get_cmp_cst(et1, &etr, &eti);

            add_instr("SET_P aux_var\n");
            add_instr("F2I\n");
            add_instr("SET   aux_var\n");
            add_instr("F2I_M %s\n",  v_name[etr % OFST]);
            add_instr("MLT   %s_arr_size\n", v_name[id]);
            add_instr("ADD   aux_var\n");
            add_instr( "%s   %s\n", ldv    , v_name[id]);
        }

        // comp const in memory and comp in memory
        if ((get_type(et1) == 5) && (et1 % OFST != 0) && (get_type(et2) == 3) && (et2 % OFST != 0))
        {
            fprintf (stdout, MSG_WARN_IDXS_MESS, line_num+1);

            get_cmp_cst(et1, &etr, &eti);

            add_instr( "%s     %s\n", f2i, v_name[etr % OFST]);
            add_instr("MLT     %s_arr_size\n"    , v_name[id]);
            add_instr("P_F2I_M %s\n",      v_name[et2 % OFST]);
            add_instr("S_ADD\n");
            add_instr( "%s     %s\n", ldv        , v_name[id]);
        }

        // comp in acc and int in acc
        if ((get_type(et1) == 3) && (et1 % OFST == 0) && (get_type(et2) == 1) && (et2 % OFST == 0))
        {
            fprintf (stdout, MSG_WARN_IDX_COMP_GRAB, line_num+1);

            add_instr("SET_P aux_var\n");
            add_instr("POP\n");
            add_instr("F2I\n");
            add_instr("MLT   %s_arr_size\n", v_name[id]);
            add_instr("ADD   aux_var\n");
            add_instr( "%s   %s\n", ldv    , v_name[id]);
        }

        // comp in acc and int in memory
        if ((get_type(et1) == 3) && (et1 % OFST == 0) && (get_type(et2) == 1) && (et2 % OFST != 0))
        {
            fprintf (stdout, MSG_WARN_IDX_COMP_GRAB, line_num+1);

            add_instr("POP\n");
            add_instr("F2I\n");
            add_instr("MLT %s_arr_size\n", v_name[id]);
            add_instr("ADD %s\n",  v_name[et2 % OFST]);
            add_instr( "%s %s\n",  ldv   , v_name[id]);
        }

        // comp in acc and float in acc
        if ((get_type(et1) == 3) && (et1 % OFST == 0) && (get_type(et2) == 2) && (et2 % OFST == 0))
        {
            fprintf (stdout, MSG_WARN_IDXS_MESS, line_num+1);

            add_instr("F2I\n");
            add_instr("SET_P aux_var\n");
            add_instr("POP\n");
            add_instr("F2I\n");
            add_instr("MLT  %s_arr_size\n", v_name[id]);
            add_instr("ADD  aux_var\n");
            add_instr( "%s  %s\n", ldv    , v_name[id]);
        }

        // comp in acc and float in memory
        if ((get_type(et1) == 3) && (et1 % OFST == 0) && (get_type(et2) == 2) && (et2 % OFST != 0))
        {
            fprintf (stdout, MSG_WARN_IDXS_MESS, line_num+1);

            add_instr("POP\n");
            add_instr("F2I\n");
            add_instr("MLT     %s_arr_size\n", v_name[id]);
            add_instr("P_F2I_M %s\n",  v_name[et2 % OFST]);
            add_instr("S_ADD\n");
            add_instr( "%s     %s\n",  ldv   , v_name[id]);
        }

        // comp in acc and comp const in memory
        if ((get_type(et1) == 3) && (et1 % OFST == 0) && (get_type(et2) == 5) && (et2 % OFST != 0))
        {
            fprintf (stdout, MSG_WARN_IDXS_MESS, line_num+1);

            get_cmp_cst(et2, &etr, &eti);

            add_instr("POP\n");
            add_instr("F2I\n");
            add_instr("MLT     %s_arr_size\n", v_name[id]);
            add_instr("P_F2I_M %s\n",  v_name[etr % OFST]);
            add_instr("S_ADD\n");
            add_instr( "%s     %s\n",  ldv   , v_name[id]);
        }

        // comp in acc and comp in acc
        if ((get_type(et1) == 3) && (et1 % OFST == 0) && (get_type(et2) == 3) && (et2 % OFST == 0))
        {
            fprintf (stdout, MSG_WARN_IDXS_MESS, line_num+1);

            add_instr("POP\n");
            add_instr("F2I\n");
            add_instr("SET_P aux_var\n");
            add_instr("POP\n");
            add_instr("F2I\n");
            add_instr("MLT   %s_arr_size\n", v_name[id]);
            add_instr("ADD   aux_var\n");
            add_instr( "%s   %s\n", ldv    , v_name[id]);
        }

        // comp in acc and comp in memory
        if ((get_type(et1) == 3) && (et1 % OFST == 0) && (get_type(et2) == 3) && (et2 % OFST != 0))
        {
            fprintf (stdout, MSG_WARN_IDXS_MESS, line_num+1);

            add_instr("POP\n");
            add_instr("F2I\n");
            add_instr("MLT     %s_arr_size\n", v_name[id]);
            add_instr("P_F2I_M %s\n",  v_name[et2 % OFST]);
            add_instr("S_ADD\n");
            add_instr( "%s     %s\n",  ldv   , v_name[id]);
        }

        // comp in memory and int in acc
        if ((get_type(et1) == 3) && (et1 % OFST != 0) && (get_type(et2) == 1) && (et2 % OFST == 0))
        {
            fprintf (stdout, MSG_WARN_IDX_COMP_GRAB, line_num+1);

            add_instr("P_F2I_M %s\n",  v_name[et1 % OFST]);
            add_instr("MLT     %s_arr_size\n", v_name[id]);
            add_instr("S_ADD\n");
            add_instr( "%s     %s\n",  ldv   , v_name[id]);
        }

        // comp in memory and int in memory
        if ((get_type(et1) == 3) && (et1 % OFST != 0) && (get_type(et2) == 1) && (et2 % OFST != 0))
        {
            fprintf (stdout, MSG_WARN_IDX_COMP_GRAB, line_num+1);

            add_instr( "%s %s\n", f2i, v_name[et1 % OFST]);
            add_instr("MLT %s_arr_size\n"    , v_name[id]);
            add_instr("ADD %s\n",      v_name[et2 % OFST]);
            add_instr( "%s %s\n", ldv        , v_name[id]);
        }

        // comp in memory and float in acc
        if ((get_type(et1) == 3) && (et1 % OFST != 0) && (get_type(et2) == 2) && (et2 % OFST == 0))
        {
            fprintf (stdout, MSG_WARN_IDXS_MESS, line_num+1);

            add_instr("F2I\n");
            add_instr("P_F2I_M %s\n",  v_name[et1 % OFST]);
            add_instr("MLT     %s_arr_size\n", v_name[id]);
            add_instr("S_ADD\n");
            add_instr( "%s     %s\n",  ldv   , v_name[id]);
        }

        // comp in memory and float in memory
        if ((get_type(et1) == 3) && (et1 % OFST != 0) && (get_type(et2) == 2) && (et2 % OFST != 0))
        {
            fprintf (stdout, MSG_WARN_IDXS_MESS, line_num+1);

            add_instr("%s %s\n", f2i,  v_name[et1 % OFST]);
            add_instr("MLT     %s_arr_size\n", v_name[id]);
            add_instr("P_F2I_M %s\n",  v_name[et2 % OFST]);
            add_instr("S_ADD\n");
            add_instr( "%s     %s\n",  ldv   , v_name[id]);
        }

        // comp in memory and comp const in memory
        if ((get_type(et1) == 3) && (et1 % OFST != 0) && (get_type(et2) == 5) && (et2 % OFST != 0))
        {
            fprintf (stdout, MSG_WARN_IDXS_MESS, line_num+1);

            get_cmp_cst(et2, &etr, &eti);

            add_instr( "%s     %s\n", f2i, v_name[et1 % OFST]);
            add_instr("MLT     %s_arr_size\n"    , v_name[id]);
            add_instr("P_F2I_M %s\n",      v_name[etr % OFST]);
            add_instr("S_ADD\n");
            add_instr( "%s     %s\n", ldv        , v_name[id]);
        }

        // comp in memory and comp in acc
        if ((get_type(et1) == 3) && (et1 % OFST != 0) && (get_type(et2) == 3) && (et2 % OFST == 0))
        {
            fprintf (stdout, MSG_WARN_IDXS_MESS, line_num+1);

            add_instr("SET_P aux_var\n");
            add_instr("F2I\n");
            add_instr("SET   aux_var\n");
            add_instr("F2I_M %s\n",  v_name[et1 % OFST]);
            add_instr("MLT   %s_arr_size\n", v_name[id]);
            add_instr("ADD   aux_var\n");
            add_instr( "%s   %s\n",  ldv   , v_name[id]);
        }

        // comp in memory and comp in memory
        if ((get_type(et1) == 3) && (et1 % OFST != 0) && (get_type(et2) == 3) && (et2 % OFST != 0))
        {
            fprintf (stdout, MSG_WARN_IDXS_MESS, line_num+1);

            add_instr("%s      %s\n", f2i, v_name[et1 % OFST]);
            add_instr("MLT     %s_arr_size\n"    , v_name[id]);
            add_instr("P_F2I_M %s\n",      v_name[et2 % OFST]);
            add_instr("S_ADD\n");
            add_instr ("%s     %s\n", ldv        , v_name[id]);
        }
    }

    // left comp --------------------------------------------------------------

    else
    {
        // int in acc and int in acc
        if ((get_type(et1) == 1) && (et1 % OFST == 0) && (get_type(et2) == 1) && (et2 % OFST == 0))
        {
            add_instr("SET_P aux_var\n");
            add_instr("MLT   %s_arr_size\n", v_name[id]);
            add_instr("ADD   aux_var\n");
            add_instr("SET   aux_var\n");
            add_instr( "%s   %s\n"  , ldv  , v_name[id]);
            add_instr("P_LOD aux_var\n");
            add_instr( "%s   %s_i\n", ldv  , v_name[id]);
        }

        // int in acc and int in memory
        if ((get_type(et1) == 1) && (et1 % OFST == 0) && (get_type(et2) == 1) && (et2 % OFST != 0))
        {
            add_instr("MLT   %s_arr_size\n" , v_name[id]);
            add_instr("ADD   %s\n"  , v_name[et2 % OFST]);
            add_instr("SET   aux_var\n");
            add_instr( "%s   %s\n"  , ldv   , v_name[id]);
            add_instr("P_LOD aux_var\n");
            add_instr( "%s   %s_i\n", ldv   , v_name[id]);
        }

        // int in acc and float in acc
        if ((get_type(et1) == 1) && (et1 % OFST == 0) && (get_type(et2) == 2) && (et2 % OFST == 0))
        {
            fprintf (stdout, MSG_WARN_IDX2_FLOAT, line_num+1);

            add_instr("F2I\n");
            add_instr("SET_P aux_var\n");
            add_instr("MLT   %s_arr_size\n", v_name[id]);
            add_instr("ADD   aux_var\n");
            add_instr("SET   aux_var\n");
            add_instr( "%s   %s\n"  , ldv  , v_name[id]);
            add_instr("P_LOD aux_var\n");
            add_instr( "%s   %s_i\n", ldv  , v_name[id]);
        }

        // int in acc and float in memory
        if ((get_type(et1) == 1) && (et1 % OFST == 0) && (get_type(et2) == 2) && (et2 % OFST != 0))
        {
            fprintf (stdout, MSG_WARN_IDX2_FLOAT, line_num+1);

            add_instr("MLT     %s_arr_size\n", v_name[id]);
            add_instr("P_F2I_M %s\n" , v_name[et2 % OFST]);
            add_instr("S_ADD\n");
            add_instr("SET     aux_var\n");
            add_instr( "%s     %s\n"  , ldv  , v_name[id]);
            add_instr("P_LOD   aux_var\n");
            add_instr( "%s     %s_i\n", ldv  , v_name[id]);
        }

        // int in acc and comp const in memory
        if ((get_type(et1) == 1) && (et1 % OFST == 0) && (get_type(et2) == 5) && (et2 % OFST != 0))
        {
            fprintf (stdout, MSG_WARN_IDX_COMP_GRAB, line_num+1);

            get_cmp_cst(et2, &etr, &eti);

            add_instr("MLT     %s_arr_size\n", v_name[id]);
            add_instr("P_F2I_M %s\n" , v_name[etr % OFST]);
            add_instr("S_ADD\n");
            add_instr("SET     aux_var\n");
            add_instr( "%s     %s\n"  , ldv  , v_name[id]);
            add_instr("P_LOD   aux_var\n");
            add_instr( "%s     %s_i\n", ldv  , v_name[id]);
        }

        // int in acc and comp in acc
        if ((get_type(et1) == 1) && (et1 % OFST == 0) && (get_type(et2) == 3) && (et2 % OFST == 0))
        {
            fprintf (stdout, MSG_WARN_IDX_COMP_GRAB, line_num+1);

            add_instr("SET_P aux_var\n");
            add_instr("F2I\n");
            add_instr("SET_P aux_var\n");
            add_instr("MLT   %s_arr_size\n", v_name[id]);
            add_instr("ADD   aux_var\n");
            add_instr("SET   aux_var\n");
            add_instr( "%s   %s\n"  , ldv  , v_name[id]);
            add_instr("P_LOD aux_var\n");
            add_instr( "%s   %s_i\n", ldv  , v_name[id]);
        }

        // int in acc and comp in memory
        if ((get_type(et1) == 1) && (et1 % OFST == 0) && (get_type(et2) == 3) && (et2 % OFST != 0))
        {
            fprintf (stdout, MSG_WARN_IDX_COMP_GRAB, line_num+1);

            add_instr("MLT     %s_arr_size\n", v_name[id]);
            add_instr("P_F2I_M %s\n",  v_name[et2 % OFST]);
            add_instr("S_ADD\n");
            add_instr("SET     aux_var\n");
            add_instr( "%s     %s\n"  , ldv  , v_name[id]);
            add_instr("P_LOD   aux_var\n");
            add_instr( "%s     %s_i\n", ldv  , v_name[id]);
        }

        // int in memory and int in acc
        if ((get_type(et1) == 1) && (et1 % OFST != 0) && (get_type(et2) == 1) && (et2 % OFST == 0))
        {
            add_instr("P_LOD %s\n",  v_name[et1 % OFST]);
            add_instr("MLT   %s_arr_size\n", v_name[id]);
            add_instr("S_ADD\n");
            add_instr("SET   aux_var\n");
            add_instr( "%s   %s\n"  , ldv  , v_name[id]);
            add_instr("P_LOD aux_var\n");
            add_instr( "%s   %s_i\n", ldv  , v_name[id]);
        }

        // int in memory and int in memory
        if ((get_type(et1) == 1) && (et1 % OFST != 0) && (get_type(et2) == 1) && (et2 % OFST != 0))
        {
            add_instr( "%s   %s\n", ldi, v_name[et1 % OFST]);
            add_instr("MLT   %s_arr_size\n"    , v_name[id]);
            add_instr("ADD   %s\n",      v_name[et2 % OFST]);
            add_instr("SET   aux_var\n");
            add_instr( "%s   %s\n"  , ldv      , v_name[id]);
            add_instr("P_LOD aux_var\n");
            add_instr( "%s   %s_i\n", ldv      , v_name[id]);
        }

        // int in memory and float in acc
        if ((get_type(et1) == 1) && (et1 % OFST != 0) && (get_type(et2) == 2) && (et2 % OFST == 0))
        {
            fprintf (stdout, MSG_WARN_IDX2_FLOAT, line_num+1);

            add_instr("F2I\n");
            add_instr("P_LOD %s\n",  v_name[et1 % OFST]);
            add_instr("MLT   %s_arr_size\n", v_name[id]);
            add_instr("S_ADD\n");
            add_instr("SET   aux_var\n");
            add_instr( "%s   %s\n"  , ldv  , v_name[id]);
            add_instr("P_LOD aux_var\n");
            add_instr( "%s   %s_i\n", ldv  , v_name[id]);
        }

        // int in memory and float in memory
        if ((get_type(et1) == 1) && (et1 % OFST != 0) && (get_type(et2) == 2) && (et2 % OFST != 0))
        {
            fprintf (stdout, MSG_WARN_IDX2_FLOAT, line_num+1);

            add_instr( "%s     %s\n", ldi, v_name[et1 % OFST]);
            add_instr("MLT     %s_arr_size\n"    , v_name[id]);
            add_instr("P_F2I_M %s\n"     , v_name[et2 % OFST]);
            add_instr("S_ADD\n");
            add_instr("SET     aux_var\n");
            add_instr( "%s     %s\n"  , ldv      , v_name[id]);
            add_instr("P_LOD   aux_var\n");
            add_instr( "%s     %s_i\n", ldv      , v_name[id]);
        }

        // int in memory and comp const in memory
        if ((get_type(et1) == 1) && (et1 % OFST != 0) && (get_type(et2) == 5) && (et2 % OFST != 0))
        {
            fprintf (stdout, MSG_WARN_IDX_COMP_GRAB, line_num+1);

            get_cmp_cst(et2, &etr, &eti);

            add_instr( "%s     %s\n", ldi, v_name[et1 % OFST]);
            add_instr("MLT     %s_arr_size\n"    , v_name[id]);
            add_instr("P_F2I_M %s\n"     , v_name[etr % OFST]);
            add_instr("S_ADD\n");
            add_instr("SET     aux_var\n");
            add_instr( "%s     %s\n"  , ldv      , v_name[id]);
            add_instr("P_LOD   aux_var\n");
            add_instr( "%s     %s_i\n", ldv      , v_name[id]);
        }

        // int in memory and comp in acc
        if ((get_type(et1) == 1) && (et1 % OFST != 0) && (get_type(et2) == 3) && (et2 % OFST == 0))
        {
            fprintf (stdout, MSG_WARN_IDX_COMP_GRAB, line_num+1);

            add_instr("SET_P aux_var\n");
            add_instr("F2I\n");
            add_instr("SET   aux_var\n");
            add_instr("LOD   %s\n",   v_name[et1 % OFST]);
            add_instr("MLT   %s_arr_size\n" , v_name[id]);
            add_instr("ADD   aux_var\n");
            add_instr("SET   aux_var\n");
            add_instr( "%s   %s\n"  , ldv   , v_name[id]);
            add_instr("P_LOD aux_var\n");
            add_instr( "%s   %s_i\n", ldv   , v_name[id]);
        }

        // int in memory and comp in memory
        if ((get_type(et1) == 1) && (et1 % OFST != 0) && (get_type(et2) == 3) && (et2 % OFST != 0))
        {
            fprintf (stdout, MSG_WARN_IDX_COMP_GRAB, line_num+1);

            add_instr( "%s     %s\n", ldi, v_name[et1 % OFST]);
            add_instr("MLT     %s_arr_size\n"    , v_name[id]);
            add_instr("P_F2I_M %s\n"     , v_name[et2 % OFST]);
            add_instr("S_ADD\n");
            add_instr("SET     aux_var\n");
            add_instr( "%s     %s\n"  , ldv      , v_name[id]);
            add_instr("P_LOD   aux_var\n");
            add_instr( "%s     %s_i\n", ldv      , v_name[id]);
        }

        // float in acc and int in acc
        if ((get_type(et1) == 2) && (et1 % OFST == 0) && (get_type(et2) == 1) && (et2 % OFST == 0))
        {
            fprintf (stdout, MSG_WARN_IDX1_FLOAT, line_num+1);

            add_instr("SET_P aux_var\n");
            add_instr("F2I\n");
            add_instr("MLT   %s_arr_size\n", v_name[id]);
            add_instr("ADD   aux_var\n");
            add_instr("SET   aux_var\n");
            add_instr( "%s   %s\n"  , ldv  , v_name[id]);
            add_instr("P_LOD aux_var\n");
            add_instr( "%s   %s_i\n", ldv  , v_name[id]);
        }

        // float in acc and int in memory
        if ((get_type(et1) == 2) && (et1 % OFST == 0) && (get_type(et2) == 1) && (et2 % OFST != 0))
        {
            fprintf (stdout, MSG_WARN_IDX1_FLOAT, line_num+1);

            add_instr("F2I\n");
            add_instr("MLT   %s_arr_size\n", v_name[id]);
            add_instr("ADD   %s\n" , v_name[et2 % OFST]);
            add_instr("SET   aux_var\n");
            add_instr( "%s   %s\n"  , ldv  , v_name[id]);
            add_instr("P_LOD aux_var\n");
            add_instr( "%s   %s_i\n", ldv  , v_name[id]);
        }

        // float in acc and float in acc
        if ((get_type(et1) == 2) && (et1 % OFST == 0) && (get_type(et2) == 2) && (et2 % OFST == 0))
        {
            fprintf (stdout, MSG_WARN_IDXS_FLOAT, line_num+1);

            add_instr("F2I\n");
            add_instr("SET_P aux_var\n");
            add_instr("F2I\n");
            add_instr("MLT   %s_arr_size\n", v_name[id]);
            add_instr("ADD   aux_var\n");
            add_instr("SET   aux_var\n");
            add_instr( "%s   %s\n"  , ldv  , v_name[id]);
            add_instr("P_LOD aux_var\n");
            add_instr( "%s   %s_i\n", ldv  , v_name[id]);
        }

        // float in acc and float in memory
        if ((get_type(et1) == 2) && (et1 % OFST == 0) && (get_type(et2) == 2) && (et2 % OFST != 0))
        {
            fprintf (stdout, MSG_WARN_IDXS_MESS, line_num+1);

            add_instr("F2I\n");
            add_instr("MLT     %s_arr_size\n", v_name[id]);
            add_instr("P_F2I_M %s\n" , v_name[et2 % OFST]);
            add_instr("S_ADD\n");
            add_instr("SET     aux_var\n");
            add_instr( "%s     %s\n"  , ldv  , v_name[id]);
            add_instr("P_LOD   aux_var\n");
            add_instr( "%s     %s_i\n", ldv  , v_name[id]);
        }

        // float in acc and comp const in memory
        if ((get_type(et1) == 2) && (et1 % OFST == 0) && (get_type(et2) == 5) && (et2 % OFST != 0))
        {
            fprintf (stdout, MSG_WARN_IDXS_MESS, line_num+1);

            get_cmp_cst(et2, &etr, &eti);

            add_instr("F2I\n");
            add_instr("MLT     %s_arr_size\n", v_name[id]);
            add_instr("P_F2I_M %s\n" , v_name[etr % OFST]);
            add_instr("S_ADD\n");
            add_instr("SET     aux_var\n");
            add_instr( "%s     %s\n"  , ldv  , v_name[id]);
            add_instr("P_LOD   aux_var\n");
            add_instr( "%s     %s_i\n", ldv  , v_name[id]);
        }

        // float in acc and comp in acc
        if ((get_type(et1) == 2) && (et1 % OFST == 0) && (get_type(et2) == 3) && (et2 % OFST == 0))
        {
            fprintf (stdout, MSG_WARN_IDXS_MESS, line_num+1);

            add_instr("SET_P aux_var\n");
            add_instr("F2I\n");
            add_instr("SET_P aux_var\n");
            add_instr("F2I\n");
            add_instr("MLT   %s_arr_size\n", v_name[id]);
            add_instr("ADD   aux_var\n");
            add_instr("SET   aux_var\n");
            add_instr( "%s   %s\n"  , ldv  , v_name[id]);
            add_instr("P_LOD aux_var\n");
            add_instr( "%s   %s_i\n", ldv  , v_name[id]);
        }

        // float in acc and comp in memory
        if ((get_type(et1) == 2) && (et1 % OFST == 0) && (get_type(et2) == 3) && (et2 % OFST != 0))
        {
            fprintf (stdout, MSG_WARN_IDXS_MESS, line_num+1);

            add_instr("F2I\n");
            add_instr("MLT     %s_arr_size\n", v_name[id]);
            add_instr("P_F2I_M %s\n" , v_name[et2 % OFST]);
            add_instr("S_ADD\n");
            add_instr("SET     aux_var\n");
            add_instr( "%s     %s\n"  , ldv  , v_name[id]);
            add_instr("P_LOD   aux_var\n");
            add_instr( "%s     %s_i\n", ldv  , v_name[id]);
        }

        // float in memory and int in acc
        if ((get_type(et1) == 2) && (et1 % OFST != 1) && (get_type(et2) == 1) && (et2 % OFST == 0))
        {
            fprintf (stdout, MSG_WARN_IDX_COMP_GRAB, line_num+1);

            add_instr("F2I\n");
            add_instr("SET   aux_var\n");
            add_instr("F2I_M %s\n" , v_name[et1 % OFST]);
            add_instr("MLT   %s_arr_size\n", v_name[id]);
            add_instr("ADD   aux_var\n");
            add_instr("SET   aux_var\n");
            add_instr( "%s   %s\n"  , ldv  , v_name[id]);
            add_instr("P_LOD aux_var\n");
            add_instr( "%s   %s_i\n", ldv  , v_name[id]);
        }

        // float in memory and int in memory
        if ((get_type(et1) == 2) && (et1 % OFST != 0) && (get_type(et2) == 1) && (et2 % OFST != 0))
        {
            fprintf (stdout, MSG_WARN_IDX_COMP_GRAB, line_num+1);

            add_instr("%s    %s\n", f2i, v_name[et1 % OFST]);
            add_instr("MLT   %s_arr_size\n"    , v_name[id]);
            add_instr("ADD   %s\n"     , v_name[et2 % OFST]);
            add_instr("SET   aux_var\n");
            add_instr( "%s   %s\n"  , ldv      , v_name[id]);
            add_instr("P_LOD aux_var\n");
            add_instr( "%s   %s_i\n", ldv      , v_name[id]);
        }

        // float in memory and float in acc
        if ((get_type(et1) == 2) && (et1 % OFST != 0) && (get_type(et2) == 2) && (et2 % OFST == 0))
        {
            fprintf (stdout, MSG_WARN_IDXS_MESS, line_num+1);

            add_instr("F2I\n");
            add_instr("SET   aux_var\n");
            add_instr("F2I_M %s\n" , v_name[et1 % OFST]);
            add_instr("MLT   %s_arr_size\n", v_name[id]);
            add_instr("ADD   aux_var\n");
            add_instr("SET   aux_var\n");
            add_instr( "%s   %s\n"  , ldv  , v_name[id]);
            add_instr("P_LOD aux_var\n");
            add_instr( "%s   %s_i\n", ldv  , v_name[id]);
        }

        // float in memory and float in memory
        if ((get_type(et1) == 2) && (et1 % OFST != 0) && (get_type(et2) == 2) && (et2 % OFST != 0))
        {
            fprintf (stdout, MSG_WARN_IDXS_MESS, line_num+1);

            add_instr( "%s     %s\n", f2i, v_name[et1 % OFST]);
            add_instr("MLT     %s_arr_size\n"    , v_name[id]);
            add_instr("P_F2I_M %s\n"     , v_name[et2 % OFST]);
            add_instr("S_ADD\n");
            add_instr("SET     aux_var\n");
            add_instr("%s      %s\n"  , ldv      , v_name[id]);
            add_instr("P_LOD   aux_var\n");
            add_instr( "%s     %s_i\n", ldv      , v_name[id]);
        }

        // float in memory and comp const in memory
        if ((get_type(et1) == 2) && (et1 % OFST != 0) && (get_type(et2) == 5) && (et2 % OFST != 0))
        {
            fprintf (stdout, MSG_WARN_IDXS_MESS, line_num+1);

            add_instr( "%s %s\n", f2i, v_name[et1 % OFST]);
            add_instr("MLT %s_arr_size\n"    , v_name[id]);

            get_cmp_cst(et2, &etr, &eti);

            add_instr("P_F2I_M %s\n", v_name[etr % OFST]);
            add_instr("S_ADD\n");
            add_instr("SET     aux_var\n");
            add_instr( "%s     %s\n"  , ldv , v_name[id]);
            add_instr("P_LOD   aux_var\n");
            add_instr( "%s     %s_i\n", ldv , v_name[id]);
        }

        // float in memory and comp in acc
        if ((get_type(et1) == 2) && (et1 % OFST != 0) && (get_type(et2) == 3) && (et2 % OFST == 0))
        {
            fprintf (stdout, MSG_WARN_IDXS_MESS, line_num+1);

            add_instr("SET_P aux_var\n");
            add_instr("F2I\n");
            add_instr("SET   aux_var\n");
            add_instr("F2I_M %s\n" , v_name[et1 % OFST]);
            add_instr("MLT   %s_arr_size\n", v_name[id]);
            add_instr("ADD   aux_var\n");
            add_instr("SET   aux_var\n");
            add_instr( "%s   %s\n"  , ldv  , v_name[id]);
            add_instr("P_LOD aux_var\n");
            add_instr( "%s   %s_i\n", ldv  , v_name[id]);
        }

        // float in memory and comp in memory
        if ((get_type(et1) == 2) && (et1 % OFST != 0) && (get_type(et2) == 3) && (et2 % OFST != 0))
        {
            fprintf (stdout, MSG_WARN_IDXS_MESS, line_num+1);

            add_instr( "%s     %s\n", f2i, v_name[et1 % OFST]);
            add_instr("MLT     %s_arr_size\n"    , v_name[id]);
            add_instr("P_F2I_M %s\n"     , v_name[et2 % OFST]);
            add_instr("S_ADD\n");
            add_instr("SET     aux_var\n");
            add_instr( "%s     %s\n"  , ldv      , v_name[id]);
            add_instr("P_LOD   aux_var\n");
            add_instr( "%s     %s_i\n", ldv      , v_name[id]);
        }

        // comp const in memory and int in acc
        if ((get_type(et1) == 5) && (et1 % OFST != 0) && (get_type(et2) == 1) && (et2 % OFST == 0))
        {
            fprintf (stdout, MSG_WARN_IDX_COMP_GRAB, line_num+1);

            get_cmp_cst(et1, &etr, &eti);

            add_instr("F2I\n");
            add_instr("SET   aux_var\n");
            add_instr("F2I_M %s\n" , v_name[etr % OFST]);
            add_instr("MLT   %s_arr_size\n", v_name[id]);
            add_instr("ADD   aux_var\n");
            add_instr("SET   aux_var\n");
            add_instr( "%s   %s\n"  , ldv  , v_name[id]);
            add_instr("P_LOD aux_var\n");
            add_instr( "%s   %s_i\n", ldv  , v_name[id]);
        }

        // comp const in memory and int in memory
        if ((get_type(et1) == 5) && (et1 % OFST != 0) && (get_type(et2) == 1) && (et2 % OFST != 0))
        {
            fprintf (stdout, MSG_WARN_IDX_COMP_GRAB, line_num+1);

            get_cmp_cst(et1, &etr, &eti);

            add_instr( "%s   %s\n", f2i, v_name[etr % OFST]);
            add_instr("MLT   %s_arr_size\n"    , v_name[id]);
            add_instr("ADD   %s\n",      v_name[et2 % OFST]);
            add_instr("SET   aux_var\n");
            add_instr( "%s   %s\n"  , ldv      , v_name[id]);
            add_instr("P_LOD aux_var\n");
            add_instr( "%s   %s_i\n", ldv      , v_name[id]);
        }

        // comp const in memory and float in acc
        if ((get_type(et1) == 5) && (et1 % OFST != 0) && (get_type(et2) == 2) && (et2 % OFST == 0))
        {
            fprintf (stdout, MSG_WARN_IDXS_MESS, line_num+1);

            get_cmp_cst(et1, &etr, &eti);

            add_instr("F2I\n");
            add_instr("SET   aux_var\n");
            add_instr("F2I_M %s\n",  v_name[etr % OFST]);
            add_instr("MLT   %s_arr_size\n", v_name[id]);
            add_instr("ADD   aux_var\n");
            add_instr("SET   aux_var\n");
            add_instr( "%s   %s\n"  , ldv  , v_name[id]);
            add_instr("P_LOD aux_var\n");
            add_instr( "%s   %s_i\n", ldv  , v_name[id]);
        }

        // comp const in memory and float in memory
        if ((get_type(et1) == 5) && (et1 % OFST != 0) && (get_type(et2) == 2) && (et2 % OFST != 0))
        {
            fprintf (stdout, MSG_WARN_IDXS_MESS, line_num+1);

            get_cmp_cst(et1, &etr, &eti);

            add_instr( "%s     %s\n", f2i, v_name[etr % OFST]);
            add_instr("MLT     %s_arr_size\n"    , v_name[id]);
            add_instr("P_F2I_M %s\n",      v_name[et2 % OFST]);
            add_instr("S_ADD\n");
            add_instr("SET     aux_var\n");
            add_instr( "%s     %s\n"  , ldv      , v_name[id]);
            add_instr("P_LOD   aux_var\n");
            add_instr( "%s     %s_i\n", ldv      , v_name[id]);
        }

        // comp const in memory and comp const in memory
        if ((get_type(et1) == 5) && (et1 % OFST != 0) && (get_type(et2) == 5) && (et2 % OFST != 0))
        {
            fprintf (stdout, MSG_WARN_IDXS_MESS, line_num+1);

            get_cmp_cst(et1, &etr, &eti);
            add_instr( "%s %s\n", f2i, v_name[etr % OFST]);
            add_instr("MLT %s_arr_size\n"    , v_name[id]);

            get_cmp_cst(et2, &etr, &eti);
            add_instr("P_F2I_M %s\n", v_name[etr % OFST]);
            add_instr("S_ADD\n");
            add_instr("SET     aux_var\n");
            add_instr( "%s     %s\n"  , ldv , v_name[id]);
            add_instr("P_LOD   aux_var\n");
            add_instr( "%s     %s_i\n", ldv , v_name[id]);
        }

        // comp const in memory and comp in acc
        if ((get_type(et1) == 5) && (et1 % OFST != 0) && (get_type(et2) == 3) && (et2 % OFST == 0))
        {
            fprintf (stdout, MSG_WARN_IDXS_MESS, line_num+1);

            get_cmp_cst(et1, &etr, &eti);

            add_instr("SET_P aux_var\n");
            add_instr("F2I\n");
            add_instr("SET   aux_var\n");
            add_instr("F2I_M %s\n",  v_name[etr % OFST]);
            add_instr("MLT   %s_arr_size\n", v_name[id]);
            add_instr("ADD   aux_var\n");
            add_instr("SET   aux_var\n");
            add_instr( "%s   %s\n"  , ldv  , v_name[id]);
            add_instr("P_LOD aux_var\n");
            add_instr( "%s   %s_i\n", ldv  , v_name[id]);
        }

        // comp const in memory and comp in memory
        if ((get_type(et1) == 5) && (et1 % OFST != 0) && (get_type(et2) == 3) && (et2 % OFST != 0))
        {
            fprintf (stdout, MSG_WARN_IDXS_MESS, line_num+1);

            get_cmp_cst(et1, &etr, &eti);

            add_instr("%s      %s\n", f2i, v_name[etr % OFST]);
            add_instr("MLT     %s_arr_size\n"    , v_name[id]);
            add_instr("P_F2I_M %s\n"     , v_name[et2 % OFST]);
            add_instr("S_ADD\n");
            add_instr("SET     aux_var\n");
            add_instr( "%s     %s\n"  , ldv      , v_name[id]);
            add_instr("P_LOD   aux_var\n");
            add_instr( "%s     %s_i\n", ldv      , v_name[id]);
        }

        // comp in acc and int in acc
        if ((get_type(et1) == 3) && (et1 % OFST == 0) && (get_type(et2) == 1) && (et2 % OFST == 0))
        {
            fprintf (stdout, MSG_WARN_IDX_COMP_GRAB, line_num+1);

            add_instr("SET_P aux_var\n");
            add_instr("POP\n");
            add_instr("F2I\n");
            add_instr("MLT   %s_arr_size\n", v_name[id]);
            add_instr("ADD   aux_var\n");
            add_instr("SET   aux_var\n");
            add_instr( "%s   %s\n"  , ldv  , v_name[id]);
            add_instr("P_LOD aux_var\n");
            add_instr( "%s   %s_i\n", ldv  , v_name[id]);
        }

        // comp in acc and int in memory
        if ((get_type(et1) == 3) && (et1 % OFST == 0) && (get_type(et2) == 1) && (et2 % OFST != 0))
        {
            fprintf (stdout, MSG_WARN_IDX_COMP_GRAB, line_num+1);

            add_instr("SET_P aux_var\n");
            add_instr("F2I\n");
            add_instr("MLT   %s_arr_size\n", v_name[id]);
            add_instr("ADD   %s\n" , v_name[et2 % OFST]);
            add_instr("SET   aux_var\n");
            add_instr( "%s   %s\n"  , ldv  , v_name[id]);
            add_instr("P_LOD aux_var\n");
            add_instr( "%s   %s_i\n", ldv  , v_name[id]);
        }

        // comp in acc and float in acc
        if ((get_type(et1) == 3) && (et1 % OFST == 0) && (get_type(et2) == 2) && (et2 % OFST == 0))
        {
            fprintf (stdout, MSG_WARN_IDXS_MESS, line_num+1);

            add_instr("F2I\n");
            add_instr("SET_P aux_var\n");
            add_instr("POP\n");
            add_instr("F2I\n");
            add_instr("MLT   %s_arr_size\n", v_name[id]);
            add_instr("ADD   aux_var\n");
            add_instr("SET   aux_var\n");
            add_instr("%s    %s\n"  , ldv  , v_name[id]);
            add_instr("P_LOD aux_var\n");
            add_instr("%s    %s_i\n", ldv  , v_name[id]);
        }

        // comp in acc and float in memory
        if ((get_type(et1) == 3) && (et1 % OFST == 0) && (get_type(et2) == 2) && (et2 % OFST != 0))
        {
            fprintf (stdout, MSG_WARN_IDXS_MESS, line_num+1);

            add_instr("SET_P   aux_var\n");
            add_instr("F2I\n");
            add_instr("MLT     %s_arr_size\n", v_name[id]);
            add_instr("P_F2I_M %s\n" , v_name[et2 % OFST]);
            add_instr("S_ADD\n");
            add_instr("SET     aux_var\n");
            add_instr ("%s     %s\n"  , ldv  , v_name[id]);
            add_instr("P_LOD   aux_var\n");
            add_instr( "%s     %s_i\n", ldv  , v_name[id]);
        }

        // comp in acc and comp const in memory
        if ((get_type(et1) == 3) && (et1 % OFST == 0) && (get_type(et2) == 5) && (et2 % OFST != 0))
        {
            fprintf (stdout, MSG_WARN_IDXS_MESS, line_num+1);

            get_cmp_cst(et2, &etr, &eti);

            add_instr("SET_P   aux_var\n");
            add_instr("F2I\n");
            add_instr("MLT     %s_arr_size\n", v_name[id]);
            add_instr("P_F2I_M %s\n" , v_name[etr % OFST]);
            add_instr("S_ADD\n");
            add_instr("SET     aux_var\n");
            add_instr( "%s     %s\n"  , ldv  , v_name[id]);
            add_instr("P_LOD   aux_var\n");
            add_instr( "%s     %s_i\n", ldv  , v_name[id]);
        }

        // comp in acc and comp in acc
        if ((get_type(et1) == 3) && (et1 % OFST == 0) && (get_type(et2) == 3) && (et2 % OFST == 0))
        {
            fprintf (stdout, MSG_WARN_IDXS_MESS, line_num+1);

            add_instr("SET_P aux_var\n");
            add_instr("F2I\n");
            add_instr("SET_P aux_var\n");
            add_instr("POP\n");
            add_instr("F2I\n");
            add_instr("MLT   %s_arr_size\n", v_name[id]);
            add_instr("ADD   aux_var\n");
            add_instr("SET   aux_var\n");
            add_instr( "%s   %s\n"  , ldv  , v_name[id]);
            add_instr("P_LOD aux_var\n");
            add_instr( "%s   %s_i\n", ldv  , v_name[id]);
        }

        // comp in acc and comp in memory
        if ((get_type(et1) == 3) && (et1 % OFST == 0) && (get_type(et2) == 3) && (et2 % OFST != 0))
        {
            fprintf (stdout, MSG_WARN_IDXS_MESS, line_num+1);

            add_instr("SET_P   aux_var\n");
            add_instr("F2I\n");
            add_instr("MLT     %s_arr_size\n", v_name[id]);
            add_instr("P_F2I_M %s\n" , v_name[et2 % OFST]);
            add_instr("S_ADD\n");
            add_instr("SET     aux_var\n");
            add_instr( "%s     %s\n"  , ldv  , v_name[id]);
            add_instr("P_LOD   aux_var\n");
            add_instr( "%s     %s_i\n", ldv  , v_name[id]);
        }

        // comp in memory and int in acc
        if ((get_type(et1) == 3) && (et1 % OFST != 0) && (get_type(et2) == 1) && (et2 % OFST == 0))
        {
            fprintf (stdout, MSG_WARN_IDX_COMP_GRAB, line_num+1);

            add_instr("P_F2I_M %s\n" , v_name[et1 % OFST]);
            add_instr("MLT     %s_arr_size\n", v_name[id]);
            add_instr("S_ADD\n");
            add_instr("SET     aux_var\n");
            add_instr("%s      %s\n"  , ldv  , v_name[id]);
            add_instr("P_LOD   aux_var\n");
            add_instr("%s      %s_i\n", ldv  , v_name[id]);
        }

        // comp in memory and int in memory
        if ((get_type(et1) == 3) && (et1 % OFST != 0) && (get_type(et2) == 1) && (et2 % OFST != 0))
        {
            fprintf (stdout, MSG_WARN_IDX_COMP_GRAB, line_num+1);

            add_instr("%s    %s\n", f2i, v_name[et1 % OFST]);
            add_instr("MLT   %s_arr_size\n"    , v_name[id]);
            add_instr("ADD   %s\n",      v_name[et2 % OFST]);
            add_instr("SET   aux_var\n");
            add_instr( "%s   %s\n"  , ldv      , v_name[id]);
            add_instr("P_LOD aux_var\n");
            add_instr( "%s   %s_i\n", ldv      , v_name[id]);
        }

        // comp in memory and float in acc
        if ((get_type(et1) == 3) && (et1 % OFST != 0) && (get_type(et2) == 2) && (et2 % OFST == 0))
        {
            fprintf (stdout, MSG_WARN_IDXS_MESS, line_num+1);

            add_instr("F2I\n");
            add_instr("P_F2I_M %s\n" , v_name[et1 % OFST]);
            add_instr("MLT     %s_arr_size\n", v_name[id]);
            add_instr("S_ADD\n");
            add_instr("SET     aux_var\n");
            add_instr( "%s     %s\n"  , ldv  , v_name[id]);
            add_instr("P_LOD   aux_var\n");
            add_instr( "%s     %s_i\n", ldv  , v_name[id]);
        }

        // comp in memory and float in memory
        if ((get_type(et1) == 3) && (et1 % OFST != 0) && (get_type(et2) == 2) && (et2 % OFST != 0))
        {
            fprintf (stdout, MSG_WARN_IDXS_MESS, line_num+1);

            add_instr( "%s     %s\n", f2i, v_name[et1 % OFST]);
            add_instr("MLT     %s_arr_size\n"    , v_name[id]);
            add_instr("P_F2I_M %s\n",      v_name[et2 % OFST]);
            add_instr("S_ADD\n");
            add_instr("SET     aux_var\n");
            add_instr( "%s     %s\n"  , ldv      , v_name[id]);
            add_instr("P_LOD   aux_var\n");
            add_instr( "%s     %s_i\n", ldv      , v_name[id]);
        }

        // comp in memory and comp const in memory
        if ((get_type(et1) == 3) && (et1 % OFST != 0) && (get_type(et2) == 5) && (et2 % OFST != 0))
        {
            fprintf (stdout, MSG_WARN_IDXS_MESS, line_num+1);

            get_cmp_cst(et2, &etr, &eti);

            add_instr( "%s     %s\n", f2i, v_name[et1 % OFST]);
            add_instr("MLT     %s_arr_size\n"    , v_name[id]);
            add_instr("P_F2I_M %s\n",      v_name[etr % OFST]);
            add_instr("S_ADD\n");
            add_instr("SET     aux_var\n");
            add_instr( "%s     %s\n"  , ldv      , v_name[id]);
            add_instr("P_LOD   aux_var\n");
            add_instr( "%s     %s_i\n", ldv      , v_name[id]);
        }

        // comp in memory and comp in acc
        if ((get_type(et1) == 3) && (et1 % OFST != 0) && (get_type(et2) == 3) && (et2 % OFST == 0))
        {
            fprintf (stdout, MSG_WARN_IDXS_MESS, line_num+1);

            add_instr("SET_P aux_var\n");
            add_instr("F2I\n");
            add_instr("SET   aux_var\n");
            add_instr("F2I_M %s\n" , v_name[et1 % OFST]);
            add_instr("MLT   %s_arr_size\n", v_name[id]);
            add_instr("ADD   aux_var\n");
            add_instr("SET   aux_var\n");
            add_instr( "%s   %s\n", ldv    , v_name[id]);
            add_instr("P_LOD aux_var\n");
            add_instr( "%s %s_i\n", ldv    , v_name[id]);
        }

        // comp in memory and comp in memory
        if ((get_type(et1) == 3) && (et1 % OFST != 0) && (get_type(et2) == 3) && (et2 % OFST != 0))
        {
            fprintf (stdout, MSG_WARN_IDXS_MESS, line_num+1);

            add_instr(" %s     %s\n", f2i, v_name[et1 % OFST]);
            add_instr("MLT     %s_arr_size\n"    , v_name[id]);
            add_instr("P_F2I_M %s\n"     , v_name[et2 % OFST]);
            add_instr("S_ADD\n");
            add_instr("SET     aux_var\n");
            add_instr(" %s     %s\n"  , ldv      , v_name[id]);
            add_instr("P_LOD   aux_var\n");
            add_instr(" %s     %s_i\n", ldv      , v_name[id]);
        }
    }

    acc_ok     = 1;
    v_used[id] = 1;

    return v_type[id]*OFST;
}