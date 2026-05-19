// ----------------------------------------------------------------------------
// SAPHO standard library -----------------------------------------------------
// ----------------------------------------------------------------------------

// global includes
#include <string.h>
#include <stdlib.h>

// local includes
#include "..\Headers\t2t.h"
#include "..\Headers\oper.h"
#include "..\Headers\stdlib.h"
#include "..\Headers\global.h"
#include "..\Headers\macros.h"
#include "..\Headers\funcoes.h"
#include "..\Headers\data_use.h"
#include "..\Headers\variaveis.h"
#include "..\Headers\diretivas.h"
#include "..\Headers\messages.h"

// ----------------------------------------------------------------------------
// input and output ----------------------------------------------------------
// ----------------------------------------------------------------------------

// input ex: int x = in(0);
expr exec_in(int id)
{
    if (atoi(v_name[id]) >= nuioin) {fprintf(stderr, MSG_ERR_NO_IN_PORT, line_num+1, v_name[id]); exit(EXIT_FAILURE);}

    if (acc_ok == 0) add_instr("INN %s\n", v_name[id]); else add_instr("P_INN %s\n", v_name[id]);

    acc_ok = 1;  // marks the acc as now holding a value

    return expr_make(1, 0);
}

// input ex: float x = fin(0);
expr exec_fin(int id)
{
    if (atoi(v_name[id]) >= nuioin) {fprintf(stderr, MSG_ERR_NO_IN_PORT, line_num+1, v_name[id]); exit(EXIT_FAILURE);}

    if (acc_ok == 0) add_instr("F_INN %s\n", v_name[id]); else add_instr("PF_INN %s\n", v_name[id]);

    acc_ok = 1;  // marks the acc as now holding a value

    return expr_make(2, 0);
}

// output ex: out(0,x);
void exec_out(int id, expr e)
{
    // ------------------------------------------------------------------------
    // consistency check ------------------------------------------------------
    // ------------------------------------------------------------------------

    // check whether et was declared
    if (e.id != 0 && v_table[e.id].type == 0) {fprintf(stderr, MSG_ERR_DECL_FIRST, line_num+1, rem_fname(v_name[e.id], fname)); exit(EXIT_FAILURE);}

    // check whether it is comp
    if (e.type > 2) {fprintf (stderr, MSG_ERR_PICK_COMP_INFO, line_num+1); exit(EXIT_FAILURE);}

    // check whether et is a variable
    if (e.id != 0 && v_table[e.id].isar > 0) {fprintf(stderr, MSG_ERR_WRONG_USE, line_num+1, rem_fname(v_name[e.id], fname)); exit(EXIT_FAILURE);}

    // check port range
    if (atoi(v_name[id]) >= nuioou) {fprintf(stderr, MSG_ERR_NO_OUT_PORT, line_num+1, v_name[id]); exit(EXIT_FAILURE);}

    // ------------------------------------------------------------------------
    // update variable status -------------------------------------------------
    // ------------------------------------------------------------------------

    if (e.id != 0) v_table[e.id].used = 1;

    // ------------------------------------------------------------------------
    // execute ----------------------------------------------------------------
    // ------------------------------------------------------------------------

    // int var
    if ((e.type == 1) && (e.id!=0))
    {
       if (acc_ok == 0) add_instr("LOD %s\n", v_name[e.id]); else add_instr("P_LOD %s\n", v_name[e.id]);
    }

    // int acc
    if ((e.type == 1) && (e.id==0))
    {
        // nothing to do
    }

    // float var
    if ((e.type == 2) && (e.id!=0))
    {
        fprintf(stdout, MSG_WARN_USE_FOUT, line_num+1);

        if (acc_ok == 0) add_instr("F2I_M %s\n", v_name[e.id]); else add_instr("P_F2I_M %s\n", v_name[e.id]);
    }

    // float acc
    if ((e.type == 2) && (e.id==0))
    {
        fprintf(stdout, MSG_WARN_USE_FOUT, line_num+1);

        add_instr("F2I\n");
    }

    add_instr("OUT %s\n", v_name[id]);

    acc_ok = 0; // libera acc    
}

// output ex: fout(0,x);
void exec_fout(int id, expr e)
{
    // ------------------------------------------------------------------------
    // consistency check ------------------------------------------------------
    // ------------------------------------------------------------------------

    // check whether et was declared
    if (e.id != 0 && v_table[e.id].type == 0) {fprintf(stderr, MSG_ERR_DECL_FIRST, line_num+1, rem_fname(v_name[e.id], fname)); exit(EXIT_FAILURE);}

    // check whether it is comp
    if (e.type > 2) {fprintf (stderr, MSG_ERR_PICK_COMP_INFO, line_num+1); exit(EXIT_FAILURE);}

    // check whether et is a variable
    if (e.id != 0 && v_table[e.id].isar > 0) {fprintf(stderr, MSG_ERR_WRONG_USE, line_num+1, rem_fname(v_name[e.id], fname)); exit(EXIT_FAILURE);}

    // check port range
    if (atoi(v_name[id]) >= nuioou) {fprintf(stderr, MSG_ERR_NO_OUT_PORT, line_num+1, v_name[id]); exit(EXIT_FAILURE);}

    // ------------------------------------------------------------------------
    // update variable status -------------------------------------------------
    // ------------------------------------------------------------------------

    if (e.id != 0) v_table[e.id].used = 1;

    // ------------------------------------------------------------------------
    // execute ----------------------------------------------------------------
    // ------------------------------------------------------------------------

    // int var
    if ((e.type == 1) && (e.id!=0))
    {
        fprintf(stdout, MSG_WARN_USE_OUT, line_num+1);

        if (acc_ok == 0) add_instr("LOD %s\n", v_name[e.id]); else add_instr("P_LOD %s\n", v_name[e.id]);
    }

    // int acc
    if ((e.type == 1) && (e.id==0))
    {
        fprintf(stdout, MSG_WARN_USE_OUT, line_num+1);
    }

    // float var
    if ((e.type == 2) && (e.id!=0))
    {
        if (acc_ok == 0) add_instr("F2I_M %s\n", v_name[e.id]); else add_instr("P_F2I_M %s\n", v_name[e.id]);
    }

    // float acc
    if ((e.type == 2) && (e.id==0))
    {
        add_instr("F2I\n");
    }

    add_instr("OUT %s\n", v_name[id]);

    acc_ok = 0; // libera acc    
}

// ----------------------------------------------------------------------------
// special functions that save code -------------------------------------------
// ----------------------------------------------------------------------------

// takes the sign of the first argument and applies it to the second
expr exec_sign(expr e1, expr e2)
{
    // ------------------------------------------------------------------------
    // consistency check ------------------------------------------------------
    // ------------------------------------------------------------------------

    // check whether et1 was declared
    if (e1.id != 0 && v_table[e1.id].type == 0) {fprintf(stderr, MSG_ERR_DECL_FIRST, line_num+1, rem_fname(v_name[e1.id], fname)); exit(EXIT_FAILURE);}
    
    // check whether et2 was declared
    if (e2.id != 0 && v_table[e2.id].type == 0) {fprintf(stderr, MSG_ERR_DECL_FIRST, line_num+1, rem_fname(v_name[e2.id], fname)); exit(EXIT_FAILURE);}

    // check whether et1 is a variable
    if (e1.id != 0 && v_table[e1.id].isar > 0) {fprintf(stderr, MSG_ERR_WRONG_USE, line_num+1, rem_fname(v_name[e1.id], fname)); exit(EXIT_FAILURE);}

    // check whether et2 is a variable
    if (e2.id != 0 && v_table[e2.id].isar > 0) {fprintf(stderr, MSG_ERR_WRONG_USE, line_num+1, rem_fname(v_name[e2.id], fname)); exit(EXIT_FAILURE);}

    // check whether there is a comp
    if ((e1.type > 2) || (e2.type > 2)) {fprintf (stderr, MSG_ERR_SIGN_COMPLEX, line_num+1); exit(EXIT_FAILURE);}

    // ------------------------------------------------------------------------
    // update variable status -------------------------------------------------
    // ------------------------------------------------------------------------

    if (e1.id != 0) v_table[e1.id].used = 1;
    if (e2.id != 0) v_table[e2.id].used = 1;

    // ------------------------------------------------------------------------
    // prepare local variables ------------------------------------------------
    // ------------------------------------------------------------------------

    char ld[10]; if (acc_ok == 0) strcpy(ld,"LOD"); else strcpy(ld,"P_LOD");

    // ------------------------------------------------------------------------
    // execute ----------------------------------------------------------------
    // ------------------------------------------------------------------------

    // int in memory and int in memory
    if ((e1.type == 1) && (e1.id != 0) && (e2.type == 1) && (e2.id != 0))
    {
        add_instr("%s %s\n", ld, v_name[e2.id]);
        add_instr("SGN %s\n"   , v_name[e1.id]);
    }

    // int in memory and int in acc
    if ((e1.type == 1) && (e1.id != 0) && (e2.type == 1) && (e2.id == 0))
    {
        add_instr("SGN %s\n", v_name[e1.id]);
    }

    // int in memory and float var in memory
    if ((e1.type == 1) && (e1.id != 0) && (e2.type == 2) && (e2.id != 0))
    {
        add_instr("%s %s\n", ld, v_name[e2.id]);
        add_instr("F_SGN %s\n" , v_name[e1.id]);
    }

    // int in memory and float in acc
    if ((e1.type == 1) && (e1.id != 0) && (e2.type == 2) && (e2.id == 0))
    {
        add_instr("F_SGN %s\n", v_name[e1.id]);
    }

    // int in acc and int in memory
    if ((e1.type == 1) && (e1.id == 0) && (e2.type == 1) && (e2.id != 0))
    {
        add_instr("%s %s\n", ld, v_name[e2.id]);
        add_instr("S_SGN\n");
    }

    // int in acc and int in acc
    if ((e1.type == 1) && (e1.id == 0) && (e2.type == 1) && (e2.id == 0))
    {
        add_instr("S_SGN\n");
    }

    // int in acc and float var in memory
    if ((e1.type == 1) && (e1.id == 0) && (e2.type == 2) && (e2.id != 0))
    {
        add_instr("P_LOD %s\n", v_name[e2.id]  );
        add_instr("SF_SGN\n");
    }

    // int in acc and float in acc
    if ((e1.type == 1) && (e1.id == 0) && (e2.type == 2) && (e2.id == 0))
    {
        add_instr("SF_SGN\n");
    }

    // float var and int var
    if ((e1.type == 2) && (e1.id != 0) && (e2.type == 1) && (e2.id != 0))
    {
        add_instr("%s %s\n", ld, v_name[e2.id]);
        add_instr("SGN %s\n"   , v_name[e1.id]);
    }

    // float var and int in acc
    if ((e1.type == 2) && (e1.id != 0) && (e2.type == 1) && (e2.id == 0))
    {
        add_instr("SGN %s\n"  , v_name[e1.id]);
    }

    // float var and float var
    if ((e1.type == 2) && (e1.id != 0) && (e2.type == 2) && (e2.id != 0))
    {
        add_instr("%s %s\n", ld, v_name[e2.id]);
        add_instr("F_SGN %s\n" , v_name[e1.id]);
    }

    // float var and float in acc
    if ((e1.type == 2) && (e1.id != 0) && (e2.type == 2) && (e2.id == 0))
    {
        add_instr("F_SGN %s\n" , v_name[e1.id]);
    }

    // float in acc and int in memory
    if ((e1.type == 2) && (e1.id == 0) && (e2.type == 1) && (e2.id != 0))
    {
        add_instr("%s %s\n", ld, v_name[e2.id]);
        add_instr("S_SGN\n");
    }

    // float in acc and int in acc
    if ((e1.type == 2) && (e1.id == 0) && (e2.type == 1) && (e2.id == 0))
    {
        add_instr("S_SGN\n");
    }

    // float in acc and float var in memory
    if ((e1.type == 2) && (e1.id == 0) && (e2.type == 2) && (e2.id != 0))
    {
        add_instr("P_LOD %s\n", v_name[e2.id]);
        add_instr("SF_SGN\n");
    }

    // float in acc and float in acc
    if ((e1.type == 2) && (e1.id == 0) && (e2.type == 2) && (e2.id == 0))
    {
        add_instr("SF_SGN\n");
    }

    acc_ok = 1;

    return expr_make(e2.type, 0);
}

