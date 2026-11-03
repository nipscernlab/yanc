// ----------------------------------------------------------------------------
// variable declaration -------------------------------------------------------
// ----------------------------------------------------------------------------


// global includes
#include <string.h>
#include <stdlib.h>
#include  <stdio.h>

// local includes
#include "../Headers/data_declar.h"
#include "../Headers/t2t.h"
#include "../Headers/ast.h"
#include "../Headers/global.h"
#include "../Headers/stdlib.h"
#include "../Headers/funcoes.h"
#include "../Headers/variaveis.h"
#include "../Headers/messages.h"
#include "../Headers/diag.h"
#include "../../common/xstr.h"

// ----------------------------------------------------------------------------
// global variable definitions ------------------------------------------------
// ----------------------------------------------------------------------------

int type_tmp; // captures the type when a variable is declared (see CMMComp.l)

// ----------------------------------------------------------------------------
// standard declarations ------------------------------------------------------
// ----------------------------------------------------------------------------

// declares a variable (non-array)
void declar_var(int id)
{
    // consistency check ------------------------------------------------------

    check_name(id, 0);   // free in the .asm, not an instruction, not declared twice

    // update the variable status ---------------------------------------------

    v_table[id].type          = type_tmp;            // the variable type is stored in type_tmp (see flex when it finds int, float or comp)
    v_table[id].used          = 0;                   // just declared, so not used yet
    v_table[id].fnid    = find_var(fname);     // record the function it belongs to

    // declare the imaginary part if comp -------------------------------------

    if (type_tmp > 2)
    {
        int idi              = get_img_id(id);
        v_table[idi].type          = TY_IMAG; // use type 4 for the imaginary part
        v_table[idi].used          = 0;
        v_table[idi].fnid    = find_var(fname);
    }

    // register the variable in the log file ----------------------------------

    // save the function it belongs to
    const char *func = strcmp(fname,"")==0 ? "global" : fname;
    // save the data in the log file
    fprintf(f_log, "%s %s %d\n", func, rem_fname(v_table[id].name), type_tmp);
    // if comp, save the imaginary part too
    if (type_tmp == 3) fprintf(f_log, "%s %s_i 3\n", func, rem_fname(v_table[id].name));
}

// Parse-time half of declar_arr_1d: v_table update + log / info messages.
// Called immediately so subsequent declarations can resolve this var.
static void declar_arr_1d_parse(int id_var, int id_arg, int id_fname)
{
    check_name(id_var, 0);

    v_table[id_var].type       = type_tmp;
    v_table[id_var].used       = 0;
    v_table[id_var].fnid = find_var(fname);
    v_table[id_var].dims       = AR_1D;
    v_table[id_var].size       = atoi(v_table[id_arg].name);

    int type = type_tmp;

    const char *func = strcmp(fname,"")==0 ? "global" : fname;
    if (sim_arr == 1)
    {
                       fprintf(f_log, "%s %s   %d %s\n", func, rem_fname(v_table[id_var].name), type_tmp, v_table[id_arg].name);
        if (type == 3) fprintf(f_log, "%s %s_i %d %s\n", func, rem_fname(v_table[id_var].name), type_tmp, v_table[id_arg].name);
    }

    // for comp, mark the imaginary half as a 1D array too
    if (type == 3) v_table[get_img_id(id_var)].dims = AR_1D;

    if (id_fname != -1)
        warn(MSG_INFO_ARRAY_FILE_INIT, v_table[id_fname].name, rem_fname(v_table[id_var].name), line_num+1);
    // the file is .mif content, loaded once: a local keeps state across calls
    if (id_fname != -1 && strcmp(fname, "") != 0)
        warn(MSG_WARN_ARRAY_FILE_LOCAL, line_num+1, rem_fname(v_table[id_var].name), v_table[id_fname].name);
}

