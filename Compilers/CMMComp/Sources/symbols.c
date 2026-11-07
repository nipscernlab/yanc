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
#include "../Headers/functions.h"
#include "../Headers/symbols.h"
#include "../Headers/directives.h"
#include "../Headers/messages.h"
#include "../Headers/data_declar.h"
#include "../Headers/diag.h"
#include "../../common/xstr.h"
#include "../../common/asm_share.h"
#include "../../common/yanc_num.h"

int          v_count = 0;  // stores the size of the table
static int   v_cap   = 0;  // current capacity of the parallel arrays

// AoS storage: one row per identifier. All attributes (name, type, used,
// dims, is_const, size, cols, n_params, param_types, owner) live here. Grown on demand by var_grow.
symbol *v_table = NULL;

// The table moves to a bigger block; each name stays where add_var put it, so
// code that holds a name, `const char *X = sym(lex_id("..."))->name`, and
// then adds a symbol (stdlib.c does it about 50 times) still reads the same
// name after the move. Nothing keeps a pointer to a row.
static void var_grow(int needed)
{
    if (needed <= v_cap) return;
    int new_cap = v_cap ? v_cap : 256;
    while (new_cap < needed) new_cap *= 2;

    v_table = xrealloc(v_table, (size_t)new_cap * sizeof(*v_table));
    memset(v_table + v_cap, 0, (size_t)(new_cap - v_cap) * sizeof(*v_table));

    v_cap = new_cap;
}

// searches the table for the variable (-1 if not found)
int find_var(const char *val)
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
symbol *sym(int id) {return &v_table[id];}   // v_table read after the argument (symbols.h)

void add_var(const char *var)
{
    var_grow(v_count + 1);
    v_table[v_count].name = xstrdup(var);
    v_count++;
}

// checks which variables and functions were used (at the end of the parse)
void check_var(void)
{
    if (pctx.mainok == 0) fatal(MSG_ERR_NO_MAIN);

    // walk the whole variable table
    for (int i = 0; i < v_count; i++)
    {
        // check whether a variable was declared but not used ...
        if (((v_table[i].type == TY_INT) || (v_table[i].type == TY_FLOAT) || (v_table[i].type == TY_COMP)) && (v_table[i].used == 0) && !v_table[i].wave)
        {
            // v_table[i].owner records the variable's owning function: a valid
            // v_table index for locals, or "" / -1 for globals. declare_arr_1d
            // etc. call find_var(fname) which returns -1 when fname is the
            // empty string and no "" entry exists in v_table yet - so treat
            // any out-of-range owner as the global case.
            int owner = v_table[i].owner;
            int is_global = (owner < 0) || (strcmp(v_table[owner].name, "") == 0);
            if (is_global)
                warn(MSG_WARN_UNUSED_GLOBAL_VAR, v_table[i].name, v_table[i].name);
            else
            {
                char *name = strip_fn(v_table[i].name, v_table[owner].name);
                warn(MSG_WARN_UNUSED_LOCAL_VAR, name, v_table[owner].name, name);
            }
        }

        // check whether a function was declared but not used: the function
        // types are 6 to 9 (void, int, float, comp); 5 is a comp constant
        if (v_table[i].type >= TY_FVOID && v_table[i].type <= TY_FCOMP && v_table[i].used == 0)
            warn(MSG_WARN_UNUSED_FUNCTION, v_table[i].name);
    }
}

// #WAVE name: the variable stays in the program, and so in the waveform, even
// if nothing reads it (asm_unread asks v_table[id].wave). It names a variable
// declared before it, a global at the top level or a local inside a function
void directive_wave(int id)
{
    char *name = strip_fn_prefix(v_table[id].name);
    if (v_table[id].type == TY_NONE) fatal(MSG_ERR_DECL_FIRST, pctx.line_num+1, name);
    if (v_table[id].type > TY_COMP)  fatal(MSG_ERR_WAVE_NOT_VAR, pctx.line_num+1, name);
    v_table[id].wave = 1;
}

// a local is <func>_<name>: strip_fn gives <name>; anything else, a global
// even if it starts like the function (`mainly` in `main`), keeps its name
char *strip_fn(char *var, const char *func)
{
    size_t L = strlen(func);
    if (L == 0) return var;
    return (strncmp(var, func, L) == 0 && var[L] == '_') ? var + L + 1 : var;   // #94 A
}

