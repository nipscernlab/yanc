// ----------------------------------------------------------------------------
// assignment instruction implementation --------------------------------------
// ----------------------------------------------------------------------------


// global includes
#include <string.h>
#include <stdlib.h>
#include  <stdio.h>

// local includes
#include "../Headers/data_assign.h"
#include "../Headers/convert.h"
#include "../Headers/global.h"
#include "../Headers/functions.h"
#include "../Headers/data_use.h"
#include "../Headers/symbols.h"
#include "../Headers/messages.h"
#include "../Headers/diag.h"

// standard assignment, e.g. x = y;
void assign_set(int id, expr e)
{
    // test whether it has been declared so an assignment is allowed
    if (v_table[id].type == TY_NONE)
    {
        fatal(MSG_ERR_DECLARE_VAR_PLEASE, pctx.line_num+1, strip_fn_prefix(v_table[id].name));
    }

    // test whether it is an array missing its index
    if (v_table[id].dims > AR_NONE)
    {
        fatal(MSG_ERR_ARRAY_NEEDS_IDX, pctx.line_num+1, strip_fn_prefix(v_table[id].name));
    }

    // ------------------------------------------------------------------------
    // perform the assignment -------------------------------------------------
    // ------------------------------------------------------------------------

    expr etr, eti;

    // left int and right int in memory ---------------------------------------

    if ((v_table[id].type == TY_INT) && (e.type == TY_INT) && (e.id != 0))
    {
        add_instr("LOD %s\n",  v_table[e.id].name);
        add_instr("SET %s\n" , v_table[id       ].name);
    }

    // left int and right int in the acc --------------------------------------

    if ((v_table[id].type == TY_INT) && (e.type == TY_INT) && (e.id == 0))
    {
        add_instr("SET %s\n", v_table[id].name);
    }

    // left int and right float in memory -------------------------------------

    if ((v_table[id].type == TY_INT) && (e.type == TY_FLOAT) && (e.id != 0))
    {
        warn(MSG_WARN_INT_RECV_FLOAT, pctx.line_num+1, strip_fn_prefix(v_table[id].name));

        add_instr("F2I_M %s\n", v_table[e.id].name);
        add_instr("SET %s\n"  , v_table[id       ].name);
    }

    // left int and right float in the acc ------------------------------------

    if ((v_table[id].type == TY_INT) && (e.type == TY_FLOAT) && (e.id == 0))
    {
        warn(MSG_WARN_INT_RECV_FLOAT, pctx.line_num+1, strip_fn_prefix(v_table[id].name));

        // consider an instruction that does f2i on the accumulator and stores in memory at once (FIAS)
        add_instr("F2I\n");
        add_instr("SET %s\n", v_table[id].name);
    }

    // left int and right comp const ------------------------------------------

    if ((v_table[id].type == TY_INT) && (e.type == TY_CCONST))
    {
        warn(MSG_WARN_ROUND_REAL, pctx.line_num+1);

        get_cmp_cst(e,&etr,&eti);

        add_instr("F2I_M %s\n", v_table[etr.id].name);
        add_instr("SET %s\n"  , v_table[id        ].name);
    }

    // left int and right comp in memory --------------------------------------

    if ((v_table[id].type == TY_INT) && (e.type == TY_COMP) && (e.id != 0))
    {
        warn(MSG_WARN_ROUND_REAL, pctx.line_num+1);

        add_instr("F2I_M %s\n", v_table[e.id].name);
        add_instr("SET %s\n"  , v_table[id       ].name);
    }

    // left int and right comp in the acc -------------------------------------

    if ((v_table[id].type == TY_INT) && (e.type == TY_COMP) && (e.id == 0))
    {
        warn(MSG_WARN_ROUND_REAL, pctx.line_num+1);

        add_instr("POP\n");
        add_instr("F2I\n");
        add_instr("SET %s\n", v_table[id].name);
    }

    // left float and right int in memory -------------------------------------

    if ((v_table[id].type == TY_FLOAT) && (e.type == TY_INT) && (e.id != 0))
    {
        warn(MSG_WARN_FLOAT_RECV_INT, pctx.line_num+1, strip_fn_prefix(v_table[id].name));

        add_instr("I2F_M %s\n", v_table[e.id].name);
        add_instr("SET %s\n"  , v_table[id     ].name);
    }

    // left float and right int in the acc ------------------------------------

    if ((v_table[id].type == TY_FLOAT) && (e.type == TY_INT) && (e.id == 0))
    {
        warn(MSG_WARN_FLOAT_RECV_INT, pctx.line_num+1, strip_fn_prefix(v_table[id].name));

        add_instr("I2F\n");
        add_instr("SET %s\n", v_table[id].name);
    }

    // left float and right float in memory -----------------------------------

    if ((v_table[id].type == TY_FLOAT) && (e.type == TY_FLOAT) && (e.id != 0))
    {
        add_instr("LOD %s\n", v_table[e.id].name);
        add_instr("SET %s\n", v_table[id     ].name);
    }

    // left float and right float in the acc ----------------------------------

    if ((v_table[id].type == TY_FLOAT) && (e.type == TY_FLOAT) && (e.id == 0))
    {
        add_instr("SET %s\n", v_table[id].name);
    }

    // left float and right comp const ----------------------------------------

    if ((v_table[id].type == TY_FLOAT) && (e.type == TY_CCONST))
    {
        warn(MSG_WARN_GRAB_REAL_ONLY, pctx.line_num+1);

        get_cmp_cst(e,&etr,&eti);

        add_instr("LOD %s\n", v_table[etr.id].name);
        add_instr("SET %s\n", v_table[id      ].name);
    }

    // left float and right comp in memory ------------------------------------

    if ((v_table[id].type == TY_FLOAT) && (e.type == TY_COMP) && (e.id != 0))
    {
        warn(MSG_WARN_GRAB_REAL_ONLY, pctx.line_num+1);

        get_cmp_ets(e,&etr,&eti);

        add_instr("LOD %s\n", v_table[etr.id].name);
        add_instr("SET %s\n", v_table[id      ].name);
    }

    // left float and right comp in the acc -----------------------------------

    if ((v_table[id].type == TY_FLOAT) && (e.type == TY_COMP) && (e.id == 0))
    {
        warn(MSG_WARN_GRAB_REAL_ONLY, pctx.line_num+1);

        add_instr("POP\n");
        add_instr("SET %s\n", v_table[id].name);
    }

    // left comp and right int in memory --------------------------------------

    if ((v_table[id].type == TY_COMP) && (e.type == TY_INT) && (e.id != 0))
    {
        warn(MSG_WARN_COMP_RECV_INT, pctx.line_num+1, strip_fn_prefix(v_table[id].name));

        add_instr("I2F_M %s\n", v_table[e.id].name);
        add_instr("SET %s\n"  , v_table[id     ].name);

        add_instr("LOD 0.0\n");
        add_instr("SET %s_i\n", v_table[id     ].name);
    }

    // left comp and right int in the acc -------------------------------------

    if ((v_table[id].type == TY_COMP) && (e.type == TY_INT) && (e.id == 0))
    {
        warn(MSG_WARN_COMP_RECV_INT, pctx.line_num+1, strip_fn_prefix(v_table[id].name));

        add_instr("I2F\n");
        add_instr("SET %s\n"  , v_table[id].name);

        add_instr("LOD 0.0\n");
        add_instr("SET %s_i\n", v_table[id].name);
    }

    // left comp and right float var in memory --------------------------------

    if ((v_table[id].type == TY_COMP) && (e.type == TY_FLOAT) && (e.id != 0))
    {
        warn(MSG_WARN_COMP_RECV_FLOAT, pctx.line_num+1, strip_fn_prefix(v_table[id].name));

        add_instr("LOD %s\n" , v_table[e.id].name);
        add_instr("SET %s\n" , v_table[id     ].name);

        add_instr("LOD 0.0\n");
        add_instr("SET %s_i\n",v_table[id].name);
    }

    // left comp and right float in the acc -----------------------------------

    if ((v_table[id].type == TY_COMP) && (e.type == TY_FLOAT) && (e.id == 0))
    {
        warn(MSG_WARN_COMP_RECV_FLOAT, pctx.line_num+1, strip_fn_prefix(v_table[id].name));

        add_instr("SET %s\n"  , v_table[id].name);

        add_instr("LOD 0.0\n");
        add_instr("SET %s_i\n", v_table[id].name);
    }

    // left comp and right comp const -----------------------------------------

    if ((v_table[id].type == TY_COMP) && (e.type == TY_CCONST))
    {
        get_cmp_cst(e,&etr,&eti);

        add_instr("LOD %s\n",   v_table[etr.id].name);
        add_instr("SET %s\n",   v_table[id      ].name);

        add_instr("LOD %s\n",   v_table[eti.id].name);
        add_instr("SET %s_i\n", v_table[id      ].name);
    }

    // left comp and right comp in memory -------------------------------------

    if ((v_table[id].type == TY_COMP) && (e.type == TY_COMP) && (e.id != 0))
    {
        get_cmp_ets(e,&etr,&eti);

        add_instr("LOD %s\n"  , v_table[etr.id].name);
        add_instr("SET %s\n"  , v_table[id      ].name);

        add_instr("LOD %s\n"  , v_table[eti.id].name);
        add_instr("SET %s_i\n", v_table[id      ].name);
    }

    // left comp and right comp in the acc ------------------------------------

    if ((v_table[id].type == TY_COMP) && (e.type == TY_COMP) && (e.id == 0))
    {
        add_instr("SET_P %s_i\n", v_table[id].name);
        add_instr("SET %s\n"    , v_table[id].name);
    }

    acc_ok     = 0;  // acc released
}

