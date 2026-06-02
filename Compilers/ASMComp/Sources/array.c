// ----------------------------------------------------------------------------
// assembly array handling ----------------------------------------------------
// ----------------------------------------------------------------------------

// global includes
#include <string.h>
#include <stdlib.h>
#include  <ctype.h>
#include   <math.h>

// local includes
#include "../Headers/t2t.h"
#include "../Headers/eval.h"
#include "../Headers/array.h"
#include "../Headers/variaveis.h"
#include "../Headers/messages.h"

// ----------------------------------------------------------------------------
// helper functions -----------------------------------------------------------
// ----------------------------------------------------------------------------

// helper to strip quotes from a string
void rem_aspas(char *str)
{
    int j = 0; char c;
    for (int i = 0; (c = str[i]) != '\0'; i++) if (c != '"') str[j++] = c;
    str[j] = '\0';
}

// helper that checks whether a file line contains a valid int
int linha_e_inteiro(char *linha, int idx, char *f_name)
{
    // formatting check -------------------------------------------------------

    // skip leading whitespace
    while (isspace((unsigned char)*linha)) linha++ ;
    // empty line is not a valid integer
    if (*linha == '\0' || *linha == '\n') {fprintf(stderr, MSG_ERR_EMPTY_LINE, idx, f_name); exit(EXIT_FAILURE);}
    // walk over the numeric characters (including the negative sign)
    char *endptr; strtol(linha, &endptr, 10);
    // make sure the remainder of the string is only whitespace
    while (isspace((unsigned char)*endptr)) endptr++;
    // it's an integer if nothing else is left
    if (*endptr != '\0') {fprintf(stderr, MSG_ERR_INVALID_INT, idx, f_name); exit(EXIT_FAILURE);}

    // check that the value fits in the bit count -----------------------------

    int max = (int) ( pow(2,nbmant+nbexpo+1-1)-1);
    int min = (int) (-pow(2,nbmant+nbexpo+1-1)  );
    int num = atoi(linha);

    if (num > max) {fprintf(stderr, MSG_ERR_INT_OVER, idx, f_name, max); exit(EXIT_FAILURE);}
    if (num < min) {fprintf(stderr, MSG_ERR_INT_UNDER, idx, f_name, min); exit(EXIT_FAILURE);}

    return num;
}

// helper that checks whether a line is a valid float
int linha_e_float(char *linha, int idx, char *f_name, float *delta)
{
    // formatting check -------------------------------------------------------

    // skip leading whitespace
    while (isspace((unsigned char)*linha)) linha++;
    // empty line
    if (*linha == '\0' || *linha == '\n') {fprintf(stderr, MSG_ERR_EMPTY_LINE, idx, f_name); exit(EXIT_FAILURE);}
    // walk over the numeric characters (including the negative sign)
    char *endptr; strtof(linha, &endptr);
    // make sure the remainder of the string is only whitespace
    while (isspace((unsigned char)*endptr)) endptr++;
    // it's a float if nothing else is left
    if (*endptr != '\0') {fprintf(stderr, MSG_ERR_INVALID_FLOAT, idx, f_name); exit(EXIT_FAILURE);}

    // range check ------------------------------------------------------------

    float max = (float)((pow(2,nbmant)-1) * pow(2, pow(2,nbexpo-1)-1)); // largest representable magnitude
    float min = (float)(                    pow(2,-pow(2,nbexpo-1)  )); // smallest representable magnitude
    float num = (atof(linha)<0.0) ? -atof(linha) : atof(linha);         // absolute value of the number

    if (num < min && num != 0.0) {fprintf(stderr, MSG_ERR_FLOAT_UNDER, idx, f_name, min); exit(EXIT_FAILURE);}
    if (num > max)               {fprintf(stderr, MSG_ERR_FLOAT_OVER, idx, f_name, max); exit(EXIT_FAILURE);}

    // convert and compute the residual ---------------------------------------

    return f2mf(linha,delta);
}

// helper that validates and extracts a float
// used when reading comp variables from a file
int parse_float(const char *str, float *out_value, const char **out_end)
{
    char *endptr;

    float val = strtof(str, &endptr); // try to convert
    if (str  == endptr)     return 0; // conversion failed
    *out_value =    val;              // grab the converted value
    *out_end   = endptr;              // points to the next character in the line
                            return 1; // conversion ok
}

