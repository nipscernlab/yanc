// ----------------------------------------------------------------------------
// CPPComp — the C preprocessor --------------------------------------------------
// ----------------------------------------------------------------------------
// cppcomp's first step, as cpp is gcc's: pp_run() turns the source into one
// text, kept in memory, with a map from each of its lines back to the file and
// line it came from (the messages point there). `cppcomp -E` prints the text.
// Handles:
//   #include "name"          (relative to current file + -I paths)
//   #include <name>          (-I paths, then the Header/ folder next to the tool)
//   #define NAME body        (object- and function-like macros, incl.
//                             variadic `...`/__VA_ARGS__, # stringize, ## paste)
//   #undef NAME
//   #ifdef NAME / #ifndef NAME / #else / #endif
//   #pragma once             (skip the file on any later #include)
//   #pragma yanc ...         (passed through verbatim)
// ----------------------------------------------------------------------------

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "../Headers/cpppp.h"
#include "../Headers/messages.h"
#include "../../common/xstr.h"
#include "../../common/exe_dir.h"

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

static int *cond_stk;                  // 1 where this #if level is taking its lines
static int *cond_seen_true;            // 1 where a branch of this level was taken
static int  cond_n = 0, cond_cap = 0;

static void cond_push(int active)
{
    if (cond_n >= cond_cap) {          // cond_stk and cond_seen_true grow together
        int cap = cond_cap;
        XGROW(cond_stk, cond_n, cap);
        XGROW(cond_seen_true, cond_n, cond_cap);
    }
    cond_stk[cond_n] = active;
    cond_seen_true[cond_n] = active;
    cond_n++;
}

