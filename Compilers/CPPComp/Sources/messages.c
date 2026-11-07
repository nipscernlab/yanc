// ----------------------------------------------------------------------------
// CPPComp — diagnostics --------------------------------------------------------
// ----------------------------------------------------------------------------

#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>

#include "../Headers/messages.h"
#include "../Headers/cpppp.h"

static const char *g_file = "<input>";

void msg_set_file(const char *name) { g_file = name; }

void msg_error(int line, const char *fmt, ...)
{
    va_list ap;
    const char *file = g_file;
    line = pp_where(line, &file);        // back to the file and line written
    if (line > 0) fprintf(stderr, "%s:%d: %s", file, line, MSG_PREFIX_ERROR);
    else          fprintf(stderr, "%s: %s", file, MSG_PREFIX_ERROR);   // the whole program (#132: no main)
    va_start(ap, fmt); vfprintf(stderr, fmt, ap); va_end(ap);
    fputc('\n', stderr);
    exit(1);
}

void msg_warning(int line, const char *fmt, ...)
{
    va_list ap;
    const char *file = g_file;
    line = pp_where(line, &file);
    if (line > 0) fprintf(stderr, "%s:%d: %s", file, line, MSG_PREFIX_WARNING);
    else          fprintf(stderr, "%s: %s", file, MSG_PREFIX_WARNING);
    va_start(ap, fmt); vfprintf(stderr, fmt, ap); va_end(ap);
    fputc('\n', stderr);
}

// what runs before an internal error aborts: abort skips the atexit
// handlers, and main's removes the outputs of a failed run (#94 N)
static void (*on_internal)(void);
void msg_on_internal(void (*fn)(void)) { on_internal = fn; }

// English only: a defect of cppcomp, for whoever maintains it (messages.h)
void msg_internal(const char *fmt, ...)
{
    va_list ap;
    fprintf(stderr, "cppcomp: internal error: ");
    va_start(ap, fmt); vfprintf(stderr, fmt, ap); va_end(ap);
    fputc('\n', stderr);
    void (*fn)(void) = on_internal;
    on_internal = NULL;                // the hook may fail too: once
    if (fn) fn();
    abort();                           // still an abort: the regression tells it from an error
}
