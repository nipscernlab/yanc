// ----------------------------------------------------------------------------
// ALU operations -------------------------------------------------------------
// ----------------------------------------------------------------------------

// global includes
#include <string.h>
#include <stdlib.h>

// local includes
#include "../Headers/t2t.h"
#include "../Headers/oper.h"
#include "../Headers/stdlib.h"
#include "../Headers/global.h"
#include "../Headers/data_use.h"
#include "../Headers/diretivas.h"
#include "../Headers/variaveis.h"
#include "../Headers/messages.h"

// ----------------------------------------------------------------------------
// arithmetic operations ------------------------------------------------------
// ----------------------------------------------------------------------------

// negates a number
expr oper_neg(expr e)
{
    expr   etr, eti;

    char  neg[10]; if (acc_ok == 0) strcpy( neg,   "NEG_M"); else strcpy( neg,  "P_NEG_M");
    char fneg[10]; if (acc_ok == 0) strcpy(fneg, "F_NEG_M"); else strcpy(fneg, "PF_NEG_M");

    // when it is an int variable in memory
    if ((e.type == 1) && (e.id != 0))
    {
        add_instr("%s %s\n", neg, v_table[e.id].name);
    }

    // when it is an int in the acc
    if ((e.type == 1) && (e.id == 0))
    {
        add_instr("NEG\n");
    }

    // when it is a float variable in memory
    if ((e.type == 2) && (e.id != 0))
    {
        add_instr("%s %s\n", fneg, v_table[e.id].name);
    }

    // when it is a float in the acc
    if ((e.type == 2) && (e.id == 0))
    {
        add_instr("F_NEG\n");
    }

    // when it is a comp constant
    if (e.type == 5)
    {
        get_cmp_cst(e,&etr,&eti);

        add_instr("%s %s\n", fneg, v_table[etr.id].name);
        add_instr("PF_NEG_M %s\n", v_table[eti.id].name);
    }

    // when it is a comp variable in memory
    if ((e.type == 3) && (e.id != 0))
    {
        get_cmp_ets(e,&etr,&eti);

        add_instr("%s %s\n", fneg, v_table[etr.id].name);
        add_instr("PF_NEG_M %s\n", v_table[eti.id].name);
    }

    // when it is a comp in the acc
    if ((e.type == 3) && (e.id == 0))
    {
        add_instr("   SET_P aux_var\n");
        add_instr(" F_NEG\n"          );
        add_instr("PF_NEG_M aux_var\n");
    }

    acc_ok = 1;

    // a comp constant got materialized into the comp (acc) representation
    if (e.type == 5) e.type = 3;

    return expr_make(e.type, 0);
}

// adds two numbers
expr oper_soma(expr e1, expr e2)
{
    expr etr, eti;

    char ld [10]; if (acc_ok == 0) strcpy(ld ,"LOD"  ); else strcpy(ld ,"P_LOD"  );
    char i2f[10]; if (acc_ok == 0) strcpy(i2f,"I2F_M"); else strcpy(i2f,"P_I2F_M");

    // int var with int var
    if ((e1.type==1) && (e1.id!=0) && (e2.type==1) && (e2.id!=0))
    {
        add_instr("%s %s\n" , ld, v_table[e1.id].name);
        add_instr("ADD %s\n",     v_table[e2.id].name);
    }

    // int var with int acc
    if ((e1.type==1) && (e1.id!=0) && (e2.type==1) && (e2.id==0))
    {
        add_instr("ADD %s\n", v_table[e1.id].name);
    }

    // int var with float var
    if ((e1.type==1) && (e1.id!=0) && (e2.type==2) && (e2.id!=0))
    {
        add_instr("%s %s\n", i2f, v_table[e1.id].name);
        add_instr("F_ADD %s\n"  , v_table[e2.id].name);
    }

    // int var with float acc
    if ((e1.type==1) && (e1.id!=0) && (e2.type==2) && (e2.id==0))
    {
        add_instr("P_I2F_M %s\n", v_table[e1.id].name);
        add_instr("SF_ADD\n");
    }

    // int var with comp const
    if ((e1.type==1) && (e1.id!=0) && (e2.type==5))
    {
        get_cmp_cst(e2,&etr,&eti);

        add_instr("%s %s\n" , i2f, v_table[e1.id].name);
        add_instr("F_ADD %s\n"   , v_table[etr.id].name);
        add_instr("P_LOD %s\n"   , v_table[eti.id].name);
    }

    // int var with comp var
    if ((e1.type==1) && (e1.id!=0) && (e2.type==3) && (e2.id!=0))
    {
        get_cmp_ets(e2,&etr,&eti);

        add_instr("%s %s\n", i2f, v_table[e1.id].name);
        add_instr("F_ADD %s\n"  , v_table[etr.id].name);
        add_instr("P_LOD  %s\n" , v_table[eti.id].name);
    }

    // int var with comp acc
    if ((e1.type==1) && (e1.id!=0) && (e2.type==3) && (e2.id==0))
    {
        add_instr("SET aux_var\n");                 // temporarily save the imaginary part
        add_instr("I2F_M %s\n", v_table[e1.id].name);  // fetch the int while converting to float
        add_instr("SF_ADD\n");                      // add acc with stack
        add_instr("P_LOD aux_var\n");               // push the result onto the stack while bringing the imag back
    }

    // int acc with int var
    if ((e1.type==1) && (e1.id==0) && (e2.type==1) && (e2.id!=0))
    {
        add_instr("ADD %s\n", v_table[e2.id].name);
    }

    // int acc with int acc
    if ((e1.type==1) && (e1.id==0) && (e2.type==1) && (e2.id==0))
    {
        add_instr("S_ADD\n");
    }

    // int acc with float var
    if ((e1.type==1) && (e1.id==0) && (e2.type==2) && (e2.id!=0))
    {
        add_instr("I2F\n");
        add_instr("F_ADD %s\n", v_table[e2.id].name);
    }

    // int acc with float acc
    if ((e1.type==1) && (e1.id==0) && (e2.type==2) && (e2.id==0))
    {
        add_instr("SET_P aux_var\n"); // temporarily save the float, pushing the int to the accumulator
        add_instr("I2F\n");           // converte o int do acumulador pra float
        add_instr("F_ADD aux_var\n"); // add with the saved float
    }

    // int acc with comp const
    if ((e1.type==1) && (e1.id==0) && (e2.type==5))
    {
        get_cmp_cst(e2,&etr,&eti);

        add_instr("I2F\n");
        add_instr("F_ADD %s\n", v_table[etr.id].name);
        add_instr("P_LOD %s\n", v_table[eti.id].name);
    }

    // int acc with comp var
    if ((e1.type==1) && (e1.id==0) && (e2.type==3) && (e2.id!=0))
    {
        get_cmp_ets(e2,&etr,&eti);

        add_instr("I2F\n");
        add_instr("F_ADD %s\n", v_table[etr.id].name);
        add_instr("P_LOD %s\n", v_table[eti.id].name);
    }

    // int acc with comp acc
    if ((e1.type==1) && (e1.id==0) && (e2.type==3) && (e2.id==0))
    {
        add_instr("SET_P aux_var\n" ); // temporarily save the imaginary part, popping the real part
        add_instr("SET_P aux_var1\n"); // temporarily save the real part, popping the int
        add_instr("I2F\n");            // converte o int do acumulador pra float
        add_instr("F_ADD aux_var1\n"); // add with the saved real part
        add_instr("P_LOD aux_var\n" ); // push the imaginary part back to the accumulator
    }

    // float var with int var
    if ((e1.type==2) && (e1.id!=0) && (e2.type==1) && (e2.id!=0))
    {
        add_instr("%s %s\n",i2f, v_table[e2.id].name);
        add_instr("F_ADD %s\n" , v_table[e1.id].name);
    }

    // float var with int acc
    if ((e1.type==2) && (e1.id!=0) && (e2.type==1) && (e2.id==0))
    {
        add_instr("I2F\n");
        add_instr("F_ADD %s\n", v_table[e1.id].name);
    }

    // float var with float var
    if ((e1.type==2) && (e1.id!=0) && (e2.type==2) && (e2.id!=0))
    {
        add_instr("%s %s\n", ld, v_table[e1.id].name);
        add_instr("F_ADD %s\n" , v_table[e2.id].name);
    }

    // float var with float acc
    if ((e1.type==2) && (e1.id!=0) && (e2.type==2) && (e2.id==0))
    {
        add_instr("F_ADD %s\n", v_table[e1.id].name);
    }

    // float var with comp const
    if ((e1.type==2) && (e1.id!=0) && (e2.type==5))
    {
        get_cmp_cst(e2,&etr,&eti);

        add_instr("%s %s\n", ld, v_table[e1.id].name);
        add_instr("F_ADD %s\n" , v_table[etr.id].name);
        add_instr("P_LOD %s\n" , v_table[eti.id].name);
    }

    // float var with comp var
    if ((e1.type==2) && (e1.id!=0) && (e2.type==3) && (e2.id!=0))
    {
        get_cmp_ets(e2,&etr,&eti);

        add_instr("%s %s\n", ld, v_table[e1.id].name);
        add_instr("F_ADD %s\n" , v_table[etr.id].name);
        add_instr("P_LOD %s\n" , v_table[eti.id].name);
    }

    // float var with comp acc
    if ((e1.type==2) && (e1.id!=0) && (e2.type==3) && (e2.id==0))
    {
        add_instr("SET_P aux_var\n");
        add_instr("F_ADD %s\n", v_table[e1.id].name);
        add_instr("P_LOD aux_var\n");
    }

    // float acc with int var
    if ((e1.type==2) && (e1.id==0) && (e2.type==1) && (e2.id!=0))
    {
        add_instr("P_I2F_M %s\n", v_table[e2.id].name);
        add_instr("SF_ADD\n");
    }

    // float acc with int acc
    if ((e1.type==2) && (e1.id==0) && (e2.type==1) && (e2.id==0))
    {
        add_instr("I2F\n");
        add_instr("SF_ADD\n");
    }

    // float acc with float var
    if ((e1.type==2) && (e1.id==0) && (e2.type==2) && (e2.id!=0))
    {
        add_instr("F_ADD %s\n", v_table[e2.id].name);
    }

    // float acc with float acc
    if ((e1.type==2) && (e1.id==0) && (e2.type==2) && (e2.id==0))
    {
        add_instr("SF_ADD\n");
    }

    // float acc with comp const
    if ((e1.type==2) && (e1.id==0) && (e2.type==5))
    {
        get_cmp_cst(e2,&etr,&eti);

        add_instr("F_ADD %s\n" , v_table[etr.id].name);
        add_instr("P_LOD %s\n" , v_table[eti.id].name);
    }

    // float acc with comp var
    if ((e1.type==2) && (e1.id==0) && (e2.type==3) && (e2.id!=0))
    {
        get_cmp_ets(e2,&etr,&eti);

        add_instr("F_ADD %s\n", v_table[etr.id].name);
        add_instr("P_LOD %s\n", v_table[eti.id].name);
    }

    // float acc with comp acc
    if ((e1.type==2) && (e1.id==0) && (e2.type==3) && (e2.id==0))
    {
        add_instr("SET_P aux_var\n");
        add_instr("SF_ADD\n");
        add_instr("P_LOD aux_var\n");
    }

    // comp const with int var
    if ((e1.type==5) && (e2.type==1) && (e2.id!=0))
    {
        get_cmp_cst(e1,&etr,&eti);

        add_instr("%s %s\n", i2f, v_table[e2.id].name);
        add_instr("F_ADD %s\n"  , v_table[etr.id].name);
        add_instr("P_LOD %s\n"  , v_table[eti.id].name);
    }

    // comp const with int acc
    if ((e1.type==5) && (e2.type==1) && (e2.id==0))
    {
        get_cmp_cst(e1,&etr,&eti);

        add_instr("I2F\n");
        add_instr("F_ADD %s\n", v_table[etr.id].name);
        add_instr("P_LOD %s\n", v_table[eti.id].name);
    }

    // comp const with float var
    if ((e1.type==5) && (e2.type==2) && (e2.id!=0))
    {
        get_cmp_cst(e1,&etr,&eti);

        add_instr("%s %s\n", ld, v_table[e2.id].name);
        add_instr("F_ADD %s\n" , v_table[etr.id].name);
        add_instr("P_LOD %s\n" , v_table[eti.id].name);
    }

    // comp const with float acc
    if ((e1.type==5) && (e2.type==2) && (e2.id==0))
    {
        get_cmp_cst(e1,&etr,&eti);

        add_instr("F_ADD %s\n", v_table[etr.id].name);
        add_instr("P_LOD %s\n", v_table[eti.id].name);
    }

    // comp const with comp const
    if ((e1.type==5) && (e2.type==5))
    {
        expr et1r, et2r;
        expr et1i, et2i;

        get_cmp_cst(e1,&et1r,&et1i);
        get_cmp_cst(e2,&et2r,&et2i);

        add_instr("%s %s\n", ld, v_table[et1r.id].name);
        add_instr("F_ADD %s\n" , v_table[et2r.id].name);

        add_instr("P_LOD %s\n" , v_table[et1i.id].name);
        add_instr("F_ADD %s\n" , v_table[et2i.id].name);
    }

    // comp const with comp var
    if ((e1.type==5) && (e2.type==3) && (e2.id!=0))
    {
        expr et1r, et2r;
        expr et1i, et2i;

        get_cmp_cst(e1,&et1r,&et1i);
        get_cmp_ets(e2,&et2r,&et2i);

        add_instr("%s %s\n", ld, v_table[et1r.id].name);
        add_instr("F_ADD %s\n" , v_table[et2r.id].name);

        add_instr("P_LOD %s\n" , v_table[et1i.id].name);
        add_instr("F_ADD %s\n" , v_table[et2i.id].name);
    }

    // comp const with comp acc
    if ((e1.type==5) && (e2.type==3) && (e2.id==0))
    {
        get_cmp_cst(e1,&etr,&eti);

        add_instr("SET_P aux_var\n");
        add_instr("F_ADD %s\n", v_table[etr.id].name);

        add_instr("P_LOD %s\n", v_table[eti.id].name);
        add_instr("F_ADD aux_var\n");
    }

    // comp var with int var
    if ((e1.type==3) && (e1.id!=0) && (e2.type==1) && (e2.id!=0))
    {
        get_cmp_ets(e1,&etr,&eti);

        add_instr("%s  %s\n", i2f, v_table[e2.id].name);
        add_instr("F_ADD %s\n"    , v_table[etr.id].name);
        add_instr("P_LOD  %s\n"    , v_table[eti.id].name);
    }

    // comp var with int acc
    if ((e1.type==3) && (e1.id!=0) && (e2.type==1) && (e2.id==0))
    {
        get_cmp_ets(e1,&etr,&eti);

        add_instr("I2F\n");
        add_instr("F_ADD %s\n", v_table[etr.id].name);
        add_instr("P_LOD %s\n", v_table[eti.id].name);
    }

    // comp var with float var
    if ((e1.type==3) && (e1.id!=0) && (e2.type==2) && (e2.id!=0))
    {
        get_cmp_ets(e1,&etr,&eti);

        add_instr("%s %s\n", ld, v_table[e2.id].name);
        add_instr("F_ADD %s\n" , v_table[etr.id].name);
        add_instr("P_LOD %s\n" , v_table[eti.id].name);
    }

    // comp var with float acc
    if ((e1.type==3) && (e1.id!=0) && (e2.type==2) && (e2.id==0))
    {
        get_cmp_ets(e1,&etr,&eti);

        add_instr("F_ADD %s\n", v_table[etr.id].name);
        add_instr("P_LOD %s\n", v_table[eti.id].name);
    }

    // comp var with comp const
    if ((e1.type==3) && (e1.id!=0) && (e2.type==5))
    {
        expr et1r, et2r;
        expr et1i, et2i;

        get_cmp_ets(e1,&et1r,&et1i);
        get_cmp_cst(e2,&et2r,&et2i);

        add_instr("%s %s\n", ld, v_table[et1r.id].name);
        add_instr("F_ADD %s\n" , v_table[et2r.id].name);

        add_instr("P_LOD %s\n" , v_table[et1i.id].name);
        add_instr("F_ADD %s\n" , v_table[et2i.id].name);
    }

    // comp var with comp var
    if ((e1.type==3) && (e1.id!=0) && (e2.type==3) && (e2.id!=0))
    {
        expr et1r, et2r;
        expr et1i, et2i;

        get_cmp_ets(e1,&et1r,&et1i);
        get_cmp_ets(e2,&et2r,&et2i);

        add_instr("%s %s\n", ld, v_table[et1r.id].name);
        add_instr("F_ADD %s\n" , v_table[et2r.id].name);

        add_instr("P_LOD %s\n" , v_table[et1i.id].name);
        add_instr("F_ADD %s\n" , v_table[et2i.id].name);
    }

    // comp var with comp acc
    if ((e1.type==3) && (e1.id!=0) && (e2.type==3) && (e2.id==0))
    {
        get_cmp_ets(e1,&etr,&eti);

        add_instr("SET_P aux_var\n");
        add_instr("F_ADD %s\n", v_table[etr.id].name);

        add_instr("P_LOD %s\n", v_table[eti.id].name);
        add_instr("F_ADD aux_var\n");
    }

    // comp acc with int var
    if ((e1.type==3) && (e1.id==0) && (e2.type==1) && (e2.id!=0))
    {
        add_instr("SET_P aux_var\n");
        add_instr("P_I2F_M %s\n", v_table[e2.id].name);
        add_instr("SF_ADD\n");
        add_instr("P_LOD aux_var\n");
    }

    // comp acc with int acc
    if ((e1.type==3) && (e1.id==0) && (e2.type==1) && (e2.id==0))
    {
        add_instr("SET_P   aux_var\n" );
        add_instr("SET_P   aux_var1\n");
        add_instr("P_I2F_M aux_var\n" );
        add_instr("SF_ADD\n");
        add_instr("P_LOD   aux_var1\n");
    }

    // comp acc with float var
    if ((e1.type==3) && (e1.id==0) && (e2.type==2) && (e2.id!=0))
    {
        add_instr("SET_P aux_var\n");
        add_instr("F_ADD %s\n", v_table[e2.id].name);
        add_instr("P_LOD aux_var\n");
    }

    // comp acc with float acc
    if ((e1.type==3) && (e1.id==0) && (e2.type==2) && (e2.id==0))
    {
        add_instr("SET_P aux_var\n" );
        add_instr("SET_P aux_var1\n");
        add_instr("F_ADD aux_var\n" );
        add_instr("P_LOD aux_var1\n");
    }

    // comp acc with comp const
    if ((e1.type==3) && (e1.id==0) && (e2.type==5))
    {
        get_cmp_cst(e2,&etr,&eti);

        add_instr("SET_P aux_var\n");
        add_instr("F_ADD %s\n", v_table[etr.id].name);

        add_instr("P_LOD aux_var\n");
        add_instr("F_ADD %s\n", v_table[eti.id].name);
    }

    // comp acc with comp var
    if ((e1.type==3) && (e1.id==0) && (e2.type==3) && (e2.id!=0))
    {
        get_cmp_ets(e2,&etr,&eti);

        add_instr("SET_P aux_var\n");
        add_instr("F_ADD %s\n", v_table[etr.id].name);

        add_instr("P_LOD aux_var\n");
        add_instr("F_ADD %s\n", v_table[eti.id].name);
    }

    // comp acc with comp acc
    if ((e1.type==3) && (e1.id==0) && (e2.type==3) && (e2.id==0))
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
         if ((e1.type > 2) || (e2.type > 2))
         type = 3;
    else if ((e1.type > 1) || (e2.type > 1))
         type = 2;
    else type = 1;

    return expr_make(type, 0);
}