// the name a variable has in the function being compiled: the parse-time
// fname, or during the deferred AST walk (when fname is "") emit_fname, the
// function the walker is in. For messages only: lex_id names with fname
char *strip_fn_prefix(char *var)
{
    return strip_fn(var, ectx.emit_fname[0] != '\0' ? ectx.emit_fname : pctx.fname);
}

// ----------------------------------------------------------------------------
// one name, one owner in the .asm --------------------------------------------
// ----------------------------------------------------------------------------
// A user name is a word (or a label) of the .asm as the table spells it: the
// local `x` of `main` is `main_x`. Before a declaration takes its row, make
// sure the name is not an instruction (the assembler would read the
// instruction) and that nobody else owns it (both would silently share one
// word). Compiler-made names start with `__`, which the lexer refuses.

// who owns row id, in words, for the messages (the block is the caller's)
static char *name_owner(int id)
{
    int  t = v_table[id].type, f = v_table[id].owner;
    if (t >= 6 && t <= 9) return xasprintf(MSG_NAME_FUNC, v_table[id].name);
    char *nm = xstrdup(v_table[id].name);
    if (t == 4) nm[strlen(nm) - 2] = '\0';                    // drop the "_i"
    char *raw = nm;
    if (f >= 0)
    {
        size_t L = strlen(v_table[f].name);
        if (strncmp(nm, v_table[f].name, L) == 0 && nm[L] == '_') raw = nm + L + 1;
    }
    char *base = f < 0 ? xasprintf(MSG_NAME_GLOBAL, raw) : xasprintf(MSG_NAME_LOCAL, raw, v_table[f].name);
    free(nm);
    if (t != 4) return base;
    char *out = xasprintf(MSG_NAME_IMAG, base);
    free(base);
    return out;
}

static void name_taken(const char *me, const char *as, const char *who)
{
    fatal(MSG_ERR_NAME_TAKEN, pctx.line_num+1, me, as, who);
}

// called by every declaration (variable, array, parameter, function) on its
// row, before the row is filled
void check_name(int id, int is_fun)
{
    char *as = v_table[id].name;
    char *me;                             // the name being declared, in words (it ends in fatal or is freed)

    if (is_fun)
    {
        const char *why = strcmp(as, "fim"    ) == 0 ? MSG_NAME_FIM        :
                          strcmp(as, "global" ) == 0 ? MSG_NAME_GLOBAL_LOG :
                          strcmp(as, "num_ins") == 0 ? MSG_NAME_NUM_INS    : NULL;
        if (why) fatal(MSG_ERR_NAME_RESERVED, pctx.line_num+1, as, why);
        me = xasprintf(MSG_NAME_FUNC, as);
    }
    else if (strcmp(pctx.fname, "") == 0) me = xasprintf(MSG_NAME_GLOBAL, as);
    else
    {
        size_t L = strlen(pctx.fname);
        char *raw = (strncmp(as, pctx.fname, L) == 0 && as[L] == '_') ? as + L + 1 : as;
        me = xasprintf(MSG_NAME_LOCAL, raw, pctx.fname);
    }

    if (asm_is_mnemonic(as))
    {
        char *q = xasprintf("'%s'", as);
        fatal(strstr(me, q) ? MSG_ERR_NAME_IS_MNEMONIC : MSG_ERR_NAME_MNEMONIC, pctx.line_num+1, me, as);
    }

    if (v_table[id].type != TY_NONE)
    {
        int t = v_table[id].type;
        if (!is_fun && t < 6 && t != 4 && v_table[id].owner == find_var(pctx.fname))
        {   // the same name declared twice in the same place
            fatal(MSG_ERR_VAR_EXISTS, pctx.line_num+1, strip_fn_prefix(as));
        }
        name_taken(me, as, name_owner(id));
    }

    if (is_fun) {free(me); return;}

    // a comp `z` also owns `z_i`, its imaginary part, and the other way round
    if (pctx.type_tmp == TY_COMP)
    {
        char *nm = xasprintf("%s_i", as);
        int idi = find_var(nm);
        if (idi >= 0 && v_table[idi].type != TY_NONE)
            name_taken(xasprintf(MSG_NAME_IMAG, me), nm, name_owner(idi));
        free(nm);
    }
    size_t L = strlen(as);
    if (L > 2 && strcmp(as + L - 2, "_i") == 0)
    {
        char *nm = xasprintf("%.*s", (int)(L - 2), as);
        int idr = find_var(nm);
        if (idr >= 0 && v_table[idr].type == TY_COMP)
            name_taken(me, as, xasprintf(MSG_NAME_IMAG, name_owner(idr)));
        free(nm);
    }
    free(me);
}

