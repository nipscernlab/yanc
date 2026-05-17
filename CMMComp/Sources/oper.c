// ----------------------------------------------------------------------------
// ALU operations -------------------------------------------------------------
// ----------------------------------------------------------------------------

// global includes
#include <string.h>
#include <stdlib.h>

// local includes
#include "..\Headers\t2t.h"
#include "..\Headers\oper.h"
#include "..\Headers\stdlib.h"
#include "..\Headers\global.h"
#include "..\Headers\data_use.h"
#include "..\Headers\diretivas.h"
#include "..\Headers\variaveis.h"
#include "..\Headers\messages.h"

// ----------------------------------------------------------------------------
// arithmetic operations ------------------------------------------------------
// ----------------------------------------------------------------------------

// negates a number
expr oper_neg(expr e)
{
    int   et = expr_to_et(e);
    int   etr, eti;

    char  neg[10]; if (acc_ok == 0) strcpy( neg,   "NEG_M"); else strcpy( neg,  "P_NEG_M");
    char fneg[10]; if (acc_ok == 0) strcpy(fneg, "F_NEG_M"); else strcpy(fneg, "PF_NEG_M");

    // when it is an int variable in memory
    if ((get_type(et) == 1) && (et % OFST != 0))
    {
        add_instr("%s %s\n", neg, v_name[et % OFST]);
    }

    // when it is an int in the acc
    if ((get_type(et) == 1) && (et % OFST == 0))
    {
        add_instr("NEG\n");
    }

    // when it is a float variable in memory
    if ((get_type(et) == 2) && (et % OFST != 0))
    {
        add_instr("%s %s\n", fneg, v_name[et % OFST]);
    }

    // when it is a float in the acc
    if ((get_type(et) == 2) && (et % OFST == 0))
    {
        add_instr("F_NEG\n");
    }

    // when it is a comp constant
    if (get_type(et) == 5)
    {
        get_cmp_cst(et,&etr,&eti);

        add_instr("%s %s\n", fneg, v_name[etr%OFST]);
        add_instr("PF_NEG_M %s\n", v_name[eti%OFST]);
    }

    // when it is a comp variable in memory
    if ((get_type(et) == 3) && (et % OFST != 0))
    {
        get_cmp_ets(et,&etr,&eti);

        add_instr("%s %s\n", fneg, v_name[etr%OFST]);
        add_instr("PF_NEG_M %s\n", v_name[eti%OFST]);
    }

    // when it is a comp in the acc
    if ((get_type(et) == 3) && (et % OFST == 0))
    {
        add_instr("   SET_P aux_var\n");
        add_instr(" F_NEG\n"          );
        add_instr("PF_NEG_M aux_var\n");
    }

    acc_ok = 1;

    if (get_type(et) == 5) et = 3*OFST;

    return expr_make(get_type(et), 0);
}

// adds two numbers
expr oper_soma(expr e1, expr e2)
{
    int et1 = expr_to_et(e1);
    int et2 = expr_to_et(e2);
    int etr, eti;

    char ld [10]; if (acc_ok == 0) strcpy(ld ,"LOD"  ); else strcpy(ld ,"P_LOD"  );
    char i2f[10]; if (acc_ok == 0) strcpy(i2f,"I2F_M"); else strcpy(i2f,"P_I2F_M");

    // int var with int var
    if ((get_type(et1)==1) && (et1%OFST!=0) && (get_type(et2)==1) && (et2%OFST!=0))
    {
        add_instr("%s %s\n" , ld, v_name[et1%OFST]);
        add_instr("ADD %s\n",     v_name[et2%OFST]);
    }

    // int var with int acc
    if ((get_type(et1)==1) && (et1%OFST!=0) && (get_type(et2)==1) && (et2%OFST==0))
    {
        add_instr("ADD %s\n", v_name[et1%OFST]);
    }

    // int var with float var
    if ((get_type(et1)==1) && (et1%OFST!=0) && (get_type(et2)==2) && (et2%OFST!=0))
    {
        add_instr("%s %s\n", i2f, v_name[et1%OFST]);
        add_instr("F_ADD %s\n"  , v_name[et2%OFST]);
    }

    // int var with float acc
    if ((get_type(et1)==1) && (et1%OFST!=0) && (get_type(et2)==2) && (et2%OFST==0))
    {
        add_instr("P_I2F_M %s\n", v_name[et1%OFST]);
        add_instr("SF_ADD\n");
    }

    // int var with comp const
    if ((get_type(et1)==1) && (et1%OFST!=0) && (get_type(et2)==5))
    {
        get_cmp_cst(et2,&etr,&eti);

        add_instr("%s %s\n" , i2f, v_name[et1%OFST]);
        add_instr("F_ADD %s\n"   , v_name[etr%OFST]);
        add_instr("P_LOD %s\n"   , v_name[eti%OFST]);
    }

    // int var with comp var
    if ((get_type(et1)==1) && (et1%OFST!=0) && (get_type(et2)==3) && (et2%OFST!=0))
    {
        get_cmp_ets(et2,&etr,&eti);

        add_instr("%s %s\n", i2f, v_name[et1%OFST]);
        add_instr("F_ADD %s\n"  , v_name[etr%OFST]);
        add_instr("P_LOD  %s\n" , v_name[eti%OFST]);
    }

    // int var with comp acc
    if ((get_type(et1)==1) && (et1%OFST!=0) && (get_type(et2)==3) && (et2%OFST==0))
    {
        add_instr("SET aux_var\n");                 // temporarily save the imaginary part
        add_instr("I2F_M %s\n", v_name[et1%OFST]);  // fetch the int while converting to float
        add_instr("SF_ADD\n");                      // add acc with stack
        add_instr("P_LOD aux_var\n");               // push the result onto the stack while bringing the imag back
    }

    // int acc with int var
    if ((get_type(et1)==1) && (et1%OFST==0) && (get_type(et2)==1) && (et2%OFST!=0))
    {
        add_instr("ADD %s\n", v_name[et2%OFST]);
    }

    // int acc with int acc
    if ((get_type(et1)==1) && (et1%OFST==0) && (get_type(et2)==1) && (et2%OFST==0))
    {
        add_instr("S_ADD\n");
    }

    // int acc with float var
    if ((get_type(et1)==1) && (et1%OFST==0) && (get_type(et2)==2) && (et2%OFST!=0))
    {
        add_instr("I2F\n");
        add_instr("F_ADD %s\n", v_name[et2%OFST]);
    }

    // int acc with float acc
    if ((get_type(et1)==1) && (et1%OFST==0) && (get_type(et2)==2) && (et2%OFST==0))
    {
        add_instr("SET_P aux_var\n"); // temporarily save the float, pushing the int to the accumulator
        add_instr("I2F\n");           // converte o int do acumulador pra float
        add_instr("F_ADD aux_var\n"); // add with the saved float
    }

    // int acc with comp const
    if ((get_type(et1)==1) && (et1%OFST==0) && (get_type(et2)==5))
    {
        get_cmp_cst(et2,&etr,&eti);

        add_instr("I2F\n");
        add_instr("F_ADD %s\n", v_name[etr%OFST]);
        add_instr("P_LOD %s\n", v_name[eti%OFST]);
    }

    // int acc with comp var
    if ((get_type(et1)==1) && (et1%OFST==0) && (get_type(et2)==3) && (et2%OFST!=0))
    {
        get_cmp_ets(et2,&etr,&eti);

        add_instr("I2F\n");
        add_instr("F_ADD %s\n", v_name[etr%OFST]);
        add_instr("P_LOD %s\n", v_name[eti%OFST]);
    }

    // int acc with comp acc
    if ((get_type(et1)==1) && (et1%OFST==0) && (get_type(et2)==3) && (et2%OFST==0))
    {
        add_instr("SET_P aux_var\n" ); // temporarily save the imaginary part, popping the real part
        add_instr("SET_P aux_var1\n"); // temporarily save the real part, popping the int
        add_instr("I2F\n");            // converte o int do acumulador pra float
        add_instr("F_ADD aux_var1\n"); // add with the saved real part
        add_instr("P_LOD aux_var\n" ); // push the imaginary part back to the accumulator
    }

    // float var with int var
    if ((get_type(et1)==2) && (et1%OFST!=0) && (get_type(et2)==1) && (et2%OFST!=0))
    {
        add_instr("%s %s\n",i2f, v_name[et2%OFST]);
        add_instr("F_ADD %s\n" , v_name[et1%OFST]);
    }

    // float var with int acc
    if ((get_type(et1)==2) && (et1%OFST!=0) && (get_type(et2)==1) && (et2%OFST==0))
    {
        add_instr("I2F\n");
        add_instr("F_ADD %s\n", v_name[et1%OFST]);
    }

    // float var with float var
    if ((get_type(et1)==2) && (et1%OFST!=0) && (get_type(et2)==2) && (et2%OFST!=0))
    {
        add_instr("%s %s\n", ld, v_name[et1%OFST]);
        add_instr("F_ADD %s\n" , v_name[et2%OFST]);
    }

    // float var with float acc
    if ((get_type(et1)==2) && (et1%OFST!=0) && (get_type(et2)==2) && (et2%OFST==0))
    {
        add_instr("F_ADD %s\n", v_name[et1%OFST]);
    }

    // float var with comp const
    if ((get_type(et1)==2) && (et1%OFST!=0) && (get_type(et2)==5))
    {
        get_cmp_cst(et2,&etr,&eti);

        add_instr("%s %s\n", ld, v_name[et1%OFST]);
        add_instr("F_ADD %s\n" , v_name[etr%OFST]);
        add_instr("P_LOD %s\n" , v_name[eti%OFST]);
    }

    // float var with comp var
    if ((get_type(et1)==2) && (et1%OFST!=0) && (get_type(et2)==3) && (et2%OFST!=0))
    {
        get_cmp_ets(et2,&etr,&eti);

        add_instr("%s %s\n", ld, v_name[et1%OFST]);
        add_instr("F_ADD %s\n" , v_name[etr%OFST]);
        add_instr("P_LOD %s\n" , v_name[eti%OFST]);
    }

    // float var with comp acc
    if ((get_type(et1)==2) && (et1%OFST!=0) && (get_type(et2)==3) && (et2%OFST==0))
    {
        add_instr("SET_P aux_var\n");
        add_instr("F_ADD %s\n", v_name[et1%OFST]);
        add_instr("P_LOD aux_var\n");
    }

    // float acc with int var
    if ((get_type(et1)==2) && (et1%OFST==0) && (get_type(et2)==1) && (et2%OFST!=0))
    {
        add_instr("P_I2F_M %s\n", v_name[et2%OFST]);
        add_instr("SF_ADD\n");
    }

    // float acc with int acc
    if ((get_type(et1)==2) && (et1%OFST==0) && (get_type(et2)==1) && (et2%OFST==0))
    {
        add_instr("I2F\n");
        add_instr("SF_ADD\n");
    }

    // float acc with float var
    if ((get_type(et1)==2) && (et1%OFST==0) && (get_type(et2)==2) && (et2%OFST!=0))
    {
        add_instr("F_ADD %s\n", v_name[et2%OFST]);
    }

    // float acc with float acc
    if ((get_type(et1)==2) && (et1%OFST==0) && (get_type(et2)==2) && (et2%OFST==0))
    {
        add_instr("SF_ADD\n");
    }

    // float acc with comp const
    if ((get_type(et1)==2) && (et1%OFST==0) && (get_type(et2)==5))
    {
        get_cmp_cst(et2,&etr,&eti);

        add_instr("F_ADD %s\n" , v_name[etr%OFST]);
        add_instr("P_LOD %s\n" , v_name[eti%OFST]);
    }

    // float acc with comp var
    if ((get_type(et1)==2) && (et1%OFST==0) && (get_type(et2)==3) && (et2%OFST!=0))
    {
        get_cmp_ets(et2,&etr,&eti);

        add_instr("F_ADD %s\n", v_name[etr%OFST]);
        add_instr("P_LOD %s\n", v_name[eti%OFST]);
    }

    // float acc with comp acc
    if ((get_type(et1)==2) && (et1%OFST==0) && (get_type(et2)==3) && (et2%OFST==0))
    {
        add_instr("SET_P aux_var\n");
        add_instr("SF_ADD\n");
        add_instr("P_LOD aux_var\n");
    }

    // comp const with int var
    if ((get_type(et1)==5) && (get_type(et2)==1) && (et2%OFST!=0))
    {
        get_cmp_cst(et1,&etr,&eti);

        add_instr("%s %s\n", i2f, v_name[et2%OFST]);
        add_instr("F_ADD %s\n"  , v_name[etr%OFST]);
        add_instr("P_LOD %s\n"  , v_name[eti%OFST]);
    }

    // comp const with int acc
    if ((get_type(et1)==5) && (get_type(et2)==1) && (et2%OFST==0))
    {
        get_cmp_cst(et1,&etr,&eti);

        add_instr("I2F\n");
        add_instr("F_ADD %s\n", v_name[etr%OFST]);
        add_instr("P_LOD %s\n", v_name[eti%OFST]);
    }

    // comp const with float var
    if ((get_type(et1)==5) && (get_type(et2)==2) && (et2%OFST!=0))
    {
        get_cmp_cst(et1,&etr,&eti);

        add_instr("%s %s\n", ld, v_name[et2%OFST]);
        add_instr("F_ADD %s\n" , v_name[etr%OFST]);
        add_instr("P_LOD %s\n" , v_name[eti%OFST]);
    }

    // comp const with float acc
    if ((get_type(et1)==5) && (get_type(et2)==2) && (et2%OFST==0))
    {
        get_cmp_cst(et1,&etr,&eti);

        add_instr("F_ADD %s\n", v_name[etr%OFST]);
        add_instr("P_LOD %s\n", v_name[eti%OFST]);
    }

    // comp const with comp const
    if ((get_type(et1)==5) && (get_type(et2)==5))
    {
        int et1r,et2r;
        int et1i,et2i;

        get_cmp_cst(et1,&et1r,&et1i);
        get_cmp_cst(et2,&et2r,&et2i);

        add_instr("%s %s\n", ld, v_name[et1r%OFST]);
        add_instr("F_ADD %s\n" , v_name[et2r%OFST]);

        add_instr("P_LOD %s\n" , v_name[et1i%OFST]);
        add_instr("F_ADD %s\n" , v_name[et2i%OFST]);
    }

    // comp const with comp var
    if ((get_type(et1)==5) && (get_type(et2)==3) && (et2%OFST!=0))
    {
        int et1r,et2r;
        int et1i,et2i;

        get_cmp_cst(et1,&et1r,&et1i);
        get_cmp_ets(et2,&et2r,&et2i);

        add_instr("%s %s\n", ld, v_name[et1r%OFST]);
        add_instr("F_ADD %s\n" , v_name[et2r%OFST]);

        add_instr("P_LOD %s\n" , v_name[et1i%OFST]);
        add_instr("F_ADD %s\n" , v_name[et2i%OFST]);
    }

    // comp const with comp acc
    if ((get_type(et1)==5) && (get_type(et2)==3) && (et2%OFST==0))
    {
        get_cmp_cst(et1,&etr,&eti);

        add_instr("SET_P aux_var\n");
        add_instr("F_ADD %s\n", v_name[etr%OFST]);

        add_instr("P_LOD %s\n", v_name[eti%OFST]);
        add_instr("F_ADD aux_var\n");
    }

    // comp var with int var
    if ((get_type(et1)==3) && (et1%OFST!=0) && (get_type(et2)==1) && (et2%OFST!=0))
    {
        get_cmp_ets(et1,&etr,&eti);

        add_instr("%s  %s\n", i2f, v_name[et2%OFST]);
        add_instr("F_ADD %s\n"    , v_name[etr%OFST]);
        add_instr("P_LOD  %s\n"    , v_name[eti%OFST]);
    }

    // comp var with int acc
    if ((get_type(et1)==3) && (et1%OFST!=0) && (get_type(et2)==1) && (et2%OFST==0))
    {
        get_cmp_ets(et1,&etr,&eti);

        add_instr("I2F\n");
        add_instr("F_ADD %s\n", v_name[etr%OFST]);
        add_instr("P_LOD %s\n", v_name[eti%OFST]);
    }

    // comp var with float var
    if ((get_type(et1)==3) && (et1%OFST!=0) && (get_type(et2)==2) && (et2%OFST!=0))
    {
        get_cmp_ets(et1,&etr,&eti);

        add_instr("%s %s\n", ld, v_name[et2%OFST]);
        add_instr("F_ADD %s\n" , v_name[etr%OFST]);
        add_instr("P_LOD %s\n" , v_name[eti%OFST]);
    }

    // comp var with float acc
    if ((get_type(et1)==3) && (et1%OFST!=0) && (get_type(et2)==2) && (et2%OFST==0))
    {
        get_cmp_ets(et1,&etr,&eti);

        add_instr("F_ADD %s\n", v_name[etr%OFST]);
        add_instr("P_LOD %s\n", v_name[eti%OFST]);
    }

    // comp var with comp const
    if ((get_type(et1)==3) && (et1%OFST!=0) && (get_type(et2)==5))
    {
        int et1r,et2r;
        int et1i,et2i;

        get_cmp_ets(et1,&et1r,&et1i);
        get_cmp_cst(et2,&et2r,&et2i);

        add_instr("%s %s\n", ld, v_name[et1r%OFST]);
        add_instr("F_ADD %s\n" , v_name[et2r%OFST]);

        add_instr("P_LOD %s\n" , v_name[et1i%OFST]);
        add_instr("F_ADD %s\n" , v_name[et2i%OFST]);
    }

    // comp var with comp var
    if ((get_type(et1)==3) && (et1%OFST!=0) && (get_type(et2)==3) && (et2%OFST!=0))
    {
        int et1r,et2r;
        int et1i,et2i;

        get_cmp_ets(et1,&et1r,&et1i);
        get_cmp_ets(et2,&et2r,&et2i);

        add_instr("%s %s\n", ld, v_name[et1r%OFST]);
        add_instr("F_ADD %s\n" , v_name[et2r%OFST]);

        add_instr("P_LOD %s\n" , v_name[et1i%OFST]);
        add_instr("F_ADD %s\n" , v_name[et2i%OFST]);
    }

    // comp var with comp acc
    if ((get_type(et1)==3) && (et1%OFST!=0) && (get_type(et2)==3) && (et2%OFST==0))
    {
        get_cmp_ets(et1,&etr,&eti);

        add_instr("SET_P aux_var\n");
        add_instr("F_ADD %s\n", v_name[etr%OFST]);

        add_instr("P_LOD %s\n", v_name[eti%OFST]);
        add_instr("F_ADD aux_var\n");
    }

    // comp acc with int var
    if ((get_type(et1)==3) && (et1%OFST==0) && (get_type(et2)==1) && (et2%OFST!=0))
    {
        add_instr("SET_P aux_var\n");
        add_instr("P_I2F_M %s\n", v_name[et2%OFST]);
        add_instr("SF_ADD\n");
        add_instr("P_LOD aux_var\n");
    }

    // comp acc with int acc
    if ((get_type(et1)==3) && (et1%OFST==0) && (get_type(et2)==1) && (et2%OFST==0))
    {
        add_instr("SET_P   aux_var\n" );
        add_instr("SET_P   aux_var1\n");
        add_instr("P_I2F_M aux_var\n" );
        add_instr("SF_ADD\n");
        add_instr("P_LOD   aux_var1\n");
    }

    // comp acc with float var
    if ((get_type(et1)==3) && (et1%OFST==0) && (get_type(et2)==2) && (et2%OFST!=0))
    {
        add_instr("SET_P aux_var\n");
        add_instr("F_ADD %s\n", v_name[et2%OFST]);
        add_instr("P_LOD aux_var\n");
    }

    // comp acc with float acc
    if ((get_type(et1)==3) && (et1%OFST==0) && (get_type(et2)==2) && (et2%OFST==0))
    {
        add_instr("SET_P aux_var\n" );
        add_instr("SET_P aux_var1\n");
        add_instr("F_ADD aux_var\n" );
        add_instr("P_LOD aux_var1\n");
    }

    // comp acc with comp const
    if ((get_type(et1)==3) && (et1%OFST==0) && (get_type(et2)==5))
    {
        get_cmp_cst(et2,&etr,&eti);

        add_instr("SET_P aux_var\n");
        add_instr("F_ADD %s\n", v_name[etr%OFST]);

        add_instr("P_LOD aux_var\n");
        add_instr("F_ADD %s\n", v_name[eti%OFST]);
    }

    // comp acc with comp var
    if ((get_type(et1)==3) && (et1%OFST==0) && (get_type(et2)==3) && (et2%OFST!=0))
    {
        get_cmp_ets(et2,&etr,&eti);

        add_instr("SET_P aux_var\n");
        add_instr("F_ADD %s\n", v_name[etr%OFST]);

        add_instr("P_LOD aux_var\n");
        add_instr("F_ADD %s\n", v_name[eti%OFST]);
    }

    // comp acc with comp acc
    if ((get_type(et1)==3) && (et1%OFST==0) && (get_type(et2)==3) && (et2%OFST==0))
    {
        add_instr("SET_P aux_var \n");
        add_instr("SET_P aux_var1\n");
        add_instr("SET_P aux_var2\n");
        add_instr("F_ADD aux_var1\n");

        add_instr("P_LOD aux_var2\n");
        add_instr("F_ADD aux_var \n");
    }

    acc_ok = 1;

    int type;
         if ((get_type(et1) > 2) || (get_type(et2) > 2))
         type = 3;
    else if ((get_type(et1) > 1) || (get_type(et2) > 1))
         type = 2;
    else type = 1;

    return expr_make(type, 0);
}

