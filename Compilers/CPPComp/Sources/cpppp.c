// ----------------------------------------------------------------------------
// CPPComp — the C preprocessor --------------------------------------------------
// ----------------------------------------------------------------------------
// cppcomp's first step, as cpp is gcc's: pp_run() turns the source into one
// text, kept in memory, with a map from each of its lines back to the file and
// line it came from (the messages point there). `cppcomp -E` prints the text.
// Each line is read with its comments already replaced by a space (a /* ... */
// may span lines), and a call of a function-like macro may go on over the
// next lines. Handles:
//   #include "name"          (relative to current file + -I paths)
//   #include <name>          (-I paths, then the Header/ folder next to the tool)
//   #include NAME            (a macro that gives one of the two forms above)
//   #define NAME body        (object- and function-like macros, incl.
//                             variadic `...`/__VA_ARGS__, # stringize, ## paste)
//   #undef NAME
//   #if / #elif / #ifdef / #ifndef / #else / #endif
//   #pragma once             (skip the file on any later #include)
//   #pragma ...              (passed on, as `#pragma <rest>`, to the lexer)
//   #error / #warning
// Any other directive is an error. A macro is not expanded again inside its
// own expansion (#define x x+1 gives x+1), as in C.
// ----------------------------------------------------------------------------

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "../Headers/cpppp.h"
#include "../Headers/messages.h"
#include "../../common/xstr.h"
#include "../../common/exe_dir.h"
#include <stdarg.h>

// where the preprocessor is reading: an error names it, as cppcomp's others
// do (#132: "can't find #include" said neither the file nor the line)
static const char *pp_at_file = NULL;
static int         pp_at_line = 0;

static MSG_FMT(1, 2) MSG_NORET void pp_fail(const char *fmt, ...);
static void pp_fail(const char *fmt, ...)
{
    va_list ap;
    if (pp_at_file) fprintf(stderr, "%s:%d: %s", pp_at_file, pp_at_line, MSG_PREFIX_ERROR);
    else            fprintf(stderr, "cppcomp: %s", MSG_PREFIX_ERROR);
    va_start(ap, fmt); vfprintf(stderr, fmt, ap); va_end(ap);
    fputc('\n', stderr);
    exit(1);
}

static const char **incdirs;           // the -I folders, then the YANC headers
static int          n_incdirs = 0, incdirs_cap = 0;

// files marked `#pragma once`: canonical paths to skip on any later #include.
static char **once_paths;               // the files a #pragma once closed
static int   n_once = 0;

// canonicalise a path so the same file reached via different #include spellings
// compares equal (absolute, forward slashes, lowercased — Windows is
// case-insensitive). Falls back to the input on failure.
static char *path_canon(const char *in)
{
#ifdef _WIN32
    char *full = _fullpath(NULL, in, 0);   // NULL: the C library allocates it
    if (full) {
        for (char *c = full; *c; c++) *c = *c == '\\' ? '/' : (char)tolower((unsigned char)*c);
        char *r = xstrdup(full); free(full);
        return r;
    }
#else
    char *rp = realpath(in, NULL);         // NULL: allocated (POSIX.1-2008)
    if (rp) { char *r = xstrdup(rp); free(rp); return r; }
#endif
    return xstrdup(in);
}

static int once_seen(const char *canon)
{
    for (int i = 0; i < n_once; i++) if (strcmp(once_paths[i], canon) == 0) return 1;
    return 0;
}
static void once_mark(const char *canon)
{
    if (once_seen(canon)) return;
    once_paths = xrealloc(once_paths, (size_t)(n_once + 1) * sizeof *once_paths);
    once_paths[n_once++] = xstrdup(canon);
}

typedef struct macro {
    char *name;
    char *body;
    int   is_func;                 // 1 = function-like macro
    int   is_variadic;             // 1 = trailing `...` (use __VA_ARGS__ in body)
    char **params;
    int   n_params;                // named params (excludes `...`)
    int   busy;                    // being expanded: its name inside stays as it is
    struct macro *next;
} macro;
static macro *macros = NULL;

// the name of n characters at s is `name`
static int name_is(const char *s, size_t n, const char *name)
{
    return strlen(name) == n && strncmp(s, name, n) == 0;
}

// the macro named by the n characters at s, without copying them
static macro *macro_find_n(const char *s, size_t n)
{
    for (macro *m = macros; m; m = m->next) if (name_is(s, n, m->name)) return m;
    return NULL;
}

static macro *macro_find(const char *name) { return macro_find_n(name, strlen(name)); }

static void macro_free_params(macro *m)
{
    for (int i = 0; i < m->n_params; i++) free(m->params[i]);
    free(m->params); m->params = NULL;
    m->n_params = 0; m->is_func = 0; m->is_variadic = 0;
}

static macro *macro_get_or_new(const char *name)
{
    macro *m = macro_find(name);
    if (m) { free(m->body); m->body = NULL; macro_free_params(m); return m; }
    m = xcalloc(1, sizeof(macro));
    m->name = xstrdup(name);
    m->next = macros;
    macros = m;
    return m;
}

static void macro_define(const char *name, const char *body)
{
    macro *m = macro_get_or_new(name);
    m->body = xstrdup(body ? body : "");
    m->is_func = 0;
}

