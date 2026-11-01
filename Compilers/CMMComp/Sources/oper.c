// ----------------------------------------------------------------------------
// ALU operations -------------------------------------------------------------
// ----------------------------------------------------------------------------

// global includes
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include <stdio.h>

// local includes
#include "../Headers/t2t.h"
#include "../Headers/oper.h"
#include "../Headers/stdlib.h"
#include "../Headers/global.h"
#include "../Headers/data_use.h"
#include "../Headers/diretivas.h"
#include "../Headers/variaveis.h"
#include "../Headers/messages.h"

// an int literal a float meets: the id of its float literal, which a memory
// form takes as it is (exact below 2^NBMANT, as add_instr's I2F_M rewrite); 0
// for any other operand
static int int_flit_id(expr e)
{
    if (e.type != 1 || e.id == 0 || v_table[e.id].isco != 1) return 0;
    long n; char end[8];
    if (sscanf(v_table[e.id].name, "%ld%7s", &n, end) != 1 || labs(n) >= (1L << nbmant)) return 0;
    char t[32]; snprintf(t, sizeof t, "%ld.0", n);
    return exec_fnum(t);
}

// an int literal a float or a comp meets is taken as that float literal, so the
// float forms load it or take it from memory and nothing is converted
static void int_lit_float(expr *e1, expr *e2)
{
    int id;
    if (e2->type >= 2 && (id = int_flit_id(*e1))) *e1 = expr_make(2, id);
    if (e1->type >= 2 && (id = int_flit_id(*e2))) *e2 = expr_make(2, id);

    // a comp constant whose imaginary part is zero is a float, its real part
    for (expr *e = e1; e; e = (e == e1) ? e2 : NULL)
        if (e->type == 5)
        {
            expr er, ei; get_cmp_cst(*e, &er, &ei);
            if (atof(v_table[ei.id].name) == 0.0) *e = num2exp(er.id, 2);
        }
}

// ----------------------------------------------------------------------------
// arithmetic operations ------------------------------------------------------
// ----------------------------------------------------------------------------

