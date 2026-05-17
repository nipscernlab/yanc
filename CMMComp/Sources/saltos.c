// ----------------------------------------------------------------------------
// routines for jump implementation -------------------------------------------
// ----------------------------------------------------------------------------

/*
TODO:
1- review switch case
*/

#include <stdlib.h>

#include "..\Headers\ast.h"
#include "..\Headers\emit.h"
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
// parked then-body stack -----------------------------------------------------
// ----------------------------------------------------------------------------
// During an if/else parse, the then-body captures finish (at the ELSE token)
// before we know what the else-body looks like. We park the then-body's AST
// node here until if_fim runs and can build the full AST_IF. The stack matches
// bison's depth-first reduction for nested if/else.

static ast_node **then_park     = NULL;
static int        then_park_n   = 0;
static int        then_park_cap = 0;

static void park_then(ast_node *n)
{
    if (then_park_n + 1 > then_park_cap)
    {
        int new_cap = then_park_cap ? then_park_cap * 2 : 16;
        ast_node **t = realloc(then_park, (size_t)new_cap * sizeof(*t));
        if (!t) {fprintf(stderr, MSG_ERR_OUT_OF_MEMORY); exit(EXIT_FAILURE);}
        then_park     = t;
        then_park_cap = new_cap;
    }
    then_park[then_park_n++] = n;
}

static ast_node *unpark_then(void)
{
    return then_park[--then_park_n];
}

// helper: build an AST_RAW node from a captured body, freeing the source text
static ast_node *body_to_node(char *captured)
{
    ast_node *n = ast_raw(captured);   // ast_raw copies the text
    free(captured);
    return n;
}

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

    // start capturing the then-body; the captured text is wrapped into an
    // AST_RAW leaf when the if/else reduction fires
    emit_push_capture();
}

// builds the AST for an if-without-else and emits it
void if_stmt()
{
    ast_node *body  = body_to_node(emit_pop_capture());
    int       label = pop_if();
    ast_node *node  = ast_if(label, body, NULL);
    ast_emit(node);
    ast_free(node);
}

// between the then and the else: park the captured then, start capturing else
void else_stmt()
{
    park_then(body_to_node(emit_pop_capture()));
    emit_push_capture();
}

// builds the AST for a full if/else and emits it
void if_fim()
{
    ast_node *els   = body_to_node(emit_pop_capture());
    ast_node *body  = unpark_then();
    int       label = pop_if();
    ast_node *node  = ast_if(label, body, els);
    ast_emit(node);
    ast_free(node);
}

// ----------------------------------------------------------------------------
// while ----------------------------------------------------------------------
// ----------------------------------------------------------------------------

// end of while: build the AST and emit it
void while_stmt()
{
    ast_node *body  = body_to_node(emit_pop_capture());
    int       label = pop_while();
    ast_node *node  = ast_while(label, body);
    ast_emit(node);
    ast_free(node);
}

// emits a JMP to the end of the while
void exec_break()
{
    // parse-time check: break must live inside a while
    if (get_while() == 0) {fprintf(stderr, MSG_ERR_BREAK_LOST, line_num+1); exit(EXIT_FAILURE);}

    // build a tiny AST node and ask it to emit itself
    ast_node *n = ast_break();
    ast_emit (n);
    ast_free (n);
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

    // start capturing the while body
    emit_push_capture();
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
    int et1 = expr_to_et(num2exp(id, type));
    // build the exp for the control variable
    int et2 = expr_to_et( id2exp(find_var("switch_exp")));
    // run the comparison (2 = EQU; the historic '4' was a typo that fell
    // through oper_cmp's switch and emitted an uninitialized mnemonic)
    oper_cmp(et1,et2,2);

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

    // capture every case label, comparison, JIZ, body, and break that
    // shows up between here and end_switch
    emit_push_capture();
}

// switch-case end: build the AST and emit it
void end_switch()
{
    ast_node *body = body_to_node(emit_pop_capture());
    ast_node *node = ast_switch(swit_cnt, case_cnt, body);
    ast_emit(node);
    ast_free(node);
    switching = 0;
}
