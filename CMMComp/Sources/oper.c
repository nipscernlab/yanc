// ----------------------------------------------------------------------------
// operacoes com a ula --------------------------------------------------------
// ----------------------------------------------------------------------------

// includes globais
#include <string.h>
#include <stdlib.h>

// includes locais
#include "..\Headers\t2t.h"
#include "..\Headers\oper.h"
#include "..\Headers\stdlib.h"
#include "..\Headers\global.h"
#include "..\Headers\data_use.h"
#include "..\Headers\diretivas.h"
#include "..\Headers\variaveis.h"
#include "..\Headers\messages.h"

// ----------------------------------------------------------------------------
// operacoes aritmeticas ------------------------------------------------------
// ----------------------------------------------------------------------------

// nega um numero
int oper_neg(int et)
{
    int   etr, eti;
    char  num[128];

    char  neg[10]; if (acc_ok == 0) strcpy( neg,   "NEG_M"); else strcpy( neg,  "P_NEG_M");
    char fneg[10]; if (acc_ok == 0) strcpy(fneg, "F_NEG_M"); else strcpy(fneg, "PF_NEG_M");

    // se for um int variavel na memoria
    if ((get_type(et) == 1) && (et % OFST != 0))
    {
        add_instr("%s %s\n", neg, v_name[et % OFST]);
    }

    // se for um int no acc
    if ((get_type(et) == 1) && (et % OFST == 0))
    {
        add_instr("NEG\n");
    }

    // se for um float variavel na memoria
    if ((get_type(et) == 2) && (et % OFST != 0))
    {
        add_instr("%s %s\n", fneg, v_name[et % OFST]);
    }

    // se for um float no acc
    if ((get_type(et) == 2) && (et % OFST == 0))
    {
        add_instr("F_NEG\n");
    }

    // se for um comp constante na memoria
    if (get_type(et) == 5)
    {
        get_cmp_cst(et,&etr,&eti);

        add_instr("%s %s\n", fneg, v_name[etr%OFST]);
        add_instr("PF_NEG_M %s\n", v_name[eti%OFST]);
    }

    // se for um comp variavel na memoria
    if ((get_type(et) == 3) && (et % OFST != 0))
    {
        get_cmp_ets(et,&etr,&eti);

        add_instr("%s %s\n", fneg, v_name[etr%OFST]);
        add_instr("PF_NEG_M %s\n", v_name[eti%OFST]);
    }

    // se for um comp no acc
    if ((get_type(et) == 3) && (et % OFST == 0))
    {
        add_instr("   SET_P aux_var\n");
        add_instr(" F_NEG\n"          );
        add_instr("PF_NEG_M aux_var\n");
    }

    acc_ok = 1;

    if (get_type(et) == 5) et = 3*OFST;

    return get_type(et)*OFST;
}

// soma dois numeros
int oper_soma(int et1, int et2)
{
    int etr, eti;

    char ld [10]; if (acc_ok == 0) strcpy(ld ,"LOD"  ); else strcpy(ld ,"P_LOD"  );
    char i2f[10]; if (acc_ok == 0) strcpy(i2f,"I2F_M"); else strcpy(i2f,"P_I2F_M");

    // int var com int var
    if ((get_type(et1)==1) && (et1%OFST!=0) && (get_type(et2)==1) && (et2%OFST!=0))
    {
        add_instr("%s %s\n" , ld, v_name[et1%OFST]);
        add_instr("ADD %s\n",     v_name[et2%OFST]);
    }

    // int var com int acc
    if ((get_type(et1)==1) && (et1%OFST!=0) && (get_type(et2)==1) && (et2%OFST==0))
    {
        add_instr("ADD %s\n", v_name[et1%OFST]);
    }

    // int var com float var
    if ((get_type(et1)==1) && (et1%OFST!=0) && (get_type(et2)==2) && (et2%OFST!=0))
    {
        add_instr("%s %s\n", i2f, v_name[et1%OFST]);
        add_instr("F_ADD %s\n"  , v_name[et2%OFST]);
    }

    // int var com float acc
    if ((get_type(et1)==1) && (et1%OFST!=0) && (get_type(et2)==2) && (et2%OFST==0))
    {
        add_instr("P_I2F_M %s\n", v_name[et1%OFST]);
        add_instr("SF_ADD\n");
    }

    // int var com comp const
    if ((get_type(et1)==1) && (et1%OFST!=0) && (get_type(et2)==5))
    {
        get_cmp_cst(et2,&etr,&eti);

        add_instr("%s %s\n" , i2f, v_name[et1%OFST]);
        add_instr("F_ADD %s\n"   , v_name[etr%OFST]);
        add_instr("P_LOD %s\n"   , v_name[eti%OFST]);
    }

    // int var com comp var
    if ((get_type(et1)==1) && (et1%OFST!=0) && (get_type(et2)==3) && (et2%OFST!=0))
    {
        get_cmp_ets(et2,&etr,&eti);

        add_instr("%s %s\n", i2f, v_name[et1%OFST]);
        add_instr("F_ADD %s\n"  , v_name[etr%OFST]);
        add_instr("P_LOD  %s\n" , v_name[eti%OFST]);
    }

    // int var com comp acc
    if ((get_type(et1)==1) && (et1%OFST!=0) && (get_type(et2)==3) && (et2%OFST==0))
    {
        add_instr("SET aux_var\n");                 // salva parte imaginaria temporariamente
        add_instr("I2F_M %s\n", v_name[et1%OFST]);  // pega o int ja convertendo pra float
        add_instr("SF_ADD\n");                      // soma acc com pilha
        add_instr("P_LOD aux_var\n");               // joga o resultado pra pilha ao mesmo tempo que pega de volta o imag
    }

    // int acc com int var
    if ((get_type(et1)==1) && (et1%OFST==0) && (get_type(et2)==1) && (et2%OFST!=0))
    {
        add_instr("ADD %s\n", v_name[et2%OFST]);
    }

    // int acc com int acc
    if ((get_type(et1)==1) && (et1%OFST==0) && (get_type(et2)==1) && (et2%OFST==0))
    {
        add_instr("S_ADD\n");
    }

    // int acc com float var
    if ((get_type(et1)==1) && (et1%OFST==0) && (get_type(et2)==2) && (et2%OFST!=0))
    {
        add_instr("I2F\n");
        add_instr("F_ADD %s\n", v_name[et2%OFST]);
    }

    // int acc com float acc
    if ((get_type(et1)==1) && (et1%OFST==0) && (get_type(et2)==2) && (et2%OFST==0))
    {
        add_instr("SET_P aux_var\n"); // salva o float temporariamente jogando o int pro acumulador
        add_instr("I2F\n");           // converte o int do acumulador pra float
        add_instr("F_ADD aux_var\n"); // soma com o float salvo
    }

    // int acc com comp const
    if ((get_type(et1)==1) && (et1%OFST==0) && (get_type(et2)==5))
    {
        get_cmp_cst(et2,&etr,&eti);

        add_instr("I2F\n");
        add_instr("F_ADD %s\n", v_name[etr%OFST]);
        add_instr("P_LOD %s\n", v_name[eti%OFST]);
    }

    // int acc com comp var
    if ((get_type(et1)==1) && (et1%OFST==0) && (get_type(et2)==3) && (et2%OFST!=0))
    {
        get_cmp_ets(et2,&etr,&eti);

        add_instr("I2F\n");
        add_instr("F_ADD %s\n", v_name[etr%OFST]);
        add_instr("P_LOD %s\n", v_name[eti%OFST]);
    }

    // int acc com comp acc
    if ((get_type(et1)==1) && (et1%OFST==0) && (get_type(et2)==3) && (et2%OFST==0))
    {
        add_instr("SET_P aux_var\n" ); // salva parte imaginaria temporariamente dando pop na parte real
        add_instr("SET_P aux_var1\n"); // salva parte real temporariamente dando pop no int
        add_instr("I2F\n");            // converte o int do acumulador pra float
        add_instr("F_ADD aux_var1\n"); // soma com a parte real salva
        add_instr("P_LOD aux_var\n" ); // joga a parte imaginaria de volta pro accumulador
    }

    // float var com int var
    if ((get_type(et1)==2) && (et1%OFST!=0) && (get_type(et2)==1) && (et2%OFST!=0))
    {
        add_instr("%s %s\n",i2f, v_name[et2%OFST]);
        add_instr("F_ADD %s\n" , v_name[et1%OFST]);
    }

    // float var com int acc
    if ((get_type(et1)==2) && (et1%OFST!=0) && (get_type(et2)==1) && (et2%OFST==0))
    {
        add_instr("I2F\n");
        add_instr("F_ADD %s\n", v_name[et1%OFST]);
    }

    // float var com float var
    if ((get_type(et1)==2) && (et1%OFST!=0) && (get_type(et2)==2) && (et2%OFST!=0))
    {
        add_instr("%s %s\n", ld, v_name[et1%OFST]);
        add_instr("F_ADD %s\n" , v_name[et2%OFST]);
    }

    // float var com float acc
    if ((get_type(et1)==2) && (et1%OFST!=0) && (get_type(et2)==2) && (et2%OFST==0))
    {
        add_instr("F_ADD %s\n", v_name[et1%OFST]);
    }

    // float var com comp const
    if ((get_type(et1)==2) && (et1%OFST!=0) && (get_type(et2)==5))
    {
        get_cmp_cst(et2,&etr,&eti);

        add_instr("%s %s\n", ld, v_name[et1%OFST]);
        add_instr("F_ADD %s\n" , v_name[etr%OFST]);
        add_instr("P_LOD %s\n" , v_name[eti%OFST]);
    }

    // float var com comp var
    if ((get_type(et1)==2) && (et1%OFST!=0) && (get_type(et2)==3) && (et2%OFST!=0))
    {
        get_cmp_ets(et2,&etr,&eti);

        add_instr("%s %s\n", ld, v_name[et1%OFST]);
        add_instr("F_ADD %s\n" , v_name[etr%OFST]);
        add_instr("P_LOD %s\n" , v_name[eti%OFST]);
    }

    // float var com comp acc
    if ((get_type(et1)==2) && (et1%OFST!=0) && (get_type(et2)==3) && (et2%OFST==0))
    {
        add_instr("SET_P aux_var\n");
        add_instr("F_ADD %s\n", v_name[et1%OFST]);
        add_instr("P_LOD aux_var\n");
    }

    // float acc com int var
    if ((get_type(et1)==2) && (et1%OFST==0) && (get_type(et2)==1) && (et2%OFST!=0))
    {
        add_instr("P_I2F_M %s\n", v_name[et2%OFST]);
        add_instr("SF_ADD\n");
    }

    // float acc com int acc
    if ((get_type(et1)==2) && (et1%OFST==0) && (get_type(et2)==1) && (et2%OFST==0))
    {
        add_instr("I2F\n");
        add_instr("SF_ADD\n");
    }

    // float acc com float var
    if ((get_type(et1)==2) && (et1%OFST==0) && (get_type(et2)==2) && (et2%OFST!=0))
    {
        add_instr("F_ADD %s\n", v_name[et2%OFST]);
    }

    // float acc com float acc
    if ((get_type(et1)==2) && (et1%OFST==0) && (get_type(et2)==2) && (et2%OFST==0))
    {
        add_instr("SF_ADD\n");
    }

    // float acc com comp const
    if ((get_type(et1)==2) && (et1%OFST==0) && (get_type(et2)==5))
    {
        get_cmp_cst(et2,&etr,&eti);

        add_instr("F_ADD %s\n" , v_name[etr%OFST]);
        add_instr("P_LOD %s\n" , v_name[eti%OFST]);
    }

    // float acc com comp var
    if ((get_type(et1)==2) && (et1%OFST==0) && (get_type(et2)==3) && (et2%OFST!=0))
    {
        get_cmp_ets(et2,&etr,&eti);

        add_instr("F_ADD %s\n", v_name[etr%OFST]);
        add_instr("P_LOD %s\n", v_name[eti%OFST]);
    }

    // float acc com comp acc
    if ((get_type(et1)==2) && (et1%OFST==0) && (get_type(et2)==3) && (et2%OFST==0))
    {
        add_instr("SET_P aux_var\n");
        add_instr("SF_ADD\n");
        add_instr("P_LOD aux_var\n");
    }

    // comp const com int var
    if ((get_type(et1)==5) && (get_type(et2)==1) && (et2%OFST!=0))
    {
        get_cmp_cst(et1,&etr,&eti);

        add_instr("%s %s\n", i2f, v_name[et2%OFST]);
        add_instr("F_ADD %s\n"  , v_name[etr%OFST]);
        add_instr("P_LOD %s\n"  , v_name[eti%OFST]);
    }

    // comp const com int acc
    if ((get_type(et1)==5) && (get_type(et2)==1) && (et2%OFST==0))
    {
        get_cmp_cst(et1,&etr,&eti);

        add_instr("I2F\n");
        add_instr("F_ADD %s\n", v_name[etr%OFST]);
        add_instr("P_LOD %s\n", v_name[eti%OFST]);
    }

    // comp const com float var
    if ((get_type(et1)==5) && (get_type(et2)==2) && (et2%OFST!=0))
    {
        get_cmp_cst(et1,&etr,&eti);

        add_instr("%s %s\n", ld, v_name[et2%OFST]);
        add_instr("F_ADD %s\n" , v_name[etr%OFST]);
        add_instr("P_LOD %s\n" , v_name[eti%OFST]);
    }

    // comp const com float acc
    if ((get_type(et1)==5) && (get_type(et2)==2) && (et2%OFST==0))
    {
        get_cmp_cst(et1,&etr,&eti);

        add_instr("F_ADD %s\n", v_name[etr%OFST]);
        add_instr("P_LOD %s\n", v_name[eti%OFST]);
    }

    // comp const com comp const
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

    // comp const com comp var
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

    // comp const com comp acc
    if ((get_type(et1)==5) && (get_type(et2)==3) && (et2%OFST==0))
    {
        get_cmp_cst(et1,&etr,&eti);

        add_instr("SET_P aux_var\n");
        add_instr("F_ADD %s\n", v_name[etr%OFST]);

        add_instr("P_LOD %s\n", v_name[eti%OFST]);
        add_instr("F_ADD aux_var\n");
    }

    // comp var com int var
    if ((get_type(et1)==3) && (et1%OFST!=0) && (get_type(et2)==1) && (et2%OFST!=0))
    {
        get_cmp_ets(et1,&etr,&eti);

        add_instr("%s  %s\n", i2f, v_name[et2%OFST]);
        add_instr("F_ADD %s\n"    , v_name[etr%OFST]);
        add_instr("P_LOD  %s\n"    , v_name[eti%OFST]);
    }

    // comp var com int acc
    if ((get_type(et1)==3) && (et1%OFST!=0) && (get_type(et2)==1) && (et2%OFST==0))
    {
        get_cmp_ets(et1,&etr,&eti);

        add_instr("I2F\n");
        add_instr("F_ADD %s\n", v_name[etr%OFST]);
        add_instr("P_LOD %s\n", v_name[eti%OFST]);
    }

    // comp var com float var
    if ((get_type(et1)==3) && (et1%OFST!=0) && (get_type(et2)==2) && (et2%OFST!=0))
    {
        get_cmp_ets(et1,&etr,&eti);

        add_instr("%s %s\n", ld, v_name[et2%OFST]);
        add_instr("F_ADD %s\n" , v_name[etr%OFST]);
        add_instr("P_LOD %s\n" , v_name[eti%OFST]);
    }

    // comp var com float acc
    if ((get_type(et1)==3) && (et1%OFST!=0) && (get_type(et2)==2) && (et2%OFST==0))
    {
        get_cmp_ets(et1,&etr,&eti);

        add_instr("F_ADD %s\n", v_name[etr%OFST]);
        add_instr("P_LOD %s\n", v_name[eti%OFST]);
    }

    // comp var com comp const
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

    // comp var com comp var
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

    // comp var com comp acc
    if ((get_type(et1)==3) && (et1%OFST!=0) && (get_type(et2)==3) && (et2%OFST==0))
    {
        get_cmp_ets(et1,&etr,&eti);

        add_instr("SET_P aux_var\n");
        add_instr("F_ADD %s\n", v_name[etr%OFST]);

        add_instr("P_LOD %s\n", v_name[eti%OFST]);
        add_instr("F_ADD aux_var\n");
    }

    // comp acc com int var
    if ((get_type(et1)==3) && (et1%OFST==0) && (get_type(et2)==1) && (et2%OFST!=0))
    {
        add_instr("SET_P aux_var\n");
        add_instr("P_I2F_M %s\n", v_name[et2%OFST]);
        add_instr("SF_ADD\n");
        add_instr("P_LOD aux_var\n");
    }

    // comp acc com int acc
    if ((get_type(et1)==3) && (et1%OFST==0) && (get_type(et2)==1) && (et2%OFST==0))
    {
        add_instr("SET_P   aux_var\n" );
        add_instr("SET_P   aux_var1\n");
        add_instr("P_I2F_M aux_var\n" );
        add_instr("SF_ADD\n");
        add_instr("P_LOD   aux_var1\n");
    }

    // comp acc com float var
    if ((get_type(et1)==3) && (et1%OFST==0) && (get_type(et2)==2) && (et2%OFST!=0))
    {
        add_instr("SET_P aux_var\n");
        add_instr("F_ADD %s\n", v_name[et2%OFST]);
        add_instr("P_LOD aux_var\n");
    }

    // comp acc com float acc
    if ((get_type(et1)==3) && (et1%OFST==0) && (get_type(et2)==2) && (et2%OFST==0))
    {
        add_instr("SET_P aux_var\n" );
        add_instr("SET_P aux_var1\n");
        add_instr("F_ADD aux_var\n" );
        add_instr("P_LOD aux_var1\n");
    }

    // comp acc com comp const
    if ((get_type(et1)==3) && (et1%OFST==0) && (get_type(et2)==5))
    {
        get_cmp_cst(et2,&etr,&eti);

        add_instr("SET_P aux_var\n");
        add_instr("F_ADD %s\n", v_name[etr%OFST]);

        add_instr("P_LOD aux_var\n");
        add_instr("F_ADD %s\n", v_name[eti%OFST]);
    }

    // comp acc com comp var
    if ((get_type(et1)==3) && (et1%OFST==0) && (get_type(et2)==3) && (et2%OFST!=0))
    {
        get_cmp_ets(et2,&etr,&eti);

        add_instr("SET_P aux_var\n");
        add_instr("F_ADD %s\n", v_name[etr%OFST]);

        add_instr("P_LOD aux_var\n");
        add_instr("F_ADD %s\n", v_name[eti%OFST]);
    }

    // comp acc com comp acc
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

    return type*OFST;
}

