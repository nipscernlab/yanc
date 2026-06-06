// ----------------------------------------------------------------------------
// functions and variables used to create and use macros ----------------------
// ----------------------------------------------------------------------------

/*
TODO:
1- implement more non-linear functions
2- check Tiago Falcao's thesis for which method is best in each case
*/

// global includes
#include <math.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

// local includes
#include "../Headers/t2t.h"
#include "../Headers/global.h"
#include "../Headers/funcoes.h"
#include "../Headers/diretivas.h"
#include "../Headers/variaveis.h"
#include "../Headers/messages.h"

// ----------------------------------------------------------------------------
// helpers for generating predefined macros -----------------------------------
// ----------------------------------------------------------------------------

// concatenates the contents of the read file into the write file
void fcat2end(char *n_read, char *n_write)
{
    FILE *f_in  = fopen(n_read , "r");
    if (f_in  == NULL) { fprintf(stderr, MSG_ERR_CANT_OPEN_FILE, n_read);  exit(EXIT_FAILURE); }
    FILE *f_out = fopen(n_write, "a");
    if (f_out == NULL) { fprintf(stderr, MSG_ERR_CANT_OPEN_FILE, n_write); exit(EXIT_FAILURE); }

    char a;
    do {a = fgetc(f_in); if (a != EOF) fputc(a, f_out);} while (a != EOF);

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
int fexp  = 0; // whether the exponential macro is needed
int flog  = 0; // whether the natural-logarithm macro is needed

// adds a flag for a predefined macro -----------------------------------------

void mac_add(char *name)
{
         if (strcmp(name, "fsqrt") == 0) fsqrt = 1; // square root
    else if (strcmp(name, "fatan") == 0) fatan = 1; // arctangent
    else if (strcmp(name, "fsin" ) == 0) fsin  = 1; // sine
    else if (strcmp(name, "fexp" ) == 0) fexp  = 1; // exponential
    else if (strcmp(name, "flog" ) == 0) flog  = 1; // natural logarithm
}

// copies the predefined macros at the end of the assembler file --------------

void mac_copy(char *fasm)
{
    // bail out if there is nothing to do -------------------------------------

    if (!(fsqrt || fatan || fsin || fexp || flog)) return;

    // copy what is needed at the end of the asm ------------------------------

    char tasm[2048]; snprintf(tasm, sizeof(tasm), "%s/%s", dir_tmp, "tasm.txt");

    if (fsqrt)
    {
        printf(MSG_INFO_SQRT_MACRO);
        snprintf(tasm, sizeof(tasm), "%s/float_sqrt.asm", dir_macro);
        fcat2end(tasm,fasm);
    }

    if (fatan)
    {
        printf(MSG_INFO_ATAN_MACRO);
        snprintf(tasm, sizeof(tasm), "%s/float_atan.asm", dir_macro);
        fcat2end(tasm,fasm);
    }

    if (fsin)
    {
        printf(MSG_INFO_SIN_MACRO);
        snprintf(tasm, sizeof(tasm), "%s/float_sin.asm", dir_macro);
        fcat2end(tasm,fasm);
    }

    if (fexp)
    {
        printf(MSG_INFO_EXP_MACRO);
        snprintf(tasm, sizeof(tasm), "%s/float_exp.asm", dir_macro);
        fcat2end(tasm,fasm);
    }

    if (flog)
    {
        printf(MSG_INFO_LOG_MACRO);
        snprintf(tasm, sizeof(tasm), "%s/float_log.asm", dir_macro);
        fcat2end(tasm,fasm);
    }
}

// ----------------------------------------------------------------------------
// backup of the c+- code for the predefined macros ---------------------------
// ----------------------------------------------------------------------------

// arctangent for float (float_atan.asm)
/*float float_atan(float x)
{
    float ax = abs(x);

    if (ax == 0.0) return 0.0;
    if (ax > 1.02) return sign(x, 1.5707963268) - my_atan(1.0/x);
    if (ax > 0.98)
    {
        float xm1 = ax-1;
        return sign(x, 0.7853981634 + xm1*0.5 - xm1*xm1*0.25);
    }

    float termo      = x;
    float x2         = x*x;
    float resultado  = termo;
    float tolerancia = 0.000008/x2;

    int indiceX = 3;

    while ((abs(termo) > tolerancia) && (indiceX < 100)) {
        termo = termo * (- x2 * (indiceX - 2)) / indiceX;

        resultado = resultado + termo;
        indiceX = indiceX + 2;
    }

    return resultado;
}*/

// sine for float (float_sin.asm)
/*float sin(float x)
{
    if (x == 0) return 0.0;

    while (abs(x) > 3.141592654) x = x - sign(x, 6.283185307);

    float termo      = x;
    float x2         = x * x;
    float resultado  = termo;
    float tolerancia = 0.000008/x2;

    int indiceX = 3;

    while (abs(termo) > tolerancia) {
        termo = termo * (- x2) / ((indiceX - 1) * indiceX);
        resultado = resultado + termo;
        indiceX = indiceX + 2;
    }

    return resultado;
}*/
