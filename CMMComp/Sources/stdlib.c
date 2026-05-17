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
int exec_in(int id)
{
    if (atoi(v_name[id]) >= nuioin) {fprintf(stderr, MSG_ERR_NO_IN_PORT, line_num+1, v_name[id]); exit(EXIT_FAILURE);}

    if (acc_ok == 0) add_instr("INN %s\n", v_name[id]); else add_instr("P_INN %s\n", v_name[id]);

    acc_ok = 1;  // marks the acc as now holding a value

    return OFST;
}

// input ex: float x = fin(0);
int exec_fin(int id)
{
    if (atoi(v_name[id]) >= nuioin) {fprintf(stderr, MSG_ERR_NO_IN_PORT, line_num+1, v_name[id]); exit(EXIT_FAILURE);}

    if (acc_ok == 0) add_instr("F_INN %s\n", v_name[id]); else add_instr("PF_INN %s\n", v_name[id]);

    acc_ok = 1;  // marks the acc as now holding a value

    return 2*OFST;
}

// output ex: out(0,x);
void exec_out(int id, int et)
{
    // ------------------------------------------------------------------------
    // consistency check ------------------------------------------------------
    // ------------------------------------------------------------------------

    // check whether et was declared
    if (et%OFST != 0 && v_type[et%OFST] == 0) {fprintf(stderr, MSG_ERR_DECL_FIRST, line_num+1, rem_fname(v_name[et%OFST], fname)); exit(EXIT_FAILURE);}

    // check whether it is comp
    if (get_type(et) > 2) {fprintf (stderr, MSG_ERR_PICK_COMP_INFO, line_num+1); exit(EXIT_FAILURE);}

    // check whether et is a variable
    if (et%OFST != 0 && v_isar[et%OFST] > 0) {fprintf(stderr, MSG_ERR_WRONG_USE, line_num+1, rem_fname(v_name[et%OFST], fname)); exit(EXIT_FAILURE);}

    // check port range
    if (atoi(v_name[id]) >= nuioou) {fprintf(stderr, MSG_ERR_NO_OUT_PORT, line_num+1, v_name[id]); exit(EXIT_FAILURE);}

    // ------------------------------------------------------------------------
    // update variable status -------------------------------------------------
    // ------------------------------------------------------------------------

    if (et%OFST != 0) v_used[et%OFST] = 1;

    // ------------------------------------------------------------------------
    // execute ----------------------------------------------------------------
    // ------------------------------------------------------------------------

    // int var
    if ((get_type(et) == 1) && (et%OFST!=0))
    {
       if (acc_ok == 0) add_instr("LOD %s\n", v_name[et%OFST]); else add_instr("P_LOD %s\n", v_name[et%OFST]);
    }

    // int acc
    if ((get_type(et) == 1) && (et%OFST==0))
    {
        // nothing to do
    }

    // float var
    if ((get_type(et) == 2) && (et%OFST!=0))
    {
        fprintf(stdout, MSG_WARN_USE_FOUT, line_num+1);

        if (acc_ok == 0) add_instr("F2I_M %s\n", v_name[et%OFST]); else add_instr("P_F2I_M %s\n", v_name[et%OFST]);
    }

    // float acc
    if ((get_type(et) == 2) && (et%OFST==0))
    {
        fprintf(stdout, MSG_WARN_USE_FOUT, line_num+1);

        add_instr("F2I\n");
    }

    add_instr("OUT %s\n", v_name[id]);

    acc_ok = 0; // libera acc    
}

