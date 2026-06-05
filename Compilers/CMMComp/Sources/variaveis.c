// ----------------------------------------------------------------------------
// identifier table implementation --------------------------------------------
// ----------------------------------------------------------------------------

// global includes
#include <string.h>
#include <stdlib.h>
#include   <math.h>

// local includes
#include "../Headers/global.h"
#include "../Headers/funcoes.h"
#include "../Headers/variaveis.h"
#include "../Headers/diretivas.h"
#include "../Headers/messages.h"

int          v_count = 0;  // stores the size of the table
static int   v_cap   = 0;  // current capacity of the parallel arrays

// AoS storage: one row per identifier. All attributes (name, type, used,
// isar, isco, size, siz2, fpar, fnid) live here. Grown on demand by var_grow.
symbol *v_table = NULL;

#define GROW(arr, old_cap, new_cap)                                       \
    do {                                                                  \
        void *_tmp = realloc((arr), (size_t)(new_cap) * sizeof(*(arr)));  \
        if (!_tmp) {                                                      \
            fprintf(stderr, MSG_ERR_OUT_OF_MEMORY);                       \
            exit(EXIT_FAILURE);                                           \
        }                                                                 \
        (arr) = _tmp;                                                     \
        memset((arr) + (old_cap), 0,                                      \
               (size_t)((new_cap) - (old_cap)) * sizeof(*(arr)));         \
    } while (0)

static void var_grow(int needed)
{
    if (needed <= v_cap) return;
    int new_cap = v_cap ? v_cap : 256;
    while (new_cap < needed) new_cap *= 2;

    GROW(v_table, v_cap, new_cap);

    v_cap = new_cap;
}

// searches the table for the variable (-1 if not found)
int find_var(char *val)
{
	int i, ind = -1;

	for (i = 0; i < v_count; i++)
	{
		if (strcmp(val, v_table[i].name) == 0)
		{
			ind = i;
			break;
		}
	}
	return ind;
}

// adds a variable to the table
void add_var(char *var)
{
    var_grow(v_count + 1);
    strcpy(v_table[v_count].name, var);
    v_count++;
}

// checks which variables and functions were used (at the end of the parse)
void check_var()
{
    if (mainok == 0) {fprintf (stderr, MSG_ERR_NO_MAIN); exit(EXIT_FAILURE);}

    // walk the whole variable table
    for (int i = 0; i < v_count; i++)
    {
        // check whether a variable was declared but not used ...
        if (((v_table[i].type == 1) || (v_table[i].type == 2) || (v_table[i].type == 3)) && (v_table[i].used == 0))
        {
            // v_table[i].fnid records the variable's owning function: a valid
            // v_table index for locals, or "" / -1 for globals. declar_arr_1d
            // etc. call find_var(fname) which returns -1 when fname is the
            // empty string and no "" entry exists in v_table yet - so treat
            // any out-of-range fnid as the global case.
            int fnid = v_table[i].fnid;
            int is_global = (fnid < 0) || (strcmp(v_table[fnid].name, "") == 0);
            if (is_global)
                fprintf (stdout, MSG_WARN_UNUSED_GLOBAL_VAR, v_table[i].name);
            else
                fprintf (stdout, MSG_WARN_UNUSED_LOCAL_VAR, rem_fname(v_table[i].name, v_table[fnid].name), v_table[fnid].name);
        }

        // check whether a function was declared but not used
        if (((v_table[i].type == 5) || (v_table[i].type == 6) || (v_table[i].type == 7)) && v_table[i].used == 0)
            fprintf (stdout, MSG_WARN_UNUSED_FUNCTION, v_table[i].name);
    }
}

// strips the function name from the variable
char* rem_fname(char *var, char *fname)
{
    // During the deferred AST walk the global fname is "" (reset at each
    // function's end), so callers that pass it cannot strip the prefix. The
    // walker keeps emit_fname pointed at the current function, so prefer it when
    // set -- this is display-only and does not affect exec_id's naming.
    if (emit_fname[0] != '\0') fname = emit_fname;
    if (strcmp(fname,"") == 0) return var;
    int    ind = 0;
    while (var[ind] == fname[ind]) ind++;
    // if ind != strlen(fname) it means the variable does not
    // contain the full function name at the beginning,
    // so it is better not to remove anything (it must be global)
    return (ind == strlen(fname)) ? var + ind + 1 : var; // the +1 skips the '_'
}