// subtraction between two numbers
expr oper_subt(expr e1, expr e2)
{
    int et1 = expr_to_et(e1);
    int et2 = expr_to_et(e2);
    int etr, eti;

    char ld [10]; if (acc_ok == 0) strcpy(ld ,"LOD"  ); else strcpy(ld ,"P_LOD"  );
    char i2f[10]; if (acc_ok == 0) strcpy(i2f,"I2F_M"); else strcpy(i2f,"P_I2F_M");

    // int var with int var
    if ((get_type(et1)==1) && (et1%OFST!=0) && (get_type(et2)==1) && (et2%OFST!=0))
    {
        return oper_soma(expr_of_et(et1), oper_neg(expr_of_et(et2)));
    }

    // int var with int acc
    if ((get_type(et1)==1) && (et1%OFST!=0) && (get_type(et2)==1) && (et2%OFST==0))
    {
        return oper_soma(expr_of_et(et1), oper_neg(expr_of_et(et2)));
    }

    // int var with float var
    if ((get_type(et1)==1) && (et1%OFST!=0) && (get_type(et2)==2) && (et2%OFST!=0))
    {
        add_instr("%s %s\n", i2f, v_name[et1%OFST]);
        add_instr("F_SU1 %s\n"  , v_name[et2%OFST]);
    }

    // int var with float acc
    if ((get_type(et1)==1) && (et1%OFST!=0) && (get_type(et2)==2) && (et2%OFST==0))
    {
        add_instr("%s %s\n", i2f, v_name[et1%OFST]);
        add_instr("SF_SU1\n");
    }

    // int var with comp const (probably never happens, but just in case...)
    if ((get_type(et1)==1) && (et1%OFST!=0) && (get_type(et2)==5))
    {
        return oper_soma(expr_of_et(et1), oper_neg(expr_of_et(et2)));
    }

    // int var with comp var
    if ((get_type(et1)==1) && (et1%OFST!=0) && (get_type(et2)==3) && (et2%OFST!=0))
    {
        get_cmp_ets(et2,&etr,&eti);

        add_instr("%s %s\n", i2f   , v_name[et1%OFST]);
        add_instr("F_SU1 %s\n"     , v_name[etr%OFST]);
        add_instr("PF_NEG_M  %s\n" , v_name[eti%OFST]);
    }

    // int var with comp acc
    if ((get_type(et1)==1) && (et1%OFST!=0) && (get_type(et2)==3) && (et2%OFST==0))
    {
        add_instr("SET aux_var\n");                 // temporarily save the imaginary part
        add_instr("I2F_M %s\n", v_name[et1%OFST]);  // fetch the int while converting to float
        add_instr("SF_SU1\n");                      // add acc with stack
        add_instr("PF_NEG_M aux_var\n");            // push the result onto the stack while bringing the imag back
    }

    // int acc with int var
    if ((get_type(et1)==1) && (et1%OFST==0) && (get_type(et2)==1) && (et2%OFST!=0))
    {
        return oper_soma(expr_of_et(et1), oper_neg(expr_of_et(et2)));
    }

    // int acc with int acc
    if ((get_type(et1)==1) && (et1%OFST==0) && (get_type(et2)==1) && (et2%OFST==0))
    {
        return oper_soma(expr_of_et(et1), oper_neg(expr_of_et(et2)));
    }

    // int acc with float var
    if ((get_type(et1)==1) && (et1%OFST==0) && (get_type(et2)==2) && (et2%OFST!=0))
    {
        add_instr("I2F\n");
        add_instr("F_SU1 %s\n", v_name[et2%OFST]);
    }

    // int acc with float acc
    if ((get_type(et1)==1) && (et1%OFST==0) && (get_type(et2)==2) && (et2%OFST==0))
    {
        add_instr("SET_P aux_var\n");
        add_instr("I2F\n");
        add_instr("F_SU1 aux_var\n");
    }

    // int acc with comp const
    if ((get_type(et1)==1) && (et1%OFST==0) && (get_type(et2)==5))
    {
        return oper_soma(expr_of_et(et1), oper_neg(expr_of_et(et2)));
    }

    // int acc with comp var
    if ((get_type(et1)==1) && (et1%OFST==0) && (get_type(et2)==3) && (et2%OFST!=0))
    {
        get_cmp_ets(et2,&etr,&eti);

        add_instr("I2F\n");
        add_instr("F_SU1 %s\n"   , v_name[etr%OFST]);
        add_instr("PF_NEG_M %s\n", v_name[eti%OFST]);
    }

    // int acc with comp acc
    if ((get_type(et1)==1) && (et1%OFST==0) && (get_type(et2)==3) && (et2%OFST==0))
    {
        add_instr("SET_P    aux_var\n" );
        add_instr("SET_P    aux_var1\n");
        add_instr("I2F\n");
        add_instr("F_SU1    aux_var1\n");
        add_instr("PF_NEG_M aux_var\n" );
    }

    // float var with int var
    if ((get_type(et1)==2) && (et1%OFST!=0) && (get_type(et2)==1) && (et2%OFST!=0))
    {
        add_instr("%s %s\n",i2f, v_name[et2%OFST]);
        add_instr("F_SU2 %s\n" , v_name[et1%OFST]);
    }

    // float var with int acc
    if ((get_type(et1)==2) && (et1%OFST!=0) && (get_type(et2)==1) && (et2%OFST==0))
    {
        add_instr("I2F\n");
        add_instr("F_SU2 %s\n", v_name[et1%OFST]);
    }

    // float var with float var
    if ((get_type(et1)==2) && (et1%OFST!=0) && (get_type(et2)==2) && (et2%OFST!=0))
    {
        add_instr("%s %s\n", ld, v_name[et1%OFST]);
        add_instr("F_SU1 %s\n" , v_name[et2%OFST]);
    }

    // float var with float acc
    if ((get_type(et1)==2) && (et1%OFST!=0) && (get_type(et2)==2) && (et2%OFST==0))
    {
        add_instr("F_SU2 %s\n", v_name[et1%OFST]);
    }

    // float var with comp const (no negative comp const)
    if ((get_type(et1)==2) && (et1%OFST!=0) && (get_type(et2)==5))
    {
        return oper_soma(expr_of_et(et1), oper_neg(expr_of_et(et2)));
    }

    // float var with comp var
    if ((get_type(et1)==2) && (et1%OFST!=0) && (get_type(et2)==3) && (et2%OFST!=0))
    {
        get_cmp_ets(et2,&etr,&eti);

        add_instr("%s %s\n",    ld, v_name[et1%OFST]);
        add_instr("F_SU1 %s\n"    , v_name[etr%OFST]);
        add_instr("PF_NEG_M %s\n" , v_name[eti%OFST]);
    }

    // float var with comp acc
    if ((get_type(et1)==2) && (et1%OFST!=0) && (get_type(et2)==3) && (et2%OFST==0))
    {
        add_instr("SET_P aux_var\n");
        add_instr("F_SU2 %s\n", v_name[et1%OFST]);
        add_instr("PF_NEG_M aux_var\n");
    }

    // float acc with int var
    if ((get_type(et1)==2) && (et1%OFST==0) && (get_type(et2)==1) && (et2%OFST!=0))
    {
        add_instr("P_I2F_M %s\n", v_name[et2%OFST]);
        add_instr("SF_SU2\n");
    }

    // float acc with int acc
    if ((get_type(et1)==2) && (et1%OFST==0) && (get_type(et2)==1) && (et2%OFST==0))
    {
        add_instr("I2F\n");
        add_instr("SF_SU2\n");
    }

    // float acc with float var
    if ((get_type(et1)==2) && (et1%OFST==0) && (get_type(et2)==2) && (et2%OFST!=0))
    {
        add_instr("F_SU1 %s\n", v_name[et2%OFST]);
    }

    // float acc with float acc
    if ((get_type(et1)==2) && (et1%OFST==0) && (get_type(et2)==2) && (et2%OFST==0))
    {
        add_instr("SF_SU2\n");
    }

    // float acc with comp const (does not happen)
    if ((get_type(et1)==2) && (et1%OFST==0) && (get_type(et2)==5))
    {
        return oper_soma(expr_of_et(et1), oper_neg(expr_of_et(et2)));
    }

    // float acc with comp var
    if ((get_type(et1)==2) && (et1%OFST==0) && (get_type(et2)==3) && (et2%OFST!=0))
    {
        get_cmp_ets(et2,&etr,&eti);

        add_instr("F_SU1 %s\n", v_name[etr%OFST]);
        add_instr("PF_NEG_M %s\n", v_name[eti%OFST]);
    }

    // float acc with comp acc
    if ((get_type(et1)==2) && (et1%OFST==0) && (get_type(et2)==3) && (et2%OFST==0))
    {
        add_instr("SET_P aux_var\n");
        add_instr("SF_SU2\n");
        add_instr("PF_NEG_M aux_var\n");
    }

    // comp const with int var
    if ((get_type(et1)==5) && (get_type(et2)==1) && (et2%OFST!=0))
    {
        get_cmp_cst(et1,&etr,&eti);

        add_instr("%s %s\n", i2f, v_name[et2%OFST]);
        add_instr("F_SU2 %s\n"  , v_name[etr%OFST]);
        add_instr("P_LOD %s\n"  , v_name[eti%OFST]);
    }

    // comp const with int acc
    if ((get_type(et1)==5) && (get_type(et2)==1) && (et2%OFST==0))
    {
        get_cmp_cst(et1,&etr,&eti);

        add_instr("I2F\n");
        add_instr("F_SU2 %s\n", v_name[etr%OFST]);
        add_instr("P_LOD %s\n", v_name[eti%OFST]);
    }

    // comp const with float var
    if ((get_type(et1)==5) && (get_type(et2)==2) && (et2%OFST!=0))
    {
        get_cmp_cst(et1,&etr,&eti);

        add_instr("%s %s\n", ld, v_name[et2%OFST]);
        add_instr("F_SU2 %s\n" , v_name[etr%OFST]);
        add_instr("P_LOD %s\n" , v_name[eti%OFST]);
    }

    // comp const with float acc
    if ((get_type(et1)==5) && (get_type(et2)==2) && (et2%OFST==0))
    {
        get_cmp_cst(et1,&etr,&eti);

        add_instr("F_SU2 %s\n", v_name[etr%OFST]);
        add_instr("P_LOD %s\n", v_name[eti%OFST]);
    }

    // comp const with comp const (no comp const subtraction)
    if ((get_type(et1)==5) && (get_type(et2)==5))
    {
        return oper_soma(expr_of_et(et1), oper_neg(expr_of_et(et2)));
    }

    // comp const with comp var
    if ((get_type(et1)==5) && (get_type(et2)==3) && (et2%OFST!=0))
    {
        int et1r,et2r;
        int et1i,et2i;

        get_cmp_cst(et1,&et1r,&et1i);
        get_cmp_ets(et2,&et2r,&et2i);

        add_instr("%s %s\n", ld, v_name[et1r%OFST]);
        add_instr("F_SU1 %s\n" , v_name[et2r%OFST]);

        add_instr("P_LOD %s\n" , v_name[et1i%OFST]);
        add_instr("F_SU1 %s\n" , v_name[et2i%OFST]);
    }

    // comp const with comp acc
    if ((get_type(et1)==5) && (get_type(et2)==3) && (et2%OFST==0))
    {
        get_cmp_cst(et1,&etr,&eti);

        add_instr("SET_P aux_var\n");
        add_instr("F_SU2 %s\n", v_name[etr%OFST]);

        add_instr("P_LOD %s\n", v_name[eti%OFST]);
        add_instr("F_SU1 aux_var\n");
    }

    // comp var with int var
    if ((get_type(et1)==3) && (et1%OFST!=0) && (get_type(et2)==1) && (et2%OFST!=0))
    {
        get_cmp_ets(et1,&etr,&eti);

        add_instr("%s  %s\n", i2f, v_name[et2%OFST]);
        add_instr("F_SU2 %s\n"   , v_name[etr%OFST]);
        add_instr("P_LOD %s\n"   , v_name[eti%OFST]);
    }

    // comp var with int acc
    if ((get_type(et1)==3) && (et1%OFST!=0) && (get_type(et2)==1) && (et2%OFST==0))
    {
        get_cmp_ets(et1,&etr,&eti);

        add_instr("I2F\n");
        add_instr("F_SU2 %s\n", v_name[etr%OFST]);
        add_instr("P_LOD %s\n", v_name[eti%OFST]);
    }

    // comp var with float var
    if ((get_type(et1)==3) && (et1%OFST!=0) && (get_type(et2)==2) && (et2%OFST!=0))
    {
        get_cmp_ets(et1,&etr,&eti);

        add_instr("%s %s\n", ld, v_name[et2%OFST]);
        add_instr("F_SU2 %s\n" , v_name[etr%OFST]);
        add_instr("P_LOD %s\n" , v_name[eti%OFST]);
    }

    // comp var with float acc
    if ((get_type(et1)==3) && (et1%OFST!=0) && (get_type(et2)==2) && (et2%OFST==0))
    {
        get_cmp_ets(et1,&etr,&eti);

        add_instr("F_SU2 %s\n", v_name[etr%OFST]);
        add_instr("P_LOD %s\n", v_name[eti%OFST]);
    }

    // comp var with comp const (no comp const subtraction)
    if ((get_type(et1)==3) && (et1%OFST!=0) && (get_type(et2)==5))
    {
        return oper_soma(expr_of_et(et1), oper_neg(expr_of_et(et2)));
    }

    // comp var with comp var
    if ((get_type(et1)==3) && (et1%OFST!=0) && (get_type(et2)==3) && (et2%OFST!=0))
    {
        int et1r,et2r;
        int et1i,et2i;

        get_cmp_ets(et1,&et1r,&et1i);
        get_cmp_ets(et2,&et2r,&et2i);

        add_instr("%s %s\n", ld, v_name[et1r%OFST]);
        add_instr("F_SU1 %s\n" , v_name[et2r%OFST]);

        add_instr("P_LOD %s\n" , v_name[et1i%OFST]);
        add_instr("F_SU1 %s\n" , v_name[et2i%OFST]);
    }

    // comp var with comp acc
    if ((get_type(et1)==3) && (et1%OFST!=0) && (get_type(et2)==3) && (et2%OFST==0))
    {
        get_cmp_ets(et1,&etr,&eti);

        add_instr("SET_P aux_var\n");
        add_instr("F_SU2 %s\n", v_name[etr%OFST]);

        add_instr("P_LOD %s\n", v_name[eti%OFST]);
        add_instr("F_SU1 aux_var\n");
    }

    // comp acc with int var
    if ((get_type(et1)==3) && (et1%OFST==0) && (get_type(et2)==1) && (et2%OFST!=0))
    {
        add_instr("SET_P aux_var\n");
        add_instr("P_I2F_M %s\n", v_name[et2%OFST]);
        add_instr("SF_SU2\n");
        add_instr("P_LOD aux_var\n");
    }

    // comp acc with int acc
    if ((get_type(et1)==3) && (et1%OFST==0) && (get_type(et2)==1) && (et2%OFST==0))
    {
        add_instr("SET_P   aux_var\n" );
        add_instr("SET_P   aux_var1\n");
        add_instr("P_I2F_M aux_var\n" );
        add_instr("SF_SU2\n");
        add_instr("P_LOD   aux_var1\n");
    }

    // comp acc with float var
    if ((get_type(et1)==3) && (et1%OFST==0) && (get_type(et2)==2) && (et2%OFST!=0))
    {
        add_instr("SET_P aux_var\n");
        add_instr("F_SU1 %s\n", v_name[et2%OFST]);
        add_instr("P_LOD aux_var\n");
    }

    // comp acc with float acc
    if ((get_type(et1)==3) && (et1%OFST==0) && (get_type(et2)==2) && (et2%OFST==0))
    {
        add_instr("SET_P aux_var\n" );
        add_instr("SET_P aux_var1\n");
        add_instr("F_SU1 aux_var\n" );
        add_instr("P_LOD aux_var1\n");
    }

    // comp acc with comp const (no comp const subtraction)
    if ((get_type(et1)==3) && (et1%OFST==0) && (get_type(et2)==5))
    {
        return oper_soma(expr_of_et(et1), oper_neg(expr_of_et(et2)));
    }

    // comp acc with comp var
    if ((get_type(et1)==3) && (et1%OFST==0) && (get_type(et2)==3) && (et2%OFST!=0))
    {
        get_cmp_ets(et2,&etr,&eti);

        add_instr("SET_P aux_var\n");
        add_instr("F_SU1 %s\n", v_name[etr%OFST]);

        add_instr("P_LOD aux_var\n");
        add_instr("F_SU1 %s\n", v_name[eti%OFST]);
    }

    // comp acc with comp acc
    if ((get_type(et1)==3) && (et1%OFST==0) && (get_type(et2)==3) && (et2%OFST==0))
    {
        add_instr("SET_P aux_var \n");
        add_instr("SET_P aux_var1\n");
        add_instr("SET_P aux_var2\n");
        add_instr("F_SU1 aux_var1\n");

        add_instr("P_LOD aux_var2\n");
        add_instr("F_SU1 aux_var \n");
    }

    acc_ok = 1;

    int type;
         if ((get_type(et1) > 2) || (get_type(et2) > 2))
         type = 3;
    else if ((get_type(et1) > 1) || (get_type(et2) > 1))
         type = 2;
    else type = 1;

    return expr_make(type, 0);
}