// helper to split a comp's real and imaginary parts
void separar_complexo(const char *entrada, char *real, char *imag) {
    char buffer[100];
    int j = 0;

    // strip whitespace
    for (int i = 0; entrada[i] != '\0'; i++) {
        if (!isspace((unsigned char)entrada[i])) {
            buffer[j++] = entrada[i];
        }
    }
    buffer[j] = '\0';

    // find the last '+' or '-' that splits real from imaginary
    int pos = -1;
    for (int i = 1; buffer[i] != '\0'; i++) { // skip index 0 to avoid confusing it with the real part's sign
        if (buffer[i] == '+' || buffer[i] == '-') {
            pos = i;
        }
    }

    if (pos == -1) {
        fprintf(stderr, MSG_ERR_BAD_FORMAT);
        real[0] = '\0';
        imag[0] = '\0';
        return;
    }

    // copy the real part
    strncpy(real, buffer, pos);
    real[pos] = '\0';

    // copy the imaginary part (without the 'i')
    strcpy(imag, buffer + pos);
    size_t len = strlen(imag);
    if (len > 0 && imag[len - 1] == 'i') {
        imag[len - 1] = '\0';
    }
}

// helper that checks whether a line contains a valid comp
int linha_e_comp(const char *linha, int idx, char *f_name, float *delta)
{
    const char *p = linha;
    float f1, f2;

    // formatting check -------------------------------------------------------

    // skip whitespace
    while (isspace((unsigned char)*p)) p++;
    // first float
    if (!parse_float(p, &f1, &p)) {fprintf(stderr, MSG_ERR_INVALID_COMP, idx, f_name); exit(EXIT_FAILURE);}
    // skip whitespace
    while (isspace((unsigned char)*p)) p++;
    // second float
    if (!parse_float(p, &f2, &p)) {fprintf(stderr, MSG_ERR_INVALID_COMP, idx, f_name); exit(EXIT_FAILURE);}
    // if the next character is not 'i', bail out
    if (*p != 'i')                {fprintf(stderr, MSG_ERR_INVALID_COMP, idx, f_name); exit(EXIT_FAILURE);}
    // advance past the 'i'
    p++;
    // skip whitespace
    while (isspace((unsigned char)*p)) p++;
    // if anything else remains on the line, it's not a valid comp
    if (*p != '\0')               {fprintf(stderr, MSG_ERR_INVALID_COMP, idx, f_name); exit(EXIT_FAILURE);}

    // range check ------------------------------------------------------------

    float max = (float)((pow(2,nbmant)-1) * pow(2, pow(2,nbexpo-1)-1)); // largest representable magnitude
    float min = (float)(                    pow(2,-pow(2,nbexpo-1)  )); // smallest representable magnitude
    char  real [64], imag[64];

    separar_complexo(linha, real, imag);

    // real part
    float num = (atof(real)<0.0) ? -atof(real) : atof(real); // absolute value of the real part
    if (num < min && num != 0.0) {fprintf(stderr, MSG_ERR_COMP_REAL_UNDER, idx, f_name, min); exit(EXIT_FAILURE);}
    if (num > max)               {fprintf(stderr, MSG_ERR_COMP_REAL_OVER, idx, f_name, max); exit(EXIT_FAILURE);}

    // imaginary part
        num = (atof(imag)<0.0) ? -atof(imag) : atof(imag); // absolute value of the imaginary part
    if (num < min && num != 0.0) {fprintf(stderr, MSG_ERR_COMP_IMAG_UNDER, idx, f_name, min); exit(EXIT_FAILURE);}
    if (num > max)               {fprintf(stderr, MSG_ERR_COMP_IMAG_OVER, idx, f_name, max); exit(EXIT_FAILURE);}

    // convert the real part and compute the residual -------------------------

    return f2mf(real,delta);
}