// ----------------------------------------------------------------------------
// arrays initialized between braces: int v[4] = {1, 2, 3}; ---------------------
// ----------------------------------------------------------------------------
// The values are constants. They go into the data memory's .mif through a
// `#arrayd <name> <type> <size> <n> <values>` line (asmcomp), as a file does
// through `#arrays`: no instruction stores them, and the words not given are
// zero, as in C. Like a file, they are loaded with the memory, once: a local
// array keeps what the function wrote into it from one call to the next, and
// a reset does not bring the values back.

typedef struct ini_val { char *txt; int kind, row, col; } ini_val;

static ini_val *ini_v;                 // the list being read
static int      ini_n, ini_cap;
static int      ini_row = -1, ini_col; // the row being read (-1: outside inner braces)
static int      ini_rows;              // inner-brace rows read so far

typedef struct ini_arr { int id; char *re, *im; int n; struct ini_arr *next; } ini_arr;
static ini_arr *ini_done;              // per array: the values of its #arrayd

void ini_add(int id, int kind, int neg)
{
    if (ini_n == ini_cap) { ini_cap = ini_cap ? 2 * ini_cap : 64; ini_v = xrealloc(ini_v, ini_cap * sizeof(ini_val)); }
    const char *t = v_table[id].name;
    char *s = xmalloc(strlen(t) + 2);
    sprintf(s, "%s%s", neg ? "-" : "", t);
    ini_v[ini_n].txt  = s;
    ini_v[ini_n].kind = kind;
    ini_v[ini_n].row  = ini_row;
    ini_v[ini_n].col  = (ini_row >= 0) ? ini_col++ : -1;
    ini_n++;
}

void ini_row_open (void) { ini_row = ini_rows++; ini_col = 0; }
void ini_row_close(void) { ini_row = -1; }

// a comp constant "3+4i" / "-1.5-2i" as its two parts; an int or a float is
// all real
static void ini_split(const ini_val *v, char **re, char **im)
{
    if (v->kind != 3) { *re = xstrdup(v->txt); *im = xstrdup("0.0"); return; }
    cnum_split(v->txt, re, im);                 // a sign after an e is the exponent's
}

// check the list against the array (type, size, rows) and keep its values
static void ini_take(int id_var, int size, int siz2)
{
    int type = v_table[id_var].type;
    const char *tname = (type == 1) ? "int" : (type == 2) ? "float" : "comp";
    int maxpos = -1;
    for (int k = 0; k < ini_n; k++)
    {
        ini_val *v = &ini_v[k];
        if (v->kind > type) fatal(MSG_ERR_ARRAY_INI_TYPE, line_num+1, rem_fname(v_table[id_var].name), tname, v->txt);
        int pos;
        if (ini_rows > 0)                        // rows between inner braces: a 2D array, every value in a row
        {
            if (siz2 == 0) fatal(MSG_ERR_ARRAY_INI_1D, line_num+1, rem_fname(v_table[id_var].name));
            if (v->row < 0 || v->col >= siz2)
                fatal(MSG_ERR_ARRAY_INI_ROWS, line_num+1, rem_fname(v_table[id_var].name), siz2);
            pos = v->row * siz2 + v->col;
        }
        else pos = k;
        if (pos >= size) fatal(MSG_ERR_ARRAY_INI_COUNT, line_num+1, rem_fname(v_table[id_var].name), size, pos + 1);
        if (pos > maxpos) maxpos = pos;
    }

    int n = maxpos + 1;
    char **re = xcalloc(n + 1, sizeof(char *)), **im = xcalloc(n + 1, sizeof(char *));
    size_t lr = 1, li = 1;
    for (int k = 0; k < ini_n; k++)
    {
        int pos = (ini_rows > 0) ? ini_v[k].row * siz2 + ini_v[k].col : k;
        ini_split(&ini_v[k], &re[pos], &im[pos]);   // a literal and its sign
    }
    const char *zero = (type == 1) ? "0" : "0.0";
    for (int p = 0; p < n; p++) { lr += strlen(re[p] ? re[p] : zero) + 1; li += strlen(im[p] ? im[p] : "0.0") + 1; }

    ini_arr *a = xcalloc(1, sizeof(ini_arr));
    a->id = id_var; a->n = n;
    a->re = xmalloc(lr); a->im = xmalloc(li);
    char *pr = a->re, *pi = a->im;         // where the next value goes: one pass, not strcat (#94 K)
    for (int p = 0; p < n; p++)
    {
        const char *r = re[p] ? re[p] : zero, *i = im[p] ? im[p] : "0.0";
        size_t kr = strlen(r), ki = strlen(i);
        *pr++ = ' '; memcpy(pr, r, kr); pr += kr;
        *pi++ = ' '; memcpy(pi, i, ki); pi += ki;
        free(re[p]); free(im[p]);
    }
    *pr = *pi = '\0';
    free(re); free(im);
    a->next = ini_done; ini_done = a;

    for (int k = 0; k < ini_n; k++) free(ini_v[k].txt);
    ini_n = 0; ini_rows = 0; ini_row = -1;

    if (strcmp(fname, "") != 0) warn(MSG_WARN_ARRAY_INI_LOCAL, line_num+1, rem_fname(v_table[id_var].name));
}