// multiplies two numbers
expr oper_mult(expr e1, expr e2)
{
    int et1 = expr_to_et(e1);
    int et2 = expr_to_et(e2);
    int etr, eti;

    char ld [10]; if (acc_ok == 0) strcpy(ld ,"LOD"  ); else strcpy(ld ,"P_LOD"  );
    char i2f[10]; if (acc_ok == 0) strcpy(i2f,"I2F_M"); else strcpy(i2f,"P_I2F_M");

    // int var with int var
    if ((get_type(et1)==1) && (et1%OFST!=0) && (get_type(et2)==1) && (et2%OFST!=0))
    {
        add_instr("%s %s\n" , ld, v_name[et1%OFST]);
        add_instr("MLT %s\n",     v_name[et2%OFST]);
    }

    // int var with int acc
    if ((get_type(et1)==1) && (et1%OFST!=0) && (get_type(et2)==1) && (et2%OFST==0))
    {
        add_instr("MLT %s\n", v_name[et1%OFST]);
    }

    // int var with float var
    if ((get_type(et1)==1) && (et1%OFST!=0) && (get_type(et2)==2) && (et2%OFST!=0))
    {
        add_instr("%s %s\n", i2f, v_name[et1%OFST]);
        add_instr("F_MLT %s\n"  , v_name[et2%OFST]);
    }

    // int var with float acc
    if ((get_type(et1)==1) && (et1%OFST!=0) && (get_type(et2)==2) && (et2%OFST==0))
    {
        add_instr("P_I2F_M %s\n", v_name[et1%OFST]);
        add_instr("SF_MLT\n");
    }

    // int var with comp const
    if ((get_type(et1)==1) && (et1%OFST!=0) && (get_type(et2)==5))
    {
        get_cmp_cst(et2,&etr,&eti);

        add_instr("%s %s\n", i2f, v_name[et1%OFST]);
        add_instr("F_MLT %s\n"  , v_name[etr%OFST]);

        add_instr("P_I2F_M %s\n", v_name[et1%OFST]);
        add_instr("F_MLT %s\n"  , v_name[eti%OFST]);
    }

    // int var with comp var
    if ((get_type(et1)==1) && (et1%OFST!=0) && (get_type(et2)==3) && (et2%OFST!=0))
    {
        get_cmp_ets(et2,&etr,&eti);

        add_instr("%s %s\n", i2f, v_name[et1%OFST]);
        add_instr("F_MLT %s\n"  , v_name[etr%OFST]);

        add_instr("P_I2F_M %s\n", v_name[et1%OFST]);
        add_instr("F_MLT %s\n"  , v_name[eti%OFST]);
    }

    // int var with comp acc
    if ((get_type(et1)==1) && (et1%OFST!=0) && (get_type(et2)==3) && (et2%OFST==0))
    {
        add_instr("SET_P aux_var\n");
        add_instr("P_I2F_M %s\n", v_name[et1%OFST]);
        add_instr("SF_MLT\n");

        add_instr("P_LOD aux_var\n");
        add_instr("P_I2F_M %s\n", v_name[et1%OFST]);
        add_instr("SF_MLT\n");
    }

    // int acc with int var
    if ((get_type(et1)==1) && (et1%OFST==0) && (get_type(et2)==1) && (et2%OFST!=0))
    {
        add_instr("MLT %s\n", v_name[et2%OFST]);
    }

    // int acc with int acc
    if ((get_type(et1)==1) && (et1%OFST==0) && (get_type(et2)==1) && (et2%OFST==0))
    {
        add_instr("S_MLT\n");
    }

    // int acc with float var
    if ((get_type(et1)==1) && (et1%OFST==0) && (get_type(et2)==2) && (et2%OFST!=0))
    {
        add_instr("I2F\n");
        add_instr("F_MLT %s\n", v_name[et2%OFST]);
    }

    // int acc with float acc
    if ((get_type(et1)==1) && (et1%OFST==0) && (get_type(et2)==2) && (et2%OFST==0))
    {
        add_instr("SET_P aux_var\n");
        add_instr("I2F\n");
        add_instr("F_MLT aux_var\n");
    }

    // int acc with comp const
    if ((get_type(et1)==1) && (et1%OFST==0) && (get_type(et2)==5))
    {
        get_cmp_cst(et2,&etr,&eti);

        add_instr("I2F\n");
        add_instr("SET   aux_var\n");
        add_instr("F_MLT %s\n", v_name[etr%OFST]);
        add_instr("P_LOD aux_var\n");
        add_instr("F_MLT %s\n", v_name[eti%OFST]);
    }

    // int acc with comp var
    if ((get_type(et1)==1) && (et1%OFST==0) && (get_type(et2)==3) && (et2%OFST!=0))
    {
        get_cmp_ets(et2,&etr,&eti);

        add_instr("I2F\n");
        add_instr("SET   aux_var\n");
        add_instr("F_MLT %s\n", v_name[etr%OFST]);
        add_instr("P_LOD aux_var\n");
        add_instr("F_MLT %s\n", v_name[eti%OFST]);
    }

    // int acc with comp acc
    if ((get_type(et1)==1) && (et1%OFST==0) && (get_type(et2)==3) && (et2%OFST==0))
    {
        add_instr("SET_P aux_var \n");
        add_instr("SET_P aux_var1\n");
        add_instr("I2F\n");
        add_instr("SET   aux_var2\n");
        add_instr("F_MLT aux_var1\n");
        add_instr("P_LOD aux_var \n");
        add_instr("F_MLT aux_var2\n");
    }

    // float var with int var
    if ((get_type(et1)==2) && (et1%OFST!=0) && (get_type(et2)==1) && (et2%OFST!=0))
    {
        add_instr("%s %s\n" ,i2f, v_name[et2%OFST]);
        add_instr("F_MLT %s\n",   v_name[et1%OFST]);
    }

    // float var with int acc
    if ((get_type(et1)==2) && (et1%OFST!=0) && (get_type(et2)==1) && (et2%OFST==0))
    {
        add_instr("I2F\n");
        add_instr("F_MLT %s\n", v_name[et1%OFST]);
    }

    // float var with float var
    if ((get_type(et1)==2) && (et1%OFST!=0) && (get_type(et2)==2) && (et2%OFST!=0))
    {
        add_instr("%s %s\n", ld, v_name[et1%OFST]);
        add_instr("F_MLT %s\n",  v_name[et2%OFST]);
    }

    // float var with float acc
    if ((get_type(et1)==2) && (et1%OFST!=0) && (get_type(et2)==2) && (et2%OFST==0))
    {
        add_instr("F_MLT %s\n", v_name[et1%OFST]);
    }

    // float var with comp const
    if ((get_type(et1)==2) && (et1%OFST!=0) && (get_type(et2)==5))
    {
        get_cmp_cst(et2,&etr,&eti);

        add_instr("%s %s\n", ld, v_name[et1%OFST]);
        add_instr("F_MLT %s\n",  v_name[etr%OFST]);

        add_instr("P_LOD %s\n",  v_name[et1%OFST]);
        add_instr("F_MLT %s\n",  v_name[eti%OFST]);
    }

    // float var with comp var
    if ((get_type(et1)==2) && (et1%OFST!=0) && (get_type(et2)==3) && (et2%OFST!=0))
    {
        get_cmp_ets(et2,&etr,&eti);

        add_instr("%s %s\n", ld, v_name[et1%OFST]);
        add_instr("F_MLT %s\n",  v_name[etr%OFST]);

        add_instr("P_LOD %s\n",  v_name[et1%OFST]);
        add_instr("F_MLT %s\n",  v_name[eti%OFST]);
    }

    // float var with comp acc
    if ((get_type(et1)==2) && (et1%OFST!=0) && (get_type(et2)==3) && (et2%OFST==0))
    {
        add_instr("SET_P aux_var\n");
        add_instr("F_MLT %s\n", v_name[et1%OFST]);
        add_instr("P_LOD aux_var\n");
        add_instr("F_MLT %s\n", v_name[et1%OFST]);
    }

    // float acc with int var
    if ((get_type(et1)==2) && (et1%OFST==0) && (get_type(et2)==1) && (et2%OFST!=0))
    {
        add_instr("P_I2F_M %s\n", v_name[et2%OFST]);
        add_instr("SF_MLT\n");
    }

    // float acc with int acc
    if ((get_type(et1)==2) && (et1%OFST==0) && (get_type(et2)==1) && (et2%OFST==0))
    {
        add_instr("I2F\n");
        add_instr("SF_MLT\n");
    }

    // float acc with float var
    if ((get_type(et1)==2) && (et1%OFST==0) && (get_type(et2)==2) && (et2%OFST!=0))
    {
        add_instr("F_MLT %s\n", v_name[et2%OFST]);
    }

    // float acc with float acc
    if ((get_type(et1)==2) && (et1%OFST==0) && (get_type(et2)==2) && (et2%OFST==0))
    {
        add_instr("SF_MLT\n");
    }

    // float acc with comp const
    if ((get_type(et1)==2) && (et1%OFST==0) && (get_type(et2)==5))
    {
        get_cmp_cst(et2,&etr,&eti);

        add_instr("SET   aux_var\n");
        add_instr("F_MLT %s\n", v_name[etr%OFST]);
        add_instr("P_LOD %s\n", v_name[eti%OFST]);
        add_instr("F_MLT aux_var\n");
    }

    // float acc with comp var
    if ((get_type(et1)==2) && (et1%OFST==0) && (get_type(et2)==3) && (et2%OFST!=0))
    {
        get_cmp_ets(et2,&etr,&eti);

        add_instr("SET   aux_var\n");
        add_instr("F_MLT %s\n", v_name[etr%OFST]);
        add_instr("P_LOD %s\n", v_name[eti%OFST]);
        add_instr("F_MLT aux_var\n");
    }

    // float acc with comp acc
    if ((get_type(et1)==2) && (et1%OFST==0) && (get_type(et2)==3) && (et2%OFST==0))
    {
        add_instr("SET_P aux_var \n");
        add_instr("SET_P aux_var1\n");
        add_instr("SET   aux_var2\n");
        add_instr("F_MLT aux_var1\n");
        add_instr("P_LOD aux_var \n");
        add_instr("F_MLT aux_var2\n");
    }

    // comp const with int var
    if ((get_type(et1)==5) && (get_type(et2)==1) && (et2%OFST!=0))
    {
        get_cmp_cst(et1,&etr,&eti);

        add_instr("%s %s\n", i2f, v_name[et2%OFST]);
        add_instr("F_MLT %s\n"  , v_name[etr%OFST]);
        add_instr("P_I2F_M %s\n", v_name[et2%OFST]);
        add_instr("F_MLT %s\n"  , v_name[eti%OFST]);
    }

    // comp const with int acc
    if ((get_type(et1)==5) && (get_type(et2)==1) && (et2%OFST==0))
    {
        get_cmp_cst(et1,&etr,&eti);

        add_instr("I2F\n");
        add_instr("SET   aux_var\n");
        add_instr("F_MLT %s\n"  , v_name[etr%OFST]);
        add_instr("P_LOD aux_var\n");
        add_instr("F_MLT %s\n"  , v_name[eti%OFST]);
    }

    // comp const with float var
    if ((get_type(et1)==5) && (get_type(et2)==2) && (et2%OFST!=0))
    {
        get_cmp_cst(et1,&etr,&eti);

        add_instr("%s %s\n", ld, v_name[et2%OFST]);
        add_instr("F_MLT %s\n" , v_name[etr%OFST]);
        add_instr("P_LOD %s\n" , v_name[et2%OFST]);
        add_instr("F_MLT %s\n" , v_name[eti%OFST]);
    }

    // comp const with float acc
    if ((get_type(et1)==5) && (get_type(et2)==2) && (et2%OFST==0))
    {
        get_cmp_cst(et1,&etr,&eti);

        add_instr("SET   aux_var\n");
        add_instr("F_MLT %s\n"  , v_name[etr%OFST]);
        add_instr("P_LOD aux_var\n");
        add_instr("F_MLT %s\n"  , v_name[eti%OFST]);
    }

    // comp const with comp const
    if ((get_type(et1)==5) && (get_type(et2)==5))
    {
        int et1r,et1i; get_cmp_cst(et1,&et1r,&et1i);
        int et2r,et2i; get_cmp_cst(et2,&et2r,&et2i);

        add_instr("%s %s\n", ld, v_name[et1r%OFST]);
        add_instr("F_MLT %s\n" , v_name[et2r%OFST]);
        add_instr("P_LOD %s\n" , v_name[et1i%OFST]);
        add_instr("F_MLT %s\n" , v_name[et2i%OFST]);
        add_instr("SF_SU2\n");

        add_instr("P_LOD %s\n" , v_name[et1r%OFST]);
        add_instr("F_MLT %s\n" , v_name[et2i%OFST]);
        add_instr("P_LOD %s\n" , v_name[et1i%OFST]);
        add_instr("F_MLT %s\n" , v_name[et2r%OFST]);
        add_instr("SF_ADD\n");
    }

    // comp const with comp var
    if ((get_type(et1)==5) && (get_type(et2)==3) && (et2%OFST!=0))
    {
        int et1r,et1i; get_cmp_cst(et1,&et1r,&et1i);
        int et2r,et2i; get_cmp_ets(et2,&et2r,&et2i);

        add_instr("%s %s\n", ld, v_name[et1r%OFST]);
        add_instr("F_MLT %s\n" , v_name[et2r%OFST]);
        add_instr("P_LOD %s\n" , v_name[et1i%OFST]);
        add_instr("F_MLT %s\n" , v_name[et2i%OFST]);
        add_instr("SF_SU2\n");

        add_instr("P_LOD %s\n" , v_name[et1r%OFST]);
        add_instr("F_MLT %s\n" , v_name[et2i%OFST]);
        add_instr("P_LOD %s\n" , v_name[et1i%OFST]);
        add_instr("F_MLT %s\n" , v_name[et2r%OFST]);
        add_instr("SF_ADD\n");
    }

    // comp const with comp acc
    if ((get_type(et1)==5) && (get_type(et2)==3) && (et2%OFST==0))
    {
        get_cmp_cst(et1,&etr,&eti);

        add_instr("SET_P aux_var \n");
        add_instr("SET   aux_var1\n");

        add_instr("F_MLT %s\n", v_name[etr%OFST]);
        add_instr("P_LOD %s\n", v_name[eti%OFST]);
        add_instr("F_MLT aux_var \n");
        add_instr("SF_SU2\n");

        add_instr("P_LOD %s\n" , v_name[etr%OFST]);
        add_instr("F_MLT aux_var \n");
        add_instr("P_LOD %s\n" , v_name[eti%OFST]);
        add_instr("F_MLT aux_var1\n");
        add_instr("SF_ADD\n");
    }

    // comp var with int var
    if ((get_type(et1)==3) && (et1%OFST!=0) && (get_type(et2)==1) && (et2%OFST!=0))
    {
        get_cmp_ets(et1,&etr,&eti);

        add_instr("%s %s\n", i2f, v_name[et2%OFST]);
        add_instr("F_MLT %s\n"  , v_name[etr%OFST]);
        add_instr("P_I2F_M %s\n", v_name[et2%OFST]);
        add_instr("F_MLT %s\n"  , v_name[eti%OFST]);
    }

    // comp var with int acc
    if ((get_type(et1)==3) && (et1%OFST!=0) && (get_type(et2)==1) && (et2%OFST==0))
    {
        get_cmp_ets(et1,&etr,&eti);

        add_instr("I2F\n");
        add_instr("SET   aux_var\n");
        add_instr("F_MLT %s\n"  , v_name[etr%OFST]);
        add_instr("P_LOD aux_var\n");
        add_instr("F_MLT %s\n"  , v_name[eti%OFST]);
    }

    // comp var with float var
    if ((get_type(et1)==3) && (et1%OFST!=0) && (get_type(et2)==2) && (et2%OFST!=0))
    {
        get_cmp_ets(et1,&etr,&eti);

        add_instr("%s %s\n", ld, v_name[et2%OFST]);
        add_instr("F_MLT %s\n" , v_name[etr%OFST]);
        add_instr("P_LOD %s\n" , v_name[et2%OFST]);
        add_instr("F_MLT %s\n" , v_name[eti%OFST]);
    }

    // comp var with float acc
    if ((get_type(et1)==3) && (et1%OFST!=0) && (get_type(et2)==2) && (et2%OFST==0))
    {
        get_cmp_ets(et1,&etr,&eti);

        add_instr("SET   aux_var\n");
        add_instr("F_MLT %s\n"  , v_name[etr%OFST]);
        add_instr("P_LOD aux_var\n");
        add_instr("F_MLT %s\n"  , v_name[eti%OFST]);
    }

    // comp var with comp const
    if ((get_type(et1)==3) && (et1%OFST!=0) && (get_type(et2)==5))
    {
        int et1r,et1i; get_cmp_ets(et1,&et1r,&et1i);
        int et2r,et2i; get_cmp_cst(et2,&et2r,&et2i);

        add_instr("%s %s\n", ld, v_name[et1r%OFST]);
        add_instr("F_MLT %s\n" , v_name[et2r%OFST]);
        add_instr("P_LOD %s\n" , v_name[et1i%OFST]);
        add_instr("F_MLT %s\n" , v_name[et2i%OFST]);
        add_instr("SF_SU2\n");

        add_instr("P_LOD %s\n" , v_name[et1r%OFST]);
        add_instr("F_MLT %s\n" , v_name[et2i%OFST]);
        add_instr("P_LOD %s\n" , v_name[et1i%OFST]);
        add_instr("F_MLT %s\n" , v_name[et2r%OFST]);
        add_instr("SF_ADD\n");
    }

    // comp var with comp var
    if ((get_type(et1)==3) && (et1%OFST!=0) && (get_type(et2)==3) && (et2%OFST!=0))
    {
        int et1r,et1i; get_cmp_ets(et1,&et1r,&et1i);
        int et2r,et2i; get_cmp_ets(et2,&et2r,&et2i);

        add_instr("%s %s\n", ld, v_name[et1r%OFST]);
        add_instr("F_MLT %s\n" , v_name[et2r%OFST]);
        add_instr("P_LOD %s\n" , v_name[et1i%OFST]);
        add_instr("F_MLT %s\n" , v_name[et2i%OFST]);
        add_instr("SF_SU2\n");

        add_instr("P_LOD %s\n" , v_name[et1r%OFST]);
        add_instr("F_MLT %s\n" , v_name[et2i%OFST]);
        add_instr("P_LOD %s\n" , v_name[et1i%OFST]);
        add_instr("F_MLT %s\n" , v_name[et2r%OFST]);
        add_instr("SF_ADD\n");
    }

    // comp var with comp acc
    if ((get_type(et1)==3) && (et1%OFST!=0) && (get_type(et2)==3) && (et2%OFST==0))
    {
        get_cmp_ets(et1,&etr,&eti);

        add_instr("SET_P aux_var \n");
        add_instr("SET   aux_var1\n");

        add_instr("F_MLT %s\n", v_name[etr%OFST]);
        add_instr("P_LOD %s\n", v_name[eti%OFST]);
        add_instr("F_MLT aux_var \n");
        add_instr("SF_SU2\n");

        add_instr("P_LOD %s\n" , v_name[etr%OFST]);
        add_instr("F_MLT aux_var \n");
        add_instr("P_LOD %s\n" , v_name[eti%OFST]);
        add_instr("F_MLT aux_var1\n");
        add_instr("SF_ADD\n");
    }

    // comp acc with int var
    if ((get_type(et1)==3) && (et1%OFST==0) && (get_type(et2)==1) && (et2%OFST!=0))
    {
        add_instr("SET_P aux_var\n" );
        add_instr("P_I2F_M %s\n", v_name[et2%OFST]);
        add_instr("SET   aux_var1\n");
        add_instr("SF_MLT\n");
        add_instr("P_LOD aux_var\n" );
        add_instr("F_MLT aux_var1\n");
    }

    // comp acc with int acc
    if ((get_type(et1)==3) && (et1%OFST==0) && (get_type(et2)==1) && (et2%OFST==0))
    {
        add_instr("I2F\n");
        add_instr("SET_P aux_var\n" );
        add_instr("SET_P aux_var1\n");
        add_instr("F_MLT aux_var\n" );
        add_instr("P_LOD aux_var1\n");
        add_instr("F_MLT aux_var\n" );
    }

    // comp acc with float var
    if ((get_type(et1)==3) && (et1%OFST==0) && (get_type(et2)==2) && (et2%OFST!=0))
    {
        add_instr("SET_P aux_var\n");
        add_instr("F_MLT %s\n", v_name[et2%OFST]);
        add_instr("P_LOD aux_var\n");
        add_instr("F_MLT %s\n", v_name[et2%OFST]);
    }

    // comp acc with float acc
    if ((get_type(et1)==3) && (et1%OFST==0) && (get_type(et2)==2) && (et2%OFST==0))
    {
        add_instr("SET_P aux_var\n" );
        add_instr("SET_P aux_var1\n");
        add_instr("F_MLT aux_var\n" );
        add_instr("P_LOD aux_var1\n");
        add_instr("F_MLT aux_var\n ");
    }

    // comp acc with comp const
    if ((get_type(et1)==3) && (et1%OFST==0) && (get_type(et2)==5))
    {
        get_cmp_cst(et2,&etr,&eti);

        add_instr("SET_P aux_var \n");
        add_instr("SET   aux_var1\n");
        add_instr("F_MLT %s\n", v_name[etr%OFST]);
        add_instr("P_LOD aux_var \n");
        add_instr("F_MLT %s\n", v_name[eti%OFST]);
        add_instr("SF_SU2\n");

        add_instr("P_LOD aux_var \n");
        add_instr("F_MLT %s\n", v_name[etr%OFST]);
        add_instr("P_LOD aux_var1\n");
        add_instr("F_MLT %s\n", v_name[eti%OFST]);
        add_instr("SF_ADD\n");
    }

    // comp acc with comp var
    if ((get_type(et1)==3) && (et1%OFST==0) && (get_type(et2)==3) && (et2%OFST!=0))
    {
        get_cmp_ets(et2,&etr,&eti);

        add_instr("SET_P aux_var \n");
        add_instr("SET   aux_var1\n");
        add_instr("F_MLT %s\n", v_name[etr%OFST]);
        add_instr("P_LOD aux_var \n");
        add_instr("F_MLT %s\n", v_name[eti%OFST]);
        add_instr("SF_SU2\n");

        add_instr("P_LOD aux_var \n");
        add_instr("F_MLT %s\n", v_name[etr%OFST]);
        add_instr("P_LOD aux_var1\n");
        add_instr("F_MLT %s\n", v_name[eti%OFST]);
        add_instr("SF_ADD\n");
    }

    // comp acc with comp acc
    if ((get_type(et1)==3) && (et1%OFST==0) && (get_type(et2)==3) && (et2%OFST==0))
    {
        add_instr("SET_P aux_var \n");
        add_instr("SET_P aux_var1\n");
        add_instr("SET_P aux_var2\n");
        add_instr("SET   aux_var3\n");

        add_instr("F_MLT aux_var1\n");
        add_instr("P_LOD aux_var \n");
        add_instr("F_MLT aux_var2\n");
        add_instr("SF_SU2\n");

        add_instr("P_LOD aux_var1\n");
        add_instr("F_MLT aux_var2\n");
        add_instr("P_LOD aux_var \n");
        add_instr("F_MLT aux_var3\n");
        add_instr("SF_ADD\n");
    }

    acc_ok = 1;

    int type;
         if ((get_type(et1) > 2) || (get_type(et2) > 2))
         type = 3;
    else if ((get_type(et1) > 1) || (get_type(et2) > 1))
         type = 2;
    else type = 1;

    return expr_make(type, 0);
}

