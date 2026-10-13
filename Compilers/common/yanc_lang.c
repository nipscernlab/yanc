// ----------------------------------------------------------------------------
// The message language of every YANC tool (see yanc_lang.h) -------------------
// ----------------------------------------------------------------------------

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "yanc_lang.h"

// 1 for en, 0 for pt, -1 for anything else
static int lang_of(const char *v)
{
    if (!strcmp(v, "en")) return 1;
    if (!strcmp(v, "pt")) return 0;
    return -1;
}

static void bad_lang(const char *tool, const char *where, const char *v)
{
    fprintf(stderr, "%s: %s takes en or pt, not '%s'\n", tool, where, v);
    exit(EXIT_FAILURE);
}

int yanc_lang_en = 1;

int yanc_lang(int *argc, char **argv, const char *tool)
{
    int en = 1;                                     // English by default

    const char *env = getenv("YANC_LANG");
    if (env && *env)
    {
        en = lang_of(env);
        if (en < 0) bad_lang(tool, "YANC_LANG", env);
    }

    int w = 1;
    for (int i = 1; i < *argc; i++)
    {
        const char *v = NULL;
        if      (!strncmp(argv[i], "--lang=", 7))            v = argv[i] + 7;
        else if (!strcmp (argv[i], "--lang") && i + 1 < *argc) v = argv[++i];
        else if (!strcmp (argv[i], "--lang"))                bad_lang(tool, "--lang", "");
        else { argv[w++] = argv[i]; continue; }

        en = lang_of(v);
        if (en < 0) bad_lang(tool, "--lang", v);
    }
    *argc = w;
    argv[w] = NULL;
    yanc_lang_en = en;
    return en;
}