// absolute value (int, float and comp)
expr exec_abs(expr e)
{
    // ------------------------------------------------------------------------
    // consistency check ------------------------------------------------------
    // ------------------------------------------------------------------------

    // check whether et was declared
    if (e.id != 0 && v_table[e.id].type == 0) {fprintf(stderr, MSG_ERR_DECL_FIRST, line_num+1, rem_fname(v_name[e.id], fname)); exit(EXIT_FAILURE);}

    // check whether et is a variable
    if (e.id != 0 && v_table[e.id].isar > 0) {fprintf(stderr, MSG_ERR_WRONG_USE, line_num+1, rem_fname(v_name[e.id], fname)); exit(EXIT_FAILURE);}

    // ------------------------------------------------------------------------
    // update variable status -------------------------------------------------
    // ------------------------------------------------------------------------

    if (e.id != 0) v_table[e.id].used = 1;

    // ------------------------------------------------------------------------
    // prepare local variables ------------------------------------------------
    // ------------------------------------------------------------------------
    
    char  ld [10]; if (acc_ok == 0) strcpy(ld  , "LOD"   ); else strcpy(ld  , "P_LOD"  );
    char  abs[10]; if (acc_ok == 0) strcpy( abs, "ABS_M" ); else strcpy( abs, "P_ABS_M");
    char fabs[10]; if (acc_ok == 0) strcpy(fabs,"F_ABS_M"); else strcpy(fabs,"PF_ABS_M");

    // ------------------------------------------------------------------------
    // execute ----------------------------------------------------------------
    // ------------------------------------------------------------------------

    // int in memory
    if ((e.type == 1) && (e.id != 0))
    {
        add_instr("%s %s\n", abs, v_name[e.id]);
    }

    // int in acc
    if ((e.type == 1) && (e.id == 0))
    {
        add_instr("ABS\n");
    }

    // float in memory
    if ((e.type == 2) && (e.id != 0))
    {
        add_instr("%s %s\n", fabs, v_name[e.id]);
    }

    // float in acc
    if ((e.type == 2) && (e.id == 0))
    {
        add_instr("F_ABS\n");
    }

    // comp const, in memory and in acc
    if ((e.type == 3) || (e.type == 5))
    {
        e = exec_sqrt(exec_mod2(e));
    }

    acc_ok = 1;

    return expr_make(e.type, 0);
}

// clears if negative
expr exec_pst(expr e)
{
    // ------------------------------------------------------------------------
    // consistency check ------------------------------------------------------
    // ------------------------------------------------------------------------

    // check whether et was declared
    if (e.id != 0 && v_table[e.id].type == 0) {fprintf(stderr, MSG_ERR_DECL_FIRST, line_num+1, rem_fname(v_name[e.id], fname)); exit(EXIT_FAILURE);}

    // check whether et is a variable
    if (e.id != 0 && v_table[e.id].isar > 0) {fprintf(stderr, MSG_ERR_WRONG_USE, line_num+1, rem_fname(v_name[e.id], fname)); exit(EXIT_FAILURE);}

    // ------------------------------------------------------------------------
    // update variable status -------------------------------------------------
    // ------------------------------------------------------------------------

    if (e.id != 0) v_table[e.id].used = 1;

    // ------------------------------------------------------------------------
    // prepare local variables ------------------------------------------------
    // ------------------------------------------------------------------------

    char  ld [10]; if (acc_ok == 0) strcpy( ld , "LOD"   ); else strcpy( ld , "P_LOD"  );
    char  pst[10]; if (acc_ok == 0) strcpy( pst, "PST_M" ); else strcpy( pst, "P_PST_M");
    char fpst[10]; if (acc_ok == 0) strcpy(fpst,"F_PST_M"); else strcpy(fpst,"PF_PST_M");

    // ------------------------------------------------------------------------
    // execute ----------------------------------------------------------------
    // ------------------------------------------------------------------------

    // int in memory
    if ((e.type == 1) && (e.id != 0))
    {
        add_instr("%s %s\n", pst, v_name[e.id]);
    }

    // int in acc
    if ((e.type == 1) && (e.id == 0))
    {
        add_instr("PST\n");
    }

    // float in memory
    if ((e.type == 2)  && (e.id != 0))
    {
        add_instr("%s %s\n", fpst, v_name[e.id]);
    }

    // float in acc
    if ((e.type == 2)  && (e.id == 0))
    {
        add_instr("F_PST\n");
    }

    // comp
    if (e.type > 2)
    {
        fprintf (stderr, MSG_ERR_PSET_COMPLEX, line_num+1);
        exit(EXIT_FAILURE);
    }

    acc_ok = 1;

    return expr_make(e.type, 0);
}

// division by constant
expr exec_norm(expr e)
{
    // ------------------------------------------------------------------------
    // consistency check ------------------------------------------------------
    // ------------------------------------------------------------------------

    // check whether et was declared
    if (e.id != 0 && v_table[e.id].type == 0) {fprintf(stderr, MSG_ERR_DECL_FIRST, line_num+1, rem_fname(v_name[e.id], fname)); exit(EXIT_FAILURE);}

    // check whether et is a variable
    if (e.id != 0 && v_table[e.id].isar > 0) {fprintf(stderr, MSG_ERR_WRONG_USE, line_num+1, rem_fname(v_name[e.id], fname)); exit(EXIT_FAILURE);}

    // check whether it is int
    if (e.type != 1) {fprintf (stderr, MSG_ERR_NORM_NON_INT, line_num+1); exit(EXIT_FAILURE);}

    // ------------------------------------------------------------------------
    // update variable status -------------------------------------------------
    // ------------------------------------------------------------------------

    if (e.id != 0) v_table[e.id].used = 1;

    // ------------------------------------------------------------------------
    // prepare local variables ------------------------------------------------
    // ------------------------------------------------------------------------

    char nrm[10]; if (acc_ok == 0) strcpy(nrm,"NRM_M"); else strcpy(nrm,"P_NRM_M");

    // ------------------------------------------------------------------------
    // execute ----------------------------------------------------------------
    // ------------------------------------------------------------------------

    // int in memory
    if ((e.type == 1) && (e.id != 0))
    {
        add_instr("%s %s\n", nrm, v_name[e.id]);
    }

    // int in acc
    if ((e.type == 1) && (e.id == 0))
    {
        add_instr("NRM\n");
    }

    acc_ok = 1;

    return expr_make(1, 0);
}

void exec_copy(expr e, int id2)
{
    // ------------------------------------------------------------------------
    // consistency check ------------------------------------------------------
    // ------------------------------------------------------------------------

    // check whether the source expression was declared
    if (e.id != 0 && v_table[e.id].type == 0) {fprintf(stderr, MSG_ERR_DECL_FIRST, line_num+1, rem_fname(v_name[e.id], fname)); exit(EXIT_FAILURE);}

    // check whether the source is a plain variable (not an array)
    if (e.id != 0 && v_table[e.id].isar > 0) {fprintf(stderr, MSG_ERR_WRONG_USE, line_num+1, rem_fname(v_name[e.id], fname)); exit(EXIT_FAILURE);}

    // check whether id2 was declared
    if (v_table[id2].type == 0) {fprintf(stderr, MSG_ERR_DECL_FIRST, line_num+1, rem_fname(v_name[id2], fname)); exit(EXIT_FAILURE);}

    // check whether id2 is a variable
    if (v_table[id2].isar > 0) {fprintf(stderr, MSG_ERR_WRONG_USE, line_num+1, rem_fname(v_name[id2], fname)); exit(EXIT_FAILURE);}

    // ------------------------------------------------------------------------
    // update variable status -------------------------------------------------
    // ------------------------------------------------------------------------

    if (e.id != 0) v_table[e.id].used = 1;

    // ------------------------------------------------------------------------
    // execute ----------------------------------------------------------------
    // ------------------------------------------------------------------------

    // from var to var
    if (e.id != 0)
    {
        add_instr("LOD %s\n", v_name[e.id]);
        add_instr("SET %s\n", v_name[id2] );
    }

    // from acc to var
    if (e.id == 0)
    {
        add_instr("SET %s\n", v_name[id2]);
    }
}

// ----------------------------------------------------------------------------
// non-linear functions -------------------------------------------------------
// ----------------------------------------------------------------------------

// square root
expr exec_sqrt(expr e)
{
    // ------------------------------------------------------------------------
    // consistency check ------------------------------------------------------
    // ------------------------------------------------------------------------

    // check whether et was declared
    if (e.id != 0 && v_table[e.id].type == 0) {fprintf(stderr, MSG_ERR_DECL_FIRST, line_num+1, rem_fname(v_name[e.id], fname)); exit(EXIT_FAILURE);}

    // check whether et is a variable
    if (e.id != 0 && v_table[e.id].isar > 0) {fprintf(stderr, MSG_ERR_WRONG_USE, line_num+1, rem_fname(v_name[e.id], fname)); exit(EXIT_FAILURE);}

    // check whether it is comp
    if (e.type > 2) {fprintf (stderr, MSG_ERR_SQRT_COMPLEX, line_num+1); exit(EXIT_FAILURE);}

    // ------------------------------------------------------------------------
    // update variable status -------------------------------------------------
    // ------------------------------------------------------------------------

    if (e.id != 0) v_table[e.id].used = 1;

    // ------------------------------------------------------------------------
    // prepare local variables ------------------------------------------------
    // ------------------------------------------------------------------------

    char ld [10]; if (acc_ok == 0) strcpy(ld ,"LOD"   ); else strcpy(ld ,"P_LOD"  );
    char i2f[10]; if (acc_ok == 0) strcpy(i2f, "I2F_M"); else strcpy(i2f,"P_I2F_M");

    // ------------------------------------------------------------------------
    // execute ----------------------------------------------------------------
    // ------------------------------------------------------------------------

    // int in memory
    if ((e.type == 1) && (e.id != 0))
    {
        add_instr("%s %s\n", i2f, v_name[e.id]);
        add_instr("CAL float_sqrt\n");
    }

    // int in acc
    if ((e.type == 1) && (e.id == 0))
    {
        add_instr("I2F\n");
        add_instr("CAL float_sqrt\n");
    }

    // float in memory
    if ((e.type == 2) && (e.id != 0))
    {
        add_instr("%s %s\n", ld, v_name[e.id]);
        add_instr("CAL float_sqrt\n");
    }

    // float in acc
    if ((e.type == 2) && (e.id == 0))
    {
        add_instr("CAL float_sqrt\n");
    }

    acc_ok = 1;

    return expr_make(2, 0);
}