// output ex: fout(0,x);
void exec_fout(int id, int et)
{
    // ------------------------------------------------------------------------
    // consistency check ------------------------------------------------------
    // ------------------------------------------------------------------------

    // check whether et was declared
    if (et%OFST != 0 && v_type[et%OFST] == 0) {fprintf(stderr, MSG_ERR_DECL_FIRST, line_num+1, rem_fname(v_name[et%OFST], fname)); exit(EXIT_FAILURE);}

    // check whether it is comp
    if (get_type(et) > 2) {fprintf (stderr, MSG_ERR_PICK_COMP_INFO, line_num+1); exit(EXIT_FAILURE);}

    // check whether et is a variable
    if (et%OFST != 0 && v_isar[et%OFST] > 0) {fprintf(stderr, MSG_ERR_WRONG_USE, line_num+1, rem_fname(v_name[et%OFST], fname)); exit(EXIT_FAILURE);}

    // check port range
    if (atoi(v_name[id]) >= nuioou) {fprintf(stderr, MSG_ERR_NO_OUT_PORT, line_num+1, v_name[id]); exit(EXIT_FAILURE);}

    // ------------------------------------------------------------------------
    // update variable status -------------------------------------------------
    // ------------------------------------------------------------------------

    if (et%OFST != 0) v_used[et%OFST] = 1;

    // ------------------------------------------------------------------------
    // execute ----------------------------------------------------------------
    // ------------------------------------------------------------------------

    // int var
    if ((get_type(et) == 1) && (et%OFST!=0))
    {
        fprintf(stdout, MSG_WARN_USE_OUT, line_num+1);

        if (acc_ok == 0) add_instr("LOD %s\n", v_name[et%OFST]); else add_instr("P_LOD %s\n", v_name[et%OFST]);
    }

    // int acc
    if ((get_type(et) == 1) && (et%OFST==0))
    {
        fprintf(stdout, MSG_WARN_USE_OUT, line_num+1);
    }

    // float var
    if ((get_type(et) == 2) && (et%OFST!=0))
    {
        if (acc_ok == 0) add_instr("F2I_M %s\n", v_name[et%OFST]); else add_instr("P_F2I_M %s\n", v_name[et%OFST]);
    }

    // float acc
    if ((get_type(et) == 2) && (et%OFST==0))
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
int exec_sign(int et1, int et2)
{
    // ------------------------------------------------------------------------
    // consistency check ------------------------------------------------------
    // ------------------------------------------------------------------------

    // check whether et1 was declared
    if (et1%OFST != 0 && v_type[et1%OFST] == 0) {fprintf(stderr, MSG_ERR_DECL_FIRST, line_num+1, rem_fname(v_name[et1%OFST], fname)); exit(EXIT_FAILURE);}
    
    // check whether et2 was declared
    if (et2%OFST != 0 && v_type[et2%OFST] == 0) {fprintf(stderr, MSG_ERR_DECL_FIRST, line_num+1, rem_fname(v_name[et2%OFST], fname)); exit(EXIT_FAILURE);}

    // check whether et1 is a variable
    if (et1%OFST != 0 && v_isar[et1%OFST] > 0) {fprintf(stderr, MSG_ERR_WRONG_USE, line_num+1, rem_fname(v_name[et1%OFST], fname)); exit(EXIT_FAILURE);}

    // check whether et2 is a variable
    if (et2%OFST != 0 && v_isar[et2%OFST] > 0) {fprintf(stderr, MSG_ERR_WRONG_USE, line_num+1, rem_fname(v_name[et2%OFST], fname)); exit(EXIT_FAILURE);}

    // check whether there is a comp
    if ((get_type(et1) > 2) || (get_type(et2) > 2)) {fprintf (stderr, MSG_ERR_SIGN_COMPLEX, line_num+1); exit(EXIT_FAILURE);}

    // ------------------------------------------------------------------------
    // update variable status -------------------------------------------------
    // ------------------------------------------------------------------------

    if (et1%OFST != 0) v_used[et1%OFST] = 1;
    if (et2%OFST != 0) v_used[et2%OFST] = 1;

    // ------------------------------------------------------------------------
    // prepare local variables ------------------------------------------------
    // ------------------------------------------------------------------------

    char ld[10]; if (acc_ok == 0) strcpy(ld,"LOD"); else strcpy(ld,"P_LOD");

    // ------------------------------------------------------------------------
    // execute ----------------------------------------------------------------
    // ------------------------------------------------------------------------

    // int in memory and int in memory
    if ((get_type(et1) == 1) && (et1 % OFST != 0) && (get_type(et2) == 1) && (et2 % OFST != 0))
    {
        add_instr("%s %s\n", ld, v_name[et2%OFST]);
        add_instr("SGN %s\n"   , v_name[et1%OFST]);
    }

    // int in memory and int in acc
    if ((get_type(et1) == 1) && (et1 % OFST != 0) && (get_type(et2) == 1) && (et2 % OFST == 0))
    {
        add_instr("SGN %s\n", v_name[et1%OFST]);
    }

    // int in memory and float var in memory
    if ((get_type(et1) == 1) && (et1 % OFST != 0) && (get_type(et2) == 2) && (et2 % OFST != 0))
    {
        add_instr("%s %s\n", ld, v_name[et2%OFST]);
        add_instr("F_SGN %s\n" , v_name[et1%OFST]);
    }

    // int in memory and float in acc
    if ((get_type(et1) == 1) && (et1 % OFST != 0) && (get_type(et2) == 2) && (et2 % OFST == 0))
    {
        add_instr("F_SGN %s\n", v_name[et1%OFST]);
    }

    // int in acc and int in memory
    if ((get_type(et1) == 1) && (et1 % OFST == 0) && (get_type(et2) == 1) && (et2 % OFST != 0))
    {
        add_instr("%s %s\n", ld, v_name[et2%OFST]);
        add_instr("S_SGN\n");
    }

    // int in acc and int in acc
    if ((get_type(et1) == 1) && (et1 % OFST == 0) && (get_type(et2) == 1) && (et2 % OFST == 0))
    {
        add_instr("S_SGN\n");
    }

    // int in acc and float var in memory
    if ((get_type(et1) == 1) && (et1 % OFST == 0) && (get_type(et2) == 2) && (et2 % OFST != 0))
    {
        add_instr("P_LOD %s\n", v_name[et2%OFST]  );
        add_instr("SF_SGN\n");
    }

    // int in acc and float in acc
    if ((get_type(et1) == 1) && (et1 % OFST == 0) && (get_type(et2) == 2) && (et2 % OFST == 0))
    {
        add_instr("SF_SGN\n");
    }

    // float var and int var
    if ((get_type(et1) == 2) && (et1 % OFST != 0) && (get_type(et2) == 1) && (et2 % OFST != 0))
    {
        add_instr("%s %s\n", ld, v_name[et2%OFST]);
        add_instr("SGN %s\n"   , v_name[et1%OFST]);
    }

    // float var and int in acc
    if ((get_type(et1) == 2) && (et1 % OFST != 0) && (get_type(et2) == 1) && (et2 % OFST == 0))
    {
        add_instr("SGN %s\n"  , v_name[et1%OFST]);
    }

    // float var and float var
    if ((get_type(et1) == 2) && (et1 % OFST != 0) && (get_type(et2) == 2) && (et2 % OFST != 0))
    {
        add_instr("%s %s\n", ld, v_name[et2%OFST]);
        add_instr("F_SGN %s\n" , v_name[et1%OFST]);
    }

    // float var and float in acc
    if ((get_type(et1) == 2) && (et1 % OFST != 0) && (get_type(et2) == 2) && (et2 % OFST == 0))
    {
        add_instr("F_SGN %s\n" , v_name[et1%OFST]);
    }

    // float in acc and int in memory
    if ((get_type(et1) == 2) && (et1 % OFST == 0) && (get_type(et2) == 1) && (et2 % OFST != 0))
    {
        add_instr("%s %s\n", ld, v_name[et2%OFST]);
        add_instr("S_SGN\n");
    }

    // float in acc and int in acc
    if ((get_type(et1) == 2) && (et1 % OFST == 0) && (get_type(et2) == 1) && (et2 % OFST == 0))
    {
        add_instr("S_SGN\n");
    }

    // float in acc and float var in memory
    if ((get_type(et1) == 2) && (et1 % OFST == 0) && (get_type(et2) == 2) && (et2 % OFST != 0))
    {
        add_instr("P_LOD %s\n", v_name[et2%OFST]);
        add_instr("SF_SGN\n");
    }

    // float in acc and float in acc
    if ((get_type(et1) == 2) && (et1 % OFST == 0) && (get_type(et2) == 2) && (et2 % OFST == 0))
    {
        add_instr("SF_SGN\n");
    }

    acc_ok = 1;

    return get_type(et2)*OFST;
}

// absolute value (int, float and comp)
int exec_abs(int et)
{
    // ------------------------------------------------------------------------
    // consistency check ------------------------------------------------------
    // ------------------------------------------------------------------------

    // check whether et was declared
    if (et%OFST != 0 && v_type[et%OFST] == 0) {fprintf(stderr, MSG_ERR_DECL_FIRST, line_num+1, rem_fname(v_name[et%OFST], fname)); exit(EXIT_FAILURE);}

    // check whether et is a variable
    if (et%OFST != 0 && v_isar[et%OFST] > 0) {fprintf(stderr, MSG_ERR_WRONG_USE, line_num+1, rem_fname(v_name[et%OFST], fname)); exit(EXIT_FAILURE);}

    // ------------------------------------------------------------------------
    // update variable status -------------------------------------------------
    // ------------------------------------------------------------------------

    if (et%OFST != 0) v_used[et%OFST] = 1;

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
    if ((get_type(et) == 1) && (et % OFST != 0))
    {
        add_instr("%s %s\n", abs, v_name[et%OFST]);
    }

    // int in acc
    if ((get_type(et) == 1) && (et % OFST == 0))
    {
        add_instr("ABS\n");
    }

    // float in memory
    if ((get_type(et) == 2) && (et % OFST != 0))
    {
        add_instr("%s %s\n", fabs, v_name[et%OFST]);
    }

    // float in acc
    if ((get_type(et) == 2) && (et % OFST == 0))
    {
        add_instr("F_ABS\n");
    }

    // comp const, in memory and in acc
    if ((get_type(et) == 3) || (get_type(et) == 5))
    {
        et = exec_sqrt(exec_mod2(et));
    }

    acc_ok = 1;

    return get_type(et)*OFST;
}

// clears if negative
int exec_pst(int et)
{
    // ------------------------------------------------------------------------
    // consistency check ------------------------------------------------------
    // ------------------------------------------------------------------------

    // check whether et was declared
    if (et%OFST != 0 && v_type[et%OFST] == 0) {fprintf(stderr, MSG_ERR_DECL_FIRST, line_num+1, rem_fname(v_name[et%OFST], fname)); exit(EXIT_FAILURE);}

    // check whether et is a variable
    if (et%OFST != 0 && v_isar[et%OFST] > 0) {fprintf(stderr, MSG_ERR_WRONG_USE, line_num+1, rem_fname(v_name[et%OFST], fname)); exit(EXIT_FAILURE);}

    // ------------------------------------------------------------------------
    // update variable status -------------------------------------------------
    // ------------------------------------------------------------------------

    if (et%OFST != 0) v_used[et%OFST] = 1;

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
    if ((get_type(et) == 1) && (et % OFST != 0))
    {
        add_instr("%s %s\n", pst, v_name[et%OFST]);
    }

    // int in acc
    if ((get_type(et) == 1) && (et % OFST == 0))
    {
        add_instr("PST\n");
    }

    // float in memory
    if ((get_type(et) == 2)  && (et % OFST != 0))
    {
        add_instr("%s %s\n", fpst, v_name[et%OFST]);
    }

    // float in acc
    if ((get_type(et) == 2)  && (et % OFST == 0))
    {
        add_instr("F_PST\n");
    }

    // comp
    if (get_type(et) > 2)
    {
        fprintf (stderr, MSG_ERR_PSET_COMPLEX, line_num+1);
        exit(EXIT_FAILURE);
    }

    acc_ok = 1;

    return get_type(et)*OFST;
}

// division by constant
int exec_norm(int et)
{
    // ------------------------------------------------------------------------
    // consistency check ------------------------------------------------------
    // ------------------------------------------------------------------------

    // check whether et was declared
    if (et%OFST != 0 && v_type[et%OFST] == 0) {fprintf(stderr, MSG_ERR_DECL_FIRST, line_num+1, rem_fname(v_name[et%OFST], fname)); exit(EXIT_FAILURE);}

    // check whether et is a variable
    if (et%OFST != 0 && v_isar[et%OFST] > 0) {fprintf(stderr, MSG_ERR_WRONG_USE, line_num+1, rem_fname(v_name[et%OFST], fname)); exit(EXIT_FAILURE);}

    // check whether it is int
    if (get_type(et) != 1) {fprintf (stderr, MSG_ERR_NORM_NON_INT, line_num+1); exit(EXIT_FAILURE);}

    // ------------------------------------------------------------------------
    // update variable status -------------------------------------------------
    // ------------------------------------------------------------------------

    if (et%OFST != 0) v_used[et%OFST] = 1;

    // ------------------------------------------------------------------------
    // prepare local variables ------------------------------------------------
    // ------------------------------------------------------------------------

    char nrm[10]; if (acc_ok == 0) strcpy(nrm,"NRM_M"); else strcpy(nrm,"P_NRM_M");

    // ------------------------------------------------------------------------
    // execute ----------------------------------------------------------------
    // ------------------------------------------------------------------------

    // int in memory
    if ((get_type(et) == 1) && (et % OFST != 0))
    {
        add_instr("%s %s\n", nrm, v_name[et%OFST]);
    }

    // int in acc
    if ((get_type(et) == 1) && (et % OFST == 0))
    {
        add_instr("NRM\n");
    }

    acc_ok = 1;

    return OFST;
}

void exec_copy(int et1, int id2)
{
    // ------------------------------------------------------------------------
    // consistency check ------------------------------------------------------
    // ------------------------------------------------------------------------

    // check whether et1 was declared
    if (et1%OFST != 0 && v_type[et1%OFST] == 0) {fprintf(stderr, MSG_ERR_DECL_FIRST, line_num+1, rem_fname(v_name[et1%OFST], fname)); exit(EXIT_FAILURE);}

    // check whether et1 is a variable
    if (et1%OFST != 0 && v_isar[et1%OFST] > 0) {fprintf(stderr, MSG_ERR_WRONG_USE, line_num+1, rem_fname(v_name[et1%OFST], fname)); exit(EXIT_FAILURE);}

    // check whether id2 was declared
    if (v_type[id2] == 0) {fprintf(stderr, MSG_ERR_DECL_FIRST, line_num+1, rem_fname(v_name[id2], fname)); exit(EXIT_FAILURE);}

    // check whether id2 is a variable
    if (v_isar[id2] > 0) {fprintf(stderr, MSG_ERR_WRONG_USE, line_num+1, rem_fname(v_name[id2], fname)); exit(EXIT_FAILURE);}

    // ------------------------------------------------------------------------
    // update variable status -------------------------------------------------
    // ------------------------------------------------------------------------

    if (et1%OFST != 0) v_used[et1%OFST] = 1;

    // ------------------------------------------------------------------------
    // execute ----------------------------------------------------------------
    // ------------------------------------------------------------------------

    // from var to var
    if (et1%OFST != 0)
    {
        add_instr("LOD %s\n", v_name[et1%OFST]);
        add_instr("SET %s\n", v_name[id2]     );
    }

    // from acc to var
    if (et1%OFST == 0)
    {
        add_instr("SET %s\n", v_name[id2]     );
    }
}

// ----------------------------------------------------------------------------
// non-linear functions -------------------------------------------------------
// ----------------------------------------------------------------------------

// square root
int exec_sqrt(int et)
{
    // ------------------------------------------------------------------------
    // consistency check ------------------------------------------------------
    // ------------------------------------------------------------------------

    // check whether et was declared
    if (et%OFST != 0 && v_type[et%OFST] == 0) {fprintf(stderr, MSG_ERR_DECL_FIRST, line_num+1, rem_fname(v_name[et%OFST], fname)); exit(EXIT_FAILURE);}

    // check whether et is a variable
    if (et%OFST != 0 && v_isar[et%OFST] > 0) {fprintf(stderr, MSG_ERR_WRONG_USE, line_num+1, rem_fname(v_name[et%OFST], fname)); exit(EXIT_FAILURE);}

    // check whether it is comp
    if (get_type(et) > 2) {fprintf (stderr, MSG_ERR_SQRT_COMPLEX, line_num+1); exit(EXIT_FAILURE);}

    // ------------------------------------------------------------------------
    // update variable status -------------------------------------------------
    // ------------------------------------------------------------------------

    if (et%OFST != 0) v_used[et%OFST] = 1;

    // ------------------------------------------------------------------------
    // prepare local variables ------------------------------------------------
    // ------------------------------------------------------------------------

    char ld [10]; if (acc_ok == 0) strcpy(ld ,"LOD"   ); else strcpy(ld ,"P_LOD"  );
    char i2f[10]; if (acc_ok == 0) strcpy(i2f, "I2F_M"); else strcpy(i2f,"P_I2F_M");

    // ------------------------------------------------------------------------
    // execute ----------------------------------------------------------------
    // ------------------------------------------------------------------------

    // int in memory
    if ((get_type(et) == 1) && (et % OFST != 0))
    {
        add_instr("%s %s\n", i2f, v_name[et%OFST]);
        add_instr("CAL float_sqrt\n");
    }

    // int in acc
    if ((get_type(et) == 1) && (et % OFST == 0))
    {
        add_instr("I2F\n");
        add_instr("CAL float_sqrt\n");
    }

    // float in memory
    if ((get_type(et) == 2) && (et % OFST != 0))
    {
        add_instr("%s %s\n", ld, v_name[et%OFST]);
        add_instr("CAL float_sqrt\n");
    }

    // float in acc
    if ((get_type(et) == 2) && (et % OFST == 0))
    {
        add_instr("CAL float_sqrt\n");
    }

    acc_ok = 1;

    return 2*OFST;
}

// arctangent
int exec_atan(int et)
{
    // ------------------------------------------------------------------------
    // consistency check ------------------------------------------------------
    // ------------------------------------------------------------------------

    // check whether et was declared
    if (et%OFST != 0 && v_type[et%OFST] == 0) {fprintf(stderr, MSG_ERR_DECL_FIRST, line_num+1, rem_fname(v_name[et%OFST], fname)); exit(EXIT_FAILURE);}

    // check whether et is a variable
    if (et%OFST != 0 && v_isar[et%OFST] > 0) {fprintf(stderr, MSG_ERR_WRONG_USE, line_num+1, rem_fname(v_name[et%OFST], fname)); exit(EXIT_FAILURE);}

    // check whether it is comp
    if (get_type(et) > 2) {fprintf (stderr, MSG_ERR_SQRT_COMPLEX, line_num+1); exit(EXIT_FAILURE);}

    // ------------------------------------------------------------------------
    // update variable status -------------------------------------------------
    // ------------------------------------------------------------------------

    if (et%OFST != 0) v_used[et%OFST] = 1;

    // ------------------------------------------------------------------------
    // prepare local variables ------------------------------------------------
    // ------------------------------------------------------------------------

    char ld [10]; if (acc_ok == 0) strcpy(ld ,"LOD"  ); else strcpy(ld ,"P_LOD"  );
    char i2f[10]; if (acc_ok == 0) strcpy(i2f,"I2F_M"); else strcpy(i2f,"P_I2F_M");

    // ------------------------------------------------------------------------
    // execute ----------------------------------------------------------------
    // ------------------------------------------------------------------------

    // int in memory
    if ((get_type(et) == 1) && (et % OFST != 0))
    {
        add_instr("%s %s\n", i2f, v_name[et%OFST]);
        add_instr("CAL float_atan\n");
    }

    // int in acc
    if ((get_type(et) == 1) && (et % OFST == 0))
    {
        add_instr("I2F\n");
        add_instr("CAL float_atan\n");
    }

    // float in memory
    if ((get_type(et) == 2) && (et % OFST != 0))
    {
        add_instr("%s %s\n", ld, v_name[et%OFST]);
        add_instr("CAL float_atan\n");
    }

    // float in acc
    if ((get_type(et) == 2) && (et % OFST == 0))
    {
        add_instr("CAL float_atan\n");
    }

    acc_ok = 1;

    return 2*OFST;
}

// sine
int exec_sin(int et)
{
    // ------------------------------------------------------------------------
    // consistency check ------------------------------------------------------
    // ------------------------------------------------------------------------

    // check whether et was declared
    if (et%OFST != 0 && v_type[et%OFST] == 0) {fprintf(stderr, MSG_ERR_DECL_FIRST, line_num+1, rem_fname(v_name[et%OFST], fname)); exit(EXIT_FAILURE);}

    // check whether et is a variable
    if (et%OFST != 0 && v_isar[et%OFST] > 0) {fprintf(stderr, MSG_ERR_WRONG_USE, line_num+1, rem_fname(v_name[et%OFST], fname)); exit(EXIT_FAILURE);}

    // check whether it is comp
    if (get_type(et) > 2) {fprintf (stderr, MSG_ERR_SQRT_COMPLEX, line_num+1); exit(EXIT_FAILURE);}

    // ------------------------------------------------------------------------
    // update variable status -------------------------------------------------
    // ------------------------------------------------------------------------

    if (et%OFST != 0) v_used[et%OFST] = 1;

    // ------------------------------------------------------------------------
    // prepare local variables ------------------------------------------------
    // ------------------------------------------------------------------------

    char ld [10]; if (acc_ok == 0) strcpy(ld ,"LOD"  ); else strcpy(ld ,"P_LOD"  );
    char i2f[10]; if (acc_ok == 0) strcpy(i2f,"I2F_M"); else strcpy(i2f,"P_I2F_M");

    // ------------------------------------------------------------------------
    // execute ----------------------------------------------------------------
    // ------------------------------------------------------------------------

    // int in memory
    if ((get_type(et) == 1) && (et % OFST != 0))
    {
        add_instr("%s %s\n", i2f, v_name[et%OFST]);
        add_instr("CAL float_sin\n");
    }

    // int in acc
    if ((get_type(et) == 1) && (et % OFST == 0))
    {
        add_instr("I2F\n");
        add_instr("CAL float_sin\n");
    }

    // float in memory
    if ((get_type(et) == 2) && (et % OFST != 0))
    {
        add_instr("%s %s\n", ld, v_name[et%OFST]);
        add_instr("CAL float_sin\n");
    }

    // float in acc
    if ((get_type(et) == 2) && (et % OFST == 0))
    {
        add_instr("CAL float_sin\n");
    }

    acc_ok = 1;

    return 2*OFST;
}

// cosine
int exec_cos(int et)
{
    // ------------------------------------------------------------------------
    // consistency check ------------------------------------------------------
    // ------------------------------------------------------------------------

    // check whether et was declared
    if (et%OFST != 0 && v_type[et%OFST] == 0) {fprintf(stderr, MSG_ERR_DECL_FIRST, line_num+1, rem_fname(v_name[et%OFST], fname)); exit(EXIT_FAILURE);}

    // check whether et is a variable
    if (et%OFST != 0 && v_isar[et%OFST] > 0) {fprintf(stderr, MSG_ERR_WRONG_USE, line_num+1, rem_fname(v_name[et%OFST], fname)); exit(EXIT_FAILURE);}

    // check whether it is comp
    if (get_type(et) > 2) {fprintf (stderr, MSG_ERR_SQRT_COMPLEX, line_num+1); exit(EXIT_FAILURE);}

    // ------------------------------------------------------------------------
    // update variable status -------------------------------------------------
    // ------------------------------------------------------------------------

    if (et%OFST != 0) v_used[et%OFST] = 1;

    // ------------------------------------------------------------------------
    // prepare local variables ------------------------------------------------
    // ------------------------------------------------------------------------

    char ld [10]; if (acc_ok == 0) strcpy(ld ,"LOD"  ); else strcpy(ld ,"P_LOD"  );
    char i2f[10]; if (acc_ok == 0) strcpy(i2f,"I2F_M"); else strcpy(i2f,"P_I2F_M");

    // ------------------------------------------------------------------------
    // execute ----------------------------------------------------------------
    // ------------------------------------------------------------------------

    // int in memory
    if ((get_type(et) == 1) && (et % OFST != 0))
    {
        add_instr("%s %s\n", i2f, v_name[et%OFST]);
        add_instr("F_NEG\n");
        add_instr("F_ADD 1.570796327");
        add_instr("CAL float_sin\n");
    }

    // int in acc
    if ((get_type(et) == 1) && (et % OFST == 0))
    {
        add_instr("I2F\n");
        add_instr("F_NEG\n");
        add_instr("F_ADD 1.570796327");
        add_instr("CAL float_sin\n");
    }

    // float in memory
    if ((get_type(et) == 2) && (et % OFST != 0))
    {
        add_instr("%s %s\n", ld, v_name[et%OFST]);
        add_instr("F_NEG\n");
        add_instr("F_ADD 1.570796327");
        add_instr("CAL float_sin\n");
    }

    // float in acc
    if ((get_type(et) == 2) && (et % OFST == 0))
    {
        add_instr("F_NEG\n");
        add_instr("F_ADD 1.570796327");
        add_instr("CAL float_sin\n");
    }

    acc_ok = 1;

    return 2*OFST;
}

// ----------------------------------------------------------------------------
// special functions for complex numbers --------------------------------------
// ----------------------------------------------------------------------------

// returns the real part of a comp
int exec_real(int et)
{
    // ------------------------------------------------------------------------
    // consistency check ------------------------------------------------------
    // ------------------------------------------------------------------------

    // check whether et was declared
    if (et%OFST != 0 && v_type[et%OFST] == 0) {fprintf(stderr, MSG_ERR_DECL_FIRST, line_num+1, rem_fname(v_name[et%OFST], fname)); exit(EXIT_FAILURE);}

    // check whether et is a variable
    if (et%OFST != 0 && v_isar[et%OFST] > 0) {fprintf(stderr, MSG_ERR_WRONG_USE, line_num+1, rem_fname(v_name[et%OFST], fname)); exit(EXIT_FAILURE);}

    // check whether it is comp
    if (get_type(et) < 3) {fprintf (stderr, MSG_ERR_REAL_ARG_COMP, line_num+1); exit(EXIT_FAILURE);}

    // ------------------------------------------------------------------------
    // update variable status -------------------------------------------------
    // ------------------------------------------------------------------------

    if (et%OFST != 0) v_used[et%OFST] = 1;

    // ------------------------------------------------------------------------
    // prepare local variables ------------------------------------------------
    // ------------------------------------------------------------------------

    char ld[10]; if (acc_ok == 0) strcpy(ld,"LOD"); else strcpy(ld,"P_LOD");

    // ------------------------------------------------------------------------
    // execute ----------------------------------------------------------------
    // ------------------------------------------------------------------------

    // comp const
    if (get_type(et) == 5)
    {
        int et_r, et_i;
        get_cmp_cst(et,&et_r,&et_i);

        add_instr("%s %s\n", ld, v_name[et_r%OFST]);
    }

    // comp in memory
    if ((get_type(et) == 3) && (et % OFST != 0))
    {
        add_instr("%s %s\n", ld, v_name[et % OFST]);
    }

    // comp in acc
    if ((get_type(et) == 3) && (et % OFST == 0))
    {
        add_instr("POP\n");
    }

    acc_ok = 1;
    
    return 2*OFST;
}

// returns the imag part of a comp
int exec_imag(int et)
{
    // ------------------------------------------------------------------------
    // consistency check ------------------------------------------------------
    // ------------------------------------------------------------------------

    // check whether et was declared
    if (et%OFST != 0 && v_type[et%OFST] == 0) {fprintf(stderr, MSG_ERR_DECL_FIRST, line_num+1, rem_fname(v_name[et%OFST], fname)); exit(EXIT_FAILURE);}

    // check whether et is a variable
    if (et%OFST != 0 && v_isar[et%OFST] > 0) {fprintf(stderr, MSG_ERR_WRONG_USE, line_num+1, rem_fname(v_name[et%OFST], fname)); exit(EXIT_FAILURE);}

    // check whether it is comp
    if (get_type(et) < 3) {fprintf (stderr, MSG_ERR_IMAG_ARG_COMP, line_num+1); exit(EXIT_FAILURE);}

    // ------------------------------------------------------------------------
    // update variable status -------------------------------------------------
    // ------------------------------------------------------------------------

    if (et%OFST != 0) v_used[et%OFST] = 1;

    // ------------------------------------------------------------------------
    // prepare local variables ------------------------------------------------
    // ------------------------------------------------------------------------

    char ld[10]; if (acc_ok == 0) strcpy(ld,"LOD"); else strcpy(ld,"P_LOD");

    // ------------------------------------------------------------------------
    // execute ----------------------------------------------------------------
    // ------------------------------------------------------------------------

    // comp const
    if (get_type(et) == 5)
    {
        int et_r, et_i;
        get_cmp_cst(et,&et_r,&et_i);

        add_instr("%s %s\n", ld, v_name[et_i%OFST]);
    }

    // comp in memory
    if ((get_type(et) == 3) && (et%OFST != 0))
    {
        int et_r, et_i;
        get_cmp_ets(et,&et_r,&et_i);

        add_instr("%s %s\n", ld, v_name[et_i%OFST]);
    }

    // comp in acc
    if ((get_type(et) == 3) && (et%OFST == 0))
    {
        add_instr("SET_P aux_var\n");
        add_instr("LOD   aux_var\n");
    }

    acc_ok = 1;

    return 2*OFST;
}

// squared magnitude of a complex number
int exec_mod2(int et)
{
    // ------------------------------------------------------------------------
    // consistency check ------------------------------------------------------
    // ------------------------------------------------------------------------

    // check whether et was declared
    if (et%OFST != 0 && v_type[et%OFST] == 0) {fprintf(stderr, MSG_ERR_DECL_FIRST, line_num+1, rem_fname(v_name[et%OFST], fname)); exit(EXIT_FAILURE);}

    // check whether et is a variable
    if (et%OFST != 0 && v_isar[et%OFST] > 0) {fprintf(stderr, MSG_ERR_WRONG_USE, line_num+1, rem_fname(v_name[et%OFST], fname)); exit(EXIT_FAILURE);}

    // check whether it is comp
    if (get_type(et) < 3) {fprintf (stderr, MSG_ERR_MOD2_ARG_COMP, line_num+1); exit(EXIT_FAILURE);}

    // ------------------------------------------------------------------------
    // update variable status -------------------------------------------------
    // ------------------------------------------------------------------------

    if (et%OFST != 0) v_used[et%OFST] = 1;

    // ------------------------------------------------------------------------
    // prepare local variables ------------------------------------------------
    // ------------------------------------------------------------------------

    int etr, eti;

    // ------------------------------------------------------------------------
    // execute ----------------------------------------------------------------
    // ------------------------------------------------------------------------

    // when it is a constant -------------------------------------------------
    if (get_type(et) == 5)
    {
        get_cmp_cst(et ,&etr,&eti);     // get the et of each float constant
        etr = expr_to_et(oper_mult(expr_of_et(etr), expr_of_et(etr)));     // real part squared
        eti = expr_to_et(oper_mult(expr_of_et(eti), expr_of_et(eti)));     // imag part squared
        etr = expr_to_et(oper_soma(expr_of_et(etr), expr_of_et(eti)));     // sum of the squares
    }

    // when it is in memory --------------------------------------------------
    if ((get_type(et) == 3) && (et % OFST != 0))
    {
        get_cmp_ets(et ,&etr,&eti);     // get the et of each variable
        etr = expr_to_et(oper_mult(expr_of_et(etr), expr_of_et(etr)));     // real part squared
        eti = expr_to_et(oper_mult(expr_of_et(eti), expr_of_et(eti)));     // imag part squared
        etr = expr_to_et(oper_soma(expr_of_et(etr), expr_of_et(eti)));     // sum of the squares
    }

    // when it is in the accumulator ------------------------------------------
    if ((get_type(et) == 3) && (et % OFST == 0))
    {
        add_instr("PSH\n");             // imag part stays in acc and on the stack
        oper_mult(expr_of_et(2*OFST), expr_of_et(2*OFST));      // multiplica acc com pilha
        add_instr("SET_P aux_var\n");   // save temp and fetch real part

        add_instr("PSH\n");             // real part stays in acc and on the stack
        oper_mult(expr_of_et(2*OFST), expr_of_et(2*OFST));      // multiplica acc com pilha
        add_instr("P_LOD aux_var\n");   // push the real-squared onto the stack and fetch the imag-squared

        oper_soma(expr_of_et(2*OFST), expr_of_et(2*OFST));       // sum of the squares

        etr = 2*OFST;                   // output must be an extended et for float in the acc
    }

    return 2*OFST;
}

// computes the phase (in radians) of a complex number
int exec_fase(int et)
{
    // ------------------------------------------------------------------------
    // consistency check ------------------------------------------------------
    // ------------------------------------------------------------------------

    // check whether et was declared
    if (et%OFST != 0 && v_type[et%OFST] == 0) {fprintf(stderr, MSG_ERR_DECL_FIRST, line_num+1, rem_fname(v_name[et%OFST], fname)); exit(EXIT_FAILURE);}

    // check whether et is a variable
    if (et%OFST != 0 && v_isar[et%OFST] > 0) {fprintf(stderr, MSG_ERR_WRONG_USE, line_num+1, rem_fname(v_name[et%OFST], fname)); exit(EXIT_FAILURE);}

    // check whether it is comp
    if (get_type(et) < 3) {fprintf (stderr, MSG_ERR_FASE_ARG_COMP, line_num+1); exit(EXIT_FAILURE);}

    // ------------------------------------------------------------------------
    // update variable status -------------------------------------------------
    // ------------------------------------------------------------------------

    if (et%OFST != 0) v_used[et%OFST] = 1;

    // ------------------------------------------------------------------------
    // prepare local variables ------------------------------------------------
    // ------------------------------------------------------------------------
    
    int et_r, et_i;

    // ------------------------------------------------------------------------
    // execute ----------------------------------------------------------------
    // ------------------------------------------------------------------------

    // comp const
    if (get_type(et) == 5)
    {
        get_cmp_cst(et,&et_i,&et_r);
        oper_divi  (et_r,et_i);
    }

    // comp in memory
    if ((get_type(et) == 3) && (et%OFST != 0))
    {
        get_cmp_ets(et,&et_i,&et_r);
        oper_divi  (et_r,et_i);
    }

    // comp in acc
    if ((get_type(et) == 3) && (et%OFST == 0))
    {
        int id = exec_id("aux_var");
        et_i   = 2*OFST + id;

        add_instr("SET_P %s\n", v_name[id]);
        oper_divi(expr_of_et(et_i), expr_of_et(2*OFST));
    }

    exec_atan(2*OFST);

    acc_ok = 1;
    return 2*OFST;
}

// joins two real numbers into a complex
int exec_comp(int etr, int eti)
{
    // ------------------------------------------------------------------------
    // consistency check ------------------------------------------------------
    // ------------------------------------------------------------------------

    // check whether etr was declared
    if (etr%OFST != 0 && v_type[etr%OFST] == 0) {fprintf(stderr, MSG_ERR_DECL_FIRST, line_num+1, rem_fname(v_name[etr%OFST], fname)); exit(EXIT_FAILURE);}

    // check whether eti was declared
    if (eti%OFST != 0 && v_type[eti%OFST] == 0) {fprintf(stderr, MSG_ERR_DECL_FIRST, line_num+1, rem_fname(v_name[eti%OFST], fname)); exit(EXIT_FAILURE);}

    // check whether etr is a variable
    if (etr%OFST != 0 && v_isar[etr%OFST] > 0) {fprintf(stderr, MSG_ERR_WRONG_USE, line_num+1, rem_fname(v_name[etr%OFST], fname)); exit(EXIT_FAILURE);}

    // check whether et is a variable
    if (eti%OFST != 0 && v_isar[eti%OFST] > 0) {fprintf(stderr, MSG_ERR_WRONG_USE, line_num+1, rem_fname(v_name[eti%OFST], fname)); exit(EXIT_FAILURE);}

    // check whether it is comp
    if (get_type(etr) > 2 || get_type(eti) > 2) {fprintf (stderr, MSG_ERR_COMPLEX_OF_COMPLEX, line_num+1); exit(EXIT_FAILURE);}

    // ------------------------------------------------------------------------
    // update variable status -------------------------------------------------
    // ------------------------------------------------------------------------

    if (etr%OFST != 0) v_used[etr%OFST] = 1;
    if (eti%OFST != 0) v_used[eti%OFST] = 1;

    // ------------------------------------------------------------------------
    // prepare local variables ------------------------------------------------
    // ------------------------------------------------------------------------

    char ld [10]; if (acc_ok == 0) strcpy(ld ,"LOD"  ); else strcpy(ld ,"P_LOD"  );
    char i2f[10]; if (acc_ok == 0) strcpy(i2f,"I2F_M"); else strcpy(i2f,"P_I2F_M");

    // ------------------------------------------------------------------------
    // execute ----------------------------------------------------------------
    // ------------------------------------------------------------------------

    // int in memory and int in memory
    if ((get_type(etr) == 1) && (etr % OFST != 0) && (get_type(eti) == 1) && (eti % OFST != 0))
    {
        add_instr("%s %s\n", i2f, v_name[etr%OFST]);
        add_instr("P_I2F_M %s\n", v_name[eti%OFST]);
    }

    // int in memory and int in acc
    if ((get_type(etr) == 1) && (etr % OFST != 0) && (get_type(eti) == 1) && (eti % OFST == 0))
    {
        add_instr("SET aux_var\n");
        add_instr("I2F_M %s\n", v_name[etr%OFST]);
        add_instr("P_I2F_M aux_var\n");
    }

    // int in memory and float in memory
    if ((get_type(etr) == 1) && (etr % OFST != 0) && (get_type(eti) == 2) && (eti % OFST != 0))
    {
        add_instr("%s %s\n", i2f, v_name[etr%OFST]);
        add_instr("P_LOD %s\n"  , v_name[eti%OFST]);
    }

    // int in memory and float in acc
    if ((get_type(etr) == 1) && (etr % OFST != 0) && (get_type(eti) == 2) && (eti % OFST == 0))
    {
        add_instr("SET aux_var\n");
        add_instr("I2F_M %s\n", v_name[etr%OFST]);
        add_instr("P_LOD aux_var\n");
    }

    // int in acc and int in memory
    if ((get_type(etr) == 1) && (etr % OFST == 0) && (get_type(eti) == 1) && (eti % OFST != 0))
    {
        add_instr("I2F\n");
        add_instr("P_I2F_M %s\n", v_name[eti%OFST]);
    }

    // int in acc and int in acc
    if ((get_type(etr) == 1) && (etr % OFST == 0) && (get_type(eti) == 1) && (eti % OFST == 0))
    {
        add_instr("SET_P aux_var\n");
        add_instr("I2F\n");
        add_instr("P_I2F_M aux_var\n");
    }

    // int in acc and float in memory
    if ((get_type(etr) == 1) && (etr % OFST == 0) && (get_type(eti) == 2) && (eti % OFST != 0))
    {
        add_instr("I2F\n");
        add_instr("P_LOD %s\n", v_name[eti%OFST]);
    }

    // int in acc and float in acc
    if ((get_type(etr) == 1) && (etr % OFST == 0) && (get_type(eti) == 2) && (eti % OFST == 0))
    {
        add_instr("SET_P aux_var\n");
        add_instr("I2F\n");
        add_instr("P_LOD aux_var\n");
    }

    // float in memory and int in memory
    if ((get_type(etr) == 2) && (etr % OFST != 0) && (get_type(eti) == 1) && (eti % OFST != 0))
    {
        add_instr("%s %s\n",  ld, v_name[etr%OFST]);
        add_instr("P_I2F_M %s\n", v_name[eti%OFST]);
    }

    // float in memory and int in acc
    if ((get_type(etr) == 2) && (etr % OFST != 0) && (get_type(eti) == 1) && (eti % OFST == 0))
    {
        add_instr("SET aux_var\n");
        add_instr("LOD %s\n", v_name[etr%OFST]);
        add_instr("P_I2F_M aux_var\n");
    }

    // float in memory and float in memory
    if ((get_type(etr) == 2) && (etr % OFST != 0) && (get_type(eti) == 2) && (eti % OFST != 0))
    {
        add_instr("%s %s\n", ld, v_name[etr%OFST]);
        add_instr("P_LOD %s\n" , v_name[eti%OFST]);
    }

    // float in memory and float in acc
    if ((get_type(etr) == 2) && (etr % OFST != 0) && (get_type(eti) == 2) && (eti % OFST == 0))
    {
        add_instr("SET aux_var\n");
        add_instr("LOD %s\n", v_name[etr%OFST]);
        add_instr("P_LOD aux_var\n");
    }

    // float in acc and int in memory
    if ((get_type(etr) == 2) && (etr % OFST == 0) && (get_type(eti) == 1) && (eti % OFST != 0))
    {
        add_instr("P_I2F_M %s\n", v_name[eti%OFST]);
    }

    // float in acc and int in acc
    if ((get_type(etr) == 2) && (etr % OFST == 0) && (get_type(eti) == 1) && (eti % OFST == 0))
    {
        add_instr("I2F\n");
    }

    // float in acc and float in memory
    if ((get_type(etr) == 2) && (etr % OFST == 0) && (get_type(eti) == 2) && (eti % OFST != 0))
    {
        add_instr("P_LOD %s\n", v_name[eti%OFST]);
    }

    // float in acc and float in acc
    if ((get_type(etr) == 1) && (etr % OFST == 0) && (get_type(eti) == 2) && (eti % OFST == 0))
    {
        // nothing to do
    }

    acc_ok = 1;
    return 3*OFST;
}

// ----------------------------------------------------------------------------
// special functions for vector work ------------------------------------------
// does not create stdlib functions directly ----------------------------------
// instead, uses Dirac notation in statements --------------------------------
// ----------------------------------------------------------------------------

// multiplication between two vectors, e.g. <a|b>
// this routine produces an exp
int exec_vtv(int id1, int id2)
{
    // ------------------------------------------------------------------------
    // consistency check ------------------------------------------------------
    // ------------------------------------------------------------------------

    // check whether id1 was declared
    if (v_type[id1] == 0) {fprintf(stderr, MSG_ERR_VAR_NOT_FOUND, line_num+1, rem_fname(v_name[id1], fname)); exit(EXIT_FAILURE);}

    // check whether id2 was declared
    if (v_type[id2] == 0) {fprintf(stderr, MSG_ERR_VAR_NOT_FOUND, line_num+1, rem_fname(v_name[id2], fname)); exit(EXIT_FAILURE);}

    // check that they really are vectors
    if (v_isar[id1] != 1 || v_isar[id2] != 1) {fprintf(stderr, MSG_ERR_INNER_NEEDS_VECTORS, line_num+1); exit(EXIT_FAILURE);}

    // check that the sizes match
    if (v_size[id1] != v_size[id2]) {fprintf(stderr, MSG_ERR_VECTOR_SIZE_DIFF, line_num+1); exit(EXIT_FAILURE);}

    // check that they are the same type
    if (v_type[id1] != v_type[id2]) {fprintf(stderr, MSG_ERR_TYPE_DIFF, line_num+1); exit(EXIT_FAILURE);}

    // check whether there is a comp variable
    if (v_type[id1] == 3 || v_type[id2] == 3) {fprintf(stderr, MSG_ERR_NOT_IMPL_COMPLEX, line_num+1); exit(EXIT_FAILURE);}

    // ------------------------------------------------------------------------
    // update variable status -------------------------------------------------
    // ------------------------------------------------------------------------

    v_used[id1] = 1;
    v_used[id2] = 1;

    // ------------------------------------------------------------------------
    // prepare local variables ------------------------------------------------
    // ------------------------------------------------------------------------

    int N = v_size[id1];

    char ld[10]; if (acc_ok == 0) strcpy(ld,"LOD"); else strcpy(ld,"P_LOD");

    // ------------------------------------------------------------------------
    // implements the vector product ------------------------------------------
    // ------------------------------------------------------------------------

    printf(MSG_INFO_DIRAC_INNER, line_num+1);

    // implement every combination

    // int with int
    if ((v_type[id1] == 1) && (v_type[id2] == 1))
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
    if ((v_type[id1] == 2) && (v_type[id2] == 2))
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
    return v_type[id1]*OFST;
}

// matrix-vector multiplication, e.g. A # |B|b>;
void exec_Mv(int idy, int idM, int idv)
{
    // ------------------------------------------------------------------------
    // consistency check ------------------------------------------------------
    // ------------------------------------------------------------------------

    // check whether idy was declared
    if (v_type[idy] == 0) {fprintf(stderr, MSG_ERR_DECL_FIRST, line_num+1, rem_fname(v_name[idy], fname)); exit(EXIT_FAILURE);}

    // check whether idM was declared
    if (v_type[idM] == 0) {fprintf(stderr, MSG_ERR_DECL_FIRST, line_num+1, rem_fname(v_name[idM], fname)); exit(EXIT_FAILURE);}

    // check whether idv was declared
    if (v_type[idv] == 0) {fprintf(stderr, MSG_ERR_DECL_FIRST, line_num+1, rem_fname(v_name[idv], fname)); exit(EXIT_FAILURE);}

    // check that the types match
    if (v_type[idy] != v_type[idM] || v_type[idy] != v_type[idv]) {fprintf(stderr, MSG_ERR_VARS_SAME_TYPE, line_num+1); exit(EXIT_FAILURE);}

    // check that it is not comp
    if (v_type[idy] == 3 || v_type[idM] == 3 || v_type[idv] == 3) {fprintf(stderr, MSG_ERR_NOT_IMPL_COMPLEX, line_num+1); exit(EXIT_FAILURE);}

    // check whether idy is a vector
    if (v_isar[idy] != 1) {fprintf(stderr, MSG_ERR_NOT_A_VECTOR, line_num+1, rem_fname(v_name[idy], fname)); exit(EXIT_FAILURE);}

    // check whether idM is a matrix
    if (v_isar[idM] != 2) {fprintf(stderr, MSG_ERR_NOT_A_MATRIX2, line_num+1, rem_fname(v_name[idM], fname)); exit(EXIT_FAILURE);}

    // check whether idv is a vector
    if (v_isar[idv] != 1) {fprintf(stderr, MSG_ERR_NOT_A_VECTOR, line_num+1, rem_fname(v_name[idv], fname)); exit(EXIT_FAILURE);}

    // check size between output and matrix
    if (v_size[idy] != v_size[idM]) {fprintf(stderr, MSG_ERR_MATRIX_ROW_MISMATCH, line_num+1, rem_fname(v_name[idM], fname), rem_fname(v_name[idy], fname)); exit(EXIT_FAILURE);}

    // check size between matrix and vector
    if (v_size[idv] != v_siz2[idM]) {fprintf(stderr, MSG_ERR_MATRIX_COL_MISMATCH, line_num+1, rem_fname(v_name[idM], fname), rem_fname(v_name[idv], fname)); exit(EXIT_FAILURE);}

    // ------------------------------------------------------------------------
    // update variable status -------------------------------------------------
    // ------------------------------------------------------------------------

    v_used[idM] = 1;
    v_used[idv] = 1;

    // ------------------------------------------------------------------------
    // prepare local variables ------------------------------------------------
    // ------------------------------------------------------------------------

    int N = v_size[idM];
    int M = v_siz2[idM];

    // ------------------------------------------------------------------------
    // implements the matrix-vector product -----------------------------------
    // ------------------------------------------------------------------------

    printf(MSG_INFO_DIRAC_MV, line_num+1);

    // implement combinations only on demand

    // int with int
    if ((v_type[idM] == 1) && (v_type[idv] == 1))
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
    if ((v_type[idM] == 2) && (v_type[idv] == 2))
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
void exec_cv(int idy, int et, int idv)
{
    // ------------------------------------------------------------------------
    // consistency check ------------------------------------------------------
    // ------------------------------------------------------------------------

    // check whether idy was declared
    if (v_type[idy] == 0) {fprintf(stderr, MSG_ERR_DECL_FIRST, line_num+1, rem_fname(v_name[idy], fname)); exit(EXIT_FAILURE);}

    // check whether et was declared
    if (et%OFST != 0 && v_type[et%OFST] == 0) {fprintf(stderr, MSG_ERR_DECL_FIRST, line_num+1, rem_fname(v_name[et%OFST], fname)); exit(EXIT_FAILURE);}

    // check whether idv was declared
    if (v_type[idv] == 0) {fprintf(stderr, MSG_ERR_DECL_FIRST, line_num+1, rem_fname(v_name[idv], fname)); exit(EXIT_FAILURE);}

    // check that the types match
    if (v_type[idy] != get_type(et) || v_type[idy] != v_type[idv]) {fprintf(stderr, MSG_ERR_VARS_SAME_TYPE, line_num+1); exit(EXIT_FAILURE);}

    // check that it is not comp
    if (v_type[idy] == 3 || get_type(et) == 3 || v_type[idv] == 3) {fprintf(stderr, MSG_ERR_NOT_IMPL_COMPLEX, line_num+1); exit(EXIT_FAILURE);}

    // check whether idy is a vector
    if (v_isar[idy] != 1) {fprintf(stderr, MSG_ERR_NOT_A_VECTOR, line_num+1, rem_fname(v_name[idy], fname)); exit(EXIT_FAILURE);}

    // check whether et is a variable
    if (et%OFST != 0 && v_isar[et%OFST] > 0) {fprintf(stderr, MSG_ERR_WRONG_USE, line_num+1, rem_fname(v_name[et%OFST], fname)); exit(EXIT_FAILURE);}

    // check whether idv is a vector
    if (v_isar[idv] != 1) {fprintf(stderr, MSG_ERR_NOT_A_VECTOR, line_num+1, rem_fname(v_name[idv], fname)); exit(EXIT_FAILURE);}

    // check size between vectors
    if (v_size[idy] != v_size[idv]) {fprintf(stderr, MSG_ERR_VECTOR_SIZE_DIFF2, line_num+1); exit(EXIT_FAILURE);}

    // ------------------------------------------------------------------------
    // update variable status -------------------------------------------------
    // ------------------------------------------------------------------------

    if (et%OFST != 0) v_used[et%OFST] = 1;
    v_used[idv] = 1;

    // ------------------------------------------------------------------------
    // prepare local variables ------------------------------------------------
    // ------------------------------------------------------------------------

    int N = v_size[idv];

    char g[64]; if (et%OFST==0) strcpy(g,"aux_var"); else strcpy(g,v_name[et%OFST]);

    // ------------------------------------------------------------------------
    // execute ----------------------------------------------------------------
    // ------------------------------------------------------------------------

    printf(MSG_INFO_DIRAC_CV, line_num+1);

    if (et%OFST==0) add_instr("SET aux_var\n");

    // implement combinations on demand

    for (int i = 0; i < N; i++)
    {
        add_instr("LOD_V %s %d\n", v_name[idv], i);

        // int with int
        if (v_type[idy] == 1)
        {
            add_instr("MLT %s\n", g);
        }

        // float with float
        if (v_type[idy] == 2)
        {
            add_instr("F_MLT %s\n", g);
        }

        add_instr("SET_V %s %d\n", v_name[idy], i);
    }

    acc_ok = 0;
}

// weighted sum into the second vector, e.g. a # |b> + c|d>;
// added mainly for RLS use
void exec_apcb(int idy, int ida, int etc, int idb)
{
    // ------------------------------------------------------------------------
    // consistency check ------------------------------------------------------
    // ------------------------------------------------------------------------

    // check whether idy was declared
    if (v_type[idy] == 0) {fprintf(stderr, MSG_ERR_DECL_FIRST, line_num+1, rem_fname(v_name[idy], fname)); exit(EXIT_FAILURE);}

    // check whether ida was declared
    if (v_type[ida] == 0) {fprintf(stderr, MSG_ERR_DECL_FIRST, line_num+1, rem_fname(v_name[ida], fname)); exit(EXIT_FAILURE);}

    // check whether etc was declared
    if (etc%OFST != 0 && v_type[etc%OFST] == 0) {fprintf(stderr, MSG_ERR_DECL_FIRST, line_num+1, rem_fname(v_name[etc%OFST], fname)); exit(EXIT_FAILURE);}
    
    // check whether idb was declared
    if (v_type[idb] == 0) {fprintf(stderr, MSG_ERR_DECL_FIRST, line_num+1, rem_fname(v_name[idb], fname)); exit(EXIT_FAILURE);}

    // check that the types match
    if (v_type[idy] != v_type[ida] || v_type[idy] != get_type(etc) || v_type[idy] != v_type[idb]) {fprintf(stderr, MSG_ERR_VARS_SAME_TYPE, line_num+1); exit(EXIT_FAILURE);}

    // check that it is not comp
    if (v_type[idy] == 3 || v_type[ida] == 3 || get_type(etc) == 3 || v_type[idb] == 3) {fprintf(stderr, MSG_ERR_NOT_IMPL_COMPLEX, line_num+1); exit(EXIT_FAILURE);}

    // check whether idy is a vector
    if (v_isar[idy] != 1) {fprintf(stderr, MSG_ERR_NOT_A_VECTOR, line_num+1, rem_fname(v_name[idy], fname)); exit(EXIT_FAILURE);}

    // check whether ida is a vector
    if (v_isar[ida] != 1) {fprintf(stderr, MSG_ERR_NOT_A_VECTOR, line_num+1, rem_fname(v_name[ida], fname)); exit(EXIT_FAILURE);}

    // check whether et is a variable
    if (etc%OFST != 0 && v_isar[etc%OFST] > 0) {fprintf(stderr, MSG_ERR_WRONG_USE, line_num+1, rem_fname(v_name[etc%OFST], fname)); exit(EXIT_FAILURE);}

    // check whether idb is a vector
    if (v_isar[idb] != 1) {fprintf(stderr, MSG_ERR_NOT_A_VECTOR, line_num+1, rem_fname(v_name[idb], fname)); exit(EXIT_FAILURE);}

    // check size between vectors
    if (v_size[idy] != v_size[ida]) {fprintf(stderr, MSG_ERR_VECTOR_SIZE_DIFF2, line_num+1); exit(EXIT_FAILURE);}

    // check size between vectors
    if (v_size[idy] != v_size[idb]) {fprintf(stderr, MSG_ERR_VECTOR_SIZE_DIFF2, line_num+1); exit(EXIT_FAILURE);}

    // ------------------------------------------------------------------------
    // update variable status -------------------------------------------------
    // ------------------------------------------------------------------------

    v_used[ida] = 1;
    if (etc%OFST != 0) v_used[etc%OFST] = 1;
    v_used[idb] = 1;

    // ------------------------------------------------------------------------
    // prepare local variables ------------------------------------------------
    // ------------------------------------------------------------------------

    printf(MSG_INFO_DIRAC_VECTOR_SUM, line_num+1);

    int N = v_size[idy];

    char g[64]; if (etc%OFST==0) strcpy(g,"aux_var"); else strcpy(g,v_name[etc%OFST]);

    // ------------------------------------------------------------------------
    // execute ----------------------------------------------------------------
    // ------------------------------------------------------------------------

    if (etc%OFST==0) add_instr("SET aux_var\n");

    for (int i = 0; i < N; i++)
    {
        add_instr("LOD_V %s %d\n", v_name[idb], i);

        // int
        if (v_type[idy] == 1)
        {
            add_instr("MLT %s\n", g);
            add_instr("ADD_V %s %d\n", v_name[ida], i);
        }

        // float
        if (v_type[idy] == 2)
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
    if (v_type[idM] == 0) {fprintf(stderr, MSG_ERR_DECL_FIRST, line_num+1, rem_fname(v_name[idM], fname)); exit(EXIT_FAILURE);}

    // check whether ida was declared
    if (v_type[ida] == 0) {fprintf(stderr, MSG_ERR_DECL_FIRST, line_num+1, rem_fname(v_name[ida], fname)); exit(EXIT_FAILURE);}
    
    // check whether idb was declared
    if (v_type[idb] == 0) {fprintf(stderr, MSG_ERR_DECL_FIRST, line_num+1, rem_fname(v_name[idb], fname)); exit(EXIT_FAILURE);}

    // check that the types match
    if (v_type[idM] != v_type[ida] || v_type[idM] != v_type[idb]) {fprintf(stderr, MSG_ERR_VARS_SAME_TYPE, line_num+1); exit(EXIT_FAILURE);}

    // check that it is not comp
    if (v_type[idM] == 3 || v_type[ida] == 3 || v_type[idb] == 3) {fprintf(stderr, MSG_ERR_NOT_IMPL_COMPLEX, line_num+1); exit(EXIT_FAILURE);}

    // check whether idM is a matrix
    if (v_isar[idM] != 2) {fprintf(stderr, MSG_ERR_NOT_A_MATRIX, line_num+1, rem_fname(v_name[idM], fname)); exit(EXIT_FAILURE);}

    // check whether ida is a vector
    if (v_isar[ida] != 1) {fprintf(stderr, MSG_ERR_NOT_A_VECTOR, line_num+1, rem_fname(v_name[ida], fname)); exit(EXIT_FAILURE);}

    // check whether idb is a vector
    if (v_isar[idb] != 1) {fprintf(stderr, MSG_ERR_NOT_A_VECTOR, line_num+1, rem_fname(v_name[idb], fname)); exit(EXIT_FAILURE);}

    // check size between elements
    if (v_size[idM] != v_size[ida] || v_size[idM] != v_size[idb]) {fprintf(stderr, MSG_ERR_DIM_MISMATCH, line_num+1); exit(EXIT_FAILURE);}

    // ------------------------------------------------------------------------
    // update variable status -------------------------------------------------
    // ------------------------------------------------------------------------

    v_used[ida] = 1;
    v_used[idb] = 1;

    // ------------------------------------------------------------------------
    // prepare local variables ------------------------------------------------
    // ------------------------------------------------------------------------

    int N = v_size[ida];

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
            if (v_type[idM] == 1)
            {
                add_instr("MLT_V %s %d\n", v_name[idb], j);
            }

            // float
            if (v_type[idM] == 2)
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
    if (v_type[idA] == 0) {fprintf(stderr, MSG_ERR_DECL_FIRST, line_num+1, rem_fname(v_name[idA], fname)); exit(EXIT_FAILURE);}

    // check whether idA was declared
    if (v_type[idB] == 0) {fprintf(stderr, MSG_ERR_DECL_FIRST, line_num+1, rem_fname(v_name[idB], fname)); exit(EXIT_FAILURE);}
    
    // check whether ida was declared
    if (v_type[ida] == 0) {fprintf(stderr, MSG_ERR_DECL_FIRST, line_num+1, rem_fname(v_name[ida], fname)); exit(EXIT_FAILURE);}

    // check whether idb was declared
    if (v_type[idb] == 0) {fprintf(stderr, MSG_ERR_DECL_FIRST, line_num+1, rem_fname(v_name[idb], fname)); exit(EXIT_FAILURE);}

    // check that the types match
    if (v_type[idA] != v_type[idB] || v_type[idA] != v_type[ida] || v_type[idA] != v_type[idb]) {fprintf(stderr, MSG_ERR_VARS_SAME_TYPE, line_num+1); exit(EXIT_FAILURE);}

    // check that it is not comp
    if (v_type[idA] == 3 || v_type[idB] == 3 || v_type[ida] == 3 || v_type[idb] == 3) {fprintf(stderr, MSG_ERR_NOT_IMPL_COMPLEX, line_num+1); exit(EXIT_FAILURE);}
    // check whether idA is a matrix
    if (v_isar[idA] != 2) {fprintf(stderr, MSG_ERR_NOT_A_MATRIX, line_num+1, rem_fname(v_name[idA], fname)); exit(EXIT_FAILURE);}

    // check whether idB is a matrix
    if (v_isar[idB] != 2) {fprintf(stderr, MSG_ERR_NOT_A_MATRIX, line_num+1, rem_fname(v_name[idB], fname)); exit(EXIT_FAILURE);}

    // check whether ida is a vector
    if (v_isar[ida] != 1) {fprintf(stderr, MSG_ERR_NOT_A_VECTOR, line_num+1, rem_fname(v_name[ida], fname)); exit(EXIT_FAILURE);}

    // check whether idb is a vector
    if (v_isar[idb] != 1) {fprintf(stderr, MSG_ERR_NOT_A_VECTOR, line_num+1, rem_fname(v_name[idb], fname)); exit(EXIT_FAILURE);}

    // check size between elements
    if (v_size[idA] != v_siz2[idA] || v_size[idA] != v_size[idB] || v_size[idA] != v_siz2[idB] || v_size[idA] != v_size[ida] || 
        v_size[idA] != v_size[idb]) {fprintf(stderr, MSG_ERR_DIM_MISMATCH, line_num+1); exit(EXIT_FAILURE);}

    // ------------------------------------------------------------------------
    // update variable status -------------------------------------------------
    // ------------------------------------------------------------------------

    v_used[idB] = 1;
    v_used[ida] = 1;
    v_used[idb] = 1;

    // ------------------------------------------------------------------------
    // prepare local variables ------------------------------------------------
    // ------------------------------------------------------------------------

    int N = v_size[ida];

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
            if (v_type[idA] == 1)
            {
                add_instr("MLT_V %s %d\n", v_name[idb],     j);
                add_instr("NEG\n");
                add_instr("ADD_V %s %d\n", v_name[idB], N*j+i);
            }

            // float
            if (v_type[idA] == 2)
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
void exec_cM(int idA, int etc, int idM)
{
    // ------------------------------------------------------------------------
    // consistency check ------------------------------------------------------
    // ------------------------------------------------------------------------

    // check whether idA was declared
    if (v_type[idA] == 0) {fprintf(stderr, MSG_ERR_DECL_FIRST, line_num+1, rem_fname(v_name[idA], fname)); exit(EXIT_FAILURE);}

    // check whether etc was declared
    if (etc%OFST != 0 && v_type[etc%OFST] == 0) {fprintf(stderr, MSG_ERR_DECL_FIRST, line_num+1, rem_fname(v_name[etc%OFST], fname)); exit(EXIT_FAILURE);}
    
    // check whether idM was declared
    if (v_type[idM] == 0) {fprintf(stderr, MSG_ERR_DECL_FIRST, line_num+1, rem_fname(v_name[idM], fname)); exit(EXIT_FAILURE);}

    // check that the types match
    if (v_type[idA] != get_type(etc) || v_type[idA] != v_type[idM]) {fprintf(stderr, MSG_ERR_VARS_SAME_TYPE, line_num+1); exit(EXIT_FAILURE);}

    // check that it is not comp
    if (v_type[idA] == 3 || get_type(etc) == 3 || v_type[idM] == 3) {fprintf(stderr, MSG_ERR_NOT_IMPL_COMPLEX, line_num+1); exit(EXIT_FAILURE);}

    // check whether idA is a matrix
    if (v_isar[idA] != 2) {fprintf(stderr, MSG_ERR_NOT_A_MATRIX, line_num+1, rem_fname(v_name[idA], fname)); exit(EXIT_FAILURE);}

    // check whether etc is a variable
    if (etc%OFST != 0 && v_isar[etc%OFST] > 0) {fprintf(stderr, MSG_ERR_WRONG_USE, line_num+1, rem_fname(v_name[etc%OFST], fname)); exit(EXIT_FAILURE);}

    // check whether idM is a matrix
    if (v_isar[idM] != 2) {fprintf(stderr, MSG_ERR_NOT_A_MATRIX, line_num+1, rem_fname(v_name[idM], fname)); exit(EXIT_FAILURE);}

    // check size between elements
    if (v_size[idA] != v_size[idM] || v_siz2[idA] != v_siz2[idM]) {fprintf(stderr, MSG_ERR_DIM_MISMATCH, line_num+1); exit(EXIT_FAILURE);}

    // ------------------------------------------------------------------------
    // update variable status -------------------------------------------------
    // ------------------------------------------------------------------------

    if (etc%OFST != 0) v_used[etc%OFST] = 1;
    v_used[idM] = 1;

    // ------------------------------------------------------------------------
    // prepare local variables ------------------------------------------------
    // ------------------------------------------------------------------------

    int N = v_size[idM];
    int M = v_siz2[idM];

    char g[64]; if (etc%OFST==0) strcpy(g,"aux_var"); else strcpy(g,v_name[etc%OFST]);

    // ------------------------------------------------------------------------
    // execute ----------------------------------------------------------------
    // ------------------------------------------------------------------------

    printf(MSG_INFO_DIRAC_CM, line_num+1);

    if (etc%OFST==0) add_instr("SET aux_var\n");

    for (int i = 0; i < N; i++)
    {
        for (int j = 0; j < M; j++)
        {
            add_instr("LOD_V %s %d\n", v_name[idM], M*i+j);

            // int
            if (v_type[idA] == 1) add_instr("MLT %s\n", g);

            // float
            if (v_type[idA] == 2) add_instr("F_MLT %s\n", g);
            
            add_instr("SET_V %s %d\n", v_name[idA], M*i+j);
        }
    }

    acc_ok = 0;
}

// generates identity matrix with constant, e.g. A # c|I|;
void exec_cI(int idM, int etc)
{
    // ------------------------------------------------------------------------
    // consistency check ------------------------------------------------------
    // ------------------------------------------------------------------------

    // check whether idM was declared
    if (v_type[idM] == 0) {fprintf(stderr, MSG_ERR_DECL_FIRST, line_num+1, rem_fname(v_name[idM], fname)); exit(EXIT_FAILURE);}

    // check whether etc was declared
    if (etc%OFST != 0 && v_type[etc%OFST] == 0) {fprintf(stderr, MSG_ERR_DECL_FIRST, line_num+1, rem_fname(v_name[etc%OFST], fname)); exit(EXIT_FAILURE);}

    // check that the types match
    if (v_type[idM] != get_type(etc)) {fprintf(stderr, MSG_ERR_VARS_SAME_TYPE, line_num+1); exit(EXIT_FAILURE);}

    // check that it is not comp
    if (v_type[idM] == 3 || get_type(etc) == 3) {fprintf(stderr, MSG_ERR_NOT_IMPL_COMPLEX, line_num+1); exit(EXIT_FAILURE);}
    // check whether idM is a matrix
    if (v_isar[idM] != 2) {fprintf(stderr, MSG_ERR_NOT_A_MATRIX, line_num+1, rem_fname(v_name[idM], fname)); exit(EXIT_FAILURE);}

    // check whether etc is a variable
    if (etc%OFST != 0 && v_isar[etc%OFST] > 0) {fprintf(stderr, MSG_ERR_WRONG_USE, line_num+1, rem_fname(v_name[etc%OFST], fname)); exit(EXIT_FAILURE);}

    // ------------------------------------------------------------------------
    // update variable status -------------------------------------------------
    // ------------------------------------------------------------------------

    if (etc%OFST != 0) v_used[etc%OFST] = 1;

    // ------------------------------------------------------------------------
    // prepare local variables ------------------------------------------------
    // ------------------------------------------------------------------------

    int N = v_size[idM];

    // ------------------------------------------------------------------------
    // execute ----------------------------------------------------------------
    // ------------------------------------------------------------------------

    printf(MSG_INFO_DIRAC_CM, line_num+1);

    if (etc%OFST!=0) add_instr("LOD %s\n",v_name[etc%OFST]);

    for (int i = 0; i < N; i++)
    {
        for (int j = 0; j < N; j++)
        {
            if (i == j) add_instr("SET_V %s %d\n", v_name[idM], N*i+j);
        }
    }

    // int
    if (v_type[idM] == 1) add_instr("LOD 0\n");

    // float
    if (v_type[idM] == 2) add_instr("LOD 0.0\n");
    
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
    if (v_type[idv] == 0) {fprintf(stderr, MSG_ERR_DECL_FIRST, line_num+1, rem_fname(v_name[idv], fname)); exit(EXIT_FAILURE);}

    // check that it is not comp
    if (v_type[idv] == 3) {fprintf(stderr, MSG_ERR_NOT_IMPL_COMPLEX, line_num+1); exit(EXIT_FAILURE);}
    
    // check whether idv is a vector
    if (v_isar[idv] != 1) {fprintf(stderr, MSG_ERR_NOT_A_VECTOR, line_num+1, rem_fname(v_name[idv], fname)); exit(EXIT_FAILURE);}

    // ------------------------------------------------------------------------
    // prepare local variables ------------------------------------------------
    // ------------------------------------------------------------------------

    int N = v_size[idv];

    // ------------------------------------------------------------------------
    // execute ----------------------------------------------------------------
    // ------------------------------------------------------------------------

    printf(MSG_INFO_DIRAC_ZERO_VECTOR, line_num+1);

    // int
    if (v_type[idv] == 1) add_instr("LOD 0\n");

    // float
    if (v_type[idv] == 2) add_instr("LOD 0.0\n");

    for (int i = 0; i < N; i++) add_instr("SET_V %s %d\n", v_name[idv], i);
}

// reads input vector with weight c, e.g. a # c|in(0)>;
void exec_cvin(int idv, int etc, int idp)
{
    // ------------------------------------------------------------------------
    // consistency check ------------------------------------------------------
    // ------------------------------------------------------------------------

    // check whether idv was declared
    if (v_type[idv] == 0) {fprintf(stderr, MSG_ERR_DECL_FIRST, line_num+1, rem_fname(v_name[idv], fname)); exit(EXIT_FAILURE);}

    // check whether etc was declared
    if (etc%OFST != 0 && v_type[etc%OFST] == 0) {fprintf(stderr, MSG_ERR_DECL_FIRST, line_num+1, rem_fname(v_name[etc%OFST], fname)); exit(EXIT_FAILURE);}

    // check that it is not comp
    if (v_type[idv] == 3 || get_type(etc) == 3) {fprintf(stderr, MSG_ERR_NOT_IMPL_COMPLEX, line_num+1); exit(EXIT_FAILURE);}

    // check whether idv is a vector
    if (v_isar[idv] != 1) {fprintf(stderr, MSG_ERR_NOT_A_VECTOR, line_num+1, rem_fname(v_name[idv], fname)); exit(EXIT_FAILURE);}

    // check whether etc is a variable
    if (etc%OFST != 0 && v_isar[etc%OFST] > 0) {fprintf(stderr, MSG_ERR_WRONG_USE, line_num+1, rem_fname(v_name[etc%OFST], fname)); exit(EXIT_FAILURE);}

    // ------------------------------------------------------------------------
    // update variable status -------------------------------------------------
    // ------------------------------------------------------------------------

    if (etc%OFST != 0) v_used[etc%OFST] = 1;

    // ------------------------------------------------------------------------
    // prepare local variables ------------------------------------------------
    // ------------------------------------------------------------------------

    int N = v_size[idv];

    char g[64]; if (etc%OFST==0) strcpy(g,"aux_var"); else strcpy(g,v_name[etc%OFST]);

    // ------------------------------------------------------------------------
    // execute ----------------------------------------------------------------
    // ------------------------------------------------------------------------

    printf(MSG_INFO_DIRAC_SET_VECTOR, line_num+1);

    if (etc%OFST==0) add_instr("SET aux_var\n");

    for (int i = 0; i < N; i++)
    {
        // int
        if (v_type[idv] == 1)
        {
            add_instr("INN %s\n", v_name[idp]);
            add_instr("MLT %s\n",g);
        }

        // float
        if (v_type[idv] == 2)
        {
            add_instr("F_INN %s\n", v_name[idp]);
            add_instr("F_MLT %s\n",g);
        }

        add_instr("SET_V %s %d\n", v_name[idv], i);
    }

    acc_ok = 0;
}

// writes vector to output with weight c, e.g. out(0, c|a>);
void exec_vout(int idp, int etc, int idv)
{
    // ------------------------------------------------------------------------
    // consistency check ------------------------------------------------------
    // ------------------------------------------------------------------------

    // check whether idv was declared
    if (v_type[idv] == 0) {fprintf(stderr, MSG_ERR_DECL_FIRST, line_num+1, rem_fname(v_name[idv], fname)); exit(EXIT_FAILURE);}

    // check whether etc was declared
    if (etc%OFST != 0 && v_type[etc%OFST] == 0) {fprintf(stderr, MSG_ERR_DECL_FIRST, line_num+1, rem_fname(v_name[etc%OFST], fname)); exit(EXIT_FAILURE);}

    // check that it is not comp
    if (v_type[idv] == 3 || get_type(etc) == 3) {fprintf(stderr, MSG_ERR_NOT_IMPL_COMPLEX, line_num+1); exit(EXIT_FAILURE);}

    // check whether idv is a vector
    if (v_isar[idv] != 1) {fprintf(stderr, MSG_ERR_NOT_A_VECTOR, line_num+1, rem_fname(v_name[idv], fname)); exit(EXIT_FAILURE);}

    // check whether etc is a variable
    if (etc%OFST != 0 && v_isar[etc%OFST] > 0) {fprintf(stderr, MSG_ERR_WRONG_USE, line_num+1, rem_fname(v_name[etc%OFST], fname)); exit(EXIT_FAILURE);}

    // ------------------------------------------------------------------------
    // update variable status -------------------------------------------------
    // ------------------------------------------------------------------------

    if (etc%OFST != 0) v_used[etc%OFST] = 1;
    v_used[idv] = 1;
    
    // ------------------------------------------------------------------------
    // prepare local variables ------------------------------------------------
    // ------------------------------------------------------------------------

    int N = v_size[idv];

    char g[64]; if (etc%OFST==0) strcpy(g,"aux_var"); else strcpy(g,v_name[etc%OFST]);

    // ------------------------------------------------------------------------
    // execute ----------------------------------------------------------------
    // ------------------------------------------------------------------------

    printf(MSG_INFO_DIRAC_FLUSH_VECTOR, line_num+1);

    if (etc%OFST==0) add_instr("SET aux_var\n");

    for (int i = 0; i < N; i++)
    {
        add_instr("LOD_V %s %d\n", v_name[idv], i);

        // int
        if (v_type[idv] == 1)
        {
            add_instr("MLT %s\n",g);
        }

        // float
        if (v_type[idv] == 2)
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
void exec_shift(int ida, int etb, int idc)
{
    // ------------------------------------------------------------------------
    // consistency check ------------------------------------------------------
    // ------------------------------------------------------------------------

    // check whether ida was declared
    if (v_type[ida] == 0) {fprintf(stderr, MSG_ERR_DECL_FIRST, line_num+1, rem_fname(v_name[ida], fname)); exit(EXIT_FAILURE);}

    // check whether etb was declared
    if (etb%OFST != 0 && v_type[etb%OFST] == 0) {fprintf(stderr, MSG_ERR_DECL_FIRST, line_num+1, rem_fname(v_name[etb%OFST], fname)); exit(EXIT_FAILURE);}

    // check whether idc equals ida
    if (idc != ida) {fprintf(stderr, MSG_ERR_SHIFT_VEC_SELF, line_num+1); exit(EXIT_FAILURE);}
    
    // check that it is not comp
    if (v_type[ida] == 3 || get_type(etb) == 3) {fprintf(stderr, MSG_ERR_NOT_IMPL_COMPLEX, line_num+1); exit(EXIT_FAILURE);}

    // check that the types match
    //if (v_type[ida] != v_type[etb%OFST]) {fprintf(stderr, MSG_ERR_VARS_SAME_TYPE, line_num+1); exit(EXIT_FAILURE);}
    
    // check whether ida is a vector
    if (v_isar[ida] != 1) {fprintf(stderr, MSG_ERR_NOT_A_VECTOR, line_num+1, rem_fname(v_name[ida], fname)); exit(EXIT_FAILURE);}

    // check whether etb is a variable
    if (etb%OFST != 0 && v_isar[etb%OFST] > 0) {fprintf(stderr, MSG_ERR_WRONG_USE, line_num+1, rem_fname(v_name[etb%OFST], fname)); exit(EXIT_FAILURE);}

    // ------------------------------------------------------------------------
    // update variable status -------------------------------------------------
    // ------------------------------------------------------------------------

    if (etb%OFST != 0) v_used[etb%OFST] = 1;
    
    // ------------------------------------------------------------------------
    // prepare local variables ------------------------------------------------
    // ------------------------------------------------------------------------

    int N = v_size[ida];

    // ------------------------------------------------------------------------
    // execute ----------------------------------------------------------------
    // ------------------------------------------------------------------------

    printf(MSG_INFO_DIRAC_SHIFT, v_name[ida], line_num+1);

    // ida int and etb int in memory
    if (v_type[ida] == 1 && get_type(etb) == 1 && etb%OFST != 0)
    {
        for (int i = N-1; i > 0; i--)
        {
            add_instr("LOD_V %s %d\n", v_name[ida], i-1);
            add_instr("SET_V %s %d\n", v_name[ida], i);
        }

        add_instr("LOD %s\n", v_name[etb%OFST]);
        add_instr("SET %s\n", v_name[ida]);
    }

    // ida float e etb int no acc
    if (v_type[ida] == 2 && get_type(etb) == 1 && etb%OFST == 0)
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
    if (v_type[ida] == 2 && get_type(etb) == 2 && etb%OFST != 0)
    {
        for (int i = N-1; i > 0; i--)
        {
            add_instr("LOD_V %s %d\n", v_name[ida], i-1);
            add_instr("SET_V %s %d\n", v_name[ida], i);
        }

        add_instr("LOD %s\n", v_name[etb%OFST]);
        add_instr("SET %s\n", v_name[ida]);
    }

    // ida float e etb float no acc
    if (v_type[ida] == 2 && get_type(etb) == 2 && etb%OFST == 0)
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