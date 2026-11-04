// global includes
#include <stdlib.h>
#include <string.h>

// local includes
#include "../Headers/tpl.h"
#include "../Headers/acc.h"
#include "../Headers/convert.h"
#include "../Headers/global.h"
#include "../Headers/symbols.h"
#include "../Headers/messages.h"
#include "../Headers/diag.h"
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
//   $F       F2I_M, or P_F2I_M the same way,
//   $1 .. $9 words the emitter passes (emit_tpl_ops): a comparison's
//            mnemonics, an array's name or its number of columns.
// A line ?MSG_NAME is no instruction: the warning MSG_NAME, with the line,
// given where the hand-written branch gave it (tpl_warn below).
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

// the warnings a template may give, by name: each takes the line only
static void tpl_warn(const char *name)
{
    static const struct { const char *name; int id; } known[] = {
        {"MSG_WARN_IDX1_FLOAT", 1}, {"MSG_WARN_IDX2_FLOAT", 2}, {"MSG_WARN_IDXS_FLOAT", 3},
        {"MSG_WARN_IDXS_MESS", 4}, {"MSG_WARN_IDX_COMP_GRAB", 5}, {"MSG_WARN_IDX_COMP_ROUND", 6},
        {"MSG_WARN_IDX_FLOAT", 7}, {"MSG_WARN_IDX_FLOAT_HEAVY", 8},
    };
    int id = 0;
    for (size_t k = 0; k < sizeof known / sizeof known[0]; k++)
        if (strcmp(known[k].name, name) == 0) id = known[k].id;
    const char *msg = id == 1 ? MSG_WARN_IDX1_FLOAT    : id == 2 ? MSG_WARN_IDX2_FLOAT
                    : id == 3 ? MSG_WARN_IDXS_FLOAT    : id == 4 ? MSG_WARN_IDXS_MESS
                    : id == 5 ? MSG_WARN_IDX_COMP_GRAB : id == 6 ? MSG_WARN_IDX_COMP_ROUND
                    : id == 7 ? MSG_WARN_IDX_FLOAT     : id == 8 ? MSG_WARN_IDX_FLOAT_HEAVY : NULL;
    if (!msg) fatal(MSG_ERR_INTERNAL, name, pctx.line_num+1);   // a template of the compiler's own, not the user's
    warn(msg, pctx.line_num+1);
}

// emits template t for the operands e1 and e2 (see above), with the
// mnemonics ops[0..3] for $1..$4 (NULL: none)
void emit_tpl_ops(const char *t, expr e1, expr e2, const char *const *ops)
{
    if (!t) return;
    expr ar = {0, 0}, ai = {0, 0}, br = {0, 0}, bi = {0, 0};
    if (place_of(e1) == PL_CM && (strstr(t, "$ar") || strstr(t, "$ai"))) get_cmp_parts(e1, &ar, &ai);
    if (place_of(e2) == PL_CM && (strstr(t, "$br") || strstr(t, "$bi"))) get_cmp_parts(e2, &br, &bi);
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
                else if (*r == 'F') {w = acc_live ? "P_F2I_M" : "F2I_M"; used = 2;}
                else if (ops && *r >= '1' && *r <= '9') {w = ops[*r - '1']; used = 2;}
            }
            char *nl = w ? xasprintf("%s%s", line, w) : xasprintf("%s%c", line, t[k]);
            free(line); line = nl;
            k += used;
        }
        if (line[0] == '?') tpl_warn(line + 1);
        else                add_instr("%s\n", line);
        free(line);
        t += n + (end ? 1 : 0);
    }
}

void emit_tpl(const char *t, expr e1, expr e2) {emit_tpl_ops(t, e1, e2, NULL);}