// arctangent
expr exec_atan(expr e)
{
    // ------------------------------------------------------------------------
    // consistency check ------------------------------------------------------
    // ------------------------------------------------------------------------

    // check whether et was declared
    if (e.id != 0 && v_table[e.id].type == 0) {fprintf(stderr, MSG_ERR_DECL_FIRST, line_num+1, rem_fname(v_name[e.id], fname)); exit(EXIT_FAILURE);}

    // check whether et is a variable
    if (e.id != 0 && v_table[e.id].isar > 0) {fprintf(stderr, MSG_ERR_WRONG_USE, line_num+1, rem_fname(v_name[e.id], fname)); exit(EXIT_FAILURE);}

    // check whether it is comp
    if (e.type > 2) {fprintf (stderr, MSG_ERR_SQRT_COMPLEX, line_num+1); exit(EXIT_FAILURE);}

    // ------------------------------------------------------------------------
    // update variable status -------------------------------------------------
    // ------------------------------------------------------------------------

    if (e.id != 0) v_table[e.id].used = 1;

    // ------------------------------------------------------------------------
    // prepare local variables ------------------------------------------------
    // ------------------------------------------------------------------------

    char ld [10]; if (acc_ok == 0) strcpy(ld ,"LOD"  ); else strcpy(ld ,"P_LOD"  );
    char i2f[10]; if (acc_ok == 0) strcpy(i2f,"I2F_M"); else strcpy(i2f,"P_I2F_M");

    // ------------------------------------------------------------------------
    // execute ----------------------------------------------------------------
    // ------------------------------------------------------------------------

    // int in memory
    if ((e.type == 1) && (e.id != 0))
    {
        add_instr("%s %s\n", i2f, v_name[e.id]);
        add_instr("CAL float_atan\n");
    }

    // int in acc
    if ((e.type == 1) && (e.id == 0))
    {
        add_instr("I2F\n");
        add_instr("CAL float_atan\n");
    }

    // float in memory
    if ((e.type == 2) && (e.id != 0))
    {
        add_instr("%s %s\n", ld, v_name[e.id]);
        add_instr("CAL float_atan\n");
    }

    // float in acc
    if ((e.type == 2) && (e.id == 0))
    {
        add_instr("CAL float_atan\n");
    }

    acc_ok = 1;

    return expr_make(2, 0);
}

// sine
expr exec_sin(expr e)
{
    // ------------------------------------------------------------------------
    // consistency check ------------------------------------------------------
    // ------------------------------------------------------------------------

    // check whether et was declared
    if (e.id != 0 && v_table[e.id].type == 0) {fprintf(stderr, MSG_ERR_DECL_FIRST, line_num+1, rem_fname(v_name[e.id], fname)); exit(EXIT_FAILURE);}

    // check whether et is a variable
    if (e.id != 0 && v_table[e.id].isar > 0) {fprintf(stderr, MSG_ERR_WRONG_USE, line_num+1, rem_fname(v_name[e.id], fname)); exit(EXIT_FAILURE);}

    // check whether it is comp
    if (e.type > 2) {fprintf (stderr, MSG_ERR_SQRT_COMPLEX, line_num+1); exit(EXIT_FAILURE);}

    // ------------------------------------------------------------------------
    // update variable status -------------------------------------------------
    // ------------------------------------------------------------------------

    if (e.id != 0) v_table[e.id].used = 1;

    // ------------------------------------------------------------------------
    // prepare local variables ------------------------------------------------
    // ------------------------------------------------------------------------

    char ld [10]; if (acc_ok == 0) strcpy(ld ,"LOD"  ); else strcpy(ld ,"P_LOD"  );
    char i2f[10]; if (acc_ok == 0) strcpy(i2f,"I2F_M"); else strcpy(i2f,"P_I2F_M");

    // ------------------------------------------------------------------------
    // execute ----------------------------------------------------------------
    // ------------------------------------------------------------------------

    // int in memory
    if ((e.type == 1) && (e.id != 0))
    {
        add_instr("%s %s\n", i2f, v_name[e.id]);
        add_instr("CAL float_sin\n");
    }

    // int in acc
    if ((e.type == 1) && (e.id == 0))
    {
        add_instr("I2F\n");
        add_instr("CAL float_sin\n");
    }

    // float in memory
    if ((e.type == 2) && (e.id != 0))
    {
        add_instr("%s %s\n", ld, v_name[e.id]);
        add_instr("CAL float_sin\n");
    }

    // float in acc
    if ((e.type == 2) && (e.id == 0))
    {
        add_instr("CAL float_sin\n");
    }

    acc_ok = 1;

    return expr_make(2, 0);
}

// cosine
expr exec_cos(expr e)
{
    // ------------------------------------------------------------------------
    // consistency check ------------------------------------------------------
    // ------------------------------------------------------------------------

    // check whether et was declared
    if (e.id != 0 && v_table[e.id].type == 0) {fprintf(stderr, MSG_ERR_DECL_FIRST, line_num+1, rem_fname(v_name[e.id], fname)); exit(EXIT_FAILURE);}

    // check whether et is a variable
    if (e.id != 0 && v_table[e.id].isar > 0) {fprintf(stderr, MSG_ERR_WRONG_USE, line_num+1, rem_fname(v_name[e.id], fname)); exit(EXIT_FAILURE);}

    // check whether it is comp
    if (e.type > 2) {fprintf (stderr, MSG_ERR_SQRT_COMPLEX, line_num+1); exit(EXIT_FAILURE);}

    // ------------------------------------------------------------------------
    // update variable status -------------------------------------------------
    // ------------------------------------------------------------------------

    if (e.id != 0) v_table[e.id].used = 1;

    // ------------------------------------------------------------------------
    // prepare local variables ------------------------------------------------
    // ------------------------------------------------------------------------

    char ld [10]; if (acc_ok == 0) strcpy(ld ,"LOD"  ); else strcpy(ld ,"P_LOD"  );
    char i2f[10]; if (acc_ok == 0) strcpy(i2f,"I2F_M"); else strcpy(i2f,"P_I2F_M");

    // ------------------------------------------------------------------------
    // execute ----------------------------------------------------------------
    // ------------------------------------------------------------------------

    // int in memory
    if ((e.type == 1) && (e.id != 0))
    {
        add_instr("%s %s\n", i2f, v_name[e.id]);
        add_instr("F_NEG\n");
        add_instr("F_ADD 1.570796327");
        add_instr("CAL float_sin\n");
    }

    // int in acc
    if ((e.type == 1) && (e.id == 0))
    {
        add_instr("I2F\n");
        add_instr("F_NEG\n");
        add_instr("F_ADD 1.570796327");
        add_instr("CAL float_sin\n");
    }

    // float in memory
    if ((e.type == 2) && (e.id != 0))
    {
        add_instr("%s %s\n", ld, v_name[e.id]);
        add_instr("F_NEG\n");
        add_instr("F_ADD 1.570796327");
        add_instr("CAL float_sin\n");
    }

    // float in acc
    if ((e.type == 2) && (e.id == 0))
    {
        add_instr("F_NEG\n");
        add_instr("F_ADD 1.570796327");
        add_instr("CAL float_sin\n");
    }

    acc_ok = 1;

    return expr_make(2, 0);
}

// ----------------------------------------------------------------------------
// special functions for complex numbers --------------------------------------
// ----------------------------------------------------------------------------

// returns the real part of a comp
expr exec_real(expr e)
{
    // ------------------------------------------------------------------------
    // consistency check ------------------------------------------------------
    // ------------------------------------------------------------------------

    // check whether et was declared
    if (e.id != 0 && v_table[e.id].type == 0) {fprintf(stderr, MSG_ERR_DECL_FIRST, line_num+1, rem_fname(v_name[e.id], fname)); exit(EXIT_FAILURE);}

    // check whether et is a variable
    if (e.id != 0 && v_table[e.id].isar > 0) {fprintf(stderr, MSG_ERR_WRONG_USE, line_num+1, rem_fname(v_name[e.id], fname)); exit(EXIT_FAILURE);}

    // check whether it is comp
    if (e.type < 3) {fprintf (stderr, MSG_ERR_REAL_ARG_COMP, line_num+1); exit(EXIT_FAILURE);}

    // ------------------------------------------------------------------------
    // update variable status -------------------------------------------------
    // ------------------------------------------------------------------------

    if (e.id != 0) v_table[e.id].used = 1;

    // ------------------------------------------------------------------------
    // prepare local variables ------------------------------------------------
    // ------------------------------------------------------------------------

    char ld[10]; if (acc_ok == 0) strcpy(ld,"LOD"); else strcpy(ld,"P_LOD");

    // ------------------------------------------------------------------------
    // execute ----------------------------------------------------------------
    // ------------------------------------------------------------------------

    // comp const
    if (e.type == 5)
    {
        expr et_r, et_i;
        get_cmp_cst(e,&et_r,&et_i);

        add_instr("%s %s\n", ld, v_name[et_r.id]);
    }

    // comp in memory
    if ((e.type == 3) && (e.id != 0))
    {
        add_instr("%s %s\n", ld, v_name[e.id]);
    }

    // comp in acc
    if ((e.type == 3) && (e.id == 0))
    {
        add_instr("POP\n");
    }

    acc_ok = 1;
    
    return expr_make(2, 0);
}