// a declared local of some function: in the .asm it is <function>_<name>,
// a name the program does not have (#108 A10)
static int is_local_row(int id)
{
    int t = v_table[id].type, f = v_table[id].owner;
    return t >= TY_INT && t <= TY_IMAG && f >= 0 && f < v_count && v_table[f].type >= TY_FVOID;
}

// used when the lexer finds an ID
int lex_id(const char *text)
{
    if (strcmp(text,"i") == 0)
        fatal(MSG_ERR_RESERVED_I, pctx.line_num+1);

    if ((int)strlen(text) > CMM_NAME_MAX)
        fatal(MSG_ERR_NAME_TOO_LONG, pctx.line_num+1, (int)strlen(text), CMM_NAME_MAX);

    int id = find_var(text);                                      // first check whether there is a global variable
    if (id != -1 && !is_local_row(id)) return id;                 // f_y is no name for f's local y

    char *var_name = strcmp(pctx.fname, "") != 0                       // otherwise, prepend the current function name
                   ? xasprintf("%s_%s", pctx.fname, text) : xstrdup(text);
    if (find_var(var_name) == -1) add_var(var_name);              // create the local variable if it does not exist yet
    id = find_var(var_name);
    free(var_name);
    return id;
}

// the same lookup as lex_id (a global first, then a local of the current
// function), without adding the name: the lexer asks it whether the name
// after a bar is an array (DBAR in CMMComp.l)
int find_id(const char *text)
{
    int k = find_var(text);
    if (k != -1 && is_local_row(k)) k = -1;                       // as lex_id: f_y is no name
    if (k != -1 || strcmp(pctx.fname, "") == 0) return k;

    char *var_name = xasprintf("%s_%s", pctx.fname, text);
    k = find_var(var_name);
    free(var_name);
    return k;
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
            if (*q > '7') fatal(MSG_ERR_OCTAL_DIGIT, pctx.line_num+1, text);
    }
    if (base_out) *base_out = base;
    errno = 0;
    unsigned long long v = strtoull(p, NULL, base);       // stops at the suffix
    return (errno == ERANGE) ? ULLONG_MAX : v;
}

// the word width for a bit pattern: #NUBITS, 2..32
static int word_bits(void) { return cfg.nubits < 2 ? 2 : cfg.nubits > 32 ? 32 : cfg.nubits; }

// what the lexer returns for an int literal: 0 a plain INUM; 1 (NINUM) a hex,
// octal or binary literal with the word's sign bit set, a negative int as in
// C (0xFFFFFFFF is -1 at 32 bits, 0xFF at 8); 2 (NIMIN) the most negative one;
// 3 (DIMIN) the decimal 2^(w-1), 128 at 8 bits: it fits only after a minus,
// so the grammar takes it only as `- 128`, the most negative int
int inum_kind(const char *text)
{
    int base; unsigned long long v = inum_value(text, &base);
    int w = word_bits();
    if (base == 10) return (v == (1ULL << (w - 1))) ? 3 : 0;
    if (v < (1ULL << (w - 1)) || v >= (1ULL << w)) return 0;
    return (v == (1ULL << (w - 1))) ? 2 : 1;
}

// the magnitude of a NINUM (2^w - v) as a positive literal; for a NIMIN, the
// largest int (the grammar writes -max - 1)
int lex_inum_neg(const char *text)
{
    int w = word_bits(); char t[32];
    unsigned long long v = inum_value(text, NULL);
    unsigned long long m = (inum_kind(text) == 2) ? (1ULL << (w - 1)) - 1 : (1ULL << w) - v;
    snprintf(t, sizeof(t), "%llu", m);
    return lex_inum(t);
}