// division between two numbers
expr oper_divi(expr e1, expr e2)
{
    int et1 = expr_to_et(e1);
    int et2 = expr_to_et(e2);
    int etr, eti;

    char ld [10]; if (acc_ok == 0) strcpy(ld ,"LOD"  ); else strcpy(ld ,"P_LOD"  );
    char i2f[10]; if (acc_ok == 0) strcpy(i2f,"I2F_M"); else strcpy(i2f,"P_I2F_M");

    // int var with int var
    if ((get_type(et1)==1) && (et1%OFST!=0) && (get_type(et2)==1) && (et2%OFST!=0))
    {
        add_instr("%s %s\n" , ld, v_name[et2%OFST]);
        add_instr("DIV %s\n",     v_name[et1%OFST]);    
    }

    // int var with int acc
    if ((get_type(et1)==1) && (et1%OFST!=0) && (get_type(et2)==1) && (et2%OFST==0))
    {
        add_instr("DIV %s\n", v_name[et1%OFST]);
    }

    // int var with float var
    if ((get_type(et1)==1) && (et1%OFST!=0) && (get_type(et2)==2) && (et2%OFST!=0))
    {
        add_instr("%s %s\n" , i2f, v_name[et1%OFST]);
        add_instr("P_LOD %s\n"   , v_name[et2%OFST]);
        add_instr("SF_DIV\n");
    }

    // int var with float acc
    if ((get_type(et1)==1) && (et1%OFST!=0) && (get_type(et2)==2) && (et2%OFST==0))
    {
        add_instr("SET   aux_var\n");
        add_instr("I2F_M %s\n", v_name[et1%OFST]);
        add_instr("P_LOD aux_var\n");
        add_instr("SF_DIV\n");
    }

    // int var with comp const
    if ((get_type(et1)==1) && (et1%OFST!=0) && (get_type(et2)==5))
    {
        get_cmp_cst(et2,&etr,&eti);

        oper_mult(expr_of_et(etr), expr_of_et(etr));           // parte real ao quadrado
        oper_mult(expr_of_et(eti), expr_of_et(eti));           // parte imag ao quadrado
        oper_soma(expr_of_et(2*OFST), expr_of_et(2*OFST));     // soma os quadrados
        add_instr("SET aux_var\n");   // save the result

        acc_ok = 0;                   // libera acumulador

        oper_mult(expr_of_et(et1), expr_of_et(etr));           // multiply int with real part
        add_instr("P_LOD aux_var\n"); // fetch the denominator
        oper_divi(expr_of_et(2*OFST), expr_of_et(2*OFST));     // faz a divisao

        oper_mult(expr_of_et(et1), expr_of_et(eti));           // multiply int with imag part
        add_instr("P_LOD aux_var\n"); // fetch the denominator
        oper_divi(expr_of_et(2*OFST), expr_of_et(2*OFST));     // faz a divisao
        oper_neg (expr_of_et(2*OFST));            // nega a parte imaginaria
    }

    // int var with comp var
    if ((get_type(et1)==1) && (et1%OFST!=0) && (get_type(et2)==3) && (et2%OFST!=0))
    {
        get_cmp_ets(et2,&etr,&eti);

        oper_mult(expr_of_et(etr), expr_of_et(etr));           // parte real ao quadrado
        oper_mult(expr_of_et(eti), expr_of_et(eti));           // parte imag ao quadrado
        oper_soma(expr_of_et(2*OFST), expr_of_et(2*OFST));     // soma os quadrados
        add_instr("SET aux_var\n");   // save the result

        acc_ok = 0;                   // libera acumulador

        oper_mult(expr_of_et(et1), expr_of_et(etr));           // multiply int with real part
        add_instr("P_LOD aux_var\n"); // fetch the denominator
        oper_divi(expr_of_et(2*OFST), expr_of_et(2*OFST));     // faz a divisao

        oper_mult(expr_of_et(et1), expr_of_et(eti));           // multiply int with imag part
        add_instr("P_LOD aux_var\n"); // fetch the denominator
        oper_divi(expr_of_et(2*OFST), expr_of_et(2*OFST));     // faz a divisao
        oper_neg (expr_of_et(2*OFST));            // nega a parte imaginaria
    }

    // int var with comp acc
    if ((get_type(et1)==1) && (et1%OFST!=0) && (get_type(et2)==3) && (et2%OFST==0))
    {
        add_instr("SET_P aux_var \n"); // save the imaginary part
        add_instr("SET   aux_var1\n"); // save the real part

        add_instr("F_MLT aux_var1\n"); // parte real ao quadrado
        add_instr("P_LOD aux_var \n"); // fetch the imaginary part
        add_instr("F_MLT aux_var \n"); // parte imag ao quadrado
        add_instr("SF_ADD        \n"); // soma os quadrados
        add_instr("SET   aux_var2\n"); // save the squared magnitude

        add_instr("I2F_M %s\n", v_name[et1%OFST]); 
        add_instr("SET   aux_var3\n"); // save the float
        add_instr("F_MLT aux_var1\n");
        add_instr("P_LOD aux_var2\n"); // fetch the squared magnitude
        oper_divi(expr_of_et(2*OFST), expr_of_et(2*OFST));      // faz a divisao

        add_instr("P_LOD aux_var3\n"); // fetch the float
        add_instr("F_MLT aux_var \n");
        add_instr("P_LOD aux_var2\n"); // fetch the squared magnitude
        oper_divi(expr_of_et(2*OFST), expr_of_et(2*OFST));      // faz a divisao
        oper_neg (expr_of_et(2*OFST));
    }

    // int acc with int var
    if ((get_type(et1)==1) && (et1%OFST==0) && (get_type(et2)==1) && (et2%OFST!=0))
    {
        add_instr("%s %s\n", ld, v_name[et2%OFST]);
        add_instr("S_DIV\n");
    }

    // int acc with int acc
    if ((get_type(et1)==1) && (et1%OFST==0) && (get_type(et2)==1) && (et2%OFST==0))
    {
        add_instr("S_DIV\n");
    }

    // int acc with float var
    if ((get_type(et1)==1) && (et1%OFST==0) && (get_type(et2)==2) && (et2%OFST!=0))
    {
        add_instr("I2F\n");
        add_instr("P_LOD %s\n", v_name[et2%OFST]);
        add_instr("SF_DIV\n");
    }

    // int acc with float acc
    if ((get_type(et1)==1) && (et1%OFST==0) && (get_type(et2)==2) && (et2%OFST==0))
    {
        add_instr("SET_P aux_soma\n");
        add_instr("I2F\n");
        add_instr("P_LOD aux_soma\n");
        add_instr("SF_DIV\n");
    }

    // int acc with comp const
    if ((get_type(et1)==1) && (et1%OFST==0) && (get_type(et2)==5))
    {
        get_cmp_cst(et2,&etr,&eti);

        add_instr("I2F\n");
        add_instr("SET aux_var\n");
        acc_ok = 0;

        oper_mult(expr_of_et(etr), expr_of_et(etr));            // parte real ao quadrado
        oper_mult(expr_of_et(eti), expr_of_et(eti));            // parte imag ao quadrado
        oper_soma(expr_of_et(2*OFST), expr_of_et(2*OFST));      // soma os quadrados
        add_instr("SET   aux_var1\n"); // save the result

        add_instr("LOD   aux_var\n");
        oper_mult(expr_of_et(2*OFST), expr_of_et(etr));         // multiply float with real part
        add_instr("P_LOD aux_var1\n"); // fetch the denominator
        oper_divi(expr_of_et(2*OFST), expr_of_et(2*OFST));      // faz a divisao

        add_instr("P_LOD  aux_var\n");
        oper_mult(expr_of_et(2*OFST), expr_of_et(eti));         // multiply float with imag part
        add_instr("P_LOD aux_var1\n"); // fetch the denominator
        oper_divi(expr_of_et(2*OFST), expr_of_et(2*OFST));      // faz a divisao
        oper_neg (expr_of_et(2*OFST));
    }

    // int acc with comp var
    if ((get_type(et1)==1) && (et1%OFST==0) && (get_type(et2)==3) && (et2%OFST!=0))
    {
        get_cmp_ets(et2,&etr,&eti);

        add_instr("I2F\n");
        add_instr("SET   aux_var\n");
        acc_ok = 0;

        oper_mult(expr_of_et(etr), expr_of_et(etr));             // parte real ao quadrado
        oper_mult(expr_of_et(eti), expr_of_et(eti));             // parte imag ao quadrado
        oper_soma(expr_of_et(2*OFST), expr_of_et(2*OFST));       // soma os quadrados
        add_instr("SET   aux_var1\n");  // save the result

        add_instr("LOD   aux_var\n" );
        oper_mult(expr_of_et(2*OFST), expr_of_et(etr));          // multiply float with real part
        add_instr("P_LOD aux_var1\n");  // fetch the denominator
        oper_divi(expr_of_et(2*OFST), expr_of_et(2*OFST));       // faz a divisao

        add_instr("P_LOD  aux_var\n" );
        oper_mult(expr_of_et(2*OFST), expr_of_et(eti));          // multiply float with imag part
        add_instr("P_LOD  aux_var1\n"); // fetch the denominator
        oper_divi(expr_of_et(2*OFST), expr_of_et(2*OFST));       // faz a divisao
        oper_neg (expr_of_et(2*OFST));
    }

    // int acc with comp acc
    if ((get_type(et1)==1) && (et1%OFST==0) && (get_type(et2)==3) && (et2%OFST==0))
    {
        add_instr("SET_P aux_var \n"); // save the imaginary part
        add_instr("SET_P aux_var1\n"); // save the real part
        add_instr("I2F           \n");
        add_instr("SET   aux_var2\n"); // save the float

        add_instr("LOD   aux_var1\n"); // push the real part onto the stack
        add_instr("F_MLT aux_var1\n");
        add_instr("P_LOD aux_var \n"); // fetch the imaginary part
        add_instr("F_MLT aux_var \n");
        add_instr("SF_ADD        \n");
        add_instr("SET   aux_var3\n"); // save the squared magnitude

        add_instr("LOD   aux_var2\n"); // load the float
        add_instr("F_MLT aux_var1\n");
        add_instr("P_LOD aux_var3\n"); // fetch the squared magnitude
        oper_divi(expr_of_et(2*OFST), expr_of_et(2*OFST));      // faz a divisao

        add_instr("P_LOD aux_var2\n"); // fetch the float
        add_instr("F_MLT aux_var \n");
        add_instr("P_LOD aux_var3\n"); // fetch the squared magnitude
        oper_divi(expr_of_et(2*OFST), expr_of_et(2*OFST));      // faz a divisao
        oper_neg (expr_of_et(2*OFST));
    }

    // float var with int var
    if ((get_type(et1)==2) && (et1%OFST!=0) && (get_type(et2)==1) && (et2%OFST!=0))
    {
        add_instr("%s %s\n", i2f, v_name[et2%OFST]);
        add_instr("F_DIV %s\n"  , v_name[et1%OFST]);
    }

    // float var with int acc
    if ((get_type(et1)==2) && (et1%OFST!=0) && (get_type(et2)==1) && (et2%OFST==0))
    {
        add_instr("I2F\n");
        add_instr("F_DIV %s\n", v_name[et1%OFST]);
    }

    // float var with float var
    if ((get_type(et1)==2) && (et1%OFST!=0) && (get_type(et2)==2) && (et2%OFST!=0))
    {
        add_instr("%s %s\n" , ld, v_name[et2%OFST]);
        add_instr("F_DIV %s\n"  , v_name[et1%OFST]);
    }

    // float var with float acc
    if ((get_type(et1)==2) && (et1%OFST!=0) && (get_type(et2)==2) && (et2%OFST==0))
    {
        add_instr("F_DIV %s\n", v_name[et1%OFST]);
    }

    // float var with comp const
    if ((get_type(et1)==2) && (et1%OFST!=0) && (get_type(et2)==5))
    {
        get_cmp_cst(et2,&etr,&eti);

        oper_mult(expr_of_et(etr), expr_of_et(etr));           // parte real ao quadrado
        oper_mult(expr_of_et(eti), expr_of_et(eti));           // parte imag ao quadrado
        oper_soma(expr_of_et(2*OFST), expr_of_et(2*OFST));     // soma os quadrados
        add_instr("SET   aux_var\n"); // save the result
        acc_ok = 0;

        oper_mult(expr_of_et(et1), expr_of_et(etr));           // multiply float with real part
        add_instr("P_LOD aux_var\n"); // fetch the denominator
        oper_divi(expr_of_et(2*OFST), expr_of_et(2*OFST));     // faz a divisao

        oper_mult(expr_of_et(et1), expr_of_et(eti));           // multiply float with imag part
        add_instr("P_LOD aux_var\n"); // fetch the denominator
        oper_divi(expr_of_et(2*OFST), expr_of_et(2*OFST));     // faz a divisao
        oper_neg (expr_of_et(2*OFST));
    }

    // float var with comp var
    if ((get_type(et1)==2) && (et1%OFST!=0) && (get_type(et2)==3) && (et2%OFST!=0))
    {
        get_cmp_ets(et2,&etr,&eti);

        oper_mult(expr_of_et(etr), expr_of_et(etr));           // parte real ao quadrado
        oper_mult(expr_of_et(eti), expr_of_et(eti));           // parte imag ao quadrado
        oper_soma(expr_of_et(2*OFST), expr_of_et(2*OFST));     // soma os quadrados
        add_instr("SET   aux_var\n"); // save the result
        acc_ok = 0;

        oper_mult(expr_of_et(et1), expr_of_et(etr));           // multiply float with real part
        add_instr("P_LOD aux_var\n"); // fetch the denominator
        oper_divi(expr_of_et(2*OFST), expr_of_et(2*OFST));     // faz a divisao

        oper_mult(expr_of_et(et1), expr_of_et(eti));           // multiply float with imag part
        add_instr("P_LOD aux_var\n"); // fetch the denominator
        oper_divi(expr_of_et(2*OFST), expr_of_et(2*OFST));     // faz a divisao
        oper_neg (expr_of_et(2*OFST));
    }

    // float var with comp acc
    if ((get_type(et1)==2) && (et1%OFST!=0) && (get_type(et2)==3) && (et2%OFST==0))
    {
        add_instr("SET_P aux_var \n");             // save the imaginary part
        add_instr("SET   aux_var1\n");             // save the real part
        add_instr("F_MLT aux_var1\n");
        add_instr("P_LOD aux_var \n");             // fetch the imaginary part
        add_instr("F_MLT aux_var \n");
        add_instr("SF_ADD        \n");
        add_instr("SET   aux_var2\n");             // save the squared magnitude

        add_instr("LOD %s\n"  , v_name[et1%OFST]); // load the float
        add_instr("F_MLT aux_var1\n");
        add_instr("P_LOD aux_var2\n");             // fetch the squared magnitude
        oper_divi(expr_of_et(2*OFST), expr_of_et(2*OFST));                  // faz a divisao

        add_instr("P_LOD %s\n", v_name[et1%OFST]); // load the float
        add_instr("F_MLT aux_var \n");
        add_instr("P_LOD aux_var2\n");             // fetch the squared magnitude
        oper_divi(expr_of_et(2*OFST), expr_of_et(2*OFST));                  // faz a divisao
        oper_neg (expr_of_et(2*OFST));
    }

    // float acc with int var
    if ((get_type(et1)==2) && (et1%OFST==0) && (get_type(et2)==1) && (et2%OFST!=0))
    {
        add_instr("P_I2F_M %s\n", v_name[et2%OFST]);
        add_instr("SF_DIV\n");
    }

    // float acc with int acc
    if ((get_type(et1)==2) && (et1%OFST==0) && (get_type(et2)==1) && (et2%OFST==0))
    {
        add_instr("I2F\n");
        add_instr("SF_DIV\n");
    }

    // float acc with float var
    if ((get_type(et1)==2) && (et1%OFST==0) && (get_type(et2)==2) && (et2%OFST!=0))
    {
        add_instr("P_LOD %s\n", v_name[et2%OFST]);
        add_instr("SF_DIV\n");
    }

    // float acc with float acc
    if ((get_type(et1)==2) && (et1%OFST==0) && (get_type(et2)==2) && (et2%OFST==0))
    {
        add_instr("SF_DIV\n");
    }

    // float acc with comp const
    if ((get_type(et1)==2) && (et1%OFST==0) && (get_type(et2)==5))
    {
        get_cmp_cst(et2,&etr,&eti);

        add_instr("SET   aux_var\n");
        acc_ok = 0;

        oper_mult(expr_of_et(etr), expr_of_et(etr));            // parte real ao quadrado
        oper_mult(expr_of_et(eti), expr_of_et(eti));            // parte imag ao quadrado
        oper_soma(expr_of_et(2*OFST), expr_of_et(2*OFST));      // soma os quadrados
        add_instr("SET   aux_var1\n"); // save the result

        add_instr("LOD   aux_var \n");
        oper_mult(expr_of_et(2*OFST), expr_of_et(etr));         // multiply float with real part
        add_instr("P_LOD aux_var1\n"); // fetch the denominator
        oper_divi(expr_of_et(2*OFST), expr_of_et(2*OFST));      // faz a divisao

        add_instr("P_LOD aux_var \n");
        oper_mult(expr_of_et(2*OFST), expr_of_et(eti));         // multiply float with imag part
        add_instr("P_LOD aux_var1\n"); // fetch the denominator
        oper_divi(expr_of_et(2*OFST), expr_of_et(2*OFST));      // faz a divisao
        oper_neg (expr_of_et(2*OFST));
    }

    // float acc with comp var
    if ((get_type(et1)==2) && (et1%OFST==0) && (get_type(et2)==3) && (et2%OFST!=0))
    {
        get_cmp_ets(et2,&etr,&eti);

        add_instr("SET aux_var\n");
        acc_ok = 0;

        oper_mult(expr_of_et(etr), expr_of_et(etr));            // parte real ao quadrado
        oper_mult(expr_of_et(eti), expr_of_et(eti));            // parte imag ao quadrado
        oper_soma(expr_of_et(2*OFST), expr_of_et(2*OFST));      // soma os quadrados
        add_instr("SET   aux_var1\n"); // save the result

        add_instr("LOD   aux_var\n");
        oper_mult(expr_of_et(2*OFST), expr_of_et(etr));         // multiply float with real part
        add_instr("P_LOD aux_var1\n"); // fetch the denominator
        oper_divi(expr_of_et(2*OFST), expr_of_et(2*OFST));      // faz a divisao

        add_instr("P_LOD aux_var\n");
        oper_mult(expr_of_et(2*OFST), expr_of_et(eti));         // multiply float with imag part
        add_instr("P_LOD aux_var1\n"); // fetch the denominator
        oper_divi(expr_of_et(2*OFST), expr_of_et(2*OFST));      // faz a divisao
        oper_neg (expr_of_et(2*OFST));
    }

    // float acc with comp acc
    if ((get_type(et1)==2) && (et1%OFST==0) && (get_type(et2)==3) && (et2%OFST==0))
    {
        add_instr("SET_P aux_var \n"); // save the imaginary part
        add_instr("SET_P aux_var1\n"); // save the real part
        add_instr("SET   aux_var2\n"); // save the float

        add_instr("LOD   aux_var1\n"); // push the real part onto the stack
        add_instr("F_MLT aux_var1\n");
        add_instr("P_LOD aux_var \n"); // fetch the imaginary part
        add_instr("F_MLT aux_var \n");
        add_instr("SF_ADD        \n");
        add_instr("SET   aux_var3\n"); // save the squared magnitude

        add_instr("LOD   aux_var2\n"); // load the float
        add_instr("F_MLT aux_var1\n");
        add_instr("P_LOD aux_var3\n"); // fetch the squared magnitude
        oper_divi(expr_of_et(2*OFST), expr_of_et(2*OFST));      // faz a divisao

        add_instr("P_LOD aux_var2\n"); // fetch the float
        add_instr("F_MLT aux_var \n");
        add_instr("P_LOD aux_var3\n"); // fetch the squared magnitude
        oper_divi(expr_of_et(2*OFST), expr_of_et(2*OFST));      // faz a divisao
        oper_neg (expr_of_et(2*OFST));
    }

    // comp const with int var
    if ((get_type(et1)==5) && (get_type(et2)==1) && (et2%OFST!=0))
    {
        get_cmp_cst(et1,&etr,&eti);

        oper_divi(expr_of_et(etr), expr_of_et(et2));
        oper_divi(expr_of_et(eti), expr_of_et(et2));
    }

    // comp const with int acc
    if ((get_type(et1)==5) && (get_type(et2)==1) && (et2%OFST==0))
    {
        get_cmp_cst(et1,&etr,&eti);

        add_instr("I2F\n");
        add_instr("SET   aux_var\n");
        oper_divi(expr_of_et(etr), expr_of_et(2*OFST));
        add_instr("P_LOD aux_var\n");
        oper_divi(expr_of_et(eti), expr_of_et(2*OFST));
    }

    // comp const with float var
    if ((get_type(et1)==5) && (get_type(et2)==2) && (et2%OFST!=0))
    {
        get_cmp_cst(et1,&etr,&eti);

        oper_divi(expr_of_et(etr), expr_of_et(et2));
        oper_divi(expr_of_et(eti), expr_of_et(et2));
    }

    // comp const with float acc
    if ((get_type(et1)==5) && (get_type(et2)==2) && (et2%OFST==0))
    {
        get_cmp_cst(et1,&etr,&eti);

        add_instr("SET   aux_var\n");
        oper_divi(expr_of_et(etr), expr_of_et(2*OFST));
        add_instr("P_LOD aux_var\n");
        oper_divi(expr_of_et(eti), expr_of_et(2*OFST));
    }

    // comp const with comp const
    if ((get_type(et1)==5) && (get_type(et2)==5))
    {
        int et1r,et2r;
        int et1i,et2i;

        get_cmp_cst(et1,&et1r,&et1i);
        get_cmp_cst(et2,&et2r,&et2i);

        oper_mult(expr_of_et(et2r), expr_of_et(et2r));
        oper_mult(expr_of_et(et2i), expr_of_et(et2i));
        oper_soma(expr_of_et(2*OFST), expr_of_et(2*OFST));
        add_instr("SET   aux_var\n");
        acc_ok = 0;

        oper_mult(expr_of_et(et1r), expr_of_et(et2r));
        oper_mult(expr_of_et(et1i), expr_of_et(et2i));
        oper_soma(expr_of_et(2*OFST), expr_of_et(2*OFST));
        add_instr("P_LOD aux_var\n");
        oper_divi(expr_of_et(2*OFST), expr_of_et(2*OFST));

        oper_mult(expr_of_et(et1i), expr_of_et(et2r));
        oper_mult(expr_of_et(et1r), expr_of_et(et2i));
        oper_subt(expr_of_et(2*OFST), expr_of_et(2*OFST));
        add_instr("P_LOD aux_var\n");
        oper_divi(expr_of_et(2*OFST), expr_of_et(2*OFST));
    }

    // comp const with comp var
    if ((get_type(et1)==5) && (get_type(et2)==3) && (et2%OFST!=0))
    {
        int et1r,et2r;
        int et1i,et2i;

        get_cmp_cst(et1,&et1r,&et1i);
        get_cmp_ets(et2,&et2r,&et2i);

        oper_mult(expr_of_et(et2r), expr_of_et(et2r));
        oper_mult(expr_of_et(et2i), expr_of_et(et2i));
        oper_soma(expr_of_et(2*OFST), expr_of_et(2*OFST));
        add_instr("SET aux_var\n");
        acc_ok = 0;

        oper_mult(expr_of_et(et1r), expr_of_et(et2r));
        oper_mult(expr_of_et(et1i), expr_of_et(et2i));
        oper_soma(expr_of_et(2*OFST), expr_of_et(2*OFST));
        add_instr("P_LOD aux_var\n");
        oper_divi(expr_of_et(2*OFST), expr_of_et(2*OFST));

        oper_mult(expr_of_et(et1i), expr_of_et(et2r));
        oper_mult(expr_of_et(et1r), expr_of_et(et2i));
        oper_subt(expr_of_et(2*OFST), expr_of_et(2*OFST));
        add_instr("P_LOD aux_var\n");
        oper_divi(expr_of_et(2*OFST), expr_of_et(2*OFST));
    }

    // comp const with comp acc
    if ((get_type(et1)==5) && (get_type(et2)==3) && (et2%OFST==0))
    {
        get_cmp_cst(et1,&etr,&eti);

        add_instr("SET_P aux_var \n");
        add_instr("SET   aux_var1\n");
        add_instr("F_MLT aux_var1\n");
        add_instr("P_LOD aux_var \n");
        add_instr("F_MLT aux_var \n");
        oper_soma(expr_of_et(2*OFST), expr_of_et(2*OFST));
        add_instr("SET   aux_var2\n");

        add_instr("LOD   aux_var1\n");
        oper_mult(expr_of_et(etr), expr_of_et(2*OFST));
        add_instr("P_LOD aux_var \n");
        oper_mult(expr_of_et(eti), expr_of_et(2*OFST));
        oper_soma(expr_of_et(2*OFST), expr_of_et(2*OFST));
        add_instr("P_LOD aux_var2\n");
        oper_divi(expr_of_et(2*OFST), expr_of_et(2*OFST));

        add_instr("P_LOD aux_var1\n");
        oper_mult(expr_of_et(eti), expr_of_et(2*OFST));
        add_instr("P_LOD aux_var \n");
        oper_mult(expr_of_et(etr), expr_of_et(2*OFST));
        oper_subt(expr_of_et(2*OFST), expr_of_et(2*OFST));
        add_instr("P_LOD aux_var2\n");
        oper_divi(expr_of_et(2*OFST), expr_of_et(2*OFST));
    }

    // comp var with int var
    if ((get_type(et1)==3) && (et1%OFST!=0) && (get_type(et2)==1) && (et2%OFST!=0))
    {
        get_cmp_ets(et1,&etr,&eti);

        oper_divi(expr_of_et(etr), expr_of_et(et2));
        oper_divi(expr_of_et(eti), expr_of_et(et2));
    }

    // comp var with int acc
    if ((get_type(et1)==3) && (et1%OFST!=0) && (get_type(et2)==1) && (et2%OFST==0))
    {
        get_cmp_ets(et1,&etr,&eti);

        add_instr("I2F\n");
        add_instr("SET   aux_var\n");
        oper_divi(expr_of_et(etr), expr_of_et(2*OFST));
        add_instr("P_LOD aux_var\n");
        oper_divi(expr_of_et(eti), expr_of_et(2*OFST));
    }

    // comp var with float var
    if ((get_type(et1)==3) && (et1%OFST!=0) && (get_type(et2)==2) && (et2%OFST!=0))
    {
        get_cmp_ets(et1,&etr,&eti);

        oper_divi(expr_of_et(etr), expr_of_et(et2));
        oper_divi(expr_of_et(eti), expr_of_et(et2));
    }

    // comp var with float acc
    if ((get_type(et1)==3) && (et1%OFST!=0) && (get_type(et2)==2) && (et2%OFST==0))
    {
        get_cmp_ets(et1,&etr,&eti);

        add_instr("SET   aux_var\n");
        oper_divi(expr_of_et(etr), expr_of_et(2*OFST));
        add_instr("P_LOD aux_var\n");
        oper_divi(expr_of_et(eti), expr_of_et(2*OFST));
    }

    // comp var with comp const
    if ((get_type(et1)==3) && (et1%OFST!=0) && (get_type(et2)==5))
    {
        int et1r,et2r;
        int et1i,et2i;

        get_cmp_ets(et1,&et1r,&et1i);
        get_cmp_cst(et2,&et2r,&et2i);

        oper_mult(expr_of_et(et2r), expr_of_et(et2r));
        oper_mult(expr_of_et(et2i), expr_of_et(et2i));
        oper_soma(expr_of_et(2*OFST), expr_of_et(2*OFST));
        add_instr("SET   aux_var\n");
        acc_ok = 0;

        oper_mult(expr_of_et(et1r), expr_of_et(et2r));
        oper_mult(expr_of_et(et1i), expr_of_et(et2i));
        oper_soma(expr_of_et(2*OFST), expr_of_et(2*OFST));
        add_instr("P_LOD aux_var\n");
        oper_divi(expr_of_et(2*OFST), expr_of_et(2*OFST));

        oper_mult(expr_of_et(et1i), expr_of_et(et2r));
        oper_mult(expr_of_et(et1r), expr_of_et(et2i));
        oper_subt(expr_of_et(2*OFST), expr_of_et(2*OFST));
        add_instr("P_LOD aux_var\n");
        oper_divi(expr_of_et(2*OFST), expr_of_et(2*OFST));
    }

    // comp var with comp var
    if ((get_type(et1)==3) && (et1%OFST!=0) && (get_type(et2)==3) && (et2%OFST!=0))
    {
        int et1r,et2r;
        int et1i,et2i;

        get_cmp_ets(et1,&et1r,&et1i);
        get_cmp_ets(et2,&et2r,&et2i);

        oper_mult(expr_of_et(et2r), expr_of_et(et2r));
        oper_mult(expr_of_et(et2i), expr_of_et(et2i));
        oper_soma(expr_of_et(2*OFST), expr_of_et(2*OFST));
        add_instr("SET   aux_var\n");
        acc_ok = 0;

        oper_mult(expr_of_et(et1r), expr_of_et(et2r));
        oper_mult(expr_of_et(et1i), expr_of_et(et2i));
        oper_soma(expr_of_et(2*OFST), expr_of_et(2*OFST));
        add_instr("P_LOD aux_var\n");
        oper_divi(expr_of_et(2*OFST), expr_of_et(2*OFST));

        oper_mult(expr_of_et(et1i), expr_of_et(et2r));
        oper_mult(expr_of_et(et1r), expr_of_et(et2i));
        oper_subt(expr_of_et(2*OFST), expr_of_et(2*OFST));
        add_instr("P_LOD aux_var\n");
        oper_divi(expr_of_et(2*OFST), expr_of_et(2*OFST));
    }

    // comp var with comp acc
    if ((get_type(et1)==3) && (et1%OFST!=0) && (get_type(et2)==3) && (et2%OFST==0))
    {
        get_cmp_ets(et1,&etr,&eti);

        add_instr("SET_P aux_var \n");
        add_instr("SET   aux_var1\n");
        add_instr("F_MLT aux_var1\n");
        add_instr("P_LOD aux_var \n");
        add_instr("F_MLT aux_var \n");
        oper_soma(expr_of_et(2*OFST), expr_of_et(2*OFST));
        add_instr("SET   aux_var2\n");

        add_instr("LOD   aux_var1\n");
        oper_mult(expr_of_et(etr), expr_of_et(2*OFST));
        add_instr("P_LOD aux_var \n");
        oper_mult(expr_of_et(eti), expr_of_et(2*OFST));
        oper_soma(expr_of_et(2*OFST), expr_of_et(2*OFST));
        add_instr("P_LOD aux_var2\n");
        oper_divi(expr_of_et(2*OFST), expr_of_et(2*OFST));

        add_instr("P_LOD aux_var1\n");
        oper_mult(expr_of_et(eti), expr_of_et(2*OFST));
        add_instr("P_LOD aux_var \n");
        oper_mult(expr_of_et(etr), expr_of_et(2*OFST));
        oper_subt(expr_of_et(2*OFST), expr_of_et(2*OFST));
        add_instr("P_LOD aux_var2\n");
        oper_divi(expr_of_et(2*OFST), expr_of_et(2*OFST));
    }

    // comp acc with int var
    if ((get_type(et1)==3) && (et1%OFST==0) && (get_type(et2)==1) && (et2%OFST!=0))
    {
        add_instr("SET_P aux_var \n");
        add_instr("P_I2F_M %s\n", v_name[et2%OFST]);
        add_instr("SET   aux_var1\n");
        oper_divi(expr_of_et(2*OFST), expr_of_et(2*OFST));
        add_instr("P_LOD aux_var \n");
        add_instr("P_LOD aux_var1\n");
        oper_divi(expr_of_et(2*OFST), expr_of_et(2*OFST));
    }

    // comp acc with int acc
    if ((get_type(et1)==3) && (et1%OFST==0) && (get_type(et2)==1) && (et2%OFST==0))
    {
        add_instr("I2F\n");
        add_instr("SET_P aux_var \n");
        add_instr("SET_P aux_var1\n");
        add_instr("P_LOD aux_var \n");
        oper_divi(expr_of_et(2*OFST), expr_of_et(2*OFST));
        add_instr("P_LOD aux_var1\n");
        add_instr("P_LOD aux_var \n");
        oper_divi(expr_of_et(2*OFST), expr_of_et(2*OFST));
    }

    // comp acc with float var
    if ((get_type(et1)==3) && (et1%OFST==0) && (get_type(et2)==2) && (et2%OFST!=0))
    {
        add_instr("SET_P aux_var\n");
        add_instr("P_LOD %s\n", v_name[et2%OFST]);
        oper_divi(expr_of_et(2*OFST), expr_of_et(2*OFST));
        add_instr("P_LOD aux_var\n");
        add_instr("P_LOD %s\n", v_name[et2%OFST]);
        oper_divi(expr_of_et(2*OFST), expr_of_et(2*OFST));
    }

    // comp acc with float acc
    if ((get_type(et1)==3) && (et1%OFST==0) && (get_type(et2)==2) && (et2%OFST==0))
    {
        add_instr("SET_P aux_var \n");
        add_instr("SET_P aux_var1\n");
        add_instr("P_LOD aux_var \n");
        oper_divi(expr_of_et(2*OFST), expr_of_et(2*OFST));
        add_instr("P_LOD aux_var1\n");
        add_instr("P_LOD aux_var \n");
        oper_divi(expr_of_et(2*OFST), expr_of_et(2*OFST));
    }

    // comp acc with comp const
    if ((get_type(et1)==3) && (et1%OFST==0) && (get_type(et2)==5))
    {
        get_cmp_cst(et2,&etr,&eti);

        add_instr("SET_P aux_var \n");
        add_instr("SET   aux_var1\n");
        acc_ok = 0;

        oper_mult(expr_of_et(etr), expr_of_et(etr));
        oper_mult(expr_of_et(eti), expr_of_et(eti));
        oper_soma(expr_of_et(2*OFST), expr_of_et(2*OFST));
        add_instr("SET   aux_var2\n");

        add_instr("LOD   aux_var1\n");
        oper_mult(expr_of_et(etr), expr_of_et(2*OFST));
        add_instr("P_LOD aux_var \n");
        oper_mult(expr_of_et(eti), expr_of_et(2*OFST));
        oper_soma(expr_of_et(2*OFST), expr_of_et(2*OFST));
        add_instr("P_LOD aux_var2\n");
        oper_divi(expr_of_et(2*OFST), expr_of_et(2*OFST));

        add_instr("P_LOD aux_var1\n");
        oper_mult(expr_of_et(eti), expr_of_et(2*OFST));
        add_instr("P_LOD aux_var \n");
        oper_mult(expr_of_et(etr), expr_of_et(2*OFST));
        oper_subt(expr_of_et(2*OFST), expr_of_et(2*OFST));
        add_instr("P_LOD aux_var2\n");
        oper_divi(expr_of_et(2*OFST), expr_of_et(2*OFST));
        oper_neg (expr_of_et(2*OFST));
    }

    // comp acc with comp var
    if ((get_type(et1)==3) && (et1%OFST==0) && (get_type(et2)==3) && (et2%OFST!=0))
    {
        get_cmp_ets(et2,&etr,&eti);

        add_instr("SET_P aux_var \n");
        add_instr("SET   aux_var1\n");
        acc_ok = 0;

        oper_mult(expr_of_et(etr), expr_of_et(etr));
        oper_mult(expr_of_et(eti), expr_of_et(eti));
        oper_soma(expr_of_et(2*OFST), expr_of_et(2*OFST));
        add_instr("SET   aux_var2\n");

        add_instr("LOD   aux_var1\n");
        oper_mult(expr_of_et(etr), expr_of_et(2*OFST));
        add_instr("P_LOD aux_var \n");
        oper_mult(expr_of_et(eti), expr_of_et(2*OFST));
        oper_soma(expr_of_et(2*OFST), expr_of_et(2*OFST));
        add_instr("P_LOD aux_var2\n");
        oper_divi(expr_of_et(2*OFST), expr_of_et(2*OFST));

        add_instr("P_LOD aux_var1\n");
        oper_mult(expr_of_et(eti), expr_of_et(2*OFST));
        add_instr("P_LOD aux_var \n");
        oper_mult(expr_of_et(etr), expr_of_et(2*OFST));
        oper_subt(expr_of_et(2*OFST), expr_of_et(2*OFST));
        add_instr("P_LOD aux_var2\n");
        oper_divi(expr_of_et(2*OFST), expr_of_et(2*OFST));
        oper_neg (expr_of_et(2*OFST));
    }

    // comp acc with comp acc
    if ((get_type(et1)==3) && (et1%OFST==0) && (get_type(et2)==3) && (et2%OFST==0))
    {
        add_instr("SET_P aux_var \n");
        add_instr("SET_P aux_var1\n");
        add_instr("SET_P aux_var2\n");
        add_instr("SET   aux_var3\n");

        add_instr("LOD   aux_var1\n");
        add_instr("F_MLT aux_var1\n");
        add_instr("P_LOD aux_var \n");
        add_instr("F_MLT aux_var \n");
        add_instr("SF_ADD        \n");
        add_instr("SET   aux_var4\n");

        add_instr("LOD   aux_var3\n");
        add_instr("F_MLT aux_var1\n");
        add_instr("P_LOD aux_var2\n");
        add_instr("F_MLT aux_var \n");
        add_instr("SF_ADD        \n");
        add_instr("P_LOD aux_var4\n");
        oper_divi(expr_of_et(2*OFST), expr_of_et(2*OFST));

        add_instr("P_LOD aux_var3\n");
        add_instr("F_MLT aux_var \n");
        add_instr("P_LOD aux_var2\n");
        add_instr("F_MLT aux_var1\n");
        oper_subt(expr_of_et(2*OFST), expr_of_et(2*OFST));
        add_instr("P_LOD aux_var4\n");
        oper_divi(expr_of_et(2*OFST), expr_of_et(2*OFST));
        oper_neg (expr_of_et(2*OFST));
    }

    acc_ok = 1;

    int type;
         if ((get_type(et1) > 2) || (get_type(et2) > 2))
         type = 3;
    else if ((get_type(et1) > 1) || (get_type(et2) > 1))
         type = 2;
    else type = 1;

    return expr_make(type, 0);
}