// returns the imag part of a comp
expr exec_imag(expr e)
{
    // ------------------------------------------------------------------------
    // consistency check ------------------------------------------------------
    // ------------------------------------------------------------------------

    // check whether et was declared
    if (e.id != 0 && v_table[e.id].type == 0) {fprintf(stderr, MSG_ERR_DECL_FIRST, line_num+1, rem_fname(v_name[e.id], fname)); exit(EXIT_FAILURE);}

    // check whether et is a variable
    if (e.id != 0 && v_table[e.id].isar > 0) {fprintf(stderr, MSG_ERR_WRONG_USE, line_num+1, rem_fname(v_name[e.id], fname)); exit(EXIT_FAILURE);}

    // check whether it is comp
    if (e.type < 3) {fprintf (stderr, MSG_ERR_IMAG_ARG_COMP, line_num+1); exit(EXIT_FAILURE);}

    // ------------------------------------------------------------------------
    // update variable status -------------------------------------------------
    // ------------------------------------------------------------------------

    if (e.id != 0) v_table[e.id].used = 1;

    // ------------------------------------------------------------------------
    // prepare local variables ------------------------------------------------
    // ------------------------------------------------------------------------

    char ld[10]; if (acc_ok == 0) strcpy(ld,"LOD"); else strcpy(ld,"P_LOD");

    // ------------------------------------------------------------------------
    // execute ----------------------------------------------------------------
    // ------------------------------------------------------------------------

    // comp const
    if (e.type == 5)
    {
        expr et_r, et_i;
        get_cmp_cst(e,&et_r,&et_i);

        add_instr("%s %s\n", ld, v_name[et_i.id]);
    }

    // comp in memory
    if ((e.type == 3) && (e.id != 0))
    {
        expr et_r, et_i;
        get_cmp_ets(e,&et_r,&et_i);

        add_instr("%s %s\n", ld, v_name[et_i.id]);
    }

    // comp in acc
    if ((e.type == 3) && (e.id == 0))
    {
        add_instr("SET_P aux_var\n");
        add_instr("LOD   aux_var\n");
    }

    acc_ok = 1;

    return expr_make(2, 0);
}

// squared magnitude of a complex number
expr exec_mod2(expr e)
{
    // ------------------------------------------------------------------------
    // consistency check ------------------------------------------------------
    // ------------------------------------------------------------------------

    // check whether et was declared
    if (e.id != 0 && v_table[e.id].type == 0) {fprintf(stderr, MSG_ERR_DECL_FIRST, line_num+1, rem_fname(v_name[e.id], fname)); exit(EXIT_FAILURE);}

    // check whether et is a variable
    if (e.id != 0 && v_table[e.id].isar > 0) {fprintf(stderr, MSG_ERR_WRONG_USE, line_num+1, rem_fname(v_name[e.id], fname)); exit(EXIT_FAILURE);}

    // check whether it is comp
    if (e.type < 3) {fprintf (stderr, MSG_ERR_MOD2_ARG_COMP, line_num+1); exit(EXIT_FAILURE);}

    // ------------------------------------------------------------------------
    // update variable status -------------------------------------------------
    // ------------------------------------------------------------------------

    if (e.id != 0) v_table[e.id].used = 1;

    // ------------------------------------------------------------------------
    // prepare local variables ------------------------------------------------
    // ------------------------------------------------------------------------

    expr etr, eti;

    // ------------------------------------------------------------------------
    // execute ----------------------------------------------------------------
    // ------------------------------------------------------------------------

    // when it is a constant -------------------------------------------------
    if (e.type == 5)
    {
        get_cmp_cst(e,&etr,&eti);     // get the et of each float constant
        etr = oper_mult(etr, etr);     // real part squared
        eti = oper_mult(eti, eti);     // imag part squared
        etr = oper_soma(etr, eti);     // sum of the squares
    }

    // when it is in memory --------------------------------------------------
    if ((e.type == 3) && (e.id != 0))
    {
        get_cmp_ets(e,&etr,&eti);     // get the et of each variable
        etr = oper_mult(etr, etr);     // real part squared
        eti = oper_mult(eti, eti);     // imag part squared
        etr = oper_soma(etr, eti);     // sum of the squares
    }

    // when it is in the accumulator ------------------------------------------
    if ((e.type == 3) && (e.id == 0))
    {
        add_instr("PSH\n");             // imag part stays in acc and on the stack
        oper_mult(expr_make(2, 0), expr_make(2, 0));      // multiplica acc com pilha
        add_instr("SET_P aux_var\n");   // save temp and fetch real part

        add_instr("PSH\n");             // real part stays in acc and on the stack
        oper_mult(expr_make(2, 0), expr_make(2, 0));      // multiplica acc com pilha
        add_instr("P_LOD aux_var\n");   // push the real-squared onto the stack and fetch the imag-squared

        oper_soma(expr_make(2, 0), expr_make(2, 0));       // sum of the squares

        etr = expr_make(2, 0);          // output is a float in the accumulator
    }

    return expr_make(2, 0);
}

// computes the phase (in radians) of a complex number
expr exec_fase(expr e)
{
    // ------------------------------------------------------------------------
    // consistency check ------------------------------------------------------
    // ------------------------------------------------------------------------

    // check whether et was declared
    if (e.id != 0 && v_table[e.id].type == 0) {fprintf(stderr, MSG_ERR_DECL_FIRST, line_num+1, rem_fname(v_name[e.id], fname)); exit(EXIT_FAILURE);}

    // check whether et is a variable
    if (e.id != 0 && v_table[e.id].isar > 0) {fprintf(stderr, MSG_ERR_WRONG_USE, line_num+1, rem_fname(v_name[e.id], fname)); exit(EXIT_FAILURE);}

    // check whether it is comp
    if (e.type < 3) {fprintf (stderr, MSG_ERR_FASE_ARG_COMP, line_num+1); exit(EXIT_FAILURE);}

    // ------------------------------------------------------------------------
    // update variable status -------------------------------------------------
    // ------------------------------------------------------------------------

    if (e.id != 0) v_table[e.id].used = 1;

    // ------------------------------------------------------------------------
    // prepare local variables ------------------------------------------------
    // ------------------------------------------------------------------------
    
    expr et_r, et_i;

    // ------------------------------------------------------------------------
    // execute ----------------------------------------------------------------
    // ------------------------------------------------------------------------

    // comp const
    if (e.type == 5)
    {
        get_cmp_cst(e,&et_i,&et_r);
        oper_divi  (et_r, et_i);
    }

    // comp in memory
    if ((e.type == 3) && (e.id != 0))
    {
        get_cmp_ets(e,&et_i,&et_r);
        oper_divi  (et_r, et_i);
    }

    // comp in acc
    if ((e.type == 3) && (e.id == 0))
    {
        int id = exec_id("aux_var");
        et_i   = expr_make(2, id);

        add_instr("SET_P %s\n", v_name[id]);
        oper_divi(et_i, expr_make(2, 0));
    }

    exec_atan(expr_make(2, 0));

    acc_ok = 1;
    return expr_make(2, 0);
}

// joins two real numbers into a complex
expr exec_comp(expr er, expr ei)
{
    // ------------------------------------------------------------------------
    // consistency check ------------------------------------------------------
    // ------------------------------------------------------------------------

    // check whether etr was declared
    if (er.id != 0 && v_table[er.id].type == 0) {fprintf(stderr, MSG_ERR_DECL_FIRST, line_num+1, rem_fname(v_name[er.id], fname)); exit(EXIT_FAILURE);}

    // check whether eti was declared
    if (ei.id != 0 && v_table[ei.id].type == 0) {fprintf(stderr, MSG_ERR_DECL_FIRST, line_num+1, rem_fname(v_name[ei.id], fname)); exit(EXIT_FAILURE);}

    // check whether etr is a variable
    if (er.id != 0 && v_table[er.id].isar > 0) {fprintf(stderr, MSG_ERR_WRONG_USE, line_num+1, rem_fname(v_name[er.id], fname)); exit(EXIT_FAILURE);}

    // check whether et is a variable
    if (ei.id != 0 && v_table[ei.id].isar > 0) {fprintf(stderr, MSG_ERR_WRONG_USE, line_num+1, rem_fname(v_name[ei.id], fname)); exit(EXIT_FAILURE);}

    // check whether it is comp
    if (er.type > 2 || ei.type > 2) {fprintf (stderr, MSG_ERR_COMPLEX_OF_COMPLEX, line_num+1); exit(EXIT_FAILURE);}

    // ------------------------------------------------------------------------
    // update variable status -------------------------------------------------
    // ------------------------------------------------------------------------

    if (er.id != 0) v_table[er.id].used = 1;
    if (ei.id != 0) v_table[ei.id].used = 1;

    // ------------------------------------------------------------------------
    // prepare local variables ------------------------------------------------
    // ------------------------------------------------------------------------

    char ld [10]; if (acc_ok == 0) strcpy(ld ,"LOD"  ); else strcpy(ld ,"P_LOD"  );
    char i2f[10]; if (acc_ok == 0) strcpy(i2f,"I2F_M"); else strcpy(i2f,"P_I2F_M");

    // ------------------------------------------------------------------------
    // execute ----------------------------------------------------------------
    // ------------------------------------------------------------------------

    // int in memory and int in memory
    if ((er.type == 1) && (er.id != 0) && (ei.type == 1) && (ei.id != 0))
    {
        add_instr("%s %s\n", i2f, v_name[er.id]);
        add_instr("P_I2F_M %s\n", v_name[ei.id]);
    }

    // int in memory and int in acc
    if ((er.type == 1) && (er.id != 0) && (ei.type == 1) && (ei.id == 0))
    {
        add_instr("SET aux_var\n");
        add_instr("I2F_M %s\n", v_name[er.id]);
        add_instr("P_I2F_M aux_var\n");
    }

    // int in memory and float in memory
    if ((er.type == 1) && (er.id != 0) && (ei.type == 2) && (ei.id != 0))
    {
        add_instr("%s %s\n", i2f, v_name[er.id]);
        add_instr("P_LOD %s\n"  , v_name[ei.id]);
    }

    // int in memory and float in acc
    if ((er.type == 1) && (er.id != 0) && (ei.type == 2) && (ei.id == 0))
    {
        add_instr("SET aux_var\n");
        add_instr("I2F_M %s\n", v_name[er.id]);
        add_instr("P_LOD aux_var\n");
    }

    // int in acc and int in memory
    if ((er.type == 1) && (er.id == 0) && (ei.type == 1) && (ei.id != 0))
    {
        add_instr("I2F\n");
        add_instr("P_I2F_M %s\n", v_name[ei.id]);
    }

    // int in acc and int in acc
    if ((er.type == 1) && (er.id == 0) && (ei.type == 1) && (ei.id == 0))
    {
        add_instr("SET_P aux_var\n");
        add_instr("I2F\n");
        add_instr("P_I2F_M aux_var\n");
    }

    // int in acc and float in memory
    if ((er.type == 1) && (er.id == 0) && (ei.type == 2) && (ei.id != 0))
    {
        add_instr("I2F\n");
        add_instr("P_LOD %s\n", v_name[ei.id]);
    }

    // int in acc and float in acc
    if ((er.type == 1) && (er.id == 0) && (ei.type == 2) && (ei.id == 0))
    {
        add_instr("SET_P aux_var\n");
        add_instr("I2F\n");
        add_instr("P_LOD aux_var\n");
    }

    // float in memory and int in memory
    if ((er.type == 2) && (er.id != 0) && (ei.type == 1) && (ei.id != 0))
    {
        add_instr("%s %s\n",  ld, v_name[er.id]);
        add_instr("P_I2F_M %s\n", v_name[ei.id]);
    }

    // float in memory and int in acc
    if ((er.type == 2) && (er.id != 0) && (ei.type == 1) && (ei.id == 0))
    {
        add_instr("SET aux_var\n");
        add_instr("LOD %s\n", v_name[er.id]);
        add_instr("P_I2F_M aux_var\n");
    }

    // float in memory and float in memory
    if ((er.type == 2) && (er.id != 0) && (ei.type == 2) && (ei.id != 0))
    {
        add_instr("%s %s\n", ld, v_name[er.id]);
        add_instr("P_LOD %s\n" , v_name[ei.id]);
    }

    // float in memory and float in acc
    if ((er.type == 2) && (er.id != 0) && (ei.type == 2) && (ei.id == 0))
    {
        add_instr("SET aux_var\n");
        add_instr("LOD %s\n", v_name[er.id]);
        add_instr("P_LOD aux_var\n");
    }

    // float in acc and int in memory
    if ((er.type == 2) && (er.id == 0) && (ei.type == 1) && (ei.id != 0))
    {
        add_instr("P_I2F_M %s\n", v_name[ei.id]);
    }

    // float in acc and int in acc
    if ((er.type == 2) && (er.id == 0) && (ei.type == 1) && (ei.id == 0))
    {
        add_instr("I2F\n");
    }

    // float in acc and float in memory
    if ((er.type == 2) && (er.id == 0) && (ei.type == 2) && (ei.id != 0))
    {
        add_instr("P_LOD %s\n", v_name[ei.id]);
    }

    // float in acc and float in acc
    if ((er.type == 1) && (er.id == 0) && (ei.type == 2) && (ei.id == 0))
    {
        // nothing to do
    }

    acc_ok = 1;
    return expr_make(3, 0);
}