static void macro_define_func(const char *name, char **params, int np, int variadic, const char *body)
{
    macro *m = macro_get_or_new(name);
    m->body = xstrdup(body ? body : "");
    m->is_func = 1;
    m->is_variadic = variadic;
    m->n_params = np;
    m->params = xmalloc((size_t)(np + 1) * sizeof *m->params);
    for (int i = 0; i < np; i++) m->params[i] = xstrdup(params[i]);
}

static void macro_undef(const char *name)
{
    macro **pp = &macros;
    while (*pp) {
        if (strcmp((*pp)->name, name) == 0) {
            macro *dead = *pp;
            *pp = dead->next;
            free(dead->name); free(dead->body); macro_free_params(dead); free(dead);
            return;
        }
        pp = &(*pp)->next;
    }
}

// one level of #if: an open #if, #ifdef or #ifndef and its branches
typedef struct {
    int         taking;                // its lines are being taken
    int         taken;                 // a branch of it was taken, or none may be
    int         had_else;              // its #else was seen
    const char *file;                  // where it was opened, for "no #endif"
    int         line;
    const char *kind;                  // "if", "ifdef" or "ifndef"
} cond_level;
static cond_level *conds;
static int         cond_n = 0, cond_cap = 0;

static int cond_active(void)
{
    for (int i = 0; i < cond_n; i++) if (!conds[i].taking) return 0;
    return 1;
}

// opens a level; inside a group being skipped no branch of it may be taken
static void cond_push(int taking, const char *kind)
{
    int outer = cond_active();
    XGROW(conds, cond_n, cond_cap);
    conds[cond_n++] = (cond_level){ outer && taking, !outer || taking, 0,
                                    pp_at_file, pp_at_line, kind };
}

static int is_ident_start(char c) { return isalpha((unsigned char)c) || c == '_'; }
static int is_ident_cont (char c) { return isalnum((unsigned char)c) || c == '_'; }

// ---- growable string buffer ------------------------------------------------

typedef struct { char *p; size_t len, cap; } sbuf;
static void sb_init(sbuf *b) { b->cap = 64; b->p = xmalloc(b->cap); b->p[0] = 0; b->len = 0; }
static void sb_putc(sbuf *b, char c) {
    if (b->len + 2 >= b->cap) { b->cap *= 2; b->p = xrealloc(b->p, b->cap); }
    b->p[b->len++] = c; b->p[b->len] = 0;
}
static void sb_puts(sbuf *b, const char *s) { while (*s) sb_putc(b, *s++); }
static void sb_putn(sbuf *b, const char *s, size_t n) { for (size_t i = 0; i < n; i++) sb_putc(b, s[i]); }

static char *dupn(const char *s, size_t n) { char *r = xmalloc(n + 1); memcpy(r, s, n); r[n] = 0; return r; }

// the identifier at s, of any length, in a block of its own; *n gets its length
static char *take_ident(const char *s, size_t *n)
{
    size_t k = 0;
    while (is_ident_cont(s[k])) k++;
    *n = k;
    return dupn(s, k);
}

// trim leading/trailing whitespace, returning a fresh string
static char *dup_trim(const char *s, size_t n)
{
    while (n > 0 && (*s == ' ' || *s == '\t')) { s++; n--; }
    while (n > 0 && (s[n-1] == ' ' || s[n-1] == '\t')) n--;
    return dupn(s, n);
}

static char *expand_str(const char *src);

// append the string-literal form of `raw` to out: wrap in quotes, collapse
// internal whitespace runs to one space, escape embedded " and \ (C `#`).
static void sb_stringize(sbuf *out, const char *raw)
{
    if (!raw) raw = "";
    while (*raw == ' ' || *raw == '\t') raw++;       // trim leading
    sb_putc(out, '"');
    int pend = 0;
    for (const char *s = raw; *s; s++) {
        if (*s == ' ' || *s == '\t') { pend = 1; continue; }
        if (pend) { sb_putc(out, ' '); pend = 0; }   // collapse runs, drop trailing
        if (*s == '"' || *s == '\\') sb_putc(out, '\\');
        sb_putc(out, *s);
    }
    sb_putc(out, '"');
}

static void sb_rstrip(sbuf *b) {                      // drop trailing spaces/tabs
    while (b->len > 0 && (b->p[b->len-1] == ' ' || b->p[b->len-1] == '\t')) b->p[--b->len] = 0;
}

#define IS_VA(s, n) name_is((s), (n), "__VA_ARGS__")

// the parameter of m named by the n characters at s, or -1
static int param_index(const macro *m, const char *s, size_t n)
{
    for (int i = 0; i < m->n_params; i++) if (name_is(s, n, m->params[i])) return i;
    return -1;
}

