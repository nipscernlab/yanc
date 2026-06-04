// ----------------------------------------------------------------------------
// assignment instruction implementation --------------------------------------
// ----------------------------------------------------------------------------

/*
TODO:
1- revisit the ++ increment operators
2- AST will save a lot of code with comp
*/

// global includes
#include <string.h>
#include <stdlib.h>
#include  <stdio.h>

// local includes
#include "../Headers/t2t.h"
#include "../Headers/global.h"
#include "../Headers/funcoes.h"
#include "../Headers/data_use.h"
#include "../Headers/variaveis.h"
#include "../Headers/messages.h"

// standard assignment, e.g. x = y;
void ass_set(int id, expr e)
{
    // test whether it has been declared so an assignment is allowed
    if (v_table[id].type == 0)
    {
        fprintf (stderr, MSG_ERR_DECLARE_VAR_PLEASE, line_num+1, rem_fname(v_table[id].name, fname));
        exit(EXIT_FAILURE);
    }

    // test whether it is an array missing its index
    if (v_table[id].isar > 0)
    {
        fprintf (stderr, MSG_ERR_ARRAY_NEEDS_IDX, line_num+1, rem_fname(v_table[id].name, fname));
        exit(EXIT_FAILURE);
    }

    // ------------------------------------------------------------------------
    // perform the assignment -------------------------------------------------
    // ------------------------------------------------------------------------

    expr etr, eti;

    // left int and right int in memory ---------------------------------------

    if ((v_table[id].type == 1) && (e.type == 1) && (e.id != 0))
    {
        add_instr("LOD %s\n",  v_table[e.id].name);
        add_instr("SET %s\n" , v_table[id       ].name);
    }

    // left int and right int in the acc --------------------------------------

    if ((v_table[id].type == 1) && (e.type == 1) && (e.id == 0))
    {
        add_instr("SET %s\n", v_table[id].name);
    }

    // left int and right float in memory -------------------------------------

    if ((v_table[id].type == 1) && (e.type == 2) && (e.id != 0))
    {
        fprintf(stdout, MSG_WARN_INT_RECV_FLOAT, line_num+1, rem_fname(v_table[id].name, fname));

        add_instr("F2I_M %s\n", v_table[e.id].name);
        add_instr("SET %s\n"  , v_table[id       ].name);
    }

    // left int and right float in the acc ------------------------------------

    if ((v_table[id].type == 1) && (e.type == 2) && (e.id == 0))
    {
        fprintf(stdout, MSG_WARN_INT_RECV_FLOAT, line_num+1, rem_fname(v_table[id].name, fname));

        // consider an instruction that does f2i on the accumulator and stores in memory at once (FIAS)
        add_instr("F2I\n");
        add_instr("SET %s\n", v_table[id].name);
    }

    // left int and right comp const ------------------------------------------

    if ((v_table[id].type == 1) && (e.type == 5))
    {
        fprintf (stdout, MSG_WARN_ROUND_REAL, line_num+1);

        get_cmp_cst(e,&etr,&eti);

        add_instr("F2I_M %s\n", v_table[etr.id].name);
        add_instr("SET %s\n"  , v_table[id        ].name);
    }

    // left int and right comp in memory --------------------------------------

    if ((v_table[id].type == 1) && (e.type == 3) && (e.id != 0))
    {
        fprintf (stdout, MSG_WARN_ROUND_REAL, line_num+1);

        add_instr("F2I_M %s\n", v_table[e.id].name);
        add_instr("SET %s\n"  , v_table[id       ].name);
    }

    // left int and right comp in the acc -------------------------------------

    if ((v_table[id].type == 1) && (e.type == 3) && (e.id == 0))
    {
        fprintf (stdout, MSG_WARN_ROUND_REAL, line_num+1);

        add_instr("POP\n");
        add_instr("F2I\n");
        add_instr("SET %s\n", v_table[id].name);
    }

    // left float and right int in memory -------------------------------------

    if ((v_table[id].type == 2) && (e.type == 1) && (e.id != 0))
    {
        fprintf(stdout, MSG_WARN_FLOAT_RECV_INT, line_num+1, rem_fname(v_table[id].name, fname));

        add_instr("I2F_M %s\n", v_table[e.id].name);
        add_instr("SET %s\n"  , v_table[id     ].name);
    }

    // left float and right int in the acc ------------------------------------

    if ((v_table[id].type == 2) && (e.type == 1) && (e.id == 0))
    {
        fprintf(stdout, MSG_WARN_FLOAT_RECV_INT, line_num+1, rem_fname(v_table[id].name, fname));

        add_instr("I2F\n");
        add_instr("SET %s\n", v_table[id].name);
    }

    // left float and right float in memory -----------------------------------

    if ((v_table[id].type == 2) && (e.type == 2) && (e.id != 0))
    {
        add_instr("LOD %s\n", v_table[e.id].name);
        add_instr("SET %s\n", v_table[id     ].name);
    }

    // left float and right float in the acc ----------------------------------

    if ((v_table[id].type == 2) && (e.type == 2) && (e.id == 0))
    {
        add_instr("SET %s\n", v_table[id].name);
    }

    // left float and right comp const ----------------------------------------

    if ((v_table[id].type == 2) && (e.type == 5))
    {
        fprintf (stdout, MSG_WARN_GRAB_REAL_ONLY, line_num+1);

        get_cmp_cst(e,&etr,&eti);

        add_instr("LOD %s\n", v_table[etr.id].name);
        add_instr("SET %s\n", v_table[id      ].name);
    }

    // left float and right comp in memory ------------------------------------

    if ((v_table[id].type == 2) && (e.type == 3) && (e.id != 0))
    {
        fprintf (stdout, MSG_WARN_GRAB_REAL_ONLY, line_num+1);

        get_cmp_ets(e,&etr,&eti);

        add_instr("LOD %s\n", v_table[etr.id].name);
        add_instr("SET %s\n", v_table[id      ].name);
    }

    // left float and right comp in the acc -----------------------------------

    if ((v_table[id].type == 2) && (e.type == 3) && (e.id == 0))
    {
        fprintf (stdout, MSG_WARN_GRAB_REAL_ONLY, line_num+1);

        add_instr("POP\n");
        add_instr("SET %s\n", v_table[id].name);
    }

    // left comp and right int in memory --------------------------------------

    if ((v_table[id].type == 3) && (e.type == 1) && (e.id != 0))
    {
        fprintf(stdout, MSG_WARN_COMP_RECV_INT, line_num+1, rem_fname(v_table[id].name, fname));

        add_instr("I2F_M %s\n", v_table[e.id].name);
        add_instr("SET %s\n"  , v_table[id     ].name);

        add_instr("LOD 0.0\n");
        add_instr("SET %s_i\n", v_table[id     ].name);
    }

    // left comp and right int in the acc -------------------------------------

    if ((v_table[id].type == 3) && (e.type == 1) && (e.id == 0))
    {
        fprintf(stdout, MSG_WARN_COMP_RECV_INT, line_num+1, rem_fname(v_table[id].name, fname));

        add_instr("I2F\n");
        add_instr("SET %s\n"  , v_table[id].name);

        add_instr("LOD 0.0\n");
        add_instr("SET %s_i\n", v_table[id].name);
    }

    // left comp and right float var in memory --------------------------------

    if ((v_table[id].type == 3) && (e.type == 2) && (e.id != 0))
    {
        fprintf(stdout, MSG_WARN_COMP_RECV_FLOAT, line_num+1, rem_fname(v_table[id].name, fname));

        add_instr("LOD %s\n" , v_table[e.id].name);
        add_instr("SET %s\n" , v_table[id     ].name);

        add_instr("LOD 0.0\n");
        add_instr("SET %s_i\n",v_table[id].name);
    }

    // left comp and right float in the acc -----------------------------------

    if ((v_table[id].type == 3) && (e.type == 2) && (e.id == 0))
    {
        fprintf(stdout, MSG_WARN_COMP_RECV_FLOAT, line_num+1, rem_fname(v_table[id].name, fname));

        add_instr("SET %s\n"  , v_table[id].name);

        add_instr("LOD 0.0\n");
        add_instr("SET %s_i\n", v_table[id].name);
    }

    // left comp and right comp const -----------------------------------------

    if ((v_table[id].type == 3) && (e.type == 5))
    {
        get_cmp_cst(e,&etr,&eti);

        add_instr("LOD %s\n",   v_table[etr.id].name);
        add_instr("SET %s\n",   v_table[id      ].name);

        add_instr("LOD %s\n",   v_table[eti.id].name);
        add_instr("SET %s_i\n", v_table[id      ].name);
    }

    // left comp and right comp in memory -------------------------------------

    if ((v_table[id].type == 3) && (e.type == 3) && (e.id != 0))
    {
        get_cmp_ets(e,&etr,&eti);

        add_instr("LOD %s\n"  , v_table[etr.id].name);
        add_instr("SET %s\n"  , v_table[id      ].name);

        add_instr("LOD %s\n"  , v_table[eti.id].name);
        add_instr("SET %s_i\n", v_table[id      ].name);
    }

    // left comp and right comp in the acc ------------------------------------

    if ((v_table[id].type == 3) && (e.type == 3) && (e.id == 0))
    {
        add_instr("SET_P %s_i\n", v_table[id].name);
        add_instr("SET %s\n"    , v_table[id].name);
    }

    acc_ok     = 0;  // acc released
}