// subtraction between two numbers
expr oper_subt(expr e1, expr e2)
{
    expr etr, eti;

    char ld [10]; if (acc_ok == 0) strcpy(ld ,"LOD"  ); else strcpy(ld ,"P_LOD"  );
    char i2f[10]; if (acc_ok == 0) strcpy(i2f,"I2F_M"); else strcpy(i2f,"P_I2F_M");

    // int var with int var
    if ((e1.type==1) && (e1.id!=0) && (e2.type==1) && (e2.id!=0))
    {
        return oper_soma(e1, oper_neg(e2));
    }

    // int var with int acc
    if ((e1.type==1) && (e1.id!=0) && (e2.type==1) && (e2.id==0))
    {
        return oper_soma(e1, oper_neg(e2));
    }

    // int var with float var
    if ((e1.type==1) && (e1.id!=0) && (e2.type==2) && (e2.id!=0))
    {
        add_instr("%s %s\n", i2f, v_table[e1.id].name);
        add_instr("F_SU1 %s\n"  , v_table[e2.id].name);
    }

    // int var with float acc
    if ((e1.type==1) && (e1.id!=0) && (e2.type==2) && (e2.id==0))
    {
        add_instr("%s %s\n", i2f, v_table[e1.id].name);
        add_instr("SF_SU1\n");
    }

    // int var with comp const (probably never happens, but just in case...)
    if ((e1.type==1) && (e1.id!=0) && (e2.type==5))
    {
        return oper_soma(e1, oper_neg(e2));
    }

    // int var with comp var
    if ((e1.type==1) && (e1.id!=0) && (e2.type==3) && (e2.id!=0))
    {
        get_cmp_ets(e2,&etr,&eti);

        add_instr("%s %s\n", i2f   , v_table[e1.id].name);
        add_instr("F_SU1 %s\n"     , v_table[etr.id].name);
        add_instr("PF_NEG_M  %s\n" , v_table[eti.id].name);
    }

    // int var with comp acc
    if ((e1.type==1) && (e1.id!=0) && (e2.type==3) && (e2.id==0))
    {
        add_instr("SET aux_var\n");                 // temporarily save the imaginary part
        add_instr("I2F_M %s\n", v_table[e1.id].name);  // fetch the int while converting to float
        add_instr("SF_SU1\n");                      // add acc with stack
        add_instr("PF_NEG_M aux_var\n");            // push the result onto the stack while bringing the imag back
    }

    // int acc with int var
    if ((e1.type==1) && (e1.id==0) && (e2.type==1) && (e2.id!=0))
    {
        return oper_soma(e1, oper_neg(e2));
    }

    // int acc with int acc
    if ((e1.type==1) && (e1.id==0) && (e2.type==1) && (e2.id==0))
    {
        return oper_soma(e1, oper_neg(e2));
    }

    // int acc with float var
    if ((e1.type==1) && (e1.id==0) && (e2.type==2) && (e2.id!=0))
    {
        add_instr("I2F\n");
        add_instr("F_SU1 %s\n", v_table[e2.id].name);
    }

    // int acc with float acc
    if ((e1.type==1) && (e1.id==0) && (e2.type==2) && (e2.id==0))
    {
        add_instr("SET_P aux_var\n");
        add_instr("I2F\n");
        add_instr("F_SU1 aux_var\n");
    }

    // int acc with comp const
    if ((e1.type==1) && (e1.id==0) && (e2.type==5))
    {
        return oper_soma(e1, oper_neg(e2));
    }

    // int acc with comp var
    if ((e1.type==1) && (e1.id==0) && (e2.type==3) && (e2.id!=0))
    {
        get_cmp_ets(e2,&etr,&eti);

        add_instr("I2F\n");
        add_instr("F_SU1 %s\n"   , v_table[etr.id].name);
        add_instr("PF_NEG_M %s\n", v_table[eti.id].name);
    }

    // int acc with comp acc
    if ((e1.type==1) && (e1.id==0) && (e2.type==3) && (e2.id==0))
    {
        add_instr("SET_P    aux_var\n" );
        add_instr("SET_P    aux_var1\n");
        add_instr("I2F\n");
        add_instr("F_SU1    aux_var1\n");
        add_instr("PF_NEG_M aux_var\n" );
    }

    // float var with int var
    if ((e1.type==2) && (e1.id!=0) && (e2.type==1) && (e2.id!=0))
    {
        add_instr("%s %s\n",i2f, v_table[e2.id].name);
        add_instr("F_SU2 %s\n" , v_table[e1.id].name);
    }

    // float var with int acc
    if ((e1.type==2) && (e1.id!=0) && (e2.type==1) && (e2.id==0))
    {
        add_instr("I2F\n");
        add_instr("F_SU2 %s\n", v_table[e1.id].name);
    }

    // float var with float var
    if ((e1.type==2) && (e1.id!=0) && (e2.type==2) && (e2.id!=0))
    {
        add_instr("%s %s\n", ld, v_table[e1.id].name);
        add_instr("F_SU1 %s\n" , v_table[e2.id].name);
    }

    // float var with float acc
    if ((e1.type==2) && (e1.id!=0) && (e2.type==2) && (e2.id==0))
    {
        add_instr("F_SU2 %s\n", v_table[e1.id].name);
    }

    // float var with comp const (no negative comp const)
    if ((e1.type==2) && (e1.id!=0) && (e2.type==5))
    {
        return oper_soma(e1, oper_neg(e2));
    }

    // float var with comp var
    if ((e1.type==2) && (e1.id!=0) && (e2.type==3) && (e2.id!=0))
    {
        get_cmp_ets(e2,&etr,&eti);

        add_instr("%s %s\n",    ld, v_table[e1.id].name);
        add_instr("F_SU1 %s\n"    , v_table[etr.id].name);
        add_instr("PF_NEG_M %s\n" , v_table[eti.id].name);
    }

    // float var with comp acc
    if ((e1.type==2) && (e1.id!=0) && (e2.type==3) && (e2.id==0))
    {
        add_instr("SET_P aux_var\n");
        add_instr("F_SU2 %s\n", v_table[e1.id].name);
        add_instr("PF_NEG_M aux_var\n");
    }

    // float acc with int var
    if ((e1.type==2) && (e1.id==0) && (e2.type==1) && (e2.id!=0))
    {
        add_instr("P_I2F_M %s\n", v_table[e2.id].name);
        add_instr("SF_SU2\n");
    }

    // float acc with int acc
    if ((e1.type==2) && (e1.id==0) && (e2.type==1) && (e2.id==0))
    {
        add_instr("I2F\n");
        add_instr("SF_SU2\n");
    }

    // float acc with float var
    if ((e1.type==2) && (e1.id==0) && (e2.type==2) && (e2.id!=0))
    {
        add_instr("F_SU1 %s\n", v_table[e2.id].name);
    }

    // float acc with float acc
    if ((e1.type==2) && (e1.id==0) && (e2.type==2) && (e2.id==0))
    {
        add_instr("SF_SU2\n");
    }

    // float acc with comp const (does not happen)
    if ((e1.type==2) && (e1.id==0) && (e2.type==5))
    {
        return oper_soma(e1, oper_neg(e2));
    }

    // float acc with comp var
    if ((e1.type==2) && (e1.id==0) && (e2.type==3) && (e2.id!=0))
    {
        get_cmp_ets(e2,&etr,&eti);

        add_instr("F_SU1 %s\n", v_table[etr.id].name);
        add_instr("PF_NEG_M %s\n", v_table[eti.id].name);
    }

    // float acc with comp acc
    if ((e1.type==2) && (e1.id==0) && (e2.type==3) && (e2.id==0))
    {
        add_instr("SET_P aux_var\n");
        add_instr("SF_SU2\n");
        add_instr("PF_NEG_M aux_var\n");
    }

    // comp const with int var
    if ((e1.type==5) && (e2.type==1) && (e2.id!=0))
    {
        get_cmp_cst(e1,&etr,&eti);

        add_instr("%s %s\n", i2f, v_table[e2.id].name);
        add_instr("F_SU2 %s\n"  , v_table[etr.id].name);
        add_instr("P_LOD %s\n"  , v_table[eti.id].name);
    }

    // comp const with int acc
    if ((e1.type==5) && (e2.type==1) && (e2.id==0))
    {
        get_cmp_cst(e1,&etr,&eti);

        add_instr("I2F\n");
        add_instr("F_SU2 %s\n", v_table[etr.id].name);
        add_instr("P_LOD %s\n", v_table[eti.id].name);
    }

    // comp const with float var
    if ((e1.type==5) && (e2.type==2) && (e2.id!=0))
    {
        get_cmp_cst(e1,&etr,&eti);

        add_instr("%s %s\n", ld, v_table[e2.id].name);
        add_instr("F_SU2 %s\n" , v_table[etr.id].name);
        add_instr("P_LOD %s\n" , v_table[eti.id].name);
    }

    // comp const with float acc
    if ((e1.type==5) && (e2.type==2) && (e2.id==0))
    {
        get_cmp_cst(e1,&etr,&eti);

        add_instr("F_SU2 %s\n", v_table[etr.id].name);
        add_instr("P_LOD %s\n", v_table[eti.id].name);
    }

    // comp const with comp const (no comp const subtraction)
    if ((e1.type==5) && (e2.type==5))
    {
        return oper_soma(e1, oper_neg(e2));
    }

    // comp const with comp var
    if ((e1.type==5) && (e2.type==3) && (e2.id!=0))
    {
        expr et1r, et2r;
        expr et1i, et2i;

        get_cmp_cst(e1,&et1r,&et1i);
        get_cmp_ets(e2,&et2r,&et2i);

        add_instr("%s %s\n", ld, v_table[et1r.id].name);
        add_instr("F_SU1 %s\n" , v_table[et2r.id].name);

        add_instr("P_LOD %s\n" , v_table[et1i.id].name);
        add_instr("F_SU1 %s\n" , v_table[et2i.id].name);
    }

    // comp const with comp acc
    if ((e1.type==5) && (e2.type==3) && (e2.id==0))
    {
        get_cmp_cst(e1,&etr,&eti);

        add_instr("SET_P aux_var\n");
        add_instr("F_SU2 %s\n", v_table[etr.id].name);

        add_instr("P_LOD %s\n", v_table[eti.id].name);
        add_instr("F_SU1 aux_var\n");
    }

    // comp var with int var
    if ((e1.type==3) && (e1.id!=0) && (e2.type==1) && (e2.id!=0))
    {
        get_cmp_ets(e1,&etr,&eti);

        add_instr("%s  %s\n", i2f, v_table[e2.id].name);
        add_instr("F_SU2 %s\n"   , v_table[etr.id].name);
        add_instr("P_LOD %s\n"   , v_table[eti.id].name);
    }

    // comp var with int acc
    if ((e1.type==3) && (e1.id!=0) && (e2.type==1) && (e2.id==0))
    {
        get_cmp_ets(e1,&etr,&eti);

        add_instr("I2F\n");
        add_instr("F_SU2 %s\n", v_table[etr.id].name);
        add_instr("P_LOD %s\n", v_table[eti.id].name);
    }

    // comp var with float var
    if ((e1.type==3) && (e1.id!=0) && (e2.type==2) && (e2.id!=0))
    {
        get_cmp_ets(e1,&etr,&eti);

        add_instr("%s %s\n", ld, v_table[e2.id].name);
        add_instr("F_SU2 %s\n" , v_table[etr.id].name);
        add_instr("P_LOD %s\n" , v_table[eti.id].name);
    }

    // comp var with float acc
    if ((e1.type==3) && (e1.id!=0) && (e2.type==2) && (e2.id==0))
    {
        get_cmp_ets(e1,&etr,&eti);

        add_instr("F_SU2 %s\n", v_table[etr.id].name);
        add_instr("P_LOD %s\n", v_table[eti.id].name);
    }

    // comp var with comp const (no comp const subtraction)
    if ((e1.type==3) && (e1.id!=0) && (e2.type==5))
    {
        return oper_soma(e1, oper_neg(e2));
    }

    // comp var with comp var
    if ((e1.type==3) && (e1.id!=0) && (e2.type==3) && (e2.id!=0))
    {
        expr et1r, et2r;
        expr et1i, et2i;

        get_cmp_ets(e1,&et1r,&et1i);
        get_cmp_ets(e2,&et2r,&et2i);

        add_instr("%s %s\n", ld, v_table[et1r.id].name);
        add_instr("F_SU1 %s\n" , v_table[et2r.id].name);

        add_instr("P_LOD %s\n" , v_table[et1i.id].name);
        add_instr("F_SU1 %s\n" , v_table[et2i.id].name);
    }

    // comp var with comp acc
    if ((e1.type==3) && (e1.id!=0) && (e2.type==3) && (e2.id==0))
    {
        get_cmp_ets(e1,&etr,&eti);

        add_instr("SET_P aux_var\n");
        add_instr("F_SU2 %s\n", v_table[etr.id].name);

        add_instr("P_LOD %s\n", v_table[eti.id].name);
        add_instr("F_SU1 aux_var\n");
    }

    // comp acc with int var
    if ((e1.type==3) && (e1.id==0) && (e2.type==1) && (e2.id!=0))
    {
        add_instr("SET_P aux_var\n");
        add_instr("P_I2F_M %s\n", v_table[e2.id].name);
        add_instr("SF_SU2\n");
        add_instr("P_LOD aux_var\n");
    }

    // comp acc with int acc
    if ((e1.type==3) && (e1.id==0) && (e2.type==1) && (e2.id==0))
    {
        add_instr("SET_P   aux_var\n" );
        add_instr("SET_P   aux_var1\n");
        add_instr("P_I2F_M aux_var\n" );
        add_instr("SF_SU2\n");
        add_instr("P_LOD   aux_var1\n");
    }

    // comp acc with float var
    if ((e1.type==3) && (e1.id==0) && (e2.type==2) && (e2.id!=0))
    {
        add_instr("SET_P aux_var\n");
        add_instr("F_SU1 %s\n", v_table[e2.id].name);
        add_instr("P_LOD aux_var\n");
    }

    // comp acc with float acc
    if ((e1.type==3) && (e1.id==0) && (e2.type==2) && (e2.id==0))
    {
        add_instr("SET_P aux_var\n" );
        add_instr("SET_P aux_var1\n");
        add_instr("F_SU1 aux_var\n" );
        add_instr("P_LOD aux_var1\n");
    }

    // comp acc with comp const (no comp const subtraction)
    if ((e1.type==3) && (e1.id==0) && (e2.type==5))
    {
        return oper_soma(e1, oper_neg(e2));
    }

    // comp acc with comp var
    if ((e1.type==3) && (e1.id==0) && (e2.type==3) && (e2.id!=0))
    {
        get_cmp_ets(e2,&etr,&eti);

        add_instr("SET_P aux_var\n");
        add_instr("F_SU1 %s\n", v_table[etr.id].name);

        add_instr("P_LOD aux_var\n");
        add_instr("F_SU1 %s\n", v_table[eti.id].name);
    }

    // comp acc with comp acc
    if ((e1.type==3) && (e1.id==0) && (e2.type==3) && (e2.id==0))
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
         if ((e1.type > 2) || (e2.type > 2))
         type = 3;
    else if ((e1.type > 1) || (e2.type > 1))
         type = 2;
    else type = 1;

    return expr_make(type, 0);
}