// substitute call args into a function-macro body. Handles # (stringize),
// ## (token paste), and __VA_ARGS__ for variadic macros, plus the common
// GNU `, ##__VA_ARGS__` comma-elision when the variadic part is empty.
// Operands of # and ## use the RAW (unexpanded) argument; elsewhere the
// argument is macro-expanded first.
static char *macro_subst(macro *m, char **args, int na)
{
    char **eargs = xmalloc((size_t)(na + 1) * sizeof *eargs);
    for (int i = 0; i < na; i++) eargs[i] = expand_str(args[i]);

    // build the variadic tail (raw and expanded), joined with ", "
    sbuf vr; sb_init(&vr); sbuf ve; sb_init(&ve);
    if (m->is_variadic) {
        for (int i = m->n_params; i < na; i++) {
            if (i > m->n_params) { sb_puts(&vr, ", "); sb_puts(&ve, ", "); }
            sb_puts(&vr, args[i]); sb_puts(&ve, eargs[i]);
        }
    }
    int va_empty = (vr.p[0] == 0);

    sbuf out; sb_init(&out);
    const char *b = m->body;
    int prev_paste = 0;                              // the next token follows ##
    while (*b) {
        if (*b == '"' || *b == '\'') {
            char q = *b; sb_putc(&out, *b++);
            while (*b && *b != q) { if (*b == '\\' && b[1]) sb_putc(&out, *b++); sb_putc(&out, *b++); }
            if (*b) sb_putc(&out, *b++);
            prev_paste = 0;
            continue;
        }
        // stringize: # <param>  or  # __VA_ARGS__
        if (*b == '#' && b[1] != '#') {
            const char *q = b + 1; while (*q == ' ' || *q == '\t') q++;
            if (is_ident_start(*q)) {
                const char *is = q; while (is_ident_cont(*q)) q++;
                size_t l = q - is;
                int pi = param_index(m, is, l);
                if (pi >= 0)               { sb_stringize(&out, pi < na ? args[pi] : ""); b = q; prev_paste = 0; continue; }
                if (m->is_variadic && IS_VA(is, l)) { sb_stringize(&out, vr.p);              b = q; prev_paste = 0; continue; }
            }
            sb_putc(&out, '#'); b++; continue;       // stray # -> literal
        }
        // token paste
        if (*b == '#' && b[1] == '#') {
            b += 2;
            sb_rstrip(&out);
            while (*b == ' ' || *b == '\t') b++;
            // GNU elision: `, ##__VA_ARGS__` with empty tail drops the comma
            if (m->is_variadic && va_empty && is_ident_start(*b)) {
                const char *is = b, *q = b; while (is_ident_cont(*q)) q++;
                if ((size_t)(q - is) == 11 && strncmp(is, "__VA_ARGS__", 11) == 0) {
                    sb_rstrip(&out);
                    if (out.len > 0 && out.p[out.len-1] == ',') out.p[--out.len] = 0;
                }
            }
            prev_paste = 1;
            continue;
        }
        if (is_ident_start(*b)) {
            const char *s = b; while (is_ident_cont(*b)) b++;
            size_t l = b - s;
            const char *q = b; while (*q == ' ' || *q == '\t') q++;
            int next_paste = (*q == '#' && q[1] == '#');
            int raw = prev_paste || next_paste;      // operands of ## are unexpanded
            int pi = param_index(m, s, l);
            if (pi >= 0)                     sb_puts(&out, pi < na ? (raw ? args[pi] : eargs[pi]) : "");
            else if (m->is_variadic && IS_VA(s, l)) sb_puts(&out, raw ? vr.p : ve.p);
            else                             sb_putn(&out, s, l);
            prev_paste = 0;
            continue;
        }
        char c = *b++;
        sb_putc(&out, c);
        if (c != ' ' && c != '\t') prev_paste = 0;
    }
    for (int i = 0; i < na; i++) free(eargs[i]);
    free(eargs);
    free(vr.p); free(ve.p);
    return out.p;
}

// past the string or char literal that starts at p (its closing quote
// included; a backslash escapes the next character)
static const char *skip_literal(const char *p)
{
    char q = *p++;
    while (*p && *p != q) { if (*p == '\\' && p[1]) p++; p++; }
    return *p ? p + 1 : p;
}

// past the number that starts at p (a digit): its letters (0x1F, 1e5, 10u) are
// not names a macro could replace
static const char *skip_number(const char *p)
{
    while (is_ident_cont(*p) || *p == '.') p++;
    return p;
}

int pp_escape(const char **pp)
{
    const char *p = *pp;
    int v;
    switch (*p) {
        case 'n':  v = '\n'; p++; break;
        case 't':  v = '\t'; p++; break;
        case 'r':  v = '\r'; p++; break;
        case 'a':  v = '\a'; p++; break;
        case 'b':  v = '\b'; p++; break;
        case 'f':  v = '\f'; p++; break;
        case 'v':  v = '\v'; p++; break;
        case '\\': case '\'': case '"': case '?': v = *p++; break;
        case 'x':                                   // \x and any number of hex digits
            if (!isxdigit((unsigned char)p[1])) return PP_ESC_BAD;
            v = 0;
            for (p++; isxdigit((unsigned char)*p); p++) {
                if (v <= 0xFF) v = v * 16 + (isdigit((unsigned char)*p) ? *p - '0' : tolower((unsigned char)*p) - 'a' + 10);
            }
            break;
        default:
            if (*p < '0' || *p > '7') return PP_ESC_BAD;
            v = 0;                                  // \ and one to three octal digits
            for (int k = 0; k < 3 && *p >= '0' && *p <= '7'; k++) v = v * 8 + (*p++ - '0');
    }
    *pp = p;
    return v > 0xFF ? PP_ESC_RANGE : v;
}

