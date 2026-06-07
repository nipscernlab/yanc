// ----------------------------------------------------------------------------
// SAPHO standard library -----------------------------------------------------
// ----------------------------------------------------------------------------

// global includes
#include <string.h>
#include <stdlib.h>

// local includes
#include "../Headers/t2t.h"
#include "../Headers/oper.h"
#include "../Headers/stdlib.h"
#include "../Headers/global.h"
#include "../Headers/macros.h"
#include "../Headers/funcoes.h"
#include "../Headers/data_use.h"
#include "../Headers/variaveis.h"
#include "../Headers/diretivas.h"
#include "../Headers/messages.h"

// ----------------------------------------------------------------------------
// input and output ----------------------------------------------------------
// ----------------------------------------------------------------------------

// input ex: int x = in(0);
expr exec_in(int id)
{
    if (atoi(v_table[id].name) >= nuioin) {fprintf(stderr, MSG_ERR_NO_IN_PORT, line_num+1, v_table[id].name); exit(EXIT_FAILURE);}

    if (acc_ok == 0) add_instr("INN %s\n", v_table[id].name); else add_instr("P_INN %s\n", v_table[id].name);

    acc_ok = 1;  // marks the acc as now holding a value

    return expr_make(1, 0);
}

// input ex: float x = fin(0);
expr exec_fin(int id)
{
    if (atoi(v_table[id].name) >= nuioin) {fprintf(stderr, MSG_ERR_NO_IN_PORT, line_num+1, v_table[id].name); exit(EXIT_FAILURE);}

    if (acc_ok == 0) add_instr("F_INN %s\n", v_table[id].name); else add_instr("PF_INN %s\n", v_table[id].name);

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
    if (e.id != 0 && v_table[e.id].type == 0) {fprintf(stderr, MSG_ERR_DECL_FIRST, line_num+1, rem_fname(v_table[e.id].name, fname)); exit(EXIT_FAILURE);}

    // check whether it is comp
    if (e.type > 2) {fprintf (stderr, MSG_ERR_PICK_COMP_INFO, line_num+1); exit(EXIT_FAILURE);}

    // check whether et is a variable
    if (e.id != 0 && v_table[e.id].isar > 0) {fprintf(stderr, MSG_ERR_WRONG_USE, line_num+1, rem_fname(v_table[e.id].name, fname)); exit(EXIT_FAILURE);}

    // check port range
    if (atoi(v_table[id].name) >= nuioou) {fprintf(stderr, MSG_ERR_NO_OUT_PORT, line_num+1, v_table[id].name); exit(EXIT_FAILURE);}

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
       if (acc_ok == 0) add_instr("LOD %s\n", v_table[e.id].name); else add_instr("P_LOD %s\n", v_table[e.id].name);
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

        if (acc_ok == 0) add_instr("F2I_M %s\n", v_table[e.id].name); else add_instr("P_F2I_M %s\n", v_table[e.id].name);
    }

    // float acc
    if ((e.type == 2) && (e.id==0))
    {
        fprintf(stdout, MSG_WARN_USE_FOUT, line_num+1);

        add_instr("F2I\n");
    }

    add_instr("OUT %s\n", v_table[id].name);

    acc_ok = 0; // libera acc    
}

// output ex: fout(0,x);
void exec_fout(int id, expr e)
{
    // ------------------------------------------------------------------------
    // consistency check ------------------------------------------------------
    // ------------------------------------------------------------------------

    // check whether et was declared
    if (e.id != 0 && v_table[e.id].type == 0) {fprintf(stderr, MSG_ERR_DECL_FIRST, line_num+1, rem_fname(v_table[e.id].name, fname)); exit(EXIT_FAILURE);}

    // check whether it is comp
    if (e.type > 2) {fprintf (stderr, MSG_ERR_PICK_COMP_INFO, line_num+1); exit(EXIT_FAILURE);}

    // check whether et is a variable
    if (e.id != 0 && v_table[e.id].isar > 0) {fprintf(stderr, MSG_ERR_WRONG_USE, line_num+1, rem_fname(v_table[e.id].name, fname)); exit(EXIT_FAILURE);}

    // check port range
    if (atoi(v_table[id].name) >= nuioou) {fprintf(stderr, MSG_ERR_NO_OUT_PORT, line_num+1, v_table[id].name); exit(EXIT_FAILURE);}

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

        if (acc_ok == 0) add_instr("LOD %s\n", v_table[e.id].name); else add_instr("P_LOD %s\n", v_table[e.id].name);
    }

    // int acc
    if ((e.type == 1) && (e.id==0))
    {
        fprintf(stdout, MSG_WARN_USE_OUT, line_num+1);
    }

    // float var
    if ((e.type == 2) && (e.id!=0))
    {
        if (acc_ok == 0) add_instr("F2I_M %s\n", v_table[e.id].name); else add_instr("P_F2I_M %s\n", v_table[e.id].name);
    }

    // float acc
    if ((e.type == 2) && (e.id==0))
    {
        add_instr("F2I\n");
    }

    add_instr("OUT %s\n", v_table[id].name);

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
    if (e1.id != 0 && v_table[e1.id].type == 0) {fprintf(stderr, MSG_ERR_DECL_FIRST, line_num+1, rem_fname(v_table[e1.id].name, fname)); exit(EXIT_FAILURE);}
    
    // check whether et2 was declared
    if (e2.id != 0 && v_table[e2.id].type == 0) {fprintf(stderr, MSG_ERR_DECL_FIRST, line_num+1, rem_fname(v_table[e2.id].name, fname)); exit(EXIT_FAILURE);}

    // check whether et1 is a variable
    if (e1.id != 0 && v_table[e1.id].isar > 0) {fprintf(stderr, MSG_ERR_WRONG_USE, line_num+1, rem_fname(v_table[e1.id].name, fname)); exit(EXIT_FAILURE);}

    // check whether et2 is a variable
    if (e2.id != 0 && v_table[e2.id].isar > 0) {fprintf(stderr, MSG_ERR_WRONG_USE, line_num+1, rem_fname(v_table[e2.id].name, fname)); exit(EXIT_FAILURE);}

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
        add_instr("%s %s\n", ld, v_table[e2.id].name);
        add_instr("SGN %s\n"   , v_table[e1.id].name);
    }

    // int in memory and int in acc
    if ((e1.type == 1) && (e1.id != 0) && (e2.type == 1) && (e2.id == 0))
    {
        add_instr("SGN %s\n", v_table[e1.id].name);
    }

    // int in memory and float var in memory
    if ((e1.type == 1) && (e1.id != 0) && (e2.type == 2) && (e2.id != 0))
    {
        add_instr("%s %s\n", ld, v_table[e2.id].name);
        add_instr("F_SGN %s\n" , v_table[e1.id].name);
    }

    // int in memory and float in acc
    if ((e1.type == 1) && (e1.id != 0) && (e2.type == 2) && (e2.id == 0))
    {
        add_instr("F_SGN %s\n", v_table[e1.id].name);
    }

    // int in acc and int in memory
    if ((e1.type == 1) && (e1.id == 0) && (e2.type == 1) && (e2.id != 0))
    {
        add_instr("%s %s\n", ld, v_table[e2.id].name);
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
        add_instr("P_LOD %s\n", v_table[e2.id].name  );
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
        add_instr("%s %s\n", ld, v_table[e2.id].name);
        add_instr("SGN %s\n"   , v_table[e1.id].name);
    }

    // float var and int in acc
    if ((e1.type == 2) && (e1.id != 0) && (e2.type == 1) && (e2.id == 0))
    {
        add_instr("SGN %s\n"  , v_table[e1.id].name);
    }

    // float var and float var
    if ((e1.type == 2) && (e1.id != 0) && (e2.type == 2) && (e2.id != 0))
    {
        add_instr("%s %s\n", ld, v_table[e2.id].name);
        add_instr("F_SGN %s\n" , v_table[e1.id].name);
    }

    // float var and float in acc
    if ((e1.type == 2) && (e1.id != 0) && (e2.type == 2) && (e2.id == 0))
    {
        add_instr("F_SGN %s\n" , v_table[e1.id].name);
    }

    // float in acc and int in memory
    if ((e1.type == 2) && (e1.id == 0) && (e2.type == 1) && (e2.id != 0))
    {
        add_instr("%s %s\n", ld, v_table[e2.id].name);
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
        add_instr("P_LOD %s\n", v_table[e2.id].name);
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
    if (e.id != 0 && v_table[e.id].type == 0) {fprintf(stderr, MSG_ERR_DECL_FIRST, line_num+1, rem_fname(v_table[e.id].name, fname)); exit(EXIT_FAILURE);}

    // check whether et is a variable
    if (e.id != 0 && v_table[e.id].isar > 0) {fprintf(stderr, MSG_ERR_WRONG_USE, line_num+1, rem_fname(v_table[e.id].name, fname)); exit(EXIT_FAILURE);}

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
        add_instr("%s %s\n", abs, v_table[e.id].name);
    }

    // int in acc
    if ((e.type == 1) && (e.id == 0))
    {
        add_instr("ABS\n");
    }

    // float in memory
    if ((e.type == 2) && (e.id != 0))
    {
        add_instr("%s %s\n", fabs, v_table[e.id].name);
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
    if (e.id != 0 && v_table[e.id].type == 0) {fprintf(stderr, MSG_ERR_DECL_FIRST, line_num+1, rem_fname(v_table[e.id].name, fname)); exit(EXIT_FAILURE);}

    // check whether et is a variable
    if (e.id != 0 && v_table[e.id].isar > 0) {fprintf(stderr, MSG_ERR_WRONG_USE, line_num+1, rem_fname(v_table[e.id].name, fname)); exit(EXIT_FAILURE);}

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
        add_instr("%s %s\n", pst, v_table[e.id].name);
    }

    // int in acc
    if ((e.type == 1) && (e.id == 0))
    {
        add_instr("PST\n");
    }

    // float in memory
    if ((e.type == 2)  && (e.id != 0))
    {
        add_instr("%s %s\n", fpst, v_table[e.id].name);
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
    if (e.id != 0 && v_table[e.id].type == 0) {fprintf(stderr, MSG_ERR_DECL_FIRST, line_num+1, rem_fname(v_table[e.id].name, fname)); exit(EXIT_FAILURE);}

    // check whether et is a variable
    if (e.id != 0 && v_table[e.id].isar > 0) {fprintf(stderr, MSG_ERR_WRONG_USE, line_num+1, rem_fname(v_table[e.id].name, fname)); exit(EXIT_FAILURE);}

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
        add_instr("%s %s\n", nrm, v_table[e.id].name);
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
    if (e.id != 0 && v_table[e.id].type == 0) {fprintf(stderr, MSG_ERR_DECL_FIRST, line_num+1, rem_fname(v_table[e.id].name, fname)); exit(EXIT_FAILURE);}

    // check whether the source is a plain variable (not an array)
    if (e.id != 0 && v_table[e.id].isar > 0) {fprintf(stderr, MSG_ERR_WRONG_USE, line_num+1, rem_fname(v_table[e.id].name, fname)); exit(EXIT_FAILURE);}

    // check whether id2 was declared
    if (v_table[id2].type == 0) {fprintf(stderr, MSG_ERR_DECL_FIRST, line_num+1, rem_fname(v_table[id2].name, fname)); exit(EXIT_FAILURE);}

    // check whether id2 is a variable
    if (v_table[id2].isar > 0) {fprintf(stderr, MSG_ERR_WRONG_USE, line_num+1, rem_fname(v_table[id2].name, fname)); exit(EXIT_FAILURE);}

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
        add_instr("LOD %s\n", v_table[e.id].name);
        add_instr("SET %s\n", v_table[id2].name );
    }

    // from acc to var
    if (e.id == 0)
    {
        add_instr("SET %s\n", v_table[id2].name);
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
    if (e.id != 0 && v_table[e.id].type == 0) {fprintf(stderr, MSG_ERR_DECL_FIRST, line_num+1, rem_fname(v_table[e.id].name, fname)); exit(EXIT_FAILURE);}

    // check whether et is a variable
    if (e.id != 0 && v_table[e.id].isar > 0) {fprintf(stderr, MSG_ERR_WRONG_USE, line_num+1, rem_fname(v_table[e.id].name, fname)); exit(EXIT_FAILURE);}

    // a complex argument is now valid -- handled below, after the checks

    // ------------------------------------------------------------------------
    // update variable status -------------------------------------------------
    // ------------------------------------------------------------------------

    if (e.id != 0) v_table[e.id].used = 1;

    // ------------------------------------------------------------------------
    // complex argument: principal square root, composed from real ops only ---
    //   z = a + b i ;  r = |z| = sqrt(a² + b²)
    //   sqrt(z) = sqrt((r + a)/2)  +  sign(b) * sqrt((r - a)/2) i
    // ------------------------------------------------------------------------
    if (e.type > 2)
    {
        expr etr, eti;                  // real (a) and imag (b) parts
        const char *A, *B;
        int pre = 0;                    // a pending acc value must be pushed first

        if (e.type == 5)                            // comp constant
        {
            get_cmp_cst(e, &etr, &eti);
            A = v_table[etr.id].name; B = v_table[eti.id].name; pre = acc_ok;
        }
        else if (e.id != 0)                         // comp in memory
        {
            get_cmp_ets(e, &etr, &eti);
            A = v_table[etr.id].name; B = v_table[eti.id].name; pre = acc_ok;
        }
        else                                        // comp in acc (acc=imag, stack=real)
        {
            int ib = exec_id("csqrt_b"), ia = exec_id("csqrt_a");
            add_instr("SET_P %s\n", v_table[ib].name);  // csqrt_b = imag; POP -> acc = real
            add_instr("SET   %s\n", v_table[ia].name);  // csqrt_a = real
            A = v_table[ia].name; B = v_table[ib].name;
        }

        const char *R  = v_table[exec_id("csqrt_r" )].name;   // |z|
        const char *RE = v_table[exec_id("csqrt_re")].name;   // result real part
        const char *IM = v_table[exec_id("csqrt_im")].name;   // result imag part

        // r = |z| = sqrt(a² + b²)
        add_instr("%s %s\n", pre ? "P_LOD" : "LOD", A);  // acc = a (push pending if any)
        add_instr("F_MLT %s\n", A);                      // acc = a²
        add_instr("P_LOD %s\n", B);                      // push a²; acc = b  (P_LOD = PSH+LOD)
        add_instr("F_MLT %s\n", B);                      // acc = b²
        add_instr("SF_ADD\n");                           // acc = a² + b²
        exec_sqrt(expr_make(2, 0));                      // acc = r
        add_instr("SET %s\n", R);

        // re = sqrt((r + a)/2)
        add_instr("LOD %s\n", R);                        // acc = r (peephole drops if redundant)
        add_instr("F_ADD %s\n", A);                      // acc = r + a
        add_instr("F_MLT 0.5\n");                        // acc = (r + a)/2
        exec_sqrt(expr_make(2, 0));                      // acc = sqrt((r+a)/2)
        add_instr("SET %s\n", RE);

        // im = sign(b) * sqrt((r - a)/2)
        add_instr("LOD %s\n", R);                        // acc = r
        add_instr("F_SU1 %s\n", A);                      // acc = r - a   (F_SU1 X = acc - X)
        add_instr("F_MLT 0.5\n");                        // acc = (r - a)/2
        exec_sqrt(expr_make(2, 0));                      // acc = sqrt((r-a)/2)  (>= 0)
        add_instr("F_SGN %s\n", B);                      // acc = magnitude with sign of b
        add_instr("SET %s\n", IM);

        // assemble the complex result: real in acc, imag on the stack
        add_instr("LOD %s\n",   RE);                     // acc = re
        add_instr("P_LOD %s\n", IM);                     // push re; acc = im -> comp in acc

        acc_ok = 1;
        return expr_make(3, 0);
    }

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
        add_instr("%s %s\n", i2f, v_table[e.id].name);
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
        add_instr("%s %s\n", ld, v_table[e.id].name);
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

// exponential (e^x)
expr exec_exp(expr e)
{
    // ------------------------------------------------------------------------
    // consistency check ------------------------------------------------------
    // ------------------------------------------------------------------------

    // check whether et was declared
    if (e.id != 0 && v_table[e.id].type == 0) {fprintf(stderr, MSG_ERR_DECL_FIRST, line_num+1, rem_fname(v_table[e.id].name, fname)); exit(EXIT_FAILURE);}

    // check whether et is a variable
    if (e.id != 0 && v_table[e.id].isar > 0) {fprintf(stderr, MSG_ERR_WRONG_USE, line_num+1, rem_fname(v_table[e.id].name, fname)); exit(EXIT_FAILURE);}

    // a complex argument is now valid -- handled below, after the checks

    // ------------------------------------------------------------------------
    // update variable status -------------------------------------------------
    // ------------------------------------------------------------------------

    if (e.id != 0) v_table[e.id].used = 1;

    // ------------------------------------------------------------------------
    // complex argument: exp(a+bi) = exp(a)*(cos b + i*sin b), composed from the
    // real exp/sin/cos. Mirrors the comp branch of exec_sqrt: same a/b extraction
    // (const/mem/acc) and the same comp-in-acc result assembly (real in acc, imag
    // pushed on the stack).
    // ------------------------------------------------------------------------
    if (e.type > 2)
    {
        expr etr, eti;                  // real (a) and imag (b) parts
        const char *A, *B;
        int pre = 0;                    // a pending acc value must be pushed first

        if (e.type == 5)                            // comp constant
        {
            get_cmp_cst(e, &etr, &eti);
            A = v_table[etr.id].name; B = v_table[eti.id].name; pre = acc_ok;
        }
        else if (e.id != 0)                         // comp in memory
        {
            get_cmp_ets(e, &etr, &eti);
            A = v_table[etr.id].name; B = v_table[eti.id].name; pre = acc_ok;
        }
        else                                        // comp in acc (acc=imag, stack=real)
        {
            int ib = exec_id("cexp_b"), ia = exec_id("cexp_a");
            add_instr("SET_P %s\n", v_table[ib].name);  // cexp_b = imag; POP -> acc = real
            add_instr("SET   %s\n", v_table[ia].name);  // cexp_a = real
            A = v_table[ia].name; B = v_table[ib].name;
        }

        const char *EA = v_table[exec_id("cexp_ea")].name;   // exp(a)
        const char *RE = v_table[exec_id("cexp_re")].name;   // result real part
        const char *IM = v_table[exec_id("cexp_im")].name;   // result imag part

        // ea = exp(a)
        add_instr("%s %s\n", pre ? "P_LOD" : "LOD", A);  // acc = a (push pending if any)
        exec_exp(expr_make(2, 0));                       // acc = exp(a)
        add_instr("SET %s\n", EA);

        // im = exp(a) * sin(b)
        add_instr("LOD %s\n", B);                        // acc = b
        exec_sin(expr_make(2, 0));                       // acc = sin(b)
        add_instr("F_MLT %s\n", EA);                     // acc = exp(a)*sin(b)
        add_instr("SET %s\n", IM);

        // re = exp(a) * cos(b)
        add_instr("LOD %s\n", B);                        // acc = b
        exec_cos(expr_make(2, 0));                       // acc = cos(b)
        add_instr("F_MLT %s\n", EA);                     // acc = exp(a)*cos(b)
        add_instr("SET %s\n", RE);

        // assemble the complex result: real in acc, imag on the stack
        add_instr("LOD %s\n",   RE);                     // acc = re
        add_instr("P_LOD %s\n", IM);                     // push re; acc = im -> comp in acc

        acc_ok = 1;
        return expr_make(3, 0);
    }

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
        add_instr("%s %s\n", i2f, v_table[e.id].name);
        add_instr("CAL float_exp\n");
    }

    // int in acc
    if ((e.type == 1) && (e.id == 0))
    {
        add_instr("I2F\n");
        add_instr("CAL float_exp\n");
    }

    // float in memory
    if ((e.type == 2) && (e.id != 0))
    {
        add_instr("%s %s\n", ld, v_table[e.id].name);
        add_instr("CAL float_exp\n");
    }

    // float in acc
    if ((e.type == 2) && (e.id == 0))
    {
        add_instr("CAL float_exp\n");
    }

    acc_ok = 1;

    return expr_make(2, 0);
}