// the magnitude of the most negative int, 2^(w-1), as a table entry: a brace
// list writes it with its minus sign, and the lexer gives it for a DIMIN (128
// at 8 bits, 2^31 at 32), so it skips the range check
int inum_min_id(void)
{
    char t[32]; snprintf(t, sizeof(t), "%llu", 1ULL << (word_bits() - 1));
    if (find_var(t) == -1) add_var(t);
    int id = find_var(t);
    v_table[id].is_const = 1;
    return id;
}

// a decimal int past the word (also a DIMIN anywhere but after a minus)
void inum_overflow(void)
{
    fatal(MSG_ERR_INT_MAX_OVERFLOW, pctx.line_num+1, (int)((1LL << (word_bits() - 1)) - 1));
}

// an int literal used as a value (an expression, a list between braces, a
// case) must fit the #NUBITS word: 200 at 8 bits is an error, not -56 (issue
// #14). A size, a port or a directive is no value of the word and is not held
// to it: the lexer takes every literal up to 2^31 - 1.
int inum_word(int id)
{
    if (strtoull(v_table[id].name, NULL, 10) > (1ULL << (word_bits() - 1)) - 1) inum_overflow();
    return id;
}

// an array size past what an int index reaches (2^(w-1) positions, indices 0
// to 2^(w-1) - 1) is allowed, with a warning: 200 positions at 8 bits
int arr_size(int id)
{
    unsigned long long n = strtoull(v_table[id].name, NULL, 10), half = 1ULL << (word_bits() - 1);
    if (n > half) warn(MSG_WARN_ARR_SIZE_WORD, pctx.line_num+1, (int)n, word_bits(), (int)(half - 1));
    return id;
}

// used when the lexer finds an int constant
int lex_inum(const char *text)
{
    // range check ------------------------------------------------------------
    // no negative constants ever reach this function

    // the largest int of a 32-bit word: the literal's own limit. Whether it
    // fits #NUBITS depends on where it stands, so the grammar checks that
    // (inum_word)
    long long max = (1LL << 31) - 1;
    unsigned long long num = inum_value(text, NULL);

    if (num > (unsigned long long)max) inum_overflow();

    // add to the table, in decimal: the .asm reads only that -----------------

    char t[32]; snprintf(t, sizeof(t), "%llu", num);

    if (find_var(t) == -1) add_var(t);

    int id = find_var(t);

    v_table[id].is_const = 1;

    return id;
}

// writes a float literal the one way the .asm reads it: C's forms (.5, 5.,
// 007.5, 1e-8, 2.5E+3f) become [-]digits.digits[e[-]digits], with no suffix,
// so asmcomp's lexer takes it and two spellings of one value are one constant
char *fnum_text(const char *in)
{
    char *buf = xmalloc(strlen(in) + 4), *o = buf;      // at most a 0 before the point, ".0" and the end
    const char *p = in;

    if (*p == '+' || *p == '-') { if (*p == '-') *o++ = '-'; p++; }
    while (*p == '0' && isdigit((unsigned char)p[1])) p++;              // leading zeros
    if (*p == '.') *o++ = '0';                                          // .5 -> 0.5
    while (isdigit((unsigned char)*p)) *o++ = *p++;
    *o++ = '.';
    if (*p == '.') p++;
    if (!isdigit((unsigned char)*p)) *o++ = '0';                        // 5. -> 5.0
    while (isdigit((unsigned char)*p)) *o++ = *p++;
    if (*p == 'e' || *p == 'E')
    {
        *o++ = 'e'; p++;
        if (*p == '+') p++; else if (*p == '-') *o++ = *p++;
        while (isdigit((unsigned char)*p)) *o++ = *p++;
    }
    *o = '\0';                                                          // the f/F/l/L suffix stays out
    return buf;
}

// splits a comp literal ("-1.5-2i", "1e-8+2e-8i", no spaces) at the sign
// between its parts: a sign right after an e belongs to the exponent. re and
// im come out as the source wrote them (im keeps its sign, loses the i)
void cnum_split(const char *txt, char **re, char **im)
{
    int len = (int)strlen(txt), k = len - 2;
    while (k > 0 && !((txt[k] == '+' || txt[k] == '-') && txt[k-1] != 'e' && txt[k-1] != 'E')) k--;
    *re = xasprintf("%.*s", k, txt);
    int b = (txt[k] == '+') ? k + 1 : k;
    *im = xasprintf("%.*s", len - 1 - b, txt + b);
}

