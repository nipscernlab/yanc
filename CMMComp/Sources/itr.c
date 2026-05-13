// ----------------------------------------------------------------------------
// tratamento de interrupcao --------------------------------------------------
// ----------------------------------------------------------------------------

#include <stdio.h>
#include <stdlib.h>

#include "..\Headers\global.h"
#include "..\Headers\messages.h"

int itr_ok = 0; // se ja usou ou nao interrupcao

// gera diretiva #ITRAD
// ainda tenho q checar os lugares q nao podem ter isso
// ex: dentro de loop, dentro de switch case, pensar ...
// talvez um warning ja sirva
void dire_inter()
{
    if (itr_ok == 1) {fprintf(stderr, MSG_ERR_DUP_INTERRUPT, line_num+1); exit(EXIT_FAILURE);}

    printf(MSG_INFO_INTERRUPT_DIRECTIVE, line_num+1);

    add_sinst(0, "#ITRAD\n");
    itr_ok = 1;
}