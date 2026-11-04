// ----------------------------------------------------------------------------
// Gathers functions and state variables driven by directives -----------------
// ----------------------------------------------------------------------------

// global includes
#include <string.h>
#include <stdlib.h>

// local includes
#include "../Headers/directives.h"
#include "../Headers/ast.h"
#include "../Headers/convert.h"
#include "../Headers/global.h"
#include "../Headers/functions.h"
#include "../Headers/symbols.h"
#include "../Headers/messages.h"
#include "../Headers/diag.h"

// ----------------------------------------------------------------------------
// global variable definitions ------------------------------------------------
// ----------------------------------------------------------------------------

target_cfg cfg = {
    .prname = "",
    .nubits = 32,
    .nbmant = 23,
    .nbexpo =  8,
    .nuioin =  1,
    .nuioou =  1,
    .fround =  0,
    .fftsiz =  3,
};

// 2^NBMANT, one past the largest mantissa. Only float code uses it, and
// asmcomp refuses a program with float whose #NBMANT passes 29 (#NUBITS of at
// most 32 = NBMANT + NBEXPO + 1, NBEXPO of at least 2: docs/directives.md).
// cmmcomp reads the directive before that check, so the shift is held where it
// is defined for any #NBMANT (#94 J): 1L << 31 is undefined where long has 32
// bits, as on 64-bit Windows
long long mant_pow2(void)
{
    int b = cfg.nbmant < 0 ? 0 : cfg.nbmant > 62 ? 62 : cfg.nbmant;
    return 1LL << b;
}

// ----------------------------------------------------------------------------
// Directive handling ---------------------------------------------------------
// ----------------------------------------------------------------------------

// Updates parse-time compiler state for the directive (every subsequent rule
// reads these globals to make decisions before the AST walker runs), then
// appends a STMT_DIRECTIVE to the open stmt_list so the emit fires from
// the global walker at `fim`.
void directive_exec(char *dir, int id, dir_code t)
{
    int ival = atoi(v_table[id].name);

    // #NBMANT, #NBEXPO, #NUIOIN, #NUIOOU, #FROUND, #NUBITS and #FFTSIZ set
    // what the compiler reads; #NUBITS, #FFTSIZ, #FROUND and #NUGAIN are
    // validated here, since the walker shifts by the first two
    // (asmcomp validates them again: it is the gate every front end passes
    // through; this one just knows the source line)
    switch(t)
    {
        case DIR_ASM_ONLY: break;
        case DIR_NUBITS: if (ival < 2 || ival > 32) fatal(MSG_ERR_NUBITS_RANGE, pctx.line_num+1, ival); cfg.nubits = ival; break;
        case DIR_FFTSIZ: if (ival < 1 || ival > 30) fatal(MSG_ERR_FFTSIZ_RANGE, pctx.line_num+1, ival); cfg.fftsiz = ival; break;
        case DIR_NBMANT: cfg.nbmant = ival             ; break;
        case DIR_NBEXPO: cfg.nbexpo = ival             ; break;
        case DIR_NUIOIN: cfg.nuioin = ival             ; break;
        case DIR_NUIOOU: cfg.nuioou = ival             ; break;
        case DIR_FROUND: if (ival < 0 || ival > 2) fatal(MSG_ERR_FROUND_RANGE, pctx.line_num+1); cfg.fround = ival; break; // #FROUND level
        case DIR_NUGAIN: if (ival <= 0 || (ival & (ival - 1)) != 0) fatal(MSG_ERR_NUGAIN_POW2, pctx.line_num+1, ival); break;
        default: fatal(MSG_ERR_INTERNAL, "directive_exec", pctx.line_num+1);
    }

    stmt_append(stmt_directive(dir, id));
}
