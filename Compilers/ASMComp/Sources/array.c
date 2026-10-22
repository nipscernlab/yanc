// ----------------------------------------------------------------------------
// assembly array handling ----------------------------------------------------
// ----------------------------------------------------------------------------

// global includes
#include <string.h>
#include <stdlib.h>
#include  <ctype.h>
#include   <math.h>
#include <limits.h>

// local includes
#include "../Headers/t2t.h"
#include "../Headers/eval.h"
#include "../Headers/array.h"
#include "../Headers/variaveis.h"
#include "../Headers/messages.h"
#include "../../common/yanc_num.h"

// ----------------------------------------------------------------------------
// helper functions -----------------------------------------------------------
// ----------------------------------------------------------------------------

// largest and smallest magnitudes of the float format, for the messages
static double fp_max(void) { return (pow(2,nbmant)-1) * pow(2, pow(2,nbexpo-1)-1); }
static double fp_min(void) { return pow(2,-pow(2,nbexpo-1)); }

// whether a float text fits the format, by the encoder f2mf uses (exact, not a
// host float): 1 too large, -1 a nonzero value that would encode as zero, 0
// it fits (a text that is not a number is f2mf's to report)
static int fp_range(const char *text)
{
    yn_float r;
    if (!yn_encode(text, nbmant, nbexpo, fround, &r)) return 0;
    if (r.overflow) return 1;
    if (r.underflow && !r.flushed) return -1;
    return 0;
}

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

    int rg = fp_range(linha);
    if (rg < 0) {fprintf(stderr, MSG_ERR_FLOAT_UNDER, idx, f_name, fp_min()); exit(EXIT_FAILURE);}
    if (rg > 0) {fprintf(stderr, MSG_ERR_FLOAT_OVER , idx, f_name, fp_max()); exit(EXIT_FAILURE);}

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

    // find the last '+' or '-' that splits real from imaginary (a sign right
    // after an e belongs to an exponent: 1e-8+2e-8i)
    int pos = -1;
    for (int i = 1; buffer[i] != '\0'; i++) { // skip index 0 to avoid confusing it with the real part's sign
        if ((buffer[i] == '+' || buffer[i] == '-') && buffer[i-1] != 'e' && buffer[i-1] != 'E') {
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

    char real[100], imag[100];

    separar_complexo(linha, real, imag);

    // real part
    int rg = fp_range(real);
    if (rg < 0) {fprintf(stderr, MSG_ERR_COMP_REAL_UNDER, idx, f_name, fp_min()); exit(EXIT_FAILURE);}
    if (rg > 0) {fprintf(stderr, MSG_ERR_COMP_REAL_OVER , idx, f_name, fp_max()); exit(EXIT_FAILURE);}

    // imaginary part
    rg = fp_range(imag);
    if (rg < 0) {fprintf(stderr, MSG_ERR_COMP_IMAG_UNDER, idx, f_name, fp_min()); exit(EXIT_FAILURE);}
    if (rg > 0) {fprintf(stderr, MSG_ERR_COMP_IMAG_OVER , idx, f_name, fp_max()); exit(EXIT_FAILURE);}

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
        snprintf(path, sizeof(path), "%s/%s"          , mac_dir, f_name+1);
    else
        snprintf(path, sizeof(path), "%s/%s", src_dir(), f_name  );

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

// ----------------------------------------------------------------------------
// #arrayd: the initial values on the directive's own line -------------------
// ----------------------------------------------------------------------------
// The compilers write it for an array initialized between braces (C+-) and
// for constant data (C++ string literals and const arrays), so the values are
// in the data memory's .mif instead of being stored by code at run time. The
// words after the n given are zero, as in C.

static char  arrd_name[128];
static int   arrd_size, arrd_type, arrd_n, arrd_i, arrd_imax;
static float arrd_dmax;
static FILE *arrd_f;

static void arrd_zero(void)                   // one zero word of the array's type
{
    if (arrd_type > 1) fprintf(arrd_f, "%s\n", itob(f2mf("0.0",NULL), nubits));
    else               fprintf(arrd_f, "%s\n", itob(0,nubits));
}

static void arrd_end(void)
{
    for (int i = arrd_n; i < arrd_size; i++) arrd_zero();
    if (arrd_type > 1 && arrd_dmax != 0.0) printf(MSG_INFO_APPROX_ERR_ARRD, arrd_name, arrd_dmax, arrd_imax);
}

int arrd_start(char *name, int size, int type, int n, FILE *f_data)
{
    snprintf(arrd_name, sizeof(arrd_name), "%s", name);
    if (n > size) {fprintf(stderr, MSG_ERR_ARRD_COUNT, name, size, n); exit(EXIT_FAILURE);}
    var_inc(size-1);
    arrd_size = size; arrd_type = type; arrd_n = n; arrd_i = 0;
    arrd_dmax = 0.0;  arrd_imax = 0;    arrd_f = f_data;
    if (n == 0) { arrd_end(); return 0; }
    return 1;
}

int arrd_value(char *va, FILE *f_data)
{
    arrd_i++;
    if (arrd_type > 1)                        // float, or a part of a comp
    {
        float delta = 0.0;
        unsigned int w = f2mf(va, &delta);
        if (fabs(delta) > fabs(arrd_dmax)) {arrd_dmax = delta; arrd_imax = arrd_i;}
        fprintf(f_data, "%s\n", itob(w, nubits));
    }
    else                                      // int: two's complement in NUBITS bits, or an
    {                                         // unsigned value up to 2^NUBITS - 1 (a char 200 at 8 bits)
        long long v   = atoll(va);
        long long max = (nubits >= 63) ? LLONG_MAX : (1LL << nubits) - 1;
        long long min = (nubits >= 63) ? LLONG_MIN : -(1LL << (nubits-1));
        if (v > max || v < min) {fprintf(stderr, MSG_ERR_ARRD_INT, arrd_i, arrd_name, va, nubits); exit(EXIT_FAILURE);}
        fprintf(f_data, "%s\n", itob((int)v, nubits));
    }
    if (arrd_i < arrd_n) return 1;
    arrd_end();
    return 0;
}

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
