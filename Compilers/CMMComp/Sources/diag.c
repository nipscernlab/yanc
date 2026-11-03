// ----------------------------------------------------------------------------
// diagnostics: the one way cmmcomp stops on an error -------------------------
// ----------------------------------------------------------------------------

// global includes
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

// local includes
#include "../Headers/diag.h"

void fatal(const char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    vfprintf(stderr, fmt, ap);
    va_end(ap);
    exit(EXIT_FAILURE);
}