// the #arrayd line(s) of an array initialized between braces; 0 if it is not one
static int ini_emit(int id_var, int size)
{
    ini_arr *a = ini_done;
    while (a && a->id != id_var) a = a->next;
    if (!a) return 0;
    int type = v_table[id_var].type;
    if (type == 3)
    {
        add_sinst(0, "#arrayd %s 3 %d %d%s\n", v_table[id_var].name,             size, a->n, a->re);
        add_sinst(0, "#arrayd %s 4 %d %d%s\n", v_table[get_img_id(id_var)].name, size, a->n, a->im);
    }
    else add_sinst(0, "#arrayd %s %d %d %d%s\n", v_table[id_var].name, type, size, a->n, a->re);
    return 1;
}

// Walker-time half: only the `#array` / `#arrays` directives.
void declar_arr_1d_emit(int id_var, int id_arg, int id_fname)
{
    int type = v_table[id_var].type;

    if (ini_emit(id_var, atoi(v_table[id_arg].name))) return;

    if (type == 1)
    {
        if (id_fname == -1) add_sinst(0, "#array %s 1 %s\n",     v_table[id_var].name, v_table[id_arg].name);
        else                add_sinst(0, "#arrays %s 1 %s %s\n", v_table[id_var].name, v_table[id_arg].name, v_table[id_fname].name);
    }
    else if (type == 2)
    {
        if (id_fname == -1) add_sinst(0, "#array %s 2 %s\n",     v_table[id_var].name, v_table[id_arg].name);
        else                add_sinst(0, "#arrays %s 2 %s %s\n", v_table[id_var].name, v_table[id_arg].name, v_table[id_fname].name);
    }
    else if (type == 3)
    {
        int idi = get_img_id(id_var);
        if (id_fname == -1)
        {
            add_sinst(0, "#array %s 3 %s\n", v_table[id_var].name, v_table[id_arg].name);
            add_sinst(0, "#array %s 4 %s\n", v_table[idi].name,    v_table[id_arg].name);
        }
        else
        {
            add_sinst(0, "#arrays %s 3 %s %s\n", v_table[id_var].name, v_table[id_arg].name, v_table[id_fname].name);
            add_sinst(0, "#arrays %s 4 %s %s\n", v_table[idi].name,    v_table[id_arg].name, v_table[id_fname].name);
        }
    }
}

// Public entry: parse-time work + defer the directive emit via STMT_DECLAR_ARR_1D.
void declar_arr_1d(int id_var, int id_arg, int id_fname)
{
    declar_arr_1d_parse(id_var, id_arg, id_fname);
    stmt_append(stmt_declar_arr_1d(id_var, id_arg, id_fname));
}

