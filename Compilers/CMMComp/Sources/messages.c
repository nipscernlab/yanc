// ----------------------------------------------------------------------------
// suporte bilingue PT/EN para as mensagens do cmmcomp ------------------------
// ----------------------------------------------------------------------------

#include "../Headers/messages.h"
#include "../../common/yanc_lang.h"

int lang_en = 1; // English unless YANC_LANG or --lang says pt

// --lang=en|pt (and YANC_LANG): sets lang_en and takes the option out of
// argv, the other arguments keeping their order (common/yanc_lang.c)
void parse_lang_flag(int *argc, char **argv)
{
    lang_en = yanc_lang(argc, argv, "cmmcomp");
}