static int cond_active(void)
{
    for (int i = 0; i < cond_n; i++) if (!cond_stk[i]) return 0;
    return 1;
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

static char *expand_str(const char *src, int depth);

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
    for (int i = 0; i < na; i++) eargs[i] = expand_str(args[i], 1);

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

// fully expand object- and function-like macros in src; returns malloc'd result
// past the string or char literal that starts at p (its closing quote
// included; a backslash escapes the next character)
static const char *skip_literal(const char *p)
{
    char q = *p++;
    while (*p && *p != q) { if (*p == '\\' && p[1]) p++; p++; }
    return *p ? p + 1 : p;
}

// past the comment that starts at p: // to the end of the line, /* to its */
static const char *skip_comment(const char *p)
{
    if (p[1] == '/') return p + strlen(p);
    const char *e = strstr(p + 2, "*/");
    return e ? e + 2 : p + strlen(p);
}

static char *expand_str(const char *src, int depth)
{
    sbuf out; sb_init(&out);
    if (depth > 64) { sb_puts(&out, src); return out.p; }   // recursion guard

    const char *p = src;
    while (*p) {
        if (*p == '"' || *p == '\'') {                      // a literal is copied as it is
            const char *e = skip_literal(p);
            sb_putn(&out, p, (size_t)(e - p)); p = e;
            continue;
        }
        if (*p == '/' && (p[1] == '/' || p[1] == '*')) {     // so is a comment: an apostrophe
            const char *e = skip_comment(p);               // in one is not a char literal (#128)
            sb_putn(&out, p, (size_t)(e - p)); p = e;
            continue;
        }
        if (is_ident_start(*p)) {
            const char *s = p; while (is_ident_cont(*p)) p++;
            size_t idlen = p - s;
            macro *m = macro_find_n(s, idlen);
            if (m && m->is_func) {
                const char *q = p; while (*q == ' ' || *q == '\t') q++;
                if (*q == '(') {
                    q++;                               // consume '('
                    char **args = NULL; int na = 0, args_cap = 0;
                    int dep = 1; const char *as = q;
                    while (*q && dep > 0) {
                        if (*q == '"' || *q == '\'') { q = skip_literal(q); continue; }   // a comma in
                        if (*q == '(') dep++;                                           // "a,b" is text (#128)
                        else if (*q == ')') { dep--; if (dep == 0) break; }
                        else if (*q == ',' && dep == 1) {
                            XGROW(args, na, args_cap);
                            args[na++] = dup_trim(as, q - as);
                            as = q + 1;
                        }
                        q++;
                    }
                    XGROW(args, na, args_cap);
                    args[na++] = dup_trim(as, q - as);
                    if (*q == ')') q++;
                    // FOO() with a 0-param macro: drop the single empty arg
                    if (m->n_params == 0 && na == 1 && args[0][0] == 0) { free(args[0]); na = 0; }
                    char *subst = macro_subst(m, args, na);
                    char *rec   = expand_str(subst, depth + 1);
                    sb_puts(&out, rec);
                    free(rec); free(subst);
                    for (int i = 0; i < na; i++) free(args[i]);
                    free(args);
                    p = q;
                    continue;
                }
                sb_putn(&out, s, idlen);               // func macro name without '(' — literal
                continue;
            }
            if (m) {                                    // object-like
                char *rec = expand_str(m->body ? m->body : "", depth + 1);
                sb_puts(&out, rec); free(rec);
                continue;
            }
            sb_putn(&out, s, idlen);
            continue;
        }
        sb_putc(&out, *p++);
    }
    return out.p;
}

// ---- #if constant-expression evaluator -------------------------------------
// Evaluates a preprocessor constant expression to a long. `defined X` /
// `defined(X)` are resolved first, then macros are expanded, then a standard
// C integer-expression grammar is parsed. Identifiers left over after expansion
// evaluate to 0 (per the C standard).

static char *resolve_defined(const char *line)
{
    sbuf out; sb_init(&out);
    const char *p = line;
    while (*p) {
        if (is_ident_start(*p)) {
            const char *s = p; while (is_ident_cont(*p)) p++;
            if ((p - s) == 7 && strncmp(s, "defined", 7) == 0) {
                while (*p == ' ' || *p == '\t') p++;
                int paren = 0;
                if (*p == '(') { paren = 1; p++; while (*p == ' ' || *p == '\t') p++; }
                const char *nm = p; while (is_ident_cont(*p)) p++;
                macro *dm = macro_find_n(nm, (size_t)(p - nm));
                if (paren) { while (*p == ' ' || *p == '\t') p++; if (*p == ')') p++; }
                sb_putc(&out, dm ? '1' : '0');
            } else {
                sb_putn(&out, s, p - s);
            }
            continue;
        }
        sb_putc(&out, *p++);
    }
    return out.p;
}

static long pp_expr(const char **pp);
static void pp_ws(const char **pp) { while (**pp == ' ' || **pp == '\t') (*pp)++; }

static long pp_primary(const char **pp)
{
    pp_ws(pp);
    char c = **pp;
    if (c == '(') { (*pp)++; long v = pp_expr(pp); pp_ws(pp); if (**pp == ')') (*pp)++; return v; }
    if (c == '\'') {                      // char constant
        (*pp)++;
        long v;
        if (**pp == '\\') { (*pp)++; char e = **pp; (*pp)++;
            v = (e=='n')?'\n':(e=='t')?'\t':(e=='r')?'\r':(e=='0')?0:e; }
        else { v = (unsigned char)**pp; (*pp)++; }
        if (**pp == '\'') (*pp)++;
        return v;
    }
    if (isdigit((unsigned char)c)) {
        char *end; long v;
        if (c == '0' && ((*pp)[1] == 'x' || (*pp)[1] == 'X')) v = strtol(*pp, &end, 16);
        else                                                 v = strtol(*pp, &end, 10);
        *pp = end;
        while (**pp=='u'||**pp=='U'||**pp=='l'||**pp=='L') (*pp)++;   // ignore suffixes
        return v;
    }
    if (is_ident_start(c)) { while (is_ident_cont(**pp)) (*pp)++; return 0; }  // undefined -> 0
    return 0;
}

static long pp_unary(const char **pp)
{
    pp_ws(pp);
    if (**pp == '!') { (*pp)++; return !pp_unary(pp); }
    if (**pp == '~') { (*pp)++; return ~pp_unary(pp); }
    if (**pp == '-') { (*pp)++; return -pp_unary(pp); }
    if (**pp == '+') { (*pp)++; return  pp_unary(pp); }
    return pp_primary(pp);
}
static long pp_mul(const char **pp){ long v=pp_unary(pp); for(;;){ pp_ws(pp); char c=**pp;
    if(c=='*'){(*pp)++; v*=pp_unary(pp);} else if(c=='/'){(*pp)++; long d=pp_unary(pp); v = d?v/d:0;}
    else if(c=='%'){(*pp)++; long d=pp_unary(pp); v = d?v%d:0;} else break;} return v; }
static long pp_add(const char **pp){ long v=pp_mul(pp); for(;;){ pp_ws(pp); char c=**pp;
    if(c=='+'){(*pp)++; v+=pp_mul(pp);} else if(c=='-'){(*pp)++; v-=pp_mul(pp);} else break;} return v; }
static long pp_shift(const char **pp){ long v=pp_add(pp); for(;;){ pp_ws(pp);
    if(**pp=='<'&&(*pp)[1]=='<'){(*pp)+=2; v<<=pp_add(pp);} else if(**pp=='>'&&(*pp)[1]=='>'){(*pp)+=2; v>>=pp_add(pp);} else break;} return v; }
static long pp_rel(const char **pp){ long v=pp_shift(pp); for(;;){ pp_ws(pp);
    if(**pp=='<'&&(*pp)[1]=='='){(*pp)+=2; v=(v<=pp_shift(pp));}
    else if(**pp=='>'&&(*pp)[1]=='='){(*pp)+=2; v=(v>=pp_shift(pp));}
    else if(**pp=='<'){(*pp)++; v=(v<pp_shift(pp));}
    else if(**pp=='>'){(*pp)++; v=(v>pp_shift(pp));} else break;} return v; }
static long pp_eq(const char **pp){ long v=pp_rel(pp); for(;;){ pp_ws(pp);
    if(**pp=='='&&(*pp)[1]=='='){(*pp)+=2; v=(v==pp_rel(pp));}
    else if(**pp=='!'&&(*pp)[1]=='='){(*pp)+=2; v=(v!=pp_rel(pp));} else break;} return v; }
static long pp_band(const char **pp){ long v=pp_eq(pp); for(;;){ pp_ws(pp);
    if(**pp=='&'&&(*pp)[1]!='&'){(*pp)++; v&=pp_eq(pp);} else break;} return v; }
static long pp_bxor(const char **pp){ long v=pp_band(pp); for(;;){ pp_ws(pp);
    if(**pp=='^'){(*pp)++; v^=pp_band(pp);} else break;} return v; }
static long pp_bor(const char **pp){ long v=pp_bxor(pp); for(;;){ pp_ws(pp);
    if(**pp=='|'&&(*pp)[1]!='|'){(*pp)++; v|=pp_bxor(pp);} else break;} return v; }
static long pp_land(const char **pp){ long v=pp_bor(pp); for(;;){ pp_ws(pp);
    if(**pp=='&'&&(*pp)[1]=='&'){(*pp)+=2; long r=pp_bor(pp); v=(v&&r);} else break;} return v; }
static long pp_lor(const char **pp){ long v=pp_land(pp); for(;;){ pp_ws(pp);
    if(**pp=='|'&&(*pp)[1]=='|'){(*pp)+=2; long r=pp_land(pp); v=(v||r);} else break;} return v; }
static long pp_expr(const char **pp){ long c=pp_lor(pp); pp_ws(pp);
    if(**pp=='?'){(*pp)++; long a=pp_expr(pp); pp_ws(pp); if(**pp==':')(*pp)++; long b=pp_expr(pp); return c?a:b;}
    return c; }

// evaluate a full `#if` / `#elif` expression line
static int eval_if(const char *text)
{
    char *d = resolve_defined(text);
    char *x = expand_str(d, 0);
    const char *cur = x;
    long v = pp_expr(&cur);
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

static int depth = 0;

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
        fprintf(stderr, ref_dir ? MSG_ERR_INCLUDE_QUOTE : MSG_ERR_INCLUDE_ANGLE, target);
        exit(1);
    }
    char *canon = path_canon(resolved);
    if (!once_seen(canon)) process_file(resolved);
    free(canon); free(resolved);
}

