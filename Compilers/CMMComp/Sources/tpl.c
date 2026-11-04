// global includes
#include <stdlib.h>
#include <string.h>

// local includes
#include "../Headers/tpl.h"
#include "../Headers/acc.h"
#include "../Headers/convert.h"
#include "../Headers/global.h"
#include "../Headers/symbols.h"
#include "../../common/xstr.h"

// ----------------------------------------------------------------------------
// operators as tables (step P10 of #93) ---------------------------------------
// ----------------------------------------------------------------------------
// An operand is in one of six places: an int, a float or a comp, in memory or
// in the accumulator. A comp constant and a comp variable are the same place,
// a comp in memory: both are a pair of names, the real part and the imaginary
// one. An operator over two operands is then a table of 6 x 6 templates, one
// instruction a line, in which
//   $a $b    the names of the left and the right operand (in memory),
//   $ar $ai  the real and imaginary parts of the left one (a comp in memory),
//   $br $bi  the same of the right one,
//   $L       LOD, or P_LOD when the acc holds a value to keep (acc_live),
//   $C       I2F_M, or P_I2F_M the same way,
//   $1 .. $4 mnemonics the operator passes (emit_tpl_ops: a comparison's).
// A NULL template emits nothing (a pair the walker never hands over).

int place_of(expr e)
{
    int mem = (e.id != 0);
    switch (e.type) {
        case TY_INT:    return mem ? PL_IM : PL_IA;
        case TY_FLOAT:  return mem ? PL_FM : PL_FA;
        case TY_CCONST: return PL_CM;
        default:        return mem ? PL_CM : PL_CA;   // TY_COMP
    }
}

// the two parts of a comp in memory: of a constant (two float literals) or of
// a variable (it and its _i), as the hand-written branches got them
static void comp_parts(expr e, expr *re, expr *im)
{
    if (e.type == TY_CCONST) get_cmp_cst(e, re, im);
    else                     get_cmp_ets(e, re, im);
}

// emits template t for the operands e1 and e2 (see above), with the
// mnemonics ops[0..3] for $1..$4 (NULL: none)
void emit_tpl_ops(const char *t, expr e1, expr e2, const char *const *ops)
{
    if (!t) return;
    expr ar = {0, 0}, ai = {0, 0}, br = {0, 0}, bi = {0, 0};
    if (place_of(e1) == PL_CM && (strstr(t, "$ar") || strstr(t, "$ai"))) comp_parts(e1, &ar, &ai);
    if (place_of(e2) == PL_CM && (strstr(t, "$br") || strstr(t, "$bi"))) comp_parts(e2, &br, &bi);
    while (*t)
    {
        const char *end = strchr(t, '\n');
        size_t n = end ? (size_t)(end - t) : strlen(t);
        char *line = xstrdup("");
        for (size_t k = 0; k < n; )
        {
            const char *w = NULL; size_t used = 1;
            if (t[k] == '$') {
                const char *r = t + k + 1;
                if      (strncmp(r, "ar", 2) == 0) {w = v_table[ar.id].name; used = 3;}
                else if (strncmp(r, "ai", 2) == 0) {w = v_table[ai.id].name; used = 3;}
                else if (strncmp(r, "br", 2) == 0) {w = v_table[br.id].name; used = 3;}
                else if (strncmp(r, "bi", 2) == 0) {w = v_table[bi.id].name; used = 3;}
                else if (*r == 'a') {w = v_table[e1.id].name; used = 2;}
                else if (*r == 'b') {w = v_table[e2.id].name; used = 2;}
                else if (*r == 'L') {w = acc_live ? "P_LOD"   : "LOD";   used = 2;}
                else if (*r == 'C') {w = acc_live ? "P_I2F_M" : "I2F_M"; used = 2;}
                else if (ops && *r >= '1' && *r <= '4') {w = ops[*r - '1']; used = 2;}
            }
            char *nl = w ? xasprintf("%s%s", line, w) : xasprintf("%s%c", line, t[k]);
            free(line); line = nl;
            k += used;
        }
        add_instr("%s\n", line);
        free(line);
        t += n + (end ? 1 : 0);
    }
}

void emit_tpl(const char *t, expr e1, expr e2) {emit_tpl_ops(t, e1, e2, NULL);}