// subtracao entre dois numeros
int oper_subt(int et1, int et2)
{
    int etr, eti;

    char ld [10]; if (acc_ok == 0) strcpy(ld ,"LOD"  ); else strcpy(ld ,"P_LOD"  );
    char i2f[10]; if (acc_ok == 0) strcpy(i2f,"I2F_M"); else strcpy(i2f,"P_I2F_M");

    // int var com int var
    if ((get_type(et1)==1) && (et1%OFST!=0) && (get_type(et2)==1) && (et2%OFST!=0))
    {
        return oper_soma(et1,oper_neg(et2));
    }

    // int var com int acc
    if ((get_type(et1)==1) && (et1%OFST!=0) && (get_type(et2)==1) && (et2%OFST==0))
    {
        return oper_soma(et1,oper_neg(et2));
    }

    // int var com float var
    if ((get_type(et1)==1) && (et1%OFST!=0) && (get_type(et2)==2) && (et2%OFST!=0))
    {
        add_instr("%s %s\n", i2f, v_name[et1%OFST]);
        add_instr("F_SU1 %s\n"  , v_name[et2%OFST]);
    }

    // int var com float acc
    if ((get_type(et1)==1) && (et1%OFST!=0) && (get_type(et2)==2) && (et2%OFST==0))
    {
        add_instr("%s %s\n", i2f, v_name[et1%OFST]);
        add_instr("SF_SU1\n");
    }

    // int var com comp const (parece que nunca vai acontecer, mas vai que...)
    if ((get_type(et1)==1) && (et1%OFST!=0) && (get_type(et2)==5))
    {
        return oper_soma(et1,oper_neg(et2));
    }

    // int var com comp var
    if ((get_type(et1)==1) && (et1%OFST!=0) && (get_type(et2)==3) && (et2%OFST!=0))
    {
        get_cmp_ets(et2,&etr,&eti);

        add_instr("%s %s\n", i2f   , v_name[et1%OFST]);
        add_instr("F_SU1 %s\n"     , v_name[etr%OFST]);
        add_instr("PF_NEG_M  %s\n" , v_name[eti%OFST]);
    }

    // int var com comp acc
    if ((get_type(et1)==1) && (et1%OFST!=0) && (get_type(et2)==3) && (et2%OFST==0))
    {
        add_instr("SET aux_var\n");                 // salva parte imaginaria temporariamente
        add_instr("I2F_M %s\n", v_name[et1%OFST]);  // pega o int ja convertendo pra float
        add_instr("SF_SU1\n");                      // soma acc com pilha
        add_instr("PF_NEG_M aux_var\n");            // joga o resultado pra pilha ao mesmo tempo que pega de volta o imag
    }

    // int acc com int var
    if ((get_type(et1)==1) && (et1%OFST==0) && (get_type(et2)==1) && (et2%OFST!=0))
    {
        return oper_soma(et1,oper_neg(et2));
    }

    // int acc com int acc
    if ((get_type(et1)==1) && (et1%OFST==0) && (get_type(et2)==1) && (et2%OFST==0))
    {
        return oper_soma(et1,oper_neg(et2));
    }

    // int acc com float var
    if ((get_type(et1)==1) && (et1%OFST==0) && (get_type(et2)==2) && (et2%OFST!=0))
    {
        add_instr("I2F\n");
        add_instr("F_SU1 %s\n", v_name[et2%OFST]);
    }

    // int acc com float acc
    if ((get_type(et1)==1) && (et1%OFST==0) && (get_type(et2)==2) && (et2%OFST==0))
    {
        add_instr("SET_P aux_var\n");
        add_instr("I2F\n");
        add_instr("F_SU1 aux_var\n");
    }

    // int acc com comp const
    if ((get_type(et1)==1) && (et1%OFST==0) && (get_type(et2)==5))
    {
        return oper_soma(et1,oper_neg(et2));
    }

    // int acc com comp var
    if ((get_type(et1)==1) && (et1%OFST==0) && (get_type(et2)==3) && (et2%OFST!=0))
    {
        get_cmp_ets(et2,&etr,&eti);

        add_instr("I2F\n");
        add_instr("F_SU1 %s\n"   , v_name[etr%OFST]);
        add_instr("PF_NEG_M %s\n", v_name[eti%OFST]);
    }

    // int acc com comp acc
    if ((get_type(et1)==1) && (et1%OFST==0) && (get_type(et2)==3) && (et2%OFST==0))
    {
        add_instr("SET_P    aux_var\n" );
        add_instr("SET_P    aux_var1\n");
        add_instr("I2F\n");
        add_instr("F_SU1    aux_var1\n");
        add_instr("PF_NEG_M aux_var\n" );
    }

    // float var com int var
    if ((get_type(et1)==2) && (et1%OFST!=0) && (get_type(et2)==1) && (et2%OFST!=0))
    {
        add_instr("%s %s\n",i2f, v_name[et2%OFST]);
        add_instr("F_SU2 %s\n" , v_name[et1%OFST]);
    }

    // float var com int acc
    if ((get_type(et1)==2) && (et1%OFST!=0) && (get_type(et2)==1) && (et2%OFST==0))
    {
        add_instr("I2F\n");
        add_instr("F_SU2 %s\n", v_name[et1%OFST]);
    }

    // float var com float var
    if ((get_type(et1)==2) && (et1%OFST!=0) && (get_type(et2)==2) && (et2%OFST!=0))
    {
        add_instr("%s %s\n", ld, v_name[et1%OFST]);
        add_instr("F_SU1 %s\n" , v_name[et2%OFST]);
    }

    // float var com float acc
    if ((get_type(et1)==2) && (et1%OFST!=0) && (get_type(et2)==2) && (et2%OFST==0))
    {
        add_instr("F_SU2 %s\n", v_name[et1%OFST]);
    }

    // float var com comp const (nao tem menos comp const)
    if ((get_type(et1)==2) && (et1%OFST!=0) && (get_type(et2)==5))
    {
        return oper_soma(et1,oper_neg(et2));
    }

    // float var com comp var
    if ((get_type(et1)==2) && (et1%OFST!=0) && (get_type(et2)==3) && (et2%OFST!=0))
    {
        get_cmp_ets(et2,&etr,&eti);

        add_instr("%s %s\n",    ld, v_name[et1%OFST]);
        add_instr("F_SU1 %s\n"    , v_name[etr%OFST]);
        add_instr("PF_NEG_M %s\n" , v_name[eti%OFST]);
    }

    // float var com comp acc
    if ((get_type(et1)==2) && (et1%OFST!=0) && (get_type(et2)==3) && (et2%OFST==0))
    {
        add_instr("SET_P aux_var\n");
        add_instr("F_SU2 %s\n", v_name[et1%OFST]);
        add_instr("PF_NEG_M aux_var\n");
    }

    // float acc com int var
    if ((get_type(et1)==2) && (et1%OFST==0) && (get_type(et2)==1) && (et2%OFST!=0))
    {
        add_instr("P_I2F_M %s\n", v_name[et2%OFST]);
        add_instr("SF_SU2\n");
    }

    // float acc com int acc
    if ((get_type(et1)==2) && (et1%OFST==0) && (get_type(et2)==1) && (et2%OFST==0))
    {
        add_instr("I2F\n");
        add_instr("SF_SU2\n");
    }

    // float acc com float var
    if ((get_type(et1)==2) && (et1%OFST==0) && (get_type(et2)==2) && (et2%OFST!=0))
    {
        add_instr("F_SU1 %s\n", v_name[et2%OFST]);
    }

    // float acc com float acc
    if ((get_type(et1)==2) && (et1%OFST==0) && (get_type(et2)==2) && (et2%OFST==0))
    {
        add_instr("SF_SU2\n");
    }

    // float acc com comp const (nao acontece)
    if ((get_type(et1)==2) && (et1%OFST==0) && (get_type(et2)==5))
    {
        return oper_soma(et1,oper_neg(et2));
    }

    // float acc com comp var
    if ((get_type(et1)==2) && (et1%OFST==0) && (get_type(et2)==3) && (et2%OFST!=0))
    {
        get_cmp_ets(et2,&etr,&eti);

        add_instr("F_SU1 %s\n", v_name[etr%OFST]);
        add_instr("PF_NEG_M %s\n", v_name[eti%OFST]);
    }

    // float acc com comp acc
    if ((get_type(et1)==2) && (et1%OFST==0) && (get_type(et2)==3) && (et2%OFST==0))
    {
        add_instr("SET_P aux_var\n");
        add_instr("SF_SU2\n");
        add_instr("PF_NEG_M aux_var\n");
    }

    // comp const com int var
    if ((get_type(et1)==5) && (get_type(et2)==1) && (et2%OFST!=0))
    {
        get_cmp_cst(et1,&etr,&eti);

        add_instr("%s %s\n", i2f, v_name[et2%OFST]);
        add_instr("F_SU2 %s\n"  , v_name[etr%OFST]);
        add_instr("P_LOD %s\n"  , v_name[eti%OFST]);
    }

    // comp const com int acc
    if ((get_type(et1)==5) && (get_type(et2)==1) && (et2%OFST==0))
    {
        get_cmp_cst(et1,&etr,&eti);

        add_instr("I2F\n");
        add_instr("F_SU2 %s\n", v_name[etr%OFST]);
        add_instr("P_LOD %s\n", v_name[eti%OFST]);
    }

    // comp const com float var
    if ((get_type(et1)==5) && (get_type(et2)==2) && (et2%OFST!=0))
    {
        get_cmp_cst(et1,&etr,&eti);

        add_instr("%s %s\n", ld, v_name[et2%OFST]);
        add_instr("F_SU2 %s\n" , v_name[etr%OFST]);
        add_instr("P_LOD %s\n" , v_name[eti%OFST]);
    }

    // comp const com float acc
    if ((get_type(et1)==5) && (get_type(et2)==2) && (et2%OFST==0))
    {
        get_cmp_cst(et1,&etr,&eti);

        add_instr("F_SU2 %s\n", v_name[etr%OFST]);
        add_instr("P_LOD %s\n", v_name[eti%OFST]);
    }

    // comp const com comp const (nao tem subtracao de comp const)
    if ((get_type(et1)==5) && (get_type(et2)==5))
    {
        return oper_soma(et1,oper_neg(et2));
    }

    // comp const com comp var
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

    // comp const com comp acc
    if ((get_type(et1)==5) && (get_type(et2)==3) && (et2%OFST==0))
    {
        get_cmp_cst(et1,&etr,&eti);

        add_instr("SET_P aux_var\n");
        add_instr("F_SU2 %s\n", v_name[etr%OFST]);

        add_instr("P_LOD %s\n", v_name[eti%OFST]);
        add_instr("F_SU1 aux_var\n");
    }

    // comp var com int var
    if ((get_type(et1)==3) && (et1%OFST!=0) && (get_type(et2)==1) && (et2%OFST!=0))
    {
        get_cmp_ets(et1,&etr,&eti);

        add_instr("%s  %s\n", i2f, v_name[et2%OFST]);
        add_instr("F_SU2 %s\n"   , v_name[etr%OFST]);
        add_instr("P_LOD %s\n"   , v_name[eti%OFST]);
    }

    // comp var com int acc
    if ((get_type(et1)==3) && (et1%OFST!=0) && (get_type(et2)==1) && (et2%OFST==0))
    {
        get_cmp_ets(et1,&etr,&eti);

        add_instr("I2F\n");
        add_instr("F_SU2 %s\n", v_name[etr%OFST]);
        add_instr("P_LOD %s\n", v_name[eti%OFST]);
    }

    // comp var com float var
    if ((get_type(et1)==3) && (et1%OFST!=0) && (get_type(et2)==2) && (et2%OFST!=0))
    {
        get_cmp_ets(et1,&etr,&eti);

        add_instr("%s %s\n", ld, v_name[et2%OFST]);
        add_instr("F_SU2 %s\n" , v_name[etr%OFST]);
        add_instr("P_LOD %s\n" , v_name[eti%OFST]);
    }

    // comp var com float acc
    if ((get_type(et1)==3) && (et1%OFST!=0) && (get_type(et2)==2) && (et2%OFST==0))
    {
        get_cmp_ets(et1,&etr,&eti);

        add_instr("F_SU2 %s\n", v_name[etr%OFST]);
        add_instr("P_LOD %s\n", v_name[eti%OFST]);
    }

    // comp var com comp const (nao tem subtracao de comp const)
    if ((get_type(et1)==3) && (et1%OFST!=0) && (get_type(et2)==5))
    {
        return oper_soma(et1,oper_neg(et2));
    }

    // comp var com comp var
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

    // comp var com comp acc
    if ((get_type(et1)==3) && (et1%OFST!=0) && (get_type(et2)==3) && (et2%OFST==0))
    {
        get_cmp_ets(et1,&etr,&eti);

        add_instr("SET_P aux_var\n");
        add_instr("F_SU2 %s\n", v_name[etr%OFST]);

        add_instr("P_LOD %s\n", v_name[eti%OFST]);
        add_instr("F_SU1 aux_var\n");
    }

    // comp acc com int var
    if ((get_type(et1)==3) && (et1%OFST==0) && (get_type(et2)==1) && (et2%OFST!=0))
    {
        add_instr("SET_P aux_var\n");
        add_instr("P_I2F_M %s\n", v_name[et2%OFST]);
        add_instr("SF_SU2\n");
        add_instr("P_LOD aux_var\n");
    }

    // comp acc com int acc
    if ((get_type(et1)==3) && (et1%OFST==0) && (get_type(et2)==1) && (et2%OFST==0))
    {
        add_instr("SET_P   aux_var\n" );
        add_instr("SET_P   aux_var1\n");
        add_instr("P_I2F_M aux_var\n" );
        add_instr("SF_SU2\n");
        add_instr("P_LOD   aux_var1\n");
    }

    // comp acc com float var
    if ((get_type(et1)==3) && (et1%OFST==0) && (get_type(et2)==2) && (et2%OFST!=0))
    {
        add_instr("SET_P aux_var\n");
        add_instr("F_SU1 %s\n", v_name[et2%OFST]);
        add_instr("P_LOD aux_var\n");
    }

    // comp acc com float acc
    if ((get_type(et1)==3) && (et1%OFST==0) && (get_type(et2)==2) && (et2%OFST==0))
    {
        add_instr("SET_P aux_var\n" );
        add_instr("SET_P aux_var1\n");
        add_instr("F_SU1 aux_var\n" );
        add_instr("P_LOD aux_var1\n");
    }

    // comp acc com comp const (nao tem subtracao de comp const)
    if ((get_type(et1)==3) && (et1%OFST==0) && (get_type(et2)==5))
    {
        return oper_soma(et1,oper_neg(et2));
    }

    // comp acc com comp var
    if ((get_type(et1)==3) && (et1%OFST==0) && (get_type(et2)==3) && (et2%OFST!=0))
    {
        get_cmp_ets(et2,&etr,&eti);

        add_instr("SET_P aux_var\n");
        add_instr("F_SU1 %s\n", v_name[etr%OFST]);

        add_instr("P_LOD aux_var\n");
        add_instr("F_SU1 %s\n", v_name[eti%OFST]);
    }

    // comp acc com comp acc
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

    return type*OFST;
}

