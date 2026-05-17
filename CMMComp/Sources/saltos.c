// ----------------------------------------------------------------------------
// routines for jump implementation -------------------------------------------
// ----------------------------------------------------------------------------

/*
TODO:
1- review switch case
*/

#include <stdlib.h>

#include "..\Headers\t2t.h"
#include "..\Headers\oper.h"
#include "..\Headers\labels.h"
#include "..\Headers\global.h"
#include "..\Headers\data_use.h"
#include "..\Headers\variaveis.h"
#include "..\Headers\messages.h"

// switch/case state variables
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
        // nothing to do
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

    int n = push_if();
    add_instr("JIZ Lif%delse\n", n); // 0 -> if
    acc_ok = 0;
}

// creates the label at the end of an if-without-else
void if_stmt()
{
    int n = pop_if();
    add_sinst(0, "@Lif%delse ", n);
}

// before the else statements
void else_stmt()
{
    add_instr("JMP Lif%dend\n@Lif%delse ", get_if(), get_if());
}

// creates the label at the end of an if/else
void if_fim()
{
    int n = pop_if();
    add_sinst(0, "@Lif%dend ", n);
}

// ----------------------------------------------------------------------------
// while ----------------------------------------------------------------------
// ----------------------------------------------------------------------------

// end of while. Emits a JMP back to the start and a label for the end right below
void while_stmt()
{
    int n = pop_while();
    add_instr("JMP Lwh%d\n@Lwh%dend ",n,n);
}

// emits a JMP to the end of the while
void exec_break()
{
    // check whether the break is inside a while
    if (get_while() == 0) {fprintf(stderr, MSG_ERR_BREAK_LOST, line_num+1); exit(EXIT_FAILURE);}

    add_instr("JMP Lwh%dend\n", get_while());
}

// the while keyword alone - emits a label here
void while_expp()
{
    int n = push_while();
    add_sinst(0, "@Lwh%d ", n);
}

// evaluates exp and emits a JIZ to decide whether to enter or not
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
        // nothing to do
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

    add_instr("JIZ Lwh%dend\n", get_while());
    acc_ok = 0;
}

// ----------------------------------------------------------------------------
// switch/case ----------------------------------------------------------------
// ----------------------------------------------------------------------------

// emits case x: of the switch-case
void case_test(int id, int type)
{
    case_cnt++;
    add_sinst(0, "@sw_case_%d_%d ", swit_cnt, case_cnt);

    // build the exp for the case value
    int et1 = num2exp(id,type);
    // build the exp for the control variable
    int et2 =  id2exp(find_var("switch_exp"));
    // run the comparison
    oper_cmp(et1,et2,4);

    add_instr("JIZ sw_case_%d_%d\n", swit_cnt, case_cnt+1);
    acc_ok = 0;
}

// emits default of the switch-case
void defaut_test()
{
    case_cnt++;
    add_sinst(0, "@sw_case_%d_%d ", swit_cnt, case_cnt);
}

// emits break of the switch-case
void switch_break()
{
    add_instr("JMP switch_end_%d\n", swit_cnt);
}

// switch-case start
void exec_switch(int et)
{
    if (switching == 1)
    {
        fprintf(stderr, MSG_ERR_NESTED_SWITCH, line_num+1);
        exit(EXIT_FAILURE);
    }

    // find the switch_exp variable (lexer) -----------------------------------

    if (find_var("switch_exp") == -1) add_var("switch_exp");
    int id = find_var("switch_exp");

    // equivalent to declar_var -----------------------------------------------

    v_type[id] = get_type(et);
    v_used[id] = 0;

    // equivalent to ass_set --------------------------------------------------

    // int var
    if ((get_type(et) == 1) && (et%OFST!=0))
    {
        add_instr("LOD %s\n", v_name[et%OFST]);
    }

    // int acc
    if ((get_type(et) == 1) && (et%OFST==0))
    {
        // nothing to do
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

    // finalize ---------------------------------------------------------------

    acc_ok     = 0;
    switching  = 1;
    case_cnt   = 0;
    swit_cnt++;
}

// switch-case end
void end_switch()
{
    add_sinst(0, "@sw_case_%d_%d ", swit_cnt, case_cnt+1);
    add_sinst(0, "@switch_end_%d ", swit_cnt);
    switching = 0;
}