// ----------------------------------------------------------------------------
// special functions for vector work ------------------------------------------
// does not create stdlib functions directly ----------------------------------
// instead, uses Dirac notation in statements --------------------------------
// ----------------------------------------------------------------------------

// multiplication between two vectors, e.g. <a|b>
// this routine produces an exp
expr exec_vtv(int id1, int id2)
{
    // ------------------------------------------------------------------------
    // consistency check ------------------------------------------------------
    // ------------------------------------------------------------------------

    // check whether id1 was declared
    if (v_table[id1].type == 0) {fprintf(stderr, MSG_ERR_VAR_NOT_FOUND, line_num+1, rem_fname(v_name[id1], fname)); exit(EXIT_FAILURE);}

    // check whether id2 was declared
    if (v_table[id2].type == 0) {fprintf(stderr, MSG_ERR_VAR_NOT_FOUND, line_num+1, rem_fname(v_name[id2], fname)); exit(EXIT_FAILURE);}

    // check that they really are vectors
    if (v_table[id1].isar != 1 || v_table[id2].isar != 1) {fprintf(stderr, MSG_ERR_INNER_NEEDS_VECTORS, line_num+1); exit(EXIT_FAILURE);}

    // check that the sizes match
    if (v_table[id1].size != v_table[id2].size) {fprintf(stderr, MSG_ERR_VECTOR_SIZE_DIFF, line_num+1); exit(EXIT_FAILURE);}

    // check that they are the same type
    if (v_table[id1].type != v_table[id2].type) {fprintf(stderr, MSG_ERR_TYPE_DIFF, line_num+1); exit(EXIT_FAILURE);}

    // check whether there is a comp variable
    if (v_table[id1].type == 3 || v_table[id2].type == 3) {fprintf(stderr, MSG_ERR_NOT_IMPL_COMPLEX, line_num+1); exit(EXIT_FAILURE);}

    // ------------------------------------------------------------------------
    // update variable status -------------------------------------------------
    // ------------------------------------------------------------------------

    v_table[id1].used = 1;
    v_table[id2].used = 1;

    // ------------------------------------------------------------------------
    // prepare local variables ------------------------------------------------
    // ------------------------------------------------------------------------

    int N = v_table[id1].size;

    char ld[10]; if (acc_ok == 0) strcpy(ld,"LOD"); else strcpy(ld,"P_LOD");

    // ------------------------------------------------------------------------
    // implements the vector product ------------------------------------------
    // ------------------------------------------------------------------------

    printf(MSG_INFO_DIRAC_INNER, line_num+1);

    // implement every combination

    // int with int
    if ((v_table[id1].type == 1) && (v_table[id2].type == 1))
    {
        add_instr( "%s %s\n", ld, v_name[id1]);
        add_instr("MLT %s\n",     v_name[id2]);

        for (int i = 1; i < N; i++)
        {
            add_instr("P_LOD_V %s %d\n", v_name[id1], i);
            add_instr(  "MLT_V %s %d\n", v_name[id2], i);
            add_instr("S_ADD\n");
        }
    }

    // float with float
    if ((v_table[id1].type == 2) && (v_table[id2].type == 2))
    {
        add_instr(   "%s %s\n", ld, v_name[id1]);
        add_instr("F_MLT %s\n",     v_name[id2]);

        for (int i = 1; i < N; i++)
        {
            add_instr("P_LOD_V %s %d\n", v_name[id1], i);
            add_instr("F_MLT_V %s %d\n", v_name[id2], i);
            add_instr("SF_ADD\n");
        }
    }

    acc_ok = 1;
    return expr_make(v_table[id1].type, 0);
}

// matrix-vector multiplication, e.g. A # |B|b>;
void exec_Mv(int idy, int idM, int idv)
{
    // ------------------------------------------------------------------------
    // consistency check ------------------------------------------------------
    // ------------------------------------------------------------------------

    // check whether idy was declared
    if (v_table[idy].type == 0) {fprintf(stderr, MSG_ERR_DECL_FIRST, line_num+1, rem_fname(v_name[idy], fname)); exit(EXIT_FAILURE);}

    // check whether idM was declared
    if (v_table[idM].type == 0) {fprintf(stderr, MSG_ERR_DECL_FIRST, line_num+1, rem_fname(v_name[idM], fname)); exit(EXIT_FAILURE);}

    // check whether idv was declared
    if (v_table[idv].type == 0) {fprintf(stderr, MSG_ERR_DECL_FIRST, line_num+1, rem_fname(v_name[idv], fname)); exit(EXIT_FAILURE);}

    // check that the types match
    if (v_table[idy].type != v_table[idM].type || v_table[idy].type != v_table[idv].type) {fprintf(stderr, MSG_ERR_VARS_SAME_TYPE, line_num+1); exit(EXIT_FAILURE);}

    // check that it is not comp
    if (v_table[idy].type == 3 || v_table[idM].type == 3 || v_table[idv].type == 3) {fprintf(stderr, MSG_ERR_NOT_IMPL_COMPLEX, line_num+1); exit(EXIT_FAILURE);}

    // check whether idy is a vector
    if (v_table[idy].isar != 1) {fprintf(stderr, MSG_ERR_NOT_A_VECTOR, line_num+1, rem_fname(v_name[idy], fname)); exit(EXIT_FAILURE);}

    // check whether idM is a matrix
    if (v_table[idM].isar != 2) {fprintf(stderr, MSG_ERR_NOT_A_MATRIX2, line_num+1, rem_fname(v_name[idM], fname)); exit(EXIT_FAILURE);}

    // check whether idv is a vector
    if (v_table[idv].isar != 1) {fprintf(stderr, MSG_ERR_NOT_A_VECTOR, line_num+1, rem_fname(v_name[idv], fname)); exit(EXIT_FAILURE);}

    // check size between output and matrix
    if (v_table[idy].size != v_table[idM].size) {fprintf(stderr, MSG_ERR_MATRIX_ROW_MISMATCH, line_num+1, rem_fname(v_name[idM], fname), rem_fname(v_name[idy], fname)); exit(EXIT_FAILURE);}

    // check size between matrix and vector
    if (v_table[idv].size != v_table[idM].siz2) {fprintf(stderr, MSG_ERR_MATRIX_COL_MISMATCH, line_num+1, rem_fname(v_name[idM], fname), rem_fname(v_name[idv], fname)); exit(EXIT_FAILURE);}

    // ------------------------------------------------------------------------
    // update variable status -------------------------------------------------
    // ------------------------------------------------------------------------

    v_table[idM].used = 1;
    v_table[idv].used = 1;

    // ------------------------------------------------------------------------
    // prepare local variables ------------------------------------------------
    // ------------------------------------------------------------------------

    int N = v_table[idM].size;
    int M = v_table[idM].siz2;

    // ------------------------------------------------------------------------
    // implements the matrix-vector product -----------------------------------
    // ------------------------------------------------------------------------

    printf(MSG_INFO_DIRAC_MV, line_num+1);

    // implement combinations only on demand

    // int with int
    if ((v_table[idM].type == 1) && (v_table[idv].type == 1))
    {
        for (int i = 0; i < N; i++)
        {
            add_instr("LOD_V %s %d\n", v_name[idM], i*M);
            add_instr("MLT %s\n"     , v_name[idv]);

            for (int j = 1; j < M; j++)
            {
                add_instr("P_LOD_V %s %d\n", v_name[idM], i*M+j);
                add_instr(  "MLT_V %s %d\n", v_name[idv],     j);
                add_instr("S_ADD\n");
            }

            add_instr("SET_V %s %d\n", v_name[idy], i);
        }
    }

    // float with float
    if ((v_table[idM].type == 2) && (v_table[idv].type == 2))
    {
        for (int i = 0; i < N; i++)
        {
            add_instr("LOD_V %s %d\n", v_name[idM], i*M);
            add_instr("F_MLT %s\n"   , v_name[idv]);

            for (int j = 1; j < M; j++)
            {
                add_instr("P_LOD_V %s %d\n", v_name[idM], i*M+j);
                add_instr("F_MLT_V %s %d\n", v_name[idv],     j);
                add_instr("SF_ADD\n");
            }

            add_instr("SET_V %s %d\n", v_name[idy], i);
        }
    }
}

