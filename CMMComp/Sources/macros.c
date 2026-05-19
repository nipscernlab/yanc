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
#include "..\Headers\t2t.h"
#include "..\Headers\global.h"
#include "..\Headers\funcoes.h"
#include "..\Headers\diretivas.h"
#include "..\Headers\variaveis.h"
#include "..\Headers\messages.h"

// ----------------------------------------------------------------------------
// management of user-created macros ------------------------------------------
// ----------------------------------------------------------------------------

// global variable definitions ------------------------------------------------

int mac_using = 0; // while reading a macro, the assembler must not be written during the parse

// blocks the parser from writing to the assembler file -----------------------
// instead, copies the code of a macro
// id_num is the third argument of the macro, which must be the number of instructions
// remove the need to pass id_num, since the instruction count is fixed
void mac_use(int ids, int global, int id_num)
{
    // consistency check ------------------------------------------------------

    if (mac_using == 1)
        {fprintf(stderr, MSG_ERR_NESTED_MACRO, line_num+1); exit(EXIT_FAILURE);}

    printf(MSG_INFO_USER_MACRO, v_table[ids].name, line_num+1);

    // global: check whether main still needs to be called --------------------
    if ((mainok == 0) && (global == 1))
    {
        add_sinst(-2, "JMP main\n");

        mainok = 1; // main function status resolved
    }

    // strip the quotes from the string (clunky!) -----------------------------
    // switch to simpler code like the one in array.c -------------------------

    char f_name[64];
    strcpy(f_name, v_table[ids].name);
    char file_name[512];
    int  tamanho = strlen(f_name); // string length
    int idxToDel = tamanho-1;      // index to delete, in this case the trailing quote
    strcpy ( file_name, "");
    memmove(&f_name[idxToDel], &f_name[idxToDel+1], 1); // actually delete the trailing quote
    strcat ( file_name, f_name+1); // copy starting after the leading quote

    char    mac_name[2048];
    snprintf(mac_name, sizeof(mac_name), "%s/%s", dir_soft, file_name);

    // copy the code from the asm file ----------------------------------------

    FILE *f_macro;
    char a;
        f_macro  =    fopen  (mac_name, "r");
    if (f_macro == 0){fprintf(stderr, MSG_ERR_MACRO_NOT_FOUND, line_num+1, file_name); exit(EXIT_FAILURE);}
	do {      a  =    fgetc  (f_macro); if (a != EOF) fputc(a,f_asm);} while (a != EOF);
                      fputc  ('\n',f_asm);
	                  fclose (f_macro);

    // fill the cmm-code table with -1 (INTERNAL) -----------------------------

    int n = atoi(v_table[id_num].name);
    for (int i = 0; i < n; i++)
    {
        num_ins++;
        fprintf(f_lin, "%s\n", itob(-4,20));
    }

    mac_using = 1;

    // We just copied an opaque macro body straight into f_asm. Anything the
    // macro emitted is not visible to the peephole, so the previous SET in
    // last_str is no longer adjacent. Reset to keep the optimization safe.
    emit_peephole_reset();
}

// releases the parser to write to the assembler file -------------------------

void mac_end()
{
    if (mac_using == 0) {fprintf(stderr, MSG_ERR_MACRO_NO_START, line_num+1); exit(EXIT_FAILURE);}
    mac_using = 0;

    // Same reasoning as mac_use: a macro just executed opaquely, so the
    // peephole window from before it is meaningless.
    emit_peephole_reset();
}

// ----------------------------------------------------------------------------
// helpers for generating predefined macros -----------------------------------
// ----------------------------------------------------------------------------

// concatenates the contents of the read file into the write file
void fcat2end(char *n_read, char *n_write)
{
    FILE *f_in  = fopen(n_read , "r");
    FILE *f_out = fopen(n_write, "a");

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

// adds a flag for a predefined macro -----------------------------------------

void mac_add(char *name)
{
         if (strcmp(name, "fsqrt") == 0) fsqrt = 1; // square root
    else if (strcmp(name, "fatan") == 0) fatan = 1; // arctangent
    else if (strcmp(name, "fsin" ) == 0) fsin  = 1; // sine
}

// copies the predefined macros at the end of the assembler file --------------

void mac_copy(char *fasm)
{
    // bail out if there is nothing to do -------------------------------------

    if (!(fsqrt || fatan || fsin)) return;

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