// drops the comments of a directive line, `// ...` and `/* ... */`, outside
// string and character literals, and the blanks they leave at the end (C
// removes comments before it reads a directive: `#define K 5 // five` is 5)
static void strip_comments(char *s)
{
    char q = 0;
    for (char *c = s; *c; c++) {
        if (q) { if (*c == '\\' && c[1]) c++; else if (*c == q) q = 0; continue; }
        if (*c == '"' || *c == '\'') { q = *c; continue; }
        if (c[0] == '/' && c[1] == '/') { *c = 0; break; }
        if (c[0] == '/' && c[1] == '*') {
            char *e = strstr(c + 2, "*/");
            if (!e) { *c = 0; break; }
            memmove(c, " ", 1); memmove(c + 1, e + 2, strlen(e + 2) + 1);
        }
    }
    int L = (int)strlen(s);
    while (L > 0 && (s[L-1] == ' ' || s[L-1] == '\t')) s[--L] = 0;
}

static char *trim_in_place(char *s)
{
    while (*s == ' ' || *s == '\t') s++;
    int L = (int)strlen(s);
    while (L > 0 && (s[L-1] == '\n' || s[L-1] == '\r' || s[L-1] == ' ' || s[L-1] == '\t')) s[--L] = 0;
    return s;
}