// multiplica dois numeros
int oper_mult(int et1, int et2)
{
    int etr, eti;

    char ld [10]; if (acc_ok == 0) strcpy(ld ,"LOD"  ); else strcpy(ld ,"P_LOD"  );
    char i2f[10]; if (acc_ok == 0) strcpy(i2f,"I2F_M"); else strcpy(i2f,"P_I2F_M");

    // int var com int var
    if ((get_type(et1)==1) && (et1%OFST!=0) && (get_type(et2)==1) && (et2%OFST!=0))
    {
        add_instr("%s %s\n" , ld, v_name[et1%OFST]);
        add_instr("MLT %s\n",     v_name[et2%OFST]);
    }

    // int var com int acc
    if ((get_type(et1)==1) && (et1%OFST!=0) && (get_type(et2)==1) && (et2%OFST==0))
    {
        add_instr("MLT %s\n", v_name[et1%OFST]);
    }

    // int var com float var
    if ((get_type(et1)==1) && (et1%OFST!=0) && (get_type(et2)==2) && (et2%OFST!=0))
    {
        add_instr("%s %s\n", i2f, v_name[et1%OFST]);
        add_instr("F_MLT %s\n"  , v_name[et2%OFST]);
    }

    // int var com float acc
    if ((get_type(et1)==1) && (et1%OFST!=0) && (get_type(et2)==2) && (et2%OFST==0))
    {
        add_instr("P_I2F_M %s\n", v_name[et1%OFST]);
        add_instr("SF_MLT\n");
    }

    // int var com comp const
    if ((get_type(et1)==1) && (et1%OFST!=0) && (get_type(et2)==5))
    {
        get_cmp_cst(et2,&etr,&eti);

        add_instr("%s %s\n", i2f, v_name[et1%OFST]);
        add_instr("F_MLT %s\n"  , v_name[etr%OFST]);

        add_instr("P_I2F_M %s\n", v_name[et1%OFST]);
        add_instr("F_MLT %s\n"  , v_name[eti%OFST]);
    }

    // int var com comp var
    if ((get_type(et1)==1) && (et1%OFST!=0) && (get_type(et2)==3) && (et2%OFST!=0))
    {
        get_cmp_ets(et2,&etr,&eti);

        add_instr("%s %s\n", i2f, v_name[et1%OFST]);
        add_instr("F_MLT %s\n"  , v_name[etr%OFST]);

        add_instr("P_I2F_M %s\n", v_name[et1%OFST]);
        add_instr("F_MLT %s\n"  , v_name[eti%OFST]);
    }

    // int var com comp acc
    if ((get_type(et1)==1) && (et1%OFST!=0) && (get_type(et2)==3) && (et2%OFST==0))
    {
        add_instr("SET_P aux_var\n");
        add_instr("P_I2F_M %s\n", v_name[et1%OFST]);
        add_instr("SF_MLT\n");

        add_instr("P_LOD aux_var\n");
        add_instr("P_I2F_M %s\n", v_name[et1%OFST]);
        add_instr("SF_MLT\n");
    }

    // int acc com int var
    if ((get_type(et1)==1) && (et1%OFST==0) && (get_type(et2)==1) && (et2%OFST!=0))
    {
        add_instr("MLT %s\n", v_name[et2%OFST]);
    }

    // int acc com int acc
    if ((get_type(et1)==1) && (et1%OFST==0) && (get_type(et2)==1) && (et2%OFST==0))
    {
        add_instr("S_MLT\n");
    }

    // int acc com float var
    if ((get_type(et1)==1) && (et1%OFST==0) && (get_type(et2)==2) && (et2%OFST!=0))
    {
        add_instr("I2F\n");
        add_instr("F_MLT %s\n", v_name[et2%OFST]);
    }

    // int acc com float acc
    if ((get_type(et1)==1) && (et1%OFST==0) && (get_type(et2)==2) && (et2%OFST==0))
    {
        add_instr("SET_P aux_var\n");
        add_instr("I2F\n");
        add_instr("F_MLT aux_var\n");
    }

    // int acc com comp const
    if ((get_type(et1)==1) && (et1%OFST==0) && (get_type(et2)==5))
    {
        get_cmp_cst(et2,&etr,&eti);

        add_instr("I2F\n");
        add_instr("SET   aux_var\n");
        add_instr("F_MLT %s\n", v_name[etr%OFST]);
        add_instr("P_LOD aux_var\n");
        add_instr("F_MLT %s\n", v_name[eti%OFST]);
    }

    // int acc com comp var
    if ((get_type(et1)==1) && (et1%OFST==0) && (get_type(et2)==3) && (et2%OFST!=0))
    {
        get_cmp_ets(et2,&etr,&eti);

        add_instr("I2F\n");
        add_instr("SET   aux_var\n");
        add_instr("F_MLT %s\n", v_name[etr%OFST]);
        add_instr("P_LOD aux_var\n");
        add_instr("F_MLT %s\n", v_name[eti%OFST]);
    }

    // int acc com comp acc
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

    // float var com int var
    if ((get_type(et1)==2) && (et1%OFST!=0) && (get_type(et2)==1) && (et2%OFST!=0))
    {
        add_instr("%s %s\n" ,i2f, v_name[et2%OFST]);
        add_instr("F_MLT %s\n",   v_name[et1%OFST]);
    }

    // float var com int acc
    if ((get_type(et1)==2) && (et1%OFST!=0) && (get_type(et2)==1) && (et2%OFST==0))
    {
        add_instr("I2F\n");
        add_instr("F_MLT %s\n", v_name[et1%OFST]);
    }

    // float var com float var
    if ((get_type(et1)==2) && (et1%OFST!=0) && (get_type(et2)==2) && (et2%OFST!=0))
    {
        add_instr("%s %s\n", ld, v_name[et1%OFST]);
        add_instr("F_MLT %s\n",  v_name[et2%OFST]);
    }

    // float var com float acc
    if ((get_type(et1)==2) && (et1%OFST!=0) && (get_type(et2)==2) && (et2%OFST==0))
    {
        add_instr("F_MLT %s\n", v_name[et1%OFST]);
    }

    // float var com comp const
    if ((get_type(et1)==2) && (et1%OFST!=0) && (get_type(et2)==5))
    {
        get_cmp_cst(et2,&etr,&eti);

        add_instr("%s %s\n", ld, v_name[et1%OFST]);
        add_instr("F_MLT %s\n",  v_name[etr%OFST]);

        add_instr("P_LOD %s\n",  v_name[et1%OFST]);
        add_instr("F_MLT %s\n",  v_name[eti%OFST]);
    }

    // float var com comp var
    if ((get_type(et1)==2) && (et1%OFST!=0) && (get_type(et2)==3) && (et2%OFST!=0))
    {
        get_cmp_ets(et2,&etr,&eti);

        add_instr("%s %s\n", ld, v_name[et1%OFST]);
        add_instr("F_MLT %s\n",  v_name[etr%OFST]);

        add_instr("P_LOD %s\n",  v_name[et1%OFST]);
        add_instr("F_MLT %s\n",  v_name[eti%OFST]);
    }

    // float var com comp acc
    if ((get_type(et1)==2) && (et1%OFST!=0) && (get_type(et2)==3) && (et2%OFST==0))
    {
        add_instr("SET_P aux_var\n");
        add_instr("F_MLT %s\n", v_name[et1%OFST]);
        add_instr("P_LOD aux_var\n");
        add_instr("F_MLT %s\n", v_name[et1%OFST]);
    }

    // float acc com int var
    if ((get_type(et1)==2) && (et1%OFST==0) && (get_type(et2)==1) && (et2%OFST!=0))
    {
        add_instr("P_I2F_M %s\n", v_name[et2%OFST]);
        add_instr("SF_MLT\n");
    }

    // float acc com int acc
    if ((get_type(et1)==2) && (et1%OFST==0) && (get_type(et2)==1) && (et2%OFST==0))
    {
        add_instr("I2F\n");
        add_instr("SF_MLT\n");
    }

    // float acc com float var
    if ((get_type(et1)==2) && (et1%OFST==0) && (get_type(et2)==2) && (et2%OFST!=0))
    {
        add_instr("F_MLT %s\n", v_name[et2%OFST]);
    }

    // float acc com float acc
    if ((get_type(et1)==2) && (et1%OFST==0) && (get_type(et2)==2) && (et2%OFST==0))
    {
        add_instr("SF_MLT\n");
    }

    // float acc com comp const
    if ((get_type(et1)==2) && (et1%OFST==0) && (get_type(et2)==5))
    {
        get_cmp_cst(et2,&etr,&eti);

        add_instr("SET   aux_var\n");
        add_instr("F_MLT %s\n", v_name[etr%OFST]);
        add_instr("P_LOD %s\n", v_name[eti%OFST]);
        add_instr("F_MLT aux_var\n");
    }

    // float acc com comp var
    if ((get_type(et1)==2) && (et1%OFST==0) && (get_type(et2)==3) && (et2%OFST!=0))
    {
        get_cmp_ets(et2,&etr,&eti);

        add_instr("SET   aux_var\n");
        add_instr("F_MLT %s\n", v_name[etr%OFST]);
        add_instr("P_LOD %s\n", v_name[eti%OFST]);
        add_instr("F_MLT aux_var\n");
    }

    // float acc com comp acc
    if ((get_type(et1)==2) && (et1%OFST==0) && (get_type(et2)==3) && (et2%OFST==0))
    {
        add_instr("SET_P aux_var \n");
        add_instr("SET_P aux_var1\n");
        add_instr("SET   aux_var2\n");
        add_instr("F_MLT aux_var1\n");
        add_instr("P_LOD aux_var \n");
        add_instr("F_MLT aux_var2\n");
    }

    // comp const com int var
    if ((get_type(et1)==5) && (get_type(et2)==1) && (et2%OFST!=0))
    {
        get_cmp_cst(et1,&etr,&eti);

        add_instr("%s %s\n", i2f, v_name[et2%OFST]);
        add_instr("F_MLT %s\n"  , v_name[etr%OFST]);
        add_instr("P_I2F_M %s\n", v_name[et2%OFST]);
        add_instr("F_MLT %s\n"  , v_name[eti%OFST]);
    }

    // comp const com int acc
    if ((get_type(et1)==5) && (get_type(et2)==1) && (et2%OFST==0))
    {
        get_cmp_cst(et1,&etr,&eti);

        add_instr("I2F\n");
        add_instr("SET   aux_var\n");
        add_instr("F_MLT %s\n"  , v_name[etr%OFST]);
        add_instr("P_LOD aux_var\n");
        add_instr("F_MLT %s\n"  , v_name[eti%OFST]);
    }

    // comp const com float var
    if ((get_type(et1)==5) && (get_type(et2)==2) && (et2%OFST!=0))
    {
        get_cmp_cst(et1,&etr,&eti);

        add_instr("%s %s\n", ld, v_name[et2%OFST]);
        add_instr("F_MLT %s\n" , v_name[etr%OFST]);
        add_instr("P_LOD %s\n" , v_name[et2%OFST]);
        add_instr("F_MLT %s\n" , v_name[eti%OFST]);
    }

    // comp const com float acc
    if ((get_type(et1)==5) && (get_type(et2)==2) && (et2%OFST==0))
    {
        get_cmp_cst(et1,&etr,&eti);

        add_instr("SET   aux_var\n");
        add_instr("F_MLT %s\n"  , v_name[etr%OFST]);
        add_instr("P_LOD aux_var\n");
        add_instr("F_MLT %s\n"  , v_name[eti%OFST]);
    }

    // comp const com comp const
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

    // comp const com comp var
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

    // comp const com comp acc
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

    // comp var com int var
    if ((get_type(et1)==3) && (et1%OFST!=0) && (get_type(et2)==1) && (et2%OFST!=0))
    {
        get_cmp_ets(et1,&etr,&eti);

        add_instr("%s %s\n", i2f, v_name[et2%OFST]);
        add_instr("F_MLT %s\n"  , v_name[etr%OFST]);
        add_instr("P_I2F_M %s\n", v_name[et2%OFST]);
        add_instr("F_MLT %s\n"  , v_name[eti%OFST]);
    }

    // comp var com int acc
    if ((get_type(et1)==3) && (et1%OFST!=0) && (get_type(et2)==1) && (et2%OFST==0))
    {
        get_cmp_ets(et1,&etr,&eti);

        add_instr("I2F\n");
        add_instr("SET   aux_var\n");
        add_instr("F_MLT %s\n"  , v_name[etr%OFST]);
        add_instr("P_LOD aux_var\n");
        add_instr("F_MLT %s\n"  , v_name[eti%OFST]);
    }

    // comp var com float var
    if ((get_type(et1)==3) && (et1%OFST!=0) && (get_type(et2)==2) && (et2%OFST!=0))
    {
        get_cmp_ets(et1,&etr,&eti);

        add_instr("%s %s\n", ld, v_name[et2%OFST]);
        add_instr("F_MLT %s\n" , v_name[etr%OFST]);
        add_instr("P_LOD %s\n" , v_name[et2%OFST]);
        add_instr("F_MLT %s\n" , v_name[eti%OFST]);
    }

    // comp var com float acc
    if ((get_type(et1)==3) && (et1%OFST!=0) && (get_type(et2)==2) && (et2%OFST==0))
    {
        get_cmp_ets(et1,&etr,&eti);

        add_instr("SET   aux_var\n");
        add_instr("F_MLT %s\n"  , v_name[etr%OFST]);
        add_instr("P_LOD aux_var\n");
        add_instr("F_MLT %s\n"  , v_name[eti%OFST]);
    }

    // comp var com comp const
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

    // comp var com comp var
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

    // comp var com comp acc
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

    // comp acc com int var
    if ((get_type(et1)==3) && (et1%OFST==0) && (get_type(et2)==1) && (et2%OFST!=0))
    {
        add_instr("SET_P aux_var\n" );
        add_instr("P_I2F_M %s\n", v_name[et2%OFST]);
        add_instr("SET   aux_var1\n");
        add_instr("SF_MLT\n");
        add_instr("P_LOD aux_var\n" );
        add_instr("F_MLT aux_var1\n");
    }

    // comp acc com int acc
    if ((get_type(et1)==3) && (et1%OFST==0) && (get_type(et2)==1) && (et2%OFST==0))
    {
        add_instr("I2F\n");
        add_instr("SET_P aux_var\n" );
        add_instr("SET_P aux_var1\n");
        add_instr("F_MLT aux_var\n" );
        add_instr("P_LOD aux_var1\n");
        add_instr("F_MLT aux_var\n" );
    }

    // comp acc com float var
    if ((get_type(et1)==3) && (et1%OFST==0) && (get_type(et2)==2) && (et2%OFST!=0))
    {
        add_instr("SET_P aux_var\n");
        add_instr("F_MLT %s\n", v_name[et2%OFST]);
        add_instr("P_LOD aux_var\n");
        add_instr("F_MLT %s\n", v_name[et2%OFST]);
    }

    // comp acc com float acc
    if ((get_type(et1)==3) && (et1%OFST==0) && (get_type(et2)==2) && (et2%OFST==0))
    {
        add_instr("SET_P aux_var\n" );
        add_instr("SET_P aux_var1\n");
        add_instr("F_MLT aux_var\n" );
        add_instr("P_LOD aux_var1\n");
        add_instr("F_MLT aux_var\n ");
    }

    // comp acc com comp const
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

    // comp acc com comp var
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

    // comp acc com comp acc
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

    return type*OFST;
}

