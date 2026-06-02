// ----------------------------------------------------------------------------
// CPPComp — standalone C preprocessor ------------------------------------------
// ----------------------------------------------------------------------------
// Standalone binary that runs before cppcomp.exe. Handles:
//   #include "name"          (relative to current file + -I paths)
//   #include <name>          (-I paths only)
//   #define NAME body        (object- and function-like macros, incl.
//                             variadic `...`/__VA_ARGS__, # stringize, ## paste)
//   #undef NAME
//   #ifdef NAME / #ifndef NAME / #else / #endif
//   #pragma once             (skip the file on any later #include)
//   #pragma yanc ...         (passed through verbatim)
//
// usage:  cpppp -i input.c [-o out.c] [-I dir]*
// ----------------------------------------------------------------------------

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "../../yanc_version.h"   // YANC_VERSION shared across all five binaries

#define MAX_INCDIRS 32
static const char *incdirs[MAX_INCDIRS];
static int         n_incdirs = 0;

// files marked `#pragma once`: canonical paths to skip on any later #include.
#define MAX_ONCE 1024
static char *once_paths[MAX_ONCE];
static int   n_once = 0;

// canonicalise a path so the same file reached via different #include spellings
// compares equal (absolute, forward slashes, lowercased — Windows is
// case-insensitive). Falls back to the input on failure.
static void path_canon(const char *in, char *out, size_t sz)
{
#ifdef _WIN32
    char tmp[2048];
    if (_fullpath(tmp, in, sizeof(tmp))) {
        size_t i = 0;
        for (; tmp[i] && i < sz - 1; i++) {
            char c = tmp[i] == '\\' ? '/' : (char)tolower((unsigned char)tmp[i]);
            out[i] = c;
        }
        out[i] = 0;
        return;
    }
#else
    // realpath may write up to PATH_MAX bytes; let glibc allocate the buffer
    // (realpath(in, NULL), POSIX.1-2008) instead of risking a fixed-size one.
    char *rp = realpath(in, NULL);
    if (rp) { snprintf(out, sz, "%s", rp); free(rp); return; }
#endif
    snprintf(out, sz, "%s", in);
}

static int once_seen(const char *canon)
{
    for (int i = 0; i < n_once; i++) if (strcmp(once_paths[i], canon) == 0) return 1;
    return 0;
}
static void once_mark(const char *canon)
{
    if (once_seen(canon) || n_once >= MAX_ONCE) return;
    once_paths[n_once++] = strdup(canon);
}

#define MAX_MPARAMS 32
typedef struct macro {
    char *name;
    char *body;
    int   is_func;                 // 1 = function-like macro
    int   is_variadic;             // 1 = trailing `...` (use __VA_ARGS__ in body)
    char *params[MAX_MPARAMS];
    int   n_params;                // named params (excludes `...`)
    struct macro *next;
} macro;
static macro *macros = NULL;

static macro *macro_find(const char *name)
{
    for (macro *m = macros; m; m = m->next) if (strcmp(m->name, name) == 0) return m;
    return NULL;
}

static void macro_free_params(macro *m)
{
    for (int i = 0; i < m->n_params; i++) free(m->params[i]);
    m->n_params = 0; m->is_func = 0; m->is_variadic = 0;
}

static macro *macro_get_or_new(const char *name)
{
    macro *m = macro_find(name);
    if (m) { free(m->body); m->body = NULL; macro_free_params(m); return m; }
    m = calloc(1, sizeof(macro));
    m->name = strdup(name);
    m->next = macros;
    macros = m;
    return m;
}

static void macro_define(const char *name, const char *body)
{
    macro *m = macro_get_or_new(name);
    m->body = strdup(body ? body : "");
    m->is_func = 0;
}