// the arguments of a call of m, from the '(' at *pp to its ')', which *pp ends
// past; their number must be the macro's
static char **macro_args(const macro *m, const char **pp, int *nargs)
{
    char **args = NULL; int na = 0, cap = 0, dep = 1;
    const char *q = *pp + 1, *as = q;
    for (;; q++) {
        if (*q == 0) pp_fail(MSG_ERR_MACRO_OPEN, m->name);
        if (*q == '"' || *q == '\'') { q = skip_literal(q) - 1; continue; }   // a comma in
        if (*q == '(') dep++;                                                 // "a,b" is text (#128)
        else if (*q == ')' && --dep == 0) break;
        else if (*q == ',' && dep == 1) {
            XGROW(args, na, cap);
            args[na++] = dup_trim(as, (size_t)(q - as));
            as = q + 1;
        }
    }
    XGROW(args, na, cap);
    args[na++] = dup_trim(as, (size_t)(q - as));
    *pp = q + 1;
    // F() with a macro of no parameter: no argument, not one empty
    if (m->n_params == 0 && na == 1 && args[0][0] == 0) { free(args[0]); na = 0; }
    if (m->is_variadic ? na < m->n_params : na != m->n_params)
        pp_fail(m->is_variadic ? MSG_ERR_MACRO_ARGS_MIN : MSG_ERR_MACRO_ARGS, m->name, m->n_params, na);
    *nargs = na;
    return args;
}

// src with its macros expanded, in a new block. A macro being expanded is
// busy: its name in its own expansion is left as it is, as C does, so
// `#define x x+1` gives x+1 and `#define A A A` gives A A.
static char *expand_str(const char *src)
{
    sbuf out; sb_init(&out);
    const char *p = src;
    while (*p) {
        if (*p == '"' || *p == '\'') {                      // a literal is copied as it is
            const char *e = skip_literal(p);
            sb_putn(&out, p, (size_t)(e - p)); p = e;
            continue;
        }
        if (isdigit((unsigned char)*p)) {                    // so is a number
            const char *e = skip_number(p);
            sb_putn(&out, p, (size_t)(e - p)); p = e;
            continue;
        }
        if (!is_ident_start(*p)) { sb_putc(&out, *p++); continue; }

        const char *s = p; while (is_ident_cont(*p)) p++;
        size_t idlen = (size_t)(p - s);
        macro *m = macro_find_n(s, idlen);
        const char *q = p; while (*q == ' ' || *q == '\t') q++;
        if (!m || m->busy || (m->is_func && *q != '(')) {    // a name, or a function-like
            sb_putn(&out, s, idlen);                          // macro's name with no call
            continue;
        }
        char *body;
        if (m->is_func) {
            int na; char **args = macro_args(m, &q, &na);
            body = macro_subst(m, args, na);
            for (int i = 0; i < na; i++) free(args[i]);
            free(args);
            p = q;
        } else {
            body = xstrdup(m->body ? m->body : "");
        }
        m->busy = 1;
        char *rec = expand_str(body);
        m->busy = 0;
        sb_puts(&out, rec);
        free(rec); free(body);
    }
    return out.p;
}

// ---- #if constant-expression evaluator -------------------------------------
// Evaluates a preprocessor constant expression in 64 bits, as C's intmax_t
// (`long` has 32 bits on Windows, which made 0xFFFFFFFF equal 0x7FFFFFFF).
// `defined X` / `defined(X)` are resolved first, then macros are expanded,
// then a C integer-expression grammar is parsed. A name left over after the
// expansion is 0, except `true`, which C++ makes 1. Numbers are read as the
// lexer reads them (010 is 8). Every value is signed: C would make -1 > 0u
// true, and here it is false. Anything the grammar does not take is an error.

typedef long long ppv;
#define PPWRAP(a, op, b) ((ppv)((unsigned long long)(a) op (unsigned long long)(b)))   // no signed overflow

static MSG_NORET void pp_bad(const char *at)
{
    if (*at) pp_fail(MSG_ERR_IF_EXPR, at);
    pp_fail(MSG_ERR_IF_END);
}

static char *resolve_defined(const char *line)
{
    sbuf out; sb_init(&out);
    const char *p = line;
    while (*p) {
        if (*p == '"' || *p == '\'') {
            const char *e = skip_literal(p);
            sb_putn(&out, p, (size_t)(e - p)); p = e;
            continue;
        }
        if (!is_ident_start(*p)) { sb_putc(&out, *p++); continue; }
        const char *s = p; while (is_ident_cont(*p)) p++;
        if (!name_is(s, (size_t)(p - s), "defined")) { sb_putn(&out, s, (size_t)(p - s)); continue; }
        while (*p == ' ' || *p == '\t') p++;
        int paren = *p == '(';
        if (paren) { p++; while (*p == ' ' || *p == '\t') p++; }
        const char *nm = p; while (is_ident_cont(*p)) p++;
        if (p == nm || !is_ident_start(*nm)) pp_bad(nm);
        macro *dm = macro_find_n(nm, (size_t)(p - nm));
        if (paren) {
            while (*p == ' ' || *p == '\t') p++;
            if (*p != ')') pp_bad(p);
            p++;
        }
        sb_putc(&out, dm ? '1' : '0');
    }
    return out.p;
}

static ppv  pp_expr(const char **pp);
static void pp_ws(const char **pp) { while (**pp == ' ' || **pp == '\t') (*pp)++; }