// divisao entre dois numeros
int oper_divi(int et1, int et2)
{
    int etr, eti;

    char ld [10]; if (acc_ok == 0) strcpy(ld ,"LOD"  ); else strcpy(ld ,"P_LOD"  );
    char i2f[10]; if (acc_ok == 0) strcpy(i2f,"I2F_M"); else strcpy(i2f,"P_I2F_M");

    // int var com int var
    if ((get_type(et1)==1) && (et1%OFST!=0) && (get_type(et2)==1) && (et2%OFST!=0))
    {
        add_instr("%s %s\n" , ld, v_name[et2%OFST]);
        add_instr("DIV %s\n",     v_name[et1%OFST]);    
    }

    // int var com int acc
    if ((get_type(et1)==1) && (et1%OFST!=0) && (get_type(et2)==1) && (et2%OFST==0))
    {
        add_instr("DIV %s\n", v_name[et1%OFST]);
    }

    // int var com float var
    if ((get_type(et1)==1) && (et1%OFST!=0) && (get_type(et2)==2) && (et2%OFST!=0))
    {
        add_instr("%s %s\n" , i2f, v_name[et1%OFST]);
        add_instr("P_LOD %s\n"   , v_name[et2%OFST]);
        add_instr("SF_DIV\n");
    }

    // int var com float acc
    if ((get_type(et1)==1) && (et1%OFST!=0) && (get_type(et2)==2) && (et2%OFST==0))
    {
        add_instr("SET   aux_var\n");
        add_instr("I2F_M %s\n", v_name[et1%OFST]);
        add_instr("P_LOD aux_var\n");
        add_instr("SF_DIV\n");
    }

    // int var com comp const
    if ((get_type(et1)==1) && (et1%OFST!=0) && (get_type(et2)==5))
    {
        get_cmp_cst(et2,&etr,&eti);

        oper_mult(etr,etr);           // parte real ao quadrado
        oper_mult(eti,eti);           // parte imag ao quadrado
        oper_soma(2*OFST,2*OFST);     // soma os quadrados
        add_instr("SET aux_var\n");   // salva o resultado

        acc_ok = 0;                   // libera acumulador

        oper_mult(et1,etr);           // mult int com parte real
        add_instr("P_LOD aux_var\n"); // pega o denominador
        oper_divi(2*OFST,2*OFST);     // faz a divisao

        oper_mult(et1,eti);           // mult int com parte imag
        add_instr("P_LOD aux_var\n"); // pega o denominador
        oper_divi(2*OFST,2*OFST);     // faz a divisao
        oper_neg (2*OFST);            // nega a parte imaginaria
    }

    // int var com comp var
    if ((get_type(et1)==1) && (et1%OFST!=0) && (get_type(et2)==3) && (et2%OFST!=0))
    {
        get_cmp_ets(et2,&etr,&eti);

        oper_mult(etr,etr);           // parte real ao quadrado
        oper_mult(eti,eti);           // parte imag ao quadrado
        oper_soma(2*OFST,2*OFST);     // soma os quadrados
        add_instr("SET aux_var\n");   // salva o resultado

        acc_ok = 0;                   // libera acumulador

        oper_mult(et1,etr);           // mult int com parte real
        add_instr("P_LOD aux_var\n"); // pega o denominador
        oper_divi(2*OFST,2*OFST);     // faz a divisao

        oper_mult(et1,eti);           // mult int com parte imag
        add_instr("P_LOD aux_var\n"); // pega o denominador
        oper_divi(2*OFST,2*OFST);     // faz a divisao
        oper_neg (2*OFST);            // nega a parte imaginaria
    }

    // int var com comp acc
    if ((get_type(et1)==1) && (et1%OFST!=0) && (get_type(et2)==3) && (et2%OFST==0))
    {
        add_instr("SET_P aux_var \n"); // salva a parte imaginaria
        add_instr("SET   aux_var1\n"); // salva a parte real

        add_instr("F_MLT aux_var1\n"); // parte real ao quadrado
        add_instr("P_LOD aux_var \n"); // pega a parte imaginaria
        add_instr("F_MLT aux_var \n"); // parte imag ao quadrado
        add_instr("SF_ADD        \n"); // soma os quadrados
        add_instr("SET   aux_var2\n"); // salva o modulo ao quadrado

        add_instr("I2F_M %s\n", v_name[et1%OFST]); 
        add_instr("SET   aux_var3\n"); // salva o float
        add_instr("F_MLT aux_var1\n");
        add_instr("P_LOD aux_var2\n"); // pega o modulo ao quadrado
        oper_divi(2*OFST,2*OFST);      // faz a divisao

        add_instr("P_LOD aux_var3\n"); // pega o float
        add_instr("F_MLT aux_var \n");
        add_instr("P_LOD aux_var2\n"); // pega o modulo ao quadrado
        oper_divi(2*OFST,2*OFST);      // faz a divisao
        oper_neg (2*OFST);
    }

    // int acc com int var
    if ((get_type(et1)==1) && (et1%OFST==0) && (get_type(et2)==1) && (et2%OFST!=0))
    {
        add_instr("%s %s\n", ld, v_name[et2%OFST]);
        add_instr("S_DIV\n");
    }

    // int acc com int acc
    if ((get_type(et1)==1) && (et1%OFST==0) && (get_type(et2)==1) && (et2%OFST==0))
    {
        add_instr("S_DIV\n");
    }

    // int acc com float var
    if ((get_type(et1)==1) && (et1%OFST==0) && (get_type(et2)==2) && (et2%OFST!=0))
    {
        add_instr("I2F\n");
        add_instr("P_LOD %s\n", v_name[et2%OFST]);
        add_instr("SF_DIV\n");
    }

    // int acc com float acc
    if ((get_type(et1)==1) && (et1%OFST==0) && (get_type(et2)==2) && (et2%OFST==0))
    {
        add_instr("SET_P aux_soma\n");
        add_instr("I2F\n");
        add_instr("P_LOD aux_soma\n");
        add_instr("SF_DIV\n");
    }

    // int acc com comp const
    if ((get_type(et1)==1) && (et1%OFST==0) && (get_type(et2)==5))
    {
        get_cmp_cst(et2,&etr,&eti);

        add_instr("I2F\n");
        add_instr("SET aux_var\n");
        acc_ok = 0;

        oper_mult(etr,etr);            // parte real ao quadrado
        oper_mult(eti,eti);            // parte imag ao quadrado
        oper_soma(2*OFST,2*OFST);      // soma os quadrados
        add_instr("SET   aux_var1\n"); // salva o resultado

        add_instr("LOD   aux_var\n");
        oper_mult(2*OFST,etr);         // mult float com parte real
        add_instr("P_LOD aux_var1\n"); // pega o denominador
        oper_divi(2*OFST,2*OFST);      // faz a divisao

        add_instr("P_LOD  aux_var\n");
        oper_mult(2*OFST,eti);         // mult float com parte imag
        add_instr("P_LOD aux_var1\n"); // pega o denominador
        oper_divi(2*OFST,2*OFST);      // faz a divisao
        oper_neg (2*OFST);
    }

    // int acc com comp var
    if ((get_type(et1)==1) && (et1%OFST==0) && (get_type(et2)==3) && (et2%OFST!=0))
    {
        get_cmp_ets(et2,&etr,&eti);

        add_instr("I2F\n");
        add_instr("SET   aux_var\n");
        acc_ok = 0;

        oper_mult(etr,etr);             // parte real ao quadrado
        oper_mult(eti,eti);             // parte imag ao quadrado
        oper_soma(2*OFST,2*OFST);       // soma os quadrados
        add_instr("SET   aux_var1\n");  // salva o resultado

        add_instr("LOD   aux_var\n" );
        oper_mult(2*OFST,etr);          // mult float com parte real
        add_instr("P_LOD aux_var1\n");  // pega o denominador
        oper_divi(2*OFST,2*OFST);       // faz a divisao

        add_instr("P_LOD  aux_var\n" );
        oper_mult(2*OFST,eti);          // mult float com parte imag
        add_instr("P_LOD  aux_var1\n"); // pega o denominador
        oper_divi(2*OFST,2*OFST);       // faz a divisao
        oper_neg (2*OFST);
    }

    // int acc com comp acc
    if ((get_type(et1)==1) && (et1%OFST==0) && (get_type(et2)==3) && (et2%OFST==0))
    {
        add_instr("SET_P aux_var \n"); // salva a parte imaginaria
        add_instr("SET_P aux_var1\n"); // salva a parte real
        add_instr("I2F           \n");
        add_instr("SET   aux_var2\n"); // salva o float

        add_instr("LOD   aux_var1\n"); // coloca a parte real na pilha
        add_instr("F_MLT aux_var1\n");
        add_instr("P_LOD aux_var \n"); // pega a parte imaginaria
        add_instr("F_MLT aux_var \n");
        add_instr("SF_ADD        \n");
        add_instr("SET   aux_var3\n"); // salva o modulo ao quadrado

        add_instr("LOD   aux_var2\n"); // carrega o float
        add_instr("F_MLT aux_var1\n");
        add_instr("P_LOD aux_var3\n"); // pega o modulo ao quadrado
        oper_divi(2*OFST,2*OFST);      // faz a divisao

        add_instr("P_LOD aux_var2\n"); // pega o float
        add_instr("F_MLT aux_var \n");
        add_instr("P_LOD aux_var3\n"); // pega o modulo ao quadrado
        oper_divi(2*OFST,2*OFST);      // faz a divisao
        oper_neg (2*OFST);
    }

    // float var com int var
    if ((get_type(et1)==2) && (et1%OFST!=0) && (get_type(et2)==1) && (et2%OFST!=0))
    {
        add_instr("%s %s\n", i2f, v_name[et2%OFST]);
        add_instr("F_DIV %s\n"  , v_name[et1%OFST]);
    }

    // float var com int acc
    if ((get_type(et1)==2) && (et1%OFST!=0) && (get_type(et2)==1) && (et2%OFST==0))
    {
        add_instr("I2F\n");
        add_instr("F_DIV %s\n", v_name[et1%OFST]);
    }

    // float var com float var
    if ((get_type(et1)==2) && (et1%OFST!=0) && (get_type(et2)==2) && (et2%OFST!=0))
    {
        add_instr("%s %s\n" , ld, v_name[et2%OFST]);
        add_instr("F_DIV %s\n"  , v_name[et1%OFST]);
    }

    // float var com float acc
    if ((get_type(et1)==2) && (et1%OFST!=0) && (get_type(et2)==2) && (et2%OFST==0))
    {
        add_instr("F_DIV %s\n", v_name[et1%OFST]);
    }

    // float var com comp const
    if ((get_type(et1)==2) && (et1%OFST!=0) && (get_type(et2)==5))
    {
        get_cmp_cst(et2,&etr,&eti);

        oper_mult(etr,etr);           // parte real ao quadrado
        oper_mult(eti,eti);           // parte imag ao quadrado
        oper_soma(2*OFST,2*OFST);     // soma os quadrados
        add_instr("SET   aux_var\n"); // salva o resultado
        acc_ok = 0;

        oper_mult(et1,etr);           // mult float com parte real
        add_instr("P_LOD aux_var\n"); // pega o denominador
        oper_divi(2*OFST,2*OFST);     // faz a divisao

        oper_mult(et1,eti);           // mult float com parte imag
        add_instr("P_LOD aux_var\n"); // pega o denominador
        oper_divi(2*OFST,2*OFST);     // faz a divisao
        oper_neg (2*OFST);
    }

    // float var com comp var
    if ((get_type(et1)==2) && (et1%OFST!=0) && (get_type(et2)==3) && (et2%OFST!=0))
    {
        get_cmp_ets(et2,&etr,&eti);

        oper_mult(etr,etr);           // parte real ao quadrado
        oper_mult(eti,eti);           // parte imag ao quadrado
        oper_soma(2*OFST,2*OFST);     // soma os quadrados
        add_instr("SET   aux_var\n"); // salva o resultado
        acc_ok = 0;

        oper_mult(et1,etr);           // mult float com parte real
        add_instr("P_LOD aux_var\n"); // pega o denominador
        oper_divi(2*OFST,2*OFST);     // faz a divisao

        oper_mult(et1,eti);           // mult float com parte imag
        add_instr("P_LOD aux_var\n"); // pega o denominador
        oper_divi(2*OFST,2*OFST);     // faz a divisao
        oper_neg (2*OFST);
    }

    // float var com comp acc
    if ((get_type(et1)==2) && (et1%OFST!=0) && (get_type(et2)==3) && (et2%OFST==0))
    {
        add_instr("SET_P aux_var \n");             // salva a parte imaginaria
        add_instr("SET   aux_var1\n");             // salva a parte real
        add_instr("F_MLT aux_var1\n");
        add_instr("P_LOD aux_var \n");             // pega a parte imaginaria
        add_instr("F_MLT aux_var \n");
        add_instr("SF_ADD        \n");
        add_instr("SET   aux_var2\n");             // salva o modulo ao quadrado

        add_instr("LOD %s\n"  , v_name[et1%OFST]); // carrega o float
        add_instr("F_MLT aux_var1\n");
        add_instr("P_LOD aux_var2\n");             // pega o modulo ao quadrado
        oper_divi(2*OFST,2*OFST);                  // faz a divisao

        add_instr("P_LOD %s\n", v_name[et1%OFST]); // carrega o float
        add_instr("F_MLT aux_var \n");
        add_instr("P_LOD aux_var2\n");             // pega o modulo ao quadrado
        oper_divi(2*OFST,2*OFST);                  // faz a divisao
        oper_neg (2*OFST);
    }

    // float acc com int var
    if ((get_type(et1)==2) && (et1%OFST==0) && (get_type(et2)==1) && (et2%OFST!=0))
    {
        add_instr("P_I2F_M %s\n", v_name[et2%OFST]);
        add_instr("SF_DIV\n");
    }

    // float acc com int acc
    if ((get_type(et1)==2) && (et1%OFST==0) && (get_type(et2)==1) && (et2%OFST==0))
    {
        add_instr("I2F\n");
        add_instr("SF_DIV\n");
    }

    // float acc com float var
    if ((get_type(et1)==2) && (et1%OFST==0) && (get_type(et2)==2) && (et2%OFST!=0))
    {
        add_instr("P_LOD %s\n", v_name[et2%OFST]);
        add_instr("SF_DIV\n");
    }

    // float acc com float acc
    if ((get_type(et1)==2) && (et1%OFST==0) && (get_type(et2)==2) && (et2%OFST==0))
    {
        add_instr("SF_DIV\n");
    }

    // float acc com comp const
    if ((get_type(et1)==2) && (et1%OFST==0) && (get_type(et2)==5))
    {
        get_cmp_cst(et2,&etr,&eti);

        add_instr("SET   aux_var\n");
        acc_ok = 0;

        oper_mult(etr,etr);            // parte real ao quadrado
        oper_mult(eti,eti);            // parte imag ao quadrado
        oper_soma(2*OFST,2*OFST);      // soma os quadrados
        add_instr("SET   aux_var1\n"); // salva o resultado

        add_instr("LOD   aux_var \n");
        oper_mult(2*OFST,etr);         // mult float com parte real
        add_instr("P_LOD aux_var1\n"); // pega o denominador
        oper_divi(2*OFST,2*OFST);      // faz a divisao

        add_instr("P_LOD aux_var \n");
        oper_mult(2*OFST,eti);         // mult float com parte imag
        add_instr("P_LOD aux_var1\n"); // pega o denominador
        oper_divi(2*OFST,2*OFST);      // faz a divisao
        oper_neg (2*OFST);
    }

    // float acc com comp var
    if ((get_type(et1)==2) && (et1%OFST==0) && (get_type(et2)==3) && (et2%OFST!=0))
    {
        get_cmp_ets(et2,&etr,&eti);

        add_instr("SET aux_var\n");
        acc_ok = 0;

        oper_mult(etr,etr);            // parte real ao quadrado
        oper_mult(eti,eti);            // parte imag ao quadrado
        oper_soma(2*OFST,2*OFST);      // soma os quadrados
        add_instr("SET   aux_var1\n"); // salva o resultado

        add_instr("LOD   aux_var\n");
        oper_mult(2*OFST,etr);         // mult float com parte real
        add_instr("P_LOD aux_var1\n"); // pega o denominador
        oper_divi(2*OFST,2*OFST);      // faz a divisao

        add_instr("P_LOD aux_var\n");
        oper_mult(2*OFST,eti);         // mult float com parte imag
        add_instr("P_LOD aux_var1\n"); // pega o denominador
        oper_divi(2*OFST,2*OFST);      // faz a divisao
        oper_neg (2*OFST);
    }

    // float acc com comp acc
    if ((get_type(et1)==2) && (et1%OFST==0) && (get_type(et2)==3) && (et2%OFST==0))
    {
        add_instr("SET_P aux_var \n"); // salva a parte imaginaria
        add_instr("SET_P aux_var1\n"); // salva a parte real
        add_instr("SET   aux_var2\n"); // salva o float

        add_instr("LOD   aux_var1\n"); // coloca a parte real na pilha
        add_instr("F_MLT aux_var1\n");
        add_instr("P_LOD aux_var \n"); // pega a parte imaginaria
        add_instr("F_MLT aux_var \n");
        add_instr("SF_ADD        \n");
        add_instr("SET   aux_var3\n"); // salva o modulo ao quadrado

        add_instr("LOD   aux_var2\n"); // carrega o float
        add_instr("F_MLT aux_var1\n");
        add_instr("P_LOD aux_var3\n"); // pega o modulo ao quadrado
        oper_divi(2*OFST,2*OFST);      // faz a divisao

        add_instr("P_LOD aux_var2\n"); // pega o float
        add_instr("F_MLT aux_var \n");
        add_instr("P_LOD aux_var3\n"); // pega o modulo ao quadrado
        oper_divi(2*OFST,2*OFST);      // faz a divisao
        oper_neg (2*OFST);
    }

    // comp const com int var
    if ((get_type(et1)==5) && (get_type(et2)==1) && (et2%OFST!=0))
    {
        get_cmp_cst(et1,&etr,&eti);

        oper_divi(etr,et2);
        oper_divi(eti,et2);
    }

    // comp const com int acc
    if ((get_type(et1)==5) && (get_type(et2)==1) && (et2%OFST==0))
    {
        get_cmp_cst(et1,&etr,&eti);

        add_instr("I2F\n");
        add_instr("SET   aux_var\n");
        oper_divi(etr,2*OFST);
        add_instr("P_LOD aux_var\n");
        oper_divi(eti,2*OFST);
    }

    // comp const com float var
    if ((get_type(et1)==5) && (get_type(et2)==2) && (et2%OFST!=0))
    {
        get_cmp_cst(et1,&etr,&eti);

        oper_divi(etr,et2);
        oper_divi(eti,et2);
    }

    // comp const com float acc
    if ((get_type(et1)==5) && (get_type(et2)==2) && (et2%OFST==0))
    {
        get_cmp_cst(et1,&etr,&eti);

        add_instr("SET   aux_var\n");
        oper_divi(etr,2*OFST);
        add_instr("P_LOD aux_var\n");
        oper_divi(eti,2*OFST);
    }

    // comp const com comp const
    if ((get_type(et1)==5) && (get_type(et2)==5))
    {
        int et1r,et2r;
        int et1i,et2i;

        get_cmp_cst(et1,&et1r,&et1i);
        get_cmp_cst(et2,&et2r,&et2i);

        oper_mult(et2r,et2r);
        oper_mult(et2i,et2i);
        oper_soma(2*OFST,2*OFST);
        add_instr("SET   aux_var\n");
        acc_ok = 0;

        oper_mult(et1r,et2r);
        oper_mult(et1i,et2i);
        oper_soma(2*OFST,2*OFST);
        add_instr("P_LOD aux_var\n");
        oper_divi(2*OFST,2*OFST);

        oper_mult(et1i,et2r);
        oper_mult(et1r,et2i);
        oper_subt(2*OFST,2*OFST);
        add_instr("P_LOD aux_var\n");
        oper_divi(2*OFST,2*OFST);
    }

    // comp const com comp var
    if ((get_type(et1)==5) && (get_type(et2)==3) && (et2%OFST!=0))
    {
        int et1r,et2r;
        int et1i,et2i;

        get_cmp_cst(et1,&et1r,&et1i);
        get_cmp_ets(et2,&et2r,&et2i);

        oper_mult(et2r,et2r);
        oper_mult(et2i,et2i);
        oper_soma(2*OFST,2*OFST);
        add_instr("SET aux_var\n");
        acc_ok = 0;

        oper_mult(et1r,et2r);
        oper_mult(et1i,et2i);
        oper_soma(2*OFST,2*OFST);
        add_instr("P_LOD aux_var\n");
        oper_divi(2*OFST,2*OFST);

        oper_mult(et1i,et2r);
        oper_mult(et1r,et2i);
        oper_subt(2*OFST,2*OFST);
        add_instr("P_LOD aux_var\n");
        oper_divi(2*OFST,2*OFST);
    }

    // comp const com comp acc
    if ((get_type(et1)==5) && (get_type(et2)==3) && (et2%OFST==0))
    {
        get_cmp_cst(et1,&etr,&eti);

        add_instr("SET_P aux_var \n");
        add_instr("SET   aux_var1\n");
        add_instr("F_MLT aux_var1\n");
        add_instr("P_LOD aux_var \n");
        add_instr("F_MLT aux_var \n");
        oper_soma(2*OFST,2*OFST);
        add_instr("SET   aux_var2\n");

        add_instr("LOD   aux_var1\n");
        oper_mult(etr   ,2*OFST);
        add_instr("P_LOD aux_var \n");
        oper_mult(eti   ,2*OFST);
        oper_soma(2*OFST,2*OFST);
        add_instr("P_LOD aux_var2\n");
        oper_divi(2*OFST,2*OFST);

        add_instr("P_LOD aux_var1\n");
        oper_mult(eti,2*OFST);
        add_instr("P_LOD aux_var \n");
        oper_mult(etr   ,2*OFST);
        oper_subt(2*OFST,2*OFST);
        add_instr("P_LOD aux_var2\n");
        oper_divi(2*OFST,2*OFST);
    }

    // comp var com int var
    if ((get_type(et1)==3) && (et1%OFST!=0) && (get_type(et2)==1) && (et2%OFST!=0))
    {
        get_cmp_ets(et1,&etr,&eti);

        oper_divi(etr,et2);
        oper_divi(eti,et2);
    }

    // comp var com int acc
    if ((get_type(et1)==3) && (et1%OFST!=0) && (get_type(et2)==1) && (et2%OFST==0))
    {
        get_cmp_ets(et1,&etr,&eti);

        add_instr("I2F\n");
        add_instr("SET   aux_var\n");
        oper_divi(etr,2*OFST);
        add_instr("P_LOD aux_var\n");
        oper_divi(eti,2*OFST);
    }

    // comp var com float var
    if ((get_type(et1)==3) && (et1%OFST!=0) && (get_type(et2)==2) && (et2%OFST!=0))
    {
        get_cmp_ets(et1,&etr,&eti);

        oper_divi(etr,et2);
        oper_divi(eti,et2);
    }

    // comp var com float acc
    if ((get_type(et1)==3) && (et1%OFST!=0) && (get_type(et2)==2) && (et2%OFST==0))
    {
        get_cmp_ets(et1,&etr,&eti);

        add_instr("SET   aux_var\n");
        oper_divi(etr,2*OFST);
        add_instr("P_LOD aux_var\n");
        oper_divi(eti,2*OFST);
    }

    // comp var com comp const
    if ((get_type(et1)==3) && (et1%OFST!=0) && (get_type(et2)==5))
    {
        int et1r,et2r;
        int et1i,et2i;

        get_cmp_ets(et1,&et1r,&et1i);
        get_cmp_cst(et2,&et2r,&et2i);

        oper_mult(et2r,et2r);
        oper_mult(et2i,et2i);
        oper_soma(2*OFST,2*OFST);
        add_instr("SET   aux_var\n");
        acc_ok = 0;

        oper_mult(et1r,et2r);
        oper_mult(et1i,et2i);
        oper_soma(2*OFST,2*OFST);
        add_instr("P_LOD aux_var\n");
        oper_divi(2*OFST,2*OFST);

        oper_mult(et1i,et2r);
        oper_mult(et1r,et2i);
        oper_subt(2*OFST,2*OFST);
        add_instr("P_LOD aux_var\n");
        oper_divi(2*OFST,2*OFST);
    }

    // comp var com comp var
    if ((get_type(et1)==3) && (et1%OFST!=0) && (get_type(et2)==3) && (et2%OFST!=0))
    {
        int et1r,et2r;
        int et1i,et2i;

        get_cmp_ets(et1,&et1r,&et1i);
        get_cmp_ets(et2,&et2r,&et2i);

        oper_mult(et2r,et2r);
        oper_mult(et2i,et2i);
        oper_soma(2*OFST,2*OFST);
        add_instr("SET   aux_var\n");
        acc_ok = 0;

        oper_mult(et1r,et2r);
        oper_mult(et1i,et2i);
        oper_soma(2*OFST,2*OFST);
        add_instr("P_LOD aux_var\n");
        oper_divi(2*OFST,2*OFST);

        oper_mult(et1i,et2r);
        oper_mult(et1r,et2i);
        oper_subt(2*OFST,2*OFST);
        add_instr("P_LOD aux_var\n");
        oper_divi(2*OFST,2*OFST);
    }

    // comp var com comp acc
    if ((get_type(et1)==3) && (et1%OFST!=0) && (get_type(et2)==3) && (et2%OFST==0))
    {
        get_cmp_ets(et1,&etr,&eti);

        add_instr("SET_P aux_var \n");
        add_instr("SET   aux_var1\n");
        add_instr("F_MLT aux_var1\n");
        add_instr("P_LOD aux_var \n");
        add_instr("F_MLT aux_var \n");
        oper_soma(2*OFST,2*OFST);
        add_instr("SET   aux_var2\n");

        add_instr("LOD   aux_var1\n");
        oper_mult(etr   ,2*OFST);
        add_instr("P_LOD aux_var \n");
        oper_mult(eti   ,2*OFST);
        oper_soma(2*OFST,2*OFST);
        add_instr("P_LOD aux_var2\n");
        oper_divi(2*OFST,2*OFST);

        add_instr("P_LOD aux_var1\n");
        oper_mult(eti   ,2*OFST);
        add_instr("P_LOD aux_var \n");
        oper_mult(etr   ,2*OFST);
        oper_subt(2*OFST,2*OFST);
        add_instr("P_LOD aux_var2\n");
        oper_divi(2*OFST,2*OFST);
    }

    // comp acc com int var
    if ((get_type(et1)==3) && (et1%OFST==0) && (get_type(et2)==1) && (et2%OFST!=0))
    {
        add_instr("SET_P aux_var \n");
        add_instr("P_I2F_M %s\n", v_name[et2%OFST]);
        add_instr("SET   aux_var1\n");
        oper_divi(2*OFST,2*OFST);
        add_instr("P_LOD aux_var \n");
        add_instr("P_LOD aux_var1\n");
        oper_divi(2*OFST,2*OFST);
    }

    // comp acc com int acc
    if ((get_type(et1)==3) && (et1%OFST==0) && (get_type(et2)==1) && (et2%OFST==0))
    {
        add_instr("I2F\n");
        add_instr("SET_P aux_var \n");
        add_instr("SET_P aux_var1\n");
        add_instr("P_LOD aux_var \n");
        oper_divi(2*OFST,2*OFST);
        add_instr("P_LOD aux_var1\n");
        add_instr("P_LOD aux_var \n");
        oper_divi(2*OFST,2*OFST);
    }

    // comp acc com float var
    if ((get_type(et1)==3) && (et1%OFST==0) && (get_type(et2)==2) && (et2%OFST!=0))
    {
        add_instr("SET_P aux_var\n");
        add_instr("P_LOD %s\n", v_name[et2%OFST]);
        oper_divi(2*OFST,2*OFST);
        add_instr("P_LOD aux_var\n");
        add_instr("P_LOD %s\n", v_name[et2%OFST]);
        oper_divi(2*OFST,2*OFST);
    }

    // comp acc com float acc
    if ((get_type(et1)==3) && (et1%OFST==0) && (get_type(et2)==2) && (et2%OFST==0))
    {
        add_instr("SET_P aux_var \n");
        add_instr("SET_P aux_var1\n");
        add_instr("P_LOD aux_var \n");
        oper_divi(2*OFST,2*OFST);
        add_instr("P_LOD aux_var1\n");
        add_instr("P_LOD aux_var \n");
        oper_divi(2*OFST,2*OFST);
    }

    // comp acc com comp const
    if ((get_type(et1)==3) && (et1%OFST==0) && (get_type(et2)==5))
    {
        get_cmp_cst(et2,&etr,&eti);

        add_instr("SET_P aux_var \n");
        add_instr("SET   aux_var1\n");
        acc_ok = 0;

        oper_mult(etr,etr);
        oper_mult(eti,eti);
        oper_soma(2*OFST,2*OFST);
        add_instr("SET   aux_var2\n");

        add_instr("LOD   aux_var1\n");
        oper_mult(etr   ,2*OFST);
        add_instr("P_LOD aux_var \n");
        oper_mult(eti   ,2*OFST);
        oper_soma(2*OFST,2*OFST);
        add_instr("P_LOD aux_var2\n");
        oper_divi(2*OFST,2*OFST);

        add_instr("P_LOD aux_var1\n");
        oper_mult(eti   ,2*OFST);
        add_instr("P_LOD aux_var \n");
        oper_mult(etr   ,2*OFST);
        oper_subt(2*OFST,2*OFST);
        add_instr("P_LOD aux_var2\n");
        oper_divi(2*OFST,2*OFST);
        oper_neg (2*OFST);
    }

    // comp acc com comp var
    if ((get_type(et1)==3) && (et1%OFST==0) && (get_type(et2)==3) && (et2%OFST!=0))
    {
        get_cmp_ets(et2,&etr,&eti);

        add_instr("SET_P aux_var \n");
        add_instr("SET   aux_var1\n");
        acc_ok = 0;

        oper_mult(etr,etr);
        oper_mult(eti,eti);
        oper_soma(2*OFST,2*OFST);
        add_instr("SET   aux_var2\n");

        add_instr("LOD   aux_var1\n");
        oper_mult(etr   ,2*OFST);
        add_instr("P_LOD aux_var \n");
        oper_mult(eti   ,2*OFST);
        oper_soma(2*OFST,2*OFST);
        add_instr("P_LOD aux_var2\n");
        oper_divi(2*OFST,2*OFST);

        add_instr("P_LOD aux_var1\n");
        oper_mult(eti   ,2*OFST);
        add_instr("P_LOD aux_var \n");
        oper_mult(etr   ,2*OFST);
        oper_subt(2*OFST,2*OFST);
        add_instr("P_LOD aux_var2\n");
        oper_divi(2*OFST,2*OFST);
        oper_neg (2*OFST);
    }

    // comp acc com comp acc
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
        oper_divi(2*OFST,2*OFST);

        add_instr("P_LOD aux_var3\n");
        add_instr("F_MLT aux_var \n");
        add_instr("P_LOD aux_var2\n");
        add_instr("F_MLT aux_var1\n");
        oper_subt(2*OFST,2*OFST);
        add_instr("P_LOD aux_var4\n");
        oper_divi(2*OFST,2*OFST);
        oper_neg (2*OFST);
    }

    acc_ok = 1;

    int type;
         if ((get_type(et1) > 2) || (get_type(et2) > 2))
         type = 3;
    else if ((get_type(et1) > 1) || (get_type(et2) > 1))
         type = 2;
    else type = 1;

    return type*OFST;
}

