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

const char *prname = ""; // processor name: the source file's (main sets it)
// Defaults: the one set shared with asmcomp, cppcomp and the HDL (audit 1.7);
// only reached when the program omits the directive.
int  nubits  = 32; // word width (bits): an int literal in hex, octal or binary is a bit pattern of this word
int  nbmant  = 23; // mantissa width (bits)
int  nbexpo  =  8; // exponent width (bits)
int  nuioin  =  1; // number of input ports
int  nuioou  =  1; // number of output ports
int  fround  =  0; // float rounding level (#FROUND)
int  fftsiz  =  3; // low bits of an index v[k) reverses (#FFTSIZ; asmcomp's default too)

// 2^NBMANT, one past the largest mantissa. Only float code uses it, and
// asmcomp refuses a program with float whose #NBMANT passes 29 (#NUBITS of at
// most 32 = NBMANT + NBEXPO + 1, NBEXPO of at least 2: docs/directives.md).
// cmmcomp reads the directive before that check, so the shift is held where it
// is defined for any #NBMANT (#94 J): 1L << 31 is undefined where long has 32
// bits, as on 64-bit Windows
long long mant_pow2(void)
{
    int b = nbmant < 0 ? 0 : nbmant > 62 ? 62 : nbmant;
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
    // what the compiler reads; #FROUND and #NUGAIN are validated here
    // (asmcomp validates them again: it is the gate every front end passes
    // through; this one just knows the source line)
    switch(t)
    {
        case DIR_ASM_ONLY: break;
        case DIR_NUBITS: nubits = ival             ; break;
        case DIR_FFTSIZ: fftsiz = ival             ; break;
        case DIR_NBMANT: nbmant = ival             ; break;
        case DIR_NBEXPO: nbexpo = ival             ; break;
        case DIR_NUIOIN: nuioin = ival             ; break;
        case DIR_NUIOOU: nuioou = ival             ; break;
        case DIR_FROUND: if (ival < 0 || ival > 2) fatal(MSG_ERR_FROUND_RANGE, line_num+1); fround = ival; break; // #FROUND level
        case DIR_NUGAIN: if (ival <= 0 || (ival & (ival - 1)) != 0) fatal(MSG_ERR_NUGAIN_POW2, line_num+1, ival); break;
    }

    stmt_append(stmt_directive(dir, id));
}