// constant-vector multiplication, e.g. a # c|b>;
void exec_cv(int idy, expr e, int idv)
{
    // ------------------------------------------------------------------------
    // consistency check ------------------------------------------------------
    // ------------------------------------------------------------------------

    // check whether idy was declared
    if (v_table[idy].type == 0) {fprintf(stderr, MSG_ERR_DECL_FIRST, line_num+1, rem_fname(v_name[idy], fname)); exit(EXIT_FAILURE);}

    // check whether et was declared
    if (e.id != 0 && v_table[e.id].type == 0) {fprintf(stderr, MSG_ERR_DECL_FIRST, line_num+1, rem_fname(v_name[e.id], fname)); exit(EXIT_FAILURE);}

    // check whether idv was declared
    if (v_table[idv].type == 0) {fprintf(stderr, MSG_ERR_DECL_FIRST, line_num+1, rem_fname(v_name[idv], fname)); exit(EXIT_FAILURE);}

    // check that the types match
    if (v_table[idy].type != e.type || v_table[idy].type != v_table[idv].type) {fprintf(stderr, MSG_ERR_VARS_SAME_TYPE, line_num+1); exit(EXIT_FAILURE);}

    // check that it is not comp
    if (v_table[idy].type == 3 || e.type == 3 || v_table[idv].type == 3) {fprintf(stderr, MSG_ERR_NOT_IMPL_COMPLEX, line_num+1); exit(EXIT_FAILURE);}

    // check whether idy is a vector
    if (v_table[idy].isar != 1) {fprintf(stderr, MSG_ERR_NOT_A_VECTOR, line_num+1, rem_fname(v_name[idy], fname)); exit(EXIT_FAILURE);}

    // check whether et is a variable
    if (e.id != 0 && v_table[e.id].isar > 0) {fprintf(stderr, MSG_ERR_WRONG_USE, line_num+1, rem_fname(v_name[e.id], fname)); exit(EXIT_FAILURE);}

    // check whether idv is a vector
    if (v_table[idv].isar != 1) {fprintf(stderr, MSG_ERR_NOT_A_VECTOR, line_num+1, rem_fname(v_name[idv], fname)); exit(EXIT_FAILURE);}

    // check size between vectors
    if (v_table[idy].size != v_table[idv].size) {fprintf(stderr, MSG_ERR_VECTOR_SIZE_DIFF2, line_num+1); exit(EXIT_FAILURE);}

    // ------------------------------------------------------------------------
    // update variable status -------------------------------------------------
    // ------------------------------------------------------------------------

    if (e.id != 0) v_table[e.id].used = 1;
    v_table[idv].used = 1;

    // ------------------------------------------------------------------------
    // prepare local variables ------------------------------------------------
    // ------------------------------------------------------------------------

    int N = v_table[idv].size;

    char g[64]; if (e.id==0) strcpy(g,"aux_var"); else strcpy(g,v_name[e.id]);

    // ------------------------------------------------------------------------
    // execute ----------------------------------------------------------------
    // ------------------------------------------------------------------------

    printf(MSG_INFO_DIRAC_CV, line_num+1);

    if (e.id==0) add_instr("SET aux_var\n");

    // implement combinations on demand

    for (int i = 0; i < N; i++)
    {
        add_instr("LOD_V %s %d\n", v_name[idv], i);

        // int with int
        if (v_table[idy].type == 1)
        {
            add_instr("MLT %s\n", g);
        }

        // float with float
        if (v_table[idy].type == 2)
        {
            add_instr("F_MLT %s\n", g);
        }

        add_instr("SET_V %s %d\n", v_name[idy], i);
    }

    acc_ok = 0;
}

// weighted sum into the second vector, e.g. a # |b> + c|d>;
// added mainly for RLS use
void exec_apcb(int idy, int ida, expr ec, int idb)
{
    // ------------------------------------------------------------------------
    // consistency check ------------------------------------------------------
    // ------------------------------------------------------------------------

    // check whether idy was declared
    if (v_table[idy].type == 0) {fprintf(stderr, MSG_ERR_DECL_FIRST, line_num+1, rem_fname(v_name[idy], fname)); exit(EXIT_FAILURE);}

    // check whether ida was declared
    if (v_table[ida].type == 0) {fprintf(stderr, MSG_ERR_DECL_FIRST, line_num+1, rem_fname(v_name[ida], fname)); exit(EXIT_FAILURE);}

    // check whether etc was declared
    if (ec.id != 0 && v_table[ec.id].type == 0) {fprintf(stderr, MSG_ERR_DECL_FIRST, line_num+1, rem_fname(v_name[ec.id], fname)); exit(EXIT_FAILURE);}
    
    // check whether idb was declared
    if (v_table[idb].type == 0) {fprintf(stderr, MSG_ERR_DECL_FIRST, line_num+1, rem_fname(v_name[idb], fname)); exit(EXIT_FAILURE);}

    // check that the types match
    if (v_table[idy].type != v_table[ida].type || v_table[idy].type != ec.type || v_table[idy].type != v_table[idb].type) {fprintf(stderr, MSG_ERR_VARS_SAME_TYPE, line_num+1); exit(EXIT_FAILURE);}

    // check that it is not comp
    if (v_table[idy].type == 3 || v_table[ida].type == 3 || ec.type == 3 || v_table[idb].type == 3) {fprintf(stderr, MSG_ERR_NOT_IMPL_COMPLEX, line_num+1); exit(EXIT_FAILURE);}

    // check whether idy is a vector
    if (v_table[idy].isar != 1) {fprintf(stderr, MSG_ERR_NOT_A_VECTOR, line_num+1, rem_fname(v_name[idy], fname)); exit(EXIT_FAILURE);}

    // check whether ida is a vector
    if (v_table[ida].isar != 1) {fprintf(stderr, MSG_ERR_NOT_A_VECTOR, line_num+1, rem_fname(v_name[ida], fname)); exit(EXIT_FAILURE);}

    // check whether et is a variable
    if (ec.id != 0 && v_table[ec.id].isar > 0) {fprintf(stderr, MSG_ERR_WRONG_USE, line_num+1, rem_fname(v_name[ec.id], fname)); exit(EXIT_FAILURE);}

    // check whether idb is a vector
    if (v_table[idb].isar != 1) {fprintf(stderr, MSG_ERR_NOT_A_VECTOR, line_num+1, rem_fname(v_name[idb], fname)); exit(EXIT_FAILURE);}

    // check size between vectors
    if (v_table[idy].size != v_table[ida].size) {fprintf(stderr, MSG_ERR_VECTOR_SIZE_DIFF2, line_num+1); exit(EXIT_FAILURE);}

    // check size between vectors
    if (v_table[idy].size != v_table[idb].size) {fprintf(stderr, MSG_ERR_VECTOR_SIZE_DIFF2, line_num+1); exit(EXIT_FAILURE);}

    // ------------------------------------------------------------------------
    // update variable status -------------------------------------------------
    // ------------------------------------------------------------------------

    v_table[ida].used = 1;
    if (ec.id != 0) v_table[ec.id].used = 1;
    v_table[idb].used = 1;

    // ------------------------------------------------------------------------
    // prepare local variables ------------------------------------------------
    // ------------------------------------------------------------------------

    printf(MSG_INFO_DIRAC_VECTOR_SUM, line_num+1);

    int N = v_table[idy].size;

    char g[64]; if (ec.id==0) strcpy(g,"aux_var"); else strcpy(g,v_name[ec.id]);

    // ------------------------------------------------------------------------
    // execute ----------------------------------------------------------------
    // ------------------------------------------------------------------------

    if (ec.id==0) add_instr("SET aux_var\n");

    for (int i = 0; i < N; i++)
    {
        add_instr("LOD_V %s %d\n", v_name[idb], i);

        // int
        if (v_table[idy].type == 1)
        {
            add_instr("MLT %s\n", g);
            add_instr("ADD_V %s %d\n", v_name[ida], i);
        }

        // float
        if (v_table[idy].type == 2)
        {
            add_instr("F_MLT %s\n", g);
            add_instr("F_ADD_V %s %d\n", v_name[ida], i);
        }

        add_instr("SET_V %s %d\n", v_name[idy], i);
    }

    acc_ok = 0;
}

// outer product between two vectors, e.g. A # |a><b|;
void exec_vvt(int idM, int ida, int idb)
{
    // ------------------------------------------------------------------------
    // consistency check ------------------------------------------------------
    // ------------------------------------------------------------------------

    // check whether idM was declared
    if (v_table[idM].type == 0) {fprintf(stderr, MSG_ERR_DECL_FIRST, line_num+1, rem_fname(v_name[idM], fname)); exit(EXIT_FAILURE);}

    // check whether ida was declared
    if (v_table[ida].type == 0) {fprintf(stderr, MSG_ERR_DECL_FIRST, line_num+1, rem_fname(v_name[ida], fname)); exit(EXIT_FAILURE);}
    
    // check whether idb was declared
    if (v_table[idb].type == 0) {fprintf(stderr, MSG_ERR_DECL_FIRST, line_num+1, rem_fname(v_name[idb], fname)); exit(EXIT_FAILURE);}

    // check that the types match
    if (v_table[idM].type != v_table[ida].type || v_table[idM].type != v_table[idb].type) {fprintf(stderr, MSG_ERR_VARS_SAME_TYPE, line_num+1); exit(EXIT_FAILURE);}

    // check that it is not comp
    if (v_table[idM].type == 3 || v_table[ida].type == 3 || v_table[idb].type == 3) {fprintf(stderr, MSG_ERR_NOT_IMPL_COMPLEX, line_num+1); exit(EXIT_FAILURE);}

    // check whether idM is a matrix
    if (v_table[idM].isar != 2) {fprintf(stderr, MSG_ERR_NOT_A_MATRIX, line_num+1, rem_fname(v_name[idM], fname)); exit(EXIT_FAILURE);}

    // check whether ida is a vector
    if (v_table[ida].isar != 1) {fprintf(stderr, MSG_ERR_NOT_A_VECTOR, line_num+1, rem_fname(v_name[ida], fname)); exit(EXIT_FAILURE);}

    // check whether idb is a vector
    if (v_table[idb].isar != 1) {fprintf(stderr, MSG_ERR_NOT_A_VECTOR, line_num+1, rem_fname(v_name[idb], fname)); exit(EXIT_FAILURE);}

    // check size between elements
    if (v_table[idM].size != v_table[ida].size || v_table[idM].size != v_table[idb].size) {fprintf(stderr, MSG_ERR_DIM_MISMATCH, line_num+1); exit(EXIT_FAILURE);}

    // ------------------------------------------------------------------------
    // update variable status -------------------------------------------------
    // ------------------------------------------------------------------------

    v_table[ida].used = 1;
    v_table[idb].used = 1;

    // ------------------------------------------------------------------------
    // prepare local variables ------------------------------------------------
    // ------------------------------------------------------------------------

    int N = v_table[ida].size;

    // ------------------------------------------------------------------------
    // execute ----------------------------------------------------------------
    // ------------------------------------------------------------------------

    printf(MSG_INFO_DIRAC_OUTER, line_num+1);

    for (int i = 0; i < N; i++)
    {
        for (int j = 0; j < N; j++)
        {
            add_instr("LOD_V %s %d\n", v_name[ida], i);

            // int
            if (v_table[idM].type == 1)
            {
                add_instr("MLT_V %s %d\n", v_name[idb], j);
            }

            // float
            if (v_table[idM].type == 2)
            {
                add_instr("F_MLT_V %s %d\n", v_name[idb], j);
            }

            add_instr("SET_V %s %d\n", v_name[idM], N*j+i);
        }
    }
}