static void process_file(const char *path)
{
    if (depth > 32) { fprintf(stderr, MSG_ERR_INCLUDE_DEEP); exit(1); }
    depth++;

    char *dir = dirname_of(path);

    FILE *fp = fopen(path, "r");
    if (!fp) { fprintf(stderr, MSG_ERR_CANT_OPEN, path); exit(1); }
    const char *file = xstrdup(path);     // the map keeps it
    int ln = 0;

    char *raw, *line = NULL;
    while ((raw = xgetline(fp)) != NULL) {          // a line of any length, ended
        free(line);                                 // with '\n' even at the end of a
        line = xasprintf("%s\n", raw); free(raw);   // file that has none
        ln++;
        char *orig = xstrdup(line);
        char *t = trim_in_place(orig);

        if (t[0] == '#') {
            char *p = t + 1;
            while (*p == ' ' || *p == '\t') p++;
            strip_comments(p);

            // ---- #include ----
            if (strncmp(p, "include", 7) == 0 && (p[7] == ' ' || p[7] == '\t' || p[7] == '"' || p[7] == '<')) {
                if (!cond_active()) { free(orig); continue; }
                p += 7; while (*p == ' ' || *p == '\t') p++;
                if (*p == '"' || *p == '<') {     // "..." looks in this file's folder first
                    char close = *p == '"' ? '"' : '>';
                    const char *t = ++p; while (*p && *p != close) p++;
                    char *target = dupn(t, (size_t)(p - t));
                    include_file(target, close == '"' ? dir : NULL);
                    free(target);
                }
                free(orig); continue;
            }

            // ---- #define (object-like or function-like) ----
            if (strncmp(p, "define", 6) == 0 && (p[6] == ' ' || p[6] == '\t')) {
                if (!cond_active()) { free(orig); continue; }
                p += 6; while (*p == ' ' || *p == '\t') p++;
                size_t n; char *name = take_ident(p, &n); p += n;
                if (*p == '(') {
                    // function-like: '(' must be glued to the name (no space)
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
                    }
                    if (*p == ')') p++;
                    while (*p == ' ' || *p == '\t') p++;
                    macro_define_func(name, params, np, variadic, p);
                    for (int k = 0; k < np; k++) free(params[k]);
                    free(params);
                } else {
                    while (*p == ' ' || *p == '\t') p++;
                    macro_define(name, p);
                }
                free(name);
                free(orig); continue;
            }

            // ---- #undef ----
            if (strncmp(p, "undef", 5) == 0 && (p[5] == ' ' || p[5] == '\t')) {
                if (!cond_active()) { free(orig); continue; }
                p += 5; while (*p == ' ' || *p == '\t') p++;
                size_t n; char *name = take_ident(p, &n);
                macro_undef(name);
                free(name);
                free(orig); continue;
            }

            // ---- #ifdef / #ifndef ----
            int is_ifdef  = (strncmp(p, "ifdef",  5) == 0 && (p[5] == ' ' || p[5] == '\t'));
            int is_ifndef = (strncmp(p, "ifndef", 6) == 0 && (p[6] == ' ' || p[6] == '\t'));
            if (is_ifdef || is_ifndef) {
                p += is_ifndef ? 6 : 5;
                while (*p == ' ' || *p == '\t') p++;
                const char *nm = p; while (is_ident_cont(*p)) p++;
                int defined = macro_find_n(nm, (size_t)(p - nm)) != NULL;
                int active = is_ifndef ? !defined : defined;
                cond_push(active);
                free(orig); continue;
            }

            // ---- #if <expr> ----
            if (strncmp(p, "if", 2) == 0 && (p[2] == ' ' || p[2] == '\t' || p[2] == '(')) {
                p += 2;
                int active = eval_if(p);
                cond_push(active);
                free(orig); continue;
            }

            // ---- #elif <expr> ----
            if (strncmp(p, "elif", 4) == 0 && (p[4] == ' ' || p[4] == '\t' || p[4] == '(')) {
                if (cond_n == 0) { fprintf(stderr, MSG_ERR_STRAY_ELIF); exit(1); }
                p += 4;
                if (cond_seen_true[cond_n-1]) {
                    cond_stk[cond_n-1] = 0;          // an earlier branch already won
                } else {
                    int active = eval_if(p);
                    cond_stk[cond_n-1] = active;
                    if (active) cond_seen_true[cond_n-1] = 1;
                }
                free(orig); continue;
            }

            // ---- #else ----
            if (strcmp(p, "else") == 0 || strncmp(p, "else ", 5) == 0 || strncmp(p, "else\t", 5) == 0) {
                if (cond_n == 0) { fprintf(stderr, MSG_ERR_STRAY_ELSE); exit(1); }
                cond_stk[cond_n-1] = !cond_seen_true[cond_n-1];
                cond_seen_true[cond_n-1] = 1;
                free(orig); continue;
            }

            // ---- #endif ----
            if (strcmp(p, "endif") == 0 || strncmp(p, "endif ", 6) == 0 || strncmp(p, "endif\t", 6) == 0) {
                if (cond_n == 0) { fprintf(stderr, MSG_ERR_STRAY_ENDIF); exit(1); }
                cond_n--;
                free(orig); continue;
            }

            // ---- #pragma once (consumed) / others (passthrough, e.g. yanc) ----
            if (strncmp(p, "pragma", 6) == 0) {
                const char *q = p + 6; while (*q == ' ' || *q == '\t') q++;
                if (strncmp(q, "once", 4) == 0 && (q[4] == 0 || isspace((unsigned char)q[4]))) {
                    if (cond_active()) {
                        char *canon = path_canon(path);
                        once_mark(canon);
                        free(canon);
                    }
                    free(orig); continue;          // don't emit `#pragma once`
                }
                if (cond_active()) pp_emit(line, file, ln, line);   // e.g. #pragma yanc
                free(orig); continue;
            }

            // ---- #error stops here; #warning only says so ----
            if ((strncmp(p, "error", 5) == 0 || strncmp(p, "warning", 7) == 0) &&
                (p[p[0] == 'e' ? 5 : 7] == 0 || isspace((unsigned char)p[p[0] == 'e' ? 5 : 7]))) {
                if (cond_active()) {
                    int err = p[0] == 'e';
                    const char *msg = p + (err ? 5 : 7); while (*msg == ' ' || *msg == '\t') msg++;
                    fprintf(stderr, "%s:%d: %s: #%s %s\n", file, ln, err ? MSG_WORD_ERROR : MSG_WORD_WARNING, err ? "error" : "warning", msg);
                    if (err) exit(1);
                }
                free(orig); continue;
            }

            // ---- unknown directive: pass through ----
            if (cond_active()) pp_emit(line, file, ln, line);
            free(orig); continue;
        }

        if (cond_active()) {
            char *expanded = expand_str(line, 0);
            pp_emit(expanded, file, ln, line);
            free(expanded);
        }
        free(orig);
    }
    free(line);
    free(dir);

    fclose(fp);
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