// an integer literal: 0x hex, 0 octal, else decimal, and a suffix u, l or ll
static ppv pp_number(const char **pp)
{
    const char *s = *pp, *p = s;
    int base = p[0] != '0' ? 10 : (p[1] == 'x' || p[1] == 'X') ? 16 : 8;
    if (base == 16) { p += 2; if (!isxdigit((unsigned char)*p)) pp_bad(s); }
    unsigned long long v = 0;
    for (;; p++) {
        int d = isdigit((unsigned char)*p) ? *p - '0'
              : base == 16 && isxdigit((unsigned char)*p) ? tolower((unsigned char)*p) - 'a' + 10 : -1;
        if (d < 0) break;
        if (d >= base) pp_fail(MSG_ERR_OCTAL, dupn(s, (size_t)(skip_number(s) - s)));
        if (v > (~0ULL - (unsigned)d) / (unsigned)base) pp_bad(s);   // past 64 bits
        v = v * (unsigned)base + (unsigned)d;
    }
    if (*p == 'u' || *p == 'U') p++;
    if ((*p == 'l' && p[1] == 'l') || (*p == 'L' && p[1] == 'L')) p += 2;
    else if (*p == 'l' || *p == 'L') p++;
    if (p[-1] != 'u' && p[-1] != 'U' && (*p == 'u' || *p == 'U')) p++;
    if (is_ident_cont(*p) || *p == '.') pp_bad(s);           // 1.5, 1uuu: not an integer
    *pp = p;
    return (ppv)v;
}

static ppv pp_primary(const char **pp)
{
    pp_ws(pp);
    const char *s = *pp;
    if (*s == '(') {
        (*pp)++;
        ppv v = pp_expr(pp);
        pp_ws(pp);
        if (**pp != ')') pp_bad(*pp);
        (*pp)++;
        return v;
    }
    if (*s == '\'') {                     // a char constant, with the lexer's escapes
        const char *p = s + 1;
        int v;
        if (*p == '\\') { p++; v = pp_escape(&p); if (v < 0) pp_bad(s); }
        else if (*p && *p != '\'') v = (unsigned char)*p++;
        else pp_bad(s);
        if (*p != '\'') pp_bad(s);
        *pp = p + 1;
        return v;
    }
    if (isdigit((unsigned char)*s)) return pp_number(pp);
    if (is_ident_start(*s)) {
        while (is_ident_cont(**pp)) (*pp)++;
        return name_is(s, (size_t)(*pp - s), "true");
    }
    pp_bad(s);
}

static ppv pp_unary(const char **pp)
{
    pp_ws(pp);
    if (**pp == '!') { (*pp)++; return !pp_unary(pp); }
    if (**pp == '~') { (*pp)++; return ~pp_unary(pp); }
    if (**pp == '-') { (*pp)++; return PPWRAP(0, -, pp_unary(pp)); }
    if (**pp == '+') { (*pp)++; return pp_unary(pp); }
    return pp_primary(pp);
}

static ppv pp_mul(const char **pp)
{
    ppv v = pp_unary(pp);
    for (;;) {
        pp_ws(pp);
        char c = **pp;
        if (c != '*' && c != '/' && c != '%') return v;
        (*pp)++;
        ppv d = pp_unary(pp);
        if (c == '*') { v = PPWRAP(v, *, d); continue; }
        if (d == 0) pp_fail(MSG_ERR_IF_DIV0);
        if (d == -1) v = c == '/' ? PPWRAP(0, -, v) : 0;     // LLONG_MIN / -1 overflows
        else         v = c == '/' ? v / d : v % d;
    }
}

static ppv pp_add(const char **pp)
{
    ppv v = pp_mul(pp);
    for (;;) {
        pp_ws(pp);
        if      (**pp == '+') { (*pp)++; v = PPWRAP(v, +, pp_mul(pp)); }
        else if (**pp == '-') { (*pp)++; v = PPWRAP(v, -, pp_mul(pp)); }
        else return v;
    }
}

// a shift by less than 0 or more than 63 gives what shifting one bit at a
// time would: 0, or the sign for >>
static ppv pp_shift(const char **pp)
{
    ppv v = pp_add(pp);
    for (;;) {
        pp_ws(pp);
        int left = **pp == '<' && (*pp)[1] == '<', right = **pp == '>' && (*pp)[1] == '>';
        if (!left && !right) return v;
        *pp += 2;
        ppv n = pp_add(pp);
        if (n < 0 || n > 63) v = left ? 0 : v < 0 ? -1 : 0;
        else                 v = left ? PPWRAP(v, <<, n) : v >> n;
    }
}

static ppv pp_rel(const char **pp)
{
    ppv v = pp_shift(pp);
    for (;;) {
        pp_ws(pp);
        if      (**pp == '<' && (*pp)[1] == '=') { *pp += 2; v = v <= pp_shift(pp); }
        else if (**pp == '>' && (*pp)[1] == '=') { *pp += 2; v = v >= pp_shift(pp); }
        else if (**pp == '<' && (*pp)[1] != '<') { (*pp)++; v = v <  pp_shift(pp); }
        else if (**pp == '>' && (*pp)[1] != '>') { (*pp)++; v = v >  pp_shift(pp); }
        else return v;
    }
}

static ppv pp_eq(const char **pp)
{
    ppv v = pp_rel(pp);
    for (;;) {
        pp_ws(pp);
        if      (**pp == '=' && (*pp)[1] == '=') { *pp += 2; v = v == pp_rel(pp); }
        else if (**pp == '!' && (*pp)[1] == '=') { *pp += 2; v = v != pp_rel(pp); }
        else return v;
    }
}

static ppv pp_band(const char **pp)
{
    ppv v = pp_eq(pp);
    for (;;) { pp_ws(pp); if (**pp != '&' || (*pp)[1] == '&') return v; (*pp)++; v &= pp_eq(pp); }
}