// helper that fills an array in data memory
// used for array initialization (e.g. int x[10] "filename")
// f_name  -> name of the file to read
// tam     -> file size
// fil_typ -> data type
// f_data  -> data-memory file
void fill_mem(char *f_name, int tam, int fil_typ, FILE *f_data)
{
    // announce that the array will be filled ---------------------------------

    if (fil_typ != 4) printf(MSG_INFO_FILL_ARRAY, tam, f_name);

    // open the file for reading ----------------------------------------------

    rem_aspas(f_name);

    char path[2048];
    // check if it's a LUT in the Macros folder
    if (f_name[0]=='$')
        sprintf(path, "%s/%s"          , mac_dir, f_name+1);
    else
        sprintf(path, "%s/Software/%s", proc_dir, f_name  );

    FILE *f_file =        fopen  (path  , "r");
    if   (f_file == NULL){fprintf(stderr, MSG_ERR_CANT_OPEN_FILE, path); exit(EXIT_FAILURE);}

    // now read the file ------------------------------------------------------

    // val = 0 defends against fil_typ outside {1,2,3,4}: the four ifs
    // below set it for the expected types, but GCC cannot prove the
    // domain so we initialize defensively.
    int  val = 0;
    char linha[128];
    char real [64], imag[64];

    int   i     =   0;
    int   idx   =   0;
    float delta = 0.0;
    float dmax  = 0.0;
    while (fgets (linha, sizeof(linha), f_file))
    {
        i++;

        // int type
        if (fil_typ == 1) val = linha_e_inteiro(linha,i,f_name);

        // float type
        if (fil_typ == 2)
        {
            val = linha_e_float(linha,i,f_name,&delta);
            // keep track of the largest approximation error
            if (fabs(delta) > fabs(dmax)) {dmax = delta; idx = i;}
        }

        // real part of a comp
        if (fil_typ == 3)
        {
            val = linha_e_comp(linha,i,f_name,&delta);
            // keep track of the largest approximation error
            if (fabs(delta) > fabs(dmax)) {dmax = delta; idx = i;}
        }

        // imaginary part of a comp
        if (fil_typ == 4)
        {
            separar_complexo(linha, real, imag);
            val = f2mf(imag,&delta);
            // keep track of the largest approximation error
            if (fabs(delta) > fabs(dmax)) {dmax = delta; idx = i;}
        }

        // if there are more values than needed, emit a warning and bail out
        if (i > tam)
        {
            fprintf(stdout, MSG_WARN_EXTRA_LINES, i-tam, f_name);
            break;
        }
        // otherwise, fill the memory with the new value
        else
            fprintf(f_data, "%s\n", itob(val,nubits));
    }

    // if there are fewer values than needed, raise an error
    if ((i < tam) && (fil_typ != 4))
        {fprintf(stderr, MSG_ERR_MISSING_LINES, tam-i, f_name); exit(EXIT_FAILURE);}

    // report the largest approximation error for float
    if (fil_typ == 2 && dmax != 0.0)
        printf(MSG_INFO_APPROX_ERR, f_name, dmax, idx);

    // report the largest approximation error for the real part of comp
    if (fil_typ == 3 && dmax != 0.0)
        printf(MSG_INFO_APPROX_ERR_REAL, f_name, dmax, idx);

    // report the largest approximation error for the imaginary part of comp
    if (fil_typ == 4 && dmax != 0.0)
        printf(MSG_INFO_APPROX_ERR_IMAG, f_name, dmax, idx);

    fclose(f_file);
}

// ----------------------------------------------------------------------------
// array manipulation functions -----------------------------------------------
// ----------------------------------------------------------------------------

// adds an array to data memory
// for a regular array (f_name = ""), fills with zeros
// for an initialized array, calls fill_mem to populate it
void arr_add(int size, int type, char *f_name, FILE *f_data)
{
    // increment memory size accordingly
    var_inc(size-1);
    // no file: fill with zeros
    if (strcmp(f_name, "") == 0)
        for (int i = 0; i < size; i++)
        {
            if (type > 1)
                fprintf(f_data, "%s\n", itob(f2mf("0.0",NULL), nubits)); // float or comp: initialize with 0.0
            else
                fprintf(f_data, "%s\n", itob(0,nubits));                 // int: initialize with 0
        }
    else
        fill_mem(f_name, size, type, f_data);
}