// array assignment with a COMPILE-TIME-CONSTANT index, e.g. x[3] = y;
// Stores into arr[k] directly with the SET_V pseudo-instruction (the assembler
// bakes the offset k into a plain SET to arr_base+k), so there is no index to
// materialise, no stack push and no indirect STI -- one fewer instruction than
// the assign_array path. The walker routes only 1D-forward assignments whose rhs
// has the array's own type here (comp: a variable, a literal or the acc, whose
// imag half is stored first and then popped off the real half); every other
// shape still goes through assign_array. A 2D one whose two indexes are constant
// comes here too (dims 2, k = row * columns + column).
void assign_array_const(int id, int k, int dims, expr e)
{
    // the same checks as assign_array (a write does not mark the array used)
    if (v_table[id].type == TY_NONE)
        fatal(MSG_ERR_DECLARE_VAR_PLEASE, pctx.line_num+1, strip_fn_prefix(v_table[id].name));
    if (v_table[id].dims == AR_NONE)
        fatal(MSG_ERR_NOT_ARRAY, pctx.line_num+1, strip_fn_prefix(v_table[id].name));
    if (v_table[id].dims == AR_2D && dims == 1)   // arr_1d_index refuses it on the general path
        fatal(MSG_ERR_ARRAY_2D, pctx.line_num+1, strip_fn_prefix(v_table[id].name));

    if (v_table[id].type == TY_COMP)
    {
        if (e.id != 0)                                          // comp var or literal
        {
            expr er, ei;
            if (e.type == TY_CCONST) get_cmp_cst(e, &er, &ei); else get_cmp_ets(e, &er, &ei);
            add_instr("LOD %s\n"     , v_table[er.id].name);
            add_instr("SET_V %s %d\n", v_table[id].name, k);
            add_instr("LOD %s\n"     , v_table[ei.id].name);
            add_instr("SET_V %s_i %d\n", v_table[id].name, k);
        }
        else                                                    // comp in the acc
        {
            add_instr("SET_V %s_i %d\n", v_table[id].name, k);
            add_instr("POP\n");
            add_instr("SET_V %s %d\n", v_table[id].name, k);
        }
        acc_ok = 0;  // acc released
        return;
    }

    if (e.id != 0) add_instr("LOD %s\n", v_table[e.id].name);  // rhs -> acc
    add_instr("SET_V %s %d\n", v_table[id].name, k);
    acc_ok = 0;  // acc released
}

