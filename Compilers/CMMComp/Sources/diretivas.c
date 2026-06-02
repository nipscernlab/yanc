// ----------------------------------------------------------------------------
// Gathers functions and state variables driven by directives -----------------
// ----------------------------------------------------------------------------

// global includes
#include <string.h>
#include <stdlib.h>

// local includes
#include "../Headers/ast.h"
#include "../Headers/t2t.h"
#include "../Headers/global.h"
#include "../Headers/funcoes.h"
#include "../Headers/variaveis.h"

// ----------------------------------------------------------------------------
// global variable definitions ------------------------------------------------
// ----------------------------------------------------------------------------

char prname[128] ; // processor name
int  nbmant  = 16; // mantissa width (bits)
int  nbexpo  =  6; // exponent width (bits)
int  nuioin  =  1; // number of input ports
int  nuioou  =  1; // number of output ports

// ----------------------------------------------------------------------------
// Directive handling ---------------------------------------------------------
// ----------------------------------------------------------------------------

// Updates parse-time compiler state for the directive (every subsequent rule
// reads these globals to make decisions before the AST walker runs), then
// appends a STMT_DIRECTIVE to the open stmt_list so the emit fires from
// the global walker at `fim`.
void dire_exec(char *dir, int id, int t)
{
    int ival = atoi(v_table[id].name);

    // action to take depending on the directive
    // only directives 1, 3 and 4 affect the cmm compiler
    switch(t)
    {
        case  1: strcpy (prname,v_table[id].name); break;
        case  3: nbmant = ival             ; break;
        case  4: nbexpo = ival             ; break;
        case  7: nuioin = ival             ; break;
        case  8: nuioou = ival             ; break;
    }

    stmt_append(stmt_directive(dir, id));
}
