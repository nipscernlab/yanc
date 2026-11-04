// ----------------------------------------------------------------------------
// functions and variables used to create and use macros ----------------------
// ----------------------------------------------------------------------------


// global includes
#include <math.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

// local includes
#include "../Headers/macros.h"
#include "../Headers/convert.h"
#include "../Headers/global.h"
#include "../Headers/functions.h"
#include "../Headers/directives.h"
#include "../Headers/symbols.h"
#include "../Headers/messages.h"
#include "../Headers/diag.h"
#include "../../common/xstr.h"

// ----------------------------------------------------------------------------
// helpers for generating predefined macros -----------------------------------
// ----------------------------------------------------------------------------

// concatenates the contents of the read file into the write file
static void fcat2end(const char *n_read, const char *n_write)
{
    FILE *f_in  = fopen(n_read , "r");
    if (f_in  == NULL) fatal(MSG_ERR_CANT_OPEN_FILE, n_read);
    FILE *f_out = fopen(n_write, "a");
    if (f_out == NULL) fatal(MSG_ERR_CANT_OPEN_FILE, n_write);

    int a;
    while ((a = fgetc(f_in)) != EOF) fputc(a, f_out);

    fclose(f_in );
    fclose(f_out);
}

// ----------------------------------------------------------------------------
// management of predefined macros --------------------------------------------
// ----------------------------------------------------------------------------

// local variables ------------------------------------------------------------

int fatan = 0; // whether the arctangent macro is needed
int fsqrt = 0; // whether the square-root macro is needed
int fsin  = 0; // whether the sine macro is needed
int ftan  = 0; // whether the tangent macro is needed
int fexp  = 0; // whether the exponential macro is needed
int flog  = 0; // whether the natural-logarithm macro is needed

// adds a flag for a predefined macro -----------------------------------------

void mac_add(const char *name)
{
         if (strcmp(name, "fsqrt") == 0) fsqrt = 1; // square root
    else if (strcmp(name, "fatan") == 0) fatan = 1; // arctangent
    else if (strcmp(name, "fsin" ) == 0) fsin  = 1; // sine
    else if (strcmp(name, "ftan" ) == 0) ftan  = 1; // tangent
    else if (strcmp(name, "fexp" ) == 0) fexp  = 1; // exponential
    else if (strcmp(name, "flog" ) == 0) flog  = 1; // natural logarithm
}

// appends the macro file `file` of the Macros folder to the .asm
static void mac_cat(const char *file, char *fasm)
{
    char *tasm = xasprintf("%s/%s", dir_macro, file);
    fcat2end(tasm, fasm);
    free(tasm);
}

// copies the predefined macros at the end of the assembler file --------------

void mac_copy(char *fasm)
{
    // bail out if there is nothing to do -------------------------------------

    if (!(fsqrt || fatan || fsin || ftan || fexp || flog)) return;

    // copy what is needed at the end of the asm ------------------------------

    if (fsqrt)
    {
        warn(MSG_INFO_SQRT_MACRO);
        mac_cat("float_sqrt.asm", fasm);
    }

    if (fatan)
    {
        warn(MSG_INFO_ATAN_MACRO);
        mac_cat("float_atan.asm", fasm);
    }

    if (fsin)
    {
        warn(MSG_INFO_SIN_MACRO);
        mac_cat("float_sin.asm", fasm);
    }

    if (ftan)
    {
        warn(MSG_INFO_TAN_MACRO);
        mac_cat("float_tan.asm", fasm);
    }

    if (fexp)
    {
        warn(MSG_INFO_EXP_MACRO);
        mac_cat("float_exp.asm", fasm);
    }

    if (flog)
    {
        warn(MSG_INFO_LOG_MACRO);
        mac_cat("float_log.asm", fasm);
    }
}
