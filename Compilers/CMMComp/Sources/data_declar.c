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
#include "../Headers/t2t.h"
#include "../Headers/ast.h"
#include "../Headers/global.h"
#include "../Headers/stdlib.h"
#include "../Headers/funcoes.h"
#include "../Headers/variaveis.h"
#include "../Headers/messages.h"

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

    if (v_table[id].type != 0)
    {
        fprintf(stderr, MSG_ERR_VAR_EXISTS, line_num+1, rem_fname(v_table[id].name, fname));
        exit(EXIT_FAILURE);
    }

    // update the variable status ---------------------------------------------

    v_table[id].type          = type_tmp;            // the variable type is stored in type_tmp (see flex when it finds int, float or comp)
    v_table[id].used          = 0;                   // just declared, so not used yet
    v_table[id].fnid    = find_var(fname);     // record the function it belongs to

    // declare the imaginary part if comp -------------------------------------

    if (type_tmp > 2)
    {
        int idi              = get_img_id(id);
        v_table[idi].type          = 4; // use type 4 for the imaginary part
        v_table[idi].used          = 0;
        v_table[idi].fnid    = find_var(fname);
    }

    // register the variable in the log file ----------------------------------

    // save the function it belongs to
    char func[256]; if (strcmp(fname,"")==0) strcpy(func, "global"); else strcpy(func, fname);
    // save the data in the log file
    fprintf(f_log, "%s %s %d\n", func, rem_fname(v_table[id].name, fname), type_tmp);
    // if comp, save the imaginary part too
    if (type_tmp == 3) fprintf(f_log, "%s %s_i 3\n", func, rem_fname(v_table[id].name, fname));
}

// Parse-time half of declar_arr_1d: v_table update + log / info messages.
// Called immediately so subsequent declarations can resolve this var.
static void declar_arr_1d_parse(int id_var, int id_arg, int id_fname)
{
    if (v_table[id_var].type != 0)
    {
        fprintf (stderr, MSG_ERR_VAR_EXISTS, line_num+1, rem_fname(v_table[id_var].name, fname));
        exit(EXIT_FAILURE);
    }

    v_table[id_var].type       = type_tmp;
    v_table[id_var].used       = 0;
    v_table[id_var].fnid = find_var(fname);
    v_table[id_var].isar       = 1;
    v_table[id_var].size       = atoi(v_table[id_arg].name);

    int type = type_tmp;

    char func[256]; if (strcmp(fname,"")==0) strcpy(func, "global"); else strcpy(func, fname);
    if (sim_arr == 1)
    {
                       fprintf(f_log, "%s %s   %d %s\n", func, rem_fname(v_table[id_var].name, fname), type_tmp, v_table[id_arg].name);
        if (type == 3) fprintf(f_log, "%s %s_i %d %s\n", func, rem_fname(v_table[id_var].name, fname), type_tmp, v_table[id_arg].name);
    }

    // for comp, mark the imaginary half as a 1D array too
    if (type == 3) v_table[get_img_id(id_var)].isar = 1;

    if (id_fname != -1)
        printf(MSG_INFO_ARRAY_FILE_INIT, v_table[id_fname].name, rem_fname(v_table[id_var].name, fname), line_num+1);
}

// Walker-time half: only the `#array` / `#arrays` directives.
void declar_arr_1d_emit(int id_var, int id_arg, int id_fname)
{
    int type = v_table[id_var].type;

    if (type == 1)
    {
        if (id_fname == -1) add_sinst(0, "#array %s 1 %s\n",     v_table[id_var].name, v_table[id_arg].name);
        else                add_sinst(0, "#arrays %s 1 %s %s\n", v_table[id_var].name, v_table[id_arg].name, v_table[id_fname].name);
    }
    else if (type == 2)
    {
        if (id_fname == -1) add_sinst(0, "#array %s 2 %s\n",     v_table[id_var].name, v_table[id_arg].name);
        else                add_sinst(0, "#arrays %s 2 %s %s\n", v_table[id_var].name, v_table[id_arg].name, v_table[id_fname].name);
    }
    else if (type == 3)
    {
        int idi = get_img_id(id_var);
        if (id_fname == -1)
        {
            add_sinst(0, "#array %s 3 %s\n", v_table[id_var].name, v_table[id_arg].name);
            add_sinst(0, "#array %s 4 %s\n", v_table[idi].name,    v_table[id_arg].name);
        }
        else
        {
            add_sinst(0, "#arrays %s 3 %s %s\n", v_table[id_var].name, v_table[id_arg].name, v_table[id_fname].name);
            add_sinst(0, "#arrays %s 4 %s %s\n", v_table[idi].name,    v_table[id_arg].name, v_table[id_fname].name);
        }
    }
}

