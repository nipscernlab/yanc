// ----------------------------------------------------------------------------
// CNIPS — standalone C preprocessor ------------------------------------------
// ----------------------------------------------------------------------------
// Standalone binary that runs before cnips.exe. Handles:
//   #include "name"          (relative to current file + -I paths)
//   #include <name>          (-I paths only)
//   #define NAME body        (object-like macros — no function macros yet)
//   #undef NAME
//   #ifdef NAME / #ifndef NAME / #else / #endif
//   #pragma yanc ...         (passed through verbatim)
//
// usage:  cnipspp -i input.c [-o out.c] [-I dir]*
// ----------------------------------------------------------------------------

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define MAX_INCDIRS 32
static const char *incdirs[MAX_INCDIRS];
static int         n_incdirs = 0;

typedef struct macro { char *name; char *body; struct macro *next; } macro;
static macro *macros = NULL;

static void macro_define(const char *name, const char *body)
{
    for (macro *m = macros; m; m = m->next) {
        if (strcmp(m->name, name) == 0) {
            free(m->body);
            m->body = strdup(body ? body : "");
            return;
        }
    }
    macro *m = malloc(sizeof(macro));
    m->name = strdup(name);
    m->body = strdup(body ? body : "");
    m->next = macros;
    macros = m;
}

static void macro_undef(const char *name)
{
    macro **pp = &macros;
    while (*pp) {
        if (strcmp((*pp)->name, name) == 0) {
            macro *dead = *pp;
            *pp = dead->next;
            free(dead->name); free(dead->body); free(dead);
            return;
        }
        pp = &(*pp)->next;
    }
}

