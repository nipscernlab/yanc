// ----------------------------------------------------------------------------
// data conversion and handling -----------------------------------------------
// ----------------------------------------------------------------------------

// global includes
#include <stdlib.h>

// local includes
#include "../Headers/t2t.h"
#include "../Headers/global.h"
#include "../Headers/data_use.h"
#include "../Headers/variaveis.h"
#include "../../common/xstr.h"

// ----------------------------------------------------------------------------
// data conversion ------------------------------------------------------------
// ----------------------------------------------------------------------------

// converts the integer x to a binary string of its w low bits (w <= 31: the
// line numbers of pc_<name>_mem.txt, 20 bits, are its only use)
char *itob(int x, int w)
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

    *er = num2exp(exec_fnum(re), 2);   // typed constants: real(5-7i) can be a built-in's argument
    *ei = num2exp(exec_fnum(im), 2);
    free(re); free(im);
}

// generates the extended float exprs for the real and imag parts
// of a complex number in memory
void get_cmp_ets(expr e, expr *er, expr *ei)
{
    *er = expr_make(TY_FLOAT, e.id);
    *ei = expr_make(TY_FLOAT, get_img_id(e.id));
}