// matrix subtraction with outer product, e.g. A # B - |a><b|;
// added mainly for RLS use
void exec_Mmvvt(int idA, int idB, int ida, int idb)
{
    // ------------------------------------------------------------------------
    // consistency check ------------------------------------------------------
    // ------------------------------------------------------------------------

    // check whether idA was declared
    if (v_table[idA].type == 0) {fprintf(stderr, MSG_ERR_DECL_FIRST, line_num+1, rem_fname(v_name[idA], fname)); exit(EXIT_FAILURE);}

    // check whether idA was declared
    if (v_table[idB].type == 0) {fprintf(stderr, MSG_ERR_DECL_FIRST, line_num+1, rem_fname(v_name[idB], fname)); exit(EXIT_FAILURE);}
    
    // check whether ida was declared
    if (v_table[ida].type == 0) {fprintf(stderr, MSG_ERR_DECL_FIRST, line_num+1, rem_fname(v_name[ida], fname)); exit(EXIT_FAILURE);}

    // check whether idb was declared
    if (v_table[idb].type == 0) {fprintf(stderr, MSG_ERR_DECL_FIRST, line_num+1, rem_fname(v_name[idb], fname)); exit(EXIT_FAILURE);}

    // check that the types match
    if (v_table[idA].type != v_table[idB].type || v_table[idA].type != v_table[ida].type || v_table[idA].type != v_table[idb].type) {fprintf(stderr, MSG_ERR_VARS_SAME_TYPE, line_num+1); exit(EXIT_FAILURE);}

    // check that it is not comp
    if (v_table[idA].type == 3 || v_table[idB].type == 3 || v_table[ida].type == 3 || v_table[idb].type == 3) {fprintf(stderr, MSG_ERR_NOT_IMPL_COMPLEX, line_num+1); exit(EXIT_FAILURE);}
    // check whether idA is a matrix
    if (v_table[idA].isar != 2) {fprintf(stderr, MSG_ERR_NOT_A_MATRIX, line_num+1, rem_fname(v_name[idA], fname)); exit(EXIT_FAILURE);}

    // check whether idB is a matrix
    if (v_table[idB].isar != 2) {fprintf(stderr, MSG_ERR_NOT_A_MATRIX, line_num+1, rem_fname(v_name[idB], fname)); exit(EXIT_FAILURE);}

    // check whether ida is a vector
    if (v_table[ida].isar != 1) {fprintf(stderr, MSG_ERR_NOT_A_VECTOR, line_num+1, rem_fname(v_name[ida], fname)); exit(EXIT_FAILURE);}

    // check whether idb is a vector
    if (v_table[idb].isar != 1) {fprintf(stderr, MSG_ERR_NOT_A_VECTOR, line_num+1, rem_fname(v_name[idb], fname)); exit(EXIT_FAILURE);}

    // check size between elements
    if (v_table[idA].size != v_table[idA].siz2 || v_table[idA].size != v_table[idB].size || v_table[idA].size != v_table[idB].siz2 || v_table[idA].size != v_table[ida].size ||
        v_table[idA].size != v_table[idb].size) {fprintf(stderr, MSG_ERR_DIM_MISMATCH, line_num+1); exit(EXIT_FAILURE);}

    // ------------------------------------------------------------------------
    // update variable status -------------------------------------------------
    // ------------------------------------------------------------------------

    v_table[idB].used = 1;
    v_table[ida].used = 1;
    v_table[idb].used = 1;

    // ------------------------------------------------------------------------
    // prepare local variables ------------------------------------------------
    // ------------------------------------------------------------------------

    int N = v_table[ida].size;

    // ------------------------------------------------------------------------
    // execute ----------------------------------------------------------------
    // ------------------------------------------------------------------------

    printf(MSG_INFO_DIRAC_OUTER, line_num+1);
    printf(MSG_INFO_DIRAC_MATRIX_SUM, line_num+1);

    for (int i = 0; i < N; i++)
    {
        for (int j = 0; j < N; j++)
        {
            add_instr("LOD_V %s %d\n", v_name[ida], i);

            // int
            if (v_table[idA].type == 1)
            {
                add_instr("MLT_V %s %d\n", v_name[idb],     j);
                add_instr("NEG\n");
                add_instr("ADD_V %s %d\n", v_name[idB], N*j+i);
            }

            // float
            if (v_table[idA].type == 2)
            {
                add_instr("F_MLT_V %s %d\n", v_name[idb],     j);
                add_instr("F_NEG\n");
                add_instr("F_ADD_V %s %d\n", v_name[idB], N*j+i);
            }

            add_instr("SET_V %s %d\n", v_name[idA], N*j+i);
        }
    }
}

// product of constant and matrix, e.g. A # c|B|;
void exec_cM(int idA, expr ec, int idM)
{
    // ------------------------------------------------------------------------
    // consistency check ------------------------------------------------------
    // ------------------------------------------------------------------------

    // check whether idA was declared
    if (v_table[idA].type == 0) {fprintf(stderr, MSG_ERR_DECL_FIRST, line_num+1, rem_fname(v_name[idA], fname)); exit(EXIT_FAILURE);}

    // check whether etc was declared
    if (ec.id != 0 && v_table[ec.id].type == 0) {fprintf(stderr, MSG_ERR_DECL_FIRST, line_num+1, rem_fname(v_name[ec.id], fname)); exit(EXIT_FAILURE);}
    
    // check whether idM was declared
    if (v_table[idM].type == 0) {fprintf(stderr, MSG_ERR_DECL_FIRST, line_num+1, rem_fname(v_name[idM], fname)); exit(EXIT_FAILURE);}

    // check that the types match
    if (v_table[idA].type != ec.type || v_table[idA].type != v_table[idM].type) {fprintf(stderr, MSG_ERR_VARS_SAME_TYPE, line_num+1); exit(EXIT_FAILURE);}

    // check that it is not comp
    if (v_table[idA].type == 3 || ec.type == 3 || v_table[idM].type == 3) {fprintf(stderr, MSG_ERR_NOT_IMPL_COMPLEX, line_num+1); exit(EXIT_FAILURE);}

    // check whether idA is a matrix
    if (v_table[idA].isar != 2) {fprintf(stderr, MSG_ERR_NOT_A_MATRIX, line_num+1, rem_fname(v_name[idA], fname)); exit(EXIT_FAILURE);}

    // check whether etc is a variable
    if (ec.id != 0 && v_table[ec.id].isar > 0) {fprintf(stderr, MSG_ERR_WRONG_USE, line_num+1, rem_fname(v_name[ec.id], fname)); exit(EXIT_FAILURE);}

    // check whether idM is a matrix
    if (v_table[idM].isar != 2) {fprintf(stderr, MSG_ERR_NOT_A_MATRIX, line_num+1, rem_fname(v_name[idM], fname)); exit(EXIT_FAILURE);}

    // check size between elements
    if (v_table[idA].size != v_table[idM].size || v_table[idA].siz2 != v_table[idM].siz2) {fprintf(stderr, MSG_ERR_DIM_MISMATCH, line_num+1); exit(EXIT_FAILURE);}

    // ------------------------------------------------------------------------
    // update variable status -------------------------------------------------
    // ------------------------------------------------------------------------

    if (ec.id != 0) v_table[ec.id].used = 1;
    v_table[idM].used = 1;

    // ------------------------------------------------------------------------
    // prepare local variables ------------------------------------------------
    // ------------------------------------------------------------------------

    int N = v_table[idM].size;
    int M = v_table[idM].siz2;

    char g[64]; if (ec.id==0) strcpy(g,"aux_var"); else strcpy(g,v_name[ec.id]);

    // ------------------------------------------------------------------------
    // execute ----------------------------------------------------------------
    // ------------------------------------------------------------------------

    printf(MSG_INFO_DIRAC_CM, line_num+1);

    if (ec.id==0) add_instr("SET aux_var\n");

    for (int i = 0; i < N; i++)
    {
        for (int j = 0; j < M; j++)
        {
            add_instr("LOD_V %s %d\n", v_name[idM], M*i+j);

            // int
            if (v_table[idA].type == 1) add_instr("MLT %s\n", g);

            // float
            if (v_table[idA].type == 2) add_instr("F_MLT %s\n", g);
            
            add_instr("SET_V %s %d\n", v_name[idA], M*i+j);
        }
    }

    acc_ok = 0;
}

// generates identity matrix with constant, e.g. A # c|I|;
void exec_cI(int idM, expr ec)
{
    // ------------------------------------------------------------------------
    // consistency check ------------------------------------------------------
    // ------------------------------------------------------------------------

    // check whether idM was declared
    if (v_table[idM].type == 0) {fprintf(stderr, MSG_ERR_DECL_FIRST, line_num+1, rem_fname(v_name[idM], fname)); exit(EXIT_FAILURE);}

    // check whether etc was declared
    if (ec.id != 0 && v_table[ec.id].type == 0) {fprintf(stderr, MSG_ERR_DECL_FIRST, line_num+1, rem_fname(v_name[ec.id], fname)); exit(EXIT_FAILURE);}

    // check that the types match
    if (v_table[idM].type != ec.type) {fprintf(stderr, MSG_ERR_VARS_SAME_TYPE, line_num+1); exit(EXIT_FAILURE);}

    // check that it is not comp
    if (v_table[idM].type == 3 || ec.type == 3) {fprintf(stderr, MSG_ERR_NOT_IMPL_COMPLEX, line_num+1); exit(EXIT_FAILURE);}
    // check whether idM is a matrix
    if (v_table[idM].isar != 2) {fprintf(stderr, MSG_ERR_NOT_A_MATRIX, line_num+1, rem_fname(v_name[idM], fname)); exit(EXIT_FAILURE);}

    // check whether etc is a variable
    if (ec.id != 0 && v_table[ec.id].isar > 0) {fprintf(stderr, MSG_ERR_WRONG_USE, line_num+1, rem_fname(v_name[ec.id], fname)); exit(EXIT_FAILURE);}

    // ------------------------------------------------------------------------
    // update variable status -------------------------------------------------
    // ------------------------------------------------------------------------

    if (ec.id != 0) v_table[ec.id].used = 1;

    // ------------------------------------------------------------------------
    // prepare local variables ------------------------------------------------
    // ------------------------------------------------------------------------

    int N = v_table[idM].size;

    // ------------------------------------------------------------------------
    // execute ----------------------------------------------------------------
    // ------------------------------------------------------------------------

    printf(MSG_INFO_DIRAC_CM, line_num+1);

    if (ec.id!=0) add_instr("LOD %s\n",v_name[ec.id]);

    for (int i = 0; i < N; i++)
    {
        for (int j = 0; j < N; j++)
        {
            if (i == j) add_instr("SET_V %s %d\n", v_name[idM], N*i+j);
        }
    }

    // int
    if (v_table[idM].type == 1) add_instr("LOD 0\n");

    // float
    if (v_table[idM].type == 2) add_instr("LOD 0.0\n");
    
    for (int i = 0; i < N; i++)
    {
        for (int j = 0; j < N; j++)
        {
            if (i != j) add_instr("SET_V %s %d\n", v_name[idM], N*i+j);
        }
    }

    acc_ok = 0;
}