// multiplies two numbers
expr oper_mult(expr e1, expr e2)
{
    expr etr, eti;

    char ld [10]; if (acc_ok == 0) strcpy(ld ,"LOD"  ); else strcpy(ld ,"P_LOD"  );
    char i2f[10]; if (acc_ok == 0) strcpy(i2f,"I2F_M"); else strcpy(i2f,"P_I2F_M");

    // int var with int var
    if ((e1.type==1) && (e1.id!=0) && (e2.type==1) && (e2.id!=0))
    {
        add_instr("%s %s\n" , ld, v_table[e1.id].name);
        add_instr("MLT %s\n",     v_table[e2.id].name);
    }

    // int var with int acc
    if ((e1.type==1) && (e1.id!=0) && (e2.type==1) && (e2.id==0))
    {
        add_instr("MLT %s\n", v_table[e1.id].name);
    }

    // int var with float var
    if ((e1.type==1) && (e1.id!=0) && (e2.type==2) && (e2.id!=0))
    {
        add_instr("%s %s\n", i2f, v_table[e1.id].name);
        add_instr("F_MLT %s\n"  , v_table[e2.id].name);
    }

    // int var with float acc
    if ((e1.type==1) && (e1.id!=0) && (e2.type==2) && (e2.id==0))
    {
        add_instr("P_I2F_M %s\n", v_table[e1.id].name);
        add_instr("SF_MLT\n");
    }

    // int var with comp const
    if ((e1.type==1) && (e1.id!=0) && (e2.type==5))
    {
        get_cmp_cst(e2,&etr,&eti);

        add_instr("%s %s\n", i2f, v_table[e1.id].name);
        add_instr("F_MLT %s\n"  , v_table[etr.id].name);

        add_instr("P_I2F_M %s\n", v_table[e1.id].name);
        add_instr("F_MLT %s\n"  , v_table[eti.id].name);
    }

    // int var with comp var
    if ((e1.type==1) && (e1.id!=0) && (e2.type==3) && (e2.id!=0))
    {
        get_cmp_ets(e2,&etr,&eti);

        add_instr("%s %s\n", i2f, v_table[e1.id].name);
        add_instr("F_MLT %s\n"  , v_table[etr.id].name);

        add_instr("P_I2F_M %s\n", v_table[e1.id].name);
        add_instr("F_MLT %s\n"  , v_table[eti.id].name);
    }

    // int var with comp acc
    if ((e1.type==1) && (e1.id!=0) && (e2.type==3) && (e2.id==0))
    {
        add_instr("SET_P aux_var\n");
        add_instr("P_I2F_M %s\n", v_table[e1.id].name);
        add_instr("SF_MLT\n");

        add_instr("P_LOD aux_var\n");
        add_instr("P_I2F_M %s\n", v_table[e1.id].name);
        add_instr("SF_MLT\n");
    }

    // int acc with int var
    if ((e1.type==1) && (e1.id==0) && (e2.type==1) && (e2.id!=0))
    {
        add_instr("MLT %s\n", v_table[e2.id].name);
    }

    // int acc with int acc
    if ((e1.type==1) && (e1.id==0) && (e2.type==1) && (e2.id==0))
    {
        add_instr("S_MLT\n");
    }

    // int acc with float var
    if ((e1.type==1) && (e1.id==0) && (e2.type==2) && (e2.id!=0))
    {
        add_instr("I2F\n");
        add_instr("F_MLT %s\n", v_table[e2.id].name);
    }

    // int acc with float acc
    if ((e1.type==1) && (e1.id==0) && (e2.type==2) && (e2.id==0))
    {
        add_instr("SET_P aux_var\n");
        add_instr("I2F\n");
        add_instr("F_MLT aux_var\n");
    }

    // int acc with comp const
    if ((e1.type==1) && (e1.id==0) && (e2.type==5))
    {
        get_cmp_cst(e2,&etr,&eti);

        add_instr("I2F\n");
        add_instr("SET   aux_var\n");
        add_instr("F_MLT %s\n", v_table[etr.id].name);
        add_instr("P_LOD aux_var\n");
        add_instr("F_MLT %s\n", v_table[eti.id].name);
    }

    // int acc with comp var
    if ((e1.type==1) && (e1.id==0) && (e2.type==3) && (e2.id!=0))
    {
        get_cmp_ets(e2,&etr,&eti);

        add_instr("I2F\n");
        add_instr("SET   aux_var\n");
        add_instr("F_MLT %s\n", v_table[etr.id].name);
        add_instr("P_LOD aux_var\n");
        add_instr("F_MLT %s\n", v_table[eti.id].name);
    }

    // int acc with comp acc
    if ((e1.type==1) && (e1.id==0) && (e2.type==3) && (e2.id==0))
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
    if ((e1.type==2) && (e1.id!=0) && (e2.type==1) && (e2.id!=0))
    {
        add_instr("%s %s\n" ,i2f, v_table[e2.id].name);
        add_instr("F_MLT %s\n",   v_table[e1.id].name);
    }

    // float var with int acc
    if ((e1.type==2) && (e1.id!=0) && (e2.type==1) && (e2.id==0))
    {
        add_instr("I2F\n");
        add_instr("F_MLT %s\n", v_table[e1.id].name);
    }

    // float var with float var
    if ((e1.type==2) && (e1.id!=0) && (e2.type==2) && (e2.id!=0))
    {
        add_instr("%s %s\n", ld, v_table[e1.id].name);
        add_instr("F_MLT %s\n",  v_table[e2.id].name);
    }

    // float var with float acc
    if ((e1.type==2) && (e1.id!=0) && (e2.type==2) && (e2.id==0))
    {
        add_instr("F_MLT %s\n", v_table[e1.id].name);
    }

    // float var with comp const
    if ((e1.type==2) && (e1.id!=0) && (e2.type==5))
    {
        get_cmp_cst(e2,&etr,&eti);

        add_instr("%s %s\n", ld, v_table[e1.id].name);
        add_instr("F_MLT %s\n",  v_table[etr.id].name);

        add_instr("P_LOD %s\n",  v_table[e1.id].name);
        add_instr("F_MLT %s\n",  v_table[eti.id].name);
    }

    // float var with comp var
    if ((e1.type==2) && (e1.id!=0) && (e2.type==3) && (e2.id!=0))
    {
        get_cmp_ets(e2,&etr,&eti);

        add_instr("%s %s\n", ld, v_table[e1.id].name);
        add_instr("F_MLT %s\n",  v_table[etr.id].name);

        add_instr("P_LOD %s\n",  v_table[e1.id].name);
        add_instr("F_MLT %s\n",  v_table[eti.id].name);
    }

    // float var with comp acc
    if ((e1.type==2) && (e1.id!=0) && (e2.type==3) && (e2.id==0))
    {
        add_instr("SET_P aux_var\n");
        add_instr("F_MLT %s\n", v_table[e1.id].name);
        add_instr("P_LOD aux_var\n");
        add_instr("F_MLT %s\n", v_table[e1.id].name);
    }

    // float acc with int var
    if ((e1.type==2) && (e1.id==0) && (e2.type==1) && (e2.id!=0))
    {
        add_instr("P_I2F_M %s\n", v_table[e2.id].name);
        add_instr("SF_MLT\n");
    }

    // float acc with int acc
    if ((e1.type==2) && (e1.id==0) && (e2.type==1) && (e2.id==0))
    {
        add_instr("I2F\n");
        add_instr("SF_MLT\n");
    }

    // float acc with float var
    if ((e1.type==2) && (e1.id==0) && (e2.type==2) && (e2.id!=0))
    {
        add_instr("F_MLT %s\n", v_table[e2.id].name);
    }

    // float acc with float acc
    if ((e1.type==2) && (e1.id==0) && (e2.type==2) && (e2.id==0))
    {
        add_instr("SF_MLT\n");
    }

    // float acc with comp const
    if ((e1.type==2) && (e1.id==0) && (e2.type==5))
    {
        get_cmp_cst(e2,&etr,&eti);

        add_instr("SET   aux_var\n");
        add_instr("F_MLT %s\n", v_table[etr.id].name);
        add_instr("P_LOD %s\n", v_table[eti.id].name);
        add_instr("F_MLT aux_var\n");
    }

    // float acc with comp var
    if ((e1.type==2) && (e1.id==0) && (e2.type==3) && (e2.id!=0))
    {
        get_cmp_ets(e2,&etr,&eti);

        add_instr("SET   aux_var\n");
        add_instr("F_MLT %s\n", v_table[etr.id].name);
        add_instr("P_LOD %s\n", v_table[eti.id].name);
        add_instr("F_MLT aux_var\n");
    }

    // float acc with comp acc
    if ((e1.type==2) && (e1.id==0) && (e2.type==3) && (e2.id==0))
    {
        add_instr("SET_P aux_var \n");
        add_instr("SET_P aux_var1\n");
        add_instr("SET   aux_var2\n");
        add_instr("F_MLT aux_var1\n");
        add_instr("P_LOD aux_var \n");
        add_instr("F_MLT aux_var2\n");
    }

    // comp const with int var
    if ((e1.type==5) && (e2.type==1) && (e2.id!=0))
    {
        get_cmp_cst(e1,&etr,&eti);

        add_instr("%s %s\n", i2f, v_table[e2.id].name);
        add_instr("F_MLT %s\n"  , v_table[etr.id].name);
        add_instr("P_I2F_M %s\n", v_table[e2.id].name);
        add_instr("F_MLT %s\n"  , v_table[eti.id].name);
    }

    // comp const with int acc
    if ((e1.type==5) && (e2.type==1) && (e2.id==0))
    {
        get_cmp_cst(e1,&etr,&eti);

        add_instr("I2F\n");
        add_instr("SET   aux_var\n");
        add_instr("F_MLT %s\n"  , v_table[etr.id].name);
        add_instr("P_LOD aux_var\n");
        add_instr("F_MLT %s\n"  , v_table[eti.id].name);
    }

    // comp const with float var
    if ((e1.type==5) && (e2.type==2) && (e2.id!=0))
    {
        get_cmp_cst(e1,&etr,&eti);

        add_instr("%s %s\n", ld, v_table[e2.id].name);
        add_instr("F_MLT %s\n" , v_table[etr.id].name);
        add_instr("P_LOD %s\n" , v_table[e2.id].name);
        add_instr("F_MLT %s\n" , v_table[eti.id].name);
    }

    // comp const with float acc
    if ((e1.type==5) && (e2.type==2) && (e2.id==0))
    {
        get_cmp_cst(e1,&etr,&eti);

        add_instr("SET   aux_var\n");
        add_instr("F_MLT %s\n"  , v_table[etr.id].name);
        add_instr("P_LOD aux_var\n");
        add_instr("F_MLT %s\n"  , v_table[eti.id].name);
    }

    // comp const with comp const
    if ((e1.type==5) && (e2.type==5))
    {
        expr et1r, et1i; get_cmp_cst(e1,&et1r,&et1i);
        expr et2r, et2i; get_cmp_cst(e2,&et2r,&et2i);

        add_instr("%s %s\n", ld, v_table[et1r.id].name);
        add_instr("F_MLT %s\n" , v_table[et2r.id].name);
        add_instr("P_LOD %s\n" , v_table[et1i.id].name);
        add_instr("F_MLT %s\n" , v_table[et2i.id].name);
        add_instr("SF_SU2\n");

        add_instr("P_LOD %s\n" , v_table[et1r.id].name);
        add_instr("F_MLT %s\n" , v_table[et2i.id].name);
        add_instr("P_LOD %s\n" , v_table[et1i.id].name);
        add_instr("F_MLT %s\n" , v_table[et2r.id].name);
        add_instr("SF_ADD\n");
    }

    // comp const with comp var
    if ((e1.type==5) && (e2.type==3) && (e2.id!=0))
    {
        expr et1r, et1i; get_cmp_cst(e1,&et1r,&et1i);
        expr et2r, et2i; get_cmp_ets(e2,&et2r,&et2i);

        add_instr("%s %s\n", ld, v_table[et1r.id].name);
        add_instr("F_MLT %s\n" , v_table[et2r.id].name);
        add_instr("P_LOD %s\n" , v_table[et1i.id].name);
        add_instr("F_MLT %s\n" , v_table[et2i.id].name);
        add_instr("SF_SU2\n");

        add_instr("P_LOD %s\n" , v_table[et1r.id].name);
        add_instr("F_MLT %s\n" , v_table[et2i.id].name);
        add_instr("P_LOD %s\n" , v_table[et1i.id].name);
        add_instr("F_MLT %s\n" , v_table[et2r.id].name);
        add_instr("SF_ADD\n");
    }

    // comp const with comp acc
    if ((e1.type==5) && (e2.type==3) && (e2.id==0))
    {
        get_cmp_cst(e1,&etr,&eti);

        add_instr("SET_P aux_var \n");
        add_instr("SET   aux_var1\n");

        add_instr("F_MLT %s\n", v_table[etr.id].name);
        add_instr("P_LOD %s\n", v_table[eti.id].name);
        add_instr("F_MLT aux_var \n");
        add_instr("SF_SU2\n");

        add_instr("P_LOD %s\n" , v_table[etr.id].name);
        add_instr("F_MLT aux_var \n");
        add_instr("P_LOD %s\n" , v_table[eti.id].name);
        add_instr("F_MLT aux_var1\n");
        add_instr("SF_ADD\n");
    }

    // comp var with int var
    if ((e1.type==3) && (e1.id!=0) && (e2.type==1) && (e2.id!=0))
    {
        get_cmp_ets(e1,&etr,&eti);

        add_instr("%s %s\n", i2f, v_table[e2.id].name);
        add_instr("F_MLT %s\n"  , v_table[etr.id].name);
        add_instr("P_I2F_M %s\n", v_table[e2.id].name);
        add_instr("F_MLT %s\n"  , v_table[eti.id].name);
    }

    // comp var with int acc
    if ((e1.type==3) && (e1.id!=0) && (e2.type==1) && (e2.id==0))
    {
        get_cmp_ets(e1,&etr,&eti);

        add_instr("I2F\n");
        add_instr("SET   aux_var\n");
        add_instr("F_MLT %s\n"  , v_table[etr.id].name);
        add_instr("P_LOD aux_var\n");
        add_instr("F_MLT %s\n"  , v_table[eti.id].name);
    }

    // comp var with float var
    if ((e1.type==3) && (e1.id!=0) && (e2.type==2) && (e2.id!=0))
    {
        get_cmp_ets(e1,&etr,&eti);

        add_instr("%s %s\n", ld, v_table[e2.id].name);
        add_instr("F_MLT %s\n" , v_table[etr.id].name);
        add_instr("P_LOD %s\n" , v_table[e2.id].name);
        add_instr("F_MLT %s\n" , v_table[eti.id].name);
    }

    // comp var with float acc
    if ((e1.type==3) && (e1.id!=0) && (e2.type==2) && (e2.id==0))
    {
        get_cmp_ets(e1,&etr,&eti);

        add_instr("SET   aux_var\n");
        add_instr("F_MLT %s\n"  , v_table[etr.id].name);
        add_instr("P_LOD aux_var\n");
        add_instr("F_MLT %s\n"  , v_table[eti.id].name);
    }

    // comp var with comp const
    if ((e1.type==3) && (e1.id!=0) && (e2.type==5))
    {
        expr et1r, et1i; get_cmp_ets(e1,&et1r,&et1i);
        expr et2r, et2i; get_cmp_cst(e2,&et2r,&et2i);

        add_instr("%s %s\n", ld, v_table[et1r.id].name);
        add_instr("F_MLT %s\n" , v_table[et2r.id].name);
        add_instr("P_LOD %s\n" , v_table[et1i.id].name);
        add_instr("F_MLT %s\n" , v_table[et2i.id].name);
        add_instr("SF_SU2\n");

        add_instr("P_LOD %s\n" , v_table[et1r.id].name);
        add_instr("F_MLT %s\n" , v_table[et2i.id].name);
        add_instr("P_LOD %s\n" , v_table[et1i.id].name);
        add_instr("F_MLT %s\n" , v_table[et2r.id].name);
        add_instr("SF_ADD\n");
    }

    // comp var with comp var
    if ((e1.type==3) && (e1.id!=0) && (e2.type==3) && (e2.id!=0))
    {
        expr et1r, et1i; get_cmp_ets(e1,&et1r,&et1i);
        expr et2r, et2i; get_cmp_ets(e2,&et2r,&et2i);

        add_instr("%s %s\n", ld, v_table[et1r.id].name);
        add_instr("F_MLT %s\n" , v_table[et2r.id].name);
        add_instr("P_LOD %s\n" , v_table[et1i.id].name);
        add_instr("F_MLT %s\n" , v_table[et2i.id].name);
        add_instr("SF_SU2\n");

        add_instr("P_LOD %s\n" , v_table[et1r.id].name);
        add_instr("F_MLT %s\n" , v_table[et2i.id].name);
        add_instr("P_LOD %s\n" , v_table[et1i.id].name);
        add_instr("F_MLT %s\n" , v_table[et2r.id].name);
        add_instr("SF_ADD\n");
    }

    // comp var with comp acc
    if ((e1.type==3) && (e1.id!=0) && (e2.type==3) && (e2.id==0))
    {
        get_cmp_ets(e1,&etr,&eti);

        add_instr("SET_P aux_var \n");
        add_instr("SET   aux_var1\n");

        add_instr("F_MLT %s\n", v_table[etr.id].name);
        add_instr("P_LOD %s\n", v_table[eti.id].name);
        add_instr("F_MLT aux_var \n");
        add_instr("SF_SU2\n");

        add_instr("P_LOD %s\n" , v_table[etr.id].name);
        add_instr("F_MLT aux_var \n");
        add_instr("P_LOD %s\n" , v_table[eti.id].name);
        add_instr("F_MLT aux_var1\n");
        add_instr("SF_ADD\n");
    }

    // comp acc with int var
    if ((e1.type==3) && (e1.id==0) && (e2.type==1) && (e2.id!=0))
    {
        add_instr("SET_P aux_var\n" );
        add_instr("P_I2F_M %s\n", v_table[e2.id].name);
        add_instr("SET   aux_var1\n");
        add_instr("SF_MLT\n");
        add_instr("P_LOD aux_var\n" );
        add_instr("F_MLT aux_var1\n");
    }

    // comp acc with int acc
    if ((e1.type==3) && (e1.id==0) && (e2.type==1) && (e2.id==0))
    {
        add_instr("I2F\n");
        add_instr("SET_P aux_var\n" );
        add_instr("SET_P aux_var1\n");
        add_instr("F_MLT aux_var\n" );
        add_instr("P_LOD aux_var1\n");
        add_instr("F_MLT aux_var\n" );
    }

    // comp acc with float var
    if ((e1.type==3) && (e1.id==0) && (e2.type==2) && (e2.id!=0))
    {
        add_instr("SET_P aux_var\n");
        add_instr("F_MLT %s\n", v_table[e2.id].name);
        add_instr("P_LOD aux_var\n");
        add_instr("F_MLT %s\n", v_table[e2.id].name);
    }

    // comp acc with float acc
    if ((e1.type==3) && (e1.id==0) && (e2.type==2) && (e2.id==0))
    {
        add_instr("SET_P aux_var\n" );
        add_instr("SET_P aux_var1\n");
        add_instr("F_MLT aux_var\n" );
        add_instr("P_LOD aux_var1\n");
        add_instr("F_MLT aux_var\n ");
    }

    // comp acc with comp const
    if ((e1.type==3) && (e1.id==0) && (e2.type==5))
    {
        get_cmp_cst(e2,&etr,&eti);

        add_instr("SET_P aux_var \n");
        add_instr("SET   aux_var1\n");
        add_instr("F_MLT %s\n", v_table[etr.id].name);
        add_instr("P_LOD aux_var \n");
        add_instr("F_MLT %s\n", v_table[eti.id].name);
        add_instr("SF_SU2\n");

        add_instr("P_LOD aux_var \n");
        add_instr("F_MLT %s\n", v_table[etr.id].name);
        add_instr("P_LOD aux_var1\n");
        add_instr("F_MLT %s\n", v_table[eti.id].name);
        add_instr("SF_ADD\n");
    }

    // comp acc with comp var
    if ((e1.type==3) && (e1.id==0) && (e2.type==3) && (e2.id!=0))
    {
        get_cmp_ets(e2,&etr,&eti);

        add_instr("SET_P aux_var \n");
        add_instr("SET   aux_var1\n");
        add_instr("F_MLT %s\n", v_table[etr.id].name);
        add_instr("P_LOD aux_var \n");
        add_instr("F_MLT %s\n", v_table[eti.id].name);
        add_instr("SF_SU2\n");

        add_instr("P_LOD aux_var \n");
        add_instr("F_MLT %s\n", v_table[etr.id].name);
        add_instr("P_LOD aux_var1\n");
        add_instr("F_MLT %s\n", v_table[eti.id].name);
        add_instr("SF_ADD\n");
    }

    // comp acc with comp acc
    if ((e1.type==3) && (e1.id==0) && (e2.type==3) && (e2.id==0))
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
         if ((e1.type > 2) || (e2.type > 2))
         type = 3;
    else if ((e1.type > 1) || (e2.type > 1))
         type = 2;
    else type = 1;

    return expr_make(type, 0);
}

