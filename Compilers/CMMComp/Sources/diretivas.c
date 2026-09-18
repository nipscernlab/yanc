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
#include "../Headers/messages.h"

// ----------------------------------------------------------------------------
// global variable definitions ------------------------------------------------
// ----------------------------------------------------------------------------

char prname[128] ; // processor name
// Defaults: the one set shared with asmcomp, cppcomp and the HDL (audit 1.7);
// only reached when the program omits the directive.
int  nbmant  = 23; // mantissa width (bits)
int  nbexpo  =  8; // exponent width (bits)
int  nuioin  =  1; // number of input ports
int  nuioou  =  1; // number of output ports
int  fround  =  0; // float rounding level (#FROUND)

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
    // only directives 1, 3, 4, 7 and 8 affect the cmm compiler; 9 and 10 are
    // validated here (asmcomp validates them again: it is the gate every front
    // end passes through; this one just knows the source line)
    switch(t)
    {
        case  1: strcpy (prname,v_table[id].name); break;
        case  3: nbmant = ival             ; break;
        case  4: nbexpo = ival             ; break;
        case  7: nuioin = ival             ; break;
        case  8: nuioou = ival             ; break;
        case  9: if (ival < 0 || ival > 2) {fprintf(stderr, MSG_ERR_FROUND_RANGE, line_num+1); exit(EXIT_FAILURE);} fround = ival; break; // #FROUND level
        case 10: if (ival <= 0 || (ival & (ival - 1)) != 0) {fprintf(stderr, MSG_ERR_NUGAIN_POW2, line_num+1, ival); exit(EXIT_FAILURE);} break;
    }

    stmt_append(stmt_directive(dir, id));
}
