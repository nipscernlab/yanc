// ----------------------------------------------------------------------------
// interrupt handling ---------------------------------------------------------
// ----------------------------------------------------------------------------

#include <stdio.h>
#include <stdlib.h>

#include "../Headers/global.h"
#include "../Headers/messages.h"

int itr_ok    = 0; // tells whether an interrupt has already been used
int itr_seen  = 0; // the program has a #PRACA (set while parsing, before any code is emitted)
int toaqui_ok = 0; // tells whether a #TOAQUI marker has already been used

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

// emits the #TOAQUI directive
// records the address that the hardware will compare against the PC; whenever
// they match the cheguei output pin asserts. Only one #TOAQUI per program.
void dire_toaqui()
{
    if (toaqui_ok == 1) {fprintf(stderr, MSG_ERR_DUP_TOAQUI, line_num+1); exit(EXIT_FAILURE);}

    printf(MSG_INFO_TOAQUI, line_num+1);

    add_sinst(0, "#TOAQUI\n");
    toaqui_ok = 1;
}
