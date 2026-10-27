// ----------------------------------------------------------------------------
// identifier table implementation --------------------------------------------
// ----------------------------------------------------------------------------

// global includes
#include <string.h>
#include <stdlib.h>
#include   <math.h>
#include  <ctype.h>
#include  <stdio.h>
#include  <errno.h>
#include <limits.h>

// local includes
#include "../Headers/global.h"
#include "../Headers/funcoes.h"
#include "../Headers/variaveis.h"
#include "../Headers/diretivas.h"
#include "../Headers/messages.h"
#include "../Headers/data_declar.h"
#include "../../common/asm_share.h"
#include "../../common/yanc_num.h"

int          v_count = 0;  // stores the size of the table
static int   v_cap   = 0;  // current capacity of the parallel arrays

// AoS storage: one row per identifier. All attributes (name, type, used,
// isar, isco, size, siz2, npar, ptyp, fnid) live here. Grown on demand by var_grow.
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

// ----------------------------------------------------------------------------
// one name, one owner in the .asm --------------------------------------------
// ----------------------------------------------------------------------------
// A user name is a word (or a label) of the .asm as the table spells it: the
// local `x` of `main` is `main_x`. Before a declaration takes its row, make
// sure the name is not an instruction (the assembler would read the
// instruction) and that nobody else owns it (both would silently share one
// word). Compiler-made names start with `__`, which the lexer refuses.

// who owns row id, in words, for the messages
static void name_owner(int id, char *out, size_t n)
{
    int  t = v_table[id].type, f = v_table[id].fnid;
    char nm[512]; strcpy(nm, v_table[id].name);
    if (t >= 6 && t <= 9) {snprintf(out, n, MSG_NAME_FUNC, nm); return;}
    if (t == 4) nm[strlen(nm) - 2] = '\0';                    // drop the "_i"
    char *raw = nm;
    if (f >= 0)
    {
        size_t L = strlen(v_table[f].name);
        if (strncmp(nm, v_table[f].name, L) == 0 && nm[L] == '_') raw = nm + L + 1;
    }
    char base[1024];
    if (f < 0) snprintf(base, sizeof(base), MSG_NAME_GLOBAL, raw);
    else       snprintf(base, sizeof(base), MSG_NAME_LOCAL , raw, v_table[f].name);
    if (t == 4) snprintf(out, n, MSG_NAME_IMAG, base);
    else        snprintf(out, n, "%s", base);
}

static void name_taken(const char *me, const char *as, const char *who)
{
    fprintf(stderr, MSG_ERR_NAME_TAKEN, line_num+1, me, as, who);
    exit(EXIT_FAILURE);
}

