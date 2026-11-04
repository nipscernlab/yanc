// ----------------------------------------------------------------------------
// Strings and tables of any length, for every YANC tool (xstr.h) -------------
// ----------------------------------------------------------------------------

#include <stdarg.h>
#include <stdlib.h>
#include <string.h>

#include "xstr.h"
#include "yanc_lang.h"

static void (*on_oom)(void) = NULL;

void xstr_oom_hook(void (*fn)(void))
{
    on_oom = fn;
}

static void out_of_memory(void)
{
    fprintf(stderr, yanc_lang_en ? "Error: out of memory.\n"
                                 : "Erro: memória insuficiente.\n");
    void (*fn)(void) = on_oom;
    on_oom = NULL;                    // the hook may allocate: a second failure ends here
    if (fn) fn();
    exit(EXIT_FAILURE);
}

void *xmalloc(size_t n)
{
    return xrealloc(NULL, n);
}

void *xcalloc(size_t n, size_t size)
{
    void *q = calloc(n ? n : 1, size ? size : 1);
    if (!q) out_of_memory();
    return q;
}

void *xrealloc(void *p, size_t n)
{
    void *q = realloc(p, n ? n : 1);
    if (!q) out_of_memory();
    return q;
}

char *xstrdup(const char *s)
{
    size_t n = strlen(s) + 1;
    char *d = xrealloc(NULL, n);
    memcpy(d, s, n);
    return d;
}

char *xvasprintf(const char *fmt, va_list ap)
{
    va_list aq;
    va_copy(aq, ap);
    int n = vsnprintf(NULL, 0, fmt, aq);             // how long the text is
    va_end(aq);
    if (n < 0) out_of_memory();                      // only an encoding error gets here
    char *s = xrealloc(NULL, (size_t)n + 1);
    vsnprintf(s, (size_t)n + 1, fmt, ap);
    return s;
}

char *xasprintf(const char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    char *s = xvasprintf(fmt, ap);
    va_end(ap);
    return s;
}

char *xgetline(FILE *f)
{
    size_t cap = 128, len = 0;
    char *s = xrealloc(NULL, cap);
    int c;
    while ((c = fgetc(f)) != EOF && c != '\n')
    {
        if (len + 1 >= cap) s = xrealloc(s, cap *= 2);
        s[len++] = (char)c;
    }
    if (c == EOF && len == 0) {free(s); return NULL;}
    if (len > 0 && s[len - 1] == '\r') len--;
    s[len] = '\0';
    return s;
}
