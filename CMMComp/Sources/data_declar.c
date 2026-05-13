// ----------------------------------------------------------------------------
// variable declaration -------------------------------------------------------
// ----------------------------------------------------------------------------

/*
TODO:
1- think of more useful initializations using Dirac notation
*/

// global includes
#include <string.h>
#include <stdlib.h>
#include  <stdio.h>

// local includes
#include "..\Headers\t2t.h"
#include "..\Headers\global.h"
#include "..\Headers\stdlib.h"
#include "..\Headers\funcoes.h"
#include "..\Headers\variaveis.h"
#include "..\Headers\messages.h"

// ----------------------------------------------------------------------------
// global variable definitions ------------------------------------------------
// ----------------------------------------------------------------------------

int type_tmp; // captures the type when a variable is declared (see c2asm.l)

// ----------------------------------------------------------------------------
// standard declarations ------------------------------------------------------
// ----------------------------------------------------------------------------

// declares a variable (non-array)
void declar_var(int id)
{
    // consistency check ------------------------------------------------------

    if (v_type[id] != 0)
    {
        fprintf(stderr, MSG_ERR_VAR_EXISTS, line_num+1, rem_fname(v_name[id], fname));
        exit(EXIT_FAILURE);
    }

    // update the variable status ---------------------------------------------

    v_type[id] = type_tmp;               // the variable type is stored in type_tmp (see flex when it finds int, float or comp)
    v_used[id] = 0;                      // just declared, so not used yet
    v_fnid[id] = find_var(fname);        // record the function it belongs to

    // declare the imaginary part if comp -------------------------------------

    if (type_tmp > 2)
    {
        int idi     = get_img_id(id);
        v_type[idi] = 4; // use type 4 for the imaginary part
        v_used[idi] = 0;
        v_fnid[idi] = find_var(fname);
    }

    // register the variable in the log file ----------------------------------

    // save the function it belongs to
    char func[256]; if (strcmp(fname,"")==0) strcpy(func, "global"); else strcpy(func, fname);
    // save the data in the log file
    fprintf(f_log, "%s %s %d\n", func, rem_fname(v_name[id], fname), type_tmp);
    // if comp, save the imaginary part too
    if (type_tmp == 3) fprintf(f_log, "%s %s_i 3\n", func, rem_fname(v_name[id], fname));
}

// declares a 1D array
void declar_arr_1d(int id_var, int id_arg, int id_fname)
{
    // consistency check ------------------------------------------------------

    if (v_type[id_var] != 0) // variable already exists
    {
        fprintf (stderr, MSG_ERR_VAR_EXISTS, line_num+1, rem_fname(v_name[id_var], fname));
        exit(EXIT_FAILURE);
    }

    // update the array status ------------------------------------------------

    v_type[id_var] = type_tmp;               // the variable type is stored in type_tmp (see flex when it finds int, float or comp)
    v_used[id_var] = 0;                      // just declared, so not used yet
    v_fnid[id_var] = find_var(fname);        // record the function it belongs to
    v_isar[id_var] = 1;                      // variable is a 1D array
    v_size[id_var] = atoi(v_name[id_arg]);   // record the array size

    // register the array in the log file -------------------------------------

    int type = type_tmp;

    // save the function it belongs to
    char func[256]; if (strcmp(fname,"")==0) strcpy(func, "global"); else strcpy(func, fname);
    // save the data in the log file
    if (sim_arr == 1)
    {
                       fprintf(f_log, "%s %s   %d %s\n", func, rem_fname(v_name[id_var], fname), type_tmp, v_name[id_arg]);
        if (type == 3) fprintf(f_log, "%s %s_i %d %s\n", func, rem_fname(v_name[id_var], fname), type_tmp, v_name[id_arg]);
    }

    // emit --------------------------------------------------------------------

    // int type, no file
    if ((type == 1) && (id_fname == -1))
    {
        add_sinst(0, "#array %s 1 %s\n", v_name[id_var], v_name[id_arg]);
    }

    // int type, with file
    if ((type == 1) && (id_fname != -1))
    {
        add_sinst(0, "#arrays %s 1 %s %s\n", v_name[id_var], v_name[id_arg], v_name[id_fname]);
        printf(MSG_INFO_ARRAY_FILE_INIT, v_name[id_fname], rem_fname(v_name[id_var],fname), line_num+1);
    }

    // float type, no file
    if ((type == 2) && (id_fname == -1))
    {
        add_sinst(0, "#array %s 2 %s\n", v_name[id_var], v_name[id_arg]);
    }

    // float type, with file
    if ((type == 2) && (id_fname != -1))
    {
        add_sinst(0, "#arrays %s 2 %s %s\n", v_name[id_var], v_name[id_arg], v_name[id_fname]);
        printf(MSG_INFO_ARRAY_FILE_INIT, v_name[id_fname], rem_fname(v_name[id_var],fname), line_num+1);
    }

    // comp type, no file
    if ((type == 3) && (id_fname == -1))
    {
        add_sinst(0, "#array %s 3 %s\n", v_name[id_var], v_name[id_arg]);
        id_var = get_img_id(id_var);
        add_sinst(0, "#array %s 4 %s\n", v_name[id_var], v_name[id_arg]);

        v_isar[id_var] = 1; // variable is a 1D array
    }

    // comp type, with file
    if ((type == 3) && (id_fname != -1))
    {
        add_sinst(0, "#arrays %s 3 %s %s\n", v_name[id_var], v_name[id_arg], v_name[id_fname]);
        id_var = get_img_id(id_var);
        add_sinst(0, "#arrays %s 4 %s %s\n", v_name[id_var], v_name[id_arg], v_name[id_fname]);

        v_isar[id_var] = 1; // variable is a 1D array

        printf(MSG_INFO_ARRAY_FILE_INIT, v_name[id_fname], rem_fname(v_name[id_var],fname), line_num+1);
    }
}