static ppv pp_bxor(const char **pp)
{
    ppv v = pp_band(pp);
    for (;;) { pp_ws(pp); if (**pp != '^') return v; (*pp)++; v ^= pp_band(pp); }
}

static ppv pp_bor(const char **pp)
{
    ppv v = pp_bxor(pp);
    for (;;) { pp_ws(pp); if (**pp != '|' || (*pp)[1] == '|') return v; (*pp)++; v |= pp_bxor(pp); }
}

static ppv pp_land(const char **pp)
{
    ppv v = pp_bor(pp);
    for (;;) { pp_ws(pp); if (**pp != '&' || (*pp)[1] != '&') return v; *pp += 2; ppv r = pp_bor(pp); v = v && r; }
}

static ppv pp_lor(const char **pp)
{
    ppv v = pp_land(pp);
    for (;;) { pp_ws(pp); if (**pp != '|' || (*pp)[1] != '|') return v; *pp += 2; ppv r = pp_land(pp); v = v || r; }
}

static ppv pp_expr(const char **pp)
{
    ppv c = pp_lor(pp);
    pp_ws(pp);
    if (**pp != '?') return c;
    (*pp)++;
    ppv a = pp_expr(pp);
    pp_ws(pp);
    if (**pp != ':') pp_bad(*pp);
    (*pp)++;
    ppv b = pp_expr(pp);
    return c ? a : b;
}

// evaluate a full `#if` / `#elif` expression line
static int eval_if(const char *text)
{
    char *d = resolve_defined(text);
    char *x = expand_str(d);
    const char *cur = x;
    ppv v = pp_expr(&cur);
    pp_ws(&cur);
    if (*cur) pp_bad(cur);
    free(d); free(x);
    return v != 0;
}

// the output: the text, and for each of its lines the file and line it came
// from (pp_where) and that line as written (pp_source)
static sbuf         out_text;
static const char **map_file = NULL;
static int         *map_line = NULL;
static char       **map_src  = NULL;
static int          map_n = 0, map_cap = 0;
static int          at_bol = 1;          // the next character starts a line

static void pp_emit(const char *s, const char *file, int line, const char *src)
{
    for (; *s; s++) {
        if (at_bol) {
            if (map_n == map_cap) {
                map_cap  = map_cap ? 2 * map_cap : 1024;
                map_file = xrealloc(map_file, map_cap * sizeof *map_file);
                map_line = xrealloc(map_line, map_cap * sizeof *map_line);
                map_src  = xrealloc(map_src , map_cap * sizeof *map_src );
            }
            map_file[map_n] = file; map_line[map_n] = line;
            map_src [map_n] = xstrdup(src);
            map_n++;
            at_bol = 0;
        }
        sb_putc(&out_text, *s);
        if (*s == '\n') at_bol = 1;
    }
}

static void process_file(const char *path);

// the path an #include of `name` opens: in ref_dir first (the "..." form),
// then in each -I folder; NULL if none has it
static char *find_include(const char *name, const char *ref_dir)
{
    for (int i = ref_dir ? -1 : 0; i < n_incdirs; i++) {
        char *path = xasprintf("%s/%s", i < 0 ? ref_dir : incdirs[i], name);
        FILE *f = fopen(path, "r");
        if (f) { fclose(f); return path; }
        free(path);
    }
    return NULL;
}

// the folder of path ("." for a bare file name)
static char *dirname_of(const char *path)
{
    const char *last = path;
    for (const char *p = path; *p; p++) if (*p == '/' || *p == '\\') last = p;
    return last == path ? xstrdup(".") : dupn(path, (size_t)(last - path));
}

// an #include of target: the file, unless a #pragma once closed it
static void include_file(const char *target, const char *ref_dir)
{
    char *resolved = find_include(target, ref_dir);
    if (!resolved) {
        pp_fail(ref_dir ? MSG_ERR_INCLUDE_QUOTE : MSG_ERR_INCLUDE_ANGLE, target);
    }
    char *canon = path_canon(resolved);
    if (!once_seen(canon)) process_file(resolved);
    free(canon); free(resolved);
}

// ---- reading a file ----------------------------------------------------------
// A file is read line by line, each line with its comments replaced by a
// space, as C does before it reads a directive or a token: a /* ... */ may
// span lines (an #include inside one is not read), and the lexer sees no
// comment.

typedef struct {
    FILE       *fp;
    const char *file;                  // its path, kept by the line map
    int         ln;                    // the line just read
    int         in_comment;            // inside a /* ... */ that began on comment_line
    int         comment_line;
    char       *orig;                  // the line as written, with its '\n'
    char       *text;                  // the line without comments, with its '\n'
} reader;

// the line s, without its '\n', with each comment replaced by one space
static char *strip_comments(const char *s, reader *r)
{
    sbuf b; sb_init(&b);
    while (*s) {
        if (r->in_comment) {
            const char *e = strstr(s, "*/");
            if (!e) break;                                 // the comment goes on
            s = e + 2; r->in_comment = 0;
            continue;
        }
        if (*s == '"' || *s == '\'') {
            const char *e = skip_literal(s);
            sb_putn(&b, s, (size_t)(e - s)); s = e;
            continue;
        }
        if (s[0] == '/' && s[1] == '/') break;
        if (s[0] == '/' && s[1] == '*') {
            sb_putc(&b, ' ');
            r->in_comment = 1; r->comment_line = r->ln;
            s += 2;
            continue;
        }
        sb_putc(&b, *s++);
    }
    sb_rstrip(&b);
    sb_putc(&b, '\n');
    return b.p;
}

