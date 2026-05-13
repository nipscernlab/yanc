// ----------------------------------------------------------------------------
// implementa instrucoes de assign --------------------------------------------
// ----------------------------------------------------------------------------

/*
TODO:
1- rever os operadores de incremento ++
2- AST vai economizar muito codigo com comp
*/

// includes globais
#include <string.h>
#include <stdlib.h>
#include  <stdio.h>

// includes locais
#include "..\Headers\t2t.h"
#include "..\Headers\global.h"
#include "..\Headers\funcoes.h"
#include "..\Headers\data_use.h"
#include "..\Headers\variaveis.h"
#include "..\Headers\messages.h"

// assign padrao, ex: x = y;
void ass_set(int id, int et)
{
    // testa se ja foi declarada pra poder dar uma atribuicao
    if (v_type[id] == 0)
    {
        fprintf (stderr, MSG_ERR_DECLARE_VAR_PLEASE, line_num+1, rem_fname(v_name[id], fname));
        exit(EXIT_FAILURE);
    }

    // testa se eh um array que esqueceram o indice
    if (v_isar[id] > 0)
    {
        fprintf (stderr, MSG_ERR_ARRAY_NEEDS_IDX, line_num+1, rem_fname(v_name[id], fname));
        exit(EXIT_FAILURE);
    }

    // ------------------------------------------------------------------------
    // executa o assign -------------------------------------------------------
    // ------------------------------------------------------------------------

    int etr, eti;

    // left int e right int na memoria ----------------------------------------

    if ((v_type[id] == 1) && (get_type(et) == 1) && (et % OFST != 0))
    {
        add_instr("LOD %s\n",  v_name[et % OFST]);
        add_instr("SET %s\n" , v_name[id       ]);
    }

    // left int e right int no acc --------------------------------------------

    if ((v_type[id] == 1) && (get_type(et) == 1) && (et % OFST == 0))
    {
        add_instr("SET %s\n", v_name[id]);
    }

    // left int e right float na memoria --------------------------------------

    if ((v_type[id] == 1) && (get_type(et) == 2) && (et % OFST != 0))
    {
        fprintf(stdout, MSG_WARN_INT_RECV_FLOAT, line_num+1, rem_fname(v_name[id], fname));

        add_instr("F2I_M %s\n", v_name[et % OFST]);
        add_instr("SET %s\n"  , v_name[id       ]);
    }

    // left int e right float no acc ------------------------------------------

    if ((v_type[id] == 1) && (get_type(et) == 2) && (et % OFST == 0))
    {
        fprintf(stdout, MSG_WARN_INT_RECV_FLOAT, line_num+1, rem_fname(v_name[id], fname));

        // tentar fazer uma instrucao que faz f2i do acumulador e seta na memoria ao mesmo tempo (FIAS)
        add_instr("F2I\n");
        add_instr("SET %s\n", v_name[id]);
    }

    // left int e right comp const --------------------------------------------

    if ((v_type[id] == 1) && (get_type(et) == 5))
    {
        fprintf (stdout, MSG_WARN_ROUND_REAL, line_num+1);

        get_cmp_cst(et,&etr,&eti);
        
        add_instr("F2I_M %s\n", v_name[etr % OFST]);
        add_instr("SET %s\n"  , v_name[id        ]);
    }

    // left int e right comp na memoria ---------------------------------------

    if ((v_type[id] == 1) && (get_type(et) == 3) && (et % OFST != 0))
    {
        fprintf (stdout, MSG_WARN_ROUND_REAL, line_num+1);
        
        add_instr("F2I_M %s\n", v_name[et % OFST]);
        add_instr("SET %s\n"  , v_name[id       ]);
    }

    // left int e right comp no acc -------------------------------------------

    if ((v_type[id] == 1) && (get_type(et) == 3) && (et % OFST == 0))
    {
        fprintf (stdout, MSG_WARN_ROUND_REAL, line_num+1);

        add_instr("POP\n");
        add_instr("F2I\n");
        add_instr("SET %s\n", v_name[id]);
    }

    // left float e right int na memoria --------------------------------------

    if ((v_type[id] == 2) && (get_type(et) == 1) && (et % OFST != 0))
    {
        fprintf(stdout, MSG_WARN_FLOAT_RECV_INT, line_num+1, rem_fname(v_name[id], fname));

        add_instr("I2F_M %s\n", v_name[et%OFST]);
        add_instr("SET %s\n"  , v_name[id     ]);
    }

    // left float e right int no acc ------------------------------------------

    if ((v_type[id] == 2) && (get_type(et) == 1) && (et % OFST == 0))
    {
        fprintf(stdout, MSG_WARN_FLOAT_RECV_INT, line_num+1, rem_fname(v_name[id], fname));
        
        add_instr("I2F\n");
        add_instr("SET %s\n", v_name[id]);
    }

    // left float e right float na memoria ------------------------------------

    if ((v_type[id] == 2) && (get_type(et) == 2) && (et%OFST != 0))
    {
        add_instr("LOD %s\n", v_name[et%OFST]);
        add_instr("SET %s\n", v_name[id     ]);
    }

    // left float e right float no acc ----------------------------------------

    if ((v_type[id] == 2) && (get_type(et) == 2) && (et % OFST == 0))
    {
        add_instr("SET %s\n", v_name[id]);
    }

    // left float e right comp const ------------------------------------------

    if ((v_type[id] == 2) && (get_type(et) == 5))
    {
        fprintf (stdout, MSG_WARN_GRAB_REAL_ONLY, line_num+1);

        get_cmp_cst(et,&etr,&eti);
        
        add_instr("LOD %s\n", v_name[etr%OFST]);
        add_instr("SET %s\n", v_name[id      ]);
    }

    // left float e right comp na memoria -------------------------------------

    if ((v_type[id] == 2) && (get_type(et) == 3) && (et % OFST != 0))
    {
        fprintf (stdout, MSG_WARN_GRAB_REAL_ONLY, line_num+1);

        get_cmp_ets(et,&etr,&eti);
        
        add_instr("LOD %s\n", v_name[etr%OFST]);
        add_instr("SET %s\n", v_name[id      ]);
    }

    // left float e right comp no acc -----------------------------------------

    if ((v_type[id] == 2) && (get_type(et) == 3) && (et % OFST == 0))
    {
        fprintf (stdout, MSG_WARN_GRAB_REAL_ONLY, line_num+1);

        add_instr("POP\n");
        add_instr("SET %s\n", v_name[id]);
    }

    // left comp e right int na memoria --------------------------------------- 

    if ((v_type[id] == 3) && (get_type(et) == 1) && (et % OFST != 0))
    {
        fprintf(stdout, MSG_WARN_COMP_RECV_INT, line_num+1, rem_fname(v_name[id], fname));

        add_instr("I2F_M %s\n", v_name[et%OFST]);
        add_instr("SET %s\n"  , v_name[id     ]);

        add_instr("LOD 0.0\n");
        add_instr("SET %s_i\n", v_name[id     ]);
    }

    // left comp e right int no acc -------------------------------------------

    if ((v_type[id] == 3) && (get_type(et) == 1) && (et % OFST == 0))
    {
        fprintf(stdout, MSG_WARN_COMP_RECV_INT, line_num+1, rem_fname(v_name[id], fname));
        
        add_instr("I2F\n");
        add_instr("SET %s\n"  , v_name[id]);

        add_instr("LOD 0.0\n");
        add_instr("SET %s_i\n", v_name[id]);
    }

    // left comp e right float var na memoria ---------------------------------

    if ((v_type[id] == 3) && (get_type(et) == 2) && (et % OFST != 0))
    {
        fprintf(stdout, MSG_WARN_COMP_RECV_FLOAT, line_num+1, rem_fname(v_name[id], fname));

        add_instr("LOD %s\n" , v_name[et%OFST]);
        add_instr("SET %s\n" , v_name[id     ]);

        add_instr("LOD 0.0\n");
        add_instr("SET %s_i\n",v_name[id]);
    }

    // left comp e right float no acc -----------------------------------------

    if ((v_type[id] == 3) && (get_type(et) == 2) && (et % OFST == 0))
    {
        fprintf(stdout, MSG_WARN_COMP_RECV_FLOAT, line_num+1, rem_fname(v_name[id], fname));
        
        add_instr("SET %s\n"  , v_name[id]);

        add_instr("LOD 0.0\n");
        add_instr("SET %s_i\n", v_name[id]);
    }

    // left comp e right comp const -------------------------------------------

    if ((v_type[id] == 3) && (get_type(et) == 5))
    {
        get_cmp_cst(et,&etr,&eti);
        
        add_instr("LOD %s\n",   v_name[etr%OFST]);
        add_instr("SET %s\n",   v_name[id      ]);

        add_instr("LOD %s\n",   v_name[eti%OFST]);
        add_instr("SET %s_i\n", v_name[id      ]);
    }

    // left comp e right comp na memoria ----------------------------------

    if ((v_type[id] == 3) && (get_type(et) == 3) && (et % OFST != 0))
    {
        get_cmp_ets(et,&etr,&eti);
        
        add_instr("LOD %s\n"  , v_name[etr%OFST]);
        add_instr("SET %s\n"  , v_name[id      ]);

        add_instr("LOD %s\n"  , v_name[eti%OFST]);
        add_instr("SET %s_i\n", v_name[id      ]);
    }

    // left comp e right comp no acc --------------------------------------

    if ((v_type[id] == 3) && (get_type(et) == 3) && (et % OFST == 0))
    {    
        add_instr("SET_P %s_i\n", v_name[id]);
        add_instr("SET %s\n"    , v_name[id]);
    }

    acc_ok     = 0;  // liberou o acc
}