static const char *macro_lookup(const char *name)
{
    for (macro *m = macros; m; m = m->next) if (strcmp(m->name, name) == 0) return m->body;
    return NULL;
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

static char *expand_line(const char *line)
{
    size_t cap = strlen(line) + 1, len = 0;
    char *buf = malloc(cap);
    buf[0] = 0;

    const char *p = line;
    while (*p) {
        if (*p == '"' || *p == '\'') {
            char q = *p;
            const char *start = p++;
            while (*p && *p != q) { if (*p == '\\' && p[1]) p++; p++; }
            if (*p) p++;
            size_t sz = p - start;
            if (len + sz + 1 >= cap) { cap = (len + sz + 1) * 2; buf = realloc(buf, cap); }
            memcpy(buf + len, start, sz); len += sz; buf[len] = 0;
            continue;
        }
        if (is_ident_start(*p)) {
            const char *s = p;
            while (is_ident_cont(*p)) p++;
            size_t idlen = p - s;
            char idbuf[256];
            if (idlen >= sizeof(idbuf)) idlen = sizeof(idbuf) - 1;
            memcpy(idbuf, s, idlen); idbuf[idlen] = 0;
            const char *body = macro_lookup(idbuf);
            if (body) {
                size_t bl = strlen(body);
                if (len + bl + 1 >= cap) { cap = (len + bl + 1) * 2; buf = realloc(buf, cap); }
                memcpy(buf + len, body, bl); len += bl; buf[len] = 0;
            } else {
                if (len + idlen + 1 >= cap) { cap = (len + idlen + 1) * 2; buf = realloc(buf, cap); }
                memcpy(buf + len, s, idlen); len += idlen; buf[len] = 0;
            }
            continue;
        }
        if (len + 2 >= cap) { cap = (len + 2) * 2; buf = realloc(buf, cap); }
        buf[len++] = *p++; buf[len] = 0;
    }
    return buf;
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
    if (depth > 32) { fprintf(stderr, "cnipspp: include too deep\n"); exit(1); }
    depth++;

    char dir[2048]; dirname_of(path, dir, sizeof(dir));

    FILE *fp = fopen(path, "r");
    if (!fp) { fprintf(stderr, "cnipspp: cannot open '%s'\n", path); exit(1); }

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
                    if (!inc) { fprintf(stderr, "cnipspp: can't find #include \"%s\"\n", target); exit(1); }
                    fclose(inc);
                    process_file(resolved);
                } else if (*p == '<') {
                    p++; int i = 0;
                    while (*p && *p != '>' && i < (int)sizeof(target)-1) target[i++] = *p++;
                    target[i] = 0;
                    char resolved[2048];
                    FILE *inc = open_include(target, NULL, resolved, sizeof(resolved));
                    if (!inc) { fprintf(stderr, "cnipspp: can't find #include <%s>\n", target); exit(1); }
                    fclose(inc);
                    process_file(resolved);
                }
                free(orig); continue;
            }

            // ---- #define ----
            if (strncmp(p, "define", 6) == 0 && (p[6] == ' ' || p[6] == '\t')) {
                if (!cond_active()) { free(orig); continue; }
                p += 6; while (*p == ' ' || *p == '\t') p++;
                char name[128]; int i = 0;
                while ((isalnum((unsigned char)*p) || *p == '_') && i < (int)sizeof(name)-1) name[i++] = *p++;
                name[i] = 0;
                while (*p == ' ' || *p == '\t') p++;
                macro_define(name, p);
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
                if (cond_n >= MAX_COND) { fprintf(stderr, "cnipspp: #ifdef nesting too deep\n"); exit(1); }
                cond_stk[cond_n] = active;
                cond_seen_true[cond_n] = active;
                cond_n++;
                free(orig); continue;
            }

            // ---- #else ----
            if (strcmp(p, "else") == 0 || strncmp(p, "else ", 5) == 0 || strncmp(p, "else\t", 5) == 0) {
                if (cond_n == 0) { fprintf(stderr, "cnipspp: stray #else\n"); exit(1); }
                cond_stk[cond_n-1] = !cond_seen_true[cond_n-1];
                cond_seen_true[cond_n-1] = 1;
                free(orig); continue;
            }

            // ---- #endif ----
            if (strcmp(p, "endif") == 0 || strncmp(p, "endif ", 6) == 0 || strncmp(p, "endif\t", 6) == 0) {
                if (cond_n == 0) { fprintf(stderr, "cnipspp: stray #endif\n"); exit(1); }
                cond_n--;
                free(orig); continue;
            }

            // ---- #pragma yanc passthrough ----
            if (strncmp(p, "pragma", 6) == 0) {
                if (cond_active()) fprintf(out_f, "%s", line);
                free(orig); continue;
            }

            // ---- unknown directive: pass through ----
            if (cond_active()) fprintf(out_f, "%s", line);
            free(orig); continue;
        }

        if (cond_active()) {
            char *expanded = expand_line(line);
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
        "cnipspp — C preprocessor for the CNIPS toolchain\n"
        "usage: cnipspp -i <input.c> [-o <output.c>] [-I <dir>]* [-D NAME[=val]]*\n"
        "  -i <file>   input .c file\n"
        "  -o <file>   output preprocessed .c (default: stdout)\n"
        "  -I <dir>    add include search directory (repeatable)\n"
        "  -D NAME=val pre-define a macro (repeatable)\n"
        "  -h          show this help\n");
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
            else { fprintf(stderr, "cnipspp: too many -I dirs\n"); exit(1); }
        }
        else if (!strcmp(argv[i], "-D") && i+1 < argc) {
            char *eq = strchr(argv[++i], '=');
            if (eq) { *eq = 0; macro_define(argv[i], eq+1); }
            else    { macro_define(argv[i], "1"); }
        }
        else if (!strcmp(argv[i], "-h") || !strcmp(argv[i], "--help")) usage();
        else { fprintf(stderr, "cnipspp: unknown option '%s'\n", argv[i]); usage(); }
    }
    if (!in_path) usage();

    out_f = out_path ? fopen(out_path, "w") : stdout;
    if (!out_f) { fprintf(stderr, "cnipspp: cannot open '%s'\n", out_path); exit(1); }

    process_file(in_path);

    if (out_f != stdout) fclose(out_f);
    return 0;
}
