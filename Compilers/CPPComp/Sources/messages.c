// ----------------------------------------------------------------------------
// CPPComp — diagnostics --------------------------------------------------------
// ----------------------------------------------------------------------------

#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>

#include "../Headers/messages.h"
#include "../Headers/cpppp.h"

static const char *g_file = "<input>";
static int         g_errs = 0;

void msg_set_file(const char *name) { g_file = name; }
int  msg_error_count(void)          { return g_errs; }

void msg_error(int line, const char *fmt, ...)
{
    va_list ap;
    const char *file = g_file;
    line = pp_where(line, &file);        // back to the file and line written
    fprintf(stderr, "%s:%d: %s", file, line, MSG_PREFIX_ERROR);
    va_start(ap, fmt); vfprintf(stderr, fmt, ap); va_end(ap);
    fputc('\n', stderr);
    g_errs++;
    exit(1);
}

void msg_warning(int line, const char *fmt, ...)
{
    va_list ap;
    const char *file = g_file;
    line = pp_where(line, &file);
    fprintf(stderr, "%s:%d: %s", file, line, MSG_PREFIX_WARNING);
    va_start(ap, fmt); vfprintf(stderr, fmt, ap); va_end(ap);
    fputc('\n', stderr);
}

// English only: a defect of cppcomp, for whoever maintains it (messages.h)
void msg_internal(const char *fmt, ...)
{
    va_list ap;
    fprintf(stderr, "cppcomp: internal error: ");
    va_start(ap, fmt); vfprintf(stderr, fmt, ap); va_end(ap);
    fputc('\n', stderr);
    abort();
}