// resto da divisao
int oper_mod(int et1, int et2)
{
    if ((get_type(et1) > 1) || (get_type(et2) > 1))
        {fprintf(stderr, MSG_ERR_MOD_NON_INT, line_num+1); exit(EXIT_FAILURE);}

    char ld[10]; if (acc_ok == 0) strcpy(ld,"LOD"); else strcpy(ld,"P_LOD");

    // int var com int var
    if ((get_type(et1) == 1) && (et1%OFST != 0) && (get_type(et2) == 1) && (et2%OFST != 0))
    {
        add_instr("%s %s\n" , ld, v_name[et2%OFST]);
        add_instr("MOD %s\n",     v_name[et1%OFST]);
    }

    // int var com int acc
    if ((get_type(et1) == 1) && (et1%OFST != 0) && (get_type(et2) == 1) && (et2%OFST == 0))
    {
        add_instr("MOD %s\n", v_name[et1%OFST]);
    }

    // int acc com int var
    if ((get_type(et1) == 1) && (et1%OFST == 0) && (get_type(et2) == 1) && (et2%OFST != 0))
    {
        add_instr("P_LOD %s\n", v_name[et2%OFST]);
        add_instr("S_MOD\n");
    }

    // int acc com int acc
    if ((get_type(et1) == 1) && (et1%OFST == 0) && (get_type(et2) == 1) && (et2%OFST == 0))
    {
        add_instr("S_MOD\n");
    }

    acc_ok = 1;

    return OFST; // retorna o id extendido de int
}