// negates a number
expr oper_neg(expr e)
{
    expr   etr, eti;

    char  neg[10]; if (acc_ok == 0) strcpy( neg,   "NEG_M"); else strcpy( neg,  "P_NEG_M");
    char fneg[10]; if (acc_ok == 0) strcpy(fneg, "F_NEG_M"); else strcpy(fneg, "PF_NEG_M");

    // a float or comp literal needs no instruction: its negation is another
    // literal (-2.5 is loaded as such, x - (3+4i) adds -3-4i). Not an int one: the
    // compiler holds int literals non-negative (exec_inum, exec_pow's exponent
    // loop). Not when a float part is zero either: "-0.0" encodes as +0, and
    // F_NEG_M 0.0 gives -0 (TODO.md item 4)
    if (e.type == 2 && e.id != 0 && v_table[e.id].isco)
    {
        char *nm = v_table[e.id].name, txt[sizeof(v_table[0].name) + 2];
        if (atof(nm) != 0.0)
        {
            if (nm[0] == '-') snprintf(txt, sizeof txt, "%s" , nm + 1);
            else              snprintf(txt, sizeof txt, "-%s", nm    );
            return num2exp(exec_fnum(txt), 2);
        }
    }
    if (e.type == 5)
    {
        char re[sizeof(v_table[0].name)], im[sizeof(v_table[0].name)], txt[2*sizeof(v_table[0].name) + 4];
        cnum_split(v_table[e.id].name, re, im);
        if (atof(re) != 0.0 && atof(im) != 0.0)
        {   // each part's sign flipped on the text, as for a float above
            snprintf(txt, sizeof txt, "%s%s%s%si", re[0] == '-' ? "" : "-", re + (re[0] == '-'),
                                                   im[0] == '-' ? "+" : "-", im + (im[0] == '-'));
            return num2exp(exec_cnum(txt), 5);
        }
    }

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
        add_instr("   SET_P __aux_var\n");
        add_instr(" F_NEG\n"          );
        add_instr("PF_NEG_M __aux_var\n");
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

    int_lit_float(&e1, &e2);

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
        add_instr("SET __aux_var\n");                 // temporarily save the imaginary part
        add_instr("I2F_M %s\n", v_table[e1.id].name);  // fetch the int while converting to float
        add_instr("SF_ADD\n");                      // add acc with stack
        add_instr("P_LOD __aux_var\n");               // push the result onto the stack while bringing the imag back
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
        add_instr("SET_P __aux_var\n"); // temporarily save the float, pushing the int to the accumulator
        add_instr("I2F\n");           // converte o int do acumulador pra float
        add_instr("F_ADD __aux_var\n"); // add with the saved float
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
        add_instr("SET_P __aux_var\n" ); // temporarily save the imaginary part, popping the real part
        add_instr("SET_P __aux_var1\n"); // temporarily save the real part, popping the int
        add_instr("I2F\n");            // converte o int do acumulador pra float
        add_instr("F_ADD __aux_var1\n"); // add with the saved real part
        add_instr("P_LOD __aux_var\n" ); // push the imaginary part back to the accumulator
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
        add_instr("SET_P __aux_var\n");
        add_instr("F_ADD %s\n", v_table[e1.id].name);
        add_instr("P_LOD __aux_var\n");
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
        add_instr("SET_P __aux_var\n");
        add_instr("SF_ADD\n");
        add_instr("P_LOD __aux_var\n");
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

        add_instr("SET_P __aux_var\n");
        add_instr("F_ADD %s\n", v_table[etr.id].name);

        add_instr("P_LOD %s\n", v_table[eti.id].name);
        add_instr("F_ADD __aux_var\n");
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

    // comp var with comp var: the walker computes it part by part (ast.c,
    // comp_partwise), so it does not reach here

    // comp var with comp acc
    if ((e1.type==3) && (e1.id!=0) && (e2.type==3) && (e2.id==0))
    {
        get_cmp_ets(e1,&etr,&eti);

        add_instr("SET_P __aux_var\n");
        add_instr("F_ADD %s\n", v_table[etr.id].name);

        add_instr("P_LOD %s\n", v_table[eti.id].name);
        add_instr("F_ADD __aux_var\n");
    }

    // comp acc with int var
    if ((e1.type==3) && (e1.id==0) && (e2.type==1) && (e2.id!=0))
    {
        add_instr("SET_P __aux_var\n");
        add_instr("P_I2F_M %s\n", v_table[e2.id].name);
        add_instr("SF_ADD\n");
        add_instr("P_LOD __aux_var\n");
    }

    // comp acc with int acc
    if ((e1.type==3) && (e1.id==0) && (e2.type==1) && (e2.id==0))
    {
        add_instr("SET_P   __aux_var\n" );
        add_instr("SET_P   __aux_var1\n");
        add_instr("P_I2F_M __aux_var\n" );
        add_instr("SF_ADD\n");
        add_instr("P_LOD   __aux_var1\n");
    }

    // comp acc with float var
    if ((e1.type==3) && (e1.id==0) && (e2.type==2) && (e2.id!=0))
    {
        add_instr("SET_P __aux_var\n");
        add_instr("F_ADD %s\n", v_table[e2.id].name);
        add_instr("P_LOD __aux_var\n");
    }

    // comp acc with float acc
    if ((e1.type==3) && (e1.id==0) && (e2.type==2) && (e2.id==0))
    {
        add_instr("SET_P __aux_var\n" );
        add_instr("SET_P __aux_var1\n");
        add_instr("F_ADD __aux_var\n" );
        add_instr("P_LOD __aux_var1\n");
    }

    // comp acc with comp const
    if ((e1.type==3) && (e1.id==0) && (e2.type==5))
    {
        get_cmp_cst(e2,&etr,&eti);

        add_instr("SET_P __aux_var\n");
        add_instr("F_ADD %s\n", v_table[etr.id].name);

        add_instr("P_LOD __aux_var\n");
        add_instr("F_ADD %s\n", v_table[eti.id].name);
    }

    // comp acc with comp var
    if ((e1.type==3) && (e1.id==0) && (e2.type==3) && (e2.id!=0))
    {
        get_cmp_ets(e2,&etr,&eti);

        add_instr("SET_P __aux_var\n");
        add_instr("F_ADD %s\n", v_table[etr.id].name);

        add_instr("P_LOD __aux_var\n");
        add_instr("F_ADD %s\n", v_table[eti.id].name);
    }

    // comp acc with comp acc
    if ((e1.type==3) && (e1.id==0) && (e2.type==3) && (e2.id==0))
    {
        add_instr("SET_P __aux_var \n");
        add_instr("SET_P __aux_var1\n");
        add_instr("SET_P __aux_var2\n");
        add_instr("F_ADD __aux_var1\n");

        add_instr("P_LOD __aux_var2\n");
        add_instr("F_ADD __aux_var \n");
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

    int_lit_float(&e1, &e2);

    // minus a comp constant is plus its negation, part by part. Unlike
    // oper_neg, a zero part is fine here: it stays 0.0, x - 0.0 and x + 0.0
    // are the same value, and add_instr drops the F_ADD 0.0
    if (e2.type == 5)
    {
        char re[sizeof(v_table[0].name)], im[sizeof(v_table[0].name)], txt[2*sizeof(v_table[0].name) + 8];
        cnum_split(v_table[e2.id].name, re, im);
        int zr = atof(re) == 0.0, zi = atof(im) == 0.0;
        snprintf(txt, sizeof txt, "%s%s%s%si",
                 zr ? "" : (re[0] == '-' ? "" : "-"), zr ? "0.0" : re + (re[0] == '-'),
                 zi ? "+" : (im[0] == '-' ? "+" : "-"), zi ? "0.0" : im + (im[0] == '-'));
        return oper_soma(e1, num2exp(exec_cnum(txt), 5));
    }

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
        add_instr("SET __aux_var\n");                 // temporarily save the imaginary part
        add_instr("I2F_M %s\n", v_table[e1.id].name);  // fetch the int while converting to float
        add_instr("SF_SU1\n");                      // add acc with stack
        add_instr("PF_NEG_M __aux_var\n");            // push the result onto the stack while bringing the imag back
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
        add_instr("SET_P __aux_var\n");
        add_instr("I2F\n");
        add_instr("F_SU1 __aux_var\n");
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
        add_instr("SET_P    __aux_var\n" );
        add_instr("SET_P    __aux_var1\n");
        add_instr("I2F\n");
        add_instr("F_SU1    __aux_var1\n");
        add_instr("PF_NEG_M __aux_var\n" );
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
        add_instr("SET_P __aux_var\n");
        add_instr("F_SU2 %s\n", v_table[e1.id].name);
        add_instr("PF_NEG_M __aux_var\n");
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
        add_instr("SET_P __aux_var\n");
        add_instr("SF_SU2\n");
        add_instr("PF_NEG_M __aux_var\n");
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

        add_instr("SET_P __aux_var\n");
        add_instr("F_SU2 %s\n", v_table[etr.id].name);

        add_instr("P_LOD %s\n", v_table[eti.id].name);
        add_instr("F_SU1 __aux_var\n");
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

    // comp var with comp var: the walker computes it part by part (ast.c,
    // comp_partwise), so it does not reach here

    // comp var with comp acc
    if ((e1.type==3) && (e1.id!=0) && (e2.type==3) && (e2.id==0))
    {
        get_cmp_ets(e1,&etr,&eti);

        add_instr("SET_P __aux_var\n");
        add_instr("F_SU2 %s\n", v_table[etr.id].name);

        add_instr("P_LOD %s\n", v_table[eti.id].name);
        add_instr("F_SU1 __aux_var\n");
    }

    // comp acc with int var
    if ((e1.type==3) && (e1.id==0) && (e2.type==1) && (e2.id!=0))
    {
        add_instr("SET_P __aux_var\n");
        add_instr("P_I2F_M %s\n", v_table[e2.id].name);
        add_instr("SF_SU2\n");
        add_instr("P_LOD __aux_var\n");
    }

    // comp acc with int acc
    if ((e1.type==3) && (e1.id==0) && (e2.type==1) && (e2.id==0))
    {
        add_instr("SET_P   __aux_var\n" );
        add_instr("SET_P   __aux_var1\n");
        add_instr("P_I2F_M __aux_var\n" );
        add_instr("SF_SU2\n");
        add_instr("P_LOD   __aux_var1\n");
    }

    // comp acc with float var
    if ((e1.type==3) && (e1.id==0) && (e2.type==2) && (e2.id!=0))
    {
        add_instr("SET_P __aux_var\n");
        add_instr("F_SU1 %s\n", v_table[e2.id].name);
        add_instr("P_LOD __aux_var\n");
    }

    // comp acc with float acc
    if ((e1.type==3) && (e1.id==0) && (e2.type==2) && (e2.id==0))
    {
        add_instr("SET_P __aux_var\n" );
        add_instr("SET_P __aux_var1\n");
        add_instr("F_SU1 __aux_var\n" );
        add_instr("P_LOD __aux_var1\n");
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

        add_instr("SET_P __aux_var\n");
        add_instr("F_SU1 %s\n", v_table[etr.id].name);

        add_instr("P_LOD __aux_var\n");
        add_instr("F_SU1 %s\n", v_table[eti.id].name);
    }

    // comp acc with comp acc
    if ((e1.type==3) && (e1.id==0) && (e2.type==3) && (e2.id==0))
    {
        add_instr("SET_P __aux_var \n");
        add_instr("SET_P __aux_var1\n");
        add_instr("SET_P __aux_var2\n");
        add_instr("F_SU1 __aux_var1\n");

        add_instr("P_LOD __aux_var2\n");
        add_instr("F_SU1 __aux_var \n");
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

    int_lit_float(&e1, &e2);

    // a comp times a pure imaginary constant ci: (a+bi)(ci) = -bc + aci, two
    // products and no sum (the multiplication is commutative: the constant goes
    // right)
    if (e1.type == 5 && e2.type == 3) { expr t = e1; e1 = e2; e2 = t; }
    if (e1.type == 3 && e2.type == 5)
    {
        expr er, ei; get_cmp_cst(e2, &er, &ei);
        char *c = v_table[ei.id].name, nt[sizeof(v_table[0].name) + 2];
        if (atof(v_table[er.id].name) == 0.0 && atof(c) != 0.0)
        {
            snprintf(nt, sizeof nt, c[0] == '-' ? "%s" : "-%s", c[0] == '-' ? c + 1 : c);
            int nid = exec_fnum(nt);                                   // -c
            if (e1.id != 0)                                            // in memory: 4 words
            {
                get_cmp_ets(e1, &etr, &eti);
                add_instr("%s %s\n", acc_ok ? "P_LOD" : "LOD", v_table[eti.id].name);
                add_instr("F_MLT %s\n", v_table[nid].name);            // real: -c b
                add_instr("P_LOD %s\n", v_table[etr.id].name);
                add_instr("F_MLT %s\n", c);                            // imag: c a
            }
            else                                                       // in the acc (real on the stack)
            {
                add_instr("F_MLT %s\n", v_table[nid].name);            // -c b
                add_instr("SET_P __aux_var\n");                        // the real part back in the acc
                add_instr("F_MLT %s\n", c);                            // c a
                add_instr("SET __aux_var1\n");
                add_instr("LOD __aux_var\n");
                add_instr("P_LOD __aux_var1\n");
            }
            acc_ok = 1;
            return expr_make(3, 0);
        }
    }

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
        add_instr("SET_P __aux_var\n");
        add_instr("P_I2F_M %s\n", v_table[e1.id].name);
        add_instr("SF_MLT\n");

        add_instr("P_LOD __aux_var\n");
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
        add_instr("SET_P __aux_var\n");
        add_instr("I2F\n");
        add_instr("F_MLT __aux_var\n");
    }

    // int acc with comp const
    if ((e1.type==1) && (e1.id==0) && (e2.type==5))
    {
        get_cmp_cst(e2,&etr,&eti);

        add_instr("I2F\n");
        add_instr("SET   __aux_var\n");
        add_instr("F_MLT %s\n", v_table[etr.id].name);
        add_instr("P_LOD __aux_var\n");
        add_instr("F_MLT %s\n", v_table[eti.id].name);
    }

    // int acc with comp var
    if ((e1.type==1) && (e1.id==0) && (e2.type==3) && (e2.id!=0))
    {
        get_cmp_ets(e2,&etr,&eti);

        add_instr("I2F\n");
        add_instr("SET   __aux_var\n");
        add_instr("F_MLT %s\n", v_table[etr.id].name);
        add_instr("P_LOD __aux_var\n");
        add_instr("F_MLT %s\n", v_table[eti.id].name);
    }

    // int acc with comp acc
    if ((e1.type==1) && (e1.id==0) && (e2.type==3) && (e2.id==0))
    {
        add_instr("SET_P __aux_var \n");
        add_instr("SET_P __aux_var1\n");
        add_instr("I2F\n");
        add_instr("SET   __aux_var2\n");
        add_instr("F_MLT __aux_var1\n");
        add_instr("P_LOD __aux_var \n");
        add_instr("F_MLT __aux_var2\n");
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
        add_instr("SET_P __aux_var\n");
        add_instr("F_MLT %s\n", v_table[e1.id].name);
        add_instr("P_LOD __aux_var\n");
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

        add_instr("SET   __aux_var\n");
        add_instr("F_MLT %s\n", v_table[etr.id].name);
        add_instr("P_LOD %s\n", v_table[eti.id].name);
        add_instr("F_MLT __aux_var\n");
    }

    // float acc with comp var
    if ((e1.type==2) && (e1.id==0) && (e2.type==3) && (e2.id!=0))
    {
        get_cmp_ets(e2,&etr,&eti);

        add_instr("SET   __aux_var\n");
        add_instr("F_MLT %s\n", v_table[etr.id].name);
        add_instr("P_LOD %s\n", v_table[eti.id].name);
        add_instr("F_MLT __aux_var\n");
    }

    // float acc with comp acc
    if ((e1.type==2) && (e1.id==0) && (e2.type==3) && (e2.id==0))
    {
        add_instr("SET_P __aux_var \n");
        add_instr("SET_P __aux_var1\n");
        add_instr("SET   __aux_var2\n");
        add_instr("F_MLT __aux_var1\n");
        add_instr("P_LOD __aux_var \n");
        add_instr("F_MLT __aux_var2\n");
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
        add_instr("SET   __aux_var\n");
        add_instr("F_MLT %s\n"  , v_table[etr.id].name);
        add_instr("P_LOD __aux_var\n");
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

        add_instr("SET   __aux_var\n");
        add_instr("F_MLT %s\n"  , v_table[etr.id].name);
        add_instr("P_LOD __aux_var\n");
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

    // comp const with a comp var or acc: swapped above, the constant goes right

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
        add_instr("SET   __aux_var\n");
        add_instr("F_MLT %s\n"  , v_table[etr.id].name);
        add_instr("P_LOD __aux_var\n");
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

        add_instr("SET   __aux_var\n");
        add_instr("F_MLT %s\n"  , v_table[etr.id].name);
        add_instr("P_LOD __aux_var\n");
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

    // comp var with comp var: the walker computes it part by part (ast.c,
    // comp_partwise), so it does not reach here

    // comp var with comp acc
    if ((e1.type==3) && (e1.id!=0) && (e2.type==3) && (e2.id==0))
    {
        get_cmp_ets(e1,&etr,&eti);

        add_instr("SET_P __aux_var \n");
        add_instr("SET   __aux_var1\n");

        add_instr("F_MLT %s\n", v_table[etr.id].name);
        add_instr("P_LOD %s\n", v_table[eti.id].name);
        add_instr("F_MLT __aux_var \n");
        add_instr("SF_SU2\n");

        add_instr("P_LOD %s\n" , v_table[etr.id].name);
        add_instr("F_MLT __aux_var \n");
        add_instr("P_LOD %s\n" , v_table[eti.id].name);
        add_instr("F_MLT __aux_var1\n");
        add_instr("SF_ADD\n");
    }

    // comp acc with int var
    if ((e1.type==3) && (e1.id==0) && (e2.type==1) && (e2.id!=0))
    {
        add_instr("SET_P __aux_var\n" );
        add_instr("P_I2F_M %s\n", v_table[e2.id].name);
        add_instr("SET   __aux_var1\n");
        add_instr("SF_MLT\n");
        add_instr("P_LOD __aux_var\n" );
        add_instr("F_MLT __aux_var1\n");
    }

    // comp acc with int acc
    if ((e1.type==3) && (e1.id==0) && (e2.type==1) && (e2.id==0))
    {
        add_instr("I2F\n");
        add_instr("SET_P __aux_var\n" );
        add_instr("SET_P __aux_var1\n");
        add_instr("F_MLT __aux_var\n" );
        add_instr("P_LOD __aux_var1\n");
        add_instr("F_MLT __aux_var\n" );
    }

    // comp acc with float var
    if ((e1.type==3) && (e1.id==0) && (e2.type==2) && (e2.id!=0))
    {
        add_instr("SET_P __aux_var\n");
        add_instr("F_MLT %s\n", v_table[e2.id].name);
        add_instr("P_LOD __aux_var\n");
        add_instr("F_MLT %s\n", v_table[e2.id].name);
    }

    // comp acc with float acc
    if ((e1.type==3) && (e1.id==0) && (e2.type==2) && (e2.id==0))
    {
        add_instr("SET_P __aux_var\n" );
        add_instr("SET_P __aux_var1\n");
        add_instr("F_MLT __aux_var\n" );
        add_instr("P_LOD __aux_var1\n");
        add_instr("F_MLT __aux_var\n");
    }

    // comp acc with comp const
    if ((e1.type==3) && (e1.id==0) && (e2.type==5))
    {
        get_cmp_cst(e2,&etr,&eti);

        add_instr("SET_P __aux_var \n");
        add_instr("SET   __aux_var1\n");
        add_instr("F_MLT %s\n", v_table[etr.id].name);
        add_instr("P_LOD __aux_var \n");
        add_instr("F_MLT %s\n", v_table[eti.id].name);
        add_instr("SF_SU2\n");

        add_instr("P_LOD __aux_var \n");
        add_instr("F_MLT %s\n", v_table[etr.id].name);
        add_instr("P_LOD __aux_var1\n");
        add_instr("F_MLT %s\n", v_table[eti.id].name);
        add_instr("SF_ADD\n");
    }

    // comp acc with comp var
    if ((e1.type==3) && (e1.id==0) && (e2.type==3) && (e2.id!=0))
    {
        get_cmp_ets(e2,&etr,&eti);

        add_instr("SET_P __aux_var \n");
        add_instr("SET   __aux_var1\n");
        add_instr("F_MLT %s\n", v_table[etr.id].name);
        add_instr("P_LOD __aux_var \n");
        add_instr("F_MLT %s\n", v_table[eti.id].name);
        add_instr("SF_SU2\n");

        add_instr("P_LOD __aux_var \n");
        add_instr("F_MLT %s\n", v_table[etr.id].name);
        add_instr("P_LOD __aux_var1\n");
        add_instr("F_MLT %s\n", v_table[eti.id].name);
        add_instr("SF_ADD\n");
    }

    // comp acc with comp acc
    if ((e1.type==3) && (e1.id==0) && (e2.type==3) && (e2.id==0))
    {
        add_instr("SET_P __aux_var \n");
        add_instr("SET_P __aux_var1\n");
        add_instr("SET_P __aux_var2\n");
        add_instr("SET   __aux_var3\n");

        add_instr("F_MLT __aux_var1\n");
        add_instr("P_LOD __aux_var \n");
        add_instr("F_MLT __aux_var2\n");
        add_instr("SF_SU2\n");

        add_instr("P_LOD __aux_var1\n");
        add_instr("F_MLT __aux_var2\n");
        add_instr("P_LOD __aux_var \n");
        add_instr("F_MLT __aux_var3\n");
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

    int_lit_float(&e1, &e2);

    // a division by a float literal is a multiplication by its reciprocal: one
    // word, and no divider. A power of two has an exact reciprocal, so it always
    // is. Any other literal gives the reciprocal's rounding, which the program
    // takes only to keep the float divider out (Luciano, 2026-10-31, issue
    // #69): both forms are emitted, the division between #DIVK and #MULK, the
    // multiplication between #MULK and #ENDK, and asm_divk keeps the division
    // when anything else in the program needs the divider, else the product
    static int divk_inner = 0;
    if (!divk_inner && e2.type == 2 && e2.id != 0 && v_table[e2.id].isco == 1)
    {
        double c = strtod(v_table[e2.id].name, NULL), r = 1.0 / c;
        if (c != 0.0 && isfinite(r) && fabs(r) < 1e37 && fabs(r) > 1e-37)
        {
            char t[40]; snprintf(t, sizeof t, "%.9g", r);
            if (!strpbrk(t, ".eE")) strcat(t, ".0");
            fnum_quiet = 1; int rid = exec_fnum(t); fnum_quiet = 0;   // the user did not write it
            int ex; if (fabs(frexp(c, &ex)) == 0.5) return oper_mult(e1, num2exp(rid, 2));

            int acc0 = acc_ok;
            add_sinst(0, "#DIVK\n");
            divk_inner = 1; oper_divi(e1, e2); divk_inner = 0;
            add_sinst(0, "#MULK\n");
            acc_ok = acc0;
            expr m = oper_mult(e1, num2exp(rid, 2));
            add_sinst(0, "#ENDK\n");
            return m;
        }
    }

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
        add_instr("SET   __aux_var\n");
        add_instr("I2F_M %s\n", v_table[e1.id].name);
        add_instr("P_LOD __aux_var\n");
        add_instr("SF_DIV\n");
    }

    // int var with comp const
    if ((e1.type==1) && (e1.id!=0) && (e2.type==5))
    {
        get_cmp_cst(e2,&etr,&eti);

        emit_sq_sum(etr, eti);
        add_instr("SET __aux_var\n");   // save the result

        acc_ok = 0;                   // libera acumulador

        oper_mult(e1, etr);           // multiply int with real part
        add_instr("P_LOD __aux_var\n"); // fetch the denominator
        oper_divi(expr_make(2, 0), expr_make(2, 0));     // faz a divisao

        oper_mult(e1, eti);           // multiply int with imag part
        add_instr("P_LOD __aux_var\n"); // fetch the denominator
        oper_divi(expr_make(2, 0), expr_make(2, 0));     // faz a divisao
        oper_neg (expr_make(2, 0));            // nega a parte imaginaria
    }

    // int var with comp var
    if ((e1.type==1) && (e1.id!=0) && (e2.type==3) && (e2.id!=0))
    {
        get_cmp_ets(e2,&etr,&eti);

        emit_sq_sum(etr, eti);
        add_instr("SET __aux_var\n");   // save the result

        acc_ok = 0;                   // libera acumulador

        oper_mult(e1, etr);           // multiply int with real part
        add_instr("P_LOD __aux_var\n"); // fetch the denominator
        oper_divi(expr_make(2, 0), expr_make(2, 0));     // faz a divisao

        oper_mult(e1, eti);           // multiply int with imag part
        add_instr("P_LOD __aux_var\n"); // fetch the denominator
        oper_divi(expr_make(2, 0), expr_make(2, 0));     // faz a divisao
        oper_neg (expr_make(2, 0));            // nega a parte imaginaria
    }

    // int var with comp acc
    if ((e1.type==1) && (e1.id!=0) && (e2.type==3) && (e2.id==0))
    {
        add_instr("SET_P __aux_var \n"); // save the imaginary part
        add_instr("SET   __aux_var1\n"); // save the real part

        add_instr("F_MLT __aux_var1\n"); // parte real ao quadrado
        add_instr("P_LOD __aux_var \n"); // fetch the imaginary part
        add_instr("F_MLT __aux_var \n"); // parte imag ao quadrado
        add_instr("SF_ADD        \n"); // soma os quadrados
        add_instr("SET   __aux_var2\n"); // save the squared magnitude

        add_instr("I2F_M %s\n", v_table[e1.id].name); 
        add_instr("SET   __aux_var3\n"); // save the float
        add_instr("F_MLT __aux_var1\n");
        add_instr("P_LOD __aux_var2\n"); // fetch the squared magnitude
        oper_divi(expr_make(2, 0), expr_make(2, 0));      // faz a divisao

        add_instr("P_LOD __aux_var3\n"); // fetch the float
        add_instr("F_MLT __aux_var \n");
        add_instr("P_LOD __aux_var2\n"); // fetch the squared magnitude
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
        add_instr("SET_P __aux_soma\n");
        add_instr("I2F\n");
        add_instr("P_LOD __aux_soma\n");
        add_instr("SF_DIV\n");
    }

    // int acc with comp const
    if ((e1.type==1) && (e1.id==0) && (e2.type==5))
    {
        get_cmp_cst(e2,&etr,&eti);

        add_instr("I2F\n");
        add_instr("SET __aux_var\n");
        acc_ok = 0;

        emit_sq_sum(etr, eti);
        add_instr("SET   __aux_var1\n"); // save the result

        add_instr("LOD   __aux_var\n");
        oper_mult(expr_make(2, 0), etr);         // multiply float with real part
        add_instr("P_LOD __aux_var1\n"); // fetch the denominator
        oper_divi(expr_make(2, 0), expr_make(2, 0));      // faz a divisao

        add_instr("P_LOD  __aux_var\n");
        oper_mult(expr_make(2, 0), eti);         // multiply float with imag part
        add_instr("P_LOD __aux_var1\n"); // fetch the denominator
        oper_divi(expr_make(2, 0), expr_make(2, 0));      // faz a divisao
        oper_neg (expr_make(2, 0));
    }

    // int acc with comp var
    if ((e1.type==1) && (e1.id==0) && (e2.type==3) && (e2.id!=0))
    {
        get_cmp_ets(e2,&etr,&eti);

        add_instr("I2F\n");
        add_instr("SET   __aux_var\n");
        acc_ok = 0;

        emit_sq_sum(etr, eti);
        add_instr("SET   __aux_var1\n");  // save the result

        add_instr("LOD   __aux_var\n" );
        oper_mult(expr_make(2, 0), etr);          // multiply float with real part
        add_instr("P_LOD __aux_var1\n");  // fetch the denominator
        oper_divi(expr_make(2, 0), expr_make(2, 0));       // faz a divisao

        add_instr("P_LOD  __aux_var\n" );
        oper_mult(expr_make(2, 0), eti);          // multiply float with imag part
        add_instr("P_LOD  __aux_var1\n"); // fetch the denominator
        oper_divi(expr_make(2, 0), expr_make(2, 0));       // faz a divisao
        oper_neg (expr_make(2, 0));
    }

    // int acc with comp acc
    if ((e1.type==1) && (e1.id==0) && (e2.type==3) && (e2.id==0))
    {
        add_instr("SET_P __aux_var \n"); // save the imaginary part
        add_instr("SET_P __aux_var1\n"); // save the real part
        add_instr("I2F           \n");
        add_instr("SET   __aux_var2\n"); // save the float

        add_instr("LOD   __aux_var1\n"); // push the real part onto the stack
        add_instr("F_MLT __aux_var1\n");
        add_instr("P_LOD __aux_var \n"); // fetch the imaginary part
        add_instr("F_MLT __aux_var \n");
        add_instr("SF_ADD        \n");
        add_instr("SET   __aux_var3\n"); // save the squared magnitude

        add_instr("LOD   __aux_var2\n"); // load the float
        add_instr("F_MLT __aux_var1\n");
        add_instr("P_LOD __aux_var3\n"); // fetch the squared magnitude
        oper_divi(expr_make(2, 0), expr_make(2, 0));      // faz a divisao

        add_instr("P_LOD __aux_var2\n"); // fetch the float
        add_instr("F_MLT __aux_var \n");
        add_instr("P_LOD __aux_var3\n"); // fetch the squared magnitude
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
        add_instr("SET   __aux_var\n"); // save the result
        acc_ok = 0;

        oper_mult(e1, etr);           // multiply float with real part
        add_instr("P_LOD __aux_var\n"); // fetch the denominator
        oper_divi(expr_make(2, 0), expr_make(2, 0));     // faz a divisao

        oper_mult(e1, eti);           // multiply float with imag part
        add_instr("P_LOD __aux_var\n"); // fetch the denominator
        oper_divi(expr_make(2, 0), expr_make(2, 0));     // faz a divisao
        oper_neg (expr_make(2, 0));
    }

    // float var with comp var
    if ((e1.type==2) && (e1.id!=0) && (e2.type==3) && (e2.id!=0))
    {
        get_cmp_ets(e2,&etr,&eti);

        emit_sq_sum(etr, eti);
        add_instr("SET   __aux_var\n"); // save the result
        acc_ok = 0;

        oper_mult(e1, etr);           // multiply float with real part
        add_instr("P_LOD __aux_var\n"); // fetch the denominator
        oper_divi(expr_make(2, 0), expr_make(2, 0));     // faz a divisao

        oper_mult(e1, eti);           // multiply float with imag part
        add_instr("P_LOD __aux_var\n"); // fetch the denominator
        oper_divi(expr_make(2, 0), expr_make(2, 0));     // faz a divisao
        oper_neg (expr_make(2, 0));
    }

    // float var with comp acc
    if ((e1.type==2) && (e1.id!=0) && (e2.type==3) && (e2.id==0))
    {
        add_instr("SET_P __aux_var \n");             // save the imaginary part
        add_instr("SET   __aux_var1\n");             // save the real part
        add_instr("F_MLT __aux_var1\n");
        add_instr("P_LOD __aux_var \n");             // fetch the imaginary part
        add_instr("F_MLT __aux_var \n");
        add_instr("SF_ADD        \n");
        add_instr("SET   __aux_var2\n");             // save the squared magnitude

        add_instr("LOD %s\n"  , v_table[e1.id].name); // load the float
        add_instr("F_MLT __aux_var1\n");
        add_instr("P_LOD __aux_var2\n");             // fetch the squared magnitude
        oper_divi(expr_make(2, 0), expr_make(2, 0));                  // faz a divisao

        add_instr("P_LOD %s\n", v_table[e1.id].name); // load the float
        add_instr("F_MLT __aux_var \n");
        add_instr("P_LOD __aux_var2\n");             // fetch the squared magnitude
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

        add_instr("SET   __aux_var\n");
        acc_ok = 0;

        emit_sq_sum(etr, eti);
        add_instr("SET   __aux_var1\n"); // save the result

        add_instr("LOD   __aux_var \n");
        oper_mult(expr_make(2, 0), etr);         // multiply float with real part
        add_instr("P_LOD __aux_var1\n"); // fetch the denominator
        oper_divi(expr_make(2, 0), expr_make(2, 0));      // faz a divisao

        add_instr("P_LOD __aux_var \n");
        oper_mult(expr_make(2, 0), eti);         // multiply float with imag part
        add_instr("P_LOD __aux_var1\n"); // fetch the denominator
        oper_divi(expr_make(2, 0), expr_make(2, 0));      // faz a divisao
        oper_neg (expr_make(2, 0));
    }

    // float acc with comp var
    if ((e1.type==2) && (e1.id==0) && (e2.type==3) && (e2.id!=0))
    {
        get_cmp_ets(e2,&etr,&eti);

        add_instr("SET __aux_var\n");
        acc_ok = 0;

        emit_sq_sum(etr, eti);
        add_instr("SET   __aux_var1\n"); // save the result

        add_instr("LOD   __aux_var\n");
        oper_mult(expr_make(2, 0), etr);         // multiply float with real part
        add_instr("P_LOD __aux_var1\n"); // fetch the denominator
        oper_divi(expr_make(2, 0), expr_make(2, 0));      // faz a divisao

        add_instr("P_LOD __aux_var\n");
        oper_mult(expr_make(2, 0), eti);         // multiply float with imag part
        add_instr("P_LOD __aux_var1\n"); // fetch the denominator
        oper_divi(expr_make(2, 0), expr_make(2, 0));      // faz a divisao
        oper_neg (expr_make(2, 0));
    }

    // float acc with comp acc
    if ((e1.type==2) && (e1.id==0) && (e2.type==3) && (e2.id==0))
    {
        add_instr("SET_P __aux_var \n"); // save the imaginary part
        add_instr("SET_P __aux_var1\n"); // save the real part
        add_instr("SET   __aux_var2\n"); // save the float

        add_instr("LOD   __aux_var1\n"); // push the real part onto the stack
        add_instr("F_MLT __aux_var1\n");
        add_instr("P_LOD __aux_var \n"); // fetch the imaginary part
        add_instr("F_MLT __aux_var \n");
        add_instr("SF_ADD        \n");
        add_instr("SET   __aux_var3\n"); // save the squared magnitude

        add_instr("LOD   __aux_var2\n"); // load the float
        add_instr("F_MLT __aux_var1\n");
        add_instr("P_LOD __aux_var3\n"); // fetch the squared magnitude
        oper_divi(expr_make(2, 0), expr_make(2, 0));      // faz a divisao

        add_instr("P_LOD __aux_var2\n"); // fetch the float
        add_instr("F_MLT __aux_var \n");
        add_instr("P_LOD __aux_var3\n"); // fetch the squared magnitude
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
        add_instr("SET   __aux_var\n");
        oper_divi(etr, expr_make(2, 0));
        add_instr("P_LOD __aux_var\n");
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

        add_instr("SET   __aux_var\n");
        oper_divi(etr, expr_make(2, 0));
        add_instr("P_LOD __aux_var\n");
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
        add_instr("SET   __aux_var\n");
        acc_ok = 0;

        oper_mult(et1r, et2r);
        oper_mult(et1i, et2i);
        oper_soma(expr_make(2, 0), expr_make(2, 0));
        add_instr("P_LOD __aux_var\n");
        oper_divi(expr_make(2, 0), expr_make(2, 0));

        oper_mult(et1i, et2r);
        oper_mult(et1r, et2i);
        oper_subt(expr_make(2, 0), expr_make(2, 0));
        add_instr("P_LOD __aux_var\n");
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
        add_instr("SET __aux_var\n");
        acc_ok = 0;

        oper_mult(et1r, et2r);
        oper_mult(et1i, et2i);
        oper_soma(expr_make(2, 0), expr_make(2, 0));
        add_instr("P_LOD __aux_var\n");
        oper_divi(expr_make(2, 0), expr_make(2, 0));

        oper_mult(et1i, et2r);
        oper_mult(et1r, et2i);
        oper_subt(expr_make(2, 0), expr_make(2, 0));
        add_instr("P_LOD __aux_var\n");
        oper_divi(expr_make(2, 0), expr_make(2, 0));
    }

    // comp const with comp acc
    if ((e1.type==5) && (e2.type==3) && (e2.id==0))
    {
        get_cmp_cst(e1,&etr,&eti);

        add_instr("SET_P __aux_var \n");
        add_instr("SET   __aux_var1\n");
        add_instr("F_MLT __aux_var1\n");
        add_instr("P_LOD __aux_var \n");
        add_instr("F_MLT __aux_var \n");
        oper_soma(expr_make(2, 0), expr_make(2, 0));
        add_instr("SET   __aux_var2\n");

        add_instr("LOD   __aux_var1\n");
        oper_mult(etr, expr_make(2, 0));
        add_instr("P_LOD __aux_var \n");
        oper_mult(eti, expr_make(2, 0));
        oper_soma(expr_make(2, 0), expr_make(2, 0));
        add_instr("P_LOD __aux_var2\n");
        oper_divi(expr_make(2, 0), expr_make(2, 0));

        add_instr("P_LOD __aux_var1\n");
        oper_mult(eti, expr_make(2, 0));
        add_instr("P_LOD __aux_var \n");
        oper_mult(etr, expr_make(2, 0));
        oper_subt(expr_make(2, 0), expr_make(2, 0));
        add_instr("P_LOD __aux_var2\n");
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
        add_instr("SET   __aux_var\n");
        oper_divi(etr, expr_make(2, 0));
        add_instr("P_LOD __aux_var\n");
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

        add_instr("SET   __aux_var\n");
        oper_divi(etr, expr_make(2, 0));
        add_instr("P_LOD __aux_var\n");
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
        add_instr("SET   __aux_var\n");
        acc_ok = 0;

        oper_mult(et1r, et2r);
        oper_mult(et1i, et2i);
        oper_soma(expr_make(2, 0), expr_make(2, 0));
        add_instr("P_LOD __aux_var\n");
        oper_divi(expr_make(2, 0), expr_make(2, 0));

        oper_mult(et1i, et2r);
        oper_mult(et1r, et2i);
        oper_subt(expr_make(2, 0), expr_make(2, 0));
        add_instr("P_LOD __aux_var\n");
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
        add_instr("SET   __aux_var\n");
        acc_ok = 0;

        oper_mult(et1r, et2r);
        oper_mult(et1i, et2i);
        oper_soma(expr_make(2, 0), expr_make(2, 0));
        add_instr("P_LOD __aux_var\n");
        oper_divi(expr_make(2, 0), expr_make(2, 0));

        oper_mult(et1i, et2r);
        oper_mult(et1r, et2i);
        oper_subt(expr_make(2, 0), expr_make(2, 0));
        add_instr("P_LOD __aux_var\n");
        oper_divi(expr_make(2, 0), expr_make(2, 0));
    }

    // comp var with comp acc
    if ((e1.type==3) && (e1.id!=0) && (e2.type==3) && (e2.id==0))
    {
        get_cmp_ets(e1,&etr,&eti);

        add_instr("SET_P __aux_var \n");
        add_instr("SET   __aux_var1\n");
        add_instr("F_MLT __aux_var1\n");
        add_instr("P_LOD __aux_var \n");
        add_instr("F_MLT __aux_var \n");
        oper_soma(expr_make(2, 0), expr_make(2, 0));
        add_instr("SET   __aux_var2\n");

        add_instr("LOD   __aux_var1\n");
        oper_mult(etr, expr_make(2, 0));
        add_instr("P_LOD __aux_var \n");
        oper_mult(eti, expr_make(2, 0));
        oper_soma(expr_make(2, 0), expr_make(2, 0));
        add_instr("P_LOD __aux_var2\n");
        oper_divi(expr_make(2, 0), expr_make(2, 0));

        add_instr("P_LOD __aux_var1\n");
        oper_mult(eti, expr_make(2, 0));
        add_instr("P_LOD __aux_var \n");
        oper_mult(etr, expr_make(2, 0));
        oper_subt(expr_make(2, 0), expr_make(2, 0));
        add_instr("P_LOD __aux_var2\n");
        oper_divi(expr_make(2, 0), expr_make(2, 0));
    }

    // comp acc with int var
    if ((e1.type==3) && (e1.id==0) && (e2.type==1) && (e2.id!=0))
    {
        add_instr("SET_P __aux_var \n");
        add_instr("P_I2F_M %s\n", v_table[e2.id].name);
        add_instr("SET   __aux_var1\n");
        oper_divi(expr_make(2, 0), expr_make(2, 0));
        add_instr("P_LOD __aux_var \n");
        add_instr("P_LOD __aux_var1\n");
        oper_divi(expr_make(2, 0), expr_make(2, 0));
    }

    // comp acc with int acc
    if ((e1.type==3) && (e1.id==0) && (e2.type==1) && (e2.id==0))
    {
        add_instr("I2F\n");
        add_instr("SET_P __aux_var \n");
        add_instr("SET_P __aux_var1\n");
        add_instr("P_LOD __aux_var \n");
        oper_divi(expr_make(2, 0), expr_make(2, 0));
        add_instr("P_LOD __aux_var1\n");
        add_instr("P_LOD __aux_var \n");
        oper_divi(expr_make(2, 0), expr_make(2, 0));
    }

    // comp acc with float var
    if ((e1.type==3) && (e1.id==0) && (e2.type==2) && (e2.id!=0))
    {
        add_instr("SET_P __aux_var\n");
        add_instr("P_LOD %s\n", v_table[e2.id].name);
        oper_divi(expr_make(2, 0), expr_make(2, 0));
        add_instr("P_LOD __aux_var\n");
        add_instr("P_LOD %s\n", v_table[e2.id].name);
        oper_divi(expr_make(2, 0), expr_make(2, 0));
    }

    // comp acc with float acc
    if ((e1.type==3) && (e1.id==0) && (e2.type==2) && (e2.id==0))
    {
        add_instr("SET_P __aux_var \n");
        add_instr("SET_P __aux_var1\n");
        add_instr("P_LOD __aux_var \n");
        oper_divi(expr_make(2, 0), expr_make(2, 0));
        add_instr("P_LOD __aux_var1\n");
        add_instr("P_LOD __aux_var \n");
        oper_divi(expr_make(2, 0), expr_make(2, 0));
    }

    // comp acc with comp const
    if ((e1.type==3) && (e1.id==0) && (e2.type==5))
    {
        get_cmp_cst(e2,&etr,&eti);

        add_instr("SET_P __aux_var \n");
        add_instr("SET   __aux_var1\n");
        acc_ok = 0;

        emit_sq_sum(etr, eti);
        add_instr("SET   __aux_var2\n");

        add_instr("LOD   __aux_var1\n");
        oper_mult(etr, expr_make(2, 0));
        add_instr("P_LOD __aux_var \n");
        oper_mult(eti, expr_make(2, 0));
        oper_soma(expr_make(2, 0), expr_make(2, 0));
        add_instr("P_LOD __aux_var2\n");
        oper_divi(expr_make(2, 0), expr_make(2, 0));

        add_instr("P_LOD __aux_var1\n");
        oper_mult(eti, expr_make(2, 0));
        add_instr("P_LOD __aux_var \n");
        oper_mult(etr, expr_make(2, 0));
        oper_subt(expr_make(2, 0), expr_make(2, 0));
        add_instr("P_LOD __aux_var2\n");
        oper_divi(expr_make(2, 0), expr_make(2, 0));
        oper_neg (expr_make(2, 0));
    }

    // comp acc with comp var
    if ((e1.type==3) && (e1.id==0) && (e2.type==3) && (e2.id!=0))
    {
        get_cmp_ets(e2,&etr,&eti);

        add_instr("SET_P __aux_var \n");
        add_instr("SET   __aux_var1\n");
        acc_ok = 0;

        emit_sq_sum(etr, eti);
        add_instr("SET   __aux_var2\n");

        add_instr("LOD   __aux_var1\n");
        oper_mult(etr, expr_make(2, 0));
        add_instr("P_LOD __aux_var \n");
        oper_mult(eti, expr_make(2, 0));
        oper_soma(expr_make(2, 0), expr_make(2, 0));
        add_instr("P_LOD __aux_var2\n");
        oper_divi(expr_make(2, 0), expr_make(2, 0));

        add_instr("P_LOD __aux_var1\n");
        oper_mult(eti, expr_make(2, 0));
        add_instr("P_LOD __aux_var \n");
        oper_mult(etr, expr_make(2, 0));
        oper_subt(expr_make(2, 0), expr_make(2, 0));
        add_instr("P_LOD __aux_var2\n");
        oper_divi(expr_make(2, 0), expr_make(2, 0));
        oper_neg (expr_make(2, 0));
    }

    // comp acc with comp acc
    if ((e1.type==3) && (e1.id==0) && (e2.type==3) && (e2.id==0))
    {
        add_instr("SET_P __aux_var \n");
        add_instr("SET_P __aux_var1\n");
        add_instr("SET_P __aux_var2\n");
        add_instr("SET   __aux_var3\n");

        add_instr("LOD   __aux_var1\n");
        add_instr("F_MLT __aux_var1\n");
        add_instr("P_LOD __aux_var \n");
        add_instr("F_MLT __aux_var \n");
        add_instr("SF_ADD        \n");
        add_instr("SET   __aux_var4\n");

        add_instr("LOD   __aux_var3\n");
        add_instr("F_MLT __aux_var1\n");
        add_instr("P_LOD __aux_var2\n");
        add_instr("F_MLT __aux_var \n");
        add_instr("SF_ADD        \n");
        add_instr("P_LOD __aux_var4\n");
        oper_divi(expr_make(2, 0), expr_make(2, 0));

        add_instr("P_LOD __aux_var3\n");
        add_instr("F_MLT __aux_var \n");
        add_instr("P_LOD __aux_var2\n");
        add_instr("F_MLT __aux_var1\n");
        oper_subt(expr_make(2, 0), expr_make(2, 0));
        add_instr("P_LOD __aux_var4\n");
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

// ----------------------------------------------------------------------------
// comparisons with a comp ----------------------------------------------------
// ----------------------------------------------------------------------------
// As in C, two complex numbers are equal when their real parts are equal and
// their imaginary parts are equal; a real is the comp with a zero imaginary
// part. They have no order: <, >, <= and >= with a comp are an error
// (Luciano, 2026-10-30; it compared |z|^2, so (1+0i) == (0+1i) and
// -5 == 3+4i held). The result is (re1 == re2) AND (im1 == im2), EQU being
// one unit for int and float.

// one side of the comparison: in memory (the parts' operands) or in the acc
typedef struct {
    int  acc;         // 1: in the acc (a comp: real on the stack, imag in the acc)
    int  comp;        // 1: a comp, 0: a real
    int  is_int;      // a real int: its real part is converted (I2F)
    char re[600];     // in memory: the real part's operand
    char im[600];     // in memory: the imaginary part's operand
} cmp_side;

static const char *cmp_flit(const char *t)
{
    char b[sizeof(v_table[0].name) + 8]; snprintf(b, sizeof b, "%s", t);
    return v_table[exec_fnum(b)].name;
}

static cmp_side cmp_side_of(expr e)
{
    cmp_side d; memset(&d, 0, sizeof d);
    d.comp = (e.type >= 3);
    if (e.type == 5) {                                  // a comp constant: two float literals
        expr er, ei; get_cmp_cst(e, &er, &ei);
        snprintf(d.re, sizeof d.re, "%s", v_table[er.id].name);
        snprintf(d.im, sizeof d.im, "%s", v_table[ei.id].name);
        return d;
    }
    if (e.id == 0) {d.acc = 1; d.is_int = (e.type == 1); return d;}
    v_table[e.id].used = 1;
    if (e.type == 3) {
        snprintf(d.re, sizeof d.re, "%s", v_table[e.id].name);
        snprintf(d.im, sizeof d.im, "%s", v_table[get_img_id(e.id)].name);
    } else if (e.type == 1 && v_table[e.id].isco == 1) {  // an int literal: the float literal
        char t[sizeof(v_table[0].name) + 4]; snprintf(t, sizeof t, "%s.0", v_table[e.id].name);
        snprintf(d.re, sizeof d.re, "%s", cmp_flit(t));
        snprintf(d.im, sizeof d.im, "%s", cmp_flit("0.0"));
    } else {
        d.is_int = (e.type == 1);
        snprintf(d.re, sizeof d.re, "%s", v_table[e.id].name);
        snprintf(d.im, sizeof d.im, "%s", cmp_flit("0.0"));
    }
    return d;
}

static expr oper_cmp_comp(expr e1, expr e2, int type)
{
    if (type != 2) {fprintf(stderr, MSG_ERR_CMP_COMP_ORDER, line_num+1); exit(EXIT_FAILURE);}

    cmp_side a = cmp_side_of(e1), b = cmp_side_of(e2);
    const char *z = cmp_flit("0.0");

    if (!a.acc && !b.acc)                               // both in memory: 5 words
    {
        if (b.is_int) {cmp_side t = a; a = b; b = t;}   // EQU is symmetric: the int one is loaded
        add_instr("%s %s\n", a.is_int ? (acc_ok ? "P_I2F_M" : "I2F_M") : (acc_ok ? "P_LOD" : "LOD"), a.re);
        add_instr("EQU %s\n", b.re);
        add_instr("P_LOD %s\n", a.im);
        add_instr("EQU %s\n", b.im);
        add_instr("S_AND\n");
    }
    else if (a.acc != b.acc)                            // one in the acc (EQU is symmetric)
    {
        cmp_side x = a.acc ? a : b, m = a.acc ? b : a;
        if (x.comp) {                                   // real on the stack, imag in the acc
            add_instr("EQU %s\n", m.im);
            add_instr("SET_P __aux_var\n");
            if (m.is_int) {add_instr("P_I2F_M %s\n", m.re); add_instr("S_EQU\n");}
            else           add_instr("EQU %s\n", m.re);
            add_instr("AND __aux_var\n");
        } else {                                        // a real in the acc, a comp in memory
            if (x.is_int) add_instr("I2F\n");
            add_instr("EQU %s\n", m.re);
            add_instr("P_LOD %s\n", m.im);
            add_instr("EQU %s\n", z);
            add_instr("S_AND\n");
        }
    }
    else if (a.comp && b.comp)                          // stack: re1, im1, re2; acc: im2
    {
        add_instr("SET_P __aux_var\n");
        add_instr("SET_P __aux_var1\n");
        add_instr("EQU __aux_var\n");
        add_instr("SET_P __aux_var2\n");
        add_instr("EQU __aux_var1\n");
        add_instr("AND __aux_var2\n");
    }
    else if (a.comp)                                    // stack: re1, im1; acc: the real
    {
        if (b.is_int) add_instr("I2F\n");
        add_instr("SET_P __aux_var\n");
        add_instr("EQU %s\n", z);
        add_instr("SET_P __aux_var1\n");
        add_instr("EQU __aux_var\n");
        add_instr("AND __aux_var1\n");
    }
    else                                                // stack: the real, re2; acc: im2
    {
        add_instr("EQU %s\n", z);
        add_instr("SET_P __aux_var\n");
        if (a.is_int) {
            add_instr("SET_P __aux_var1\n");
            add_instr("I2F\n");
            add_instr("EQU __aux_var1\n");
        } else add_instr("S_EQU\n");
        add_instr("AND __aux_var\n");
    }

    acc_ok = 1;
    return expr_make(1, 0); // int boolean in the accumulator
}

// compares greater-than, less-than and equal-to
// - separar o igual pra economizar clock
// - review to avoid unnecessary stack use!
expr oper_cmp(expr e1, expr e2, int type)
{

    if (e1.type >= 3 || e2.type >= 3) return oper_cmp_comp(e1, e2, type);   // a comp: its own rule

    int_lit_float(&e1, &e2);

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

    // The memory forms compute `X op acc` (ula in1 = memory, in2 = acc), so an
    // operand already in the acc is compared against one in memory in a single
    // instruction: e1 in acc, e2 in memory is `rev e2` (a < b == b > a); e2 in
    // acc, e1 in memory is `op e1`. EQU is one unit for int and float.
    char rev [16]; strcpy(rev, type == 0 ? "GRE" : type == 1 ? "LES" : "EQU");
    char fop [16]; if (type == 2) strcpy(fop , "EQU"); else sprintf(fop , "F_%s", op );
    char frev[16]; if (type == 2) strcpy(frev, "EQU"); else sprintf(frev, "F_%s", rev);

    // int var with int var: the variable is loaded and the literal stays in
    // memory, so a value the acc already holds is not loaded again
    if ((e1.type==1) && (e1.id!=0) && (e2.type==1) && (e2.id!=0))
    {
        if (v_table[e2.id].isco == 1 && v_table[e1.id].isco != 1)
        {
            add_instr("%s %s\n", ld , v_table[e1.id].name);
            add_instr("%s %s\n", rev, v_table[e2.id].name);
        }
        else
        {
            add_instr("%s %s\n", ld, v_table[e2.id].name);
            add_instr("%s %s\n", op, v_table[e1.id].name);
        }
    }

    // int var with int acc
    if ((e1.type==1) && (e1.id!=0) && (e2.type==1) && (e2.id==0))
    {
        add_instr("%s %s\n", op, v_table[e1.id].name);
    }

    // int var with float var
    if ((e1.type==1) && (e1.id!=0) && (e2.type==2) && (e2.id!=0))
    {
        add_instr("%s %s\n", i2f , v_table[e1.id].name);
        add_instr("%s %s\n", frev, v_table[e2.id].name);
    }

    // int var with float acc: the variable is converted over the float, which
    // the stack then holds (an int literal is a float literal by now)
    if ((e1.type==1) && (e1.id!=0) && (e2.type==2) && (e2.id==0))
    {
        add_instr("P_I2F_M %s\n", v_table[e1.id].name);
        if (strcmp(op,"EQU")==0) add_instr("S_EQU\n"); else add_instr("SF_%s\n", rev);
    }

    // int acc with int var
    if ((e1.type==1) && (e1.id==0) && (e2.type==1) && (e2.id!=0))
    {
        add_instr("%s %s\n", rev, v_table[e2.id].name);
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
        add_instr("%s %s\n", frev, v_table[e2.id].name);
    }

    // int acc with float acc: the float is set aside, the int converted in the acc
    if ((e1.type==1) && (e1.id==0) && (e2.type==2) && (e2.id==0))
    {
        add_instr("SET_P __aux_var\n");
        add_instr("I2F\n");
        add_instr("%s __aux_var\n", frev);
    }

    // float var with int var
    if ((e1.type==2) && (e1.id!=0) && (e2.type==1) && (e2.id!=0))
    {
        add_instr("%s %s\n", i2f, v_table[e2.id].name);
        add_instr("%s %s\n", fop, v_table[e1.id].name);
    }

    // float var with int acc
    if ((e1.type==2) && (e1.id!=0) && (e2.type==1) && (e2.id==0))
    {
        add_instr("I2F\n");
        add_instr("%s %s\n", fop, v_table[e1.id].name);
    }

    // float var with float var: the variable is loaded, the literal stays in memory
    if ((e1.type==2) && (e1.id!=0) && (e2.type==2) && (e2.id!=0))
    {
        if (v_table[e1.id].isco == 1 && v_table[e2.id].isco != 1)
        {
            add_instr("%s %s\n", ld , v_table[e2.id].name);
            add_instr("%s %s\n", fop, v_table[e1.id].name);
        }
        else
        {
            add_instr("%s %s\n", ld  , v_table[e1.id].name);
            add_instr("%s %s\n", frev, v_table[e2.id].name);
        }
    }

    // float var with float acc
    if ((e1.type==2) && (e1.id!=0) && (e2.type==2) && (e2.id==0))
    {
        add_instr("%s %s\n", fop, v_table[e1.id].name);
    }

    // float acc with int var (an int literal is a float literal by now)
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
        add_instr("%s %s\n", frev, v_table[e2.id].name);
    }

    // float acc with float acc
    if ((e1.type==2) && (e1.id==0) && (e2.type==2) && (e2.id==0))
    {
        if (strcmp(op,"EQU")==0) add_instr("S_EQU\n"); else add_instr("SF_%s\n", op);
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
        // a float is zero exactly when its mantissa is (see emit_cond_int_load)
        add_instr("%s %s\n", ld, v_table[e.id].name);
        add_instr("AND %ld\n", (1L << nbmant) - 1);
        add_instr("LIN\n");
    }

    // when it is a float in the acc
    if ((e.type == 2) && (e.id == 0))
    {
        add_instr("AND %ld\n", (1L << nbmant) - 1);
        add_instr("LIN\n");
    }

    // when it is a comp const
    if (e.type == 5)
    {
        // a comp is true when its real or its imaginary part is not zero
        // (C99): (re OR im) AND mask, then LIN
        get_cmp_cst(e,&etr,&eti);

        add_instr("%s %s\n", ld, v_table[etr.id].name);
        add_instr("ORR %s\n", v_table[eti.id].name);
        add_instr("AND %ld\n", (1L << nbmant) - 1);
        add_instr("LIN\n");
    }

    // when it is a comp variable
    if ((e.type == 3) && (e.id != 0))
    {
        add_instr("%s %s\n", ld, v_table[e.id].name);
        add_instr("ORR %s\n", v_table[get_img_id(e.id)].name);
        add_instr("AND %ld\n", (1L << nbmant) - 1);
        add_instr("LIN\n");
    }

    // when it is a comp in the acc (the imag part in the acc, the real on the stack)
    if ((e.type == 3) && (e.id == 0))
    {
        add_instr("S_ORR\n");
        add_instr("AND %ld\n", (1L << nbmant) - 1);
        add_instr("LIN\n");
    }

    acc_ok = 1;

    return expr_make(1, 0); // int in the accumulator
}

// logical and/or (&& ||)
expr oper_lanor(expr e1, expr e2, int type)
{

    if ((e1.type > 2) || (e2.type > 2))
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

    // a float operand counts by its truth, the word AND 2^NBMANT - 1
    // (zero exactly when the float is); LAN and LOR commute, so the order of
    // the two operands does not matter
    if ((e1.type == 2) || (e2.type == 2))
    {
        long mask = (1L << nbmant) - 1;
        if ((e1.id != 0) && (e2.id != 0))                     // var with var
        {
            add_instr("%s %s\n", ld, v_table[e2.id].name);
            if (e2.type == 2) add_instr("AND %ld\n", mask);
            if (e1.type == 2) { add_instr("P_LOD %s\n", v_table[e1.id].name); add_instr("AND %ld\n", mask); add_instr("S_%s\n", op); }
            else              add_instr("%s %s\n", op, v_table[e1.id].name);
        }
        else if ((e1.id != 0) || (e2.id != 0))                // var with acc
        {
            expr ea = (e1.id == 0) ? e1 : e2;                  // the one in the acc
            expr ev = (e1.id == 0) ? e2 : e1;                  // the one in memory
            if (ea.type == 2) add_instr("AND %ld\n", mask);
            if (ev.type == 2) { add_instr("P_LOD %s\n", v_table[ev.id].name); add_instr("AND %ld\n", mask); add_instr("S_%s\n", op); }
            else              add_instr("%s %s\n", op, v_table[ev.id].name);
        }
        else                                                  // acc with acc: e2 in the acc, e1 on the stack
        {
            if (e2.type == 2) add_instr("AND %ld\n", mask);
            if (e1.type == 2) { add_instr("SET_P __aux_var\n"); add_instr("AND %ld\n", mask); add_instr("%s __aux_var\n", op); }
            else              add_instr("S_%s\n", op);
        }
        acc_ok = 1;
        return expr_make(1, 0);
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

    // acc with var: the three are commutative, so the var is the memory operand
    if ((e1.id == 0) && (e2.id != 0))
    {
        add_instr("%s %s\n", op, v_table[e2.id].name);
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