// assign em array, ex x[i] = y;
void ass_array(int id, int et, int fft)
{
    // testa se ja foi declarada pra poder dar uma atribuicao
    if (v_type[id] == 0)
    {
        fprintf (stderr, MSG_ERR_DECLARE_VAR_PLEASE, line_num+1, rem_fname(v_name[id], fname));
        exit(EXIT_FAILURE);
    }

    // checagem entre array e nao-array
    if (v_isar[id] == 0)
    {
        fprintf (stderr, MSG_ERR_NOT_ARRAY, line_num+1, rem_fname(v_name[id], fname));
        exit(EXIT_FAILURE);
    }

    // ------------------------------------------------------------------------
    // executa o set ----------------------------------------------------------
    // ------------------------------------------------------------------------

    int  etr, eti;

    char set_type[16]; if (fft == 0) strcpy(set_type, "STI"); else strcpy(set_type, "ISI");

    // left int e right int na memoria ----------------------------------------

    if ((v_type[id] == 1) && (get_type(et) == 1) && (et % OFST != 0))
    {
        add_instr("P_LOD %s\n",   v_name[et%OFST]);
        add_instr("%s %s\n", set_type, v_name[id]);
    }

    // left int e right int no acc --------------------------------------------

    if ((v_type[id] == 1) && (get_type(et) == 1) && (et % OFST == 0))
    {
        add_instr("%s %s\n", set_type, v_name[id]);
    }

    // left int e right float na memoria --------------------------------------

    if ((v_type[id] == 1) && (get_type(et) == 2) && (et % OFST != 0))
    {
        fprintf(stdout, MSG_WARN_INT_RECV_FLOAT, line_num+1, rem_fname(v_name[id], fname));

        add_instr("P_F2I_M %s\n", v_name[et%OFST]);
        add_instr("%s %s\n", set_type, v_name[id]);
    }

    // left int e right float no acc ------------------------------------------

    if ((v_type[id] == 1) && (get_type(et) == 2) && (et % OFST == 0))
    {
        fprintf(stdout, MSG_WARN_INT_RECV_FLOAT, line_num+1, rem_fname(v_name[id], fname));
        
        add_instr("F2I\n");
        add_instr("%s %s\n", set_type, v_name[id]);
    }

    // left int e right comp const --------------------------------------------

    if ((v_type[id] == 1) && (get_type(et) == 5))
    {
        fprintf (stdout, MSG_WARN_ROUND_REAL, line_num+1);

        get_cmp_cst(et,&etr,&eti);
        
        add_instr("P_F2I_M %s\n", v_name[etr%OFST]);
        add_instr("%s %s\n", set_type,  v_name[id]);
    }

    // left int e right comp na memoria ---------------------------------------

    if ((v_type[id] == 1) && (get_type(et) == 3) && (et % OFST != 0))
    {
        fprintf (stdout, MSG_WARN_ROUND_REAL, line_num+1);

        get_cmp_ets(et,&etr,&eti);
        
        add_instr("P_F2I_M %s\n", v_name[etr%OFST]);
        add_instr("%s %s\n", set_type,  v_name[id]);
    }

    // left int e right comp no acc -------------------------------------------

    if ((v_type[id] == 1) && (get_type(et) == 3) && (et % OFST == 0))
    {
        fprintf (stdout, MSG_WARN_ROUND_REAL, line_num+1);

        add_instr("POP\n");
        add_instr("F2I\n");
        add_instr("%s %s\n", set_type, v_name[id]);
    }

    // left float e right int na memoria --------------------------------------

    if ((v_type[id] == 2) && (get_type(et) == 1) && (et % OFST != 0))
    {
        fprintf(stdout, MSG_WARN_FLOAT_RECV_INT, line_num+1, rem_fname(v_name[id], fname));

        add_instr("P_I2F_M %s\n", v_name[et%OFST]);
        add_instr("%s %s\n", set_type, v_name[id]);
    }

    // left float e right int no acc ------------------------------------------

    if ((v_type[id] == 2) && (get_type(et) == 1) && (et % OFST == 0))
    {
        fprintf(stdout, MSG_WARN_FLOAT_RECV_INT, line_num+1, rem_fname(v_name[id], fname));
        
        add_instr("I2F\n");
        add_instr("%s %s\n", set_type, v_name[id]);
    }

    // left float e right float na memoria ------------------------------------

    if ((v_type[id] == 2) && (get_type(et) == 2) && (et % OFST != 0))
    {
        add_instr("P_LOD %s\n",   v_name[et%OFST]);
        add_instr("%s %s\n", set_type, v_name[id]);
    }

    // left float e right float no acc ----------------------------------------

    if ((v_type[id] == 2) && (get_type(et) == 2) && (et % OFST == 0))
    {
        add_instr("%s %s\n", set_type, v_name[id]);
    }

    // left float e right comp const ------------------------------------------

    if ((v_type[id] == 2) && (get_type(et) == 5))
    {
        fprintf (stdout, MSG_WARN_GRAB_REAL_ONLY, line_num+1);

        get_cmp_cst(et,&etr,&eti);
        
        add_instr("P_LOD %s\n",  v_name[etr%OFST]);
        add_instr("%s %s\n", set_type, v_name[id]);
    }

    // left float e right comp na memoria -------------------------------------

    if ((v_type[id] == 2) && (get_type(et) == 3) && (et % OFST != 0))
    {
        fprintf (stdout, MSG_WARN_GRAB_REAL_ONLY, line_num+1);

        get_cmp_ets(et,&etr,&eti);
        
        add_instr("P_LOD %s\n",  v_name[etr%OFST]);
        add_instr("%s %s\n", set_type, v_name[id]);
    }

    // left float e right comp no acc -----------------------------------------

    if ((v_type[id] == 2) && (get_type(et) == 3) && (et % OFST == 0))
    {
        fprintf (stdout, MSG_WARN_GRAB_REAL_ONLY, line_num+1);

        add_instr("POP\n");
        add_instr("%s %s\n", set_type, v_name[id]);
    }

    // left comp e right int na memoria ---------------------------------------

    if ((v_type[id] == 3) && (get_type(et) == 1) && (et % OFST != 0))
    {
        fprintf(stdout, MSG_WARN_COMP_RECV_INT, line_num+1, rem_fname(v_name[id], fname));

        add_instr("SET aux_var\n");
        add_instr("P_I2F_M %s\n", v_name[et%OFST]);
        add_instr("%s %s\n"  , set_type, v_name[id]);

        add_instr("LOD aux_var\n");
        add_instr("P_LOD 0.0\n"  );
        add_instr("%s %s_i\n", set_type, v_name[id]);
    }

    // left comp e right int no acc -------------------------------------------

    if ((v_type[id] == 3) && (get_type(et) == 1) && (et % OFST == 0))
    {
        fprintf(stdout, MSG_WARN_COMP_RECV_INT, line_num+1, rem_fname(v_name[id], fname));
        
        add_instr("I2F\n"                   );
        add_instr("SET_P aux_var\n"         );
        add_instr("SET   aux_var2\n"        );
        add_instr("P_LOD aux_var\n"         );
        add_instr("%s %s\n"  , set_type, v_name[id]);

        add_instr("LOD   aux_var2\n"        );
        add_instr("P_LOD 0.0\n"             );
        add_instr("%s %s_i\n", set_type, v_name[id]);
    }

    // left comp e right float na memoria -------------------------------------

    if ((v_type[id] == 3) && (get_type(et) == 2) && (et % OFST != 0))
    {
        fprintf(stdout, MSG_WARN_COMP_RECV_FLOAT, line_num+1, rem_fname(v_name[id], fname));

        add_instr("SET   aux_var\n"         );
        add_instr("P_LOD %s\n"  , v_name[et%OFST]);
        add_instr("%s %s\n"  , set_type, v_name[id]);

        add_instr("LOD   aux_var\n"         );
        add_instr("P_LOD 0.0\n"             );
        add_instr("%s %s_i\n", set_type, v_name[id]);
    }

    // left comp e right float no acc -----------------------------------------

    if ((v_type[id] == 3) && (get_type(et) == 2) && (et % OFST == 0))
    {
        fprintf(stdout, MSG_WARN_COMP_RECV_FLOAT, line_num+1, rem_fname(v_name[id], fname));
        
        add_instr("SET_P aux_var\n"         );
        add_instr("SET   aux_var2\n"        );
        add_instr("P_LOD aux_var\n"         );
        add_instr("%s %s\n"  , set_type, v_name[id]);

        add_instr("LOD   aux_var2\n"        );
        add_instr("P_LOD 0.0\n"             );
        add_instr("%s %s_i\n", set_type, v_name[id]);
    }

    // left comp e right comp const -------------------------------------------

    if ((v_type[id] == 3) && (get_type(et) == 5))
    {
        get_cmp_cst(et,&etr,&eti);
        
        add_instr("SET   aux_var\n");
        add_instr("P_LOD %s\n"  , v_name[etr%OFST]);
        add_instr("%s %s\n" , set_type, v_name[id]);

        add_instr("LOD   aux_var\n");
        add_instr("P_LOD %s\n"   , v_name[eti%OFST]);
        add_instr("%s %s_i\n", set_type, v_name[id]);
    }

    // left comp e right comp na memoria ----------------------------------

    if ((v_type[id] == 3) && (get_type(et) == 3) && (et % OFST != 0))
    {
        get_cmp_ets(et,&etr,&eti);
        
        add_instr("SET   aux_var\n");
        add_instr("P_LOD %s\n"   , v_name[etr%OFST]);
        add_instr("%s %s\n"  , set_type, v_name[id]);

        add_instr("LOD   aux_var\n");
        add_instr("P_LOD %s\n"   , v_name[eti%OFST]);
        add_instr("%s %s_i\n", set_type, v_name[id]);
    }

    // left comp e right comp no acc --------------------------------------

    if ((v_type[id] == 3) && (get_type(et) == 3) && (et % OFST == 0))
    {    
        add_instr("SET_P aux_var\n" );
        add_instr("SET_P aux_var2\n");
        add_instr("SET   aux_var3\n");

        add_instr("P_LOD aux_var2\n");
        add_instr("%s %s\n"  , set_type, v_name[id]);

        add_instr("LOD   aux_var3\n");
        add_instr("P_LOD aux_var\n" );
        add_instr("%s %s_i\n", set_type, v_name[id]);
    }

    acc_ok     = 0;  // liberou o acc
}

// assign com operador ++
void ass_pplus(int id)
{
    pplus2exp(id);
    acc_ok = 0; // liberou o acc;
}

// assign com operador ++ em array 1D
void ass_aplus(int id, int et)
{
    pplus1d2exp(id,et);
    acc_ok = 0; // liberou o acc;
}

// assign com operador ++ em array 2D
void ass_apl2d(int id, int et1, int et2)
{
    pplus2d2exp(id,et1,et2);
    acc_ok = 0; // liberou o acc;
}
