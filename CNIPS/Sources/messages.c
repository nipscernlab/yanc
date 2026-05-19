// ----------------------------------------------------------------------------
// CNIPS — diagnostics --------------------------------------------------------
// ----------------------------------------------------------------------------

#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>

#include "..\Headers\messages.h"

static const char *g_file = "<input>";
static int         g_errs = 0;

void msg_set_file(const char *name) { g_file = name; }
int  msg_error_count(void)          { return g_errs; }

void msg_error(int line, const char *fmt, ...)
{
    va_list ap;
    fprintf(stderr, "%s:%d: error: ", g_file, line);
    va_start(ap, fmt); vfprintf(stderr, fmt, ap); va_end(ap);
    fputc('\n', stderr);
    g_errs++;
    exit(1);
}

void msg_warning(int line, const char *fmt, ...)
{
    va_list ap;
    fprintf(stderr, "%s:%d: warning: ", g_file, line);
    va_start(ap, fmt); vfprintf(stderr, fmt, ap); va_end(ap);
    fputc('\n', stderr);
}

void msg_internal(const char *fmt, ...)
{
    va_list ap;
    fprintf(stderr, "cnips: internal error: ");
    va_start(ap, fmt); vfprintf(stderr, fmt, ap); va_end(ap);
    fputc('\n', stderr);
    abort();
}