// ----------------------------------------------------------------------------
// operacoes de comparacao ----------------------------------------------------
// ----------------------------------------------------------------------------

// compara maior, menor e igual
// - separar o igual pra economizar clock
// - rever para evitar uso de pilha a toa!
int oper_cmp(int et1, int et2, int type)
{
    int  etr, eti;

    char ld [10]; if (acc_ok == 0) strcpy(ld , "LOD" ); else strcpy(ld ,"P_LOD"  );
    char i2f[10]; if (acc_ok == 0) strcpy(i2f,"I2F_M"); else strcpy(i2f,"P_I2F_M");

    char op[16];
    switch (type)
    {
        case 0: strcpy(op, "LES"); break;
        case 1: strcpy(op, "GRE"); break;
        case 2: strcpy(op, "EQU"); break;
    }

    // int var com int var
    if ((get_type(et1)==1) && (et1%OFST!=0) && (get_type(et2)==1) && (et2%OFST!=0))
    {
        add_instr("%s %s\n", ld, v_name[et2%OFST]);
        add_instr("%s %s\n", op, v_name[et1%OFST]);
    }

    // int var com int acc
    if ((get_type(et1)==1) && (et1%OFST!=0) && (get_type(et2)==1) && (et2%OFST==0))
    {
        add_instr("%s %s\n", op, v_name[et1%OFST]);
    }

    // int var com float var
    if ((get_type(et1)==1) && (et1%OFST!=0) && (get_type(et2)==2) && (et2%OFST!=0))
    {
        add_instr("%s %s\n", i2f, v_name[et1%OFST]);
        add_instr("P_LOD %s\n"  , v_name[et2%OFST]);

        if (strcmp(op,"EQU")==0) add_instr("S_EQU\n"); else add_instr("SF_%s\n", op);
    }

    // int var com float acc
    if ((get_type(et1)==1) && (et1%OFST!=0) && (get_type(et2)==2) && (et2%OFST==0))
    {
        add_instr("SET   aux_var\n");
        add_instr("I2F_M %s\n", v_name[et1%OFST]);
        add_instr("P_LOD aux_var\n");
        
        if (strcmp(op,"EQU")==0) add_instr("S_EQU\n"); else add_instr("SF_%s\n", op);
    }

    // int var com comp const
    if ((get_type(et1)==1) && (et1%OFST!=0) && (get_type(et2)==5))
    {
        fprintf(stdout, MSG_WARN_CMP_INT_COMP, line_num+1);

        oper_mult(et1,et1);
        exec_mod2(et2);
        oper_cmp (OFST,2*OFST,type);
    }

    // int var com comp var
    if ((get_type(et1)==1) && (et1%OFST!=0) && (get_type(et2)==3) && (et2%OFST!=0))
    {
        fprintf(stdout, MSG_WARN_CMP_INT_COMP, line_num+1);

        oper_mult(et1,et1);
        exec_mod2(et2);
        oper_cmp (OFST,2*OFST,type);
    }

    // int var com comp acc
    if ((get_type(et1)==1) && (et1%OFST!=0) && (get_type(et2)==3) && (et2%OFST==0))
    {
        fprintf(stdout, MSG_WARN_CMP_INT_COMP, line_num+1);

        add_instr("SET_P aux_var \n");
        add_instr("SET   aux_var1\n");
        acc_ok = 0;

        oper_mult(et1,et1);
        add_instr("P_LOD aux_var1\n");
        add_instr("P_LOD aux_var \n");
        exec_mod2(3*OFST);
        oper_cmp (OFST,2*OFST,type);
    }

    // int acc com int var
    if ((get_type(et1)==1) && (et1%OFST==0) && (get_type(et2)==1) && (et2%OFST!=0))
    {
        add_instr("%s %s\n", ld, v_name[et2%OFST]);
        add_instr("S_%s\n", op);
    }

    // int acc com int acc
    if ((get_type(et1)==1) && (et1%OFST==0) && (get_type(et2)==1) && (et2%OFST==0))
    {
        add_instr("S_%s\n", op);
    }

    // int acc com float var
    if ((get_type(et1)==1) && (et1%OFST==0) && (get_type(et2)==2) && (et2%OFST!=0))
    {
        add_instr("I2F\n");
        add_instr("P_LOD %s\n", v_name[et2%OFST]);
        
        if (strcmp(op,"EQU")==0) add_instr("S_EQU\n"); else add_instr("SF_%s\n", op);
    }

    // int acc com float acc
    if ((get_type(et1)==1) && (et1%OFST==0) && (get_type(et2)==2) && (et2%OFST==0))
    {
        add_instr("SET_P aux_var\n");
        add_instr("I2F\n");
        add_instr("P_LOD aux_var\n");
        
        if (strcmp(op,"EQU")==0) add_instr("S_EQU\n"); else add_instr("SF_%s\n", op);
    }

    // int acc com comp const
    if ((get_type(et1)==1) && (et1%OFST==0) && (get_type(et2)==5))
    {
        fprintf(stdout, MSG_WARN_CMP_INT_COMP, line_num+1);

        add_instr("SET   aux_var\n");
        add_instr("P_LOD aux_var\n");
        oper_mult(et1,et1);
        exec_mod2(et2);
        oper_cmp (OFST,2*OFST,type);
    }

    // int acc com comp var
    if ((get_type(et1)==1) && (et1%OFST==0) && (get_type(et2)==3) && (et2%OFST!=0))
    {
        fprintf(stdout, MSG_WARN_CMP_INT_COMP, line_num+1);

        add_instr("SET   aux_var\n");
        add_instr("P_LOD aux_var\n");
        oper_mult(et1,et1);
        exec_mod2(et2);
        oper_cmp (OFST,2*OFST,type);
    }

    // int acc com comp acc
    if ((get_type(et1)==1) && (et1%OFST==0) && (get_type(et2)==3) && (et2%OFST==0))
    {
        fprintf(stdout, MSG_WARN_CMP_INT_COMP, line_num+1);

        add_instr("SET_P aux_var \n");
        add_instr("SET_P aux_var1\n");
        add_instr("SET   aux_var2\n");

        add_instr("P_LOD aux_var2\n");
        oper_mult(et1,et1);
        add_instr("P_LOD aux_var1\n");
        add_instr("P_LOD aux_var \n");
        exec_mod2(3*OFST);
        oper_cmp (OFST,2*OFST,type);
    }

    // float var com int var
    if ((get_type(et1)==2) && (et1%OFST!=0) && (get_type(et2)==1) && (et2%OFST!=0))
    {
        add_instr("%s %s\n", ld , v_name[et1%OFST]);
        add_instr("P_I2F_M %s\n", v_name[et2%OFST]);
        
        if (strcmp(op,"EQU")==0) add_instr("S_EQU\n"); else add_instr("SF_%s\n", op);
    }

    // float var com int acc
    if ((get_type(et1)==2) && (et1%OFST!=0) && (get_type(et2)==1) && (et2%OFST==0))
    {
        add_instr("I2F\n");
        add_instr("SET   aux_var\n");
        add_instr("LOD %s\n", v_name[et1%OFST]);
        add_instr("P_LOD aux_var\n");
        
        if (strcmp(op,"EQU")==0) add_instr("S_EQU\n"); else add_instr("SF_%s\n", op);
    }

    // float var com float var
    if ((get_type(et1)==2) && (et1%OFST!=0) && (get_type(et2)==2) && (et2%OFST!=0))
    {
        add_instr("%s %s\n"   , ld, v_name[et1%OFST]);
        add_instr("P_LOD %s\n",     v_name[et2%OFST]);
        
        if (strcmp(op,"EQU")==0) add_instr("S_EQU\n"); else add_instr("SF_%s\n", op);
    }

    // float var com float acc
    if ((get_type(et1)==2) && (et1%OFST!=0) && (get_type(et2)==2) && (et2%OFST==0))
    {
        add_instr("SET   aux_var\n");
        add_instr("LOD %s\n", v_name[et1%OFST]);
        add_instr("P_LOD aux_var\n");
        
        if (strcmp(op,"EQU")==0) add_instr("S_EQU\n"); else add_instr("SF_%s\n", op);
    }

    // float var com comp const
    if ((get_type(et1)==2) && (et1%OFST!=0) && (get_type(et2)==5))
    {
        fprintf(stdout, MSG_WARN_CMP_FLOAT_COMP, line_num+1);

        oper_mult(et1,et1);
        exec_mod2(et2);
        oper_cmp (2*OFST,2*OFST,type);
    }

    // float var com comp var
    if ((get_type(et1)==2) && (et1%OFST!=0) && (get_type(et2)==3) && (et2%OFST!=0))
    {
        fprintf(stdout, MSG_WARN_CMP_FLOAT_COMP, line_num+1);

        oper_mult(et1,et1);
        exec_mod2(et2);
        oper_cmp (2*OFST,2*OFST,type);
    }

    // float var com comp acc
    if ((get_type(et1)==2) && (et1%OFST!=0) && (get_type(et2)==3) && (et2%OFST==0))
    {
        fprintf(stdout, MSG_WARN_CMP_INT_COMP, line_num+1);

        add_instr("SET_P aux_var \n");
        add_instr("SET   aux_var1\n");
        acc_ok = 0;

        oper_mult(et1,et1);
        add_instr("P_LOD aux_var1\n");
        add_instr("P_LOD aux_var \n");
        exec_mod2(3*OFST);
        oper_cmp (2*OFST,2*OFST,type);
    }

    // float acc com int var
    if ((get_type(et1)==2) && (et1%OFST==0) && (get_type(et2)==1) && (et2%OFST!=0))
    {
        add_instr("P_I2F_M %s\n", v_name[et2%OFST]);
        
        if (strcmp(op,"EQU")==0) add_instr("S_EQU\n"); else add_instr("SF_%s\n", op);
    }

    // float acc com int acc
    if ((get_type(et1)==2) && (et1%OFST==0) && (get_type(et2)==1) && (et2%OFST==0))
    {
        add_instr("I2F\n");
        
        if (strcmp(op,"EQU")==0) add_instr("S_EQU\n"); else add_instr("SF_%s\n", op);
    }

    // float acc com float var
    if ((get_type(et1)==2) && (et1%OFST==0) && (get_type(et2)==2) && (et2%OFST!=0))
    {
        add_instr("P_LOD %s\n", v_name[et2%OFST]);
        
        if (strcmp(op,"EQU")==0) add_instr("S_EQU\n"); else add_instr("SF_%s\n", op);
    }

    // float acc com float acc
    if ((get_type(et1)==2) && (et1%OFST==0) && (get_type(et2)==2) && (et2%OFST==0))
    {
        if (strcmp(op,"EQU")==0) add_instr("S_EQU\n"); else add_instr("SF_%s\n", op);
    }

    // float acc com comp const
    if ((get_type(et1)==2) && (et1%OFST==0) && (get_type(et2)==5))
    {
        fprintf(stdout, MSG_WARN_CMP_FLOAT_COMP, line_num+1);

        add_instr("SET   aux_var\n");
        add_instr("P_LOD aux_var\n");
        oper_mult(et1,et1);
        exec_mod2(et2);
        oper_cmp (2*OFST,2*OFST,type);
    }

    // float acc com comp var
    if ((get_type(et1)==2) && (et1%OFST==0) && (get_type(et2)==3) && (et2%OFST!=0))
    {
        fprintf(stdout, MSG_WARN_CMP_FLOAT_COMP, line_num+1);

        add_instr("SET   aux_var\n");
        add_instr("P_LOD aux_var\n");
        oper_mult(et1,et1);
        exec_mod2(et2);
        oper_cmp (2*OFST,2*OFST,type);
    }

    // float acc com comp acc
    if ((get_type(et1)==2) && (et1%OFST==0) && (get_type(et2)==3) && (et2%OFST==0))
    {
        fprintf(stdout, MSG_WARN_CMP_FLOAT_COMP, line_num+1);

        add_instr("SET_P aux_var \n");
        add_instr("SET_P aux_var1\n");
        add_instr("SET   aux_var2\n");

        add_instr("P_LOD aux_var2\n");
        oper_mult(et1,et1);
        add_instr("P_LOD aux_var1\n");
        add_instr("P_LOD aux_var \n");
        exec_mod2(3*OFST);
        oper_cmp (2*OFST,2*OFST,type);
    }

    // comp const com int var
    if ((get_type(et1)==5) && (get_type(et2)==1) && (et2%OFST!=0))
    {
        fprintf(stdout, MSG_WARN_CMP_INT_COMP, line_num+1);

        exec_mod2(et1);
        oper_mult(et2,et2);
        oper_cmp (2*OFST,OFST,type);
    }

    // comp const com int acc
    if ((get_type(et1)==5) && (get_type(et2)==1) && (et2%OFST==0))
    {
        fprintf(stdout, MSG_WARN_CMP_INT_COMP, line_num+1);

        add_instr("SET   aux_var\n");
        acc_ok = 0;
        exec_mod2(et1);
        add_instr("P_LOD aux_var\n");
        add_instr("P_LOD aux_var\n");
        oper_mult(et2,et2);
        oper_cmp (2*OFST,OFST,type);
    }

    // comp const com float var
    if ((get_type(et1)==5) && (get_type(et2)==2) && (et2%OFST!=0))
    {
        fprintf(stdout, MSG_WARN_CMP_FLOAT_COMP, line_num+1);

        exec_mod2(et1);
        oper_mult(et2,et2);
        oper_cmp (2*OFST,2*OFST,type);
    }

    // comp const com float acc
    if ((get_type(et1)==5) && (get_type(et2)==2) && (et2%OFST==0))
    {
        fprintf(stdout, MSG_WARN_CMP_FLOAT_COMP, line_num+1);

        add_instr("SET   aux_var\n");
        acc_ok = 0;
        exec_mod2(et1);
        add_instr("P_LOD aux_var\n");
        add_instr("P_LOD aux_var\n");
        oper_mult(et2,et2);
        oper_cmp (2*OFST,2*OFST,type);
    }

    // comp const com comp const
    if ((get_type(et1)==5) && (get_type(et2)==5))
    {
        fprintf(stdout, MSG_WARN_CMP_COMPLEX, line_num+1);

        exec_mod2(et1);
        exec_mod2(et2);
        oper_cmp (2*OFST,2*OFST,type);
    }

    // comp const com comp var
    if ((get_type(et1)==5) && (get_type(et2)==3) && (et2%OFST!=0))
    {
        fprintf(stdout, MSG_WARN_CMP_COMPLEX, line_num+1);

        exec_mod2(et1);
        exec_mod2(et2);
        oper_cmp (2*OFST,2*OFST,type);
    }

    // comp const com comp acc
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
        oper_cmp (2*OFST,2*OFST,type);
    }

    // comp var com int var
    if ((get_type(et1)==3) && (et1%OFST!=0) && (get_type(et2)==1) && (et2%OFST!=0))
    {
        fprintf(stdout, MSG_WARN_CMP_INT_COMP, line_num+1);

        exec_mod2(et1);
        oper_mult(et2,et2);
        oper_cmp (2*OFST,OFST,type);
    }

    // comp var com int acc
    if ((get_type(et1)==3) && (et1%OFST!=0) && (get_type(et2)==1) && (et2%OFST==0))
    {
        fprintf(stdout, MSG_WARN_CMP_INT_COMP, line_num+1);

        add_instr("SET   aux_var\n");
        acc_ok = 0;
        exec_mod2(et1);
        add_instr("P_LOD aux_var\n");
        add_instr("P_LOD aux_var\n");
        oper_mult(et2,et2);
        oper_cmp (2*OFST,OFST,type);
    }

    // comp var com float var
    if ((get_type(et1)==3) && (et1%OFST!=0) && (get_type(et2)==2) && (et2%OFST!=0))
    {
        fprintf(stdout, MSG_WARN_CMP_FLOAT_COMP, line_num+1);

        exec_mod2(et1);
        oper_mult(et2,et2);
        oper_cmp (2*OFST,2*OFST,type);
    }

    // comp var com float acc
    if ((get_type(et1)==3) && (et1%OFST!=0) && (get_type(et2)==2) && (et2%OFST==0))
    {
        fprintf(stdout, MSG_WARN_CMP_FLOAT_COMP, line_num+1);

        add_instr("SET   aux_var\n");
        acc_ok = 0;
        exec_mod2(et1);
        add_instr("P_LOD aux_var\n");
        add_instr("P_LOD aux_var\n");
        oper_mult(et2,et2);
        oper_cmp (2*OFST,2*OFST,type);
    }

    // comp var com comp const
    if ((get_type(et1)==3) && (et1%OFST!=0) && (get_type(et2)==5))
    {
        fprintf(stdout, MSG_WARN_CMP_COMPLEX, line_num+1);

        exec_mod2(et1);
        exec_mod2(et2);
        oper_cmp (2*OFST,2*OFST,type);
    }

    // comp var com comp var
    if ((get_type(et1)==3) && (et1%OFST!=0) && (get_type(et2)==3) && (et2%OFST!=0))
    {
        fprintf(stdout, MSG_WARN_CMP_COMPLEX, line_num+1);

        exec_mod2(et1);
        exec_mod2(et2);
        oper_cmp (2*OFST,2*OFST,type);
    }

    // comp var com comp acc
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
        oper_cmp (2*OFST,2*OFST,type);
    }

    // comp acc com int var
    if ((get_type(et1)==3) && (et1%OFST==0) && (get_type(et2)==1) && (et2%OFST!=0))
    {
        fprintf(stdout, MSG_WARN_CMP_COMPLEX, line_num+1);

        exec_mod2(et1);
        oper_mult(et2,et2);
        oper_cmp (2*OFST,OFST,type);
    }

    // comp acc com int acc
    if ((get_type(et1)==3) && (et1%OFST==0) && (get_type(et2)==1) && (et2%OFST==0))
    {
        fprintf(stdout, MSG_WARN_CMP_COMPLEX, line_num+1);

        add_instr("SET_P aux_var\n");
        exec_mod2(et1);
        add_instr("P_LOD aux_var\n");
        add_instr("P_LOD aux_var\n");
        oper_mult(et2,et2);
        oper_cmp (2*OFST,OFST,type);
    }

    // comp acc com float var
    if ((get_type(et1)==3) && (et1%OFST==0) && (get_type(et2)==2) && (et2%OFST!=0))
    {
        fprintf(stdout, MSG_WARN_CMP_COMPLEX, line_num+1);

        exec_mod2(et1);
        oper_mult(et2,et2);
        oper_cmp (2*OFST,2*OFST,type);
    }

    // comp acc com float acc
    if ((get_type(et1)==3) && (et1%OFST==0) && (get_type(et2)==2) && (et2%OFST==0))
    {
        fprintf(stdout, MSG_WARN_CMP_COMPLEX, line_num+1);

        add_instr("SET_P aux_var\n");
        exec_mod2(et1);
        add_instr("P_LOD aux_var\n");
        add_instr("P_LOD aux_var\n");
        oper_mult(et2,et2);
        oper_cmp (2*OFST,2*OFST,type);
    }

    // comp acc com comp const
    if ((get_type(et1)==3) && (et1%OFST==0) && (get_type(et2)==5))
    {
        fprintf(stdout, MSG_WARN_CMP_COMPLEX, line_num+1);

        exec_mod2(et1);
        exec_mod2(et2);
        oper_cmp (2*OFST,2*OFST,type);
    }

    // comp acc com comp var
    if ((get_type(et1)==3) && (et1%OFST==0) && (get_type(et2)==3) && (et2%OFST!=0))
    {
        fprintf(stdout, MSG_WARN_CMP_COMPLEX, line_num+1);

        exec_mod2(et1);
        exec_mod2(et2);
        oper_cmp (2*OFST,2*OFST,type);
    }
    
    // comp acc com comp acc
    if ((get_type(et1)==3) && (et1%OFST==0) && (get_type(et2)==3) && (et2%OFST==0))
    {
        fprintf(stdout, MSG_WARN_CMP_COMPLEX, line_num+1);

        add_instr("SET_P aux_var \n");
        add_instr("SET_P aux_var1\n");
        exec_mod2(et1);
        add_instr("P_LOD aux_var1\n");
        add_instr("P_LOD aux_var \n");
        exec_mod2(et2);
        oper_cmp (2*OFST,2*OFST,type);
    }

    acc_ok = 1;

    return OFST;
}

