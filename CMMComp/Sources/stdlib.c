// ----------------------------------------------------------------------------
// biblioteca padrao do sapho -------------------------------------------------
// ----------------------------------------------------------------------------

// includes globais
#include <string.h>
#include <stdlib.h>

// includes locais
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
// entrada e saida ------------------------------------------------------------
// ----------------------------------------------------------------------------

// input ex: int x = in(0);
int exec_in(int id)
{
    if (atoi(v_name[id]) >= nuioin) {fprintf(stderr, MSG_ERR_NO_IN_PORT, line_num+1, v_name[id]); exit(EXIT_FAILURE);}

    if (acc_ok == 0) add_instr("INN %s\n", v_name[id]); else add_instr("P_INN %s\n", v_name[id]);

    acc_ok = 1;  // diz que o acc agora tem um valor carregado

    return OFST;
}

// input ex: float x = fin(0);
int exec_fin(int id)
{
    if (atoi(v_name[id]) >= nuioin) {fprintf(stderr, MSG_ERR_NO_IN_PORT, line_num+1, v_name[id]); exit(EXIT_FAILURE);}

    if (acc_ok == 0) add_instr("F_INN %s\n", v_name[id]); else add_instr("PF_INN %s\n", v_name[id]);

    acc_ok = 1;  // diz que o acc agora tem um valor carregado

    return 2*OFST;
}