// division remainder
expr oper_mod(expr e1, expr e2)
{
    int et1 = expr_to_et(e1);
    int et2 = expr_to_et(e2);

    if ((get_type(et1) > 1) || (get_type(et2) > 1))
        {fprintf(stderr, MSG_ERR_MOD_NON_INT, line_num+1); exit(EXIT_FAILURE);}

    char ld[10]; if (acc_ok == 0) strcpy(ld,"LOD"); else strcpy(ld,"P_LOD");

    // int var with int var
    if ((get_type(et1) == 1) && (et1%OFST != 0) && (get_type(et2) == 1) && (et2%OFST != 0))
    {
        add_instr("%s %s\n" , ld, v_name[et2%OFST]);
        add_instr("MOD %s\n",     v_name[et1%OFST]);
    }

    // int var with int acc
    if ((get_type(et1) == 1) && (et1%OFST != 0) && (get_type(et2) == 1) && (et2%OFST == 0))
    {
        add_instr("MOD %s\n", v_name[et1%OFST]);
    }

    // int acc with int var
    if ((get_type(et1) == 1) && (et1%OFST == 0) && (get_type(et2) == 1) && (et2%OFST != 0))
    {
        add_instr("P_LOD %s\n", v_name[et2%OFST]);
        add_instr("S_MOD\n");
    }

    // int acc with int acc
    if ((get_type(et1) == 1) && (et1%OFST == 0) && (get_type(et2) == 1) && (et2%OFST == 0))
    {
        add_instr("S_MOD\n");
    }

    acc_ok = 1;

    return expr_make(1, 0); // int in the accumulator
}