// Emits er² + ei² into the accumulator -- the |comp|² used as the division
// denominator and by mod2/abs. er/ei are the real/imag halves (data-memory
// cells). The sum is commutative, so square whichever half the accumulator
// already holds first: its LOD is then dropped by the add_instr peephole (e.g.
// `r = x/y` right after `y = ...` leaves d live, so squaring d² first saves the
// load). When neither half is live it falls to the original er²-first order, so
// the emitted assembly is unchanged.
void emit_sq_sum(expr er, expr ei)
{
    if (ei.id != 0 && acc_holds(v_table[ei.id].name) &&
        !(er.id != 0 && acc_holds(v_table[er.id].name)))
    {
        oper_mult(ei, ei);
        oper_mult(er, er);
    }
    else
    {
        oper_mult(er, er);
        oper_mult(ei, ei);
    }
    oper_soma(expr_make(2, 0), expr_make(2, 0));
}

// division between two numbers
expr oper_divi(expr e1, expr e2)
{
    expr etr, eti;

    char ld [10]; if (acc_ok == 0) strcpy(ld ,"LOD"  ); else strcpy(ld ,"P_LOD"  );
    char i2f[10]; if (acc_ok == 0) strcpy(i2f,"I2F_M"); else strcpy(i2f,"P_I2F_M");

    // int var with int var
    if ((e1.type==1) && (e1.id!=0) && (e2.type==1) && (e2.id!=0))
    {
        add_instr("%s %s\n" , ld, v_table[e2.id].name);
        add_instr("DIV %s\n",     v_table[e1.id].name);    
    }

    // int var with int acc
    if ((e1.type==1) && (e1.id!=0) && (e2.type==1) && (e2.id==0))
    {
        add_instr("DIV %s\n", v_table[e1.id].name);
    }

    // int var with float var
    if ((e1.type==1) && (e1.id!=0) && (e2.type==2) && (e2.id!=0))
    {
        add_instr("%s %s\n" , i2f, v_table[e1.id].name);
        add_instr("P_LOD %s\n"   , v_table[e2.id].name);
        add_instr("SF_DIV\n");
    }

    // int var with float acc
    if ((e1.type==1) && (e1.id!=0) && (e2.type==2) && (e2.id==0))
    {
        add_instr("SET   aux_var\n");
        add_instr("I2F_M %s\n", v_table[e1.id].name);
        add_instr("P_LOD aux_var\n");
        add_instr("SF_DIV\n");
    }

    // int var with comp const
    if ((e1.type==1) && (e1.id!=0) && (e2.type==5))
    {
        get_cmp_cst(e2,&etr,&eti);

        emit_sq_sum(etr, eti);
        add_instr("SET aux_var\n");   // save the result

        acc_ok = 0;                   // libera acumulador

        oper_mult(e1, etr);           // multiply int with real part
        add_instr("P_LOD aux_var\n"); // fetch the denominator
        oper_divi(expr_make(2, 0), expr_make(2, 0));     // faz a divisao

        oper_mult(e1, eti);           // multiply int with imag part
        add_instr("P_LOD aux_var\n"); // fetch the denominator
        oper_divi(expr_make(2, 0), expr_make(2, 0));     // faz a divisao
        oper_neg (expr_make(2, 0));            // nega a parte imaginaria
    }

    // int var with comp var
    if ((e1.type==1) && (e1.id!=0) && (e2.type==3) && (e2.id!=0))
    {
        get_cmp_ets(e2,&etr,&eti);

        emit_sq_sum(etr, eti);
        add_instr("SET aux_var\n");   // save the result

        acc_ok = 0;                   // libera acumulador

        oper_mult(e1, etr);           // multiply int with real part
        add_instr("P_LOD aux_var\n"); // fetch the denominator
        oper_divi(expr_make(2, 0), expr_make(2, 0));     // faz a divisao

        oper_mult(e1, eti);           // multiply int with imag part
        add_instr("P_LOD aux_var\n"); // fetch the denominator
        oper_divi(expr_make(2, 0), expr_make(2, 0));     // faz a divisao
        oper_neg (expr_make(2, 0));            // nega a parte imaginaria
    }

    // int var with comp acc
    if ((e1.type==1) && (e1.id!=0) && (e2.type==3) && (e2.id==0))
    {
        add_instr("SET_P aux_var \n"); // save the imaginary part
        add_instr("SET   aux_var1\n"); // save the real part

        add_instr("F_MLT aux_var1\n"); // parte real ao quadrado
        add_instr("P_LOD aux_var \n"); // fetch the imaginary part
        add_instr("F_MLT aux_var \n"); // parte imag ao quadrado
        add_instr("SF_ADD        \n"); // soma os quadrados
        add_instr("SET   aux_var2\n"); // save the squared magnitude

        add_instr("I2F_M %s\n", v_table[e1.id].name); 
        add_instr("SET   aux_var3\n"); // save the float
        add_instr("F_MLT aux_var1\n");
        add_instr("P_LOD aux_var2\n"); // fetch the squared magnitude
        oper_divi(expr_make(2, 0), expr_make(2, 0));      // faz a divisao

        add_instr("P_LOD aux_var3\n"); // fetch the float
        add_instr("F_MLT aux_var \n");
        add_instr("P_LOD aux_var2\n"); // fetch the squared magnitude
        oper_divi(expr_make(2, 0), expr_make(2, 0));      // faz a divisao
        oper_neg (expr_make(2, 0));
    }

    // int acc with int var
    if ((e1.type==1) && (e1.id==0) && (e2.type==1) && (e2.id!=0))
    {
        add_instr("%s %s\n", ld, v_table[e2.id].name);
        add_instr("S_DIV\n");
    }

    // int acc with int acc
    if ((e1.type==1) && (e1.id==0) && (e2.type==1) && (e2.id==0))
    {
        add_instr("S_DIV\n");
    }

    // int acc with float var
    if ((e1.type==1) && (e1.id==0) && (e2.type==2) && (e2.id!=0))
    {
        add_instr("I2F\n");
        add_instr("P_LOD %s\n", v_table[e2.id].name);
        add_instr("SF_DIV\n");
    }

    // int acc with float acc
    if ((e1.type==1) && (e1.id==0) && (e2.type==2) && (e2.id==0))
    {
        add_instr("SET_P aux_soma\n");
        add_instr("I2F\n");
        add_instr("P_LOD aux_soma\n");
        add_instr("SF_DIV\n");
    }

    // int acc with comp const
    if ((e1.type==1) && (e1.id==0) && (e2.type==5))
    {
        get_cmp_cst(e2,&etr,&eti);

        add_instr("I2F\n");
        add_instr("SET aux_var\n");
        acc_ok = 0;

        emit_sq_sum(etr, eti);
        add_instr("SET   aux_var1\n"); // save the result

        add_instr("LOD   aux_var\n");
        oper_mult(expr_make(2, 0), etr);         // multiply float with real part
        add_instr("P_LOD aux_var1\n"); // fetch the denominator
        oper_divi(expr_make(2, 0), expr_make(2, 0));      // faz a divisao

        add_instr("P_LOD  aux_var\n");
        oper_mult(expr_make(2, 0), eti);         // multiply float with imag part
        add_instr("P_LOD aux_var1\n"); // fetch the denominator
        oper_divi(expr_make(2, 0), expr_make(2, 0));      // faz a divisao
        oper_neg (expr_make(2, 0));
    }

    // int acc with comp var
    if ((e1.type==1) && (e1.id==0) && (e2.type==3) && (e2.id!=0))
    {
        get_cmp_ets(e2,&etr,&eti);

        add_instr("I2F\n");
        add_instr("SET   aux_var\n");
        acc_ok = 0;

        emit_sq_sum(etr, eti);
        add_instr("SET   aux_var1\n");  // save the result

        add_instr("LOD   aux_var\n" );
        oper_mult(expr_make(2, 0), etr);          // multiply float with real part
        add_instr("P_LOD aux_var1\n");  // fetch the denominator
        oper_divi(expr_make(2, 0), expr_make(2, 0));       // faz a divisao

        add_instr("P_LOD  aux_var\n" );
        oper_mult(expr_make(2, 0), eti);          // multiply float with imag part
        add_instr("P_LOD  aux_var1\n"); // fetch the denominator
        oper_divi(expr_make(2, 0), expr_make(2, 0));       // faz a divisao
        oper_neg (expr_make(2, 0));
    }

    // int acc with comp acc
    if ((e1.type==1) && (e1.id==0) && (e2.type==3) && (e2.id==0))
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
        oper_divi(expr_make(2, 0), expr_make(2, 0));      // faz a divisao

        add_instr("P_LOD aux_var2\n"); // fetch the float
        add_instr("F_MLT aux_var \n");
        add_instr("P_LOD aux_var3\n"); // fetch the squared magnitude
        oper_divi(expr_make(2, 0), expr_make(2, 0));      // faz a divisao
        oper_neg (expr_make(2, 0));
    }

    // float var with int var
    if ((e1.type==2) && (e1.id!=0) && (e2.type==1) && (e2.id!=0))
    {
        add_instr("%s %s\n", i2f, v_table[e2.id].name);
        add_instr("F_DIV %s\n"  , v_table[e1.id].name);
    }

    // float var with int acc
    if ((e1.type==2) && (e1.id!=0) && (e2.type==1) && (e2.id==0))
    {
        add_instr("I2F\n");
        add_instr("F_DIV %s\n", v_table[e1.id].name);
    }

    // float var with float var
    if ((e1.type==2) && (e1.id!=0) && (e2.type==2) && (e2.id!=0))
    {
        add_instr("%s %s\n" , ld, v_table[e2.id].name);
        add_instr("F_DIV %s\n"  , v_table[e1.id].name);
    }

    // float var with float acc
    if ((e1.type==2) && (e1.id!=0) && (e2.type==2) && (e2.id==0))
    {
        add_instr("F_DIV %s\n", v_table[e1.id].name);
    }

    // float var with comp const
    if ((e1.type==2) && (e1.id!=0) && (e2.type==5))
    {
        get_cmp_cst(e2,&etr,&eti);

        emit_sq_sum(etr, eti);
        add_instr("SET   aux_var\n"); // save the result
        acc_ok = 0;

        oper_mult(e1, etr);           // multiply float with real part
        add_instr("P_LOD aux_var\n"); // fetch the denominator
        oper_divi(expr_make(2, 0), expr_make(2, 0));     // faz a divisao

        oper_mult(e1, eti);           // multiply float with imag part
        add_instr("P_LOD aux_var\n"); // fetch the denominator
        oper_divi(expr_make(2, 0), expr_make(2, 0));     // faz a divisao
        oper_neg (expr_make(2, 0));
    }

    // float var with comp var
    if ((e1.type==2) && (e1.id!=0) && (e2.type==3) && (e2.id!=0))
    {
        get_cmp_ets(e2,&etr,&eti);

        emit_sq_sum(etr, eti);
        add_instr("SET   aux_var\n"); // save the result
        acc_ok = 0;

        oper_mult(e1, etr);           // multiply float with real part
        add_instr("P_LOD aux_var\n"); // fetch the denominator
        oper_divi(expr_make(2, 0), expr_make(2, 0));     // faz a divisao

        oper_mult(e1, eti);           // multiply float with imag part
        add_instr("P_LOD aux_var\n"); // fetch the denominator
        oper_divi(expr_make(2, 0), expr_make(2, 0));     // faz a divisao
        oper_neg (expr_make(2, 0));
    }

    // float var with comp acc
    if ((e1.type==2) && (e1.id!=0) && (e2.type==3) && (e2.id==0))
    {
        add_instr("SET_P aux_var \n");             // save the imaginary part
        add_instr("SET   aux_var1\n");             // save the real part
        add_instr("F_MLT aux_var1\n");
        add_instr("P_LOD aux_var \n");             // fetch the imaginary part
        add_instr("F_MLT aux_var \n");
        add_instr("SF_ADD        \n");
        add_instr("SET   aux_var2\n");             // save the squared magnitude

        add_instr("LOD %s\n"  , v_table[e1.id].name); // load the float
        add_instr("F_MLT aux_var1\n");
        add_instr("P_LOD aux_var2\n");             // fetch the squared magnitude
        oper_divi(expr_make(2, 0), expr_make(2, 0));                  // faz a divisao

        add_instr("P_LOD %s\n", v_table[e1.id].name); // load the float
        add_instr("F_MLT aux_var \n");
        add_instr("P_LOD aux_var2\n");             // fetch the squared magnitude
        oper_divi(expr_make(2, 0), expr_make(2, 0));                  // faz a divisao
        oper_neg (expr_make(2, 0));
    }

    // float acc with int var
    if ((e1.type==2) && (e1.id==0) && (e2.type==1) && (e2.id!=0))
    {
        add_instr("P_I2F_M %s\n", v_table[e2.id].name);
        add_instr("SF_DIV\n");
    }

    // float acc with int acc
    if ((e1.type==2) && (e1.id==0) && (e2.type==1) && (e2.id==0))
    {
        add_instr("I2F\n");
        add_instr("SF_DIV\n");
    }

    // float acc with float var
    if ((e1.type==2) && (e1.id==0) && (e2.type==2) && (e2.id!=0))
    {
        add_instr("P_LOD %s\n", v_table[e2.id].name);
        add_instr("SF_DIV\n");
    }

    // float acc with float acc
    if ((e1.type==2) && (e1.id==0) && (e2.type==2) && (e2.id==0))
    {
        add_instr("SF_DIV\n");
    }

    // float acc with comp const
    if ((e1.type==2) && (e1.id==0) && (e2.type==5))
    {
        get_cmp_cst(e2,&etr,&eti);

        add_instr("SET   aux_var\n");
        acc_ok = 0;

        emit_sq_sum(etr, eti);
        add_instr("SET   aux_var1\n"); // save the result

        add_instr("LOD   aux_var \n");
        oper_mult(expr_make(2, 0), etr);         // multiply float with real part
        add_instr("P_LOD aux_var1\n"); // fetch the denominator
        oper_divi(expr_make(2, 0), expr_make(2, 0));      // faz a divisao

        add_instr("P_LOD aux_var \n");
        oper_mult(expr_make(2, 0), eti);         // multiply float with imag part
        add_instr("P_LOD aux_var1\n"); // fetch the denominator
        oper_divi(expr_make(2, 0), expr_make(2, 0));      // faz a divisao
        oper_neg (expr_make(2, 0));
    }

    // float acc with comp var
    if ((e1.type==2) && (e1.id==0) && (e2.type==3) && (e2.id!=0))
    {
        get_cmp_ets(e2,&etr,&eti);

        add_instr("SET aux_var\n");
        acc_ok = 0;

        emit_sq_sum(etr, eti);
        add_instr("SET   aux_var1\n"); // save the result

        add_instr("LOD   aux_var\n");
        oper_mult(expr_make(2, 0), etr);         // multiply float with real part
        add_instr("P_LOD aux_var1\n"); // fetch the denominator
        oper_divi(expr_make(2, 0), expr_make(2, 0));      // faz a divisao

        add_instr("P_LOD aux_var\n");
        oper_mult(expr_make(2, 0), eti);         // multiply float with imag part
        add_instr("P_LOD aux_var1\n"); // fetch the denominator
        oper_divi(expr_make(2, 0), expr_make(2, 0));      // faz a divisao
        oper_neg (expr_make(2, 0));
    }

    // float acc with comp acc
    if ((e1.type==2) && (e1.id==0) && (e2.type==3) && (e2.id==0))
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
        oper_divi(expr_make(2, 0), expr_make(2, 0));      // faz a divisao

        add_instr("P_LOD aux_var2\n"); // fetch the float
        add_instr("F_MLT aux_var \n");
        add_instr("P_LOD aux_var3\n"); // fetch the squared magnitude
        oper_divi(expr_make(2, 0), expr_make(2, 0));      // faz a divisao
        oper_neg (expr_make(2, 0));
    }

    // comp const with int var
    if ((e1.type==5) && (e2.type==1) && (e2.id!=0))
    {
        get_cmp_cst(e1,&etr,&eti);

        oper_divi(etr, e2);
        oper_divi(eti, e2);
    }

    // comp const with int acc
    if ((e1.type==5) && (e2.type==1) && (e2.id==0))
    {
        get_cmp_cst(e1,&etr,&eti);

        add_instr("I2F\n");
        add_instr("SET   aux_var\n");
        oper_divi(etr, expr_make(2, 0));
        add_instr("P_LOD aux_var\n");
        oper_divi(eti, expr_make(2, 0));
    }

    // comp const with float var
    if ((e1.type==5) && (e2.type==2) && (e2.id!=0))
    {
        get_cmp_cst(e1,&etr,&eti);

        oper_divi(etr, e2);
        oper_divi(eti, e2);
    }

    // comp const with float acc
    if ((e1.type==5) && (e2.type==2) && (e2.id==0))
    {
        get_cmp_cst(e1,&etr,&eti);

        add_instr("SET   aux_var\n");
        oper_divi(etr, expr_make(2, 0));
        add_instr("P_LOD aux_var\n");
        oper_divi(eti, expr_make(2, 0));
    }

    // comp const with comp const
    if ((e1.type==5) && (e2.type==5))
    {
        expr et1r, et2r;
        expr et1i, et2i;

        get_cmp_cst(e1,&et1r,&et1i);
        get_cmp_cst(e2,&et2r,&et2i);

        emit_sq_sum(et2r, et2i);
        add_instr("SET   aux_var\n");
        acc_ok = 0;

        oper_mult(et1r, et2r);
        oper_mult(et1i, et2i);
        oper_soma(expr_make(2, 0), expr_make(2, 0));
        add_instr("P_LOD aux_var\n");
        oper_divi(expr_make(2, 0), expr_make(2, 0));

        oper_mult(et1i, et2r);
        oper_mult(et1r, et2i);
        oper_subt(expr_make(2, 0), expr_make(2, 0));
        add_instr("P_LOD aux_var\n");
        oper_divi(expr_make(2, 0), expr_make(2, 0));
    }

    // comp const with comp var
    if ((e1.type==5) && (e2.type==3) && (e2.id!=0))
    {
        expr et1r, et2r;
        expr et1i, et2i;

        get_cmp_cst(e1,&et1r,&et1i);
        get_cmp_ets(e2,&et2r,&et2i);

        emit_sq_sum(et2r, et2i);
        add_instr("SET aux_var\n");
        acc_ok = 0;

        oper_mult(et1r, et2r);
        oper_mult(et1i, et2i);
        oper_soma(expr_make(2, 0), expr_make(2, 0));
        add_instr("P_LOD aux_var\n");
        oper_divi(expr_make(2, 0), expr_make(2, 0));

        oper_mult(et1i, et2r);
        oper_mult(et1r, et2i);
        oper_subt(expr_make(2, 0), expr_make(2, 0));
        add_instr("P_LOD aux_var\n");
        oper_divi(expr_make(2, 0), expr_make(2, 0));
    }

    // comp const with comp acc
    if ((e1.type==5) && (e2.type==3) && (e2.id==0))
    {
        get_cmp_cst(e1,&etr,&eti);

        add_instr("SET_P aux_var \n");
        add_instr("SET   aux_var1\n");
        add_instr("F_MLT aux_var1\n");
        add_instr("P_LOD aux_var \n");
        add_instr("F_MLT aux_var \n");
        oper_soma(expr_make(2, 0), expr_make(2, 0));
        add_instr("SET   aux_var2\n");

        add_instr("LOD   aux_var1\n");
        oper_mult(etr, expr_make(2, 0));
        add_instr("P_LOD aux_var \n");
        oper_mult(eti, expr_make(2, 0));
        oper_soma(expr_make(2, 0), expr_make(2, 0));
        add_instr("P_LOD aux_var2\n");
        oper_divi(expr_make(2, 0), expr_make(2, 0));

        add_instr("P_LOD aux_var1\n");
        oper_mult(eti, expr_make(2, 0));
        add_instr("P_LOD aux_var \n");
        oper_mult(etr, expr_make(2, 0));
        oper_subt(expr_make(2, 0), expr_make(2, 0));
        add_instr("P_LOD aux_var2\n");
        oper_divi(expr_make(2, 0), expr_make(2, 0));
    }

    // comp var with int var
    if ((e1.type==3) && (e1.id!=0) && (e2.type==1) && (e2.id!=0))
    {
        get_cmp_ets(e1,&etr,&eti);

        oper_divi(etr, e2);
        oper_divi(eti, e2);
    }

    // comp var with int acc
    if ((e1.type==3) && (e1.id!=0) && (e2.type==1) && (e2.id==0))
    {
        get_cmp_ets(e1,&etr,&eti);

        add_instr("I2F\n");
        add_instr("SET   aux_var\n");
        oper_divi(etr, expr_make(2, 0));
        add_instr("P_LOD aux_var\n");
        oper_divi(eti, expr_make(2, 0));
    }

    // comp var with float var
    if ((e1.type==3) && (e1.id!=0) && (e2.type==2) && (e2.id!=0))
    {
        get_cmp_ets(e1,&etr,&eti);

        oper_divi(etr, e2);
        oper_divi(eti, e2);
    }

    // comp var with float acc
    if ((e1.type==3) && (e1.id!=0) && (e2.type==2) && (e2.id==0))
    {
        get_cmp_ets(e1,&etr,&eti);

        add_instr("SET   aux_var\n");
        oper_divi(etr, expr_make(2, 0));
        add_instr("P_LOD aux_var\n");
        oper_divi(eti, expr_make(2, 0));
    }

    // comp var with comp const
    if ((e1.type==3) && (e1.id!=0) && (e2.type==5))
    {
        expr et1r, et2r;
        expr et1i, et2i;

        get_cmp_ets(e1,&et1r,&et1i);
        get_cmp_cst(e2,&et2r,&et2i);

        emit_sq_sum(et2r, et2i);
        add_instr("SET   aux_var\n");
        acc_ok = 0;

        oper_mult(et1r, et2r);
        oper_mult(et1i, et2i);
        oper_soma(expr_make(2, 0), expr_make(2, 0));
        add_instr("P_LOD aux_var\n");
        oper_divi(expr_make(2, 0), expr_make(2, 0));

        oper_mult(et1i, et2r);
        oper_mult(et1r, et2i);
        oper_subt(expr_make(2, 0), expr_make(2, 0));
        add_instr("P_LOD aux_var\n");
        oper_divi(expr_make(2, 0), expr_make(2, 0));
    }

    // comp var with comp var
    if ((e1.type==3) && (e1.id!=0) && (e2.type==3) && (e2.id!=0))
    {
        expr et1r, et2r;
        expr et1i, et2i;

        get_cmp_ets(e1,&et1r,&et1i);
        get_cmp_ets(e2,&et2r,&et2i);

        emit_sq_sum(et2r, et2i);
        add_instr("SET   aux_var\n");
        acc_ok = 0;

        oper_mult(et1r, et2r);
        oper_mult(et1i, et2i);
        oper_soma(expr_make(2, 0), expr_make(2, 0));
        add_instr("P_LOD aux_var\n");
        oper_divi(expr_make(2, 0), expr_make(2, 0));

        oper_mult(et1i, et2r);
        oper_mult(et1r, et2i);
        oper_subt(expr_make(2, 0), expr_make(2, 0));
        add_instr("P_LOD aux_var\n");
        oper_divi(expr_make(2, 0), expr_make(2, 0));
    }

    // comp var with comp acc
    if ((e1.type==3) && (e1.id!=0) && (e2.type==3) && (e2.id==0))
    {
        get_cmp_ets(e1,&etr,&eti);

        add_instr("SET_P aux_var \n");
        add_instr("SET   aux_var1\n");
        add_instr("F_MLT aux_var1\n");
        add_instr("P_LOD aux_var \n");
        add_instr("F_MLT aux_var \n");
        oper_soma(expr_make(2, 0), expr_make(2, 0));
        add_instr("SET   aux_var2\n");

        add_instr("LOD   aux_var1\n");
        oper_mult(etr, expr_make(2, 0));
        add_instr("P_LOD aux_var \n");
        oper_mult(eti, expr_make(2, 0));
        oper_soma(expr_make(2, 0), expr_make(2, 0));
        add_instr("P_LOD aux_var2\n");
        oper_divi(expr_make(2, 0), expr_make(2, 0));

        add_instr("P_LOD aux_var1\n");
        oper_mult(eti, expr_make(2, 0));
        add_instr("P_LOD aux_var \n");
        oper_mult(etr, expr_make(2, 0));
        oper_subt(expr_make(2, 0), expr_make(2, 0));
        add_instr("P_LOD aux_var2\n");
        oper_divi(expr_make(2, 0), expr_make(2, 0));
    }

    // comp acc with int var
    if ((e1.type==3) && (e1.id==0) && (e2.type==1) && (e2.id!=0))
    {
        add_instr("SET_P aux_var \n");
        add_instr("P_I2F_M %s\n", v_table[e2.id].name);
        add_instr("SET   aux_var1\n");
        oper_divi(expr_make(2, 0), expr_make(2, 0));
        add_instr("P_LOD aux_var \n");
        add_instr("P_LOD aux_var1\n");
        oper_divi(expr_make(2, 0), expr_make(2, 0));
    }

    // comp acc with int acc
    if ((e1.type==3) && (e1.id==0) && (e2.type==1) && (e2.id==0))
    {
        add_instr("I2F\n");
        add_instr("SET_P aux_var \n");
        add_instr("SET_P aux_var1\n");
        add_instr("P_LOD aux_var \n");
        oper_divi(expr_make(2, 0), expr_make(2, 0));
        add_instr("P_LOD aux_var1\n");
        add_instr("P_LOD aux_var \n");
        oper_divi(expr_make(2, 0), expr_make(2, 0));
    }

    // comp acc with float var
    if ((e1.type==3) && (e1.id==0) && (e2.type==2) && (e2.id!=0))
    {
        add_instr("SET_P aux_var\n");
        add_instr("P_LOD %s\n", v_table[e2.id].name);
        oper_divi(expr_make(2, 0), expr_make(2, 0));
        add_instr("P_LOD aux_var\n");
        add_instr("P_LOD %s\n", v_table[e2.id].name);
        oper_divi(expr_make(2, 0), expr_make(2, 0));
    }

    // comp acc with float acc
    if ((e1.type==3) && (e1.id==0) && (e2.type==2) && (e2.id==0))
    {
        add_instr("SET_P aux_var \n");
        add_instr("SET_P aux_var1\n");
        add_instr("P_LOD aux_var \n");
        oper_divi(expr_make(2, 0), expr_make(2, 0));
        add_instr("P_LOD aux_var1\n");
        add_instr("P_LOD aux_var \n");
        oper_divi(expr_make(2, 0), expr_make(2, 0));
    }

    // comp acc with comp const
    if ((e1.type==3) && (e1.id==0) && (e2.type==5))
    {
        get_cmp_cst(e2,&etr,&eti);

        add_instr("SET_P aux_var \n");
        add_instr("SET   aux_var1\n");
        acc_ok = 0;

        emit_sq_sum(etr, eti);
        add_instr("SET   aux_var2\n");

        add_instr("LOD   aux_var1\n");
        oper_mult(etr, expr_make(2, 0));
        add_instr("P_LOD aux_var \n");
        oper_mult(eti, expr_make(2, 0));
        oper_soma(expr_make(2, 0), expr_make(2, 0));
        add_instr("P_LOD aux_var2\n");
        oper_divi(expr_make(2, 0), expr_make(2, 0));

        add_instr("P_LOD aux_var1\n");
        oper_mult(eti, expr_make(2, 0));
        add_instr("P_LOD aux_var \n");
        oper_mult(etr, expr_make(2, 0));
        oper_subt(expr_make(2, 0), expr_make(2, 0));
        add_instr("P_LOD aux_var2\n");
        oper_divi(expr_make(2, 0), expr_make(2, 0));
        oper_neg (expr_make(2, 0));
    }

    // comp acc with comp var
    if ((e1.type==3) && (e1.id==0) && (e2.type==3) && (e2.id!=0))
    {
        get_cmp_ets(e2,&etr,&eti);

        add_instr("SET_P aux_var \n");
        add_instr("SET   aux_var1\n");
        acc_ok = 0;

        emit_sq_sum(etr, eti);
        add_instr("SET   aux_var2\n");

        add_instr("LOD   aux_var1\n");
        oper_mult(etr, expr_make(2, 0));
        add_instr("P_LOD aux_var \n");
        oper_mult(eti, expr_make(2, 0));
        oper_soma(expr_make(2, 0), expr_make(2, 0));
        add_instr("P_LOD aux_var2\n");
        oper_divi(expr_make(2, 0), expr_make(2, 0));

        add_instr("P_LOD aux_var1\n");
        oper_mult(eti, expr_make(2, 0));
        add_instr("P_LOD aux_var \n");
        oper_mult(etr, expr_make(2, 0));
        oper_subt(expr_make(2, 0), expr_make(2, 0));
        add_instr("P_LOD aux_var2\n");
        oper_divi(expr_make(2, 0), expr_make(2, 0));
        oper_neg (expr_make(2, 0));
    }

    // comp acc with comp acc
    if ((e1.type==3) && (e1.id==0) && (e2.type==3) && (e2.id==0))
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
        oper_divi(expr_make(2, 0), expr_make(2, 0));

        add_instr("P_LOD aux_var3\n");
        add_instr("F_MLT aux_var \n");
        add_instr("P_LOD aux_var2\n");
        add_instr("F_MLT aux_var1\n");
        oper_subt(expr_make(2, 0), expr_make(2, 0));
        add_instr("P_LOD aux_var4\n");
        oper_divi(expr_make(2, 0), expr_make(2, 0));
        oper_neg (expr_make(2, 0));
    }

    acc_ok = 1;

    int type;
         if ((e1.type > 2) || (e2.type > 2))
         type = 3;
    else if ((e1.type > 1) || (e2.type > 1))
         type = 2;
    else type = 1;

    return expr_make(type, 0);
}