// output ex: out(0,x);
void exec_out(int id, int et)
{
    // ------------------------------------------------------------------------
    // checa consistencia -----------------------------------------------------
    // ------------------------------------------------------------------------

    // checa se et foi declarada
    if (et%OFST != 0 && v_type[et%OFST] == 0) {fprintf(stderr, MSG_ERR_DECL_FIRST, line_num+1, rem_fname(v_name[et%OFST], fname)); exit(EXIT_FAILURE);}

    // checa se eh comp
    if (get_type(et) > 2) {fprintf (stderr, MSG_ERR_PICK_COMP_INFO, line_num+1); exit(EXIT_FAILURE);}

    // checa se et eh uma variavel
    if (et%OFST != 0 && v_isar[et%OFST] > 0) {fprintf(stderr, MSG_ERR_WRONG_USE, line_num+1, rem_fname(v_name[et%OFST], fname)); exit(EXIT_FAILURE);}

    // checa range de porta
    if (atoi(v_name[id]) >= nuioou) {fprintf(stderr, MSG_ERR_NO_OUT_PORT, line_num+1, v_name[id]); exit(EXIT_FAILURE);}

    // ------------------------------------------------------------------------
    // atualiza status das variaveis ------------------------------------------
    // ------------------------------------------------------------------------

    if (et%OFST != 0) v_used[et%OFST] = 1;

    // ------------------------------------------------------------------------
    // executa ----------------------------------------------------------------
    // ------------------------------------------------------------------------

    // int var
    if ((get_type(et) == 1) && (et%OFST!=0))
    {
       if (acc_ok == 0) add_instr("LOD %s\n", v_name[et%OFST]); else add_instr("P_LOD %s\n", v_name[et%OFST]);
    }

    // int acc
    if ((get_type(et) == 1) && (et%OFST==0))
    {
        // nao faz nada
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
    // checa consistencia -----------------------------------------------------
    // ------------------------------------------------------------------------

    // checa se et foi declarada
    if (et%OFST != 0 && v_type[et%OFST] == 0) {fprintf(stderr, MSG_ERR_DECL_FIRST, line_num+1, rem_fname(v_name[et%OFST], fname)); exit(EXIT_FAILURE);}

    // checa se eh comp
    if (get_type(et) > 2) {fprintf (stderr, MSG_ERR_PICK_COMP_INFO, line_num+1); exit(EXIT_FAILURE);}

    // checa se et eh uma variavel
    if (et%OFST != 0 && v_isar[et%OFST] > 0) {fprintf(stderr, MSG_ERR_WRONG_USE, line_num+1, rem_fname(v_name[et%OFST], fname)); exit(EXIT_FAILURE);}

    // checa range de porta
    if (atoi(v_name[id]) >= nuioou) {fprintf(stderr, MSG_ERR_NO_OUT_PORT, line_num+1, v_name[id]); exit(EXIT_FAILURE);}

    // ------------------------------------------------------------------------
    // atualiza status das variaveis ------------------------------------------
    // ------------------------------------------------------------------------

    if (et%OFST != 0) v_used[et%OFST] = 1;

    // ------------------------------------------------------------------------
    // executa ----------------------------------------------------------------
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
// funcoes especiais que evitam codigo ----------------------------------------
// ----------------------------------------------------------------------------

// pega o sinal do primeiro argumento e coloca no segundo
int exec_sign(int et1, int et2)
{
    // ------------------------------------------------------------------------
    // checa consistencia -----------------------------------------------------
    // ------------------------------------------------------------------------

    // checa se et1 foi declarada
    if (et1%OFST != 0 && v_type[et1%OFST] == 0) {fprintf(stderr, MSG_ERR_DECL_FIRST, line_num+1, rem_fname(v_name[et1%OFST], fname)); exit(EXIT_FAILURE);}
    
    // checa se et2 foi declarada
    if (et2%OFST != 0 && v_type[et2%OFST] == 0) {fprintf(stderr, MSG_ERR_DECL_FIRST, line_num+1, rem_fname(v_name[et2%OFST], fname)); exit(EXIT_FAILURE);}

    // checa se et1 eh uma variavel
    if (et1%OFST != 0 && v_isar[et1%OFST] > 0) {fprintf(stderr, MSG_ERR_WRONG_USE, line_num+1, rem_fname(v_name[et1%OFST], fname)); exit(EXIT_FAILURE);}

    // checa se et2 eh uma variavel
    if (et2%OFST != 0 && v_isar[et2%OFST] > 0) {fprintf(stderr, MSG_ERR_WRONG_USE, line_num+1, rem_fname(v_name[et2%OFST], fname)); exit(EXIT_FAILURE);}

    // checa se tem comp
    if ((get_type(et1) > 2) || (get_type(et2) > 2)) {fprintf (stderr, MSG_ERR_SIGN_COMPLEX, line_num+1); exit(EXIT_FAILURE);}

    // ------------------------------------------------------------------------
    // atualiza status das variaveis ------------------------------------------
    // ------------------------------------------------------------------------

    if (et1%OFST != 0) v_used[et1%OFST] = 1;
    if (et2%OFST != 0) v_used[et2%OFST] = 1;

    // ------------------------------------------------------------------------
    // prepara variaveis locais -----------------------------------------------
    // ------------------------------------------------------------------------

    char ld[10]; if (acc_ok == 0) strcpy(ld,"LOD"); else strcpy(ld,"P_LOD");

    // ------------------------------------------------------------------------
    // executa ----------------------------------------------------------------
    // ------------------------------------------------------------------------

    // int na memoria e int na memoria
    if ((get_type(et1) == 1) && (et1 % OFST != 0) && (get_type(et2) == 1) && (et2 % OFST != 0))
    {
        add_instr("%s %s\n", ld, v_name[et2%OFST]);
        add_instr("SGN %s\n"   , v_name[et1%OFST]);
    }

    // int na memoria e int no acc
    if ((get_type(et1) == 1) && (et1 % OFST != 0) && (get_type(et2) == 1) && (et2 % OFST == 0))
    {
        add_instr("SGN %s\n", v_name[et1%OFST]);
    }

    // int na memoria e float var na memoria
    if ((get_type(et1) == 1) && (et1 % OFST != 0) && (get_type(et2) == 2) && (et2 % OFST != 0))
    {
        add_instr("%s %s\n", ld, v_name[et2%OFST]);
        add_instr("F_SGN %s\n" , v_name[et1%OFST]);
    }

    // int na memoria e float no acc
    if ((get_type(et1) == 1) && (et1 % OFST != 0) && (get_type(et2) == 2) && (et2 % OFST == 0))
    {
        add_instr("F_SGN %s\n", v_name[et1%OFST]);
    }

    // int no acc e int na memoria
    if ((get_type(et1) == 1) && (et1 % OFST == 0) && (get_type(et2) == 1) && (et2 % OFST != 0))
    {
        add_instr("%s %s\n", ld, v_name[et2%OFST]);
        add_instr("S_SGN\n");
    }

    // int no acc e int no acc
    if ((get_type(et1) == 1) && (et1 % OFST == 0) && (get_type(et2) == 1) && (et2 % OFST == 0))
    {
        add_instr("S_SGN\n");
    }

    // int no acc e float var na memoria
    if ((get_type(et1) == 1) && (et1 % OFST == 0) && (get_type(et2) == 2) && (et2 % OFST != 0))
    {
        add_instr("P_LOD %s\n", v_name[et2%OFST]  );
        add_instr("SF_SGN\n");
    }

    // int no acc e float no acc
    if ((get_type(et1) == 1) && (et1 % OFST == 0) && (get_type(et2) == 2) && (et2 % OFST == 0))
    {
        add_instr("SF_SGN\n");
    }

    // float var e int var
    if ((get_type(et1) == 2) && (et1 % OFST != 0) && (get_type(et2) == 1) && (et2 % OFST != 0))
    {
        add_instr("%s %s\n", ld, v_name[et2%OFST]);
        add_instr("SGN %s\n"   , v_name[et1%OFST]);
    }

    // float var e int no acc
    if ((get_type(et1) == 2) && (et1 % OFST != 0) && (get_type(et2) == 1) && (et2 % OFST == 0))
    {
        add_instr("SGN %s\n"  , v_name[et1%OFST]);
    }

    // float var e float var
    if ((get_type(et1) == 2) && (et1 % OFST != 0) && (get_type(et2) == 2) && (et2 % OFST != 0))
    {
        add_instr("%s %s\n", ld, v_name[et2%OFST]);
        add_instr("F_SGN %s\n" , v_name[et1%OFST]);
    }

    // float var e float no acc
    if ((get_type(et1) == 2) && (et1 % OFST != 0) && (get_type(et2) == 2) && (et2 % OFST == 0))
    {
        add_instr("F_SGN %s\n" , v_name[et1%OFST]);
    }

    // float no acc e int na memoria
    if ((get_type(et1) == 2) && (et1 % OFST == 0) && (get_type(et2) == 1) && (et2 % OFST != 0))
    {
        add_instr("%s %s\n", ld, v_name[et2%OFST]);
        add_instr("S_SGN\n");
    }

    // float no acc e int no acc
    if ((get_type(et1) == 2) && (et1 % OFST == 0) && (get_type(et2) == 1) && (et2 % OFST == 0))
    {
        add_instr("S_SGN\n");
    }

    // float no acc e float var na memoria
    if ((get_type(et1) == 2) && (et1 % OFST == 0) && (get_type(et2) == 2) && (et2 % OFST != 0))
    {
        add_instr("P_LOD %s\n", v_name[et2%OFST]);
        add_instr("SF_SGN\n");
    }

    // float no acc e float no acc
    if ((get_type(et1) == 2) && (et1 % OFST == 0) && (get_type(et2) == 2) && (et2 % OFST == 0))
    {
        add_instr("SF_SGN\n");
    }

    acc_ok = 1;

    return get_type(et2)*OFST;
}

// valor absoluto (int, float e comp)
int exec_abs(int et)
{
    // ------------------------------------------------------------------------
    // checa consistencia -----------------------------------------------------
    // ------------------------------------------------------------------------

    // checa se et foi declarada
    if (et%OFST != 0 && v_type[et%OFST] == 0) {fprintf(stderr, MSG_ERR_DECL_FIRST, line_num+1, rem_fname(v_name[et%OFST], fname)); exit(EXIT_FAILURE);}

    // checa se et eh uma variavel
    if (et%OFST != 0 && v_isar[et%OFST] > 0) {fprintf(stderr, MSG_ERR_WRONG_USE, line_num+1, rem_fname(v_name[et%OFST], fname)); exit(EXIT_FAILURE);}

    // ------------------------------------------------------------------------
    // atualiza status das variaveis ------------------------------------------
    // ------------------------------------------------------------------------

    if (et%OFST != 0) v_used[et%OFST] = 1;

    // ------------------------------------------------------------------------
    // prepara variaveis locais -----------------------------------------------
    // ------------------------------------------------------------------------
    
    char  ld [10]; if (acc_ok == 0) strcpy(ld  , "LOD"   ); else strcpy(ld  , "P_LOD"  );
    char  abs[10]; if (acc_ok == 0) strcpy( abs, "ABS_M" ); else strcpy( abs, "P_ABS_M");
    char fabs[10]; if (acc_ok == 0) strcpy(fabs,"F_ABS_M"); else strcpy(fabs,"PF_ABS_M");

    // ------------------------------------------------------------------------
    // executa ----------------------------------------------------------------
    // ------------------------------------------------------------------------

    // int na memoria
    if ((get_type(et) == 1) && (et % OFST != 0))
    {
        add_instr("%s %s\n", abs, v_name[et%OFST]);
    }

    // int no acc
    if ((get_type(et) == 1) && (et % OFST == 0))
    {
        add_instr("ABS\n");
    }

    // float na memoria
    if ((get_type(et) == 2) && (et % OFST != 0))
    {
        add_instr("%s %s\n", fabs, v_name[et%OFST]);
    }

    // float no acc
    if ((get_type(et) == 2) && (et % OFST == 0))
    {
        add_instr("F_ABS\n");
    }

    // comp const, na memoria e no acc
    if ((get_type(et) == 3) || (get_type(et) == 5))
    {
        et = exec_sqrt(exec_mod2(et));
    }

    acc_ok = 1;

    return get_type(et)*OFST;
}

// zera se for negativo
int exec_pst(int et)
{
    // ------------------------------------------------------------------------
    // checa consistencia -----------------------------------------------------
    // ------------------------------------------------------------------------

    // checa se et foi declarada
    if (et%OFST != 0 && v_type[et%OFST] == 0) {fprintf(stderr, MSG_ERR_DECL_FIRST, line_num+1, rem_fname(v_name[et%OFST], fname)); exit(EXIT_FAILURE);}

    // checa se et eh uma variavel
    if (et%OFST != 0 && v_isar[et%OFST] > 0) {fprintf(stderr, MSG_ERR_WRONG_USE, line_num+1, rem_fname(v_name[et%OFST], fname)); exit(EXIT_FAILURE);}

    // ------------------------------------------------------------------------
    // atualiza status das variaveis ------------------------------------------
    // ------------------------------------------------------------------------

    if (et%OFST != 0) v_used[et%OFST] = 1;

    // ------------------------------------------------------------------------
    // prepara variaveis locais -----------------------------------------------
    // ------------------------------------------------------------------------

    char  ld [10]; if (acc_ok == 0) strcpy( ld , "LOD"   ); else strcpy( ld , "P_LOD"  );
    char  pst[10]; if (acc_ok == 0) strcpy( pst, "PST_M" ); else strcpy( pst, "P_PST_M");
    char fpst[10]; if (acc_ok == 0) strcpy(fpst,"F_PST_M"); else strcpy(fpst,"PF_PST_M");

    // ------------------------------------------------------------------------
    // executa ----------------------------------------------------------------
    // ------------------------------------------------------------------------

    // int na memoria
    if ((get_type(et) == 1) && (et % OFST != 0))
    {
        add_instr("%s %s\n", pst, v_name[et%OFST]);
    }

    // int no acc
    if ((get_type(et) == 1) && (et % OFST == 0))
    {
        add_instr("PST\n");
    }

    // float na memoria
    if ((get_type(et) == 2)  && (et % OFST != 0))
    {
        add_instr("%s %s\n", fpst, v_name[et%OFST]);
    }

    // float no acc
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

// divisao por constante
int exec_norm(int et)
{
    // ------------------------------------------------------------------------
    // checa consistencia -----------------------------------------------------
    // ------------------------------------------------------------------------

    // checa se et foi declarada
    if (et%OFST != 0 && v_type[et%OFST] == 0) {fprintf(stderr, MSG_ERR_DECL_FIRST, line_num+1, rem_fname(v_name[et%OFST], fname)); exit(EXIT_FAILURE);}

    // checa se et eh uma variavel
    if (et%OFST != 0 && v_isar[et%OFST] > 0) {fprintf(stderr, MSG_ERR_WRONG_USE, line_num+1, rem_fname(v_name[et%OFST], fname)); exit(EXIT_FAILURE);}

    // checa se eh int
    if (get_type(et) != 1) {fprintf (stderr, MSG_ERR_NORM_NON_INT, line_num+1); exit(EXIT_FAILURE);}

    // ------------------------------------------------------------------------
    // atualiza status das variaveis ------------------------------------------
    // ------------------------------------------------------------------------

    if (et%OFST != 0) v_used[et%OFST] = 1;

    // ------------------------------------------------------------------------
    // prepara variaveis locais -----------------------------------------------
    // ------------------------------------------------------------------------

    char nrm[10]; if (acc_ok == 0) strcpy(nrm,"NRM_M"); else strcpy(nrm,"P_NRM_M");

    // ------------------------------------------------------------------------
    // executa ----------------------------------------------------------------
    // ------------------------------------------------------------------------

    // int na memoria
    if ((get_type(et) == 1) && (et % OFST != 0))
    {
        add_instr("%s %s\n", nrm, v_name[et%OFST]);
    }

    // int no acc
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
    // checa consistencia -----------------------------------------------------
    // ------------------------------------------------------------------------

    // checa se et1 foi declarada
    if (et1%OFST != 0 && v_type[et1%OFST] == 0) {fprintf(stderr, MSG_ERR_DECL_FIRST, line_num+1, rem_fname(v_name[et1%OFST], fname)); exit(EXIT_FAILURE);}

    // checa se et1 eh uma variavel
    if (et1%OFST != 0 && v_isar[et1%OFST] > 0) {fprintf(stderr, MSG_ERR_WRONG_USE, line_num+1, rem_fname(v_name[et1%OFST], fname)); exit(EXIT_FAILURE);}

    // checa se id2 foi declarada
    if (v_type[id2] == 0) {fprintf(stderr, MSG_ERR_DECL_FIRST, line_num+1, rem_fname(v_name[id2], fname)); exit(EXIT_FAILURE);}

    // checa se id2 eh uma variavel
    if (v_isar[id2] > 0) {fprintf(stderr, MSG_ERR_WRONG_USE, line_num+1, rem_fname(v_name[id2], fname)); exit(EXIT_FAILURE);}

    // ------------------------------------------------------------------------
    // atualiza status das variaveis ------------------------------------------
    // ------------------------------------------------------------------------

    if (et1%OFST != 0) v_used[et1%OFST] = 1;

    // ------------------------------------------------------------------------
    // executa ----------------------------------------------------------------
    // ------------------------------------------------------------------------

    // de var para var
    if (et1%OFST != 0)
    {
        add_instr("LOD %s\n", v_name[et1%OFST]);
        add_instr("SET %s\n", v_name[id2]     );
    }

    // de acc para var
    if (et1%OFST == 0)
    {
        add_instr("SET %s\n", v_name[id2]     );
    }
}

// ----------------------------------------------------------------------------
// funcoes nao-lineares -------------------------------------------------------
// ----------------------------------------------------------------------------

// raiz quadrada
int exec_sqrt(int et)
{
    // ------------------------------------------------------------------------
    // checa consistencia -----------------------------------------------------
    // ------------------------------------------------------------------------

    // checa se et foi declarada
    if (et%OFST != 0 && v_type[et%OFST] == 0) {fprintf(stderr, MSG_ERR_DECL_FIRST, line_num+1, rem_fname(v_name[et%OFST], fname)); exit(EXIT_FAILURE);}

    // checa se et eh uma variavel
    if (et%OFST != 0 && v_isar[et%OFST] > 0) {fprintf(stderr, MSG_ERR_WRONG_USE, line_num+1, rem_fname(v_name[et%OFST], fname)); exit(EXIT_FAILURE);}

    // checa se eh comp
    if (get_type(et) > 2) {fprintf (stderr, MSG_ERR_SQRT_COMPLEX, line_num+1); exit(EXIT_FAILURE);}

    // ------------------------------------------------------------------------
    // atualiza status das variaveis ------------------------------------------
    // ------------------------------------------------------------------------

    if (et%OFST != 0) v_used[et%OFST] = 1;

    // ------------------------------------------------------------------------
    // prepara variaveis locais -----------------------------------------------
    // ------------------------------------------------------------------------

    char ld [10]; if (acc_ok == 0) strcpy(ld ,"LOD"   ); else strcpy(ld ,"P_LOD"  );
    char i2f[10]; if (acc_ok == 0) strcpy(i2f, "I2F_M"); else strcpy(i2f,"P_I2F_M");

    // ------------------------------------------------------------------------
    // executa ----------------------------------------------------------------
    // ------------------------------------------------------------------------

    // int na memoria
    if ((get_type(et) == 1) && (et % OFST != 0))
    {
        add_instr("%s %s\n", i2f, v_name[et%OFST]);
        add_instr("CAL float_sqrt\n");
    }

    // int no acc
    if ((get_type(et) == 1) && (et % OFST == 0))
    {
        add_instr("I2F\n");
        add_instr("CAL float_sqrt\n");
    }

    // float na memoria
    if ((get_type(et) == 2) && (et % OFST != 0))
    {
        add_instr("%s %s\n", ld, v_name[et%OFST]);
        add_instr("CAL float_sqrt\n");
    }

    // float no acc
    if ((get_type(et) == 2) && (et % OFST == 0))
    {
        add_instr("CAL float_sqrt\n");
    }

    acc_ok = 1;

    return 2*OFST;
}

// arco-tangente
int exec_atan(int et)
{
    // ------------------------------------------------------------------------
    // checa consistencia -----------------------------------------------------
    // ------------------------------------------------------------------------

    // checa se et foi declarada
    if (et%OFST != 0 && v_type[et%OFST] == 0) {fprintf(stderr, MSG_ERR_DECL_FIRST, line_num+1, rem_fname(v_name[et%OFST], fname)); exit(EXIT_FAILURE);}

    // checa se et eh uma variavel
    if (et%OFST != 0 && v_isar[et%OFST] > 0) {fprintf(stderr, MSG_ERR_WRONG_USE, line_num+1, rem_fname(v_name[et%OFST], fname)); exit(EXIT_FAILURE);}

    // checa se eh comp
    if (get_type(et) > 2) {fprintf (stderr, MSG_ERR_SQRT_COMPLEX, line_num+1); exit(EXIT_FAILURE);}

    // ------------------------------------------------------------------------
    // atualiza status das variaveis ------------------------------------------
    // ------------------------------------------------------------------------

    if (et%OFST != 0) v_used[et%OFST] = 1;

    // ------------------------------------------------------------------------
    // prepara variaveis locais -----------------------------------------------
    // ------------------------------------------------------------------------

    char ld [10]; if (acc_ok == 0) strcpy(ld ,"LOD"  ); else strcpy(ld ,"P_LOD"  );
    char i2f[10]; if (acc_ok == 0) strcpy(i2f,"I2F_M"); else strcpy(i2f,"P_I2F_M");

    // ------------------------------------------------------------------------
    // executa ----------------------------------------------------------------
    // ------------------------------------------------------------------------

    // int na memoria
    if ((get_type(et) == 1) && (et % OFST != 0))
    {
        add_instr("%s %s\n", i2f, v_name[et%OFST]);
        add_instr("CAL float_atan\n");
    }

    // int no acc
    if ((get_type(et) == 1) && (et % OFST == 0))
    {
        add_instr("I2F\n");
        add_instr("CAL float_atan\n");
    }

    // float na memoria
    if ((get_type(et) == 2) && (et % OFST != 0))
    {
        add_instr("%s %s\n", ld, v_name[et%OFST]);
        add_instr("CAL float_atan\n");
    }

    // float no acc
    if ((get_type(et) == 2) && (et % OFST == 0))
    {
        add_instr("CAL float_atan\n");
    }

    acc_ok = 1;

    return 2*OFST;
}

// seno
int exec_sin(int et)
{
    // ------------------------------------------------------------------------
    // checa consistencia -----------------------------------------------------
    // ------------------------------------------------------------------------

    // checa se et foi declarada
    if (et%OFST != 0 && v_type[et%OFST] == 0) {fprintf(stderr, MSG_ERR_DECL_FIRST, line_num+1, rem_fname(v_name[et%OFST], fname)); exit(EXIT_FAILURE);}

    // checa se et eh uma variavel
    if (et%OFST != 0 && v_isar[et%OFST] > 0) {fprintf(stderr, MSG_ERR_WRONG_USE, line_num+1, rem_fname(v_name[et%OFST], fname)); exit(EXIT_FAILURE);}

    // checa se eh comp
    if (get_type(et) > 2) {fprintf (stderr, MSG_ERR_SQRT_COMPLEX, line_num+1); exit(EXIT_FAILURE);}

    // ------------------------------------------------------------------------
    // atualiza status das variaveis ------------------------------------------
    // ------------------------------------------------------------------------

    if (et%OFST != 0) v_used[et%OFST] = 1;

    // ------------------------------------------------------------------------
    // prepara variaveis locais -----------------------------------------------
    // ------------------------------------------------------------------------

    char ld [10]; if (acc_ok == 0) strcpy(ld ,"LOD"  ); else strcpy(ld ,"P_LOD"  );
    char i2f[10]; if (acc_ok == 0) strcpy(i2f,"I2F_M"); else strcpy(i2f,"P_I2F_M");

    // ------------------------------------------------------------------------
    // executa ----------------------------------------------------------------
    // ------------------------------------------------------------------------

    // int na memoria
    if ((get_type(et) == 1) && (et % OFST != 0))
    {
        add_instr("%s %s\n", i2f, v_name[et%OFST]);
        add_instr("CAL float_sin\n");
    }

    // int no acc
    if ((get_type(et) == 1) && (et % OFST == 0))
    {
        add_instr("I2F\n");
        add_instr("CAL float_sin\n");
    }

    // float na memoria
    if ((get_type(et) == 2) && (et % OFST != 0))
    {
        add_instr("%s %s\n", ld, v_name[et%OFST]);
        add_instr("CAL float_sin\n");
    }

    // float no acc
    if ((get_type(et) == 2) && (et % OFST == 0))
    {
        add_instr("CAL float_sin\n");
    }

    acc_ok = 1;

    return 2*OFST;
}

// cosseno
int exec_cos(int et)
{
    // ------------------------------------------------------------------------
    // checa consistencia -----------------------------------------------------
    // ------------------------------------------------------------------------

    // checa se et foi declarada
    if (et%OFST != 0 && v_type[et%OFST] == 0) {fprintf(stderr, MSG_ERR_DECL_FIRST, line_num+1, rem_fname(v_name[et%OFST], fname)); exit(EXIT_FAILURE);}

    // checa se et eh uma variavel
    if (et%OFST != 0 && v_isar[et%OFST] > 0) {fprintf(stderr, MSG_ERR_WRONG_USE, line_num+1, rem_fname(v_name[et%OFST], fname)); exit(EXIT_FAILURE);}

    // checa se eh comp
    if (get_type(et) > 2) {fprintf (stderr, MSG_ERR_SQRT_COMPLEX, line_num+1); exit(EXIT_FAILURE);}

    // ------------------------------------------------------------------------
    // atualiza status das variaveis ------------------------------------------
    // ------------------------------------------------------------------------

    if (et%OFST != 0) v_used[et%OFST] = 1;

    // ------------------------------------------------------------------------
    // prepara variaveis locais -----------------------------------------------
    // ------------------------------------------------------------------------

    char ld [10]; if (acc_ok == 0) strcpy(ld ,"LOD"  ); else strcpy(ld ,"P_LOD"  );
    char i2f[10]; if (acc_ok == 0) strcpy(i2f,"I2F_M"); else strcpy(i2f,"P_I2F_M");

    // ------------------------------------------------------------------------
    // executa ----------------------------------------------------------------
    // ------------------------------------------------------------------------

    // int na memoria
    if ((get_type(et) == 1) && (et % OFST != 0))
    {
        add_instr("%s %s\n", i2f, v_name[et%OFST]);
        add_instr("F_NEG\n");
        add_instr("F_ADD 1.570796327");
        add_instr("CAL float_sin\n");
    }

    // int no acc
    if ((get_type(et) == 1) && (et % OFST == 0))
    {
        add_instr("I2F\n");
        add_instr("F_NEG\n");
        add_instr("F_ADD 1.570796327");
        add_instr("CAL float_sin\n");
    }

    // float na memoria
    if ((get_type(et) == 2) && (et % OFST != 0))
    {
        add_instr("%s %s\n", ld, v_name[et%OFST]);
        add_instr("F_NEG\n");
        add_instr("F_ADD 1.570796327");
        add_instr("CAL float_sin\n");
    }

    // float no acc
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
// funcoes especiais para numeros complexos -----------------------------------
// ----------------------------------------------------------------------------

// retorna a parte real de um comp
int exec_real(int et)
{
    // ------------------------------------------------------------------------
    // checa consistencia -----------------------------------------------------
    // ------------------------------------------------------------------------

    // checa se et foi declarada
    if (et%OFST != 0 && v_type[et%OFST] == 0) {fprintf(stderr, MSG_ERR_DECL_FIRST, line_num+1, rem_fname(v_name[et%OFST], fname)); exit(EXIT_FAILURE);}

    // checa se et eh uma variavel
    if (et%OFST != 0 && v_isar[et%OFST] > 0) {fprintf(stderr, MSG_ERR_WRONG_USE, line_num+1, rem_fname(v_name[et%OFST], fname)); exit(EXIT_FAILURE);}

    // checa se eh comp
    if (get_type(et) < 3) {fprintf (stderr, MSG_ERR_REAL_ARG_COMP, line_num+1); exit(EXIT_FAILURE);}

    // ------------------------------------------------------------------------
    // atualiza status das variaveis ------------------------------------------
    // ------------------------------------------------------------------------

    if (et%OFST != 0) v_used[et%OFST] = 1;

    // ------------------------------------------------------------------------
    // prepara variaveis locais -----------------------------------------------
    // ------------------------------------------------------------------------

    char ld[10]; if (acc_ok == 0) strcpy(ld,"LOD"); else strcpy(ld,"P_LOD");

    // ------------------------------------------------------------------------
    // executa ----------------------------------------------------------------
    // ------------------------------------------------------------------------

    // comp const
    if (get_type(et) == 5)
    {
        int et_r, et_i;
        get_cmp_cst(et,&et_r,&et_i);

        add_instr("%s %s\n", ld, v_name[et_r%OFST]);
    }

    // comp na memoria
    if ((get_type(et) == 3) && (et % OFST != 0))
    {
        add_instr("%s %s\n", ld, v_name[et % OFST]);
    }

    // comp no acc
    if ((get_type(et) == 3) && (et % OFST == 0))
    {
        add_instr("POP\n");
    }

    acc_ok = 1;
    
    return 2*OFST;
}

// retorna a parte imag de um comp
int exec_imag(int et)
{
    // ------------------------------------------------------------------------
    // checa consistencia -----------------------------------------------------
    // ------------------------------------------------------------------------

    // checa se et foi declarada
    if (et%OFST != 0 && v_type[et%OFST] == 0) {fprintf(stderr, MSG_ERR_DECL_FIRST, line_num+1, rem_fname(v_name[et%OFST], fname)); exit(EXIT_FAILURE);}

    // checa se et eh uma variavel
    if (et%OFST != 0 && v_isar[et%OFST] > 0) {fprintf(stderr, MSG_ERR_WRONG_USE, line_num+1, rem_fname(v_name[et%OFST], fname)); exit(EXIT_FAILURE);}

    // checa se eh comp
    if (get_type(et) < 3) {fprintf (stderr, MSG_ERR_IMAG_ARG_COMP, line_num+1); exit(EXIT_FAILURE);}

    // ------------------------------------------------------------------------
    // atualiza status das variaveis ------------------------------------------
    // ------------------------------------------------------------------------

    if (et%OFST != 0) v_used[et%OFST] = 1;

    // ------------------------------------------------------------------------
    // prepara variaveis locais -----------------------------------------------
    // ------------------------------------------------------------------------

    char ld[10]; if (acc_ok == 0) strcpy(ld,"LOD"); else strcpy(ld,"P_LOD");

    // ------------------------------------------------------------------------
    // executa ----------------------------------------------------------------
    // ------------------------------------------------------------------------

    // comp const
    if (get_type(et) == 5)
    {
        int et_r, et_i;
        get_cmp_cst(et,&et_r,&et_i);

        add_instr("%s %s\n", ld, v_name[et_i%OFST]);
    }

    // comp na memoria
    if ((get_type(et) == 3) && (et%OFST != 0))
    {
        int et_r, et_i;
        get_cmp_ets(et,&et_r,&et_i);

        add_instr("%s %s\n", ld, v_name[et_i%OFST]);
    }

    // comp no acc
    if ((get_type(et) == 3) && (et%OFST == 0))
    {
        add_instr("SET_P aux_var\n");
        add_instr("LOD   aux_var\n");
    }

    acc_ok = 1;

    return 2*OFST;
}

// modulo ao quadrado de um num complexo
int exec_mod2(int et)
{
    // ------------------------------------------------------------------------
    // checa consistencia -----------------------------------------------------
    // ------------------------------------------------------------------------

    // checa se et foi declarada
    if (et%OFST != 0 && v_type[et%OFST] == 0) {fprintf(stderr, MSG_ERR_DECL_FIRST, line_num+1, rem_fname(v_name[et%OFST], fname)); exit(EXIT_FAILURE);}

    // checa se et eh uma variavel
    if (et%OFST != 0 && v_isar[et%OFST] > 0) {fprintf(stderr, MSG_ERR_WRONG_USE, line_num+1, rem_fname(v_name[et%OFST], fname)); exit(EXIT_FAILURE);}

    // checa se eh comp
    if (get_type(et) < 3) {fprintf (stderr, MSG_ERR_MOD2_ARG_COMP, line_num+1); exit(EXIT_FAILURE);}

    // ------------------------------------------------------------------------
    // atualiza status das variaveis ------------------------------------------
    // ------------------------------------------------------------------------

    if (et%OFST != 0) v_used[et%OFST] = 1;

    // ------------------------------------------------------------------------
    // prepara variaveis locais -----------------------------------------------
    // ------------------------------------------------------------------------

    int etr, eti;

    // ------------------------------------------------------------------------
    // executa ----------------------------------------------------------------
    // ------------------------------------------------------------------------

    // se for uma constante ---------------------------------------------------
    if (get_type(et) == 5)
    {
        get_cmp_cst(et ,&etr,&eti);     // pega o et de cada constante float
        etr  = oper_mult(etr, etr);     // parte real ao quadrado
        eti  = oper_mult(eti, eti);     // parte imag ao quadrado
        etr  = oper_soma(etr, eti);     // soma os quadrados
    }

    // se estiver na memoria --------------------------------------------------
    if ((get_type(et) == 3) && (et % OFST != 0))
    {
        get_cmp_ets(et ,&etr,&eti);     // pega o et de cada constante float
        etr  = oper_mult(etr, etr);     // parte real ao quadrado
        eti  = oper_mult(eti, eti);     // parte imag ao quadrado
        etr  = oper_soma(etr, eti);     // soma os quadrados
    }

    // se estiver no acumulador -----------------------------------------------
    if ((get_type(et) == 3) && (et % OFST == 0))
    {
        add_instr("PSH\n");             // parte imag fica no acc e pilha
        oper_mult(2*OFST,2*OFST );      // multiplica acc com pilha
        add_instr("SET_P aux_var\n");   // salva temp e pega parte real

        add_instr("PSH\n");             // parte real fica no acc e pilha
        oper_mult(2*OFST,2*OFST );      // multiplica acc com pilha
        add_instr("P_LOD aux_var\n");   // xuxa o quadr do real pra pilha e pega o quadr do imag

        oper_soma(2*OFST,2*OFST);       // soma os quadrados

        etr = 2*OFST;                   // saida tem q ser et estendido pra float no acc
    }

    return 2*OFST;
}

// calcula a fase (em radianos) de um num complexo
int exec_fase(int et)
{
    // ------------------------------------------------------------------------
    // checa consistencia -----------------------------------------------------
    // ------------------------------------------------------------------------

    // checa se et foi declarada
    if (et%OFST != 0 && v_type[et%OFST] == 0) {fprintf(stderr, MSG_ERR_DECL_FIRST, line_num+1, rem_fname(v_name[et%OFST], fname)); exit(EXIT_FAILURE);}

    // checa se et eh uma variavel
    if (et%OFST != 0 && v_isar[et%OFST] > 0) {fprintf(stderr, MSG_ERR_WRONG_USE, line_num+1, rem_fname(v_name[et%OFST], fname)); exit(EXIT_FAILURE);}

    // checa se eh comp
    if (get_type(et) < 3) {fprintf (stderr, MSG_ERR_FASE_ARG_COMP, line_num+1); exit(EXIT_FAILURE);}

    // ------------------------------------------------------------------------
    // atualiza status das variaveis ------------------------------------------
    // ------------------------------------------------------------------------

    if (et%OFST != 0) v_used[et%OFST] = 1;

    // ------------------------------------------------------------------------
    // prepara variaveis locais -----------------------------------------------
    // ------------------------------------------------------------------------
    
    int et_r, et_i;

    // ------------------------------------------------------------------------
    // executa ----------------------------------------------------------------
    // ------------------------------------------------------------------------

    // comp const
    if (get_type(et) == 5)
    {
        get_cmp_cst(et,&et_i,&et_r);
        oper_divi  (et_r,et_i);
    }

    // comp na memoria
    if ((get_type(et) == 3) && (et%OFST != 0))
    {
        get_cmp_ets(et,&et_i,&et_r);
        oper_divi  (et_r,et_i);
    }

    // comp no acc
    if ((get_type(et) == 3) && (et%OFST == 0))
    {
        int id = exec_id("aux_var");
        et_i   = 2*OFST + id;

        add_instr("SET_P %s\n", v_name[id]);
        oper_divi(et_i,2*OFST);
    }

    exec_atan(2*OFST);

    acc_ok = 1;
    return 2*OFST;
}

// junta doi numeros reais pra fazer um complexo
int exec_comp(int etr, int eti)
{
    // ------------------------------------------------------------------------
    // checa consistencia -----------------------------------------------------
    // ------------------------------------------------------------------------

    // checa se etr foi declarada
    if (etr%OFST != 0 && v_type[etr%OFST] == 0) {fprintf(stderr, MSG_ERR_DECL_FIRST, line_num+1, rem_fname(v_name[etr%OFST], fname)); exit(EXIT_FAILURE);}

    // checa se eti foi declarada
    if (eti%OFST != 0 && v_type[eti%OFST] == 0) {fprintf(stderr, MSG_ERR_DECL_FIRST, line_num+1, rem_fname(v_name[eti%OFST], fname)); exit(EXIT_FAILURE);}

    // checa se etr eh uma variavel
    if (etr%OFST != 0 && v_isar[etr%OFST] > 0) {fprintf(stderr, MSG_ERR_WRONG_USE, line_num+1, rem_fname(v_name[etr%OFST], fname)); exit(EXIT_FAILURE);}

    // checa se et eh uma variavel
    if (eti%OFST != 0 && v_isar[eti%OFST] > 0) {fprintf(stderr, MSG_ERR_WRONG_USE, line_num+1, rem_fname(v_name[eti%OFST], fname)); exit(EXIT_FAILURE);}

    // checa se eh comp
    if (get_type(etr) > 2 || get_type(eti) > 2) {fprintf (stderr, MSG_ERR_COMPLEX_OF_COMPLEX, line_num+1); exit(EXIT_FAILURE);}

    // ------------------------------------------------------------------------
    // atualiza status das variaveis ------------------------------------------
    // ------------------------------------------------------------------------

    if (etr%OFST != 0) v_used[etr%OFST] = 1;
    if (eti%OFST != 0) v_used[eti%OFST] = 1;

    // ------------------------------------------------------------------------
    // prepara variaveis locais -----------------------------------------------
    // ------------------------------------------------------------------------

    char ld [10]; if (acc_ok == 0) strcpy(ld ,"LOD"  ); else strcpy(ld ,"P_LOD"  );
    char i2f[10]; if (acc_ok == 0) strcpy(i2f,"I2F_M"); else strcpy(i2f,"P_I2F_M");

    // ------------------------------------------------------------------------
    // executa ----------------------------------------------------------------
    // ------------------------------------------------------------------------

    // int na memoria e int na memoria
    if ((get_type(etr) == 1) && (etr % OFST != 0) && (get_type(eti) == 1) && (eti % OFST != 0))
    {
        add_instr("%s %s\n", i2f, v_name[etr%OFST]);
        add_instr("P_I2F_M %s\n", v_name[eti%OFST]);
    }

    // int na memoria e int no acc
    if ((get_type(etr) == 1) && (etr % OFST != 0) && (get_type(eti) == 1) && (eti % OFST == 0))
    {
        add_instr("SET aux_var\n");
        add_instr("I2F_M %s\n", v_name[etr%OFST]);
        add_instr("P_I2F_M aux_var\n");
    }

    // int na memoria e float na memoria
    if ((get_type(etr) == 1) && (etr % OFST != 0) && (get_type(eti) == 2) && (eti % OFST != 0))
    {
        add_instr("%s %s\n", i2f, v_name[etr%OFST]);
        add_instr("P_LOD %s\n"  , v_name[eti%OFST]);
    }

    // int na memoria e float no acc
    if ((get_type(etr) == 1) && (etr % OFST != 0) && (get_type(eti) == 2) && (eti % OFST == 0))
    {
        add_instr("SET aux_var\n");
        add_instr("I2F_M %s\n", v_name[etr%OFST]);
        add_instr("P_LOD aux_var\n");
    }

    // int no acc e int na memoria
    if ((get_type(etr) == 1) && (etr % OFST == 0) && (get_type(eti) == 1) && (eti % OFST != 0))
    {
        add_instr("I2F\n");
        add_instr("P_I2F_M %s\n", v_name[eti%OFST]);
    }

    // int no acc e int no acc
    if ((get_type(etr) == 1) && (etr % OFST == 0) && (get_type(eti) == 1) && (eti % OFST == 0))
    {
        add_instr("SET_P aux_var\n");
        add_instr("I2F\n");
        add_instr("P_I2F_M aux_var\n");
    }

    // int no acc e float na memoria
    if ((get_type(etr) == 1) && (etr % OFST == 0) && (get_type(eti) == 2) && (eti % OFST != 0))
    {
        add_instr("I2F\n");
        add_instr("P_LOD %s\n", v_name[eti%OFST]);
    }

    // int no acc e float no acc
    if ((get_type(etr) == 1) && (etr % OFST == 0) && (get_type(eti) == 2) && (eti % OFST == 0))
    {
        add_instr("SET_P aux_var\n");
        add_instr("I2F\n");
        add_instr("P_LOD aux_var\n");
    }

    // float na memoria e int na memoria
    if ((get_type(etr) == 2) && (etr % OFST != 0) && (get_type(eti) == 1) && (eti % OFST != 0))
    {
        add_instr("%s %s\n",  ld, v_name[etr%OFST]);
        add_instr("P_I2F_M %s\n", v_name[eti%OFST]);
    }

    // float na memoria e int no acc
    if ((get_type(etr) == 2) && (etr % OFST != 0) && (get_type(eti) == 1) && (eti % OFST == 0))
    {
        add_instr("SET aux_var\n");
        add_instr("LOD %s\n", v_name[etr%OFST]);
        add_instr("P_I2F_M aux_var\n");
    }

    // float na memoria e float na memoria
    if ((get_type(etr) == 2) && (etr % OFST != 0) && (get_type(eti) == 2) && (eti % OFST != 0))
    {
        add_instr("%s %s\n", ld, v_name[etr%OFST]);
        add_instr("P_LOD %s\n" , v_name[eti%OFST]);
    }

    // float na memoria e float no acc
    if ((get_type(etr) == 2) && (etr % OFST != 0) && (get_type(eti) == 2) && (eti % OFST == 0))
    {
        add_instr("SET aux_var\n");
        add_instr("LOD %s\n", v_name[etr%OFST]);
        add_instr("P_LOD aux_var\n");
    }

    // float no acc e int na memoria
    if ((get_type(etr) == 2) && (etr % OFST == 0) && (get_type(eti) == 1) && (eti % OFST != 0))
    {
        add_instr("P_I2F_M %s\n", v_name[eti%OFST]);
    }

    // float no acc e int no acc
    if ((get_type(etr) == 2) && (etr % OFST == 0) && (get_type(eti) == 1) && (eti % OFST == 0))
    {
        add_instr("I2F\n");
    }

    // float no acc e float na memoria
    if ((get_type(etr) == 2) && (etr % OFST == 0) && (get_type(eti) == 2) && (eti % OFST != 0))
    {
        add_instr("P_LOD %s\n", v_name[eti%OFST]);
    }

    // float no acc e float no acc
    if ((get_type(etr) == 1) && (etr % OFST == 0) && (get_type(eti) == 2) && (eti % OFST == 0))
    {
        // nao faz nada
    }

    acc_ok = 1;
    return 3*OFST;
}

// ----------------------------------------------------------------------------
// funcoes especiais para trabalho com vetores --------------------------------
// nao cria funcoes no stdlib diretamente -------------------------------------
// ao inves disso, usa notacao de Dirac nos statments -------------------------
// ----------------------------------------------------------------------------

// multiplicacao entre dois vetores, ex: <a|b>
// essa rotina gera um exp
int exec_vtv(int id1, int id2)
{
    // ------------------------------------------------------------------------
    // checa consistencia -----------------------------------------------------
    // ------------------------------------------------------------------------

    // checa se id1 foi declarado
    if (v_type[id1] == 0) {fprintf(stderr, MSG_ERR_VAR_NOT_FOUND, line_num+1, rem_fname(v_name[id1], fname)); exit(EXIT_FAILURE);}

    // checa se id2 foi declarado
    if (v_type[id2] == 0) {fprintf(stderr, MSG_ERR_VAR_NOT_FOUND, line_num+1, rem_fname(v_name[id2], fname)); exit(EXIT_FAILURE);}

    // checa se sao vetores mesmo
    if (v_isar[id1] != 1 || v_isar[id2] != 1) {fprintf(stderr, MSG_ERR_INNER_NEEDS_VECTORS, line_num+1); exit(EXIT_FAILURE);}

    // checa se tamanhos sao iguais
    if (v_size[id1] != v_size[id2]) {fprintf(stderr, MSG_ERR_VECTOR_SIZE_DIFF, line_num+1); exit(EXIT_FAILURE);}

    // checa se sao do mesmo tipo
    if (v_type[id1] != v_type[id2]) {fprintf(stderr, MSG_ERR_TYPE_DIFF, line_num+1); exit(EXIT_FAILURE);}

    // checa se tem variavel tipo comp
    if (v_type[id1] == 3 || v_type[id2] == 3) {fprintf(stderr, MSG_ERR_NOT_IMPL_COMPLEX, line_num+1); exit(EXIT_FAILURE);}

    // ------------------------------------------------------------------------
    // atualiza status das variaveis ------------------------------------------
    // ------------------------------------------------------------------------

    v_used[id1] = 1;
    v_used[id2] = 1;

    // ------------------------------------------------------------------------
    // prepara variaveis locais -----------------------------------------------
    // ------------------------------------------------------------------------

    int N = v_size[id1];

    char ld[10]; if (acc_ok == 0) strcpy(ld,"LOD"); else strcpy(ld,"P_LOD");

    // ------------------------------------------------------------------------
    // implementa o produto entre vetores -------------------------------------
    // ------------------------------------------------------------------------

    printf(MSG_INFO_DIRAC_INNER, line_num+1);

    // implementar para todas as combinacoes

    // int com int
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

    // float com float
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

// multiplicacao de matriz por vetor, ex: A # |B|b>;
void exec_Mv(int idy, int idM, int idv)
{
    // ------------------------------------------------------------------------
    // checa consistencia -----------------------------------------------------
    // ------------------------------------------------------------------------

    // checa se idy foi declarada
    if (v_type[idy] == 0) {fprintf(stderr, MSG_ERR_DECL_FIRST, line_num+1, rem_fname(v_name[idy], fname)); exit(EXIT_FAILURE);}

    // checa se idM foi declarada
    if (v_type[idM] == 0) {fprintf(stderr, MSG_ERR_DECL_FIRST, line_num+1, rem_fname(v_name[idM], fname)); exit(EXIT_FAILURE);}

    // checa se idv foi declarada
    if (v_type[idv] == 0) {fprintf(stderr, MSG_ERR_DECL_FIRST, line_num+1, rem_fname(v_name[idv], fname)); exit(EXIT_FAILURE);}

    // checa se os tipos sao os mesmos
    if (v_type[idy] != v_type[idM] || v_type[idy] != v_type[idv]) {fprintf(stderr, MSG_ERR_VARS_SAME_TYPE, line_num+1); exit(EXIT_FAILURE);}

    // checa se nao eh comp
    if (v_type[idy] == 3 || v_type[idM] == 3 || v_type[idv] == 3) {fprintf(stderr, MSG_ERR_NOT_IMPL_COMPLEX, line_num+1); exit(EXIT_FAILURE);}

    // checa se idy eh um vetor
    if (v_isar[idy] != 1) {fprintf(stderr, MSG_ERR_NOT_A_VECTOR, line_num+1, rem_fname(v_name[idy], fname)); exit(EXIT_FAILURE);}

    // checa se idM eh uma matriz
    if (v_isar[idM] != 2) {fprintf(stderr, MSG_ERR_NOT_A_MATRIX2, line_num+1, rem_fname(v_name[idM], fname)); exit(EXIT_FAILURE);}

    // checa se idv eh um vetor
    if (v_isar[idv] != 1) {fprintf(stderr, MSG_ERR_NOT_A_VECTOR, line_num+1, rem_fname(v_name[idv], fname)); exit(EXIT_FAILURE);}

    // checa tamanho entre saida e matriz
    if (v_size[idy] != v_size[idM]) {fprintf(stderr, MSG_ERR_MATRIX_ROW_MISMATCH, line_num+1, rem_fname(v_name[idM], fname), rem_fname(v_name[idy], fname)); exit(EXIT_FAILURE);}

    // checa tamanho entre matriz e vetor
    if (v_size[idv] != v_siz2[idM]) {fprintf(stderr, MSG_ERR_MATRIX_COL_MISMATCH, line_num+1, rem_fname(v_name[idM], fname), rem_fname(v_name[idv], fname)); exit(EXIT_FAILURE);}

    // ------------------------------------------------------------------------
    // atualiza status das variaveis ------------------------------------------
    // ------------------------------------------------------------------------

    v_used[idM] = 1;
    v_used[idv] = 1;

    // ------------------------------------------------------------------------
    // prepara variaveis locais -----------------------------------------------
    // ------------------------------------------------------------------------

    int N = v_size[idM];
    int M = v_siz2[idM];

    // ------------------------------------------------------------------------
    // implementa o produto entre matriz e vetor ------------------------------
    // ------------------------------------------------------------------------

    printf(MSG_INFO_DIRAC_MV, line_num+1);

    // implementar combinacoes sob demanda apenas

    // int com int
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

    // float com float
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

// multiplicacao de constante por vetor, ex: a # c|b>;
void exec_cv(int idy, int et, int idv)
{
    // ------------------------------------------------------------------------
    // checa consistencia -----------------------------------------------------
    // ------------------------------------------------------------------------

    // checa se idy foi declarada
    if (v_type[idy] == 0) {fprintf(stderr, MSG_ERR_DECL_FIRST, line_num+1, rem_fname(v_name[idy], fname)); exit(EXIT_FAILURE);}

    // checa se et foi declarada
    if (et%OFST != 0 && v_type[et%OFST] == 0) {fprintf(stderr, MSG_ERR_DECL_FIRST, line_num+1, rem_fname(v_name[et%OFST], fname)); exit(EXIT_FAILURE);}

    // checa se idv foi declarada
    if (v_type[idv] == 0) {fprintf(stderr, MSG_ERR_DECL_FIRST, line_num+1, rem_fname(v_name[idv], fname)); exit(EXIT_FAILURE);}

    // checa se os tipos sao os mesmos
    if (v_type[idy] != get_type(et) || v_type[idy] != v_type[idv]) {fprintf(stderr, MSG_ERR_VARS_SAME_TYPE, line_num+1); exit(EXIT_FAILURE);}

    // checa se nao eh comp
    if (v_type[idy] == 3 || get_type(et) == 3 || v_type[idv] == 3) {fprintf(stderr, MSG_ERR_NOT_IMPL_COMPLEX, line_num+1); exit(EXIT_FAILURE);}

    // checa se idy eh um vetor
    if (v_isar[idy] != 1) {fprintf(stderr, MSG_ERR_NOT_A_VECTOR, line_num+1, rem_fname(v_name[idy], fname)); exit(EXIT_FAILURE);}

    // checa se et eh uma variavel
    if (et%OFST != 0 && v_isar[et%OFST] > 0) {fprintf(stderr, MSG_ERR_WRONG_USE, line_num+1, rem_fname(v_name[et%OFST], fname)); exit(EXIT_FAILURE);}

    // checa se idv eh um vetor
    if (v_isar[idv] != 1) {fprintf(stderr, MSG_ERR_NOT_A_VECTOR, line_num+1, rem_fname(v_name[idv], fname)); exit(EXIT_FAILURE);}

    // checa tamanho entre vetores
    if (v_size[idy] != v_size[idv]) {fprintf(stderr, MSG_ERR_VECTOR_SIZE_DIFF2, line_num+1); exit(EXIT_FAILURE);}

    // ------------------------------------------------------------------------
    // atualiza status das variaveis ------------------------------------------
    // ------------------------------------------------------------------------

    if (et%OFST != 0) v_used[et%OFST] = 1;
                      v_used[idv    ] = 1;

    // ------------------------------------------------------------------------
    // prepara variaveis locais -----------------------------------------------
    // ------------------------------------------------------------------------

    int N = v_size[idv];

    char g[64]; if (et%OFST==0) strcpy(g,"aux_var"); else strcpy(g,v_name[et%OFST]);

    // ------------------------------------------------------------------------
    // executa ----------------------------------------------------------------
    // ------------------------------------------------------------------------

    printf(MSG_INFO_DIRAC_CV, line_num+1);

    if (et%OFST==0) add_instr("SET aux_var\n");

    // implementar combinacoes sob demanda

    for (int i = 0; i < N; i++)
    {
        add_instr("LOD_V %s %d\n", v_name[idv], i);

        // int com int
        if (v_type[idy] == 1)
        {
            add_instr("MLT %s\n", g);
        }

        // float com float
        if (v_type[idy] == 2)
        {
            add_instr("F_MLT %s\n", g);
        }

        add_instr("SET_V %s %d\n", v_name[idy], i);
    }

    acc_ok = 0;
}

// soma ponderada no segundo vetor, ex: a # |b> + c|d>;
// fiz mais pra usar no RLS mesmo
void exec_apcb(int idy, int ida, int etc, int idb)
{
    // ------------------------------------------------------------------------
    // checa consistencia -----------------------------------------------------
    // ------------------------------------------------------------------------

    // checa se idy foi declarada
    if (v_type[idy] == 0) {fprintf(stderr, MSG_ERR_DECL_FIRST, line_num+1, rem_fname(v_name[idy], fname)); exit(EXIT_FAILURE);}

    // checa se ida foi declarada
    if (v_type[ida] == 0) {fprintf(stderr, MSG_ERR_DECL_FIRST, line_num+1, rem_fname(v_name[ida], fname)); exit(EXIT_FAILURE);}

    // checa se etc foi declarada
    if (etc%OFST != 0 && v_type[etc%OFST] == 0) {fprintf(stderr, MSG_ERR_DECL_FIRST, line_num+1, rem_fname(v_name[etc%OFST], fname)); exit(EXIT_FAILURE);}
    
    // checa se idb foi declarada
    if (v_type[idb] == 0) {fprintf(stderr, MSG_ERR_DECL_FIRST, line_num+1, rem_fname(v_name[idb], fname)); exit(EXIT_FAILURE);}

    // checa se os tipos sao os mesmos
    if (v_type[idy] != v_type[ida] || v_type[idy] != get_type(etc) || v_type[idy] != v_type[idb]) {fprintf(stderr, MSG_ERR_VARS_SAME_TYPE, line_num+1); exit(EXIT_FAILURE);}

    // checa se nao eh comp
    if (v_type[idy] == 3 || v_type[ida] == 3 || get_type(etc) == 3 || v_type[idb] == 3) {fprintf(stderr, MSG_ERR_NOT_IMPL_COMPLEX, line_num+1); exit(EXIT_FAILURE);}

    // checa se idy eh um vetor
    if (v_isar[idy] != 1) {fprintf(stderr, MSG_ERR_NOT_A_VECTOR, line_num+1, rem_fname(v_name[idy], fname)); exit(EXIT_FAILURE);}

    // checa se ida eh um vetor
    if (v_isar[ida] != 1) {fprintf(stderr, MSG_ERR_NOT_A_VECTOR, line_num+1, rem_fname(v_name[ida], fname)); exit(EXIT_FAILURE);}

    // checa se et eh uma variavel
    if (etc%OFST != 0 && v_isar[etc%OFST] > 0) {fprintf(stderr, MSG_ERR_WRONG_USE, line_num+1, rem_fname(v_name[etc%OFST], fname)); exit(EXIT_FAILURE);}

    // checa se idb eh um vetor
    if (v_isar[idb] != 1) {fprintf(stderr, MSG_ERR_NOT_A_VECTOR, line_num+1, rem_fname(v_name[idb], fname)); exit(EXIT_FAILURE);}

    // checa tamanho entre vetores
    if (v_size[idy] != v_size[ida]) {fprintf(stderr, MSG_ERR_VECTOR_SIZE_DIFF2, line_num+1); exit(EXIT_FAILURE);}

    // checa tamanho entre vetores
    if (v_size[idy] != v_size[idb]) {fprintf(stderr, MSG_ERR_VECTOR_SIZE_DIFF2, line_num+1); exit(EXIT_FAILURE);}

    // ------------------------------------------------------------------------
    // atualiza status das variaveis ------------------------------------------
    // ------------------------------------------------------------------------

                       v_used[ida     ] = 1;
    if (etc%OFST != 0) v_used[etc%OFST] = 1;
                       v_used[idb     ] = 1;

    // ------------------------------------------------------------------------
    // prepara variaveis locais -----------------------------------------------
    // ------------------------------------------------------------------------

    printf(MSG_INFO_DIRAC_VECTOR_SUM, line_num+1);

    int N = v_size[idy];

    char g[64]; if (etc%OFST==0) strcpy(g,"aux_var"); else strcpy(g,v_name[etc%OFST]);

    // ------------------------------------------------------------------------
    // executa ----------------------------------------------------------------
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

// produto externo entre dois vetores, ex: A # |a><b|;
void exec_vvt(int idM, int ida, int idb)
{
    // ------------------------------------------------------------------------
    // checa consistencia -----------------------------------------------------
    // ------------------------------------------------------------------------

    // checa se idM foi declarada
    if (v_type[idM] == 0) {fprintf(stderr, MSG_ERR_DECL_FIRST, line_num+1, rem_fname(v_name[idM], fname)); exit(EXIT_FAILURE);}

    // checa se ida foi declarada
    if (v_type[ida] == 0) {fprintf(stderr, MSG_ERR_DECL_FIRST, line_num+1, rem_fname(v_name[ida], fname)); exit(EXIT_FAILURE);}
    
    // checa se idb foi declarada
    if (v_type[idb] == 0) {fprintf(stderr, MSG_ERR_DECL_FIRST, line_num+1, rem_fname(v_name[idb], fname)); exit(EXIT_FAILURE);}

    // checa se os tipos sao os mesmos
    if (v_type[idM] != v_type[ida] || v_type[idM] != v_type[idb]) {fprintf(stderr, MSG_ERR_VARS_SAME_TYPE, line_num+1); exit(EXIT_FAILURE);}

    // checa se nao eh comp
    if (v_type[idM] == 3 || v_type[ida] == 3 || v_type[idb] == 3) {fprintf(stderr, MSG_ERR_NOT_IMPL_COMPLEX, line_num+1); exit(EXIT_FAILURE);}

    // checa se idM eh uma matriz
    if (v_isar[idM] != 2) {fprintf(stderr, MSG_ERR_NOT_A_MATRIX, line_num+1, rem_fname(v_name[idM], fname)); exit(EXIT_FAILURE);}

    // checa se ida eh um vetor
    if (v_isar[ida] != 1) {fprintf(stderr, MSG_ERR_NOT_A_VECTOR, line_num+1, rem_fname(v_name[ida], fname)); exit(EXIT_FAILURE);}

    // checa se idb eh um vetor
    if (v_isar[idb] != 1) {fprintf(stderr, MSG_ERR_NOT_A_VECTOR, line_num+1, rem_fname(v_name[idb], fname)); exit(EXIT_FAILURE);}

    // checa tamanho entre elementos
    if (v_size[idM] != v_size[ida] || v_size[idM] != v_size[idb]) {fprintf(stderr, MSG_ERR_DIM_MISMATCH, line_num+1); exit(EXIT_FAILURE);}

    // ------------------------------------------------------------------------
    // atualiza status das variaveis ------------------------------------------
    // ------------------------------------------------------------------------

    v_used[ida] = 1;
    v_used[idb] = 1;

    // ------------------------------------------------------------------------
    // prepara variaveis locais -----------------------------------------------
    // ------------------------------------------------------------------------

    int N = v_size[ida];

    // ------------------------------------------------------------------------
    // executa ----------------------------------------------------------------
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

// subtracao de matriz com produto externo, ex: A # B - |a><b|;
// esse eu fiz mais por causa do RLS mesmo
void exec_Mmvvt(int idA, int idB, int ida, int idb)
{
    // ------------------------------------------------------------------------
    // checa consistencia -----------------------------------------------------
    // ------------------------------------------------------------------------

    // checa se idA foi declarada
    if (v_type[idA] == 0) {fprintf(stderr, MSG_ERR_DECL_FIRST, line_num+1, rem_fname(v_name[idA], fname)); exit(EXIT_FAILURE);}

    // checa se idA foi declarada
    if (v_type[idB] == 0) {fprintf(stderr, MSG_ERR_DECL_FIRST, line_num+1, rem_fname(v_name[idB], fname)); exit(EXIT_FAILURE);}
    
    // checa se ida foi declarada
    if (v_type[ida] == 0) {fprintf(stderr, MSG_ERR_DECL_FIRST, line_num+1, rem_fname(v_name[ida], fname)); exit(EXIT_FAILURE);}

    // checa se idb foi declarada
    if (v_type[idb] == 0) {fprintf(stderr, MSG_ERR_DECL_FIRST, line_num+1, rem_fname(v_name[idb], fname)); exit(EXIT_FAILURE);}

    // checa se os tipos sao os mesmos
    if (v_type[idA] != v_type[idB] || v_type[idA] != v_type[ida] || v_type[idA] != v_type[idb]) {fprintf(stderr, MSG_ERR_VARS_SAME_TYPE, line_num+1); exit(EXIT_FAILURE);}

    // checa se nao eh comp
    if (v_type[idA] == 3 || v_type[idB] == 3 || v_type[ida] == 3 || v_type[idb] == 3) {fprintf(stderr, MSG_ERR_NOT_IMPL_COMPLEX, line_num+1); exit(EXIT_FAILURE);}
    // checa se idA eh uma matriz
    if (v_isar[idA] != 2) {fprintf(stderr, MSG_ERR_NOT_A_MATRIX, line_num+1, rem_fname(v_name[idA], fname)); exit(EXIT_FAILURE);}

    // checa se idB eh uma matriz
    if (v_isar[idB] != 2) {fprintf(stderr, MSG_ERR_NOT_A_MATRIX, line_num+1, rem_fname(v_name[idB], fname)); exit(EXIT_FAILURE);}

    // checa se ida eh um vetor
    if (v_isar[ida] != 1) {fprintf(stderr, MSG_ERR_NOT_A_VECTOR, line_num+1, rem_fname(v_name[ida], fname)); exit(EXIT_FAILURE);}

    // checa se idb eh um vetor
    if (v_isar[idb] != 1) {fprintf(stderr, MSG_ERR_NOT_A_VECTOR, line_num+1, rem_fname(v_name[idb], fname)); exit(EXIT_FAILURE);}

    // checa tamanho entre elementos
    if (v_size[idA] != v_siz2[idA] || v_size[idA] != v_size[idB] || v_size[idA] != v_siz2[idB] || v_size[idA] != v_size[ida] || 
        v_size[idA] != v_size[idb]) {fprintf(stderr, MSG_ERR_DIM_MISMATCH, line_num+1); exit(EXIT_FAILURE);}

    // ------------------------------------------------------------------------
    // atualiza status das variaveis ------------------------------------------
    // ------------------------------------------------------------------------

    v_used[idB] = 1;
    v_used[ida] = 1;
    v_used[idb] = 1;

    // ------------------------------------------------------------------------
    // prepara variaveis locais -----------------------------------------------
    // ------------------------------------------------------------------------

    int N = v_size[ida];

    // ------------------------------------------------------------------------
    // executa ----------------------------------------------------------------
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

// produto entre constante e matriz, ex: A # c|B|;
void exec_cM(int idA, int etc, int idM)
{
    // ------------------------------------------------------------------------
    // checa consistencia -----------------------------------------------------
    // ------------------------------------------------------------------------

    // checa se idA foi declarada
    if (v_type[idA] == 0) {fprintf(stderr, MSG_ERR_DECL_FIRST, line_num+1, rem_fname(v_name[idA], fname)); exit(EXIT_FAILURE);}

    // checa se etc foi declarada
    if (etc%OFST != 0 && v_type[etc%OFST] == 0) {fprintf(stderr, MSG_ERR_DECL_FIRST, line_num+1, rem_fname(v_name[etc%OFST], fname)); exit(EXIT_FAILURE);}
    
    // checa se idM foi declarada
    if (v_type[idM] == 0) {fprintf(stderr, MSG_ERR_DECL_FIRST, line_num+1, rem_fname(v_name[idM], fname)); exit(EXIT_FAILURE);}

    // checa se os tipos sao os mesmos
    if (v_type[idA] != get_type(etc) || v_type[idA] != v_type[idM]) {fprintf(stderr, MSG_ERR_VARS_SAME_TYPE, line_num+1); exit(EXIT_FAILURE);}

    // checa se nao eh comp
    if (v_type[idA] == 3 || get_type(etc) == 3 || v_type[idM] == 3) {fprintf(stderr, MSG_ERR_NOT_IMPL_COMPLEX, line_num+1); exit(EXIT_FAILURE);}

    // checa se idA eh uma matriz
    if (v_isar[idA] != 2) {fprintf(stderr, MSG_ERR_NOT_A_MATRIX, line_num+1, rem_fname(v_name[idA], fname)); exit(EXIT_FAILURE);}

    // checa se etc eh uma variavel
    if (etc%OFST != 0 && v_isar[etc%OFST] > 0) {fprintf(stderr, MSG_ERR_WRONG_USE, line_num+1, rem_fname(v_name[etc%OFST], fname)); exit(EXIT_FAILURE);}

    // checa se idM eh uma matriz
    if (v_isar[idM] != 2) {fprintf(stderr, MSG_ERR_NOT_A_MATRIX, line_num+1, rem_fname(v_name[idM], fname)); exit(EXIT_FAILURE);}

    // checa tamanho entre elementos
    if (v_size[idA] != v_size[idM] || v_siz2[idA] != v_siz2[idM]) {fprintf(stderr, MSG_ERR_DIM_MISMATCH, line_num+1); exit(EXIT_FAILURE);}

    // ------------------------------------------------------------------------
    // atualiza status das variaveis ------------------------------------------
    // ------------------------------------------------------------------------

    if (etc%OFST != 0) v_used[etc%OFST] = 1;
                       v_used[idM     ] = 1;

    // ------------------------------------------------------------------------
    // prepara variaveis locais -----------------------------------------------
    // ------------------------------------------------------------------------

    int N = v_size[idM];
    int M = v_siz2[idM];

    char g[64]; if (etc%OFST==0) strcpy(g,"aux_var"); else strcpy(g,v_name[etc%OFST]);

    // ------------------------------------------------------------------------
    // executa ----------------------------------------------------------------
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

// gera matriz identidade com constante, ex: A # c|I|;
void exec_cI(int idM, int etc)
{
    // ------------------------------------------------------------------------
    // checa consistencia -----------------------------------------------------
    // ------------------------------------------------------------------------

    // checa se idM foi declarada
    if (v_type[idM] == 0) {fprintf(stderr, MSG_ERR_DECL_FIRST, line_num+1, rem_fname(v_name[idM], fname)); exit(EXIT_FAILURE);}

    // checa se etc foi declarada
    if (etc%OFST != 0 && v_type[etc%OFST] == 0) {fprintf(stderr, MSG_ERR_DECL_FIRST, line_num+1, rem_fname(v_name[etc%OFST], fname)); exit(EXIT_FAILURE);}

    // checa se os tipos sao os mesmos
    if (v_type[idM] != get_type(etc)) {fprintf(stderr, MSG_ERR_VARS_SAME_TYPE, line_num+1); exit(EXIT_FAILURE);}

    // checa se nao eh comp
    if (v_type[idM] == 3 || get_type(etc) == 3) {fprintf(stderr, MSG_ERR_NOT_IMPL_COMPLEX, line_num+1); exit(EXIT_FAILURE);}
    // checa se idM eh uma matriz
    if (v_isar[idM] != 2) {fprintf(stderr, MSG_ERR_NOT_A_MATRIX, line_num+1, rem_fname(v_name[idM], fname)); exit(EXIT_FAILURE);}

    // checa se etc eh uma variavel
    if (etc%OFST != 0 && v_isar[etc%OFST] > 0) {fprintf(stderr, MSG_ERR_WRONG_USE, line_num+1, rem_fname(v_name[etc%OFST], fname)); exit(EXIT_FAILURE);}

    // ------------------------------------------------------------------------
    // atualiza status das variaveis ------------------------------------------
    // ------------------------------------------------------------------------

    if (etc%OFST != 0) v_used[etc%OFST] = 1;

    // ------------------------------------------------------------------------
    // prepara variaveis locais -----------------------------------------------
    // ------------------------------------------------------------------------

    int N = v_size[idM];

    // ------------------------------------------------------------------------
    // executa ----------------------------------------------------------------
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

// gera vetor de zeros, ex: a # |0>;
void exec_v0(int idv)
{
    // ------------------------------------------------------------------------
    // checa consistencia -----------------------------------------------------
    // ------------------------------------------------------------------------

    // checa se idv foi declarada
    if (v_type[idv] == 0) {fprintf(stderr, MSG_ERR_DECL_FIRST, line_num+1, rem_fname(v_name[idv], fname)); exit(EXIT_FAILURE);}

    // checa se nao eh comp
    if (v_type[idv] == 3) {fprintf(stderr, MSG_ERR_NOT_IMPL_COMPLEX, line_num+1); exit(EXIT_FAILURE);}
    
    // checa se idv eh um vetor
    if (v_isar[idv] != 1) {fprintf(stderr, MSG_ERR_NOT_A_VECTOR, line_num+1, rem_fname(v_name[idv], fname)); exit(EXIT_FAILURE);}

    // ------------------------------------------------------------------------
    // prepara variaveis locais -----------------------------------------------
    // ------------------------------------------------------------------------

    int N = v_size[idv];

    // ------------------------------------------------------------------------
    // executa ----------------------------------------------------------------
    // ------------------------------------------------------------------------

    printf(MSG_INFO_DIRAC_ZERO_VECTOR, line_num+1);

    // int
    if (v_type[idv] == 1) add_instr("LOD 0\n");

    // float
    if (v_type[idv] == 2) add_instr("LOD 0.0\n");

    for (int i = 0; i < N; i++) add_instr("SET_V %s %d\n", v_name[idv], i);
}

// le vetor de entrada com peso c, ex: a # c|in(0)>;
void exec_cvin(int idv, int etc, int idp)
{
    // ------------------------------------------------------------------------
    // checa consistencia -----------------------------------------------------
    // ------------------------------------------------------------------------

    // checa se idv foi declarada
    if (v_type[idv] == 0) {fprintf(stderr, MSG_ERR_DECL_FIRST, line_num+1, rem_fname(v_name[idv], fname)); exit(EXIT_FAILURE);}

    // checa se etc foi declarada
    if (etc%OFST != 0 && v_type[etc%OFST] == 0) {fprintf(stderr, MSG_ERR_DECL_FIRST, line_num+1, rem_fname(v_name[etc%OFST], fname)); exit(EXIT_FAILURE);}

    // checa se nao eh comp
    if (v_type[idv] == 3 || get_type(etc) == 3) {fprintf(stderr, MSG_ERR_NOT_IMPL_COMPLEX, line_num+1); exit(EXIT_FAILURE);}

    // checa se idv eh um vetor
    if (v_isar[idv] != 1) {fprintf(stderr, MSG_ERR_NOT_A_VECTOR, line_num+1, rem_fname(v_name[idv], fname)); exit(EXIT_FAILURE);}

    // checa se etc eh uma variavel
    if (etc%OFST != 0 && v_isar[etc%OFST] > 0) {fprintf(stderr, MSG_ERR_WRONG_USE, line_num+1, rem_fname(v_name[etc%OFST], fname)); exit(EXIT_FAILURE);}

    // ------------------------------------------------------------------------
    // atualiza status das variaveis ------------------------------------------
    // ------------------------------------------------------------------------

    if (etc%OFST != 0) v_used[etc%OFST] = 1;

    // ------------------------------------------------------------------------
    // prepara variaveis locais -----------------------------------------------
    // ------------------------------------------------------------------------

    int N = v_size[idv];

    char g[64]; if (etc%OFST==0) strcpy(g,"aux_var"); else strcpy(g,v_name[etc%OFST]);

    // ------------------------------------------------------------------------
    // executa ----------------------------------------------------------------
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

// escreve vetor pra saida com peso c, ex: out(0, c|a>);
void exec_vout(int idp, int etc, int idv)
{
    // ------------------------------------------------------------------------
    // checa consistencia -----------------------------------------------------
    // ------------------------------------------------------------------------

    // checa se idv foi declarada
    if (v_type[idv] == 0) {fprintf(stderr, MSG_ERR_DECL_FIRST, line_num+1, rem_fname(v_name[idv], fname)); exit(EXIT_FAILURE);}

    // checa se etc foi declarada
    if (etc%OFST != 0 && v_type[etc%OFST] == 0) {fprintf(stderr, MSG_ERR_DECL_FIRST, line_num+1, rem_fname(v_name[etc%OFST], fname)); exit(EXIT_FAILURE);}

    // checa se nao eh comp
    if (v_type[idv] == 3 || get_type(etc) == 3) {fprintf(stderr, MSG_ERR_NOT_IMPL_COMPLEX, line_num+1); exit(EXIT_FAILURE);}

    // checa se idv eh um vetor
    if (v_isar[idv] != 1) {fprintf(stderr, MSG_ERR_NOT_A_VECTOR, line_num+1, rem_fname(v_name[idv], fname)); exit(EXIT_FAILURE);}

    // checa se etc eh uma variavel
    if (etc%OFST != 0 && v_isar[etc%OFST] > 0) {fprintf(stderr, MSG_ERR_WRONG_USE, line_num+1, rem_fname(v_name[etc%OFST], fname)); exit(EXIT_FAILURE);}

    // ------------------------------------------------------------------------
    // atualiza status das variaveis ------------------------------------------
    // ------------------------------------------------------------------------

    if (etc%OFST != 0) v_used[etc%OFST] = 1;
                       v_used[idv     ] = 1;
    
    // ------------------------------------------------------------------------
    // prepara variaveis locais -----------------------------------------------
    // ------------------------------------------------------------------------

    int N = v_size[idv];

    char g[64]; if (etc%OFST==0) strcpy(g,"aux_var"); else strcpy(g,v_name[etc%OFST]);

    // ------------------------------------------------------------------------
    // executa ----------------------------------------------------------------
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

// executa um shift register no vetor com o valor dado a esquerda, ex: a # b -> |c>;
// a e c tem que ser os mesmos vetores
// fazer um novo tipo de array para shift register?
void exec_shift(int ida, int etb, int idc)
{
    // ------------------------------------------------------------------------
    // checa consistencia -----------------------------------------------------
    // ------------------------------------------------------------------------

    // checa se ida foi declarada
    if (v_type[ida] == 0) {fprintf(stderr, MSG_ERR_DECL_FIRST, line_num+1, rem_fname(v_name[ida], fname)); exit(EXIT_FAILURE);}

    // checa se etb foi declarada
    if (etb%OFST != 0 && v_type[etb%OFST] == 0) {fprintf(stderr, MSG_ERR_DECL_FIRST, line_num+1, rem_fname(v_name[etb%OFST], fname)); exit(EXIT_FAILURE);}

    // checa se idc eh igual a ida
    if (idc != ida) {fprintf(stderr, MSG_ERR_SHIFT_VEC_SELF, line_num+1); exit(EXIT_FAILURE);}
    
    // checa se nao eh comp
    if (v_type[ida] == 3 || get_type(etb) == 3) {fprintf(stderr, MSG_ERR_NOT_IMPL_COMPLEX, line_num+1); exit(EXIT_FAILURE);}

    // checa se sao tipos iguais
    //if (v_type[ida] != v_type[etb%OFST]) {fprintf(stderr, MSG_ERR_VARS_SAME_TYPE, line_num+1); exit(EXIT_FAILURE);}
    
    // checa se ida eh um vetor
    if (v_isar[ida] != 1) {fprintf(stderr, MSG_ERR_NOT_A_VECTOR, line_num+1, rem_fname(v_name[ida], fname)); exit(EXIT_FAILURE);}

    // checa se etb eh uma variavel
    if (etb%OFST != 0 && v_isar[etb%OFST] > 0) {fprintf(stderr, MSG_ERR_WRONG_USE, line_num+1, rem_fname(v_name[etb%OFST], fname)); exit(EXIT_FAILURE);}

    // ------------------------------------------------------------------------
    // atualiza status das variaveis ------------------------------------------
    // ------------------------------------------------------------------------

    if (etb%OFST != 0) v_used[etb%OFST] = 1;
    
    // ------------------------------------------------------------------------
    // prepara variaveis locais -----------------------------------------------
    // ------------------------------------------------------------------------

    int N = v_size[ida];

    // ------------------------------------------------------------------------
    // executa ----------------------------------------------------------------
    // ------------------------------------------------------------------------

    printf(MSG_INFO_DIRAC_SHIFT, v_name[ida], line_num+1);

    // ida int e etb int na memoria
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

    // ida float e etb float na memoria
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