// ----------------------------------------------------------------------------
// comparison operations ------------------------------------------------------
// ----------------------------------------------------------------------------

// compares greater-than, less-than and equal-to
// - separar o igual pra economizar clock
// - review to avoid unnecessary stack use!
expr oper_cmp(expr e1, expr e2, int type)
{
    int et1 = expr_to_et(e1);
    int et2 = expr_to_et(e2);

    char ld [10]; if (acc_ok == 0) strcpy(ld , "LOD" ); else strcpy(ld ,"P_LOD"  );
    char i2f[10]; if (acc_ok == 0) strcpy(i2f,"I2F_M"); else strcpy(i2f,"P_I2F_M");

    char op[16] = "";  // initialize so an unhandled 'type' produces a deterministic empty mnemonic instead of stack garbage
    switch (type)
    {
        case 0: strcpy(op, "LES"); break;
        case 1: strcpy(op, "GRE"); break;
        case 2: strcpy(op, "EQU"); break;
        default:
            fprintf(stderr, "oper_cmp: invalid type %d at line %d\n", type, line_num+1);
            exit(EXIT_FAILURE);
    }

    // int var with int var
    if ((get_type(et1)==1) && (et1%OFST!=0) && (get_type(et2)==1) && (et2%OFST!=0))
    {
        add_instr("%s %s\n", ld, v_name[et2%OFST]);
        add_instr("%s %s\n", op, v_name[et1%OFST]);
    }

    // int var with int acc
    if ((get_type(et1)==1) && (et1%OFST!=0) && (get_type(et2)==1) && (et2%OFST==0))
    {
        add_instr("%s %s\n", op, v_name[et1%OFST]);
    }

    // int var with float var
    if ((get_type(et1)==1) && (et1%OFST!=0) && (get_type(et2)==2) && (et2%OFST!=0))
    {
        add_instr("%s %s\n", i2f, v_name[et1%OFST]);
        add_instr("P_LOD %s\n"  , v_name[et2%OFST]);

        if (strcmp(op,"EQU")==0) add_instr("S_EQU\n"); else add_instr("SF_%s\n", op);
    }

    // int var with float acc
    if ((get_type(et1)==1) && (et1%OFST!=0) && (get_type(et2)==2) && (et2%OFST==0))
    {
        add_instr("SET   aux_var\n");
        add_instr("I2F_M %s\n", v_name[et1%OFST]);
        add_instr("P_LOD aux_var\n");
        
        if (strcmp(op,"EQU")==0) add_instr("S_EQU\n"); else add_instr("SF_%s\n", op);
    }

    // int var with comp const
    if ((get_type(et1)==1) && (et1%OFST!=0) && (get_type(et2)==5))
    {
        fprintf(stdout, MSG_WARN_CMP_INT_COMP, line_num+1);

        oper_mult(expr_of_et(et1), expr_of_et(et1));
        exec_mod2(et2);
        oper_cmp(expr_of_et(OFST), expr_of_et(2*OFST), type);
    }

    // int var with comp var
    if ((get_type(et1)==1) && (et1%OFST!=0) && (get_type(et2)==3) && (et2%OFST!=0))
    {
        fprintf(stdout, MSG_WARN_CMP_INT_COMP, line_num+1);

        oper_mult(expr_of_et(et1), expr_of_et(et1));
        exec_mod2(et2);
        oper_cmp(expr_of_et(OFST), expr_of_et(2*OFST), type);
    }

    // int var with comp acc
    if ((get_type(et1)==1) && (et1%OFST!=0) && (get_type(et2)==3) && (et2%OFST==0))
    {
        fprintf(stdout, MSG_WARN_CMP_INT_COMP, line_num+1);

        add_instr("SET_P aux_var \n");
        add_instr("SET   aux_var1\n");
        acc_ok = 0;

        oper_mult(expr_of_et(et1), expr_of_et(et1));
        add_instr("P_LOD aux_var1\n");
        add_instr("P_LOD aux_var \n");
        exec_mod2(3*OFST);
        oper_cmp(expr_of_et(OFST), expr_of_et(2*OFST), type);
    }

    // int acc with int var
    if ((get_type(et1)==1) && (et1%OFST==0) && (get_type(et2)==1) && (et2%OFST!=0))
    {
        add_instr("%s %s\n", ld, v_name[et2%OFST]);
        add_instr("S_%s\n", op);
    }

    // int acc with int acc
    if ((get_type(et1)==1) && (et1%OFST==0) && (get_type(et2)==1) && (et2%OFST==0))
    {
        add_instr("S_%s\n", op);
    }

    // int acc with float var
    if ((get_type(et1)==1) && (et1%OFST==0) && (get_type(et2)==2) && (et2%OFST!=0))
    {
        add_instr("I2F\n");
        add_instr("P_LOD %s\n", v_name[et2%OFST]);
        
        if (strcmp(op,"EQU")==0) add_instr("S_EQU\n"); else add_instr("SF_%s\n", op);
    }

    // int acc with float acc
    if ((get_type(et1)==1) && (et1%OFST==0) && (get_type(et2)==2) && (et2%OFST==0))
    {
        add_instr("SET_P aux_var\n");
        add_instr("I2F\n");
        add_instr("P_LOD aux_var\n");
        
        if (strcmp(op,"EQU")==0) add_instr("S_EQU\n"); else add_instr("SF_%s\n", op);
    }

    // int acc with comp const
    if ((get_type(et1)==1) && (et1%OFST==0) && (get_type(et2)==5))
    {
        fprintf(stdout, MSG_WARN_CMP_INT_COMP, line_num+1);

        add_instr("SET   aux_var\n");
        add_instr("P_LOD aux_var\n");
        oper_mult(expr_of_et(et1), expr_of_et(et1));
        exec_mod2(et2);
        oper_cmp(expr_of_et(OFST), expr_of_et(2*OFST), type);
    }

    // int acc with comp var
    if ((get_type(et1)==1) && (et1%OFST==0) && (get_type(et2)==3) && (et2%OFST!=0))
    {
        fprintf(stdout, MSG_WARN_CMP_INT_COMP, line_num+1);

        add_instr("SET   aux_var\n");
        add_instr("P_LOD aux_var\n");
        oper_mult(expr_of_et(et1), expr_of_et(et1));
        exec_mod2(et2);
        oper_cmp(expr_of_et(OFST), expr_of_et(2*OFST), type);
    }

    // int acc with comp acc
    if ((get_type(et1)==1) && (et1%OFST==0) && (get_type(et2)==3) && (et2%OFST==0))
    {
        fprintf(stdout, MSG_WARN_CMP_INT_COMP, line_num+1);

        add_instr("SET_P aux_var \n");
        add_instr("SET_P aux_var1\n");
        add_instr("SET   aux_var2\n");

        add_instr("P_LOD aux_var2\n");
        oper_mult(expr_of_et(et1), expr_of_et(et1));
        add_instr("P_LOD aux_var1\n");
        add_instr("P_LOD aux_var \n");
        exec_mod2(3*OFST);
        oper_cmp(expr_of_et(OFST), expr_of_et(2*OFST), type);
    }

    // float var with int var
    if ((get_type(et1)==2) && (et1%OFST!=0) && (get_type(et2)==1) && (et2%OFST!=0))
    {
        add_instr("%s %s\n", ld , v_name[et1%OFST]);
        add_instr("P_I2F_M %s\n", v_name[et2%OFST]);
        
        if (strcmp(op,"EQU")==0) add_instr("S_EQU\n"); else add_instr("SF_%s\n", op);
    }

    // float var with int acc
    if ((get_type(et1)==2) && (et1%OFST!=0) && (get_type(et2)==1) && (et2%OFST==0))
    {
        add_instr("I2F\n");
        add_instr("SET   aux_var\n");
        add_instr("LOD %s\n", v_name[et1%OFST]);
        add_instr("P_LOD aux_var\n");
        
        if (strcmp(op,"EQU")==0) add_instr("S_EQU\n"); else add_instr("SF_%s\n", op);
    }

    // float var with float var
    if ((get_type(et1)==2) && (et1%OFST!=0) && (get_type(et2)==2) && (et2%OFST!=0))
    {
        add_instr("%s %s\n"   , ld, v_name[et1%OFST]);
        add_instr("P_LOD %s\n",     v_name[et2%OFST]);
        
        if (strcmp(op,"EQU")==0) add_instr("S_EQU\n"); else add_instr("SF_%s\n", op);
    }

    // float var with float acc
    if ((get_type(et1)==2) && (et1%OFST!=0) && (get_type(et2)==2) && (et2%OFST==0))
    {
        add_instr("SET   aux_var\n");
        add_instr("LOD %s\n", v_name[et1%OFST]);
        add_instr("P_LOD aux_var\n");
        
        if (strcmp(op,"EQU")==0) add_instr("S_EQU\n"); else add_instr("SF_%s\n", op);
    }

    // float var with comp const
    if ((get_type(et1)==2) && (et1%OFST!=0) && (get_type(et2)==5))
    {
        fprintf(stdout, MSG_WARN_CMP_FLOAT_COMP, line_num+1);

        oper_mult(expr_of_et(et1), expr_of_et(et1));
        exec_mod2(et2);
        oper_cmp(expr_of_et(2*OFST), expr_of_et(2*OFST), type);
    }

    // float var with comp var
    if ((get_type(et1)==2) && (et1%OFST!=0) && (get_type(et2)==3) && (et2%OFST!=0))
    {
        fprintf(stdout, MSG_WARN_CMP_FLOAT_COMP, line_num+1);

        oper_mult(expr_of_et(et1), expr_of_et(et1));
        exec_mod2(et2);
        oper_cmp(expr_of_et(2*OFST), expr_of_et(2*OFST), type);
    }

    // float var with comp acc
    if ((get_type(et1)==2) && (et1%OFST!=0) && (get_type(et2)==3) && (et2%OFST==0))
    {
        fprintf(stdout, MSG_WARN_CMP_INT_COMP, line_num+1);

        add_instr("SET_P aux_var \n");
        add_instr("SET   aux_var1\n");
        acc_ok = 0;

        oper_mult(expr_of_et(et1), expr_of_et(et1));
        add_instr("P_LOD aux_var1\n");
        add_instr("P_LOD aux_var \n");
        exec_mod2(3*OFST);
        oper_cmp(expr_of_et(2*OFST), expr_of_et(2*OFST), type);
    }

    // float acc with int var
    if ((get_type(et1)==2) && (et1%OFST==0) && (get_type(et2)==1) && (et2%OFST!=0))
    {
        add_instr("P_I2F_M %s\n", v_name[et2%OFST]);
        
        if (strcmp(op,"EQU")==0) add_instr("S_EQU\n"); else add_instr("SF_%s\n", op);
    }

    // float acc with int acc
    if ((get_type(et1)==2) && (et1%OFST==0) && (get_type(et2)==1) && (et2%OFST==0))
    {
        add_instr("I2F\n");
        
        if (strcmp(op,"EQU")==0) add_instr("S_EQU\n"); else add_instr("SF_%s\n", op);
    }

    // float acc with float var
    if ((get_type(et1)==2) && (et1%OFST==0) && (get_type(et2)==2) && (et2%OFST!=0))
    {
        add_instr("P_LOD %s\n", v_name[et2%OFST]);
        
        if (strcmp(op,"EQU")==0) add_instr("S_EQU\n"); else add_instr("SF_%s\n", op);
    }

    // float acc with float acc
    if ((get_type(et1)==2) && (et1%OFST==0) && (get_type(et2)==2) && (et2%OFST==0))
    {
        if (strcmp(op,"EQU")==0) add_instr("S_EQU\n"); else add_instr("SF_%s\n", op);
    }

    // float acc with comp const
    if ((get_type(et1)==2) && (et1%OFST==0) && (get_type(et2)==5))
    {
        fprintf(stdout, MSG_WARN_CMP_FLOAT_COMP, line_num+1);

        add_instr("SET   aux_var\n");
        add_instr("P_LOD aux_var\n");
        oper_mult(expr_of_et(et1), expr_of_et(et1));
        exec_mod2(et2);
        oper_cmp(expr_of_et(2*OFST), expr_of_et(2*OFST), type);
    }

    // float acc with comp var
    if ((get_type(et1)==2) && (et1%OFST==0) && (get_type(et2)==3) && (et2%OFST!=0))
    {
        fprintf(stdout, MSG_WARN_CMP_FLOAT_COMP, line_num+1);

        add_instr("SET   aux_var\n");
        add_instr("P_LOD aux_var\n");
        oper_mult(expr_of_et(et1), expr_of_et(et1));
        exec_mod2(et2);
        oper_cmp(expr_of_et(2*OFST), expr_of_et(2*OFST), type);
    }

    // float acc with comp acc
    if ((get_type(et1)==2) && (et1%OFST==0) && (get_type(et2)==3) && (et2%OFST==0))
    {
        fprintf(stdout, MSG_WARN_CMP_FLOAT_COMP, line_num+1);

        add_instr("SET_P aux_var \n");
        add_instr("SET_P aux_var1\n");
        add_instr("SET   aux_var2\n");

        add_instr("P_LOD aux_var2\n");
        oper_mult(expr_of_et(et1), expr_of_et(et1));
        add_instr("P_LOD aux_var1\n");
        add_instr("P_LOD aux_var \n");
        exec_mod2(3*OFST);
        oper_cmp(expr_of_et(2*OFST), expr_of_et(2*OFST), type);
    }

    // comp const with int var
    if ((get_type(et1)==5) && (get_type(et2)==1) && (et2%OFST!=0))
    {
        fprintf(stdout, MSG_WARN_CMP_INT_COMP, line_num+1);

        exec_mod2(et1);
        oper_mult(expr_of_et(et2), expr_of_et(et2));
        oper_cmp(expr_of_et(2*OFST), expr_of_et(OFST), type);
    }

    // comp const with int acc
    if ((get_type(et1)==5) && (get_type(et2)==1) && (et2%OFST==0))
    {
        fprintf(stdout, MSG_WARN_CMP_INT_COMP, line_num+1);

        add_instr("SET   aux_var\n");
        acc_ok = 0;
        exec_mod2(et1);
        add_instr("P_LOD aux_var\n");
        add_instr("P_LOD aux_var\n");
        oper_mult(expr_of_et(et2), expr_of_et(et2));
        oper_cmp(expr_of_et(2*OFST), expr_of_et(OFST), type);
    }

    // comp const with float var
    if ((get_type(et1)==5) && (get_type(et2)==2) && (et2%OFST!=0))
    {
        fprintf(stdout, MSG_WARN_CMP_FLOAT_COMP, line_num+1);

        exec_mod2(et1);
        oper_mult(expr_of_et(et2), expr_of_et(et2));
        oper_cmp(expr_of_et(2*OFST), expr_of_et(2*OFST), type);
    }

    // comp const with float acc
    if ((get_type(et1)==5) && (get_type(et2)==2) && (et2%OFST==0))
    {
        fprintf(stdout, MSG_WARN_CMP_FLOAT_COMP, line_num+1);

        add_instr("SET   aux_var\n");
        acc_ok = 0;
        exec_mod2(et1);
        add_instr("P_LOD aux_var\n");
        add_instr("P_LOD aux_var\n");
        oper_mult(expr_of_et(et2), expr_of_et(et2));
        oper_cmp(expr_of_et(2*OFST), expr_of_et(2*OFST), type);
    }

    // comp const with comp const
    if ((get_type(et1)==5) && (get_type(et2)==5))
    {
        fprintf(stdout, MSG_WARN_CMP_COMPLEX, line_num+1);

        exec_mod2(et1);
        exec_mod2(et2);
        oper_cmp(expr_of_et(2*OFST), expr_of_et(2*OFST), type);
    }

    // comp const with comp var
    if ((get_type(et1)==5) && (get_type(et2)==3) && (et2%OFST!=0))
    {
        fprintf(stdout, MSG_WARN_CMP_COMPLEX, line_num+1);

        exec_mod2(et1);
        exec_mod2(et2);
        oper_cmp(expr_of_et(2*OFST), expr_of_et(2*OFST), type);
    }

    // comp const with comp acc
    if ((get_type(et1)==5) && (get_type(et2)==3) && (et2%OFST==0))
    {
        fprintf(stdout, MSG_WARN_CMP_COMPLEX, line_num+1);

        add_instr("SET_P aux_var \n");
        add_instr("SET   aux_var1\n");
        acc_ok = 0;
        exec_mod2(et1);
        add_instr("P_LOD aux_var1\n");
        add_instr("P_LOD aux_var \n");
        exec_mod2(et2);
        oper_cmp(expr_of_et(2*OFST), expr_of_et(2*OFST), type);
    }

    // comp var with int var
    if ((get_type(et1)==3) && (et1%OFST!=0) && (get_type(et2)==1) && (et2%OFST!=0))
    {
        fprintf(stdout, MSG_WARN_CMP_INT_COMP, line_num+1);

        exec_mod2(et1);
        oper_mult(expr_of_et(et2), expr_of_et(et2));
        oper_cmp(expr_of_et(2*OFST), expr_of_et(OFST), type);
    }

    // comp var with int acc
    if ((get_type(et1)==3) && (et1%OFST!=0) && (get_type(et2)==1) && (et2%OFST==0))
    {
        fprintf(stdout, MSG_WARN_CMP_INT_COMP, line_num+1);

        add_instr("SET   aux_var\n");
        acc_ok = 0;
        exec_mod2(et1);
        add_instr("P_LOD aux_var\n");
        add_instr("P_LOD aux_var\n");
        oper_mult(expr_of_et(et2), expr_of_et(et2));
        oper_cmp(expr_of_et(2*OFST), expr_of_et(OFST), type);
    }

    // comp var with float var
    if ((get_type(et1)==3) && (et1%OFST!=0) && (get_type(et2)==2) && (et2%OFST!=0))
    {
        fprintf(stdout, MSG_WARN_CMP_FLOAT_COMP, line_num+1);

        exec_mod2(et1);
        oper_mult(expr_of_et(et2), expr_of_et(et2));
        oper_cmp(expr_of_et(2*OFST), expr_of_et(2*OFST), type);
    }

    // comp var with float acc
    if ((get_type(et1)==3) && (et1%OFST!=0) && (get_type(et2)==2) && (et2%OFST==0))
    {
        fprintf(stdout, MSG_WARN_CMP_FLOAT_COMP, line_num+1);

        add_instr("SET   aux_var\n");
        acc_ok = 0;
        exec_mod2(et1);
        add_instr("P_LOD aux_var\n");
        add_instr("P_LOD aux_var\n");
        oper_mult(expr_of_et(et2), expr_of_et(et2));
        oper_cmp(expr_of_et(2*OFST), expr_of_et(2*OFST), type);
    }

    // comp var with comp const
    if ((get_type(et1)==3) && (et1%OFST!=0) && (get_type(et2)==5))
    {
        fprintf(stdout, MSG_WARN_CMP_COMPLEX, line_num+1);

        exec_mod2(et1);
        exec_mod2(et2);
        oper_cmp(expr_of_et(2*OFST), expr_of_et(2*OFST), type);
    }

    // comp var with comp var
    if ((get_type(et1)==3) && (et1%OFST!=0) && (get_type(et2)==3) && (et2%OFST!=0))
    {
        fprintf(stdout, MSG_WARN_CMP_COMPLEX, line_num+1);

        exec_mod2(et1);
        exec_mod2(et2);
        oper_cmp(expr_of_et(2*OFST), expr_of_et(2*OFST), type);
    }

    // comp var with comp acc
    if ((get_type(et1)==3) && (et1%OFST!=0) && (get_type(et2)==3) && (et2%OFST==0))
    {
        fprintf(stdout, MSG_WARN_CMP_COMPLEX, line_num+1);

        add_instr("SET_P aux_var \n");
        add_instr("SET   aux_var1\n");
        acc_ok = 0;
        exec_mod2(et1);
        add_instr("P_LOD aux_var1\n");
        add_instr("P_LOD aux_var \n");
        exec_mod2(et2);
        oper_cmp(expr_of_et(2*OFST), expr_of_et(2*OFST), type);
    }

    // comp acc with int var
    if ((get_type(et1)==3) && (et1%OFST==0) && (get_type(et2)==1) && (et2%OFST!=0))
    {
        fprintf(stdout, MSG_WARN_CMP_COMPLEX, line_num+1);

        exec_mod2(et1);
        oper_mult(expr_of_et(et2), expr_of_et(et2));
        oper_cmp(expr_of_et(2*OFST), expr_of_et(OFST), type);
    }

    // comp acc with int acc
    if ((get_type(et1)==3) && (et1%OFST==0) && (get_type(et2)==1) && (et2%OFST==0))
    {
        fprintf(stdout, MSG_WARN_CMP_COMPLEX, line_num+1);

        add_instr("SET_P aux_var\n");
        exec_mod2(et1);
        add_instr("P_LOD aux_var\n");
        add_instr("P_LOD aux_var\n");
        oper_mult(expr_of_et(et2), expr_of_et(et2));
        oper_cmp(expr_of_et(2*OFST), expr_of_et(OFST), type);
    }

    // comp acc with float var
    if ((get_type(et1)==3) && (et1%OFST==0) && (get_type(et2)==2) && (et2%OFST!=0))
    {
        fprintf(stdout, MSG_WARN_CMP_COMPLEX, line_num+1);

        exec_mod2(et1);
        oper_mult(expr_of_et(et2), expr_of_et(et2));
        oper_cmp(expr_of_et(2*OFST), expr_of_et(2*OFST), type);
    }

    // comp acc with float acc
    if ((get_type(et1)==3) && (et1%OFST==0) && (get_type(et2)==2) && (et2%OFST==0))
    {
        fprintf(stdout, MSG_WARN_CMP_COMPLEX, line_num+1);

        add_instr("SET_P aux_var\n");
        exec_mod2(et1);
        add_instr("P_LOD aux_var\n");
        add_instr("P_LOD aux_var\n");
        oper_mult(expr_of_et(et2), expr_of_et(et2));
        oper_cmp(expr_of_et(2*OFST), expr_of_et(2*OFST), type);
    }

    // comp acc with comp const
    if ((get_type(et1)==3) && (et1%OFST==0) && (get_type(et2)==5))
    {
        fprintf(stdout, MSG_WARN_CMP_COMPLEX, line_num+1);

        exec_mod2(et1);
        exec_mod2(et2);
        oper_cmp(expr_of_et(2*OFST), expr_of_et(2*OFST), type);
    }

    // comp acc with comp var
    if ((get_type(et1)==3) && (et1%OFST==0) && (get_type(et2)==3) && (et2%OFST!=0))
    {
        fprintf(stdout, MSG_WARN_CMP_COMPLEX, line_num+1);

        exec_mod2(et1);
        exec_mod2(et2);
        oper_cmp(expr_of_et(2*OFST), expr_of_et(2*OFST), type);
    }
    
    // comp acc with comp acc
    if ((get_type(et1)==3) && (et1%OFST==0) && (get_type(et2)==3) && (et2%OFST==0))
    {
        fprintf(stdout, MSG_WARN_CMP_COMPLEX, line_num+1);

        add_instr("SET_P aux_var \n");
        add_instr("SET_P aux_var1\n");
        exec_mod2(et1);
        add_instr("P_LOD aux_var1\n");
        add_instr("P_LOD aux_var \n");
        exec_mod2(et2);
        oper_cmp(expr_of_et(2*OFST), expr_of_et(2*OFST), type);
    }

    acc_ok = 1;

    return expr_make(1, 0); // int boolean in the accumulator
}

