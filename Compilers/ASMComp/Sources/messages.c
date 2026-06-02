// ----------------------------------------------------------------------------
// PT/EN bilingual support for asmcomp messages -------------------------------
// ----------------------------------------------------------------------------

#include <string.h>

#include "../Headers/messages.h"

int lang_en = 0; // default: Portuguese

// scans argv for -en/-pt, sets lang_en and removes the flag
// preserves the remaining arguments in their original order
void parse_lang_flag(int *argc, char **argv)
{
    int w = 1;
    for (int i = 1; i < *argc; i++)
    {
        if      (strcmp(argv[i], "-en") == 0) lang_en = 1;
        else if (strcmp(argv[i], "-pt") == 0) lang_en = 0;
        else argv[w++] = argv[i];
    }
    *argc = w;
}