// natural logarithm (ln x)
expr exec_log(expr e)
{
    // ------------------------------------------------------------------------
    // consistency check ------------------------------------------------------
    // ------------------------------------------------------------------------

    // check whether et was declared
    if (e.id != 0 && v_table[e.id].type == 0) {fprintf(stderr, MSG_ERR_DECL_FIRST, line_num+1, rem_fname(v_table[e.id].name, fname)); exit(EXIT_FAILURE);}

    // check whether et is a variable
    if (e.id != 0 && v_table[e.id].isar > 0) {fprintf(stderr, MSG_ERR_WRONG_USE, line_num+1, rem_fname(v_table[e.id].name, fname)); exit(EXIT_FAILURE);}

    // a complex argument is now valid -- handled below, after the checks

    // ------------------------------------------------------------------------
    // update variable status -------------------------------------------------
    // ------------------------------------------------------------------------

    if (e.id != 0) v_table[e.id].used = 1;

    // ------------------------------------------------------------------------
    // complex argument: log(a+bi) = 1/2*ln(a²+b²) + i*fase(z). Real part is half
    // the log of the squared magnitude (a²+b² inline); imaginary part is the
    // phase, reusing exec_fase (the 4-quadrant atan2). Mirrors exec_exp's a/b
    // extraction and comp-in-acc assembly.
    // ------------------------------------------------------------------------
    if (e.type > 2)
    {
        expr etr, eti;                  // real (a) and imag (b) parts
        const char *A, *B;
        int pre = 0;

        if (e.type == 5)                            // comp constant
        {
            get_cmp_cst(e, &etr, &eti);
            A = v_table[etr.id].name; B = v_table[eti.id].name; pre = acc_ok;
        }
        else if (e.id != 0)                         // comp in memory
        {
            get_cmp_ets(e, &etr, &eti);
            A = v_table[etr.id].name; B = v_table[eti.id].name; pre = acc_ok;
        }
        else                                        // comp in acc (acc=imag, stack=real)
        {
            int ib = exec_id("clog_b"), ia = exec_id("clog_a");
            add_instr("SET_P %s\n", v_table[ib].name);  // clog_b = imag; POP -> acc = real
            add_instr("SET   %s\n", v_table[ia].name);  // clog_a = real
            A = v_table[ia].name; B = v_table[ib].name;
        }

        const char *RE = v_table[exec_id("clog_re")].name;   // result real part
        const char *IM = v_table[exec_id("clog_im")].name;   // result imag part

        // re = 1/2 * ln(a² + b²)
        add_instr("%s %s\n", pre ? "P_LOD" : "LOD", A);  // acc = a (push pending if any)
        add_instr("F_MLT %s\n", A);                      // a²
        add_instr("P_LOD %s\n", B);                      // push a²; acc = b  (P_LOD = PSH+LOD)
        add_instr("F_MLT %s\n", B);                      // b²
        add_instr("SF_ADD\n");                           // a² + b²
        exec_log(expr_make(2, 0));                       // ln(a²+b²)
        add_instr("F_MLT 0.5\n");                        // 1/2 * ln(...)
        add_instr("SET %s\n", RE);

        // im = fase(z) -- rebuild the comp in the acc (real on stack, imag in acc)
        add_instr("LOD %s\n",   A);                      // acc = a (real)
        add_instr("P_LOD %s\n", B);                      // push a; acc = b -> comp in acc
        exec_fase(expr_make(3, 0));                      // arg(z)
        add_instr("SET %s\n", IM);

        // assemble the complex result: real in acc, imag on the stack
        add_instr("LOD %s\n",   RE);
        add_instr("P_LOD %s\n", IM);

        acc_ok = 1;
        return expr_make(3, 0);
    }

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
        add_instr("%s %s\n", i2f, v_table[e.id].name);
        add_instr("CAL float_log\n");
    }

    // int in acc
    if ((e.type == 1) && (e.id == 0))
    {
        add_instr("I2F\n");
        add_instr("CAL float_log\n");
    }

    // float in memory
    if ((e.type == 2) && (e.id != 0))
    {
        add_instr("%s %s\n", ld, v_table[e.id].name);
        add_instr("CAL float_log\n");
    }

    // float in acc
    if ((e.type == 2) && (e.id == 0))
    {
        add_instr("CAL float_log\n");
    }

    acc_ok = 1;

    return expr_make(2, 0);
}