// compares greater-or-equal (opposite of less-than)
expr oper_greq(expr e1, expr e2)
{
    return oper_lin(oper_cmp(e1, e2, 0));
}

// compares less-or-equal (opposite of greater-than)
expr oper_leeq(expr e1, expr e2)
{
    return oper_lin(oper_cmp(e1, e2, 1));
}

// compares not-equal (opposite of equal-to)
expr oper_dife(expr e1, expr e2)
{
    return oper_lin(oper_cmp(e1, e2, 2));
}

// ----------------------------------------------------------------------------
// conditional operations (used in if/else/while) -----------------------------
// ----------------------------------------------------------------------------

// logical inversion (!)
expr oper_lin(expr e)
{
    int et = expr_to_et(e);
    int etr, eti;

    char ld [10]; if (acc_ok == 0) strcpy(ld ,"LOD"  ); else strcpy(ld ,"P_LOD"  );
    char f2i[10]; if (acc_ok == 0) strcpy(f2i,"F2I_M"); else strcpy(f2i,"P_F2I_M");
    char lin[10]; if (acc_ok == 0) strcpy(lin,"LIN_M"); else strcpy(lin,"P_LIN_M");

    // when it is an int in memory
    if ((get_type(et) == 1) && (et % OFST != 0))
    {
        add_instr("%s %s\n", lin, v_name[et%OFST]);
    }

    // when it is an int in the acc
    if ((get_type(et) == 1) && (et % OFST == 0))
    {
        add_instr("LIN\n");
    }

    // when it is a float variable in memory
    if ((get_type(et) == 2) && (et % OFST != 0))
    {
        fprintf(stdout, MSG_WARN_LOGIC_FLOAT, line_num+1);

        add_instr("%s %s\n", f2i, v_name[et%OFST]);
        add_instr("LIN\n");
    }

    // when it is a float in the acc
    if ((get_type(et) == 2) && (et % OFST == 0))
    {
        fprintf(stdout, MSG_WARN_LOGIC_FLOAT, line_num+1);

        add_instr("F2I\n");
        add_instr("LIN\n");
    }

    // when it is a comp const
    if (get_type(et) == 5)
    {
        fprintf(stdout, MSG_WARN_LOGIC_COMP, line_num+1);

        get_cmp_cst(et,&etr,&eti);

        add_instr("%s %s\n", f2i, v_name[etr%OFST]);
        add_instr("LIN\n");
    }

    // when it is a comp variable
    if ((get_type(et) == 3) && (et % OFST != 0))
    {
        fprintf(stdout, MSG_WARN_LOGIC_COMP, line_num+1);

        add_instr("%s %s\n", f2i, v_name[et%OFST]);
        add_instr("LIN\n");
    }

    // when it is a comp in the acc
    if ((get_type(et) == 3) && (et % OFST == 0))
    {
        fprintf(stdout, MSG_WARN_LOGIC_COMP, line_num+1);

        add_instr("POP\n");
        add_instr("F2I\n");
        add_instr("LIN\n");
    }

    acc_ok = 1;

    return expr_make(1, 0); // int in the accumulator
}