static void macro_define_func(const char *name, char **params, int np, int variadic, const char *body)
{
    macro *m = macro_get_or_new(name);
    m->body = strdup(body ? body : "");
    m->is_func = 1;
    m->is_variadic = variadic;
    m->n_params = np;
    for (int i = 0; i < np; i++) m->params[i] = strdup(params[i]);
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

static const char *macro_lookup(const char *name)
{
    macro *m = macro_find(name);
    return m ? (m->body ? m->body : "") : NULL;
}

#define MAX_COND 64
static int cond_stk[MAX_COND];
static int cond_seen_true[MAX_COND];
static int cond_n = 0;

static int cond_active(void)
{
    for (int i = 0; i < cond_n; i++) if (!cond_stk[i]) return 0;
    return 1;
}

static int is_ident_start(char c) { return isalpha((unsigned char)c) || c == '_'; }
static int is_ident_cont (char c) { return isalnum((unsigned char)c) || c == '_'; }

// ---- growable string buffer ------------------------------------------------

typedef struct { char *p; size_t len, cap; } sbuf;
static void sb_init(sbuf *b) { b->cap = 64; b->p = malloc(b->cap); b->p[0] = 0; b->len = 0; }
static void sb_putc(sbuf *b, char c) {
    if (b->len + 2 >= b->cap) { b->cap *= 2; b->p = realloc(b->p, b->cap); }
    b->p[b->len++] = c; b->p[b->len] = 0;
}
static void sb_puts(sbuf *b, const char *s) { while (*s) sb_putc(b, *s++); }
static void sb_putn(sbuf *b, const char *s, size_t n) { for (size_t i = 0; i < n; i++) sb_putc(b, s[i]); }

static char *dupn(const char *s, size_t n) { char *r = malloc(n + 1); memcpy(r, s, n); r[n] = 0; return r; }

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

#define IS_VA(id) (strcmp((id), "__VA_ARGS__") == 0)

// substitute call args into a function-macro body. Handles # (stringize),
// ## (token paste), and __VA_ARGS__ for variadic macros, plus the common
// GNU `, ##__VA_ARGS__` comma-elision when the variadic part is empty.
// Operands of # and ## use the RAW (unexpanded) argument; elsewhere the
// argument is macro-expanded first.
static char *macro_subst(macro *m, char **args, int na)
{
    char *eargs[MAX_MPARAMS];
    for (int i = 0; i < na && i < MAX_MPARAMS; i++) eargs[i] = expand_str(args[i], 1);

    // build the variadic tail (raw and expanded), joined with ", "
    sbuf vr; sb_init(&vr); sbuf ve; sb_init(&ve);
    if (m->is_variadic) {
        for (int i = m->n_params; i < na && i < MAX_MPARAMS; i++) {
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
                char id[256]; size_t l = q - is, cl = l < sizeof(id)-1 ? l : sizeof(id)-1;
                memcpy(id, is, cl); id[cl] = 0;
                int pi = -1;
                for (int i = 0; i < m->n_params; i++) if (strcmp(id, m->params[i]) == 0) { pi = i; break; }
                if (pi >= 0)               { sb_stringize(&out, pi < na ? args[pi] : ""); b = q; prev_paste = 0; continue; }
                if (m->is_variadic && IS_VA(id)) { sb_stringize(&out, vr.p);              b = q; prev_paste = 0; continue; }
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
            char id[256]; size_t cl = l < sizeof(id)-1 ? l : sizeof(id)-1;
            memcpy(id, s, cl); id[cl] = 0;
            const char *q = b; while (*q == ' ' || *q == '\t') q++;
            int next_paste = (*q == '#' && q[1] == '#');
            int raw = prev_paste || next_paste;      // operands of ## are unexpanded
            int pi = -1;
            for (int i = 0; i < m->n_params; i++) if (strcmp(id, m->params[i]) == 0) { pi = i; break; }
            if (pi >= 0)                     sb_puts(&out, pi < na ? (raw ? args[pi] : eargs[pi]) : "");
            else if (m->is_variadic && IS_VA(id)) sb_puts(&out, raw ? vr.p : ve.p);
            else                             sb_putn(&out, s, l);
            prev_paste = 0;
            continue;
        }
        char c = *b++;
        sb_putc(&out, c);
        if (c != ' ' && c != '\t') prev_paste = 0;
    }
    for (int i = 0; i < na && i < MAX_MPARAMS; i++) free(eargs[i]);
    free(vr.p); free(ve.p);
    return out.p;
}

// fully expand object- and function-like macros in src; returns malloc'd result
static char *expand_str(const char *src, int depth)
{
    sbuf out; sb_init(&out);
    if (depth > 64) { sb_puts(&out, src); return out.p; }   // recursion guard

    const char *p = src;
    while (*p) {
        if (*p == '"' || *p == '\'') {
            char q = *p; sb_putc(&out, *p++);
            while (*p && *p != q) { if (*p == '\\' && p[1]) sb_putc(&out, *p++); sb_putc(&out, *p++); }
            if (*p) sb_putc(&out, *p++);
            continue;
        }
        if (is_ident_start(*p)) {
            const char *s = p; while (is_ident_cont(*p)) p++;
            size_t idlen = p - s;
            char id[256]; size_t cl = idlen < sizeof(id)-1 ? idlen : sizeof(id)-1;
            memcpy(id, s, cl); id[cl] = 0;
            macro *m = macro_find(id);
            if (m && m->is_func) {
                const char *q = p; while (*q == ' ' || *q == '\t') q++;
                if (*q == '(') {
                    q++;                               // consume '('
                    char *args[MAX_MPARAMS]; int na = 0;
                    int dep = 1; const char *as = q;
                    while (*q && dep > 0) {
                        if (*q == '(') dep++;
                        else if (*q == ')') { dep--; if (dep == 0) break; }
                        else if (*q == ',' && dep == 1) {
                            if (na < MAX_MPARAMS) args[na++] = dup_trim(as, q - as);
                            as = q + 1;
                        }
                        q++;
                    }
                    if (na < MAX_MPARAMS) args[na++] = dup_trim(as, q - as);
                    if (*q == ')') q++;
                    // FOO() with a 0-param macro: drop the single empty arg
                    if (m->n_params == 0 && na == 1 && args[0][0] == 0) { free(args[0]); na = 0; }
                    char *subst = macro_subst(m, args, na);
                    char *rec   = expand_str(subst, depth + 1);
                    sb_puts(&out, rec);
                    free(rec); free(subst);
                    for (int i = 0; i < na; i++) free(args[i]);
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
                char nm[128]; int j = 0;
                while (is_ident_cont(*p) && j < (int)sizeof(nm)-1) nm[j++] = *p++;
                nm[j] = 0;
                if (paren) { while (*p == ' ' || *p == '\t') p++; if (*p == ')') p++; }
                sb_putc(&out, macro_find(nm) ? '1' : '0');
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

static FILE *out_f;
static int   depth = 0;

static void process_file(const char *path);

static FILE *open_include(const char *name, const char *ref_dir, char *resolved, size_t rsz)
{
    char path[2048];
    if (ref_dir) {
        snprintf(path, sizeof(path), "%s/%s", ref_dir, name);
        FILE *f = fopen(path, "r");
        if (f) { snprintf(resolved, rsz, "%s", path); return f; }
    }
    for (int i = 0; i < n_incdirs; i++) {
        snprintf(path, sizeof(path), "%s/%s", incdirs[i], name);
        FILE *f = fopen(path, "r");
        if (f) { snprintf(resolved, rsz, "%s", path); return f; }
    }
    return NULL;
}

static void dirname_of(const char *path, char *out, size_t sz)
{
    const char *last = path;
    for (const char *p = path; *p; p++) if (*p == '/' || *p == '\\') last = p;
    if (last == path) snprintf(out, sz, ".");
    else {
        size_t L = last - path;
        if (L >= sz) L = sz - 1;
        memcpy(out, path, L); out[L] = 0;
    }
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
    if (depth > 32) { fprintf(stderr, "cpppp: include too deep\n"); exit(1); }
    depth++;

    char dir[2048]; dirname_of(path, dir, sizeof(dir));

    FILE *fp = fopen(path, "r");
    if (!fp) { fprintf(stderr, "cpppp: cannot open '%s'\n", path); exit(1); }

    char line[4096];
    while (fgets(line, sizeof(line), fp)) {
        char *orig = strdup(line);
        char *t = trim_in_place(orig);

        if (t[0] == '#') {
            char *p = t + 1;
            while (*p == ' ' || *p == '\t') p++;

            // ---- #include ----
            if (strncmp(p, "include", 7) == 0 && (p[7] == ' ' || p[7] == '\t' || p[7] == '"' || p[7] == '<')) {
                if (!cond_active()) { free(orig); continue; }
                p += 7; while (*p == ' ' || *p == '\t') p++;
                char target[1024]; target[0] = 0;
                if (*p == '"') {
                    p++; int i = 0;
                    while (*p && *p != '"' && i < (int)sizeof(target)-1) target[i++] = *p++;
                    target[i] = 0;
                    char resolved[2048];
                    FILE *inc = open_include(target, dir, resolved, sizeof(resolved));
                    if (!inc) { fprintf(stderr, "cpppp: can't find #include \"%s\"\n", target); exit(1); }
                    fclose(inc);
                    char canon[2048]; path_canon(resolved, canon, sizeof(canon));
                    if (!once_seen(canon)) process_file(resolved);
                } else if (*p == '<') {
                    p++; int i = 0;
                    while (*p && *p != '>' && i < (int)sizeof(target)-1) target[i++] = *p++;
                    target[i] = 0;
                    char resolved[2048];
                    FILE *inc = open_include(target, NULL, resolved, sizeof(resolved));
                    if (!inc) { fprintf(stderr, "cpppp: can't find #include <%s>\n", target); exit(1); }
                    fclose(inc);
                    char canon[2048]; path_canon(resolved, canon, sizeof(canon));
                    if (!once_seen(canon)) process_file(resolved);
                }
                free(orig); continue;
            }

            // ---- #define (object-like or function-like) ----
            if (strncmp(p, "define", 6) == 0 && (p[6] == ' ' || p[6] == '\t')) {
                if (!cond_active()) { free(orig); continue; }
                p += 6; while (*p == ' ' || *p == '\t') p++;
                char name[128]; int i = 0;
                while ((isalnum((unsigned char)*p) || *p == '_') && i < (int)sizeof(name)-1) name[i++] = *p++;
                name[i] = 0;
                if (*p == '(') {
                    // function-like: '(' must be glued to the name (no space)
                    p++;
                    char *params[MAX_MPARAMS]; int np = 0; int variadic = 0;
                    while (*p && *p != ')') {
                        while (*p == ' ' || *p == '\t') p++;
                        if (*p == '.' && p[1] == '.' && p[2] == '.') {  // C99 `...`
                            p += 3; variadic = 1;
                            while (*p == ' ' || *p == '\t') p++;
                            break;
                        }
                        char pn[64]; int j = 0;
                        while ((isalnum((unsigned char)*p) || *p == '_') && j < (int)sizeof(pn)-1) pn[j++] = *p++;
                        pn[j] = 0;
                        if (j > 0 && np < MAX_MPARAMS) params[np++] = strdup(pn);
                        while (*p == ' ' || *p == '\t') p++;
                        if (*p == ',') p++;
                    }
                    if (*p == ')') p++;
                    while (*p == ' ' || *p == '\t') p++;
                    macro_define_func(name, params, np, variadic, p);
                    for (int k = 0; k < np; k++) free(params[k]);
                } else {
                    while (*p == ' ' || *p == '\t') p++;
                    macro_define(name, p);
                }
                free(orig); continue;
            }

            // ---- #undef ----
            if (strncmp(p, "undef", 5) == 0 && (p[5] == ' ' || p[5] == '\t')) {
                if (!cond_active()) { free(orig); continue; }
                p += 5; while (*p == ' ' || *p == '\t') p++;
                char name[128]; int i = 0;
                while ((isalnum((unsigned char)*p) || *p == '_') && i < (int)sizeof(name)-1) name[i++] = *p++;
                name[i] = 0;
                macro_undef(name);
                free(orig); continue;
            }

            // ---- #ifdef / #ifndef ----
            int is_ifdef  = (strncmp(p, "ifdef",  5) == 0 && (p[5] == ' ' || p[5] == '\t'));
            int is_ifndef = (strncmp(p, "ifndef", 6) == 0 && (p[6] == ' ' || p[6] == '\t'));
            if (is_ifdef || is_ifndef) {
                p += is_ifndef ? 6 : 5;
                while (*p == ' ' || *p == '\t') p++;
                char name[128]; int i = 0;
                while ((isalnum((unsigned char)*p) || *p == '_') && i < (int)sizeof(name)-1) name[i++] = *p++;
                name[i] = 0;
                int defined = macro_lookup(name) != NULL;
                int active = is_ifndef ? !defined : defined;
                if (cond_n >= MAX_COND) { fprintf(stderr, "cpppp: #ifdef nesting too deep\n"); exit(1); }
                cond_stk[cond_n] = active;
                cond_seen_true[cond_n] = active;
                cond_n++;
                free(orig); continue;
            }

            // ---- #if <expr> ----
            if (strncmp(p, "if", 2) == 0 && (p[2] == ' ' || p[2] == '\t' || p[2] == '(')) {
                p += 2;
                int active = eval_if(p);
                if (cond_n >= MAX_COND) { fprintf(stderr, "cpppp: #if nesting too deep\n"); exit(1); }
                cond_stk[cond_n] = active;
                cond_seen_true[cond_n] = active;
                cond_n++;
                free(orig); continue;
            }

            // ---- #elif <expr> ----
            if (strncmp(p, "elif", 4) == 0 && (p[4] == ' ' || p[4] == '\t' || p[4] == '(')) {
                if (cond_n == 0) { fprintf(stderr, "cpppp: stray #elif\n"); exit(1); }
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
                if (cond_n == 0) { fprintf(stderr, "cpppp: stray #else\n"); exit(1); }
                cond_stk[cond_n-1] = !cond_seen_true[cond_n-1];
                cond_seen_true[cond_n-1] = 1;
                free(orig); continue;
            }

            // ---- #endif ----
            if (strcmp(p, "endif") == 0 || strncmp(p, "endif ", 6) == 0 || strncmp(p, "endif\t", 6) == 0) {
                if (cond_n == 0) { fprintf(stderr, "cpppp: stray #endif\n"); exit(1); }
                cond_n--;
                free(orig); continue;
            }

            // ---- #pragma once (consumed) / others (passthrough, e.g. yanc) ----
            if (strncmp(p, "pragma", 6) == 0) {
                const char *q = p + 6; while (*q == ' ' || *q == '\t') q++;
                if (strncmp(q, "once", 4) == 0 && (q[4] == 0 || isspace((unsigned char)q[4]))) {
                    if (cond_active()) {
                        char canon[2048]; path_canon(path, canon, sizeof(canon));
                        once_mark(canon);
                    }
                    free(orig); continue;          // don't emit `#pragma once`
                }
                if (cond_active()) fprintf(out_f, "%s", line);   // e.g. #pragma yanc
                free(orig); continue;
            }

            // ---- unknown directive: pass through ----
            if (cond_active()) fprintf(out_f, "%s", line);
            free(orig); continue;
        }

        if (cond_active()) {
            char *expanded = expand_str(line, 0);
            fputs(expanded, out_f);
            free(expanded);
        }
        free(orig);
    }

    fclose(fp);
    depth--;
}

static void usage(void)
{
    fprintf(stderr,
        "cpppp %s — C preprocessor for the CPPComp toolchain\n"
        "usage: cpppp -i <input.c> [-o <output.c>] [-I <dir>]*\n"
        "  -i <file>        input .c file\n"
        "  -o <file>        output preprocessed .c (default: stdout)\n"
        "  -I <dir>         add include search directory (repeatable)\n"
        "  -h, --help       show this help\n"
        "  -V, --version    show version and exit\n",
        YANC_VERSION);
    exit(1);
}

int main(int argc, char **argv)
{
    const char *in_path  = NULL;
    const char *out_path = NULL;

    for (int i = 1; i < argc; i++) {
        if      (!strcmp(argv[i], "-i") && i+1 < argc) in_path  = argv[++i];
        else if (!strcmp(argv[i], "-o") && i+1 < argc) out_path = argv[++i];
        else if (!strcmp(argv[i], "-I") && i+1 < argc) {
            if (n_incdirs < MAX_INCDIRS) incdirs[n_incdirs++] = argv[++i];
            else { fprintf(stderr, "cpppp: too many -I dirs\n"); exit(1); }
        }
        else if (!strcmp(argv[i], "-h") || !strcmp(argv[i], "--help")) usage();
        else if (!strcmp(argv[i], "-V") || !strcmp(argv[i], "--version")) {
            printf("cpppp %s\n", YANC_VERSION);
            return 0;
        }
        else { fprintf(stderr, "cpppp: unknown option '%s'\n", argv[i]); usage(); }
    }
    if (!in_path) usage();

    out_f = out_path ? fopen(out_path, "w") : stdout;
    if (!out_f) { fprintf(stderr, "cpppp: cannot open '%s'\n", out_path); exit(1); }

    process_file(in_path);

    if (out_f != stdout) fclose(out_f);
    return 0;
}
