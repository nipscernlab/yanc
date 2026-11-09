// ----------------------------------------------------------------------------
// interrupt handling ---------------------------------------------------------
// ----------------------------------------------------------------------------

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../Headers/interrupt.h"
#include "../Headers/global.h"
#include "../Headers/messages.h"
#include "../Headers/diag.h"
#include "../Headers/symbols.h"

static int itr_ok    = 0; // tells whether an interrupt has already been used
int itr_seen  = 0; // the program has a #PRACA (set while parsing, before any code is emitted)
static int toaqui_ok = 0; // tells whether a #TOAQUI marker has already been used

// emits the #ITRAD directive
// #PRACA is a restart point: the itr pulse empties both stacks and the
// program runs again from here (#188), so it goes in main, the only function
// no CAL enters. asmcomp checks the rest on the .asm: both stacks empty right
// before the point, on every path that reaches it.
void directive_inter(void)
{
    if (itr_ok == 1) fatal(MSG_ERR_DUP_INTERRUPT, pctx.line_num+1);
    if (strcmp(sym(pctx.fun_parse)->name, "main") != 0)
        fatal(MSG_ERR_PRACA_NOT_MAIN, pctx.line_num+1, sym(pctx.fun_parse)->name);

    warn(MSG_INFO_INTERRUPT_DIRECTIVE, pctx.line_num+1);

    add_sinst(0, "#ITRAD\n");
    itr_ok = 1;
}

// emits the #TOAQUI directive
// records the address that the hardware will compare against the PC; whenever
// they match the cheguei output pin asserts. Only one #TOAQUI per program.
void directive_toaqui(void)
{
    if (toaqui_ok == 1) fatal(MSG_ERR_DUP_TOAQUI, pctx.line_num+1);

    warn(MSG_INFO_TOAQUI, pctx.line_num+1);

    add_sinst(0, "#TOAQUI\n");
    toaqui_ok = 1;
}