// reads the next line into r->orig and r->text; 0 at the end of the file
static int reader_next(reader *r)
{
    free(r->orig); free(r->text);
    r->orig = r->text = NULL;
    char *raw = xgetline(r->fp);               // a line of any length, ended with
    if (!raw) return 0;                        // '\n' even at the end of a file that has none
    r->ln++;
    pp_at_line = r->ln;
    const char *s = raw;
    if (r->ln == 1 && strncmp(s, "\xEF\xBB\xBF", 3) == 0) s += 3;   // the UTF-8 mark Windows editors write
    r->orig = xasprintf("%s\n", s);
    r->text = strip_comments(s, r);
    free(raw);
    return 1;
}

// a call of a function-like macro that the text opens and does not close:
// its arguments go on in the next line
static int call_open(const char *p)
{
    while (*p) {
        if (*p == '"' || *p == '\'') { p = skip_literal(p); continue; }
        if (isdigit((unsigned char)*p)) { p = skip_number(p); continue; }
        if (!is_ident_start(*p)) { p++; continue; }
        const char *s = p; while (is_ident_cont(*p)) p++;
        macro *m = macro_find_n(s, (size_t)(p - s));
        const char *q = p; while (*q == ' ' || *q == '\t') q++;
        if (!m || !m->is_func || *q != '(') continue;
        int dep = 0;
        for (; *q; q++) {
            if (*q == '"' || *q == '\'') { q = skip_literal(q) - 1; continue; }
            if (*q == '(') dep++;
            else if (*q == ')' && --dep == 0) break;
        }
        if (!*q) return 1;
        p = q + 1;
    }
    return 0;
}

static char *trim_in_place(char *s)
{
    while (*s == ' ' || *s == '\t') s++;
    int L = (int)strlen(s);
    while (L > 0 && (s[L-1] == '\n' || s[L-1] == '\r' || s[L-1] == ' ' || s[L-1] == '\t')) s[--L] = 0;
    return s;
}

// ---- directives --------------------------------------------------------------

// the level of #if an #elif, #else or #endif of this file closes; stray is
// the message for one with no #if open in the file
static cond_level *cond_top(const char *stray, int cond_base)
{
    if (cond_n <= cond_base) pp_fail("%s", stray);
    return &conds[cond_n - 1];
}

// #include "name", <name>, or a macro that gives one of them
static void do_include(const char *p, const char *dir)
{
    char *x = NULL;
    if (*p != '"' && *p != '<') { x = expand_str(p); p = trim_in_place(x); }
    if (*p != '"' && *p != '<') pp_fail(MSG_ERR_INCLUDE_FORM);
    char close = *p == '"' ? '"' : '>';
    const char *t = ++p; while (*p && *p != close) p++;
    if (*p != close || *trim_in_place((char *)p + 1)) pp_fail(MSG_ERR_INCLUDE_FORM);
    char *target = dupn(t, (size_t)(p - t));
    include_file(target, close == '"' ? dir : NULL);   // "..." looks in this file's folder first
    free(target); free(x);
}

// #define NAME body, or NAME(params) body with the '(' glued to the name
static void do_define(const char *p)
{
    size_t n; char *name = take_ident(p, &n);
    if (n == 0 || !is_ident_start(*name)) pp_fail(MSG_ERR_DIRECTIVE_NAME, "define");
    p += n;
    if (*p != '(') {
        while (*p == ' ' || *p == '\t') p++;
        macro_define(name, p);
        free(name);
        return;
    }
    p++;
    char **params = NULL; int np = 0, params_cap = 0, variadic = 0;
    while (*p && *p != ')') {
        while (*p == ' ' || *p == '\t') p++;
        if (*p == '.' && p[1] == '.' && p[2] == '.') {  // C99 `...`
            p += 3; variadic = 1;
            while (*p == ' ' || *p == '\t') p++;
            break;
        }
        size_t pl; char *pn = take_ident(p, &pl); p += pl;
        if (pl > 0) { XGROW(params, np, params_cap); params[np++] = pn; }
        else free(pn);
        while (*p == ' ' || *p == '\t') p++;
        if (*p == ',') p++;
        else if (*p != ')') break;
    }
    if (*p != ')') pp_fail(MSG_ERR_MACRO_PARAMS, name);
    p++;
    while (*p == ' ' || *p == '\t') p++;
    macro_define_func(name, params, np, variadic, p);
    for (int k = 0; k < np; k++) free(params[k]);
    free(params);
    free(name);
}

