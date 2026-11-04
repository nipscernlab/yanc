// ----------------------------------------------------------------------------
// gathers functions and variables driven by directives -----------------------
// ----------------------------------------------------------------------------

// the directives: what each sets (directive_exec writes cfg, context.h) and
// the walker's emission of them

#ifndef CMM_DIRECTIVES_H
#define CMM_DIRECTIVES_H

#include "context.h"

long long mant_pow2(void); // 2^NBMANT, defined for any #NBMANT

// directive parsing
// what a configuration directive sets in cmmcomp (DIR_ASM_ONLY: nothing,
// only asmcomp reads it: #NDSTAC, #SDEPTH)
typedef enum {
    DIR_ASM_ONLY, DIR_NBMANT, DIR_NBEXPO, DIR_NUIOIN, DIR_NUIOOU,
    DIR_FROUND, DIR_NUGAIN, DIR_NUBITS, DIR_FFTSIZ
} dir_code;

void directive_exec (const char *dir, int id, dir_code t);

#endif // CMM_DIRECTIVES_H