// logical and/or (&& ||)
expr oper_lanor(expr e1, expr e2, int type)
{
    int et1 = expr_to_et(e1);
    int et2 = expr_to_et(e2);

    if ((get_type(et1) > 1) || (get_type(et2) > 1))
    {
        fprintf(stderr, MSG_ERR_LOGIC_NON_INT, line_num+1);
        exit(EXIT_FAILURE);
    }

    char ld[10];
    if (acc_ok == 0) strcpy(ld,"LOD"); else strcpy(ld,"P_LOD");

    char op[16];
    switch (type)
    {
        case 0: strcpy(op, "LAN"); break;
        case 1: strcpy(op, "LOR" ); break;
    }

    // int var with int var
    if ((get_type(et1)==1) && (et1%OFST!=0) && (get_type(et2)==1) && (et2%OFST!=0))
    {
        add_instr("%s %s\n", ld, v_name[et2%OFST]);
        add_instr("%s %s\n", op, v_name[et1%OFST]);
    }

    // int var with int acc
    if ((get_type(et1)==1) && (et1%OFST!=0) && (get_type(et2)==1) && (et2%OFST==0))
    {
        add_instr("%s %s\n", op, v_name[et1%OFST]);
    }

    // int acc with int var
    if ((get_type(et1)==1) && (et1%OFST==0) && (get_type(et2)==1) && (et2%OFST!=0))
    {
        add_instr("%s %s\n", op, v_name[et2%OFST]);
    }

    // int acc with int acc
    if ((get_type(et1)==1) && (et1%OFST==0) && (get_type(et2)==1) && (et2%OFST==0))
    {
        add_instr("S_%s\n", op);
    }

    acc_ok = 1;

    return expr_make(1, 0); // int boolean in the accumulator
}

// ----------------------------------------------------------------------------
// bitwise logical-gate operations (~ & |) ------------------------------------
// ----------------------------------------------------------------------------

// inverter gate
expr oper_inv(expr e)
{
    int et = expr_to_et(e);

    if (get_type(et) > 1)
        {fprintf(stderr, MSG_ERR_INV_NON_INT, line_num+1); exit(EXIT_FAILURE);}

    char inv[10]; if (acc_ok == 0) strcpy(inv,"INV_M"); else strcpy(inv,"P_INV_M");

    // when it is an int in memory
    if ((get_type(et) == 1) && (et % OFST != 0))
    {
        add_instr("%s %s\n", inv, v_name[et%OFST]);
    }

    // when it is an int in the acc
    if ((get_type(et) == 1) && (et % OFST == 0))
    {
        add_instr("INV\n");
    }

    acc_ok = 1;

    return expr_make(1, 0); // int in the accumulator
}

// two-input logical gates (& | ^)
expr oper_bitw(expr e1, expr e2, int type)
{
    int et1 = expr_to_et(e1);
    int et2 = expr_to_et(e2);

    if (get_type(et1) > 2 || get_type(et2) > 2)
    {
        fprintf(stderr, MSG_ERR_BITWISE_COMPLEX, line_num+1);
        exit(EXIT_FAILURE);
    }
    
    char op[16];

    switch (type)
    {
        case  0: strcpy(op, "AND"); break;
        case  1: strcpy(op, "ORR"); break;
        case  2: strcpy(op, "XOR"); break;
    }

    char ld[10]; if (acc_ok == 0) strcpy(ld,"LOD"); else strcpy(ld,"P_LOD");

    // var with var
    if ((et1%OFST != 0) && (et2%OFST != 0))
    {
        add_instr("%s %s\n", ld, v_name[et2%OFST]);
        add_instr("%s %s\n", op, v_name[et1%OFST]);
    }

    // var with acc
    if ((et1%OFST != 0) && (et2%OFST == 0))
    {
        add_instr("%s %s\n", op, v_name[et1%OFST]);
    }

    // acc with var
    if ((et1%OFST == 0) && (et2%OFST != 0))
    {
        add_instr("P_LOD %s\n", v_name[et2%OFST]);
        add_instr("S_%s\n", op);
    }

    // acc with acc
    if ((et1%OFST == 0) && (et2%OFST == 0))
    {
        add_instr("S_%s\n", op);
    }

    acc_ok = 1;

    return expr_make(1, 0); // int in the accumulator
}

// ----------------------------------------------------------------------------
// bit-shift operations -------------------------------------------------------
// ----------------------------------------------------------------------------

expr oper_shift(expr e1, expr e2, int type)
{
    int et1 = expr_to_et(e1);
    int et2 = expr_to_et(e2);

    if (get_type(et1) > 2)
        {fprintf(stderr, MSG_ERR_SHIFT_COMPLEX, line_num+1); exit(EXIT_FAILURE);}

    if (get_type(et2) > 2)
        {fprintf(stderr, MSG_ERR_SHIFT_BY_COMP, line_num+1); exit(EXIT_FAILURE);}

    char op[16];

    switch (type)
    {
        case  0: strcpy(op, "SHL"); break;
        case  1: strcpy(op, "SHR"); break;
        case  2: strcpy(op, "SRS"); break;
    }

    char ld[10]; if (acc_ok == 0) strcpy(ld,"LOD"); else strcpy(ld,"P_LOD");

    // int/float var with int var
    if ((et1%OFST != 0) && (get_type(et2) == 1) && (et2%OFST != 0))
    {
        add_instr("%s %s\n", ld, v_name[et2%OFST]);
        add_instr("%s %s\n", op, v_name[et1%OFST]);
    }

    // int/float var with int acc
    if ((et1%OFST != 0) && (get_type(et2) == 1) && (et2%OFST == 0))
    {
        add_instr("%s %s\n", op, v_name[et1%OFST]);
    }

    // int/float var with float var
    if ((et1%OFST != 0) && (get_type(et2) == 2) && (et2%OFST != 0))
    {
        fprintf(stdout, MSG_WARN_SHIFT_BY_FLOAT, line_num+1);

        add_instr("%s %s\n", ld, v_name[et2%OFST]);
        add_instr("F2I\n");
        add_instr("%s %s\n", op, v_name[et1%OFST]);
    }

    // int/float var with float acc
    if ((et1%OFST != 0) && (get_type(et2) == 2) && (et2%OFST == 0))
    {
        fprintf(stdout, MSG_WARN_SHIFT_BY_FLOAT, line_num+1);

        add_instr("F2I\n");
        add_instr("%s %s\n", op, v_name[et1%OFST]);
    }

    // int/float acc with int var
    if ((et1%OFST == 0) && (get_type(et2) == 1) && (et2%OFST != 0))
    {
        add_instr("P_LOD %s\n", v_name[et2%OFST]);
        add_instr("S_%s\n", op);
    }

    // int/float acc with int acc
    if ((et1%OFST == 0) && (get_type(et2) == 1) && (et2%OFST == 0))
    {
        add_instr("S_%s\n", op);
    }

    // int/float acc with float var
    if ((et1%OFST == 0) && (get_type(et2) == 2) && (et2%OFST != 0))
    {
        fprintf(stdout, MSG_WARN_SHIFT_BY_FLOAT, line_num+1);

        add_instr("P_LOD %s\n", v_name[et2%OFST]);
        add_instr("F2I\n");
        add_instr("S_%s\n", op);
    }

    // int/float acc with float acc
    if ((et1%OFST == 0) && (get_type(et2) == 2) && (et2%OFST == 0))
    {
        fprintf(stdout, MSG_WARN_SHIFT_BY_FLOAT, line_num+1);
        
        add_instr("F2I\n");
        add_instr("S_%s\n", op);
    }

    acc_ok = 1;

    return expr_make(1, 0); // int in the accumulator
}