// called by every declaration (variable, array, parameter, function) on its
// row, before the row is filled
void check_name(int id, int is_fun)
{
    char *as = v_table[id].name;
    char  me[1200], who[1200];

    if (is_fun)
    {
        const char *why = strcmp(as, "fim"    ) == 0 ? MSG_NAME_FIM        :
                          strcmp(as, "global" ) == 0 ? MSG_NAME_GLOBAL_LOG :
                          strcmp(as, "num_ins") == 0 ? MSG_NAME_NUM_INS    : NULL;
        if (why) {fprintf(stderr, MSG_ERR_NAME_RESERVED, line_num+1, as, why); exit(EXIT_FAILURE);}
        snprintf(me, sizeof(me), MSG_NAME_FUNC, as);
    }
    else if (strcmp(fname, "") == 0) snprintf(me, sizeof(me), MSG_NAME_GLOBAL, as);
    else
    {
        size_t L = strlen(fname);
        char *raw = (strncmp(as, fname, L) == 0 && as[L] == '_') ? as + L + 1 : as;
        snprintf(me, sizeof(me), MSG_NAME_LOCAL, raw, fname);
    }

    if (asm_is_mnemonic(as))
    {
        char q[600]; snprintf(q, sizeof(q), "'%s'", as);
        fprintf(stderr, strstr(me, q) ? MSG_ERR_NAME_IS_MNEMONIC : MSG_ERR_NAME_MNEMONIC, line_num+1, me, as);
        exit(EXIT_FAILURE);
    }

    if (v_table[id].type != 0)
    {
        int t = v_table[id].type;
        if (!is_fun && t < 6 && t != 4 && v_table[id].fnid == find_var(fname))
        {   // the same name declared twice in the same place
            fprintf(stderr, MSG_ERR_VAR_EXISTS, line_num+1, rem_fname(as, fname));
            exit(EXIT_FAILURE);
        }
        name_owner(id, who, sizeof(who));
        name_taken(me, as, who);
    }

    if (is_fun) return;

    // a comp `z` also owns `z_i`, its imaginary part, and the other way round
    char nm[1024];
    if (type_tmp == 3)
    {
        snprintf(nm, sizeof(nm), "%s_i", as);
        int idi = find_var(nm);
        if (idi >= 0 && v_table[idi].type != 0)
        {
            char im[1300]; snprintf(im, sizeof(im), MSG_NAME_IMAG, me);
            name_owner(idi, who, sizeof(who));
            name_taken(im, nm, who);
        }
    }
    size_t L = strlen(as);
    if (L > 2 && strcmp(as + L - 2, "_i") == 0)
    {
        snprintf(nm, sizeof(nm), "%.*s", (int)(L - 2), as);
        int idr = find_var(nm);
        if (idr >= 0 && v_table[idr].type == 3)
        {
            char base[1024]; name_owner(idr, base, sizeof(base));
            snprintf(who, sizeof(who), MSG_NAME_IMAG, base);
            name_taken(me, as, who);
        }
    }
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

// the value of an int literal in any form C writes it: 42, 052 (octal),
// 0x2A, 0b101010, with a u/U/l/L suffix (*base, if given, says which)
static unsigned long long inum_value(const char *text, int *base_out)
{
    int base = 10; const char *p = text;
    if      (p[0] == '0' && (p[1] == 'x' || p[1] == 'X')) {base = 16; p += 2;}
    else if (p[0] == '0' && (p[1] == 'b' || p[1] == 'B')) {base =  2; p += 2;}
    else if (p[0] == '0' && isdigit((unsigned char)p[1]))
    {
        base = 8; p++;
        for (const char *q = p; isdigit((unsigned char)*q); q++)
            if (*q > '7') {fprintf(stderr, MSG_ERR_OCTAL_DIGIT, line_num+1, text); exit(EXIT_FAILURE);}
    }
    if (base_out) *base_out = base;
    errno = 0;
    unsigned long long v = strtoull(p, NULL, base);       // stops at the suffix
    return (errno == ERANGE) ? ULLONG_MAX : v;
}

// the word width for a bit pattern: #NUBITS, 2..32
static int word_bits(void) { return nubits < 2 ? 2 : nubits > 32 ? 32 : nubits; }

// what the lexer returns for an int literal: 0 a plain INUM; 1 (NINUM) a hex,
// octal or binary literal with the word's sign bit set, a negative int as in
// C (0xFFFFFFFF is -1 at 32 bits, 0xFF at 8); 2 (NIMIN) the most negative one
int inum_kind(const char *text)
{
    int base; unsigned long long v = inum_value(text, &base);
    int w = word_bits();
    if (base == 10 || v < (1ULL << (w - 1)) || v >= (1ULL << w)) return 0;
    return (v == (1ULL << (w - 1))) ? 2 : 1;
}

// the magnitude of a NINUM (2^w - v) as a positive literal; for a NIMIN, the
// largest int (the grammar writes -max - 1)
int exec_inum_neg(char *text)
{
    int w = word_bits(); char t[32];
    unsigned long long v = inum_value(text, NULL);
    unsigned long long m = (inum_kind(text) == 2) ? (1ULL << (w - 1)) - 1 : (1ULL << w) - v;
    snprintf(t, sizeof(t), "%llu", m);
    return exec_inum(t);
}

// the magnitude of the most negative int, 2^(w-1), as a table entry: only a
// brace list writes it (with its minus sign), so it skips the range check
int inum_min_id(void)
{
    char t[32]; snprintf(t, sizeof(t), "%llu", 1ULL << (word_bits() - 1));
    if (find_var(t) == -1) add_var(t);
    int id = find_var(t);
    v_table[id].isco = 1;
    return id;
}

// used when the lexer finds an int constant
int exec_inum(char *text)
{
    // range check ------------------------------------------------------------
    // no negative constants ever reach this function

    // the width the directives give so far, never past 32: they may be in flux
    // (#NBMANT 25 read before #NBEXPO 6 is a 34-bit word for a moment)
    int       w   = nbmant + nbexpo + 1;
    long long max = (1LL << ((w < 32 ? w : 32) - 1)) - 1;
    unsigned long long num = inum_value(text, NULL);

    if (num > (unsigned long long)max) {fprintf (stderr, MSG_ERR_INT_MAX_OVERFLOW, line_num+1, (int)max); exit(EXIT_FAILURE);}

    // add to the table, in decimal: the .asm reads only that -----------------

    char t[32]; snprintf(t, sizeof(t), "%llu", num);

    if (find_var(t) == -1) add_var(t);

    int id = find_var(t);

    v_table[id].isco = 1;

    return id;
}

// writes a float literal the one way the .asm reads it: C's forms (.5, 5.,
// 007.5, 1e-8, 2.5E+3f) become [-]digits.digits[e[-]digits], with no suffix,
// so asmcomp's lexer takes it and two spellings of one value are one constant
void fnum_text(const char *in, char *out, size_t n)
{
    char buf[sizeof(v_table[0].name)], *o = buf, *end = buf + sizeof(buf) - 4;
    const char *p = in;

    if (*p == '+' || *p == '-') { if (*p == '-') *o++ = '-'; p++; }
    while (*p == '0' && isdigit((unsigned char)p[1])) p++;              // leading zeros
    if (*p == '.') *o++ = '0';                                          // .5 -> 0.5
    while (isdigit((unsigned char)*p) && o < end) *o++ = *p++;
    *o++ = '.';
    if (*p == '.') p++;
    if (!isdigit((unsigned char)*p)) *o++ = '0';                        // 5. -> 5.0
    while (isdigit((unsigned char)*p) && o < end) *o++ = *p++;
    if (*p == 'e' || *p == 'E')
    {
        *o++ = 'e'; p++;
        if (*p == '+') p++; else if (*p == '-') *o++ = *p++;
        while (isdigit((unsigned char)*p) && o < end) *o++ = *p++;
    }
    *o = '\0';                                                          // the f/F/l/L suffix stays out
    snprintf(out, n, "%s", buf);
}

// splits a comp literal ("-1.5-2i", "1e-8+2e-8i", no spaces) at the sign
// between its parts: a sign right after an e belongs to the exponent. re and
// im come out as the source wrote them (im keeps its sign, loses the i)
void cnum_split(const char *txt, char *re, char *im)
{
    int len = (int)strlen(txt), k = len - 2;
    while (k > 0 && !((txt[k] == '+' || txt[k] == '-') && txt[k-1] != 'e' && txt[k-1] != 'E')) k--;
    memcpy(re, txt, k); re[k] = '\0';
    int b = (txt[k] == '+') ? k + 1 : k;
    memcpy(im, txt + b, len - 1 - b); im[len - 1 - b] = '\0';
}

// used when the lexer finds a float constant
int exec_fnum(char *text)
{
    char t[sizeof(v_table[0].name)];
    fnum_text(text, t, sizeof(t));

    // encode it as asmcomp will (common/yanc_num.c, exact): range and residual

    yn_float r;
    yn_encode(t, nbmant, nbexpo, fround, &r);

    double max = (pow(2,nbmant)-1) * pow(2, pow(2,nbexpo-1)-1);         // largest representable magnitude
    double min =                     pow(2,-pow(2,nbexpo-1));           // smallest representable magnitude

    if (r.overflow)                {fprintf (stderr, MSG_ERR_FLOAT_MAX, line_num+1, max); exit(EXIT_FAILURE);}
    if (r.underflow && !r.flushed) {fprintf (stderr, MSG_ERR_FLOAT_MIN, line_num+1, min); exit(EXIT_FAILURE);}

    // a nonzero constant flushed to zero (below the smallest normal number at
    // #FROUND >= 1) is not an approximation the user can shrug off: warn
    if (r.flushed)
        printf(MSG_WARN_CONST_FLUSHED, line_num+1, text, pow(2,nbmant-1)*pow(2,-pow(2,nbexpo-1)), fround);
    else if (r.inexact)
    {
        double mf = ldexp((double)r.m, (int)r.e);
        printf(MSG_INFO_CONST_APPROX, text, line_num+1, r.s ? -mf : mf, r.delta);
    }

    // add to the table -------------------------------------------------------

    if (find_var(t) == -1) add_var(t);

    int id = find_var(t);

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
        while (temp[i] == ' ' || temp[i] == '\t') i++;
        text[j] = temp[i];
        if (temp[i] == '\0') break;
        i++;
        j++;
    }
    text[j] = '\0';

    // each part as a float literal is written (fnum_text) --------------------

    char re[sizeof(v_table[0].name)], im[sizeof(v_table[0].name)], t[2*sizeof(v_table[0].name) + 4];
    cnum_split(text, re, im);
    fnum_text(re, re, sizeof(re));
    fnum_text(im, im, sizeof(im));
    snprintf(t, sizeof(t), "%s%s%si", re, im[0] == '-' ? "" : "+", im);

    // add to the table -------------------------------------------------------

    if (find_var(t) == -1) add_var(t);
    return find_var(t);
}