// Public entry: parse-time work + defer the directive emit via STMT_DECLAR_ARR_1D.
void declar_arr_1d(int id_var, int id_arg, int id_fname)
{
    declar_arr_1d_parse(id_var, id_arg, id_fname);
    stmt_append(stmt_declar_arr_1d(id_var, id_arg, id_fname));
}

// Parse-time half of declar_arr_2d.
static void declar_arr_2d_parse(int id_var, int id_x, int id_y, int id_fname)
{
    if (v_table[id_var].type != 0)
    {
        fprintf (stderr, MSG_ERR_VAR_EXISTS, line_num+1, rem_fname(v_table[id_var].name, fname));
        exit(EXIT_FAILURE);
    }

    v_table[id_var].type       = type_tmp;
    v_table[id_var].used       = 0;
    v_table[id_var].fnid = find_var(fname);
    v_table[id_var].isar       = 2;
    v_table[id_var].size       = atoi(v_table[id_x].name);
    v_table[id_var].siz2 = atoi(v_table[id_y].name);

    int type = type_tmp;

    if (type == 3) v_table[get_img_id(id_var)].isar = 2;  // comp imag also 2D

    if (id_fname != -1)
        printf(MSG_INFO_ARRAY_FILE_INIT, v_table[id_fname].name, rem_fname(v_table[id_var].name, fname), line_num+1);
}

// Walker-time half: `#array` directive + the LOD/SET arr_size helper instrs.
void declar_arr_2d_emit(int id_var, int id_x, int id_y, int id_fname)
{
    int type = v_table[id_var].type;
    int size = atoi(v_table[id_x].name) * atoi(v_table[id_y].name);

    if (type == 1)
    {
        if (id_fname == -1) add_sinst(0, "#array %s 1 %d\n",     v_table[id_var].name, size);
        else                add_sinst(0, "#arrays %s 1 %d %s\n", v_table[id_var].name, size, v_table[id_fname].name);
    }
    else if (type == 2)
    {
        if (id_fname == -1) add_sinst(0, "#array %s 2 %d\n",     v_table[id_var].name, size);
        else                add_sinst(0, "#arrays %s 2 %d %s\n", v_table[id_var].name, size, v_table[id_fname].name);
    }
    else if (type == 3)
    {
        int idi = get_img_id(id_var);
        if (id_fname == -1)
        {
            add_sinst(0, "#array %s 3 %d\n", v_table[id_var].name, size);
            add_sinst(0, "#array %s 4 %d\n", v_table[idi].name,    size);
        }
        else
        {
            add_sinst(0, "#arrays %s 3 %d %s\n", v_table[id_var].name, size, v_table[id_fname].name);
            add_sinst(0, "#arrays %s 4 %d %s\n", v_table[idi].name,    size, v_table[id_fname].name);
        }
    }

    // helper variable: x-dimension size, used by 2D index flattening
    add_instr("LOD %s\n",          v_table[id_y  ].name);
    add_instr("SET %s_arr_size\n", v_table[id_var].name);
}

// Public entry.
void declar_arr_2d(int id_var, int id_x, int id_y, int id_fname)
{
    declar_arr_2d_parse(id_var, id_x, id_y, id_fname);
    stmt_append(stmt_declar_arr_2d(id_var, id_x, id_y, id_fname));
}

// ----------------------------------------------------------------------------
// array declarations initialized with Dirac notation -------------------------
// ----------------------------------------------------------------------------

// added on demand

// declares a 1D array as a matrix-vector product, e.g. float v[4] # |M|u>;
// Parse-time work is identical to a plain `float v[N]` declaration; the
// walker (STMT_DECLAR_MV case in ast.c) emits the #array directive plus the
// exec_Mv codegen for the matrix product.
void declar_Mv(int id_name, int id_N, int id_M, int id_v)
{
    declar_arr_1d_parse(id_name, id_N, -1);
    stmt_append(stmt_declar_Mv(id_name, id_N, id_M, id_v));
}

// declares a 1D array as a constant-vector product, e.g. float a[4] # c|b>;
// Takes the coefficient as a raw expr_node so the walker (STMT_DECLAR_CV)
// drives the scalar's codegen at body-close time, in source order.
void declar_cv(int id_name, int id_N, expr_node *c, int id_v)
{
    declar_arr_1d_parse(id_name, id_N, -1);
    stmt_append(stmt_declar_cv(id_name, id_N, c, id_v));
}