// generates a zero vector, e.g. a # |0>;
void exec_v0(int idv)
{
    // ------------------------------------------------------------------------
    // consistency check ------------------------------------------------------
    // ------------------------------------------------------------------------

    // check whether idv was declared
    if (v_table[idv].type == 0) {fprintf(stderr, MSG_ERR_DECL_FIRST, line_num+1, rem_fname(v_name[idv], fname)); exit(EXIT_FAILURE);}

    // check that it is not comp
    if (v_table[idv].type == 3) {fprintf(stderr, MSG_ERR_NOT_IMPL_COMPLEX, line_num+1); exit(EXIT_FAILURE);}
    
    // check whether idv is a vector
    if (v_table[idv].isar != 1) {fprintf(stderr, MSG_ERR_NOT_A_VECTOR, line_num+1, rem_fname(v_name[idv], fname)); exit(EXIT_FAILURE);}

    // ------------------------------------------------------------------------
    // prepare local variables ------------------------------------------------
    // ------------------------------------------------------------------------

    int N = v_table[idv].size;

    // ------------------------------------------------------------------------
    // execute ----------------------------------------------------------------
    // ------------------------------------------------------------------------

    printf(MSG_INFO_DIRAC_ZERO_VECTOR, line_num+1);

    // int
    if (v_table[idv].type == 1) add_instr("LOD 0\n");

    // float
    if (v_table[idv].type == 2) add_instr("LOD 0.0\n");

    for (int i = 0; i < N; i++) add_instr("SET_V %s %d\n", v_name[idv], i);
}

// reads input vector with weight c, e.g. a # c|in(0)>;
void exec_cvin(int idv, expr ec, int idp)
{
    // ------------------------------------------------------------------------
    // consistency check ------------------------------------------------------
    // ------------------------------------------------------------------------

    // check whether idv was declared
    if (v_table[idv].type == 0) {fprintf(stderr, MSG_ERR_DECL_FIRST, line_num+1, rem_fname(v_name[idv], fname)); exit(EXIT_FAILURE);}

    // check whether etc was declared
    if (ec.id != 0 && v_table[ec.id].type == 0) {fprintf(stderr, MSG_ERR_DECL_FIRST, line_num+1, rem_fname(v_name[ec.id], fname)); exit(EXIT_FAILURE);}

    // check that it is not comp
    if (v_table[idv].type == 3 || ec.type == 3) {fprintf(stderr, MSG_ERR_NOT_IMPL_COMPLEX, line_num+1); exit(EXIT_FAILURE);}

    // check whether idv is a vector
    if (v_table[idv].isar != 1) {fprintf(stderr, MSG_ERR_NOT_A_VECTOR, line_num+1, rem_fname(v_name[idv], fname)); exit(EXIT_FAILURE);}

    // check whether etc is a variable
    if (ec.id != 0 && v_table[ec.id].isar > 0) {fprintf(stderr, MSG_ERR_WRONG_USE, line_num+1, rem_fname(v_name[ec.id], fname)); exit(EXIT_FAILURE);}

    // ------------------------------------------------------------------------
    // update variable status -------------------------------------------------
    // ------------------------------------------------------------------------

    if (ec.id != 0) v_table[ec.id].used = 1;

    // ------------------------------------------------------------------------
    // prepare local variables ------------------------------------------------
    // ------------------------------------------------------------------------

    int N = v_table[idv].size;

    char g[64]; if (ec.id==0) strcpy(g,"aux_var"); else strcpy(g,v_name[ec.id]);

    // ------------------------------------------------------------------------
    // execute ----------------------------------------------------------------
    // ------------------------------------------------------------------------

    printf(MSG_INFO_DIRAC_SET_VECTOR, line_num+1);

    if (ec.id==0) add_instr("SET aux_var\n");

    for (int i = 0; i < N; i++)
    {
        // int
        if (v_table[idv].type == 1)
        {
            add_instr("INN %s\n", v_name[idp]);
            add_instr("MLT %s\n",g);
        }

        // float
        if (v_table[idv].type == 2)
        {
            add_instr("F_INN %s\n", v_name[idp]);
            add_instr("F_MLT %s\n",g);
        }

        add_instr("SET_V %s %d\n", v_name[idv], i);
    }

    acc_ok = 0;
}

// writes vector to output with weight c, e.g. out(0, c|a>);
void exec_vout(int idp, expr ec, int idv)
{
    // ------------------------------------------------------------------------
    // consistency check ------------------------------------------------------
    // ------------------------------------------------------------------------

    // check whether idv was declared
    if (v_table[idv].type == 0) {fprintf(stderr, MSG_ERR_DECL_FIRST, line_num+1, rem_fname(v_name[idv], fname)); exit(EXIT_FAILURE);}

    // check whether etc was declared
    if (ec.id != 0 && v_table[ec.id].type == 0) {fprintf(stderr, MSG_ERR_DECL_FIRST, line_num+1, rem_fname(v_name[ec.id], fname)); exit(EXIT_FAILURE);}

    // check that it is not comp
    if (v_table[idv].type == 3 || ec.type == 3) {fprintf(stderr, MSG_ERR_NOT_IMPL_COMPLEX, line_num+1); exit(EXIT_FAILURE);}

    // check whether idv is a vector
    if (v_table[idv].isar != 1) {fprintf(stderr, MSG_ERR_NOT_A_VECTOR, line_num+1, rem_fname(v_name[idv], fname)); exit(EXIT_FAILURE);}

    // check whether etc is a variable
    if (ec.id != 0 && v_table[ec.id].isar > 0) {fprintf(stderr, MSG_ERR_WRONG_USE, line_num+1, rem_fname(v_name[ec.id], fname)); exit(EXIT_FAILURE);}

    // ------------------------------------------------------------------------
    // update variable status -------------------------------------------------
    // ------------------------------------------------------------------------

    if (ec.id != 0) v_table[ec.id].used = 1;
    v_table[idv].used = 1;
    
    // ------------------------------------------------------------------------
    // prepare local variables ------------------------------------------------
    // ------------------------------------------------------------------------

    int N = v_table[idv].size;

    char g[64]; if (ec.id==0) strcpy(g,"aux_var"); else strcpy(g,v_name[ec.id]);

    // ------------------------------------------------------------------------
    // execute ----------------------------------------------------------------
    // ------------------------------------------------------------------------

    printf(MSG_INFO_DIRAC_FLUSH_VECTOR, line_num+1);

    if (ec.id==0) add_instr("SET aux_var\n");

    for (int i = 0; i < N; i++)
    {
        add_instr("LOD_V %s %d\n", v_name[idv], i);

        // int
        if (v_table[idv].type == 1)
        {
            add_instr("MLT %s\n",g);
        }

        // float
        if (v_table[idv].type == 2)
        {
            add_instr("F_MLT %s\n", g);
            add_instr("F2I\n");
        }

        add_instr("OUT %s\n", v_name[idp]);
    }

    acc_ok = 0;
}

// performs a shift register on the vector with the value given on the left, e.g. a # b -> |c>;
// a and c must be the same vector
// create a new array type for shift register?
void exec_shift(int ida, expr eb, int idc)
{
    // ------------------------------------------------------------------------
    // consistency check ------------------------------------------------------
    // ------------------------------------------------------------------------

    // check whether ida was declared
    if (v_table[ida].type == 0) {fprintf(stderr, MSG_ERR_DECL_FIRST, line_num+1, rem_fname(v_name[ida], fname)); exit(EXIT_FAILURE);}

    // check whether etb was declared
    if (eb.id != 0 && v_table[eb.id].type == 0) {fprintf(stderr, MSG_ERR_DECL_FIRST, line_num+1, rem_fname(v_name[eb.id], fname)); exit(EXIT_FAILURE);}

    // check whether idc equals ida
    if (idc != ida) {fprintf(stderr, MSG_ERR_SHIFT_VEC_SELF, line_num+1); exit(EXIT_FAILURE);}
    
    // check that it is not comp
    if (v_table[ida].type == 3 || eb.type == 3) {fprintf(stderr, MSG_ERR_NOT_IMPL_COMPLEX, line_num+1); exit(EXIT_FAILURE);}

    // check that the types match
    //if (v_table[ida].type != v_table[eb.id].type) {fprintf(stderr, MSG_ERR_VARS_SAME_TYPE, line_num+1); exit(EXIT_FAILURE);}
    
    // check whether ida is a vector
    if (v_table[ida].isar != 1) {fprintf(stderr, MSG_ERR_NOT_A_VECTOR, line_num+1, rem_fname(v_name[ida], fname)); exit(EXIT_FAILURE);}

    // check whether etb is a variable
    if (eb.id != 0 && v_table[eb.id].isar > 0) {fprintf(stderr, MSG_ERR_WRONG_USE, line_num+1, rem_fname(v_name[eb.id], fname)); exit(EXIT_FAILURE);}

    // ------------------------------------------------------------------------
    // update variable status -------------------------------------------------
    // ------------------------------------------------------------------------

    if (eb.id != 0) v_table[eb.id].used = 1;
    
    // ------------------------------------------------------------------------
    // prepare local variables ------------------------------------------------
    // ------------------------------------------------------------------------

    int N = v_table[ida].size;

    // ------------------------------------------------------------------------
    // execute ----------------------------------------------------------------
    // ------------------------------------------------------------------------

    printf(MSG_INFO_DIRAC_SHIFT, v_name[ida], line_num+1);

    // ida int and etb int in memory
    if (v_table[ida].type == 1 && eb.type == 1 && eb.id != 0)
    {
        for (int i = N-1; i > 0; i--)
        {
            add_instr("LOD_V %s %d\n", v_name[ida], i-1);
            add_instr("SET_V %s %d\n", v_name[ida], i);
        }

        add_instr("LOD %s\n", v_name[eb.id]);
        add_instr("SET %s\n", v_name[ida]);
    }

    // ida float e etb int no acc
    if (v_table[ida].type == 2 && eb.type == 1 && eb.id == 0)
    {
        add_instr("SET aux_var\n");

        for (int i = N-1; i > 0; i--)
        {
            add_instr("LOD_V %s %d\n", v_name[ida], i-1);
            add_instr("SET_V %s %d\n", v_name[ida], i);
        }

        add_instr("I2F_M aux_var\n");
        add_instr("SET %s\n", v_name[ida]);
    }

    // ida float and etb float in memory
    if (v_table[ida].type == 2 && eb.type == 2 && eb.id != 0)
    {
        for (int i = N-1; i > 0; i--)
        {
            add_instr("LOD_V %s %d\n", v_name[ida], i-1);
            add_instr("SET_V %s %d\n", v_name[ida], i);
        }

        add_instr("LOD %s\n", v_name[eb.id]);
        add_instr("SET %s\n", v_name[ida]);
    }

    // ida float e etb float no acc
    if (v_table[ida].type == 2 && eb.type == 2 && eb.id == 0)
    {
        add_instr("SET aux_var\n");

        for (int i = N-1; i > 0; i--)
        {
            add_instr("LOD_V %s %d\n", v_name[ida], i-1);
            add_instr("SET_V %s %d\n", v_name[ida], i);
        }

        add_instr("LOD aux_var\n");
        add_instr("SET %s\n", v_name[ida]);
    }

    acc_ok = 0;
}