// division remainder
expr oper_mod(expr e1, expr e2)
{

    if ((e1.type > 1) || (e2.type > 1))
        {fprintf(stderr, MSG_ERR_MOD_NON_INT, line_num+1); exit(EXIT_FAILURE);}

    char ld[10]; if (acc_ok == 0) strcpy(ld,"LOD"); else strcpy(ld,"P_LOD");

    // int var with int var
    if ((e1.type == 1) && (e1.id != 0) && (e2.type == 1) && (e2.id != 0))
    {
        add_instr("%s %s\n" , ld, v_table[e2.id].name);
        add_instr("MOD %s\n",     v_table[e1.id].name);
    }

    // int var with int acc
    if ((e1.type == 1) && (e1.id != 0) && (e2.type == 1) && (e2.id == 0))
    {
        add_instr("MOD %s\n", v_table[e1.id].name);
    }

    // int acc with int var
    if ((e1.type == 1) && (e1.id == 0) && (e2.type == 1) && (e2.id != 0))
    {
        add_instr("P_LOD %s\n", v_table[e2.id].name);
        add_instr("S_MOD\n");
    }

    // int acc with int acc
    if ((e1.type == 1) && (e1.id == 0) && (e2.type == 1) && (e2.id == 0))
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
    if ((e1.type==1) && (e1.id!=0) && (e2.type==1) && (e2.id!=0))
    {
        add_instr("%s %s\n", ld, v_table[e2.id].name);
        add_instr("%s %s\n", op, v_table[e1.id].name);
    }

    // int var with int acc
    if ((e1.type==1) && (e1.id!=0) && (e2.type==1) && (e2.id==0))
    {
        add_instr("%s %s\n", op, v_table[e1.id].name);
    }

    // int var with float var
    if ((e1.type==1) && (e1.id!=0) && (e2.type==2) && (e2.id!=0))
    {
        add_instr("%s %s\n", i2f, v_table[e1.id].name);
        add_instr("P_LOD %s\n"  , v_table[e2.id].name);

        if (strcmp(op,"EQU")==0) add_instr("S_EQU\n"); else add_instr("SF_%s\n", op);
    }

    // int var with float acc
    if ((e1.type==1) && (e1.id!=0) && (e2.type==2) && (e2.id==0))
    {
        add_instr("SET   aux_var\n");
        add_instr("I2F_M %s\n", v_table[e1.id].name);
        add_instr("P_LOD aux_var\n");
        
        if (strcmp(op,"EQU")==0) add_instr("S_EQU\n"); else add_instr("SF_%s\n", op);
    }

    // int var with comp const
    if ((e1.type==1) && (e1.id!=0) && (e2.type==5))
    {
        fprintf(stdout, MSG_WARN_CMP_INT_COMP, line_num+1);

        oper_mult(e1, e1);
        exec_mod2(e2);
        oper_cmp(expr_make(1, 0), expr_make(2, 0), type);
    }

    // int var with comp var
    if ((e1.type==1) && (e1.id!=0) && (e2.type==3) && (e2.id!=0))
    {
        fprintf(stdout, MSG_WARN_CMP_INT_COMP, line_num+1);

        oper_mult(e1, e1);
        exec_mod2(e2);
        oper_cmp(expr_make(1, 0), expr_make(2, 0), type);
    }

    // int var with comp acc
    if ((e1.type==1) && (e1.id!=0) && (e2.type==3) && (e2.id==0))
    {
        fprintf(stdout, MSG_WARN_CMP_INT_COMP, line_num+1);

        add_instr("SET_P aux_var \n");
        add_instr("SET   aux_var1\n");
        acc_ok = 0;

        oper_mult(e1, e1);
        add_instr("P_LOD aux_var1\n");
        add_instr("P_LOD aux_var \n");
        exec_mod2(expr_make(3, 0));
        oper_cmp(expr_make(1, 0), expr_make(2, 0), type);
    }

    // int acc with int var
    if ((e1.type==1) && (e1.id==0) && (e2.type==1) && (e2.id!=0))
    {
        add_instr("%s %s\n", ld, v_table[e2.id].name);
        add_instr("S_%s\n", op);
    }

    // int acc with int acc
    if ((e1.type==1) && (e1.id==0) && (e2.type==1) && (e2.id==0))
    {
        add_instr("S_%s\n", op);
    }

    // int acc with float var
    if ((e1.type==1) && (e1.id==0) && (e2.type==2) && (e2.id!=0))
    {
        add_instr("I2F\n");
        add_instr("P_LOD %s\n", v_table[e2.id].name);
        
        if (strcmp(op,"EQU")==0) add_instr("S_EQU\n"); else add_instr("SF_%s\n", op);
    }

    // int acc with float acc
    if ((e1.type==1) && (e1.id==0) && (e2.type==2) && (e2.id==0))
    {
        add_instr("SET_P aux_var\n");
        add_instr("I2F\n");
        add_instr("P_LOD aux_var\n");
        
        if (strcmp(op,"EQU")==0) add_instr("S_EQU\n"); else add_instr("SF_%s\n", op);
    }

    // int acc with comp const
    if ((e1.type==1) && (e1.id==0) && (e2.type==5))
    {
        fprintf(stdout, MSG_WARN_CMP_INT_COMP, line_num+1);

        add_instr("SET   aux_var\n");
        add_instr("P_LOD aux_var\n");
        oper_mult(e1, e1);
        exec_mod2(e2);
        oper_cmp(expr_make(1, 0), expr_make(2, 0), type);
    }

    // int acc with comp var
    if ((e1.type==1) && (e1.id==0) && (e2.type==3) && (e2.id!=0))
    {
        fprintf(stdout, MSG_WARN_CMP_INT_COMP, line_num+1);

        add_instr("SET   aux_var\n");
        add_instr("P_LOD aux_var\n");
        oper_mult(e1, e1);
        exec_mod2(e2);
        oper_cmp(expr_make(1, 0), expr_make(2, 0), type);
    }

    // int acc with comp acc
    if ((e1.type==1) && (e1.id==0) && (e2.type==3) && (e2.id==0))
    {
        fprintf(stdout, MSG_WARN_CMP_INT_COMP, line_num+1);

        add_instr("SET_P aux_var \n");
        add_instr("SET_P aux_var1\n");
        add_instr("SET   aux_var2\n");

        add_instr("P_LOD aux_var2\n");
        oper_mult(e1, e1);
        add_instr("P_LOD aux_var1\n");
        add_instr("P_LOD aux_var \n");
        exec_mod2(expr_make(3, 0));
        oper_cmp(expr_make(1, 0), expr_make(2, 0), type);
    }

    // float var with int var
    if ((e1.type==2) && (e1.id!=0) && (e2.type==1) && (e2.id!=0))
    {
        add_instr("%s %s\n", ld , v_table[e1.id].name);
        add_instr("P_I2F_M %s\n", v_table[e2.id].name);
        
        if (strcmp(op,"EQU")==0) add_instr("S_EQU\n"); else add_instr("SF_%s\n", op);
    }

    // float var with int acc
    if ((e1.type==2) && (e1.id!=0) && (e2.type==1) && (e2.id==0))
    {
        add_instr("I2F\n");
        add_instr("SET   aux_var\n");
        add_instr("LOD %s\n", v_table[e1.id].name);
        add_instr("P_LOD aux_var\n");
        
        if (strcmp(op,"EQU")==0) add_instr("S_EQU\n"); else add_instr("SF_%s\n", op);
    }

    // float var with float var
    if ((e1.type==2) && (e1.id!=0) && (e2.type==2) && (e2.id!=0))
    {
        add_instr("%s %s\n"   , ld, v_table[e1.id].name);
        add_instr("P_LOD %s\n",     v_table[e2.id].name);
        
        if (strcmp(op,"EQU")==0) add_instr("S_EQU\n"); else add_instr("SF_%s\n", op);
    }

    // float var with float acc
    if ((e1.type==2) && (e1.id!=0) && (e2.type==2) && (e2.id==0))
    {
        add_instr("SET   aux_var\n");
        add_instr("LOD %s\n", v_table[e1.id].name);
        add_instr("P_LOD aux_var\n");
        
        if (strcmp(op,"EQU")==0) add_instr("S_EQU\n"); else add_instr("SF_%s\n", op);
    }

    // float var with comp const
    if ((e1.type==2) && (e1.id!=0) && (e2.type==5))
    {
        fprintf(stdout, MSG_WARN_CMP_FLOAT_COMP, line_num+1);

        oper_mult(e1, e1);
        exec_mod2(e2);
        oper_cmp(expr_make(2, 0), expr_make(2, 0), type);
    }

    // float var with comp var
    if ((e1.type==2) && (e1.id!=0) && (e2.type==3) && (e2.id!=0))
    {
        fprintf(stdout, MSG_WARN_CMP_FLOAT_COMP, line_num+1);

        oper_mult(e1, e1);
        exec_mod2(e2);
        oper_cmp(expr_make(2, 0), expr_make(2, 0), type);
    }

    // float var with comp acc
    if ((e1.type==2) && (e1.id!=0) && (e2.type==3) && (e2.id==0))
    {
        fprintf(stdout, MSG_WARN_CMP_INT_COMP, line_num+1);

        add_instr("SET_P aux_var \n");
        add_instr("SET   aux_var1\n");
        acc_ok = 0;

        oper_mult(e1, e1);
        add_instr("P_LOD aux_var1\n");
        add_instr("P_LOD aux_var \n");
        exec_mod2(expr_make(3, 0));
        oper_cmp(expr_make(2, 0), expr_make(2, 0), type);
    }

    // float acc with int var
    if ((e1.type==2) && (e1.id==0) && (e2.type==1) && (e2.id!=0))
    {
        add_instr("P_I2F_M %s\n", v_table[e2.id].name);
        
        if (strcmp(op,"EQU")==0) add_instr("S_EQU\n"); else add_instr("SF_%s\n", op);
    }

    // float acc with int acc
    if ((e1.type==2) && (e1.id==0) && (e2.type==1) && (e2.id==0))
    {
        add_instr("I2F\n");
        
        if (strcmp(op,"EQU")==0) add_instr("S_EQU\n"); else add_instr("SF_%s\n", op);
    }

    // float acc with float var
    if ((e1.type==2) && (e1.id==0) && (e2.type==2) && (e2.id!=0))
    {
        add_instr("P_LOD %s\n", v_table[e2.id].name);
        
        if (strcmp(op,"EQU")==0) add_instr("S_EQU\n"); else add_instr("SF_%s\n", op);
    }

    // float acc with float acc
    if ((e1.type==2) && (e1.id==0) && (e2.type==2) && (e2.id==0))
    {
        if (strcmp(op,"EQU")==0) add_instr("S_EQU\n"); else add_instr("SF_%s\n", op);
    }

    // float acc with comp const
    if ((e1.type==2) && (e1.id==0) && (e2.type==5))
    {
        fprintf(stdout, MSG_WARN_CMP_FLOAT_COMP, line_num+1);

        add_instr("SET   aux_var\n");
        add_instr("P_LOD aux_var\n");
        oper_mult(e1, e1);
        exec_mod2(e2);
        oper_cmp(expr_make(2, 0), expr_make(2, 0), type);
    }

    // float acc with comp var
    if ((e1.type==2) && (e1.id==0) && (e2.type==3) && (e2.id!=0))
    {
        fprintf(stdout, MSG_WARN_CMP_FLOAT_COMP, line_num+1);

        add_instr("SET   aux_var\n");
        add_instr("P_LOD aux_var\n");
        oper_mult(e1, e1);
        exec_mod2(e2);
        oper_cmp(expr_make(2, 0), expr_make(2, 0), type);
    }

    // float acc with comp acc
    if ((e1.type==2) && (e1.id==0) && (e2.type==3) && (e2.id==0))
    {
        fprintf(stdout, MSG_WARN_CMP_FLOAT_COMP, line_num+1);

        add_instr("SET_P aux_var \n");
        add_instr("SET_P aux_var1\n");
        add_instr("SET   aux_var2\n");

        add_instr("P_LOD aux_var2\n");
        oper_mult(e1, e1);
        add_instr("P_LOD aux_var1\n");
        add_instr("P_LOD aux_var \n");
        exec_mod2(expr_make(3, 0));
        oper_cmp(expr_make(2, 0), expr_make(2, 0), type);
    }

    // comp const with int var
    if ((e1.type==5) && (e2.type==1) && (e2.id!=0))
    {
        fprintf(stdout, MSG_WARN_CMP_INT_COMP, line_num+1);

        exec_mod2(e1);
        oper_mult(e2, e2);
        oper_cmp(expr_make(2, 0), expr_make(1, 0), type);
    }

    // comp const with int acc
    if ((e1.type==5) && (e2.type==1) && (e2.id==0))
    {
        fprintf(stdout, MSG_WARN_CMP_INT_COMP, line_num+1);

        add_instr("SET   aux_var\n");
        acc_ok = 0;
        exec_mod2(e1);
        add_instr("P_LOD aux_var\n");
        add_instr("P_LOD aux_var\n");
        oper_mult(e2, e2);
        oper_cmp(expr_make(2, 0), expr_make(1, 0), type);
    }

    // comp const with float var
    if ((e1.type==5) && (e2.type==2) && (e2.id!=0))
    {
        fprintf(stdout, MSG_WARN_CMP_FLOAT_COMP, line_num+1);

        exec_mod2(e1);
        oper_mult(e2, e2);
        oper_cmp(expr_make(2, 0), expr_make(2, 0), type);
    }

    // comp const with float acc
    if ((e1.type==5) && (e2.type==2) && (e2.id==0))
    {
        fprintf(stdout, MSG_WARN_CMP_FLOAT_COMP, line_num+1);

        add_instr("SET   aux_var\n");
        acc_ok = 0;
        exec_mod2(e1);
        add_instr("P_LOD aux_var\n");
        add_instr("P_LOD aux_var\n");
        oper_mult(e2, e2);
        oper_cmp(expr_make(2, 0), expr_make(2, 0), type);
    }

    // comp const with comp const
    if ((e1.type==5) && (e2.type==5))
    {
        fprintf(stdout, MSG_WARN_CMP_COMPLEX, line_num+1);

        exec_mod2(e1);
        exec_mod2(e2);
        oper_cmp(expr_make(2, 0), expr_make(2, 0), type);
    }

    // comp const with comp var
    if ((e1.type==5) && (e2.type==3) && (e2.id!=0))
    {
        fprintf(stdout, MSG_WARN_CMP_COMPLEX, line_num+1);

        exec_mod2(e1);
        exec_mod2(e2);
        oper_cmp(expr_make(2, 0), expr_make(2, 0), type);
    }

    // comp const with comp acc
    if ((e1.type==5) && (e2.type==3) && (e2.id==0))
    {
        fprintf(stdout, MSG_WARN_CMP_COMPLEX, line_num+1);

        add_instr("SET_P aux_var \n");
        add_instr("SET   aux_var1\n");
        acc_ok = 0;
        exec_mod2(e1);
        add_instr("P_LOD aux_var1\n");
        add_instr("P_LOD aux_var \n");
        exec_mod2(e2);
        oper_cmp(expr_make(2, 0), expr_make(2, 0), type);
    }

    // comp var with int var
    if ((e1.type==3) && (e1.id!=0) && (e2.type==1) && (e2.id!=0))
    {
        fprintf(stdout, MSG_WARN_CMP_INT_COMP, line_num+1);

        exec_mod2(e1);
        oper_mult(e2, e2);
        oper_cmp(expr_make(2, 0), expr_make(1, 0), type);
    }

    // comp var with int acc
    if ((e1.type==3) && (e1.id!=0) && (e2.type==1) && (e2.id==0))
    {
        fprintf(stdout, MSG_WARN_CMP_INT_COMP, line_num+1);

        add_instr("SET   aux_var\n");
        acc_ok = 0;
        exec_mod2(e1);
        add_instr("P_LOD aux_var\n");
        add_instr("P_LOD aux_var\n");
        oper_mult(e2, e2);
        oper_cmp(expr_make(2, 0), expr_make(1, 0), type);
    }

    // comp var with float var
    if ((e1.type==3) && (e1.id!=0) && (e2.type==2) && (e2.id!=0))
    {
        fprintf(stdout, MSG_WARN_CMP_FLOAT_COMP, line_num+1);

        exec_mod2(e1);
        oper_mult(e2, e2);
        oper_cmp(expr_make(2, 0), expr_make(2, 0), type);
    }

    // comp var with float acc
    if ((e1.type==3) && (e1.id!=0) && (e2.type==2) && (e2.id==0))
    {
        fprintf(stdout, MSG_WARN_CMP_FLOAT_COMP, line_num+1);

        add_instr("SET   aux_var\n");
        acc_ok = 0;
        exec_mod2(e1);
        add_instr("P_LOD aux_var\n");
        add_instr("P_LOD aux_var\n");
        oper_mult(e2, e2);
        oper_cmp(expr_make(2, 0), expr_make(2, 0), type);
    }

    // comp var with comp const
    if ((e1.type==3) && (e1.id!=0) && (e2.type==5))
    {
        fprintf(stdout, MSG_WARN_CMP_COMPLEX, line_num+1);

        exec_mod2(e1);
        exec_mod2(e2);
        oper_cmp(expr_make(2, 0), expr_make(2, 0), type);
    }

    // comp var with comp var
    if ((e1.type==3) && (e1.id!=0) && (e2.type==3) && (e2.id!=0))
    {
        fprintf(stdout, MSG_WARN_CMP_COMPLEX, line_num+1);

        exec_mod2(e1);
        exec_mod2(e2);
        oper_cmp(expr_make(2, 0), expr_make(2, 0), type);
    }

    // comp var with comp acc
    if ((e1.type==3) && (e1.id!=0) && (e2.type==3) && (e2.id==0))
    {
        fprintf(stdout, MSG_WARN_CMP_COMPLEX, line_num+1);

        add_instr("SET_P aux_var \n");
        add_instr("SET   aux_var1\n");
        acc_ok = 0;
        exec_mod2(e1);
        add_instr("P_LOD aux_var1\n");
        add_instr("P_LOD aux_var \n");
        exec_mod2(e2);
        oper_cmp(expr_make(2, 0), expr_make(2, 0), type);
    }

    // comp acc with int var
    if ((e1.type==3) && (e1.id==0) && (e2.type==1) && (e2.id!=0))
    {
        fprintf(stdout, MSG_WARN_CMP_COMPLEX, line_num+1);

        exec_mod2(e1);
        oper_mult(e2, e2);
        oper_cmp(expr_make(2, 0), expr_make(1, 0), type);
    }

    // comp acc with int acc
    if ((e1.type==3) && (e1.id==0) && (e2.type==1) && (e2.id==0))
    {
        fprintf(stdout, MSG_WARN_CMP_COMPLEX, line_num+1);

        add_instr("SET_P aux_var\n");
        exec_mod2(e1);
        add_instr("P_LOD aux_var\n");
        add_instr("P_LOD aux_var\n");
        oper_mult(e2, e2);
        oper_cmp(expr_make(2, 0), expr_make(1, 0), type);
    }

    // comp acc with float var
    if ((e1.type==3) && (e1.id==0) && (e2.type==2) && (e2.id!=0))
    {
        fprintf(stdout, MSG_WARN_CMP_COMPLEX, line_num+1);

        exec_mod2(e1);
        oper_mult(e2, e2);
        oper_cmp(expr_make(2, 0), expr_make(2, 0), type);
    }

    // comp acc with float acc
    if ((e1.type==3) && (e1.id==0) && (e2.type==2) && (e2.id==0))
    {
        fprintf(stdout, MSG_WARN_CMP_COMPLEX, line_num+1);

        add_instr("SET_P aux_var\n");
        exec_mod2(e1);
        add_instr("P_LOD aux_var\n");
        add_instr("P_LOD aux_var\n");
        oper_mult(e2, e2);
        oper_cmp(expr_make(2, 0), expr_make(2, 0), type);
    }

    // comp acc with comp const
    if ((e1.type==3) && (e1.id==0) && (e2.type==5))
    {
        fprintf(stdout, MSG_WARN_CMP_COMPLEX, line_num+1);

        exec_mod2(e1);
        exec_mod2(e2);
        oper_cmp(expr_make(2, 0), expr_make(2, 0), type);
    }

    // comp acc with comp var
    if ((e1.type==3) && (e1.id==0) && (e2.type==3) && (e2.id!=0))
    {
        fprintf(stdout, MSG_WARN_CMP_COMPLEX, line_num+1);

        exec_mod2(e1);
        exec_mod2(e2);
        oper_cmp(expr_make(2, 0), expr_make(2, 0), type);
    }
    
    // comp acc with comp acc
    if ((e1.type==3) && (e1.id==0) && (e2.type==3) && (e2.id==0))
    {
        fprintf(stdout, MSG_WARN_CMP_COMPLEX, line_num+1);

        add_instr("SET_P aux_var \n");
        add_instr("SET_P aux_var1\n");
        exec_mod2(e1);
        add_instr("P_LOD aux_var1\n");
        add_instr("P_LOD aux_var \n");
        exec_mod2(e2);
        oper_cmp(expr_make(2, 0), expr_make(2, 0), type);
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
    expr etr, eti;

    char ld [10]; if (acc_ok == 0) strcpy(ld ,"LOD"  ); else strcpy(ld ,"P_LOD"  );
    char f2i[10]; if (acc_ok == 0) strcpy(f2i,"F2I_M"); else strcpy(f2i,"P_F2I_M");
    char lin[10]; if (acc_ok == 0) strcpy(lin,"LIN_M"); else strcpy(lin,"P_LIN_M");

    // when it is an int in memory
    if ((e.type == 1) && (e.id != 0))
    {
        add_instr("%s %s\n", lin, v_table[e.id].name);
    }

    // when it is an int in the acc
    if ((e.type == 1) && (e.id == 0))
    {
        add_instr("LIN\n");
    }

    // when it is a float variable in memory
    if ((e.type == 2) && (e.id != 0))
    {
        fprintf(stdout, MSG_WARN_LOGIC_FLOAT, line_num+1);

        add_instr("%s %s\n", f2i, v_table[e.id].name);
        add_instr("LIN\n");
    }

    // when it is a float in the acc
    if ((e.type == 2) && (e.id == 0))
    {
        fprintf(stdout, MSG_WARN_LOGIC_FLOAT, line_num+1);

        add_instr("F2I\n");
        add_instr("LIN\n");
    }

    // when it is a comp const
    if (e.type == 5)
    {
        fprintf(stdout, MSG_WARN_LOGIC_COMP, line_num+1);

        get_cmp_cst(e,&etr,&eti);

        add_instr("%s %s\n", f2i, v_table[etr.id].name);
        add_instr("LIN\n");
    }

    // when it is a comp variable
    if ((e.type == 3) && (e.id != 0))
    {
        fprintf(stdout, MSG_WARN_LOGIC_COMP, line_num+1);

        add_instr("%s %s\n", f2i, v_table[e.id].name);
        add_instr("LIN\n");
    }

    // when it is a comp in the acc
    if ((e.type == 3) && (e.id == 0))
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

    if ((e1.type > 1) || (e2.type > 1))
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
    if ((e1.type==1) && (e1.id!=0) && (e2.type==1) && (e2.id!=0))
    {
        add_instr("%s %s\n", ld, v_table[e2.id].name);
        add_instr("%s %s\n", op, v_table[e1.id].name);
    }

    // int var with int acc
    if ((e1.type==1) && (e1.id!=0) && (e2.type==1) && (e2.id==0))
    {
        add_instr("%s %s\n", op, v_table[e1.id].name);
    }

    // int acc with int var
    if ((e1.type==1) && (e1.id==0) && (e2.type==1) && (e2.id!=0))
    {
        add_instr("%s %s\n", op, v_table[e2.id].name);
    }

    // int acc with int acc
    if ((e1.type==1) && (e1.id==0) && (e2.type==1) && (e2.id==0))
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

    if (e.type > 1)
        {fprintf(stderr, MSG_ERR_INV_NON_INT, line_num+1); exit(EXIT_FAILURE);}

    char inv[10]; if (acc_ok == 0) strcpy(inv,"INV_M"); else strcpy(inv,"P_INV_M");

    // when it is an int in memory
    if ((e.type == 1) && (e.id != 0))
    {
        add_instr("%s %s\n", inv, v_table[e.id].name);
    }

    // when it is an int in the acc
    if ((e.type == 1) && (e.id == 0))
    {
        add_instr("INV\n");
    }

    acc_ok = 1;

    return expr_make(1, 0); // int in the accumulator
}

// two-input logical gates (& | ^)
expr oper_bitw(expr e1, expr e2, int type)
{

    if (e1.type > 2 || e2.type > 2)
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
    if ((e1.id != 0) && (e2.id != 0))
    {
        add_instr("%s %s\n", ld, v_table[e2.id].name);
        add_instr("%s %s\n", op, v_table[e1.id].name);
    }

    // var with acc
    if ((e1.id != 0) && (e2.id == 0))
    {
        add_instr("%s %s\n", op, v_table[e1.id].name);
    }

    // acc with var
    if ((e1.id == 0) && (e2.id != 0))
    {
        add_instr("P_LOD %s\n", v_table[e2.id].name);
        add_instr("S_%s\n", op);
    }

    // acc with acc
    if ((e1.id == 0) && (e2.id == 0))
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

    if (e1.type > 2)
        {fprintf(stderr, MSG_ERR_SHIFT_COMPLEX, line_num+1); exit(EXIT_FAILURE);}

    if (e2.type > 2)
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
    if ((e1.id != 0) && (e2.type == 1) && (e2.id != 0))
    {
        add_instr("%s %s\n", ld, v_table[e2.id].name);
        add_instr("%s %s\n", op, v_table[e1.id].name);
    }

    // int/float var with int acc
    if ((e1.id != 0) && (e2.type == 1) && (e2.id == 0))
    {
        add_instr("%s %s\n", op, v_table[e1.id].name);
    }

    // int/float var with float var
    if ((e1.id != 0) && (e2.type == 2) && (e2.id != 0))
    {
        fprintf(stdout, MSG_WARN_SHIFT_BY_FLOAT, line_num+1);

        add_instr("%s %s\n", ld, v_table[e2.id].name);
        add_instr("F2I\n");
        add_instr("%s %s\n", op, v_table[e1.id].name);
    }

    // int/float var with float acc
    if ((e1.id != 0) && (e2.type == 2) && (e2.id == 0))
    {
        fprintf(stdout, MSG_WARN_SHIFT_BY_FLOAT, line_num+1);

        add_instr("F2I\n");
        add_instr("%s %s\n", op, v_table[e1.id].name);
    }

    // int/float acc with int var
    if ((e1.id == 0) && (e2.type == 1) && (e2.id != 0))
    {
        add_instr("P_LOD %s\n", v_table[e2.id].name);
        add_instr("S_%s\n", op);
    }

    // int/float acc with int acc
    if ((e1.id == 0) && (e2.type == 1) && (e2.id == 0))
    {
        add_instr("S_%s\n", op);
    }

    // int/float acc with float var
    if ((e1.id == 0) && (e2.type == 2) && (e2.id != 0))
    {
        fprintf(stdout, MSG_WARN_SHIFT_BY_FLOAT, line_num+1);

        add_instr("P_LOD %s\n", v_table[e2.id].name);
        add_instr("F2I\n");
        add_instr("S_%s\n", op);
    }

    // int/float acc with float acc
    if ((e1.id == 0) && (e2.type == 2) && (e2.id == 0))
    {
        fprintf(stdout, MSG_WARN_SHIFT_BY_FLOAT, line_num+1);
        
        add_instr("F2I\n");
        add_instr("S_%s\n", op);
    }

    acc_ok = 1;

    return expr_make(1, 0); // int in the accumulator
}