// compara maior e igual (contrario de menor)
int oper_greq(int et1, int et2)
{
    return oper_lin(oper_cmp(et1,et2,0));
}

// compara menor e igual (contrasrio de maior)
int oper_leeq(int et1, int et2)
{
    return oper_lin(oper_cmp(et1,et2,1));
}

// comparar se eh diferente (contrario de igual)
int oper_dife(int et1, int et2)
{
    return oper_lin(oper_cmp(et1,et2,2));
}

// ----------------------------------------------------------------------------
// operacoes condicionais (usadas em if else while) ---------------------------
// ----------------------------------------------------------------------------

// inversao logica (!)
int oper_lin(int et)
{
    int etr, eti;

    char ld [10]; if (acc_ok == 0) strcpy(ld ,"LOD"  ); else strcpy(ld ,"P_LOD"  );
    char f2i[10]; if (acc_ok == 0) strcpy(f2i,"F2I_M"); else strcpy(f2i,"P_F2I_M");
    char lin[10]; if (acc_ok == 0) strcpy(lin,"LIN_M"); else strcpy(lin,"P_LIN_M");

    // se for um int na memoria
    if ((get_type(et) == 1) && (et % OFST != 0))
    {
        add_instr("%s %s\n", lin, v_name[et%OFST]);
    }

    // se for um int no acc
    if ((get_type(et) == 1) && (et % OFST == 0))
    {
        add_instr("LIN\n");
    }

    // se for um float var na memoria
    if ((get_type(et) == 2) && (et % OFST != 0))
    {
        fprintf(stdout, MSG_WARN_LOGIC_FLOAT, line_num+1);

        add_instr("%s %s\n", f2i, v_name[et%OFST]);
        add_instr("LIN\n");
    }

    // se for um float no acc
    if ((get_type(et) == 2) && (et % OFST == 0))
    {
        fprintf(stdout, MSG_WARN_LOGIC_FLOAT, line_num+1);

        add_instr("F2I\n");
        add_instr("LIN\n");
    }

    // se for um comp const
    if (get_type(et) == 5)
    {
        fprintf(stdout, MSG_WARN_LOGIC_COMP, line_num+1);

        get_cmp_cst(et,&etr,&eti);

        add_instr("%s %s\n", f2i, v_name[etr%OFST]);
        add_instr("LIN\n");
    }

    // se for um comp var
    if ((get_type(et) == 3) && (et % OFST != 0))
    {
        fprintf(stdout, MSG_WARN_LOGIC_COMP, line_num+1);

        add_instr("%s %s\n", f2i, v_name[et%OFST]);
        add_instr("LIN\n");
    }

    // se for um comp no acc
    if ((get_type(et) == 3) && (et % OFST == 0))
    {
        fprintf(stdout, MSG_WARN_LOGIC_COMP, line_num+1);

        add_instr("POP\n");
        add_instr("F2I\n");
        add_instr("LIN\n");
    }

    acc_ok = 1;

    return OFST; // retorna o id extendido de int
}

