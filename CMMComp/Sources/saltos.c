// ----------------------------------------------------------------------------
// rotinas para implementacao de saltos ---------------------------------------
// ----------------------------------------------------------------------------

/*
TODO:
1- revisar switch case
*/

#include <stdlib.h>

#include "..\Headers\t2t.h"
#include "..\Headers\oper.h"
#include "..\Headers\labels.h"
#include "..\Headers\global.h"
#include "..\Headers\data_use.h"
#include "..\Headers\variaveis.h"
#include "..\Headers\messages.h"

// variaveis de estado para switch case
int switching = 0;
int case_cnt  = 0;
int swit_cnt  = 0;

// ----------------------------------------------------------------------------
// if/else --------------------------------------------------------------------
// ----------------------------------------------------------------------------

void if_exp(int et)
{
    // int var
    if ((get_type(et) == 1) && (et%OFST!=0))
    {
        add_instr("LOD %s\n", v_name[et%OFST]);
    }

    // int acc
    if ((get_type(et) == 1) && (et%OFST==0))
    {
        // nao faz nada
    }

    // float var
    if ((get_type(et) == 2) && (et%OFST!=0))
    {
        fprintf(stdout, MSG_WARN_COND_FLOAT, line_num+1);

        add_instr("F2I_M %s\n", v_name[et%OFST]);
    }

    // float acc
    if ((get_type(et) == 2) && (et%OFST==0))
    {
        fprintf(stdout, MSG_WARN_COND_FLOAT, line_num+1);
        
        add_instr("F2I\n");
    }

    // comp const
    if (get_type(et) == 5)
    {
        fprintf(stdout, MSG_WARN_COND_COMP, line_num+1);

        int etr,eti;
        get_cmp_cst(et,&etr,&eti);

        add_instr("F2I_M %s\n", v_name[etr%OFST]);
    }

    // comp var
    if ((get_type(et) == 3) && (et % OFST != 0))
    {
        fprintf(stdout, MSG_WARN_COND_COMP, line_num+1);

        add_instr("F2I_M %s\n", v_name[et%OFST]);
    }

    // comp acc
    if ((get_type(et) == 3) && (et % OFST == 0))
    {
        fprintf(stdout, MSG_WARN_COND_COMP, line_num+1);

        add_instr("POP\n");
        add_instr("F2I\n");
    }

    int n = push_lab(0);
    add_instr("JIZ L%delse\n", n); // 0 -> if
    acc_ok = 0;
}

// cria label do final do if sem else
void if_stmt()
{
    int n = pop_lab();
    add_sinst(0, "@L%delse ", n);
}

// antes dos statments do else
void else_stmt()
{
    add_instr("JMP L%dend\n@L%delse ", get_lab(), get_lab());
}

// cria label do final do if/else
void if_fim()
{
    int n = pop_lab();
    add_sinst(0, "@L%dend ", n);
}

// ----------------------------------------------------------------------------
// while ----------------------------------------------------------------------
// ----------------------------------------------------------------------------

// final do while. Da um JMP para o inicio e cria um label pro final logo abaixo
void while_stmt()
{
    int n = pop_lab();
    add_instr("JMP L%d\n@L%dend ",n,n);
}

// da um JMP pro final do while
void exec_break()
{
    // checa se o break esta dentro de um while
    if (get_while() == 0) {fprintf(stderr, MSG_ERR_BREAK_LOST, line_num+1); exit(EXIT_FAILURE);}

    add_instr("JMP L%dend\n", get_while());
}

// somente a palavra-chave while - gera um label nesse ponto
void while_expp()
{
    int n = push_lab(1);
    add_sinst(0, "@L%d ", n);
}