// declares a 2D array
void declar_arr_2d(int id_var, int id_x, int id_y, int id_fname)
{
    int idi;

    // size of the equivalent 1D array
    int size = atoi(v_name[id_x])*atoi(v_name[id_y]);

    if (v_type[id_var] != 0) // variable already exists
    {
        fprintf (stderr, MSG_ERR_VAR_EXISTS, line_num+1, rem_fname(v_name[id_var], fname));
        exit(EXIT_FAILURE);
    }

    v_type[id_var] = type_tmp;           // the variable type is stored in type_tmp (see flex when it finds int, float or comp)
    v_used[id_var] = 0;                  // just declared, so not used yet
    v_fnid[id_var] = find_var(fname);    // record the function it belongs to
    v_isar[id_var] = 2;                  // variable is a 2D array
    v_size[id_var] = atoi(v_name[id_x]); // record the i dimension size
    v_siz2[id_var] = atoi(v_name[id_y]); // record the j dimension size

    int type = type_tmp;

    // int type, no file
    if ((type == 1) && (id_fname == -1))
    {
        add_sinst(0, "#array %s 1 %d\n", v_name[id_var], size);
    }

    // int type, with file
    if ((type == 1) && (id_fname != -1))
    {
        add_sinst(0, "#arrays %s 1 %d %s\n", v_name[id_var], size, v_name[id_fname]);
        printf(MSG_INFO_ARRAY_FILE_INIT, v_name[id_fname], rem_fname(v_name[id_var],fname), line_num+1);
    }

    // float type, no file
    if ((type == 2) && (id_fname == -1))
    {
        add_sinst(0, "#array %s 2 %d\n", v_name[id_var], size);
    }

    // float type, with file
    if ((type == 2) && (id_fname != -1))
    {
        add_sinst(0, "#arrays %s 2 %d %s\n", v_name[id_var], size, v_name[id_fname]);
        printf(MSG_INFO_ARRAY_FILE_INIT, v_name[id_fname], rem_fname(v_name[id_var],fname), line_num+1);
    }

    // comp type, no file
    if ((type == 3) && (id_fname == -1))
    {
        add_sinst(0, "#array %s 3 %d\n", v_name[id_var], size);
        idi = get_img_id(id_var);
        add_sinst(0, "#array %s 4 %d\n", v_name[idi]   , size);

        v_isar[idi] = 2; // variable is a 2D array
    }

    // comp type, with file
    if ((type == 3) && (id_fname != -1))
    {
        add_sinst(0, "#arrays %s 3 %d %s\n", v_name[id_var], size, v_name[id_fname]);
        idi = get_img_id(id_var);
        add_sinst(0, "#arrays %s 4 %d %s\n", v_name[idi]   , size, v_name[id_fname]);

        v_isar[idi] = 2; // variable is a 2D array

        printf(MSG_INFO_ARRAY_FILE_INIT, v_name[id_fname], rem_fname(v_name[id_var],fname), line_num+1);
    }

    // create a helper variable to store the size of the x dimension
    add_instr("LOD %s\n",          v_name[id_y  ]);
    add_instr("SET %s_arr_size\n", v_name[id_var]);
}

// ----------------------------------------------------------------------------
// array declarations initialized with Dirac notation -------------------------
// ----------------------------------------------------------------------------

// added on demand

// declares a 1D array as a matrix-vector product, e.g. float A[4,4] # |B|a>;
void declar_Mv(int id_name, int id_N, int id_M, int id_v)
{
    declar_arr_1d(id_name,id_N,  -1);
    exec_Mv      (id_name,id_M,id_v);
}

// declares a 1D array as a constant-vector product, e.g. float a[4] # c|a>;
void declar_cv(int id_name, int id_N, int id_c, int id_v)
{
    declar_arr_1d(id_name,id_N,  -1);
    exec_cv      (id_name,id_c,id_v);
}