// array assignment with a COMPILE-TIME-CONSTANT index, e.g. x[3] = y;
// Stores into arr[k] directly with the SET_V pseudo-instruction (the assembler
// bakes the offset k into a plain SET to arr_base+k), so there is no index to
// materialise, no stack push and no indirect STI -- one fewer instruction than
// the ass_array path. The walker routes only int-array / int-rhs / 1D-forward
// assignments here; every other shape still goes through ass_array.
void ass_array_const(int id, int k, expr e)
{
    if (e.id != 0) add_instr("LOD %s\n", v_table[e.id].name);  // rhs -> acc
    add_instr("SET_V %s %d\n", v_table[id].name, k);
    acc_ok = 0;  // acc released
}

// array assignment, e.g. x[i] = y;
void ass_array(int id, expr e, int fft)
{
    // test whether it has been declared so an assignment is allowed
    if (v_table[id].type == 0)
    {
        fprintf (stderr, MSG_ERR_DECLARE_VAR_PLEASE, line_num+1, rem_fname(v_table[id].name, fname));
        exit(EXIT_FAILURE);
    }

    // array vs non-array check
    if (v_table[id].isar == 0)
    {
        fprintf (stderr, MSG_ERR_NOT_ARRAY, line_num+1, rem_fname(v_table[id].name, fname));
        exit(EXIT_FAILURE);
    }

    // ------------------------------------------------------------------------
    // perform the set --------------------------------------------------------
    // ------------------------------------------------------------------------

    expr  etr, eti;

    char set_type[16]; if (fft == 0) strcpy(set_type, "STI"); else strcpy(set_type, "ISI");

    // left int and right int in memory ---------------------------------------

    if ((v_table[id].type == 1) && (e.type == 1) && (e.id != 0))
    {
        add_instr("P_LOD %s\n",   v_table[e.id].name);
        add_instr("%s %s\n", set_type, v_table[id].name);
    }

    // left int and right int in the acc --------------------------------------

    if ((v_table[id].type == 1) && (e.type == 1) && (e.id == 0))
    {
        add_instr("%s %s\n", set_type, v_table[id].name);
    }

    // left int and right float in memory -------------------------------------

    if ((v_table[id].type == 1) && (e.type == 2) && (e.id != 0))
    {
        fprintf(stdout, MSG_WARN_INT_RECV_FLOAT, line_num+1, rem_fname(v_table[id].name, fname));

        add_instr("P_F2I_M %s\n", v_table[e.id].name);
        add_instr("%s %s\n", set_type, v_table[id].name);
    }

    // left int and right float in the acc ------------------------------------

    if ((v_table[id].type == 1) && (e.type == 2) && (e.id == 0))
    {
        fprintf(stdout, MSG_WARN_INT_RECV_FLOAT, line_num+1, rem_fname(v_table[id].name, fname));

        add_instr("F2I\n");
        add_instr("%s %s\n", set_type, v_table[id].name);
    }

    // left int and right comp const ------------------------------------------

    if ((v_table[id].type == 1) && (e.type == 5))
    {
        fprintf (stdout, MSG_WARN_ROUND_REAL, line_num+1);

        get_cmp_cst(e,&etr,&eti);

        add_instr("P_F2I_M %s\n", v_table[etr.id].name);
        add_instr("%s %s\n", set_type,  v_table[id].name);
    }

    // left int and right comp in memory --------------------------------------

    if ((v_table[id].type == 1) && (e.type == 3) && (e.id != 0))
    {
        fprintf (stdout, MSG_WARN_ROUND_REAL, line_num+1);

        get_cmp_ets(e,&etr,&eti);

        add_instr("P_F2I_M %s\n", v_table[etr.id].name);
        add_instr("%s %s\n", set_type,  v_table[id].name);
    }

    // left int and right comp in the acc -------------------------------------

    if ((v_table[id].type == 1) && (e.type == 3) && (e.id == 0))
    {
        fprintf (stdout, MSG_WARN_ROUND_REAL, line_num+1);

        add_instr("POP\n");
        add_instr("F2I\n");
        add_instr("%s %s\n", set_type, v_table[id].name);
    }

    // left float and right int in memory -------------------------------------

    if ((v_table[id].type == 2) && (e.type == 1) && (e.id != 0))
    {
        fprintf(stdout, MSG_WARN_FLOAT_RECV_INT, line_num+1, rem_fname(v_table[id].name, fname));

        add_instr("P_I2F_M %s\n", v_table[e.id].name);
        add_instr("%s %s\n", set_type, v_table[id].name);
    }

    // left float and right int in the acc ------------------------------------

    if ((v_table[id].type == 2) && (e.type == 1) && (e.id == 0))
    {
        fprintf(stdout, MSG_WARN_FLOAT_RECV_INT, line_num+1, rem_fname(v_table[id].name, fname));

        add_instr("I2F\n");
        add_instr("%s %s\n", set_type, v_table[id].name);
    }

    // left float and right float in memory -----------------------------------

    if ((v_table[id].type == 2) && (e.type == 2) && (e.id != 0))
    {
        add_instr("P_LOD %s\n",   v_table[e.id].name);
        add_instr("%s %s\n", set_type, v_table[id].name);
    }

    // left float and right float in the acc ----------------------------------

    if ((v_table[id].type == 2) && (e.type == 2) && (e.id == 0))
    {
        add_instr("%s %s\n", set_type, v_table[id].name);
    }

    // left float and right comp const ----------------------------------------

    if ((v_table[id].type == 2) && (e.type == 5))
    {
        fprintf (stdout, MSG_WARN_GRAB_REAL_ONLY, line_num+1);

        get_cmp_cst(e,&etr,&eti);

        add_instr("P_LOD %s\n",  v_table[etr.id].name);
        add_instr("%s %s\n", set_type, v_table[id].name);
    }

    // left float and right comp in memory ------------------------------------

    if ((v_table[id].type == 2) && (e.type == 3) && (e.id != 0))
    {
        fprintf (stdout, MSG_WARN_GRAB_REAL_ONLY, line_num+1);

        get_cmp_ets(e,&etr,&eti);

        add_instr("P_LOD %s\n",  v_table[etr.id].name);
        add_instr("%s %s\n", set_type, v_table[id].name);
    }

    // left float and right comp in the acc -----------------------------------

    if ((v_table[id].type == 2) && (e.type == 3) && (e.id == 0))
    {
        fprintf (stdout, MSG_WARN_GRAB_REAL_ONLY, line_num+1);

        add_instr("POP\n");
        add_instr("%s %s\n", set_type, v_table[id].name);
    }

    // left comp and right int in memory --------------------------------------

    if ((v_table[id].type == 3) && (e.type == 1) && (e.id != 0))
    {
        fprintf(stdout, MSG_WARN_COMP_RECV_INT, line_num+1, rem_fname(v_table[id].name, fname));

        add_instr("SET aux_var\n");
        add_instr("P_I2F_M %s\n", v_table[e.id].name);
        add_instr("%s %s\n"  , set_type, v_table[id].name);

        add_instr("LOD aux_var\n");
        add_instr("P_LOD 0.0\n"  );
        add_instr("%s %s_i\n", set_type, v_table[id].name);
    }

    // left comp and right int in the acc -------------------------------------

    if ((v_table[id].type == 3) && (e.type == 1) && (e.id == 0))
    {
        fprintf(stdout, MSG_WARN_COMP_RECV_INT, line_num+1, rem_fname(v_table[id].name, fname));

        add_instr("I2F\n"                   );
        add_instr("SET_P aux_var\n"         );
        add_instr("SET   aux_var2\n"        );
        add_instr("P_LOD aux_var\n"         );
        add_instr("%s %s\n"  , set_type, v_table[id].name);

        add_instr("LOD   aux_var2\n"        );
        add_instr("P_LOD 0.0\n"             );
        add_instr("%s %s_i\n", set_type, v_table[id].name);
    }

    // left comp and right float in memory ------------------------------------

    if ((v_table[id].type == 3) && (e.type == 2) && (e.id != 0))
    {
        fprintf(stdout, MSG_WARN_COMP_RECV_FLOAT, line_num+1, rem_fname(v_table[id].name, fname));

        add_instr("SET   aux_var\n"         );
        add_instr("P_LOD %s\n"  , v_table[e.id].name);
        add_instr("%s %s\n"  , set_type, v_table[id].name);

        add_instr("LOD   aux_var\n"         );
        add_instr("P_LOD 0.0\n"             );
        add_instr("%s %s_i\n", set_type, v_table[id].name);
    }

    // left comp and right float in the acc -----------------------------------

    if ((v_table[id].type == 3) && (e.type == 2) && (e.id == 0))
    {
        fprintf(stdout, MSG_WARN_COMP_RECV_FLOAT, line_num+1, rem_fname(v_table[id].name, fname));

        add_instr("SET_P aux_var\n"         );
        add_instr("SET   aux_var2\n"        );
        add_instr("P_LOD aux_var\n"         );
        add_instr("%s %s\n"  , set_type, v_table[id].name);

        add_instr("LOD   aux_var2\n"        );
        add_instr("P_LOD 0.0\n"             );
        add_instr("%s %s_i\n", set_type, v_table[id].name);
    }

    // left comp and right comp const -----------------------------------------

    if ((v_table[id].type == 3) && (e.type == 5))
    {
        get_cmp_cst(e,&etr,&eti);

        add_instr("SET   aux_var\n");
        add_instr("P_LOD %s\n"  , v_table[etr.id].name);
        add_instr("%s %s\n" , set_type, v_table[id].name);

        add_instr("LOD   aux_var\n");
        add_instr("P_LOD %s\n"   , v_table[eti.id].name);
        add_instr("%s %s_i\n", set_type, v_table[id].name);
    }

    // left comp and right comp in memory -------------------------------------

    if ((v_table[id].type == 3) && (e.type == 3) && (e.id != 0))
    {
        get_cmp_ets(e,&etr,&eti);

        add_instr("SET   aux_var\n");
        add_instr("P_LOD %s\n"   , v_table[etr.id].name);
        add_instr("%s %s\n"  , set_type, v_table[id].name);

        add_instr("LOD   aux_var\n");
        add_instr("P_LOD %s\n"   , v_table[eti.id].name);
        add_instr("%s %s_i\n", set_type, v_table[id].name);
    }

    // left comp and right comp in the acc ------------------------------------

    if ((v_table[id].type == 3) && (e.type == 3) && (e.id == 0))
    {
        add_instr("SET_P aux_var\n" );
        add_instr("SET_P aux_var2\n");
        add_instr("SET   aux_var3\n");

        add_instr("P_LOD aux_var2\n");
        add_instr("%s %s\n"  , set_type, v_table[id].name);

        add_instr("LOD   aux_var3\n");
        add_instr("P_LOD aux_var\n" );
        add_instr("%s %s_i\n", set_type, v_table[id].name);
    }

    acc_ok     = 0;  // acc released
}

// ++ operator assignment
void ass_pplus(int id)
{
    pplus2exp(id);
    acc_ok = 0; // acc released
}

// ++ operator assignment on a 1D array
void ass_aplus(int id, expr e)
{
    pplus1d2exp(id, e);
    acc_ok = 0; // acc released
}

// ++ operator assignment on a 2D array
void ass_apl2d(int id, expr e1, expr e2)
{
    pplus2d2exp(id, e1, e2);
    acc_ok = 0; // acc released
}
