// ----------------------------------------------------------------------------
// interrupt handling ---------------------------------------------------------
// ----------------------------------------------------------------------------

#include <stdio.h>
#include <stdlib.h>

#include "..\Headers\global.h"
#include "..\Headers\messages.h"

int itr_ok = 0; // tells whether an interrupt has already been used

// emits the #ITRAD directive
// still need to check the places where this is not allowed
// e.g. inside a loop, inside a switch case, etc.
// a warning may already be enough
void dire_inter()
{
    if (itr_ok == 1) {fprintf(stderr, MSG_ERR_DUP_INTERRUPT, line_num+1); exit(EXIT_FAILURE);}

    printf(MSG_INFO_INTERRUPT_DIRECTIVE, line_num+1);

    add_sinst(0, "#ITRAD\n");
    itr_ok = 1;
}