// executa o exp e cria um JIZ pra ver se entra ou nao
void while_expexp(int et)
{
    // int var
    if ((get_type(et) == 1) && (et%OFST!=0))
    {
        add_instr("LOD %s\n", v_name[et%OFST]);
    }

    // int acc
    if ((get_type(et) == 1) && (et%OFST==0))
    {
        // nao faz nada
    }

    // float var
    if ((get_type(et) == 2) && (et%OFST!=0))
    {
        fprintf(stdout, MSG_WARN_COND_FLOAT, line_num+1);

        add_instr("F2I_M %s\n", v_name[et%OFST]);
    }

    // float acc
    if ((get_type(et) == 2) && (et%OFST==0))
    {
        fprintf(stdout, MSG_WARN_COND_FLOAT, line_num+1);
        
        add_instr("F2I\n");
    }

    // comp const
    if (get_type(et) == 5)
    {
        fprintf(stdout, MSG_WARN_COND_COMP, line_num+1);

        int etr,eti;
        get_cmp_cst(et,&etr,&eti);

        add_instr("F2I_M %s\n", v_name[etr%OFST]);
    }

    // comp var
    if ((get_type(et) == 3) && (et % OFST != 0))
    {
        fprintf(stdout, MSG_WARN_COND_COMP, line_num+1);

        add_instr("F2I_M %s\n", v_name[et%OFST]);
    }

    // comp acc
    if ((get_type(et) == 3) && (et % OFST == 0))
    {
        fprintf(stdout, MSG_WARN_COND_COMP, line_num+1);

        add_instr("POP\n");
        add_instr("F2I\n");
    }

    add_instr("JIZ L%dend\n", get_lab());
    acc_ok = 0;
}

// ----------------------------------------------------------------------------
// switch/case ----------------------------------------------------------------
// ----------------------------------------------------------------------------

// executa case x: do switch case
void case_test(int id, int type)
{
    case_cnt++;
    add_sinst(0, "@sw_case_%d_%d ", swit_cnt, case_cnt);

    // gera o exp do valor do case
    int et1 = num2exp(id,type);
    // gera o exp da variavel de controle
    int et2 =  id2exp(find_var("switch_exp"));
    // faz operacao de comparacao
    oper_cmp(et1,et2,4);

    add_instr("JIZ sw_case_%d_%d\n", swit_cnt, case_cnt+1);
    acc_ok = 0;
}

// executa default do switch case
void defaut_test()
{
    case_cnt++;
    add_sinst(0, "@sw_case_%d_%d ", swit_cnt, case_cnt);
}

// executa break do switch case
void switch_break()
{
    add_instr("JMP switch_end_%d\n", swit_cnt);
}

// inicio do switch case
void exec_switch(int et)
{
    if (switching == 1)
    {
        fprintf(stderr, MSG_ERR_NESTED_SWITCH, line_num+1);
        exit(EXIT_FAILURE);
    }

    // acha a variavel switch_exp (lexer) -------------------------------------

    if (find_var("switch_exp") == -1) add_var("switch_exp");
    int id = find_var("switch_exp");

    // equivalente a declar_var -----------------------------------------------

    v_type[id] = get_type(et);
    v_used[id] = 0;

    // equivalente a ass_set --------------------------------------------------

    // int var
    if ((get_type(et) == 1) && (et%OFST!=0))
    {
        add_instr("LOD %s\n", v_name[et%OFST]);
    }

    // int acc
    if ((get_type(et) == 1) && (et%OFST==0))
    {
        // nao faz nada
    }

    // float var
    if ((get_type(et) == 2) && (et%OFST!=0))
    {
        fprintf(stdout, MSG_WARN_CASE_FLOAT, line_num+1);

        add_instr("F2I_M %s\n", v_name[et%OFST]);
    }

    // float acc
    if ((get_type(et) == 2) && (et%OFST==0))
    {
        fprintf(stdout, MSG_WARN_CASE_FLOAT, line_num+1);
        
        add_instr("F2I\n");
    }

    // comp const
    if (get_type(et) == 5)
    {
        fprintf(stdout, MSG_WARN_CASE_COMP, line_num+1);

        int etr,eti;
        get_cmp_cst(et,&etr,&eti);

        add_instr("F2I_M %s\n", v_name[etr%OFST]);
    }

    // comp var
    if ((get_type(et) == 3) && (et % OFST != 0))
    {
        fprintf(stdout, MSG_WARN_CASE_COMP, line_num+1);

        add_instr("F2I_M %s\n", v_name[et%OFST]);
    }

    // comp acc
    if ((get_type(et) == 3) && (et % OFST == 0))
    {
        fprintf(stdout, MSG_WARN_CASE_COMP, line_num+1);

        add_instr("POP\n");
        add_instr("F2I\n");
    }

    add_instr("SET switch_exp\n");

    // finaliza ---------------------------------------------------------------

    acc_ok     = 0;
    switching  = 1;
    case_cnt   = 0;
    swit_cnt++;
}

// fim do switch case
void end_switch()
{
    add_sinst(0, "@sw_case_%d_%d ", swit_cnt, case_cnt+1);
    add_sinst(0, "@switch_end_%d ", swit_cnt);
    switching = 0;
}
