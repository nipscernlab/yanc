// ----------------------------------------------------------------------------
// data conversion and handling -----------------------------------------------
// ----------------------------------------------------------------------------

// global includes
#include <stdlib.h>

// local includes
#include "../Headers/convert.h"
#include "../Headers/global.h"
#include "../Headers/data_use.h"
#include "../Headers/symbols.h"
#include "../Headers/tpl.h"
#include "../../common/xstr.h"

// ----------------------------------------------------------------------------
// data conversion ------------------------------------------------------------
// ----------------------------------------------------------------------------

// converts the integer x to a binary string of its w low bits (w <= 31: the
// line numbers of pc_<name>_mem.txt, 20 bits, are its only use)
char *int_to_bin(int x, int w)
{
    char *b = (char *) xmalloc(w+1);
    int n = 0;
    for (int z = 1 << (w - 1); z > 0; z >>= 1)
        b[n++] = ((x & z) == z) ? '1' : '0';
    b[n] = '\0';
    return b;
}

// ----------------------------------------------------------------------------
// helper functions for accessing terminals -----------------------------------
// ----------------------------------------------------------------------------

// gets the id of the imag part of a complex var
// the real part is in the id parameter
int get_img_id(int id)
{
    char *name = xasprintf("%s_i", v_table[id].name);

    if (find_var(name) == -1) add_var(name);
    int idi = find_var(name);
    free(name);
    return idi;
}

// splits the real and imaginary parts of a complex constant
// generating two floating-point entries in the table
void get_cmp_cst(expr e, expr *er, expr *ei)
{
    char *re, *im;

    cnum_split(v_table[e.id].name, &re, &im);   // the text, not a host float: no digit lost

    *er = num2exp(lex_fnum(re), TY_FLOAT);   // typed constants: real(5-7i) can be a built-in's argument
    *ei = num2exp(lex_fnum(im), TY_FLOAT);
    free(re); free(im);
}

// the two parts of a comp in memory, a constant (two float literals) or a
// variable (it and its _i): one place, so one code path (tpl.c, oper.c)
void get_cmp_parts(expr e, expr *er, expr *ei)
{
    if (e.type == TY_CCONST) get_cmp_cst(e, er, ei);
    else                     get_cmp_ets(e, er, ei);
}

// generates the extended float exprs for the real and imag parts
// of a complex number in memory
void get_cmp_ets(expr e, expr *er, expr *ei)
{
    *er = expr_make(TY_FLOAT, e.id);
    *ei = expr_make(TY_FLOAT, get_img_id(e.id));
}

// ----------------------------------------------------------------------------
// an operand loaded as a type (#108 C2) ----------------------------------------
// ----------------------------------------------------------------------------
// The value of an argument (par_check) or of a returned expression
// (declare_ret) goes into the acc as the type it is given to: an int, a
// float, or a comp (real, then imaginary on top, as a comp in the acc is).
// Row: the type it becomes, by its place in memory (IM an int, FM a float,
// CM a comp; the other rows are no type); column: the operand's place. A
// NULL cell is an operand that is already what it becomes, in the acc. The
// caller gives the warning of a conversion, which names its own context.
static const char *const load_tpl[PL_N][PL_N] = {
  /* int   */ {"$L $b", NULL, "$F $b", "F2I", "$F $br", "POP\nF2I"},
  /* -     */ {NULL, NULL, NULL, NULL, NULL, NULL},
  /* float */ {"$C $b", "I2F", "$L $b", NULL, "$L $br", "POP"},
  /* -     */ {NULL, NULL, NULL, NULL, NULL, NULL},
  /* comp  */ {"$C $b\nP_LOD 0.0", "I2F\nP_LOD 0.0", "$L $b\nP_LOD 0.0", "P_LOD 0.0", "$L $br\nP_LOD $bi", NULL},
  /* -     */ {NULL, NULL, NULL, NULL, NULL, NULL},
};

void load_as(int ty, expr e)
{
    int row = ty == TY_INT ? PL_IM : ty == TY_FLOAT ? PL_FM : PL_CM;
    emit_tpl(load_tpl[row][place_of(e)], expr_make(ty, 0), e);
}