// used when the lexer finds an ID
int exec_id(char *text)
{
    if (strcmp(text,"i") == 0)
        {fprintf (stderr, MSG_ERR_RESERVED_I, line_num+1); exit(EXIT_FAILURE);}

    char var_name[64];

    if (find_var(text) == -1)                                     // first check whether there is a global variable
    {
            strcpy  (var_name, fname);
        if (strcmp  (var_name, "") != 0) strcat (var_name, "_");  // otherwise, prepend the current function name
            strcat  (var_name, text);
        if (find_var(var_name)    == -1) add_var(var_name);       // create the local variable if it does not exist yet
    }
    else
    {
        strcpy(var_name, text);
    }
    return find_var(var_name);
}

// used when the lexer finds an int constant
int exec_inum(char *text)
{
    // range check ------------------------------------------------------------
    // no negative constants ever reach this function

    int max = (int) (pow(2,nbmant+nbexpo+1-1)-1);
    int num = atoi(text);

    if (num > max) {fprintf (stderr, MSG_ERR_INT_MAX_OVERFLOW, line_num+1, max); exit(EXIT_FAILURE);}

    // add to the table -------------------------------------------------------

    if (find_var(text) == -1) add_var(text);

    int id = find_var(text);

    v_table[id].isco = 1;

    return id;
}

// converts an IEEE-754 32-bit float to "my float"
// could be revised to convert 64-bit floats too
void f2mf(char *va, int *s, int *m, int *e)
{
    float f = atof(va);

    if (f == 0.0) {*m = 0; *e = 0;}

    int *ifl = (int*)&f;

    // unpack standard IEEE ---------------------------------------------------

    *s = ( *ifl >> 31) & 0x00000001;
    *e = ((*ifl >> 23) & 0xFF) - 127 - 22;
    *m = ((*ifl & 0x007FFFFF) + 0x00800000) >> 1;

    // exponent ---------------------------------------------------------------

    *e = *e + (23-nbmant);

    int sh = 0;
    while (*e < -pow(2, nbexpo-1)) {*e = *e+1; sh = sh+1;}

    // mantissa ---------------------------------------------------------------

    if (nbmant == 23)
    {
        if (*ifl & 0x00000001) *m = *m+1; // round
    }
    else
    {
        sh = 23-nbmant+sh;
        int carry = (*m >> (sh-1)) & 0x00000001; // rounding carry
        *m = *m >> sh;
        if (carry) *m = *m+1; // round
    }
}

// used when the lexer finds a float constant
int exec_fnum(char *text)
{
    // range check ------------------------------------------------------------
    // the negative sign of the constant never enters here

    float max = (float)((pow(2,nbmant)-1) * pow(2, pow(2,nbexpo-1)-1)); // largest representable magnitude
    float min = (float)(                    pow(2,-pow(2,nbexpo-1)  )); // smallest representable magnitude
    float num = atof(text);                                             // absolute value of the number
    float abs = (num < 0.0) ? -num : num;                               // absolute value of the number

    if (abs < min && abs != 0.0) {fprintf (stderr, MSG_ERR_FLOAT_MIN, line_num+1, min); exit(EXIT_FAILURE);}
    if (abs > max)               {fprintf (stderr, MSG_ERR_FLOAT_MAX, line_num+1, max); exit(EXIT_FAILURE);}

    // compute the residual ---------------------------------------------------

    int s,m,e; f2mf(text,&s,&m,&e);

    float mf    = (s) ? -m*pow(2,e) : m*pow(2,e);
    float delta = mf-num;

    if (delta != 0.0 && num != 0.0) printf(MSG_INFO_CONST_APPROX,text,line_num+1,mf,delta);

    // add to the table -------------------------------------------------------

    if (find_var(text) == -1) add_var(text);

    int id = find_var(text);

    v_table[id].isco = 1;

    return id;
}

// used when the lexer finds a comp constant
int exec_cnum(char *text)
{
    // strip whitespace -------------------------------------------------------

    int i = 0, j = 0;
    char temp[strlen(text) + 1];
    strcpy(temp, text);
    while (temp[i] != '\0')
    {
        while (temp[i] == ' ') i++;
        text[j] = temp[i];
        i++;
        j++;
    }
    text[j] = '\0';

    // add to the table -------------------------------------------------------

    if (find_var(text) == -1) add_var(text);
    return find_var(text);
}