// Parse-time half of declar_arr_2d.
static void declar_arr_2d_parse(int id_var, int id_x, int id_y, int id_fname)
{
    check_name(id_var, 0);

    v_table[id_var].type       = type_tmp;
    v_table[id_var].used       = 0;
    v_table[id_var].fnid = find_var(fname);
    v_table[id_var].dims       = AR_2D;
    v_table[id_var].size       = atoi(v_table[id_x].name);
    v_table[id_var].siz2 = atoi(v_table[id_y].name);

    int type = type_tmp;

    if (type == 3) v_table[get_img_id(id_var)].dims = AR_2D;  // comp imag also 2D

    if (id_fname != -1)
        warn(MSG_INFO_ARRAY_FILE_INIT, v_table[id_fname].name, rem_fname(v_table[id_var].name), line_num+1);
    // the file is .mif content, loaded once: a local keeps state across calls
    if (id_fname != -1 && strcmp(fname, "") != 0)
        warn(MSG_WARN_ARRAY_FILE_LOCAL, line_num+1, rem_fname(v_table[id_var].name), v_table[id_fname].name);
}

// Walker-time half: the `#array` directive.
void declar_arr_2d_emit(int id_var, int id_x, int id_y, int id_fname)
{
    int type = v_table[id_var].type;
    int size = atoi(v_table[id_x].name) * atoi(v_table[id_y].name);

    if (ini_emit(id_var, size)) return;

    if (type == 1)
    {
        if (id_fname == -1) add_sinst(0, "#array %s 1 %d\n",     v_table[id_var].name, size);
        else                add_sinst(0, "#arrays %s 1 %d %s\n", v_table[id_var].name, size, v_table[id_fname].name);
    }
    else if (type == 2)
    {
        if (id_fname == -1) add_sinst(0, "#array %s 2 %d\n",     v_table[id_var].name, size);
        else                add_sinst(0, "#arrays %s 2 %d %s\n", v_table[id_var].name, size, v_table[id_fname].name);
    }
    else if (type == 3)
    {
        int idi = get_img_id(id_var);
        if (id_fname == -1)
        {
            add_sinst(0, "#array %s 3 %d\n", v_table[id_var].name, size);
            add_sinst(0, "#array %s 4 %d\n", v_table[idi].name,    size);
        }
        else
        {
            add_sinst(0, "#arrays %s 3 %d %s\n", v_table[id_var].name, size, v_table[id_fname].name);
            add_sinst(0, "#arrays %s 4 %d %s\n", v_table[idi].name,    size, v_table[id_fname].name);
        }
    }
    // no helper variable: 2D index flattening multiplies by the row size as
    // a constant (MLT <siz2>), so the declaration emits no instruction
}

// Public entry.
void declar_arr_2d(int id_var, int id_x, int id_y, int id_fname)
{
    declar_arr_2d_parse(id_var, id_x, id_y, id_fname);
    stmt_append(stmt_declar_arr_2d(id_var, id_x, id_y, id_fname));
}

// int v[4] = {1, 2, 3};
void declar_arr_1d_ini(int id_var, int id_arg)
{
    declar_arr_1d_parse(id_var, id_arg, -1);
    ini_take(id_var, atoi(v_table[id_arg].name), 0);
    stmt_append(stmt_declar_arr_1d(id_var, id_arg, -1));
}

// int m[2][3] = {{1, 2, 3}, {4, 5, 6}};  or  = {1, 2, 3, 4, 5, 6};
void declar_arr_2d_ini(int id_var, int id_x, int id_y)
{
    declar_arr_2d_parse(id_var, id_x, id_y, -1);
    ini_take(id_var, atoi(v_table[id_x].name) * atoi(v_table[id_y].name), atoi(v_table[id_y].name));
    stmt_append(stmt_declar_arr_2d(id_var, id_x, id_y, -1));
}

// ----------------------------------------------------------------------------
// array declarations initialized with Dirac notation -------------------------
// ----------------------------------------------------------------------------

// declares a 1D array with a Dirac expression, e.g. float v[4] # |M|u>;
// Parse-time work is identical to a plain `float v[N]` declaration; the
// walker (STMT_DECLAR_DIRAC case in ast.c) emits the #array directive and
// then the expression, as an assignment v # ...; would.
void declar_dirac(int id_name, int id_N, expr_node *rhs)
{
    declar_arr_1d_parse(id_name, id_N, -1);
    stmt_append(stmt_declar_dirac(id_name, id_N, rhs));
}