// and ou logicos (&& ||)
int oper_lanor(int et1, int et2, int type)
{
    if ((get_type(et1) > 1) || (get_type(et2) > 1))
    {
        fprintf(stderr, MSG_ERR_LOGIC_NON_INT, line_num+1);
        exit(EXIT_FAILURE);
    }

    int etr, eti;

    char ld[10];
    if (acc_ok == 0) strcpy(ld,"LOD"); else strcpy(ld,"P_LOD");

    char op[16];
    switch (type)
    {
        case 0: strcpy(op, "LAN"); break;
        case 1: strcpy(op, "LOR" ); break;
    }

    // int var com int var
    if ((get_type(et1)==1) && (et1%OFST!=0) && (get_type(et2)==1) && (et2%OFST!=0))
    {
        add_instr("%s %s\n", ld, v_name[et2%OFST]);
        add_instr("%s %s\n", op, v_name[et1%OFST]);
    }

    // int var com int acc
    if ((get_type(et1)==1) && (et1%OFST!=0) && (get_type(et2)==1) && (et2%OFST==0))
    {
        add_instr("%s %s\n", op, v_name[et1%OFST]);
    }

    // int acc com int var
    if ((get_type(et1)==1) && (et1%OFST==0) && (get_type(et2)==1) && (et2%OFST!=0))
    {
        add_instr("%s %s\n", op, v_name[et2%OFST]);
    }

    // int acc com int acc
    if ((get_type(et1)==1) && (et1%OFST==0) && (get_type(et2)==1) && (et2%OFST==0))
    {
        add_instr("S_%s\n", op);
    }

    acc_ok = 1;

    return OFST;
}

// ----------------------------------------------------------------------------
// operacao com portas logicas bitwise (~ & |) --------------------------------
// ----------------------------------------------------------------------------

// porta inversora
int oper_inv(int et)
{
    if (get_type(et) > 1)
        {fprintf(stderr, MSG_ERR_INV_NON_INT, line_num+1); exit(EXIT_FAILURE);}

    int etr, eti;

    char inv[10]; if (acc_ok == 0) strcpy(inv,"INV_M"); else strcpy(inv,"P_INV_M");

    // se for um int na memoria
    if ((get_type(et) == 1) && (et % OFST != 0))
    {
        add_instr("%s %s\n", inv, v_name[et%OFST]);
    }

    // se for um int no acc
    if ((get_type(et) == 1) && (et % OFST == 0))
    {
        add_instr("INV\n");
    }

    acc_ok = 1;

    return OFST; // retorna o id extendido de int
}

// portas logicas de duas entradas (& | ^)
int oper_bitw(int et1, int et2, int type)
{
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

    // var com var
    if ((et1%OFST != 0) && (et2%OFST != 0))
    {
        add_instr("%s %s\n", ld, v_name[et2%OFST]);
        add_instr("%s %s\n", op, v_name[et1%OFST]);
    }

    // var com acc
    if ((et1%OFST != 0) && (et2%OFST == 0))
    {
        add_instr("%s %s\n", op, v_name[et1%OFST]);
    }

    // acc com var
    if ((et1%OFST == 0) && (et2%OFST != 0))
    {
        add_instr("P_LOD %s\n", v_name[et2%OFST]);
        add_instr("S_%s\n", op);
    }

    // acc com acc
    if ((et1%OFST == 0) && (et2%OFST == 0))
    {
        add_instr("S_%s\n", op);
    }

    acc_ok = 1;

    return OFST; // retorna o id extendido de int
}

// ----------------------------------------------------------------------------
// operacoes de deslocamento de bits ------------------------------------------
// ----------------------------------------------------------------------------

int oper_shift(int et1, int et2, int type)
{
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

    int etr, eti;

    char ld[10]; if (acc_ok == 0) strcpy(ld,"LOD"); else strcpy(ld,"P_LOD");

    // int/float var com int var
    if ((et1%OFST != 0) && (get_type(et2) == 1) && (et2%OFST != 0))
    {
        add_instr("%s %s\n", ld, v_name[et2%OFST]);
        add_instr("%s %s\n", op, v_name[et1%OFST]);
    }

    // int/float var com int acc
    if ((et1%OFST != 0) && (get_type(et2) == 1) && (et2%OFST == 0))
    {
        add_instr("%s %s\n", op, v_name[et1%OFST]);
    }

    // int/float var com float var
    if ((et1%OFST != 0) && (get_type(et2) == 2) && (et2%OFST != 0))
    {
        fprintf(stdout, MSG_WARN_SHIFT_BY_FLOAT, line_num+1);

        add_instr("%s %s\n", ld, v_name[et2%OFST]);
        add_instr("F2I\n");
        add_instr("%s %s\n", op, v_name[et1%OFST]);
    }

    // int/float var com float acc
    if ((et1%OFST != 0) && (get_type(et2) == 2) && (et2%OFST == 0))
    {
        fprintf(stdout, MSG_WARN_SHIFT_BY_FLOAT, line_num+1);

        add_instr("F2I\n");
        add_instr("%s %s\n", op, v_name[et1%OFST]);
    }

    // int/float acc com int var
    if ((et1%OFST == 0) && (get_type(et2) == 1) && (et2%OFST != 0))
    {
        add_instr("P_LOD %s\n", v_name[et2%OFST]);
        add_instr("S_%s\n", op);
    }

    // int/float acc com int acc
    if ((et1%OFST == 0) && (get_type(et2) == 1) && (et2%OFST == 0))
    {
        add_instr("S_%s\n", op);
    }

    // int/float acc com float var
    if ((et1%OFST == 0) && (get_type(et2) == 2) && (et2%OFST != 0))
    {
        fprintf(stdout, MSG_WARN_SHIFT_BY_FLOAT, line_num+1);

        add_instr("P_LOD %s\n", v_name[et2%OFST]);
        add_instr("F2I\n");
        add_instr("S_%s\n", op);
    }

    // int/float acc com float acc
    if ((et1%OFST == 0) && (get_type(et2) == 2) && (et2%OFST == 0))
    {
        fprintf(stdout, MSG_WARN_SHIFT_BY_FLOAT, line_num+1);
        
        add_instr("F2I\n");
        add_instr("S_%s\n", op);
    }

    acc_ok = 1;

    return OFST; // retorna o id extendido de int
}