// array assignment, e.g. x[i] = y;
// the int variable that is the whole 1D index of the array assignment being
// emitted, when its right-hand side cannot change it (ast.c sets it): a comp
// element is stored in two words, and the second store reads the index again
// instead of keeping it in __aux_var words
int assign_idx_reload = 0;

void assign_array(int id, expr e, int fft)
{
    // test whether it has been declared so an assignment is allowed
    if (v_table[id].type == TY_NONE)
    {
        fatal(MSG_ERR_DECLARE_VAR_PLEASE, pctx.line_num+1, strip_fn_prefix(v_table[id].name));
    }

    // array vs non-array check
    if (v_table[id].dims == AR_NONE)
    {
        fatal(MSG_ERR_NOT_ARRAY, pctx.line_num+1, strip_fn_prefix(v_table[id].name));
    }

    // ------------------------------------------------------------------------
    // perform the set --------------------------------------------------------
    // ------------------------------------------------------------------------

    expr  etr, eti;

    char set_type[16]; if (fft == 0) strcpy(set_type, "STI"); else strcpy(set_type, "ISI");

    // left int and right int in memory ---------------------------------------

    if ((v_table[id].type == TY_INT) && (e.type == TY_INT) && (e.id != 0))
    {
        add_instr("P_LOD %s\n",   v_table[e.id].name);
        add_instr("%s %s\n", set_type, v_table[id].name);
    }

    // left int and right int in the acc --------------------------------------

    if ((v_table[id].type == TY_INT) && (e.type == TY_INT) && (e.id == 0))
    {
        add_instr("%s %s\n", set_type, v_table[id].name);
    }

    // left int and right float in memory -------------------------------------

    if ((v_table[id].type == TY_INT) && (e.type == TY_FLOAT) && (e.id != 0))
    {
        warn(MSG_WARN_INT_RECV_FLOAT, pctx.line_num+1, strip_fn_prefix(v_table[id].name));

        add_instr("P_F2I_M %s\n", v_table[e.id].name);
        add_instr("%s %s\n", set_type, v_table[id].name);
    }

    // left int and right float in the acc ------------------------------------

    if ((v_table[id].type == TY_INT) && (e.type == TY_FLOAT) && (e.id == 0))
    {
        warn(MSG_WARN_INT_RECV_FLOAT, pctx.line_num+1, strip_fn_prefix(v_table[id].name));

        add_instr("F2I\n");
        add_instr("%s %s\n", set_type, v_table[id].name);
    }

    // left int and right comp const ------------------------------------------

    if ((v_table[id].type == TY_INT) && (e.type == TY_CCONST))
    {
        warn(MSG_WARN_ROUND_REAL, pctx.line_num+1);

        get_cmp_cst(e,&etr,&eti);

        add_instr("P_F2I_M %s\n", v_table[etr.id].name);
        add_instr("%s %s\n", set_type,  v_table[id].name);
    }

    // left int and right comp in memory --------------------------------------

    if ((v_table[id].type == TY_INT) && (e.type == TY_COMP) && (e.id != 0))
    {
        warn(MSG_WARN_ROUND_REAL, pctx.line_num+1);

        get_cmp_ets(e,&etr,&eti);

        add_instr("P_F2I_M %s\n", v_table[etr.id].name);
        add_instr("%s %s\n", set_type,  v_table[id].name);
    }

    // left int and right comp in the acc -------------------------------------

    if ((v_table[id].type == TY_INT) && (e.type == TY_COMP) && (e.id == 0))
    {
        warn(MSG_WARN_ROUND_REAL, pctx.line_num+1);

        add_instr("POP\n");
        add_instr("F2I\n");
        add_instr("%s %s\n", set_type, v_table[id].name);
    }

    // left float and right int in memory -------------------------------------

    if ((v_table[id].type == TY_FLOAT) && (e.type == TY_INT) && (e.id != 0))
    {
        warn(MSG_WARN_FLOAT_RECV_INT, pctx.line_num+1, strip_fn_prefix(v_table[id].name));

        add_instr("P_I2F_M %s\n", v_table[e.id].name);
        add_instr("%s %s\n", set_type, v_table[id].name);
    }

    // left float and right int in the acc ------------------------------------

    if ((v_table[id].type == TY_FLOAT) && (e.type == TY_INT) && (e.id == 0))
    {
        warn(MSG_WARN_FLOAT_RECV_INT, pctx.line_num+1, strip_fn_prefix(v_table[id].name));

        add_instr("I2F\n");
        add_instr("%s %s\n", set_type, v_table[id].name);
    }

    // left float and right float in memory -----------------------------------

    if ((v_table[id].type == TY_FLOAT) && (e.type == TY_FLOAT) && (e.id != 0))
    {
        add_instr("P_LOD %s\n",   v_table[e.id].name);
        add_instr("%s %s\n", set_type, v_table[id].name);
    }

    // left float and right float in the acc ----------------------------------

    if ((v_table[id].type == TY_FLOAT) && (e.type == TY_FLOAT) && (e.id == 0))
    {
        add_instr("%s %s\n", set_type, v_table[id].name);
    }

    // left float and right comp const ----------------------------------------

    if ((v_table[id].type == TY_FLOAT) && (e.type == TY_CCONST))
    {
        warn(MSG_WARN_GRAB_REAL_ONLY, pctx.line_num+1);

        get_cmp_cst(e,&etr,&eti);

        add_instr("P_LOD %s\n",  v_table[etr.id].name);
        add_instr("%s %s\n", set_type, v_table[id].name);
    }

    // left float and right comp in memory ------------------------------------

    if ((v_table[id].type == TY_FLOAT) && (e.type == TY_COMP) && (e.id != 0))
    {
        warn(MSG_WARN_GRAB_REAL_ONLY, pctx.line_num+1);

        get_cmp_ets(e,&etr,&eti);

        add_instr("P_LOD %s\n",  v_table[etr.id].name);
        add_instr("%s %s\n", set_type, v_table[id].name);
    }

    // left float and right comp in the acc -----------------------------------

    if ((v_table[id].type == TY_FLOAT) && (e.type == TY_COMP) && (e.id == 0))
    {
        warn(MSG_WARN_GRAB_REAL_ONLY, pctx.line_num+1);

        add_instr("POP\n");
        add_instr("%s %s\n", set_type, v_table[id].name);
    }

    // left comp and right int in memory --------------------------------------

    if ((v_table[id].type == TY_COMP) && (e.type == TY_INT) && (e.id != 0))
    {
        warn(MSG_WARN_COMP_RECV_INT, pctx.line_num+1, strip_fn_prefix(v_table[id].name));

        add_instr("SET __aux_var\n");
        add_instr("P_I2F_M %s\n", v_table[e.id].name);
        add_instr("%s %s\n"  , set_type, v_table[id].name);

        add_instr("LOD __aux_var\n");
        add_instr("P_LOD 0.0\n"  );
        add_instr("%s %s_i\n", set_type, v_table[id].name);
    }

    // left comp and right int in the acc -------------------------------------

    if ((v_table[id].type == TY_COMP) && (e.type == TY_INT) && (e.id == 0) && assign_idx_reload)
    {
        warn(MSG_WARN_COMP_RECV_INT, pctx.line_num+1, strip_fn_prefix(v_table[id].name));

        add_instr("I2F\n");                                          // the index is on the stack
        add_instr("%s %s\n"  , set_type, v_table[id].name);
        add_instr("LOD %s\n" , v_table[assign_idx_reload].name);
        add_instr("P_LOD 0.0\n");
        add_instr("%s %s_i\n", set_type, v_table[id].name);
    }
    else if ((v_table[id].type == TY_COMP) && (e.type == TY_INT) && (e.id == 0))
    {
        warn(MSG_WARN_COMP_RECV_INT, pctx.line_num+1, strip_fn_prefix(v_table[id].name));

        add_instr("I2F\n"                   );
        add_instr("SET_P __aux_var\n"         );
        add_instr("SET   __aux_var2\n"        );
        add_instr("P_LOD __aux_var\n"         );
        add_instr("%s %s\n"  , set_type, v_table[id].name);

        add_instr("LOD   __aux_var2\n"        );
        add_instr("P_LOD 0.0\n"             );
        add_instr("%s %s_i\n", set_type, v_table[id].name);
    }

    // left comp and right float in memory ------------------------------------

    if ((v_table[id].type == TY_COMP) && (e.type == TY_FLOAT) && (e.id != 0))
    {
        warn(MSG_WARN_COMP_RECV_FLOAT, pctx.line_num+1, strip_fn_prefix(v_table[id].name));

        if (!assign_idx_reload) add_instr("SET   __aux_var\n");      // else the index is read again
        add_instr("P_LOD %s\n"  , v_table[e.id].name);
        add_instr("%s %s\n"  , set_type, v_table[id].name);

        add_instr("LOD %s\n", assign_idx_reload ? v_table[assign_idx_reload].name : "__aux_var");
        add_instr("P_LOD 0.0\n"             );
        add_instr("%s %s_i\n", set_type, v_table[id].name);
    }

    // left comp and right float in the acc -----------------------------------

    if ((v_table[id].type == TY_COMP) && (e.type == TY_FLOAT) && (e.id == 0) && assign_idx_reload)
    {
        warn(MSG_WARN_COMP_RECV_FLOAT, pctx.line_num+1, strip_fn_prefix(v_table[id].name));

        add_instr("%s %s\n"  , set_type, v_table[id].name);         // the index is on the stack
        add_instr("LOD %s\n" , v_table[assign_idx_reload].name);
        add_instr("P_LOD 0.0\n");
        add_instr("%s %s_i\n", set_type, v_table[id].name);
    }
    else if ((v_table[id].type == TY_COMP) && (e.type == TY_FLOAT) && (e.id == 0))
    {
        warn(MSG_WARN_COMP_RECV_FLOAT, pctx.line_num+1, strip_fn_prefix(v_table[id].name));

        add_instr("SET_P __aux_var\n"         );
        add_instr("SET   __aux_var2\n"        );
        add_instr("P_LOD __aux_var\n"         );
        add_instr("%s %s\n"  , set_type, v_table[id].name);

        add_instr("LOD   __aux_var2\n"        );
        add_instr("P_LOD 0.0\n"             );
        add_instr("%s %s_i\n", set_type, v_table[id].name);
    }

    // left comp and right comp const -----------------------------------------

    if ((v_table[id].type == TY_COMP) && (e.type == TY_CCONST))
    {
        get_cmp_cst(e,&etr,&eti);

        if (!assign_idx_reload) add_instr("SET   __aux_var\n");      // else the index is read again
        add_instr("P_LOD %s\n"  , v_table[etr.id].name);
        add_instr("%s %s\n" , set_type, v_table[id].name);

        add_instr("LOD %s\n", assign_idx_reload ? v_table[assign_idx_reload].name : "__aux_var");
        add_instr("P_LOD %s\n"   , v_table[eti.id].name);
        add_instr("%s %s_i\n", set_type, v_table[id].name);
    }

    // left comp and right comp in memory -------------------------------------

    if ((v_table[id].type == TY_COMP) && (e.type == TY_COMP) && (e.id != 0))
    {
        get_cmp_ets(e,&etr,&eti);

        if (!assign_idx_reload) add_instr("SET   __aux_var\n");      // else the index is read again
        add_instr("P_LOD %s\n"   , v_table[etr.id].name);
        add_instr("%s %s\n"  , set_type, v_table[id].name);

        add_instr("LOD %s\n", assign_idx_reload ? v_table[assign_idx_reload].name : "__aux_var");
        add_instr("P_LOD %s\n"   , v_table[eti.id].name);
        add_instr("%s %s_i\n", set_type, v_table[id].name);
    }

    // left comp and right comp in the acc ------------------------------------

    if ((v_table[id].type == TY_COMP) && (e.type == TY_COMP) && (e.id == 0) && assign_idx_reload)
    {
        add_instr("SET_P __aux_var\n");                              // the imag aside, the real in the acc
        add_instr("%s %s\n"  , set_type, v_table[id].name);         // over the index on the stack
        add_instr("LOD %s\n" , v_table[assign_idx_reload].name);
        add_instr("P_LOD __aux_var\n");
        add_instr("%s %s_i\n", set_type, v_table[id].name);
    }
    else if ((v_table[id].type == TY_COMP) && (e.type == TY_COMP) && (e.id == 0))
    {
        add_instr("SET_P __aux_var\n" );
        add_instr("SET_P __aux_var2\n");
        add_instr("SET   __aux_var3\n");

        add_instr("P_LOD __aux_var2\n");
        add_instr("%s %s\n"  , set_type, v_table[id].name);

        add_instr("LOD   __aux_var3\n");
        add_instr("P_LOD __aux_var\n" );
        add_instr("%s %s_i\n", set_type, v_table[id].name);
    }

    acc_ok     = 0;  // acc released
}

// ++ operator assignment
void assign_pplus(int id)
{
    pplus2exp(id, 0);
    acc_ok = 0; // acc released
}

// ++ operator assignment on a 1D array
void assign_aplus(int id, expr e)
{
    pplus1d2exp(id, e, 0);
    acc_ok = 0; // acc released
}

// ++ operator assignment on a 2D array
void assign_apl2d(int id, expr e1, expr e2)
{
    pplus2d2exp(id, e1, e2, 0);
    acc_ok = 0; // acc released
}