// power x^y ------------------------------------------------------------------
// The exponent's kind picks the strategy (tested here, the usual way):
//   * int constant (isco)  -> square-and-multiply, unrolled at compile time
//                             (exact; pulls in no exp/log macro). A literal is
//                             always >= 0 (a leading '-' is a separate negate).
//   * int variable / acc   -> runtime multiply loop: x folded |y| times, then
//                             a reciprocal when y < 0 (so pow(x,-2) = 0.25 too).
//   * float                -> general identity x^y = exp(y*ln x), needs x > 0.
// The base is widened to float in every path; the result is a float in the acc.
expr exec_pow(expr ex, expr ey)
{
    static int pow_lbl = 0;

    // ------------------------------------------------------------------------
    // consistency check ------------------------------------------------------
    // ------------------------------------------------------------------------

    // check whether the operands were declared
    if (ex.id != 0 && v_table[ex.id].type == 0) {fprintf(stderr, MSG_ERR_DECL_FIRST, line_num+1, rem_fname(v_table[ex.id].name, fname)); exit(EXIT_FAILURE);}
    if (ey.id != 0 && v_table[ey.id].type == 0) {fprintf(stderr, MSG_ERR_DECL_FIRST, line_num+1, rem_fname(v_table[ey.id].name, fname)); exit(EXIT_FAILURE);}

    // check whether the operands are plain variables (not arrays)
    if (ex.id != 0 && v_table[ex.id].isar > 0) {fprintf(stderr, MSG_ERR_WRONG_USE, line_num+1, rem_fname(v_table[ex.id].name, fname)); exit(EXIT_FAILURE);}
    if (ey.id != 0 && v_table[ey.id].isar > 0) {fprintf(stderr, MSG_ERR_WRONG_USE, line_num+1, rem_fname(v_table[ey.id].name, fname)); exit(EXIT_FAILURE);}

    // a complex base or exponent is not supported
    if (ex.type > 2 || ey.type > 2) {fprintf(stderr, MSG_ERR_POW_COMPLEX, line_num+1); exit(EXIT_FAILURE);}

    // ------------------------------------------------------------------------
    // update variable status -------------------------------------------------
    // ------------------------------------------------------------------------

    if (ex.id != 0) v_table[ex.id].used = 1;
    if (ey.id != 0) v_table[ey.id].used = 1;

    // ------------------------------------------------------------------------
    // exponent is an integer CONSTANT -> square-and-multiply -----------------
    // The literal is always non-negative, so the base is the only operand that
    // can be live; widen it to float and unroll the binary exponentiation.
    // ------------------------------------------------------------------------
    if ((ey.type == 1) && (ey.id != 0) && (v_table[ey.id].isco == 1))
    {
        long n = atol(v_table[ey.id].name);

        char ld [10]; if (acc_ok == 0) strcpy(ld ,"LOD"  ); else strcpy(ld ,"P_LOD"  );
        char i2f[10]; if (acc_ok == 0) strcpy(i2f,"I2F_M"); else strcpy(i2f,"P_I2F_M");

        // x^0 = 1 (the base value is simply discarded)
        if (n == 0) { add_instr("LOD 1.0\n"); acc_ok = 1; return expr_make(2, 0); }

        // widen the base to a float in the acc (the four operand shapes)
        if ((ex.type == 1) && (ex.id != 0)) add_instr("%s %s\n", i2f, v_table[ex.id].name);  // int in memory
        if ((ex.type == 1) && (ex.id == 0)) add_instr("I2F\n");                               // int in acc
        if ((ex.type == 2) && (ex.id != 0)) add_instr("%s %s\n", ld , v_table[ex.id].name);  // float in memory
        // float in acc: already there
        acc_ok = 1;

        if (n == 1) return expr_make(2, 0);                  // x^1 = x

        const char *B = v_table[exec_id("pow_b")].name;      // running square (x, x², x⁴, ...)
        const char *R = v_table[exec_id("pow_r")].name;      // running product (the result)
        add_instr("SET %s\n", B);                            // pow_b = base

        int started = 0;                                     // has the result been seeded yet?
        for (long e = n; e > 0; e >>= 1)
        {
            if (e & 1)                                       // bit set -> fold pow_b into the result
            {
                if (!started) { add_instr("LOD %s\n", B); started = 1; }
                else          { add_instr("LOD %s\n", R); add_instr("F_MLT %s\n", B); }
                add_instr("SET %s\n", R);
            }
            if (e >> 1)                                      // more bits remain -> square pow_b
            {
                add_instr("LOD %s\n", B);
                add_instr("F_MLT %s\n", B);
                add_instr("SET %s\n", B);
            }
        }
        add_instr("LOD %s\n", R);                            // result -> acc (peephole drops if redundant)
        acc_ok = 1;
        return expr_make(2, 0);
    }

    // ------------------------------------------------------------------------
    // normalize the base into pow_x and the exponent into pow_y (both float in
    // memory), so the two runtime strategies below read plain operands. The
    // only entangled position is "both in the acc" (base on the stack, exponent
    // on top), spilled with SET_P exactly like exec_fase / the complex-sqrt
    // branch.
    // ------------------------------------------------------------------------
    const char *X = v_table[exec_id("pow_x")].name;          // base     (float)
    const char *Y = v_table[exec_id("pow_y")].name;          // exponent (float)

    if (ey.id == 0)                       // exponent in the acc
    {
        if (ex.id == 0)                   // BOTH in the acc: acc = exponent, stack = base
        {
            add_instr("SET_P %s\n", Y);   // pow_y = exponent ; POP -> acc = base
            if (ex.type == 1) add_instr("I2F\n");
            add_instr("SET %s\n", X);
        }
        else                              // base in memory, exponent in the acc
        {
            add_instr("SET %s\n", Y);
            if (ex.type == 1) add_instr("I2F_M %s\n", v_table[ex.id].name);
            else              add_instr("LOD %s\n",   v_table[ex.id].name);
            add_instr("SET %s\n", X);
        }
        if (ey.type == 1) { add_instr("I2F_M %s\n", Y); add_instr("SET %s\n", Y); }   // widen int exponent
    }
    else                                  // exponent in memory (variable)
    {
        char ld [10]; if (acc_ok == 0) strcpy(ld ,"LOD"  ); else strcpy(ld ,"P_LOD"  );
        char i2f[10]; if (acc_ok == 0) strcpy(i2f,"I2F_M"); else strcpy(i2f,"P_I2F_M");

        if (ex.id == 0)                   // base in the acc
        {
            if (ex.type == 1) add_instr("I2F\n");
            add_instr("SET %s\n", X);
        }
        else                              // base in memory
        {
            if (ex.type == 1) add_instr("%s %s\n", i2f, v_table[ex.id].name);
            else              add_instr("%s %s\n", ld , v_table[ex.id].name);
            add_instr("SET %s\n", X);
        }
        if (ey.type == 1) add_instr("I2F_M %s\n", v_table[ey.id].name);
        else              add_instr("LOD %s\n",   v_table[ey.id].name);
        add_instr("SET %s\n", Y);
    }
    acc_ok = 1;

    // ------------------------------------------------------------------------
    // exponent is an integer VARIABLE / acc -> runtime multiply loop ---------
    //   c = |y| ; r = 1 ; while (c > 0) { r *= x ; c -= 1 } ; if (y < 0) r = 1/r
    // (counter kept as a float so the F_LES / F_ADD test mirrors the macros)
    // ------------------------------------------------------------------------
    if (ey.type == 1)
    {
        const char *R = v_table[exec_id("pow_r")].name;      // running product
        const char *C = v_table[exec_id("pow_c")].name;      // counter |y|
        int m = ++pow_lbl;

        add_instr("LOD 1.0\n");   add_instr("SET %s\n", R);                          // r = 1
        add_instr("F_ABS_M %s\n", Y); add_instr("SET %s\n", C);                      // c = |y|  (F_ABS_M = LOD Y abs)

        add_sinst(0, "@Lpow%d ", m);
        add_instr("LOD %s\n", C);
        add_instr("F_LES 0.5\n");                                                     // c > 0 ? (F_LES true when acc > X)
        add_instr("JIZ Lpow%dend\n", m);
        add_instr("LOD %s\n", R); add_instr("F_MLT %s\n", X); add_instr("SET %s\n", R);   // r *= x
        add_instr("LOD %s\n", C); add_instr("F_ADD -1.0\n");  add_instr("SET %s\n", C);   // c -= 1
        add_instr("JMP Lpow%d\n", m);
        add_sinst(0, "@Lpow%dend ", m);

        add_instr("LOD 0.0\n"); add_instr("F_LES %s\n", Y);                          // y < 0 ?  (0 > y)
        add_instr("JIZ Lpow%dpos\n", m);
        add_instr("LOD %s\n", R); add_instr("F_DIV 1.0\n"); add_instr("SET %s\n", R);     // r = 1/r
        add_sinst(0, "@Lpow%dpos ", m);
        add_instr("LOD %s\n", R);
        acc_ok = 1;
        return expr_make(2, 0);
    }

    // ------------------------------------------------------------------------
    // exponent is a FLOAT -> general identity x^y = exp(y * ln x), needs x > 0
    // ------------------------------------------------------------------------
    add_instr("LOD %s\n", X);
    add_instr("CAL float_log\n");        // ln(x)
    add_instr("F_MLT %s\n", Y);          // y * ln(x)
    add_instr("CAL float_exp\n");        // exp(...)
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
    if (e.id != 0 && v_table[e.id].type == 0) {fprintf(stderr, MSG_ERR_DECL_FIRST, line_num+1, rem_fname(v_table[e.id].name, fname)); exit(EXIT_FAILURE);}

    // check whether et is a variable
    if (e.id != 0 && v_table[e.id].isar > 0) {fprintf(stderr, MSG_ERR_WRONG_USE, line_num+1, rem_fname(v_table[e.id].name, fname)); exit(EXIT_FAILURE);}

    // a complex argument is now valid -- handled below, after the checks

    // ------------------------------------------------------------------------
    // update variable status -------------------------------------------------
    // ------------------------------------------------------------------------

    if (e.id != 0) v_table[e.id].used = 1;

    // ------------------------------------------------------------------------
    // complex argument: atan(a+bi) =
    //   Re = 1/2 * atan2(2a, 1 - a² - b²)                          (uses fase)
    //   Im = 1/4 * ln( (a²+(b+1)²) / (a²+(b-1)²) )                 (uses real log)
    // Mirrors the exec_exp/exec_log comp branch (a/b extraction + comp-in-acc
    // assembly). atan2 is built by reconstructing the comp (1-a²-b²) + (2a)i in
    // the acc and calling exec_fase.
    // ------------------------------------------------------------------------
    if (e.type > 2)
    {
        expr etr, eti;                  // real (a) and imag (b) parts
        const char *A, *B;
        int pre = 0;

        if (e.type == 5)                            // comp constant
        {
            get_cmp_cst(e, &etr, &eti);
            A = v_table[etr.id].name; B = v_table[eti.id].name; pre = acc_ok;
        }
        else if (e.id != 0)                         // comp in memory
        {
            get_cmp_ets(e, &etr, &eti);
            A = v_table[etr.id].name; B = v_table[eti.id].name; pre = acc_ok;
        }
        else                                        // comp in acc (acc=imag, stack=real)
        {
            int ib = exec_id("catan_b"), ia = exec_id("catan_a");
            add_instr("SET_P %s\n", v_table[ib].name);  // catan_b = imag; POP -> acc = real
            add_instr("SET   %s\n", v_table[ia].name);  // catan_a = real
            A = v_table[ia].name; B = v_table[ib].name;
        }

        const char *A2  = v_table[exec_id("catan_a2" )].name;   // a²
        const char *B2  = v_table[exec_id("catan_b2" )].name;   // b²
        const char *WR  = v_table[exec_id("catan_wr" )].name;   // 1 - a² - b²
        const char *WI  = v_table[exec_id("catan_wi" )].name;   // 2a
        const char *BP  = v_table[exec_id("catan_bp" )].name;   // b+1
        const char *BM  = v_table[exec_id("catan_bm" )].name;   // b-1
        const char *NUM = v_table[exec_id("catan_num")].name;   // a²+(b+1)²
        const char *DEN = v_table[exec_id("catan_den")].name;   // a²+(b-1)²
        const char *RE  = v_table[exec_id("catan_re" )].name;   // result real
        const char *IM  = v_table[exec_id("catan_im" )].name;   // result imag

        // a², b²
        add_instr("%s %s\n", pre ? "P_LOD" : "LOD", A);  // acc = a (push pending if any)
        add_instr("F_MLT %s\n", A);                      // a²
        add_instr("SET %s\n", A2);
        add_instr("LOD %s\n", B);
        add_instr("F_MLT %s\n", B);                      // b²
        add_instr("SET %s\n", B2);

        // Re = 1/2 * atan2(2a, 1 - a² - b²)
        add_instr("LOD %s\n", A2);
        add_instr("F_ADD %s\n", B2);                     // a² + b²
        add_instr("F_SU2 1.0\n");                        // 1 - (a²+b²)   (F_SU2 X = X - acc)
        add_instr("SET %s\n", WR);
        add_instr("LOD %s\n", A);
        add_instr("F_MLT 2.0\n");                        // 2a
        add_instr("SET %s\n", WI);
        add_instr("LOD %s\n",   WR);                     // rebuild the comp wr + wi*i in the acc
        add_instr("P_LOD %s\n", WI);                     // acc = wi(imag), stack = wr(real)
        exec_fase(expr_make(3, 0));                      // atan2(2a, 1-a²-b²)
        add_instr("F_MLT 0.5\n");
        add_instr("SET %s\n", RE);

        // Im = 1/4 * ln( (a²+(b+1)²) / (a²+(b-1)²) )
        add_instr("LOD %s\n", B);
        add_instr("F_ADD 1.0\n");                        // b+1
        add_instr("SET %s\n", BP);
        add_instr("F_MLT %s\n", BP);                     // (b+1)²
        add_instr("F_ADD %s\n", A2);                     // a² + (b+1)²
        add_instr("SET %s\n", NUM);
        add_instr("LOD %s\n", B);
        add_instr("F_ADD -1.0\n");                       // b-1
        add_instr("SET %s\n", BM);
        add_instr("F_MLT %s\n", BM);                     // (b-1)²
        add_instr("F_ADD %s\n", A2);                     // a² + (b-1)²
        add_instr("SET %s\n", DEN);
        add_instr("LOD %s\n", DEN);
        add_instr("F_DIV %s\n", NUM);                    // num/den   (F_DIV X = X/acc)
        exec_log(expr_make(2, 0));                       // ln(num/den)
        add_instr("F_MLT 0.25\n");
        add_instr("SET %s\n", IM);

        // assemble the complex result: real in acc, imag on the stack
        add_instr("LOD %s\n",   RE);
        add_instr("P_LOD %s\n", IM);

        acc_ok = 1;
        return expr_make(3, 0);
    }

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
        add_instr("%s %s\n", i2f, v_table[e.id].name);
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
        add_instr("%s %s\n", ld, v_table[e.id].name);
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
    if (e.id != 0 && v_table[e.id].type == 0) {fprintf(stderr, MSG_ERR_DECL_FIRST, line_num+1, rem_fname(v_table[e.id].name, fname)); exit(EXIT_FAILURE);}

    // check whether et is a variable
    if (e.id != 0 && v_table[e.id].isar > 0) {fprintf(stderr, MSG_ERR_WRONG_USE, line_num+1, rem_fname(v_table[e.id].name, fname)); exit(EXIT_FAILURE);}

    // a complex argument is now valid -- handled below, after the checks

    // ------------------------------------------------------------------------
    // update variable status -------------------------------------------------
    // ------------------------------------------------------------------------

    if (e.id != 0) v_table[e.id].used = 1;

    // ------------------------------------------------------------------------
    // complex argument: sin(a+bi) = sin a*cosh b + i*cos a*sinh b. Fast form --
    // ONE exp gives e^b; e^-b = 1/e^b (reciprocal), so cosh/sinh come from a
    // single exponential instead of four exec_cosh/exec_sinh calls. Mirrors the
    // exec_exp/exec_sqrt comp branch (a/b extraction + comp-in-acc assembly).
    // ------------------------------------------------------------------------
    if (e.type > 2)
    {
        expr etr, eti;                  // real (a) and imag (b) parts
        const char *A, *B;
        int pre = 0;

        if (e.type == 5)                            // comp constant
        {
            get_cmp_cst(e, &etr, &eti);
            A = v_table[etr.id].name; B = v_table[eti.id].name; pre = acc_ok;
        }
        else if (e.id != 0)                         // comp in memory
        {
            get_cmp_ets(e, &etr, &eti);
            A = v_table[etr.id].name; B = v_table[eti.id].name; pre = acc_ok;
        }
        else                                        // comp in acc (acc=imag, stack=real)
        {
            int ib = exec_id("csin_b"), ia = exec_id("csin_a");
            add_instr("SET_P %s\n", v_table[ib].name);  // csin_b = imag; POP -> acc = real
            add_instr("SET   %s\n", v_table[ia].name);  // csin_a = real
            A = v_table[ia].name; B = v_table[ib].name;
        }

        const char *EB  = v_table[exec_id("csin_eb" )].name;   // e^b
        const char *EMB = v_table[exec_id("csin_emb")].name;   // e^-b
        const char *CHB = v_table[exec_id("csin_chb")].name;   // cosh b
        const char *SHB = v_table[exec_id("csin_shb")].name;   // sinh b
        const char *SA  = v_table[exec_id("csin_sa" )].name;   // sin a
        const char *CA  = v_table[exec_id("csin_ca" )].name;   // cos a
        const char *RE  = v_table[exec_id("csin_re" )].name;   // result real
        const char *IM  = v_table[exec_id("csin_im" )].name;   // result imag

        // e^b (single exponential), e^-b = 1/e^b
        add_instr("%s %s\n", pre ? "P_LOD" : "LOD", B);  // acc = b (push pending if any)
        exec_exp(expr_make(2, 0));                       // acc = e^b
        add_instr("SET %s\n", EB);
        add_instr("F_DIV 1.0\n");                        // 1.0/e^b = e^-b  (F_DIV X = X/acc)
        add_instr("SET %s\n", EMB);

        // cosh b = (e^b + e^-b)/2 ; sinh b = (e^b - e^-b)/2
        add_instr("LOD %s\n", EB); add_instr("F_ADD %s\n", EMB); add_instr("F_MLT 0.5\n"); add_instr("SET %s\n", CHB);
        add_instr("LOD %s\n", EB); add_instr("F_SU1 %s\n", EMB); add_instr("F_MLT 0.5\n"); add_instr("SET %s\n", SHB);

        // sin a, cos a
        add_instr("LOD %s\n", A); exec_sin(expr_make(2, 0)); add_instr("SET %s\n", SA);
        add_instr("LOD %s\n", A); exec_cos(expr_make(2, 0)); add_instr("SET %s\n", CA);

        // re = sin a * cosh b ; im = cos a * sinh b
        add_instr("LOD %s\n", SA); add_instr("F_MLT %s\n", CHB); add_instr("SET %s\n", RE);
        add_instr("LOD %s\n", CA); add_instr("F_MLT %s\n", SHB); add_instr("SET %s\n", IM);

        // assemble the complex result: real in acc, imag on the stack
        add_instr("LOD %s\n",   RE);
        add_instr("P_LOD %s\n", IM);

        acc_ok = 1;
        return expr_make(3, 0);
    }

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
        add_instr("%s %s\n", i2f, v_table[e.id].name);
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
        add_instr("%s %s\n", ld, v_table[e.id].name);
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

// tangent
expr exec_tan(expr e)
{
    // ------------------------------------------------------------------------
    // consistency check ------------------------------------------------------
    // ------------------------------------------------------------------------

    // check whether et was declared
    if (e.id != 0 && v_table[e.id].type == 0) {fprintf(stderr, MSG_ERR_DECL_FIRST, line_num+1, rem_fname(v_table[e.id].name, fname)); exit(EXIT_FAILURE);}

    // check whether et is a variable
    if (e.id != 0 && v_table[e.id].isar > 0) {fprintf(stderr, MSG_ERR_WRONG_USE, line_num+1, rem_fname(v_table[e.id].name, fname)); exit(EXIT_FAILURE);}

    // a complex argument is now valid -- handled below, after the checks

    // ------------------------------------------------------------------------
    // update variable status -------------------------------------------------
    // ------------------------------------------------------------------------

    if (e.id != 0) v_table[e.id].used = 1;

    // ------------------------------------------------------------------------
    // complex argument: tan(a+bi) = (sin 2a + i*sinh 2b) / (cos 2a + cosh 2b).
    // The denominator D = cos 2a + cosh 2b is REAL, so no complex division is
    // needed. cosh/sinh 2b come from a single exp(2b) + reciprocal (fast form).
    // Mirrors the exec_exp/exec_log comp branch (a/b extraction + comp assembly).
    // ------------------------------------------------------------------------
    if (e.type > 2)
    {
        expr etr, eti;
        const char *A, *B;
        int pre = 0;

        if (e.type == 5)                            // comp constant
        {
            get_cmp_cst(e, &etr, &eti);
            A = v_table[etr.id].name; B = v_table[eti.id].name; pre = acc_ok;
        }
        else if (e.id != 0)                         // comp in memory
        {
            get_cmp_ets(e, &etr, &eti);
            A = v_table[etr.id].name; B = v_table[eti.id].name; pre = acc_ok;
        }
        else                                        // comp in acc (acc=imag, stack=real)
        {
            int ib = exec_id("ctan_b"), ia = exec_id("ctan_a");
            add_instr("SET_P %s\n", v_table[ib].name);  // ctan_b = imag; POP -> acc = real
            add_instr("SET   %s\n", v_table[ia].name);  // ctan_a = real
            A = v_table[ia].name; B = v_table[ib].name;
        }

        const char *TA  = v_table[exec_id("ctan_ta" )].name;   // 2a
        const char *TB  = v_table[exec_id("ctan_tb" )].name;   // 2b
        const char *S2A = v_table[exec_id("ctan_s2a")].name;   // sin 2a
        const char *C2A = v_table[exec_id("ctan_c2a")].name;   // cos 2a
        const char *EB  = v_table[exec_id("ctan_eb" )].name;   // e^(2b)
        const char *EMB = v_table[exec_id("ctan_emb")].name;   // e^(-2b)
        const char *CHB = v_table[exec_id("ctan_chb")].name;   // cosh 2b
        const char *SHB = v_table[exec_id("ctan_shb")].name;   // sinh 2b
        const char *D   = v_table[exec_id("ctan_d"  )].name;   // denominator
        const char *RE  = v_table[exec_id("ctan_re" )].name;   // result real
        const char *IM  = v_table[exec_id("ctan_im" )].name;   // result imag

        // 2a, 2b
        add_instr("%s %s\n", pre ? "P_LOD" : "LOD", A);  // acc = a (push pending if any)
        add_instr("F_MLT 2.0\n");  add_instr("SET %s\n", TA);
        add_instr("LOD %s\n", B);
        add_instr("F_MLT 2.0\n");  add_instr("SET %s\n", TB);

        // sin 2a, cos 2a
        add_instr("LOD %s\n", TA); exec_sin(expr_make(2, 0)); add_instr("SET %s\n", S2A);
        add_instr("LOD %s\n", TA); exec_cos(expr_make(2, 0)); add_instr("SET %s\n", C2A);

        // e^(2b), e^(-2b) = 1/e^(2b); cosh 2b, sinh 2b
        add_instr("LOD %s\n", TB); exec_exp(expr_make(2, 0)); add_instr("SET %s\n", EB);
        add_instr("F_DIV 1.0\n");  add_instr("SET %s\n", EMB);                         // e^(-2b)
        add_instr("LOD %s\n", EB); add_instr("F_ADD %s\n", EMB); add_instr("F_MLT 0.5\n"); add_instr("SET %s\n", CHB);
        add_instr("LOD %s\n", EB); add_instr("F_SU1 %s\n", EMB); add_instr("F_MLT 0.5\n"); add_instr("SET %s\n", SHB);

        // D = cos 2a + cosh 2b
        add_instr("LOD %s\n", C2A); add_instr("F_ADD %s\n", CHB); add_instr("SET %s\n", D);

        // Re = sin 2a / D ; Im = sinh 2b / D   (F_DIV X = X/acc)
        add_instr("LOD %s\n", D); add_instr("F_DIV %s\n", S2A); add_instr("SET %s\n", RE);
        add_instr("LOD %s\n", D); add_instr("F_DIV %s\n", SHB); add_instr("SET %s\n", IM);

        // assemble the complex result: real in acc, imag on the stack
        add_instr("LOD %s\n",   RE);
        add_instr("P_LOD %s\n", IM);

        acc_ok = 1;
        return expr_make(3, 0);
    }

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
        add_instr("%s %s\n", i2f, v_table[e.id].name);
        add_instr("CAL float_tan\n");
    }

    // int in acc
    if ((e.type == 1) && (e.id == 0))
    {
        add_instr("I2F\n");
        add_instr("CAL float_tan\n");
    }

    // float in memory
    if ((e.type == 2) && (e.id != 0))
    {
        add_instr("%s %s\n", ld, v_table[e.id].name);
        add_instr("CAL float_tan\n");
    }

    // float in acc
    if ((e.type == 2) && (e.id == 0))
    {
        add_instr("CAL float_tan\n");
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
    if (e.id != 0 && v_table[e.id].type == 0) {fprintf(stderr, MSG_ERR_DECL_FIRST, line_num+1, rem_fname(v_table[e.id].name, fname)); exit(EXIT_FAILURE);}

    // check whether et is a variable
    if (e.id != 0 && v_table[e.id].isar > 0) {fprintf(stderr, MSG_ERR_WRONG_USE, line_num+1, rem_fname(v_table[e.id].name, fname)); exit(EXIT_FAILURE);}

    // a complex argument is now valid -- handled below, after the checks

    // ------------------------------------------------------------------------
    // update variable status -------------------------------------------------
    // ------------------------------------------------------------------------

    if (e.id != 0) v_table[e.id].used = 1;

    // ------------------------------------------------------------------------
    // complex argument: cos(a+bi) = cos a*cosh b - i*sin a*sinh b. Fast form --
    // ONE exp gives e^b; e^-b = 1/e^b (reciprocal), so cosh/sinh come from a
    // single exponential. Mirrors the exec_sin comp branch; only the real/imag
    // assignment differs (re uses cos a, im is negated).
    // ------------------------------------------------------------------------
    if (e.type > 2)
    {
        expr etr, eti;                  // real (a) and imag (b) parts
        const char *A, *B;
        int pre = 0;

        if (e.type == 5)                            // comp constant
        {
            get_cmp_cst(e, &etr, &eti);
            A = v_table[etr.id].name; B = v_table[eti.id].name; pre = acc_ok;
        }
        else if (e.id != 0)                         // comp in memory
        {
            get_cmp_ets(e, &etr, &eti);
            A = v_table[etr.id].name; B = v_table[eti.id].name; pre = acc_ok;
        }
        else                                        // comp in acc (acc=imag, stack=real)
        {
            int ib = exec_id("ccos_b"), ia = exec_id("ccos_a");
            add_instr("SET_P %s\n", v_table[ib].name);  // ccos_b = imag; POP -> acc = real
            add_instr("SET   %s\n", v_table[ia].name);  // ccos_a = real
            A = v_table[ia].name; B = v_table[ib].name;
        }

        const char *EB  = v_table[exec_id("ccos_eb" )].name;   // e^b
        const char *EMB = v_table[exec_id("ccos_emb")].name;   // e^-b
        const char *CHB = v_table[exec_id("ccos_chb")].name;   // cosh b
        const char *SHB = v_table[exec_id("ccos_shb")].name;   // sinh b
        const char *SA  = v_table[exec_id("ccos_sa" )].name;   // sin a
        const char *CA  = v_table[exec_id("ccos_ca" )].name;   // cos a
        const char *RE  = v_table[exec_id("ccos_re" )].name;   // result real
        const char *IM  = v_table[exec_id("ccos_im" )].name;   // result imag

        // e^b (single exponential), e^-b = 1/e^b
        add_instr("%s %s\n", pre ? "P_LOD" : "LOD", B);  // acc = b (push pending if any)
        exec_exp(expr_make(2, 0));                       // acc = e^b
        add_instr("SET %s\n", EB);
        add_instr("F_DIV 1.0\n");                        // 1.0/e^b = e^-b  (F_DIV X = X/acc)
        add_instr("SET %s\n", EMB);

        // cosh b = (e^b + e^-b)/2 ; sinh b = (e^b - e^-b)/2
        add_instr("LOD %s\n", EB); add_instr("F_ADD %s\n", EMB); add_instr("F_MLT 0.5\n"); add_instr("SET %s\n", CHB);
        add_instr("LOD %s\n", EB); add_instr("F_SU1 %s\n", EMB); add_instr("F_MLT 0.5\n"); add_instr("SET %s\n", SHB);

        // sin a, cos a
        add_instr("LOD %s\n", A); exec_sin(expr_make(2, 0)); add_instr("SET %s\n", SA);
        add_instr("LOD %s\n", A); exec_cos(expr_make(2, 0)); add_instr("SET %s\n", CA);

        // re = cos a * cosh b ; im = -(sin a * sinh b)
        add_instr("LOD %s\n", CA); add_instr("F_MLT %s\n", CHB); add_instr("SET %s\n", RE);
        add_instr("LOD %s\n", SA); add_instr("F_MLT %s\n", SHB); add_instr("F_NEG\n"); add_instr("SET %s\n", IM);

        // assemble the complex result: real in acc, imag on the stack
        add_instr("LOD %s\n",   RE);
        add_instr("P_LOD %s\n", IM);

        acc_ok = 1;
        return expr_make(3, 0);
    }

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
        add_instr("%s %s\n", i2f, v_table[e.id].name);
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
        add_instr("%s %s\n", ld, v_table[e.id].name);
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
// hyperbolic functions -------------------------------------------------------
// cosh(x)=(e^x+e^-x)/2, sinh(x)=(e^x-e^-x)/2, tanh(x)=(e^2x-1)/(e^2x+1). Pure
// compositions of the real exponential (CAL float_exp pulls in float_exp.asm),
// no new hardware. All take int/float and return a float; comp is rejected.
// ----------------------------------------------------------------------------

// hyperbolic cosine
expr exec_cosh(expr e)
{
    // ------------------------------------------------------------------------
    // consistency check ------------------------------------------------------
    // ------------------------------------------------------------------------

    if (e.id != 0 && v_table[e.id].type == 0) {fprintf(stderr, MSG_ERR_DECL_FIRST, line_num+1, rem_fname(v_table[e.id].name, fname)); exit(EXIT_FAILURE);}
    if (e.id != 0 && v_table[e.id].isar > 0) {fprintf(stderr, MSG_ERR_WRONG_USE, line_num+1, rem_fname(v_table[e.id].name, fname)); exit(EXIT_FAILURE);}
    if (e.type > 2) {fprintf (stderr, MSG_ERR_SQRT_COMPLEX, line_num+1); exit(EXIT_FAILURE);}

    if (e.id != 0) v_table[e.id].used = 1;

    // ------------------------------------------------------------------------
    // prepare local variables ------------------------------------------------
    // ------------------------------------------------------------------------

    char ld [10]; if (acc_ok == 0) strcpy(ld ,"LOD"  ); else strcpy(ld ,"P_LOD"  );
    char i2f[10]; if (acc_ok == 0) strcpy(i2f,"I2F_M"); else strcpy(i2f,"P_I2F_M");

    // ------------------------------------------------------------------------
    // execute -- bring x into the acc as a float (the four operand shapes) ----
    // ------------------------------------------------------------------------

    // int in memory
    if ((e.type == 1) && (e.id != 0)) add_instr("%s %s\n", i2f, v_table[e.id].name);

    // int in acc
    if ((e.type == 1) && (e.id == 0)) add_instr("I2F\n");

    // float in memory
    if ((e.type == 2) && (e.id != 0)) add_instr("%s %s\n", ld , v_table[e.id].name);

    // float in acc: already there

    acc_ok = 1;

    // cosh(x) = (exp(x) + exp(-x)) / 2
    const char *X = v_table[exec_id("cosh_x")].name;
    const char *T = v_table[exec_id("cosh_t")].name;

    add_instr("SET %s\n", X);              // stash x
    add_instr("CAL float_exp\n");          // exp(x)   (x was in acc)
    add_instr("SET %s\n", T);              // T = exp(x)
    add_instr("F_NEG_M %s\n", X);          // -x in one op (F_NEG_M = LOD X negated)
    add_instr("CAL float_exp\n");          // exp(-x)
    add_instr("F_ADD %s\n", T);            // exp(-x) + exp(x)
    add_instr("F_MLT 0.5\n");              // / 2

    acc_ok = 1;
    return expr_make(2, 0);
}

// hyperbolic sine
expr exec_sinh(expr e)
{
    // ------------------------------------------------------------------------
    // consistency check ------------------------------------------------------
    // ------------------------------------------------------------------------

    if (e.id != 0 && v_table[e.id].type == 0) {fprintf(stderr, MSG_ERR_DECL_FIRST, line_num+1, rem_fname(v_table[e.id].name, fname)); exit(EXIT_FAILURE);}
    if (e.id != 0 && v_table[e.id].isar > 0) {fprintf(stderr, MSG_ERR_WRONG_USE, line_num+1, rem_fname(v_table[e.id].name, fname)); exit(EXIT_FAILURE);}
    if (e.type > 2) {fprintf (stderr, MSG_ERR_SQRT_COMPLEX, line_num+1); exit(EXIT_FAILURE);}

    if (e.id != 0) v_table[e.id].used = 1;

    // ------------------------------------------------------------------------
    // prepare local variables ------------------------------------------------
    // ------------------------------------------------------------------------

    char ld [10]; if (acc_ok == 0) strcpy(ld ,"LOD"  ); else strcpy(ld ,"P_LOD"  );
    char i2f[10]; if (acc_ok == 0) strcpy(i2f,"I2F_M"); else strcpy(i2f,"P_I2F_M");

    // ------------------------------------------------------------------------
    // execute -- bring x into the acc as a float (the four operand shapes) ----
    // ------------------------------------------------------------------------

    // int in memory
    if ((e.type == 1) && (e.id != 0)) add_instr("%s %s\n", i2f, v_table[e.id].name);

    // int in acc
    if ((e.type == 1) && (e.id == 0)) add_instr("I2F\n");

    // float in memory
    if ((e.type == 2) && (e.id != 0)) add_instr("%s %s\n", ld , v_table[e.id].name);

    // float in acc: already there

    acc_ok = 1;

    // sinh(x) = (exp(x) - exp(-x)) / 2
    const char *X = v_table[exec_id("sinh_x")].name;
    const char *T = v_table[exec_id("sinh_t")].name;

    add_instr("SET %s\n", X);              // stash x
    add_instr("CAL float_exp\n");          // exp(x)
    add_instr("SET %s\n", T);              // T = exp(x)
    add_instr("F_NEG_M %s\n", X);          // -x in one op (F_NEG_M = LOD X negated)
    add_instr("CAL float_exp\n");          // exp(-x)  (acc = exp(-x))
    add_instr("F_SU2 %s\n", T);            // T - acc = exp(x) - exp(-x)   (F_SU2 X = X - acc)
    add_instr("F_MLT 0.5\n");              // / 2

    acc_ok = 1;
    return expr_make(2, 0);
}

// hyperbolic tangent
expr exec_tanh(expr e)
{
    // ------------------------------------------------------------------------
    // consistency check ------------------------------------------------------
    // ------------------------------------------------------------------------

    if (e.id != 0 && v_table[e.id].type == 0) {fprintf(stderr, MSG_ERR_DECL_FIRST, line_num+1, rem_fname(v_table[e.id].name, fname)); exit(EXIT_FAILURE);}
    if (e.id != 0 && v_table[e.id].isar > 0) {fprintf(stderr, MSG_ERR_WRONG_USE, line_num+1, rem_fname(v_table[e.id].name, fname)); exit(EXIT_FAILURE);}
    if (e.type > 2) {fprintf (stderr, MSG_ERR_SQRT_COMPLEX, line_num+1); exit(EXIT_FAILURE);}

    if (e.id != 0) v_table[e.id].used = 1;

    // ------------------------------------------------------------------------
    // prepare local variables ------------------------------------------------
    // ------------------------------------------------------------------------

    char ld [10]; if (acc_ok == 0) strcpy(ld ,"LOD"  ); else strcpy(ld ,"P_LOD"  );
    char i2f[10]; if (acc_ok == 0) strcpy(i2f,"I2F_M"); else strcpy(i2f,"P_I2F_M");

    // ------------------------------------------------------------------------
    // execute -- bring x into the acc as a float (the four operand shapes) ----
    // ------------------------------------------------------------------------

    // int in memory
    if ((e.type == 1) && (e.id != 0)) add_instr("%s %s\n", i2f, v_table[e.id].name);

    // int in acc
    if ((e.type == 1) && (e.id == 0)) add_instr("I2F\n");

    // float in memory
    if ((e.type == 2) && (e.id != 0)) add_instr("%s %s\n", ld , v_table[e.id].name);

    // float in acc: already there

    acc_ok = 1;

    // tanh(x) = (exp(2x) - 1) / (exp(2x) + 1)   -- one exponential
    const char *E = v_table[exec_id("tanh_e")].name;     // exp(2x)
    const char *N = v_table[exec_id("tanh_n")].name;     // numerator

    add_instr("F_MLT 2.0\n");              // 2x
    add_instr("CAL float_exp\n");          // exp(2x)
    add_instr("SET %s\n", E);
    add_instr("F_ADD -1.0\n");             // exp(2x) - 1
    add_instr("SET %s\n", N);              // N = numerator
    add_instr("LOD %s\n", E);
    add_instr("F_ADD 1.0\n");              // exp(2x) + 1  = denominator (acc)
    add_instr("F_DIV %s\n", N);            // N / acc = num/den   (F_DIV X = X/acc)

    acc_ok = 1;
    return expr_make(2, 0);
}

// ----------------------------------------------------------------------------
// rounding functions ---------------------------------------------------------
// F2I truncates toward zero, so I2F(F2I(x)) = trunc(x). floor/ceil nudge that
// by one in the single direction where trunc disagrees with them; round uses
// the sign-half bias. All three take int/float and return an integral float.
// ----------------------------------------------------------------------------

// largest integral float <= x
expr exec_floor(expr e)
{
    static int floor_lbl = 0;

    // ------------------------------------------------------------------------
    // consistency check ------------------------------------------------------
    // ------------------------------------------------------------------------

    if (e.id != 0 && v_table[e.id].type == 0) {fprintf(stderr, MSG_ERR_DECL_FIRST, line_num+1, rem_fname(v_table[e.id].name, fname)); exit(EXIT_FAILURE);}
    if (e.id != 0 && v_table[e.id].isar > 0) {fprintf(stderr, MSG_ERR_WRONG_USE, line_num+1, rem_fname(v_table[e.id].name, fname)); exit(EXIT_FAILURE);}
    if (e.type > 2) {fprintf (stderr, MSG_ERR_SQRT_COMPLEX, line_num+1); exit(EXIT_FAILURE);}

    if (e.id != 0) v_table[e.id].used = 1;

    // ------------------------------------------------------------------------
    // prepare local variables ------------------------------------------------
    // ------------------------------------------------------------------------

    char ld [10]; if (acc_ok == 0) strcpy(ld ,"LOD"  ); else strcpy(ld ,"P_LOD"  );
    char i2f[10]; if (acc_ok == 0) strcpy(i2f,"I2F_M"); else strcpy(i2f,"P_I2F_M");

    // ------------------------------------------------------------------------
    // execute -- bring x into the acc as a float (the four operand shapes) ----
    // ------------------------------------------------------------------------

    // int in memory
    if ((e.type == 1) && (e.id != 0)) add_instr("%s %s\n", i2f, v_table[e.id].name);

    // int in acc
    if ((e.type == 1) && (e.id == 0)) add_instr("I2F\n");

    // float in memory
    if ((e.type == 2) && (e.id != 0)) add_instr("%s %s\n", ld , v_table[e.id].name);

    // float in acc: already there

    acc_ok = 1;

    // floor(x) = trunc(x); if (trunc > x) trunc -= 1  (only for negative non-integers)
    const char *X = v_table[exec_id("floor_x")].name;
    const char *T = v_table[exec_id("floor_t")].name;
    int n = ++floor_lbl;

    add_instr("SET %s\n", X);              // stash x
    add_instr("F2I\n");                    // trunc toward zero (int)
    add_instr("I2F\n");                    // back to float
    add_instr("SET %s\n", T);              // T = trunc(x)   (acc still = T)
    add_instr("F_LES %s\n", X);            // trunc > x ?  [F_LES true when acc > X]
    add_instr("JIZ Lflr%dend\n", n);
    add_instr("LOD %s\n", T);
    add_instr("F_ADD -1.0\n");
    add_instr("SET %s\n", T);
    add_sinst(0, "@Lflr%dend ", n);
    add_instr("LOD %s\n", T);

    acc_ok = 1;
    return expr_make(2, 0);
}

// smallest integral float >= x
expr exec_ceil(expr e)
{
    static int ceil_lbl = 0;

    // ------------------------------------------------------------------------
    // consistency check ------------------------------------------------------
    // ------------------------------------------------------------------------

    if (e.id != 0 && v_table[e.id].type == 0) {fprintf(stderr, MSG_ERR_DECL_FIRST, line_num+1, rem_fname(v_table[e.id].name, fname)); exit(EXIT_FAILURE);}
    if (e.id != 0 && v_table[e.id].isar > 0) {fprintf(stderr, MSG_ERR_WRONG_USE, line_num+1, rem_fname(v_table[e.id].name, fname)); exit(EXIT_FAILURE);}
    if (e.type > 2) {fprintf (stderr, MSG_ERR_SQRT_COMPLEX, line_num+1); exit(EXIT_FAILURE);}

    if (e.id != 0) v_table[e.id].used = 1;

    // ------------------------------------------------------------------------
    // prepare local variables ------------------------------------------------
    // ------------------------------------------------------------------------

    char ld [10]; if (acc_ok == 0) strcpy(ld ,"LOD"  ); else strcpy(ld ,"P_LOD"  );
    char i2f[10]; if (acc_ok == 0) strcpy(i2f,"I2F_M"); else strcpy(i2f,"P_I2F_M");

    // ------------------------------------------------------------------------
    // execute -- bring x into the acc as a float (the four operand shapes) ----
    // ------------------------------------------------------------------------

    // int in memory
    if ((e.type == 1) && (e.id != 0)) add_instr("%s %s\n", i2f, v_table[e.id].name);

    // int in acc
    if ((e.type == 1) && (e.id == 0)) add_instr("I2F\n");

    // float in memory
    if ((e.type == 2) && (e.id != 0)) add_instr("%s %s\n", ld , v_table[e.id].name);

    // float in acc: already there

    acc_ok = 1;

    // ceil(x) = trunc(x); if (trunc < x) trunc += 1  (only for positive non-integers)
    const char *X = v_table[exec_id("ceil_x")].name;
    const char *T = v_table[exec_id("ceil_t")].name;
    int n = ++ceil_lbl;

    add_instr("SET %s\n", X);              // stash x
    add_instr("F2I\n");                    // trunc toward zero (int)
    add_instr("I2F\n");                    // back to float
    add_instr("SET %s\n", T);              // T = trunc(x)
    add_instr("LOD %s\n", X);              // x back in the acc for the compare
    add_instr("F_LES %s\n", T);            // x > trunc ?  (i.e. trunc < x)
    add_instr("JIZ Lcel%dend\n", n);
    add_instr("LOD %s\n", T);
    add_instr("F_ADD 1.0\n");
    add_instr("SET %s\n", T);
    add_sinst(0, "@Lcel%dend ", n);
    add_instr("LOD %s\n", T);

    acc_ok = 1;
    return expr_make(2, 0);
}

// nearest integral float, ties away from zero
expr exec_round(expr e)
{
    // ------------------------------------------------------------------------
    // consistency check ------------------------------------------------------
    // ------------------------------------------------------------------------

    if (e.id != 0 && v_table[e.id].type == 0) {fprintf(stderr, MSG_ERR_DECL_FIRST, line_num+1, rem_fname(v_table[e.id].name, fname)); exit(EXIT_FAILURE);}
    if (e.id != 0 && v_table[e.id].isar > 0) {fprintf(stderr, MSG_ERR_WRONG_USE, line_num+1, rem_fname(v_table[e.id].name, fname)); exit(EXIT_FAILURE);}
    if (e.type > 2) {fprintf (stderr, MSG_ERR_SQRT_COMPLEX, line_num+1); exit(EXIT_FAILURE);}

    if (e.id != 0) v_table[e.id].used = 1;

    // ------------------------------------------------------------------------
    // prepare local variables ------------------------------------------------
    // ------------------------------------------------------------------------

    char ld [10]; if (acc_ok == 0) strcpy(ld ,"LOD"  ); else strcpy(ld ,"P_LOD"  );
    char i2f[10]; if (acc_ok == 0) strcpy(i2f,"I2F_M"); else strcpy(i2f,"P_I2F_M");

    // ------------------------------------------------------------------------
    // execute -- bring x into the acc as a float (the four operand shapes) ----
    // ------------------------------------------------------------------------

    // int in memory
    if ((e.type == 1) && (e.id != 0)) add_instr("%s %s\n", i2f, v_table[e.id].name);

    // int in acc
    if ((e.type == 1) && (e.id == 0)) add_instr("I2F\n");

    // float in memory
    if ((e.type == 2) && (e.id != 0)) add_instr("%s %s\n", ld , v_table[e.id].name);

    // float in acc: already there

    acc_ok = 1;

    // round(x) = trunc(x + copysign(0.5, x))  -> ties away from zero
    const char *X = v_table[exec_id("round_x")].name;

    add_instr("SET %s\n", X);              // stash x
    add_instr("LOD 0.5\n");
    add_instr("F_SGN %s\n", X);            // copysign(0.5, x)
    add_instr("F_ADD %s\n", X);            // x + copysign(0.5, x)
    add_instr("F2I\n");                    // truncate toward zero
    add_instr("I2F\n");                    // back to float

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
    if (e.id != 0 && v_table[e.id].type == 0) {fprintf(stderr, MSG_ERR_DECL_FIRST, line_num+1, rem_fname(v_table[e.id].name, fname)); exit(EXIT_FAILURE);}

    // check whether et is a variable
    if (e.id != 0 && v_table[e.id].isar > 0) {fprintf(stderr, MSG_ERR_WRONG_USE, line_num+1, rem_fname(v_table[e.id].name, fname)); exit(EXIT_FAILURE);}

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

        add_instr("%s %s\n", ld, v_table[et_r.id].name);
    }

    // comp in memory
    if ((e.type == 3) && (e.id != 0))
    {
        add_instr("%s %s\n", ld, v_table[e.id].name);
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
    if (e.id != 0 && v_table[e.id].type == 0) {fprintf(stderr, MSG_ERR_DECL_FIRST, line_num+1, rem_fname(v_table[e.id].name, fname)); exit(EXIT_FAILURE);}

    // check whether et is a variable
    if (e.id != 0 && v_table[e.id].isar > 0) {fprintf(stderr, MSG_ERR_WRONG_USE, line_num+1, rem_fname(v_table[e.id].name, fname)); exit(EXIT_FAILURE);}

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

        add_instr("%s %s\n", ld, v_table[et_i.id].name);
    }

    // comp in memory
    if ((e.type == 3) && (e.id != 0))
    {
        expr et_r, et_i;
        get_cmp_ets(e,&et_r,&et_i);

        add_instr("%s %s\n", ld, v_table[et_i.id].name);
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
    if (e.id != 0 && v_table[e.id].type == 0) {fprintf(stderr, MSG_ERR_DECL_FIRST, line_num+1, rem_fname(v_table[e.id].name, fname)); exit(EXIT_FAILURE);}

    // check whether et is a variable
    if (e.id != 0 && v_table[e.id].isar > 0) {fprintf(stderr, MSG_ERR_WRONG_USE, line_num+1, rem_fname(v_table[e.id].name, fname)); exit(EXIT_FAILURE);}

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
        emit_sq_sum(etr, eti);   // c²+d² (acc-aware: squares the live half first)
    }

    // when it is in memory --------------------------------------------------
    if ((e.type == 3) && (e.id != 0))
    {
        get_cmp_ets(e,&etr,&eti);     // get the et of each variable
        emit_sq_sum(etr, eti);   // c²+d² (acc-aware: squares the live half first)
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
    if (e.id != 0 && v_table[e.id].type == 0) {fprintf(stderr, MSG_ERR_DECL_FIRST, line_num+1, rem_fname(v_table[e.id].name, fname)); exit(EXIT_FAILURE);}

    // check whether et is a variable
    if (e.id != 0 && v_table[e.id].isar > 0) {fprintf(stderr, MSG_ERR_WRONG_USE, line_num+1, rem_fname(v_table[e.id].name, fname)); exit(EXIT_FAILURE);}

    // check whether it is comp
    if (e.type < 3) {fprintf (stderr, MSG_ERR_FASE_ARG_COMP, line_num+1); exit(EXIT_FAILURE);}

    // ------------------------------------------------------------------------
    // update variable status -------------------------------------------------
    // ------------------------------------------------------------------------

    if (e.id != 0) v_table[e.id].used = 1;

    // ------------------------------------------------------------------------
    // prepare local variables ------------------------------------------------
    // ------------------------------------------------------------------------
    
    // ------------------------------------------------------------------------
    // execute -- proper atan2(imag, real): correct in all four quadrants and on
    // the axes, and never divides when real == 0.
    // ------------------------------------------------------------------------

    static int fase_lbl = 0;

    // Resolve the real (R) and imag (M) parts to data-memory cell names. `pre`
    // means a pending accumulator value must be preserved on the stack by the
    // first load (P_LOD) -- only for const/mem inputs. An acc input *is* that
    // pending value and is consumed into temps here, so pre stays 0.
    const char *R, *M;
    int pre = 0;
    expr etr, eti;

    if (e.type == 5)                          // comp constant
    {
        get_cmp_cst(e, &etr, &eti);
        R = v_table[etr.id].name; M = v_table[eti.id].name; pre = acc_ok;
    }
    else if (e.id != 0)                       // comp in memory
    {
        get_cmp_ets(e, &etr, &eti);
        R = v_table[etr.id].name; M = v_table[eti.id].name; pre = acc_ok;
    }
    else                                      // comp in acc (acc=imag, stack top=real)
    {
        int im = exec_id("fase_im"), re = exec_id("fase_re");
        add_instr("SET_P %s\n", v_table[im].name);   // fase_im = imag ; POP -> acc = real
        add_instr("SET %s\n",   v_table[re].name);   // fase_re = real
        R = v_table[re].name; M = v_table[im].name;
    }

    const char *t = v_table[exec_id("fase_t")].name;   // base atan result
    int         n = ++fase_lbl;

    // F_LES X is true when acc > X, so every "v < 0" test is `LOD 0.0; F_LES v`
    // (0 > v) and every "v > 0" test is `LOD v; F_LES 0.0` (v > 0). (This routine
    // was originally written with the opposite F_LES polarity, which silently
    // produced fase(-z) -- the cmm_comp_fase golden had been blessed with those
    // wrong values.)
    add_instr("%s\n", pre ? "P_LOD 0.0" : "LOD 0.0");
    add_instr("F_LES %s\n", R);                        // real < 0 ?  (0 > real)
    add_instr("JIZ Lfa%da\n", n);

    // --- real < 0 (nonzero): atan(imag/real) +/- PI ---
    add_instr("LOD %s\n", R);
    add_instr("F_DIV %s\n", M);                        // imag/real  (F_DIV X = X/acc)
    exec_atan(expr_make(2, 0));                         // atan(imag/real) -> acc
    add_instr("SET %s\n", t);
    add_instr("LOD 0.0\n");
    add_instr("F_LES %s\n", M);                        // imag < 0 ?  (0 > imag)
    add_instr("JIZ Lfa%db\n", n);
    add_instr("LOD %s\n", t);
    add_instr("F_ADD -3.14159265359\n");               // base - PI
    add_instr("JMP Lfa%dz\n", n);
    add_sinst(0, "@Lfa%db ", n);
    add_instr("LOD %s\n", t);
    add_instr("F_ADD 3.14159265359\n");                // base + PI
    add_instr("JMP Lfa%dz\n", n);

    // --- real >= 0 ---
    add_sinst(0, "@Lfa%da ", n);
    add_instr("LOD %s\n", R);
    add_instr("F_LES 0.0\n");                          // real > 0 ?
    add_instr("JIZ Lfa%dc\n", n);
    add_instr("LOD %s\n", R);
    add_instr("F_DIV %s\n", M);                        // imag/real
    exec_atan(expr_make(2, 0));
    add_instr("JMP Lfa%dz\n", n);

    // --- real == 0 : +/- PI/2 (0 if imag == 0) ---
    add_sinst(0, "@Lfa%dc ", n);
    add_instr("LOD %s\n", M);
    add_instr("F_LES 0.0\n");                          // imag > 0 ?
    add_instr("JIZ Lfa%dd\n", n);
    add_instr("LOD 1.57079632679\n");                  // +PI/2
    add_instr("JMP Lfa%dz\n", n);
    add_sinst(0, "@Lfa%dd ", n);
    add_instr("LOD 0.0\n");
    add_instr("F_LES %s\n", M);                        // imag < 0 ?  (0 > imag)
    add_instr("JIZ Lfa%de\n", n);
    add_instr("LOD -1.57079632679\n");                 // -PI/2
    add_instr("JMP Lfa%dz\n", n);
    add_sinst(0, "@Lfa%de ", n);
    add_instr("LOD 0.0\n");                            // imag == 0, real == 0 -> 0

    add_sinst(0, "@Lfa%dz ", n);

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
    if (er.id != 0 && v_table[er.id].type == 0) {fprintf(stderr, MSG_ERR_DECL_FIRST, line_num+1, rem_fname(v_table[er.id].name, fname)); exit(EXIT_FAILURE);}

    // check whether eti was declared
    if (ei.id != 0 && v_table[ei.id].type == 0) {fprintf(stderr, MSG_ERR_DECL_FIRST, line_num+1, rem_fname(v_table[ei.id].name, fname)); exit(EXIT_FAILURE);}

    // check whether etr is a variable
    if (er.id != 0 && v_table[er.id].isar > 0) {fprintf(stderr, MSG_ERR_WRONG_USE, line_num+1, rem_fname(v_table[er.id].name, fname)); exit(EXIT_FAILURE);}

    // check whether et is a variable
    if (ei.id != 0 && v_table[ei.id].isar > 0) {fprintf(stderr, MSG_ERR_WRONG_USE, line_num+1, rem_fname(v_table[ei.id].name, fname)); exit(EXIT_FAILURE);}

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
        add_instr("%s %s\n", i2f, v_table[er.id].name);
        add_instr("P_I2F_M %s\n", v_table[ei.id].name);
    }

    // int in memory and int in acc
    if ((er.type == 1) && (er.id != 0) && (ei.type == 1) && (ei.id == 0))
    {
        add_instr("SET aux_var\n");
        add_instr("I2F_M %s\n", v_table[er.id].name);
        add_instr("P_I2F_M aux_var\n");
    }

    // int in memory and float in memory
    if ((er.type == 1) && (er.id != 0) && (ei.type == 2) && (ei.id != 0))
    {
        add_instr("%s %s\n", i2f, v_table[er.id].name);
        add_instr("P_LOD %s\n"  , v_table[ei.id].name);
    }

    // int in memory and float in acc
    if ((er.type == 1) && (er.id != 0) && (ei.type == 2) && (ei.id == 0))
    {
        add_instr("SET aux_var\n");
        add_instr("I2F_M %s\n", v_table[er.id].name);
        add_instr("P_LOD aux_var\n");
    }

    // int in acc and int in memory
    if ((er.type == 1) && (er.id == 0) && (ei.type == 1) && (ei.id != 0))
    {
        add_instr("I2F\n");
        add_instr("P_I2F_M %s\n", v_table[ei.id].name);
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
        add_instr("P_LOD %s\n", v_table[ei.id].name);
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
        add_instr("%s %s\n",  ld, v_table[er.id].name);
        add_instr("P_I2F_M %s\n", v_table[ei.id].name);
    }

    // float in memory and int in acc
    if ((er.type == 2) && (er.id != 0) && (ei.type == 1) && (ei.id == 0))
    {
        add_instr("SET aux_var\n");
        add_instr("LOD %s\n", v_table[er.id].name);
        add_instr("P_I2F_M aux_var\n");
    }

    // float in memory and float in memory
    if ((er.type == 2) && (er.id != 0) && (ei.type == 2) && (ei.id != 0))
    {
        add_instr("%s %s\n", ld, v_table[er.id].name);
        add_instr("P_LOD %s\n" , v_table[ei.id].name);
    }

    // float in memory and float in acc
    if ((er.type == 2) && (er.id != 0) && (ei.type == 2) && (ei.id == 0))
    {
        add_instr("SET aux_var\n");
        add_instr("LOD %s\n", v_table[er.id].name);
        add_instr("P_LOD aux_var\n");
    }

    // float in acc and int in memory
    if ((er.type == 2) && (er.id == 0) && (ei.type == 1) && (ei.id != 0))
    {
        add_instr("P_I2F_M %s\n", v_table[ei.id].name);
    }

    // float in acc and int in acc
    if ((er.type == 2) && (er.id == 0) && (ei.type == 1) && (ei.id == 0))
    {
        add_instr("I2F\n");
    }

    // float in acc and float in memory
    if ((er.type == 2) && (er.id == 0) && (ei.type == 2) && (ei.id != 0))
    {
        add_instr("P_LOD %s\n", v_table[ei.id].name);
    }

    // float in acc and float in acc
    if ((er.type == 1) && (er.id == 0) && (ei.type == 2) && (ei.id == 0))
    {
        // nothing to do
    }

    acc_ok = 1;
    return expr_make(3, 0);
}

// complex conjugate: comp -> a - bi ; int/float -> x + 0i (always returns a comp)
expr exec_conj(expr e)
{
    // ------------------------------------------------------------------------
    // consistency check ------------------------------------------------------
    // ------------------------------------------------------------------------

    // check whether et was declared
    if (e.id != 0 && v_table[e.id].type == 0) {fprintf(stderr, MSG_ERR_DECL_FIRST, line_num+1, rem_fname(v_table[e.id].name, fname)); exit(EXIT_FAILURE);}

    // check whether et is a variable
    if (e.id != 0 && v_table[e.id].isar > 0) {fprintf(stderr, MSG_ERR_WRONG_USE, line_num+1, rem_fname(v_table[e.id].name, fname)); exit(EXIT_FAILURE);}

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
    // result is comp-in-acc: real on the stack, imag in the acc.
    // ------------------------------------------------------------------------

    // comp constant: a - bi
    if (e.type == 5)
    {
        expr etr, eti;
        get_cmp_cst(e, &etr, &eti);
        add_instr("%s %s\n", ld, v_table[etr.id].name);     // acc = real (push pending if any)
        add_instr("PF_NEG_M %s\n", v_table[eti.id].name);   // push real; acc = -imag  (PF_NEG_M = PSH + F_NEG_M)
    }

    // comp variable in memory: a - bi
    if ((e.type == 3) && (e.id != 0))
    {
        expr etr, eti;
        get_cmp_ets(e, &etr, &eti);
        add_instr("%s %s\n", ld, v_table[etr.id].name);
        add_instr("PF_NEG_M %s\n", v_table[eti.id].name);
    }

    // comp in the acc (acc=imag, stack=real): just negate the imaginary part
    if ((e.type == 3) && (e.id == 0))
    {
        add_instr("F_NEG\n");                               // -imag in acc; real stays on the stack
    }

    // int in memory: x + 0i
    if ((e.type == 1) && (e.id != 0))
    {
        fprintf(stdout, MSG_WARN_CONJ_REAL, line_num+1);
        add_instr("%s %s\n", i2f, v_table[e.id].name);      // acc = (float) x
        add_instr("P_LOD 0.0\n");                           // push x; acc = 0 (imag)
    }

    // int in acc: x + 0i
    if ((e.type == 1) && (e.id == 0))
    {
        fprintf(stdout, MSG_WARN_CONJ_REAL, line_num+1);
        add_instr("I2F\n");
        add_instr("P_LOD 0.0\n");
    }

    // float in memory: x + 0i
    if ((e.type == 2) && (e.id != 0))
    {
        fprintf(stdout, MSG_WARN_CONJ_REAL, line_num+1);
        add_instr("%s %s\n", ld, v_table[e.id].name);
        add_instr("P_LOD 0.0\n");
    }

    // float in acc: x + 0i
    if ((e.type == 2) && (e.id == 0))
    {
        fprintf(stdout, MSG_WARN_CONJ_REAL, line_num+1);
        add_instr("P_LOD 0.0\n");                           // push x (acc); acc = 0
    }

    acc_ok = 1;
    return expr_make(3, 0);                                 // always comp
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
    if (v_table[id1].type == 0) {fprintf(stderr, MSG_ERR_VAR_NOT_FOUND, line_num+1, rem_fname(v_table[id1].name, fname)); exit(EXIT_FAILURE);}

    // check whether id2 was declared
    if (v_table[id2].type == 0) {fprintf(stderr, MSG_ERR_VAR_NOT_FOUND, line_num+1, rem_fname(v_table[id2].name, fname)); exit(EXIT_FAILURE);}

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
        add_instr( "%s %s\n", ld, v_table[id1].name);
        add_instr("MLT %s\n",     v_table[id2].name);

        for (int i = 1; i < N; i++)
        {
            add_instr("P_LOD_V %s %d\n", v_table[id1].name, i);
            add_instr(  "MLT_V %s %d\n", v_table[id2].name, i);
            add_instr("S_ADD\n");
        }
    }

    // float with float
    if ((v_table[id1].type == 2) && (v_table[id2].type == 2))
    {
        add_instr(   "%s %s\n", ld, v_table[id1].name);
        add_instr("F_MLT %s\n",     v_table[id2].name);

        for (int i = 1; i < N; i++)
        {
            add_instr("P_LOD_V %s %d\n", v_table[id1].name, i);
            add_instr("F_MLT_V %s %d\n", v_table[id2].name, i);
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
    if (v_table[idy].type == 0) {fprintf(stderr, MSG_ERR_DECL_FIRST, line_num+1, rem_fname(v_table[idy].name, fname)); exit(EXIT_FAILURE);}

    // check whether idM was declared
    if (v_table[idM].type == 0) {fprintf(stderr, MSG_ERR_DECL_FIRST, line_num+1, rem_fname(v_table[idM].name, fname)); exit(EXIT_FAILURE);}

    // check whether idv was declared
    if (v_table[idv].type == 0) {fprintf(stderr, MSG_ERR_DECL_FIRST, line_num+1, rem_fname(v_table[idv].name, fname)); exit(EXIT_FAILURE);}

    // check that the types match
    if (v_table[idy].type != v_table[idM].type || v_table[idy].type != v_table[idv].type) {fprintf(stderr, MSG_ERR_VARS_SAME_TYPE, line_num+1); exit(EXIT_FAILURE);}

    // check that it is not comp
    if (v_table[idy].type == 3 || v_table[idM].type == 3 || v_table[idv].type == 3) {fprintf(stderr, MSG_ERR_NOT_IMPL_COMPLEX, line_num+1); exit(EXIT_FAILURE);}

    // check whether idy is a vector
    if (v_table[idy].isar != 1) {fprintf(stderr, MSG_ERR_NOT_A_VECTOR, line_num+1, rem_fname(v_table[idy].name, fname)); exit(EXIT_FAILURE);}

    // check whether idM is a matrix
    if (v_table[idM].isar != 2) {fprintf(stderr, MSG_ERR_NOT_A_MATRIX2, line_num+1, rem_fname(v_table[idM].name, fname)); exit(EXIT_FAILURE);}

    // check whether idv is a vector
    if (v_table[idv].isar != 1) {fprintf(stderr, MSG_ERR_NOT_A_VECTOR, line_num+1, rem_fname(v_table[idv].name, fname)); exit(EXIT_FAILURE);}

    // check size between output and matrix
    if (v_table[idy].size != v_table[idM].size) {fprintf(stderr, MSG_ERR_MATRIX_ROW_MISMATCH, line_num+1, rem_fname(v_table[idM].name, fname), rem_fname(v_table[idy].name, fname)); exit(EXIT_FAILURE);}

    // check size between matrix and vector
    if (v_table[idv].size != v_table[idM].siz2) {fprintf(stderr, MSG_ERR_MATRIX_COL_MISMATCH, line_num+1, rem_fname(v_table[idM].name, fname), rem_fname(v_table[idv].name, fname)); exit(EXIT_FAILURE);}

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
            add_instr("LOD_V %s %d\n", v_table[idM].name, i*M);
            add_instr("MLT %s\n"     , v_table[idv].name);

            for (int j = 1; j < M; j++)
            {
                add_instr("P_LOD_V %s %d\n", v_table[idM].name, i*M+j);
                add_instr(  "MLT_V %s %d\n", v_table[idv].name,     j);
                add_instr("S_ADD\n");
            }

            add_instr("SET_V %s %d\n", v_table[idy].name, i);
        }
    }

    // float with float
    if ((v_table[idM].type == 2) && (v_table[idv].type == 2))
    {
        for (int i = 0; i < N; i++)
        {
            add_instr("LOD_V %s %d\n", v_table[idM].name, i*M);
            add_instr("F_MLT %s\n"   , v_table[idv].name);

            for (int j = 1; j < M; j++)
            {
                add_instr("P_LOD_V %s %d\n", v_table[idM].name, i*M+j);
                add_instr("F_MLT_V %s %d\n", v_table[idv].name,     j);
                add_instr("SF_ADD\n");
            }

            add_instr("SET_V %s %d\n", v_table[idy].name, i);
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
    if (v_table[idy].type == 0) {fprintf(stderr, MSG_ERR_DECL_FIRST, line_num+1, rem_fname(v_table[idy].name, fname)); exit(EXIT_FAILURE);}

    // check whether et was declared
    if (e.id != 0 && v_table[e.id].type == 0) {fprintf(stderr, MSG_ERR_DECL_FIRST, line_num+1, rem_fname(v_table[e.id].name, fname)); exit(EXIT_FAILURE);}

    // check whether idv was declared
    if (v_table[idv].type == 0) {fprintf(stderr, MSG_ERR_DECL_FIRST, line_num+1, rem_fname(v_table[idv].name, fname)); exit(EXIT_FAILURE);}

    // check that the types match
    if (v_table[idy].type != e.type || v_table[idy].type != v_table[idv].type) {fprintf(stderr, MSG_ERR_VARS_SAME_TYPE, line_num+1); exit(EXIT_FAILURE);}

    // check that it is not comp
    if (v_table[idy].type == 3 || e.type == 3 || v_table[idv].type == 3) {fprintf(stderr, MSG_ERR_NOT_IMPL_COMPLEX, line_num+1); exit(EXIT_FAILURE);}

    // check whether idy is a vector
    if (v_table[idy].isar != 1) {fprintf(stderr, MSG_ERR_NOT_A_VECTOR, line_num+1, rem_fname(v_table[idy].name, fname)); exit(EXIT_FAILURE);}

    // check whether et is a variable
    if (e.id != 0 && v_table[e.id].isar > 0) {fprintf(stderr, MSG_ERR_WRONG_USE, line_num+1, rem_fname(v_table[e.id].name, fname)); exit(EXIT_FAILURE);}

    // check whether idv is a vector
    if (v_table[idv].isar != 1) {fprintf(stderr, MSG_ERR_NOT_A_VECTOR, line_num+1, rem_fname(v_table[idv].name, fname)); exit(EXIT_FAILURE);}

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

    char g[64]; if (e.id==0) strcpy(g,"aux_var"); else strcpy(g,v_table[e.id].name);

    // ------------------------------------------------------------------------
    // execute ----------------------------------------------------------------
    // ------------------------------------------------------------------------

    printf(MSG_INFO_DIRAC_CV, line_num+1);

    if (e.id==0) add_instr("SET aux_var\n");

    // implement combinations on demand

    for (int i = 0; i < N; i++)
    {
        add_instr("LOD_V %s %d\n", v_table[idv].name, i);

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

        add_instr("SET_V %s %d\n", v_table[idy].name, i);
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
    if (v_table[idy].type == 0) {fprintf(stderr, MSG_ERR_DECL_FIRST, line_num+1, rem_fname(v_table[idy].name, fname)); exit(EXIT_FAILURE);}

    // check whether ida was declared
    if (v_table[ida].type == 0) {fprintf(stderr, MSG_ERR_DECL_FIRST, line_num+1, rem_fname(v_table[ida].name, fname)); exit(EXIT_FAILURE);}

    // check whether etc was declared
    if (ec.id != 0 && v_table[ec.id].type == 0) {fprintf(stderr, MSG_ERR_DECL_FIRST, line_num+1, rem_fname(v_table[ec.id].name, fname)); exit(EXIT_FAILURE);}
    
    // check whether idb was declared
    if (v_table[idb].type == 0) {fprintf(stderr, MSG_ERR_DECL_FIRST, line_num+1, rem_fname(v_table[idb].name, fname)); exit(EXIT_FAILURE);}

    // check that the types match
    if (v_table[idy].type != v_table[ida].type || v_table[idy].type != ec.type || v_table[idy].type != v_table[idb].type) {fprintf(stderr, MSG_ERR_VARS_SAME_TYPE, line_num+1); exit(EXIT_FAILURE);}

    // check that it is not comp
    if (v_table[idy].type == 3 || v_table[ida].type == 3 || ec.type == 3 || v_table[idb].type == 3) {fprintf(stderr, MSG_ERR_NOT_IMPL_COMPLEX, line_num+1); exit(EXIT_FAILURE);}

    // check whether idy is a vector
    if (v_table[idy].isar != 1) {fprintf(stderr, MSG_ERR_NOT_A_VECTOR, line_num+1, rem_fname(v_table[idy].name, fname)); exit(EXIT_FAILURE);}

    // check whether ida is a vector
    if (v_table[ida].isar != 1) {fprintf(stderr, MSG_ERR_NOT_A_VECTOR, line_num+1, rem_fname(v_table[ida].name, fname)); exit(EXIT_FAILURE);}

    // check whether et is a variable
    if (ec.id != 0 && v_table[ec.id].isar > 0) {fprintf(stderr, MSG_ERR_WRONG_USE, line_num+1, rem_fname(v_table[ec.id].name, fname)); exit(EXIT_FAILURE);}

    // check whether idb is a vector
    if (v_table[idb].isar != 1) {fprintf(stderr, MSG_ERR_NOT_A_VECTOR, line_num+1, rem_fname(v_table[idb].name, fname)); exit(EXIT_FAILURE);}

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

    char g[64]; if (ec.id==0) strcpy(g,"aux_var"); else strcpy(g,v_table[ec.id].name);

    // ------------------------------------------------------------------------
    // execute ----------------------------------------------------------------
    // ------------------------------------------------------------------------

    if (ec.id==0) add_instr("SET aux_var\n");

    for (int i = 0; i < N; i++)
    {
        add_instr("LOD_V %s %d\n", v_table[idb].name, i);

        // int
        if (v_table[idy].type == 1)
        {
            add_instr("MLT %s\n", g);
            add_instr("ADD_V %s %d\n", v_table[ida].name, i);
        }

        // float
        if (v_table[idy].type == 2)
        {
            add_instr("F_MLT %s\n", g);
            add_instr("F_ADD_V %s %d\n", v_table[ida].name, i);
        }

        add_instr("SET_V %s %d\n", v_table[idy].name, i);
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
    if (v_table[idM].type == 0) {fprintf(stderr, MSG_ERR_DECL_FIRST, line_num+1, rem_fname(v_table[idM].name, fname)); exit(EXIT_FAILURE);}

    // check whether ida was declared
    if (v_table[ida].type == 0) {fprintf(stderr, MSG_ERR_DECL_FIRST, line_num+1, rem_fname(v_table[ida].name, fname)); exit(EXIT_FAILURE);}
    
    // check whether idb was declared
    if (v_table[idb].type == 0) {fprintf(stderr, MSG_ERR_DECL_FIRST, line_num+1, rem_fname(v_table[idb].name, fname)); exit(EXIT_FAILURE);}

    // check that the types match
    if (v_table[idM].type != v_table[ida].type || v_table[idM].type != v_table[idb].type) {fprintf(stderr, MSG_ERR_VARS_SAME_TYPE, line_num+1); exit(EXIT_FAILURE);}

    // check that it is not comp
    if (v_table[idM].type == 3 || v_table[ida].type == 3 || v_table[idb].type == 3) {fprintf(stderr, MSG_ERR_NOT_IMPL_COMPLEX, line_num+1); exit(EXIT_FAILURE);}

    // check whether idM is a matrix
    if (v_table[idM].isar != 2) {fprintf(stderr, MSG_ERR_NOT_A_MATRIX, line_num+1, rem_fname(v_table[idM].name, fname)); exit(EXIT_FAILURE);}

    // check whether ida is a vector
    if (v_table[ida].isar != 1) {fprintf(stderr, MSG_ERR_NOT_A_VECTOR, line_num+1, rem_fname(v_table[ida].name, fname)); exit(EXIT_FAILURE);}

    // check whether idb is a vector
    if (v_table[idb].isar != 1) {fprintf(stderr, MSG_ERR_NOT_A_VECTOR, line_num+1, rem_fname(v_table[idb].name, fname)); exit(EXIT_FAILURE);}

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
            add_instr("LOD_V %s %d\n", v_table[ida].name, i);

            // int
            if (v_table[idM].type == 1)
            {
                add_instr("MLT_V %s %d\n", v_table[idb].name, j);
            }

            // float
            if (v_table[idM].type == 2)
            {
                add_instr("F_MLT_V %s %d\n", v_table[idb].name, j);
            }

            add_instr("SET_V %s %d\n", v_table[idM].name, N*j+i);
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
    if (v_table[idA].type == 0) {fprintf(stderr, MSG_ERR_DECL_FIRST, line_num+1, rem_fname(v_table[idA].name, fname)); exit(EXIT_FAILURE);}

    // check whether idA was declared
    if (v_table[idB].type == 0) {fprintf(stderr, MSG_ERR_DECL_FIRST, line_num+1, rem_fname(v_table[idB].name, fname)); exit(EXIT_FAILURE);}
    
    // check whether ida was declared
    if (v_table[ida].type == 0) {fprintf(stderr, MSG_ERR_DECL_FIRST, line_num+1, rem_fname(v_table[ida].name, fname)); exit(EXIT_FAILURE);}

    // check whether idb was declared
    if (v_table[idb].type == 0) {fprintf(stderr, MSG_ERR_DECL_FIRST, line_num+1, rem_fname(v_table[idb].name, fname)); exit(EXIT_FAILURE);}

    // check that the types match
    if (v_table[idA].type != v_table[idB].type || v_table[idA].type != v_table[ida].type || v_table[idA].type != v_table[idb].type) {fprintf(stderr, MSG_ERR_VARS_SAME_TYPE, line_num+1); exit(EXIT_FAILURE);}

    // check that it is not comp
    if (v_table[idA].type == 3 || v_table[idB].type == 3 || v_table[ida].type == 3 || v_table[idb].type == 3) {fprintf(stderr, MSG_ERR_NOT_IMPL_COMPLEX, line_num+1); exit(EXIT_FAILURE);}
    // check whether idA is a matrix
    if (v_table[idA].isar != 2) {fprintf(stderr, MSG_ERR_NOT_A_MATRIX, line_num+1, rem_fname(v_table[idA].name, fname)); exit(EXIT_FAILURE);}

    // check whether idB is a matrix
    if (v_table[idB].isar != 2) {fprintf(stderr, MSG_ERR_NOT_A_MATRIX, line_num+1, rem_fname(v_table[idB].name, fname)); exit(EXIT_FAILURE);}

    // check whether ida is a vector
    if (v_table[ida].isar != 1) {fprintf(stderr, MSG_ERR_NOT_A_VECTOR, line_num+1, rem_fname(v_table[ida].name, fname)); exit(EXIT_FAILURE);}

    // check whether idb is a vector
    if (v_table[idb].isar != 1) {fprintf(stderr, MSG_ERR_NOT_A_VECTOR, line_num+1, rem_fname(v_table[idb].name, fname)); exit(EXIT_FAILURE);}

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
            add_instr("LOD_V %s %d\n", v_table[ida].name, i);

            // int
            if (v_table[idA].type == 1)
            {
                add_instr("MLT_V %s %d\n", v_table[idb].name,     j);
                add_instr("NEG\n");
                add_instr("ADD_V %s %d\n", v_table[idB].name, N*j+i);
            }

            // float
            if (v_table[idA].type == 2)
            {
                add_instr("F_MLT_V %s %d\n", v_table[idb].name,     j);
                add_instr("F_NEG\n");
                add_instr("F_ADD_V %s %d\n", v_table[idB].name, N*j+i);
            }

            add_instr("SET_V %s %d\n", v_table[idA].name, N*j+i);
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
    if (v_table[idA].type == 0) {fprintf(stderr, MSG_ERR_DECL_FIRST, line_num+1, rem_fname(v_table[idA].name, fname)); exit(EXIT_FAILURE);}

    // check whether etc was declared
    if (ec.id != 0 && v_table[ec.id].type == 0) {fprintf(stderr, MSG_ERR_DECL_FIRST, line_num+1, rem_fname(v_table[ec.id].name, fname)); exit(EXIT_FAILURE);}
    
    // check whether idM was declared
    if (v_table[idM].type == 0) {fprintf(stderr, MSG_ERR_DECL_FIRST, line_num+1, rem_fname(v_table[idM].name, fname)); exit(EXIT_FAILURE);}

    // check that the types match
    if (v_table[idA].type != ec.type || v_table[idA].type != v_table[idM].type) {fprintf(stderr, MSG_ERR_VARS_SAME_TYPE, line_num+1); exit(EXIT_FAILURE);}

    // check that it is not comp
    if (v_table[idA].type == 3 || ec.type == 3 || v_table[idM].type == 3) {fprintf(stderr, MSG_ERR_NOT_IMPL_COMPLEX, line_num+1); exit(EXIT_FAILURE);}

    // check whether idA is a matrix
    if (v_table[idA].isar != 2) {fprintf(stderr, MSG_ERR_NOT_A_MATRIX, line_num+1, rem_fname(v_table[idA].name, fname)); exit(EXIT_FAILURE);}

    // check whether etc is a variable
    if (ec.id != 0 && v_table[ec.id].isar > 0) {fprintf(stderr, MSG_ERR_WRONG_USE, line_num+1, rem_fname(v_table[ec.id].name, fname)); exit(EXIT_FAILURE);}

    // check whether idM is a matrix
    if (v_table[idM].isar != 2) {fprintf(stderr, MSG_ERR_NOT_A_MATRIX, line_num+1, rem_fname(v_table[idM].name, fname)); exit(EXIT_FAILURE);}

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

    char g[64]; if (ec.id==0) strcpy(g,"aux_var"); else strcpy(g,v_table[ec.id].name);

    // ------------------------------------------------------------------------
    // execute ----------------------------------------------------------------
    // ------------------------------------------------------------------------

    printf(MSG_INFO_DIRAC_CM, line_num+1);

    if (ec.id==0) add_instr("SET aux_var\n");

    for (int i = 0; i < N; i++)
    {
        for (int j = 0; j < M; j++)
        {
            add_instr("LOD_V %s %d\n", v_table[idM].name, M*i+j);

            // int
            if (v_table[idA].type == 1) add_instr("MLT %s\n", g);

            // float
            if (v_table[idA].type == 2) add_instr("F_MLT %s\n", g);
            
            add_instr("SET_V %s %d\n", v_table[idA].name, M*i+j);
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
    if (v_table[idM].type == 0) {fprintf(stderr, MSG_ERR_DECL_FIRST, line_num+1, rem_fname(v_table[idM].name, fname)); exit(EXIT_FAILURE);}

    // check whether etc was declared
    if (ec.id != 0 && v_table[ec.id].type == 0) {fprintf(stderr, MSG_ERR_DECL_FIRST, line_num+1, rem_fname(v_table[ec.id].name, fname)); exit(EXIT_FAILURE);}

    // check that the types match
    if (v_table[idM].type != ec.type) {fprintf(stderr, MSG_ERR_VARS_SAME_TYPE, line_num+1); exit(EXIT_FAILURE);}

    // check that it is not comp
    if (v_table[idM].type == 3 || ec.type == 3) {fprintf(stderr, MSG_ERR_NOT_IMPL_COMPLEX, line_num+1); exit(EXIT_FAILURE);}
    // check whether idM is a matrix
    if (v_table[idM].isar != 2) {fprintf(stderr, MSG_ERR_NOT_A_MATRIX, line_num+1, rem_fname(v_table[idM].name, fname)); exit(EXIT_FAILURE);}

    // check whether etc is a variable
    if (ec.id != 0 && v_table[ec.id].isar > 0) {fprintf(stderr, MSG_ERR_WRONG_USE, line_num+1, rem_fname(v_table[ec.id].name, fname)); exit(EXIT_FAILURE);}

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

    if (ec.id!=0) add_instr("LOD %s\n",v_table[ec.id].name);

    for (int i = 0; i < N; i++)
    {
        for (int j = 0; j < N; j++)
        {
            if (i == j) add_instr("SET_V %s %d\n", v_table[idM].name, N*i+j);
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
            if (i != j) add_instr("SET_V %s %d\n", v_table[idM].name, N*i+j);
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
    if (v_table[idv].type == 0) {fprintf(stderr, MSG_ERR_DECL_FIRST, line_num+1, rem_fname(v_table[idv].name, fname)); exit(EXIT_FAILURE);}

    // check that it is not comp
    if (v_table[idv].type == 3) {fprintf(stderr, MSG_ERR_NOT_IMPL_COMPLEX, line_num+1); exit(EXIT_FAILURE);}
    
    // check whether idv is a vector
    if (v_table[idv].isar != 1) {fprintf(stderr, MSG_ERR_NOT_A_VECTOR, line_num+1, rem_fname(v_table[idv].name, fname)); exit(EXIT_FAILURE);}

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

    for (int i = 0; i < N; i++) add_instr("SET_V %s %d\n", v_table[idv].name, i);
}

// reads input vector with weight c, e.g. a # c|in(0)>;
void exec_cvin(int idv, expr ec, int idp)
{
    // ------------------------------------------------------------------------
    // consistency check ------------------------------------------------------
    // ------------------------------------------------------------------------

    // check whether idv was declared
    if (v_table[idv].type == 0) {fprintf(stderr, MSG_ERR_DECL_FIRST, line_num+1, rem_fname(v_table[idv].name, fname)); exit(EXIT_FAILURE);}

    // check whether etc was declared
    if (ec.id != 0 && v_table[ec.id].type == 0) {fprintf(stderr, MSG_ERR_DECL_FIRST, line_num+1, rem_fname(v_table[ec.id].name, fname)); exit(EXIT_FAILURE);}

    // check that it is not comp
    if (v_table[idv].type == 3 || ec.type == 3) {fprintf(stderr, MSG_ERR_NOT_IMPL_COMPLEX, line_num+1); exit(EXIT_FAILURE);}

    // check whether idv is a vector
    if (v_table[idv].isar != 1) {fprintf(stderr, MSG_ERR_NOT_A_VECTOR, line_num+1, rem_fname(v_table[idv].name, fname)); exit(EXIT_FAILURE);}

    // check whether etc is a variable
    if (ec.id != 0 && v_table[ec.id].isar > 0) {fprintf(stderr, MSG_ERR_WRONG_USE, line_num+1, rem_fname(v_table[ec.id].name, fname)); exit(EXIT_FAILURE);}

    // ------------------------------------------------------------------------
    // update variable status -------------------------------------------------
    // ------------------------------------------------------------------------

    if (ec.id != 0) v_table[ec.id].used = 1;

    // ------------------------------------------------------------------------
    // prepare local variables ------------------------------------------------
    // ------------------------------------------------------------------------

    int N = v_table[idv].size;

    char g[64]; if (ec.id==0) strcpy(g,"aux_var"); else strcpy(g,v_table[ec.id].name);

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
            add_instr("INN %s\n", v_table[idp].name);
            add_instr("MLT %s\n",g);
        }

        // float
        if (v_table[idv].type == 2)
        {
            add_instr("F_INN %s\n", v_table[idp].name);
            add_instr("F_MLT %s\n",g);
        }

        add_instr("SET_V %s %d\n", v_table[idv].name, i);
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
    if (v_table[idv].type == 0) {fprintf(stderr, MSG_ERR_DECL_FIRST, line_num+1, rem_fname(v_table[idv].name, fname)); exit(EXIT_FAILURE);}

    // check whether etc was declared
    if (ec.id != 0 && v_table[ec.id].type == 0) {fprintf(stderr, MSG_ERR_DECL_FIRST, line_num+1, rem_fname(v_table[ec.id].name, fname)); exit(EXIT_FAILURE);}

    // check that it is not comp
    if (v_table[idv].type == 3 || ec.type == 3) {fprintf(stderr, MSG_ERR_NOT_IMPL_COMPLEX, line_num+1); exit(EXIT_FAILURE);}

    // check whether idv is a vector
    if (v_table[idv].isar != 1) {fprintf(stderr, MSG_ERR_NOT_A_VECTOR, line_num+1, rem_fname(v_table[idv].name, fname)); exit(EXIT_FAILURE);}

    // check whether etc is a variable
    if (ec.id != 0 && v_table[ec.id].isar > 0) {fprintf(stderr, MSG_ERR_WRONG_USE, line_num+1, rem_fname(v_table[ec.id].name, fname)); exit(EXIT_FAILURE);}

    // ------------------------------------------------------------------------
    // update variable status -------------------------------------------------
    // ------------------------------------------------------------------------

    if (ec.id != 0) v_table[ec.id].used = 1;
    v_table[idv].used = 1;
    
    // ------------------------------------------------------------------------
    // prepare local variables ------------------------------------------------
    // ------------------------------------------------------------------------

    int N = v_table[idv].size;

    char g[64]; if (ec.id==0) strcpy(g,"aux_var"); else strcpy(g,v_table[ec.id].name);

    // ------------------------------------------------------------------------
    // execute ----------------------------------------------------------------
    // ------------------------------------------------------------------------

    printf(MSG_INFO_DIRAC_FLUSH_VECTOR, line_num+1);

    if (ec.id==0) add_instr("SET aux_var\n");

    for (int i = 0; i < N; i++)
    {
        add_instr("LOD_V %s %d\n", v_table[idv].name, i);

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

        add_instr("OUT %s\n", v_table[idp].name);
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
    if (v_table[ida].type == 0) {fprintf(stderr, MSG_ERR_DECL_FIRST, line_num+1, rem_fname(v_table[ida].name, fname)); exit(EXIT_FAILURE);}

    // check whether etb was declared
    if (eb.id != 0 && v_table[eb.id].type == 0) {fprintf(stderr, MSG_ERR_DECL_FIRST, line_num+1, rem_fname(v_table[eb.id].name, fname)); exit(EXIT_FAILURE);}

    // check whether idc equals ida
    if (idc != ida) {fprintf(stderr, MSG_ERR_SHIFT_VEC_SELF, line_num+1); exit(EXIT_FAILURE);}
    
    // check that it is not comp
    if (v_table[ida].type == 3 || eb.type == 3) {fprintf(stderr, MSG_ERR_NOT_IMPL_COMPLEX, line_num+1); exit(EXIT_FAILURE);}

    // check that the types match
    //if (v_table[ida].type != v_table[eb.id].type) {fprintf(stderr, MSG_ERR_VARS_SAME_TYPE, line_num+1); exit(EXIT_FAILURE);}
    
    // check whether ida is a vector
    if (v_table[ida].isar != 1) {fprintf(stderr, MSG_ERR_NOT_A_VECTOR, line_num+1, rem_fname(v_table[ida].name, fname)); exit(EXIT_FAILURE);}

    // check whether etb is a variable
    if (eb.id != 0 && v_table[eb.id].isar > 0) {fprintf(stderr, MSG_ERR_WRONG_USE, line_num+1, rem_fname(v_table[eb.id].name, fname)); exit(EXIT_FAILURE);}

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

    printf(MSG_INFO_DIRAC_SHIFT, v_table[ida].name, line_num+1);

    // ida int and etb int in memory
    if (v_table[ida].type == 1 && eb.type == 1 && eb.id != 0)
    {
        for (int i = N-1; i > 0; i--)
        {
            add_instr("LOD_V %s %d\n", v_table[ida].name, i-1);
            add_instr("SET_V %s %d\n", v_table[ida].name, i);
        }

        add_instr("LOD %s\n", v_table[eb.id].name);
        add_instr("SET %s\n", v_table[ida].name);
    }

    // ida float e etb int no acc
    if (v_table[ida].type == 2 && eb.type == 1 && eb.id == 0)
    {
        add_instr("SET aux_var\n");

        for (int i = N-1; i > 0; i--)
        {
            add_instr("LOD_V %s %d\n", v_table[ida].name, i-1);
            add_instr("SET_V %s %d\n", v_table[ida].name, i);
        }

        add_instr("I2F_M aux_var\n");
        add_instr("SET %s\n", v_table[ida].name);
    }

    // ida float and etb float in memory
    if (v_table[ida].type == 2 && eb.type == 2 && eb.id != 0)
    {
        for (int i = N-1; i > 0; i--)
        {
            add_instr("LOD_V %s %d\n", v_table[ida].name, i-1);
            add_instr("SET_V %s %d\n", v_table[ida].name, i);
        }

        add_instr("LOD %s\n", v_table[eb.id].name);
        add_instr("SET %s\n", v_table[ida].name);
    }

    // ida float e etb float no acc
    if (v_table[ida].type == 2 && eb.type == 2 && eb.id == 0)
    {
        add_instr("SET aux_var\n");

        for (int i = N-1; i > 0; i--)
        {
            add_instr("LOD_V %s %d\n", v_table[ida].name, i-1);
            add_instr("SET_V %s %d\n", v_table[ida].name, i);
        }

        add_instr("LOD aux_var\n");
        add_instr("SET %s\n", v_table[ida].name);
    }

    acc_ok = 0;
}
