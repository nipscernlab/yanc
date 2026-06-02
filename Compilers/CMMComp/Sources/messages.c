// ----------------------------------------------------------------------------
// suporte bilingue PT/EN para as mensagens do cmmcomp ------------------------
// ----------------------------------------------------------------------------

#include <string.h>

#include "../Headers/messages.h"

int lang_en = 0; // default: portugues

// procura -en/-pt em argv, seta lang_en e remove a flag
// mantem o resto dos argumentos na ordem original
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
