// ----------------------------------------------------------------------------
// diagnostics: errors that stop cmmcomp, warnings and notes ------------------
// ----------------------------------------------------------------------------

// global includes
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

// local includes
#include "../Headers/diag.h"

static void (*on_fatal)(void) = NULL;

void fatal_hook(void (*fn)(void))
{
    on_fatal = fn;
}

void fatal(const char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    vfprintf(stderr, fmt, ap);
    va_end(ap);
    void (*fn)(void) = on_fatal;
    on_fatal = NULL;                  // an error inside the hook ends here
    if (fn) fn();
    exit(EXIT_FAILURE);
}

void warn(const char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    vfprintf(stdout, fmt, ap);
    va_end(ap);
}
