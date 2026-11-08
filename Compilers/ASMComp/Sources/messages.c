// ----------------------------------------------------------------------------
// PT/EN bilingual support for asmcomp messages -------------------------------
// ----------------------------------------------------------------------------

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

#include "../Headers/messages.h"
#include "../../common/yanc_lang.h"

int lang_en = 1; // English unless YANC_LANG or --lang says pt

// --lang=en|pt (and YANC_LANG): sets lang_en and takes the option out of
// argv, the other arguments keeping their order (common/yanc_lang.c)
void parse_lang_flag(int *argc, char **argv)
{
    lang_en = yanc_lang(argc, argv, "asmcomp");
}

void fatal(const char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    vfprintf(stderr, fmt, ap);
    va_end(ap);
    exit(EXIT_FAILURE);
}