// one directive: p is the line past its '#', without comments; cond_base is
// the number of #if levels open when the file began
static void directive(char *p, const char *path, const char *dir, int cond_base)
{
    p = trim_in_place(p);
    size_t n; char *name = take_ident(p, &n);
    p += n;
    while (*p == ' ' || *p == '\t') p++;

    // the conditionals are read even in a group being skipped, to find its end
    if (!strcmp(name, "if") || !strcmp(name, "ifdef") || !strcmp(name, "ifndef")) {
        int taking = 0;
        if (cond_active()) {                       // a skipped group's #if is not evaluated
            if (name[2] == 0) taking = eval_if(p);
            else {
                size_t l; char *nm = take_ident(p, &l);
                if (l == 0 || !is_ident_start(*nm)) pp_fail(MSG_ERR_DIRECTIVE_NAME, name);
                taking = (macro_find(nm) != NULL) == (name[2] == 'd');
                free(nm);
            }
        }
        cond_push(taking, name[2] == 0 ? "if" : name[2] == 'd' ? "ifdef" : "ifndef");
    } else if (!strcmp(name, "elif")) {
        cond_level *c = cond_top(MSG_ERR_STRAY_ELIF, cond_base);
        if (c->had_else) pp_fail(MSG_ERR_AFTER_ELSE, name);
        if (c->taken) c->taking = 0;               // an earlier branch already won
        else { c->taking = eval_if(p); c->taken = c->taking; }
    } else if (!strcmp(name, "else")) {
        cond_level *c = cond_top(MSG_ERR_STRAY_ELSE, cond_base);
        if (c->had_else) pp_fail(MSG_ERR_AFTER_ELSE, name);
        c->taking = !c->taken;
        c->taken = c->had_else = 1;
    } else if (!strcmp(name, "endif")) {
        cond_top(MSG_ERR_STRAY_ENDIF, cond_base);
        cond_n--;
    } else if (!cond_active()) {
        ;                                          // nothing else is read in a skipped group
    } else if (!strcmp(name, "include")) {
        do_include(p, dir);
    } else if (!strcmp(name, "define")) {
        do_define(p);
    } else if (!strcmp(name, "undef")) {
        size_t l; char *nm = take_ident(p, &l);
        if (l == 0 || !is_ident_start(*nm)) pp_fail(MSG_ERR_DIRECTIVE_NAME, name);
        macro_undef(nm);
        free(nm);
    } else if (!strcmp(name, "pragma")) {
        if (!strncmp(p, "once", 4) && (p[4] == 0 || isspace((unsigned char)p[4]))) {
            char *canon = path_canon(path);
            once_mark(canon);
            free(canon);
        } else {                                   // #pragma yanc and the others: the lexer's
            char *t = xasprintf("#pragma %s\n", p);
            pp_emit(t, pp_at_file, pp_at_line, t);
            free(t);
        }
    } else if (!strcmp(name, "error") || !strcmp(name, "warning")) {
        int err = name[0] == 'e';
        fprintf(stderr, "%s:%d: %s: #%s %s\n", pp_at_file, pp_at_line,
                err ? MSG_WORD_ERROR : MSG_WORD_WARNING, name, p);
        if (err) exit(1);
    } else if (n > 0 || *p) {                      // a lone '#' is the null directive
        pp_fail(MSG_ERR_DIRECTIVE_UNKNOWN, n > 0 ? name : p);
    }
    free(name);
}

static int depth = 0;

static void process_file(const char *path)
{
    if (depth > 32) pp_fail(MSG_ERR_INCLUDE_DEEP);
    depth++;

    reader r = { 0 };
    r.fp = fopen(path, "r");
    if (!r.fp) pp_fail(MSG_ERR_CANT_OPEN, path);
    r.file = xstrdup(path);                   // the map keeps it
    char *dir = dirname_of(path);
    const char *sv_file = pp_at_file;          // the includer's place, back at the end
    int         sv_line = pp_at_line;
    int         cond_base = cond_n;            // an #if is closed in the file that opens it
    pp_at_file = r.file;

    while (reader_next(&r)) {
        char *t = r.text;
        while (*t == ' ' || *t == '\t') t++;
        if (*t == '#') { directive(t + 1, path, dir, cond_base); continue; }
        if (!cond_active()) continue;

        int   ln   = r.ln;
        char *src  = xstrdup(r.orig);
        char *code = xstrdup(r.text);
        while (call_open(code) && reader_next(&r)) {   // the call goes on in the next line
            code[strlen(code) - 1] = ' ';
            char *j = xasprintf("%s%s", code, r.text);
            free(code); code = j;
        }
        pp_at_line = ln;
        char *expanded = expand_str(code);
        pp_emit(expanded, r.file, ln, src);
        free(expanded); free(code); free(src);
    }
    if (r.in_comment) { pp_at_line = r.comment_line; pp_fail(MSG_ERR_COMMENT_OPEN); }
    if (cond_n > cond_base) {
        cond_level *c = &conds[cond_n - 1];
        pp_at_file = c->file; pp_at_line = c->line;
        pp_fail(MSG_ERR_IF_OPEN, c->kind);
    }
    free(dir);
    fclose(r.fp);
    pp_at_file = sv_file; pp_at_line = sv_line;
    depth--;
}

void pp_add_incdir(const char *dir)
{
    XGROW(incdirs, n_incdirs, incdirs_cap);
    incdirs[n_incdirs++] = dir;
}

const char *pp_run(const char *in_path)
{
    // after the -I dirs, the Header/ folder next to the tool (exe_dir.h)
    static const char *const header_dirs[] = EXE_HEADER_DIRS;
    char *hdr = exe_find_dir(header_dirs);
    if (hdr) pp_add_incdir(hdr);

    sb_init(&out_text);
    process_file(in_path);
    return out_text.p;
}

int pp_where(int line, const char **file)
{
    if (line < 1 || line > map_n) return line;    // *file left as it was
    *file = map_file[line - 1];
    return map_line[line - 1];
}

const char *pp_source(int line)
{
    return (line < 1 || line > map_n) ? NULL : map_src[line - 1];
}