// used when the lexer finds a float constant
int lex_fnum(const char *text)
{
    char *t = fnum_text(text);

    // encode it as asmcomp will (common/yanc_num.c, exact): range and residual

    yn_float r;
    yn_encode(t, cfg.nbmant, cfg.nbexpo, cfg.fround, &r);

    double max = (pow(2,cfg.nbmant)-1) * pow(2, pow(2,cfg.nbexpo-1)-1);         // largest representable magnitude
    double min =                     pow(2,-pow(2,cfg.nbexpo-1));           // smallest representable magnitude

    if (r.overflow)                fatal(MSG_ERR_FLOAT_MAX, pctx.line_num+1, max);
    if (r.underflow && !r.flushed) fatal(MSG_ERR_FLOAT_MIN, pctx.line_num+1, min);

    // a nonzero constant flushed to zero (below the smallest normal number at
    // #FROUND >= 1) is not an approximation the user can shrug off: warn
    if (r.flushed)
        warn(MSG_WARN_CONST_FLUSHED, pctx.line_num+1, text, pow(2,cfg.nbmant-1)*pow(2,-pow(2,cfg.nbexpo-1)), cfg.fround);
    else if (r.inexact && !ectx.fnum_quiet)
    {
        double mf = ldexp((double)r.m, (int)r.e);
        warn(MSG_INFO_CONST_APPROX, text, pctx.line_num+1, r.s ? -mf : mf, r.delta);
    }

    // add to the table -------------------------------------------------------

    if (find_var(t) == -1) add_var(t);

    int id = find_var(t);
    free(t);

    v_table[id].is_const = 1;

    return id;
}

// the comp literal re + (or -, sub) the imaginary literal imid: re is the
// real part as the source wrote it, with its sign ("3", "-1.5")
int cnum_join(const char *re, int sub, int imid)
{
    char *r0, *im;
    cnum_split(v_table[imid].name, &r0, &im);
    const char *sim = !sub ? im : im[0] == '-' ? im + 1 : NULL;     // NULL: "-" and im
    char *t = sim ? xasprintf("%s%s%si", re, sim[0] == '-' ? "" : "+", sim)
                  : xasprintf("%s-%si", re, im);
    int id = lex_cnum(t);
    free(t); free(r0); free(im);
    return id;
}

// the text of literal id with a minus in front, for a brace list's -1 + 2i
char *neg_text(int id)
{
    return xasprintf("-%s", v_table[id].name);
}

// the imaginary part of comp literal id, negated when neg, as a float literal
int cnum_imag_fnum(int id, int neg)
{
    char *re, *im;
    cnum_split(v_table[id].name, &re, &im);
    char *t = !neg ? xstrdup(im) : im[0] == '-' ? xstrdup(im + 1) : xasprintf("-%s", im);
    int r = lex_fnum(t);
    free(t); free(re); free(im);
    return r;
}

// is the comp literal id an imaginary one (its real part zero)?
int cnum_is_imag(int id)
{
    char *re, *im;
    cnum_split(v_table[id].name, &re, &im);
    int r = strtod(re, NULL) == 0.0;
    free(re); free(im);
    return r;
}

// used when the lexer finds a comp constant
int lex_cnum(char *text)
{
    // strip whitespace -------------------------------------------------------

    int j = 0;                                    // in place: j never passes i
    for (int i = 0; text[i] != '\0'; i++)
        if (text[i] != ' ' && text[i] != '\t') text[j++] = text[i];
    text[j] = '\0';

    // each part as a float literal is written (fnum_text) --------------------

    char *re0, *im0;
    cnum_split(text, &re0, &im0);
    char *re = fnum_text(re0), *im = fnum_text(im0);
    char *t  = xasprintf("%s%s%si", re, im[0] == '-' ? "" : "+", im);
    free(re0); free(im0); free(re); free(im);

    // add to the table -------------------------------------------------------

    if (find_var(t) == -1) add_var(t);
    int id = find_var(t);
    free(t);
    return id;
}
