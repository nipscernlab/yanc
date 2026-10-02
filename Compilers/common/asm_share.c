// asm_share -- scalars whose lives never overlap share one data word. See asm_share.h.

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <ctype.h>
#include <limits.h>
#include "asm_share.h"

// ---- what each mnemonic does to its operand and to the flow -----------------
// A copy of five columns of isa.tsv (mnemonic, operand class, effect on the
// word the operand names, flow, data stack); Scripts/check_isa.py holds it
// to the table.

typedef struct { const char *mn, *cls, *opnd, *flow, *stk; } as_isa;

static const as_isa isa[] = {
    {"LOD", "data", "r", "-", "-"},
    {"P_LOD", "data", "r", "-", "push"},
    {"LDI", "data", "base_r", "-", "-"},
    {"ILI", "data", "base_r", "-", "-"},
    {"SET", "data", "w", "-", "-"},
    {"SET_P", "data", "w", "-", "pop"},
    {"STI", "data", "base_w", "-", "pop"},
    {"ISI", "data", "base_w", "-", "pop"},
    {"PSH", "none", "-", "-", "push"},
    {"POP", "none", "-", "-", "pop"},
    {"INN", "in", "-", "-", "-"},
    {"F_INN", "in", "-", "-", "-"},
    {"P_INN", "in", "-", "-", "push"},
    {"PF_INN", "in", "-", "-", "push"},
    {"OUT", "out", "-", "-", "-"},
    {"JMP", "code", "-", "jmp", "-"},
    {"JIZ", "code", "-", "jz", "-"},
    {"CAL", "code", "-", "call", "-"},
    {"RET", "none", "-", "ret", "-"},
    {"ADD", "data", "r", "-", "-"},
    {"S_ADD", "none", "-", "-", "pop"},
    {"F_ADD", "data", "r", "-", "-"},
    {"SF_ADD", "none", "-", "-", "pop"},
    {"MLT", "data", "r", "-", "-"},
    {"S_MLT", "none", "-", "-", "pop"},
    {"F_MLT", "data", "r", "-", "-"},
    {"SF_MLT", "none", "-", "-", "pop"},
    {"DIV", "data", "r", "-", "-"},
    {"S_DIV", "none", "-", "-", "pop"},
    {"F_DIV", "data", "r", "-", "-"},
    {"SF_DIV", "none", "-", "-", "pop"},
    {"MOD", "data", "r", "-", "-"},
    {"S_MOD", "none", "-", "-", "pop"},
    {"SGN", "data", "r", "-", "-"},
    {"S_SGN", "none", "-", "-", "pop"},
    {"F_SGN", "data", "r", "-", "-"},
    {"SF_SGN", "none", "-", "-", "pop"},
    {"NEG", "none", "-", "-", "-"},
    {"NEG_M", "data", "r", "-", "-"},
    {"P_NEG_M", "data", "r", "-", "push"},
    {"F_NEG", "none", "-", "-", "-"},
    {"F_NEG_M", "data", "r", "-", "-"},
    {"PF_NEG_M", "data", "r", "-", "push"},
    {"ABS", "none", "-", "-", "-"},
    {"ABS_M", "data", "r", "-", "-"},
    {"P_ABS_M", "data", "r", "-", "push"},
    {"F_ABS", "none", "-", "-", "-"},
    {"F_ABS_M", "data", "r", "-", "-"},
    {"PF_ABS_M", "data", "r", "-", "push"},
    {"PST", "none", "-", "-", "-"},
    {"PST_M", "data", "r", "-", "-"},
    {"P_PST_M", "data", "r", "-", "push"},
    {"F_PST", "none", "-", "-", "-"},
    {"F_PST_M", "data", "r", "-", "-"},
    {"PF_PST_M", "data", "r", "-", "push"},
    {"NRM", "none", "-", "-", "-"},
    {"NRM_M", "data", "r", "-", "-"},
    {"P_NRM_M", "data", "r", "-", "push"},
    {"I2F", "none", "-", "-", "-"},
    {"I2F_M", "data", "r", "-", "-"},
    {"P_I2F_M", "data", "r", "-", "push"},
    {"F2I", "none", "-", "-", "-"},
    {"F2I_M", "data", "r", "-", "-"},
    {"P_F2I_M", "data", "r", "-", "push"},
    {"AND", "data", "r", "-", "-"},
    {"S_AND", "none", "-", "-", "pop"},
    {"ORR", "data", "r", "-", "-"},
    {"S_ORR", "none", "-", "-", "pop"},
    {"XOR", "data", "r", "-", "-"},
    {"S_XOR", "none", "-", "-", "pop"},
    {"INV", "none", "-", "-", "-"},
    {"INV_M", "data", "r", "-", "-"},
    {"P_INV_M", "data", "r", "-", "push"},
    {"LAN", "data", "r", "-", "-"},
    {"S_LAN", "none", "-", "-", "pop"},
    {"LOR", "data", "r", "-", "-"},
    {"S_LOR", "none", "-", "-", "pop"},
    {"LIN", "none", "-", "-", "-"},
    {"LIN_M", "data", "r", "-", "-"},
    {"P_LIN_M", "data", "r", "-", "push"},
    {"LES", "data", "r", "-", "-"},
    {"S_LES", "none", "-", "-", "pop"},
    {"F_LES", "data", "r", "-", "-"},
    {"SF_LES", "none", "-", "-", "pop"},
    {"GRE", "data", "r", "-", "-"},
    {"S_GRE", "none", "-", "-", "pop"},
    {"F_GRE", "data", "r", "-", "-"},
    {"SF_GRE", "none", "-", "-", "pop"},
    {"EQU", "data", "r", "-", "-"},
    {"S_EQU", "none", "-", "-", "pop"},
    {"SHL", "data", "r", "-", "-"},
    {"S_SHL", "none", "-", "-", "pop"},
    {"SHR", "data", "r", "-", "-"},
    {"S_SHR", "none", "-", "-", "pop"},
    {"SRS", "data", "r", "-", "-"},
    {"S_SRS", "none", "-", "-", "pop"},
    {"NOP", "none", "-", "-", "-"},
    {"F_ROT", "none", "-", "-", "-"},
    {"F_SU1", "data", "r", "-", "-"},
    {"F_SU2", "data", "r", "-", "-"},
    {"SF_SU1", "none", "-", "-", "pop"},
    {"SF_SU2", "none", "-", "-", "pop"},
    {"F_SCL", "data", "r", "-", "-"},
    {"SF_SCL", "none", "-", "-", "pop"},
    {"XPO", "none", "-", "-", "-"},
    {"XPO_M", "data", "r", "-", "-"},
    {"LEA", "lea", "addr", "-", "-"},
    {"LOD_V", "offset", "r", "-", "-"},
    {"P_LOD_V", "offset", "r", "-", "push"},
    {"SET_V", "offset", "w", "-", "-"},
    {"ADD_V", "offset", "r", "-", "-"},
    {"F_ADD_V", "offset", "r", "-", "-"},
    {"MLT_V", "offset", "r", "-", "-"},
    {"F_MLT_V", "offset", "r", "-", "-"},
};

#define N_ISA ((int)(sizeof(isa) / sizeof(isa[0])))

static int isa_find(const char *mn)
{
    for (int i = 0; i < N_ISA; i++) if (strcmp(isa[i].mn, mn) == 0) return i;
    return -1;
}

// ---- small growable tables ---------------------------------------------------

typedef struct { char **s; int n, cap; } strtab;

static int st_find(strtab *t, const char *s)
{
    for (int i = 0; i < t->n; i++) if (strcmp(t->s[i], s) == 0) return i;
    return -1;
}

static int st_add(strtab *t, const char *s)            // index of s, added if new
{
    int i = st_find(t, s);
    if (i >= 0) return i;
    if (t->n == t->cap)
    {
        t->cap = t->cap ? 2 * t->cap : 64;
        t->s   = realloc(t->s, t->cap * sizeof(char *));
    }
    t->s[t->n] = strdup(s);
    return t->n++;
}

static void st_free(strtab *t) { for (int i = 0; i < t->n; i++) free(t->s[i]); free(t->s); }

// ---- one instruction ----------------------------------------------------------

typedef struct {
    int  op;          // row of isa[]
    int  name;        // data name it touches (-1: none, or a literal)
    int  line;        // line of the file it sits on
    char target[128]; // label, for jmp / jz / call
    int  nsucc, succ[2];
} as_ins;

static int is_literal(const char *s)
{
    if (*s == '-' || *s == '+') s++;
    if (*s == '.') s++;
    return isdigit((unsigned char)*s);
}

// ---- bit sets -------------------------------------------------------------------

#define BS_TEST(b, i) (((b)[(i) >> 6] >> ((i) & 63)) & 1)
#define BS_SET(b, i)  ((b)[(i) >> 6] |= (uint64_t)1 << ((i) & 63))

// ---- the pass -------------------------------------------------------------------

int asm_share(const char *asm_path)
{
    // read the whole file, line by line ---------------------------------------

    FILE *f = fopen(asm_path, "r");
    if (!f) return -1;
    strtab lines = {0};
    char buf[4096];
    while (fgets(buf, sizeof(buf), f))
    {
        if (lines.n == lines.cap)
        {
            lines.cap = lines.cap ? 2 * lines.cap : 1024;
            lines.s   = realloc(lines.s, lines.cap * sizeof(char *));
        }
        lines.s[lines.n++] = strdup(buf);
    }
    fclose(f);

    strtab  names = {0}, labels = {0};
    int    *lab_at = NULL, lab_cap = 0;
    as_ins *ins = NULL; int n = 0, cap = 0;
    char   *pinned = NULL; int pin_cap = 0;
    int     itr = -1, saved = 0, ok = 1;

    // parse: labels, directives, instructions -----------------------------------

    for (int ln = 0; ln < lines.n && ok; ln++)
    {
        char *text = lines.s[ln];
        int   end  = (int)strlen(text);
        char *cm   = strstr(text, "//");
        if (cm) end = (int)(cm - text);

        // split into tokens, keeping where each one starts
        int ts[8], tl[8], nt = 0;
        for (int p = 0; p < end && nt < 8; )
        {
            while (p < end && isspace((unsigned char)text[p])) p++;
            if (p >= end) break;
            ts[nt] = p;
            while (p < end && !isspace((unsigned char)text[p])) p++;
            tl[nt] = p - ts[nt]; nt++;
        }

        int t = 0;
        char tok[256];
        #define TOK(k) (snprintf(tok, sizeof(tok), "%.*s", tl[k], text + ts[k]), tok)

        for (; t < nt && text[ts[t]] == '@'; t++)          // labels point at the next instruction
        {
            snprintf(tok, sizeof(tok), "%.*s", tl[t] - 1, text + ts[t] + 1);
            int li = st_add(&labels, tok);
            if (li >= lab_cap) { lab_cap = 2 * li + 64; lab_at = realloc(lab_at, lab_cap * sizeof(int)); }
            lab_at[li] = n;
        }
        if (t >= nt) continue;

        if (text[ts[t]] == '#')
        {
            TOK(t);
            if (strcmp(tok, "#ITRAD") == 0) itr = n;
            if ((strcmp(tok, "#array") == 0 || strcmp(tok, "#arrays") == 0) && t + 1 < nt)
            {
                int ni = st_add(&names, TOK(t + 1));
                if (ni >= pin_cap)
                {
                    int nc = 2 * ni + 64;
                    pinned = realloc(pinned, nc);
                    memset(pinned + pin_cap, 0, nc - pin_cap);
                    pin_cap = nc;
                }
                pinned[ni] = 1;
            }
            continue;
        }

        int op = isa_find(TOK(t));
        if (op < 0) { ok = 0; break; }                     // not ours to guess

        if (n == cap) { cap = cap ? 2 * cap : 1024; ins = realloc(ins, cap * sizeof(as_ins)); }
        as_ins *x = &ins[n++];
        memset(x, 0, sizeof(*x));
        x->op = op; x->name = -1; x->line = ln;

        const char *cls = isa[op].cls;
        if (strcmp(cls, "code") == 0)
        {
            if (t + 1 >= nt) { ok = 0; break; }
            snprintf(x->target, sizeof(x->target), "%.*s", tl[t + 1], text + ts[t + 1]);
        }
        else if (strcmp(cls, "data") == 0 || strcmp(cls, "offset") == 0 || strcmp(cls, "lea") == 0)
        {
            if (t + 1 >= nt) { ok = 0; break; }
            TOK(t + 1);
            if (!is_literal(tok))
            {
                int ni = st_add(&names, tok);
                if (ni >= pin_cap)
                {
                    int nc = 2 * ni + 64;
                    pinned = realloc(pinned, nc);
                    memset(pinned + pin_cap, 0, nc - pin_cap);
                    pin_cap = nc;
                }
                x->name = ni;
                const char *e = isa[op].opnd;
                if (strcmp(cls, "data") != 0 || (strcmp(e, "r") != 0 && strcmp(e, "w") != 0))
                    pinned[ni] = 1;                        // touched through an address
            }
        }
        #undef TOK
    }

    int nn = names.n;
    if (!ok || n == 0 || nn < 2) goto done;

    // flow graph ---------------------------------------------------------------

    int *rets = malloc((n + 1) * sizeof(int)), nret = 0;
    for (int i = 0; i < n; i++) if (strcmp(isa[ins[i].op].flow, "call") == 0 && i + 1 < n) rets[nret++] = i + 1;

    for (int i = 0; i < n && ok; i++)
    {
        const char *fl = isa[ins[i].op].flow;
        as_ins *x = &ins[i];
        int tgt = -1;
        if (x->target[0])
        {
            int li = st_find(&labels, x->target);
            if (li < 0) { ok = 0; break; }
            tgt = lab_at[li];
        }
        if      (strcmp(fl, "jmp")  == 0 || strcmp(fl, "call") == 0) { x->succ[0] = tgt; x->nsucc = 1; }
        else if (strcmp(fl, "jz")   == 0) { x->succ[0] = tgt; x->succ[1] = i + 1; x->nsucc = 2; }
        else if (strcmp(fl, "ret")  == 0) x->nsucc = -1;   // every return site, below
        else                              { x->succ[0] = i + 1; x->nsucc = 1; }
    }
    if (!ok) { free(rets); goto done; }

    // liveness, iterated to a fixed point -------------------------------------------

    int W = (nn + 63) / 64;
    uint64_t *lin  = calloc((size_t)n * W, sizeof(uint64_t));
    uint64_t *lout = calloc((size_t)n * W, sizeof(uint64_t));
    uint64_t *tmp  = malloc(W * sizeof(uint64_t));

    for (int changed = 1; changed; )
    {
        changed = 0;
        for (int i = n - 1; i >= 0; i--)
        {
            as_ins *x = &ins[i];
            memset(tmp, 0, W * sizeof(uint64_t));
            int ns = x->nsucc < 0 ? nret : x->nsucc;
            for (int k = 0; k < ns; k++)
            {
                int s = x->nsucc < 0 ? rets[k] : x->succ[k];
                if (s < 0 || s >= n) continue;
                for (int w = 0; w < W; w++) tmp[w] |= lin[(size_t)s * W + w];
            }
            if (itr >= 0 && itr < n)
                for (int w = 0; w < W; w++) tmp[w] |= lin[(size_t)itr * W + w];

            uint64_t *o = &lout[(size_t)i * W], *li = &lin[(size_t)i * W];
            if (memcmp(o, tmp, W * sizeof(uint64_t))) { memcpy(o, tmp, W * sizeof(uint64_t)); changed = 1; }

            // live in = use | (live out - def)
            const char *e = isa[x->op].opnd;
            int def = x->name >= 0 && strcmp(isa[x->op].cls, "data") == 0 && strcmp(e, "w") == 0;
            for (int w = 0; w < W; w++)
            {
                uint64_t v = tmp[w];
                if (def && (x->name >> 6) == w) v &= ~((uint64_t)1 << (x->name & 63));
                if (!def && x->name >= 0 && (x->name >> 6) == w) v |= (uint64_t)1 << (x->name & 63);
                if (li[w] != v) { li[w] = v; changed = 1; }
            }
        }
    }

    // interference: a write clobbers everything still alive after it, and the
    // names read before any write all need their own initial value -------------

    uint64_t *adj = calloc((size_t)nn * W, sizeof(uint64_t));
    for (int i = 0; i < n; i++)
    {
        as_ins *x = &ins[i];
        if (!(x->name >= 0 && strcmp(isa[x->op].cls, "data") == 0 && strcmp(isa[x->op].opnd, "w") == 0)) continue;
        int d = x->name;
        for (int v = 0; v < nn; v++)
            if (v != d && BS_TEST(&lout[(size_t)i * W], v)) { BS_SET(&adj[(size_t)d * W], v); BS_SET(&adj[(size_t)v * W], d); }
    }
    for (int a = 0; a < nn; a++)
        for (int b = 0; b < nn; b++)
            if (a != b && BS_TEST(lin, a) && BS_TEST(lin, b)) BS_SET(&adj[(size_t)a * W], b);

    // first appearance order, for a stable choice of the name a group keeps
    int *order = malloc(nn * sizeof(int)), no = 0;
    char *seen = calloc(nn, 1);
    for (int i = 0; i < n; i++)
        if (ins[i].name >= 0 && !seen[ins[i].name]) { seen[ins[i].name] = 1; order[no++] = ins[i].name; }

    // greedy colouring, most-constrained first --------------------------------------

    int *deg = calloc(nn, sizeof(int)), *color = malloc(nn * sizeof(int));
    for (int v = 0; v < nn; v++) { color[v] = -1; for (int u = 0; u < nn; u++) deg[v] += BS_TEST(&adj[(size_t)v * W], u); }

    int *byd = malloc(no * sizeof(int));
    memcpy(byd, order, no * sizeof(int));
    for (int a = 1; a < no; a++)                           // stable insertion sort on degree
    {
        int v = byd[a], b = a - 1;
        while (b >= 0 && deg[byd[b]] < deg[v]) { byd[b + 1] = byd[b]; b--; }
        byd[b + 1] = v;
    }

    int ncol = 0, ncand = 0;
    char *used = calloc(nn + 1, 1);
    for (int k = 0; k < no; k++)
    {
        int v = byd[k];
        if (pinned[v]) continue;
        ncand++;
        memset(used, 0, nn + 1);
        for (int u = 0; u < nn; u++) if (color[u] >= 0 && BS_TEST(&adj[(size_t)v * W], u)) used[color[u]] = 1;
        int c = 0; while (used[c]) c++;
        color[v] = c; if (c + 1 > ncol) ncol = c + 1;
    }

    // the name each group keeps: the one read before any write, else the first seen
    int *keep = malloc((ncol + 1) * sizeof(int));
    for (int c = 0; c < ncol; c++) keep[c] = -1;
    for (int k = 0; k < no; k++) { int v = order[k]; if (color[v] >= 0 && BS_TEST(lin, v)) keep[color[v]] = v; }
    for (int k = 0; k < no; k++) { int v = order[k]; if (color[v] >= 0 && keep[color[v]] < 0) keep[color[v]] = v; }

    saved = ncand - ncol;

    // rewrite: one "#SHARE <name> <home>" per name that lives in another's
    // word, ahead of the first instruction after NOP; the code keeps every
    // name, so the listing and the waveform still show each variable ---------

    if (saved > 0)
    {
        int at = n > 1 ? ins[1].line : lines.n;
        f = fopen(asm_path, "w");
        if (!f) saved = -1;
        else
        {
            for (int ln = 0; ln < lines.n; ln++)
            {
                if (ln == at)
                    for (int q = 0; q < no; q++)
                    {
                        int v = order[q];
                        if (color[v] >= 0 && keep[color[v]] != v)
                            fprintf(f, "#SHARE %s %s\n", names.s[v], names.s[keep[color[v]]]);
                    }
                fputs(lines.s[ln], f);
            }
            fclose(f);
            printf("Info: %d variables share %d data words (%d saved)\n", ncand, ncol, saved);
        }
    }

    free(used); free(keep); free(byd); free(deg); free(color); free(seen); free(order);
    free(adj); free(tmp); free(lout); free(lin); free(rets);

done:
    free(ins); free(lab_at); free(pinned);
    st_free(&names); st_free(&labels); st_free(&lines);
    return saved;
}

// ---- asm_reach: drop the code no path from the entry gets to ---------------------
// See asm_share.h. One record per line of the file, one per instruction.

enum { RL_OTHER, RL_INS, RL_IFLIVE, RL_ENDLIVE, RL_ARRAY };

typedef struct {
    int kind;       // RL_*
    int ins;        // RL_INS: its index; any other line: the next instruction's
    int lab0, nlab; // its labels: lab_of[lab0 .. lab0+nlab-1]
    int rest;       // offset where the text after the labels starts
    int name;       // RL_ARRAY: the array it declares
} rl_line;

typedef struct {
    int op, name;   // row of isa[] (-1: unknown), data name (-1: none)
    int target;     // label index of a jump or call (-1: none)
    int region;     // the #IFLIVE block it sits in (-1: none)
} rl_ins;

static void *grow(void *p, int *cap, int need, size_t sz)   // room for index `need`
{
    if (need < *cap) return p;
    int nc = *cap ? *cap : 64;
    while (nc <= need) nc *= 2;
    p = realloc(p, nc * sz);
    *cap = nc;
    return p;
}

static int lab_add(strtab *labels, int **at, int *cap, const char *s)
{
    int li = st_add(labels, s);
    if (li >= *cap)
    {
        int old = *cap;
        *at = grow(*at, cap, li, sizeof(int));
        for (int k = old; k < *cap; k++) (*at)[k] = -1;   // -1: not defined (yet)
    }
    return li;
}

static void read_lines(FILE *f, strtab *t)
{
    char buf[4096];
    while (fgets(buf, sizeof(buf), f))
    {
        if (t->n == t->cap)
        {
            t->cap = t->cap ? 2 * t->cap : 1024;
            t->s   = realloc(t->s, t->cap * sizeof(char *));
        }
        t->s[t->n++] = strdup(buf);
    }
}

int asm_reach(const char *asm_path, const char *pc_path)
{
    FILE *f = fopen(asm_path, "r");
    if (!f) return -1;
    strtab lines = {0};
    read_lines(f, &lines);
    fclose(f);

    strtab   labels = {0}, names = {0}, pc = {0};
    int     *lab_at  = NULL, lab_cap  = 0;              // label -> instruction it points at
    int     *lab_of  = NULL, lof_n = 0, lof_cap = 0;    // the labels of every line, in order
    int     *reg_lab = NULL, nreg  = 0, reg_cap = 0;    // #IFLIVE block -> the label it waits for
    int     *roots   = NULL, nroot = 0, root_cap = 0;
    rl_line *L   = calloc(lines.n + 1, sizeof(rl_line));
    rl_ins  *ins = NULL; int n = 0, cap = 0;
    int      ok = 1, region = -1, markers = 0, removed = 0;
    char    *live = NULL, *used = NULL, *needed = NULL;
    int     *work = NULL, *next_live = NULL, *carry = NULL;

    // parse ----------------------------------------------------------------------

    for (int ln = 0; ln < lines.n; ln++)
    {
        char *text = lines.s[ln];
        int   end  = (int)strlen(text);
        char *cm   = strstr(text, "//");
        if (cm) end = (int)(cm - text);

        int ts[8], tl[8], nt = 0;
        for (int p = 0; p < end && nt < 8; )
        {
            while (p < end && isspace((unsigned char)text[p])) p++;
            if (p >= end) break;
            ts[nt] = p;
            while (p < end && !isspace((unsigned char)text[p])) p++;
            tl[nt] = p - ts[nt]; nt++;
        }

        char tok[256];
        #define TOK(k) (snprintf(tok, sizeof(tok), "%.*s", tl[k], text + ts[k]), tok)

        rl_line *l = &L[ln];
        l->kind = RL_OTHER; l->lab0 = lof_n; l->name = -1;
        int t = 0;
        for (; t < nt && text[ts[t]] == '@'; t++)          // labels point at the next instruction
        {
            snprintf(tok, sizeof(tok), "%.*s", tl[t] - 1, text + ts[t] + 1);
            int li = lab_add(&labels, &lab_at, &lab_cap, tok);
            lab_at[li] = n;
            lab_of = grow(lab_of, &lof_cap, lof_n, sizeof(int));
            lab_of[lof_n++] = li;
            l->nlab++;
        }
        l->rest = t > 0 ? ts[t - 1] + tl[t - 1] : 0;
        l->ins  = n;
        if (t >= nt) continue;

        if (text[ts[t]] == '#')
        {
            TOK(t);
            if (strcmp(tok, "#ITRAD") == 0 || strcmp(tok, "#TOAQUI") == 0)
            {
                roots = grow(roots, &root_cap, nroot, sizeof(int));
                roots[nroot++] = n;                        // the interrupt, and the marked address
            }
            else if (strcmp(tok, "#IFLIVE") == 0)
            {
                markers = 1; l->kind = RL_IFLIVE;
                if (region >= 0 || t + 1 >= nt) { ok = 0; continue; }
                reg_lab = grow(reg_lab, &reg_cap, nreg, sizeof(int));
                reg_lab[nreg] = lab_add(&labels, &lab_at, &lab_cap, TOK(t + 1));
                region = nreg++;
            }
            else if (strcmp(tok, "#ENDLIVE") == 0)
            {
                markers = 1; l->kind = RL_ENDLIVE;
                if (region < 0) ok = 0;
                region = -1;
            }
            else if ((strcmp(tok, "#array") == 0 || strcmp(tok, "#arrays") == 0) && t + 1 < nt)
            {
                l->kind = RL_ARRAY;
                l->name = st_add(&names, TOK(t + 1));
            }
            continue;
        }

        ins = grow(ins, &cap, n, sizeof(rl_ins));
        rl_ins *x = &ins[n++];
        x->op = isa_find(TOK(t)); x->name = -1; x->target = -1; x->region = region;
        l->kind = RL_INS;
        if (x->op < 0) { ok = 0; continue; }               // not ours to guess

        const char *cls = isa[x->op].cls;
        if (strcmp(cls, "code") == 0)
        {
            if (t + 1 >= nt) { ok = 0; continue; }
            x->target = lab_add(&labels, &lab_at, &lab_cap, TOK(t + 1));
        }
        else if ((strcmp(cls, "data") == 0 || strcmp(cls, "offset") == 0 || strcmp(cls, "lea") == 0) && t + 1 < nt)
        {
            if (!is_literal(TOK(t + 1))) x->name = st_add(&names, tok);
        }
        if (region >= 0 && strcmp(isa[x->op].flow, "-") != 0) ok = 0;   // a block is straight-line code
        #undef TOK
    }
    if (region >= 0) ok = 0;
    for (int i = 0; i < n && ok; i++)
        if (ins[i].target >= 0 && lab_at[ins[i].target] < 0) ok = 0;   // a jump to a label that is not there

    // the map the waveform reads: one line per instruction of the program part
    if (pc_path && (f = fopen(pc_path, "r")))
    {
        read_lines(f, &pc);
        fclose(f);
        if (pc.n > n) ok = 0;                              // not the map of this file
    }

    // reach: from the entry, the interrupt, #TOAQUI and @fim ------------------------

    live = calloc(n + 1, 1);
    used = calloc(names.n + 1, 1);
    work = malloc((n + 1) * sizeof(int));
    int nw = 0, fim = st_find(&labels, "fim");
    #define VISIT(k) do { int k_ = (k); if (k_ >= 0 && k_ < n && !live[k_]) { live[k_] = 1; work[nw++] = k_; } } while (0)
    if (ok)
    {
        VISIT(0);
        for (int r = 0; r < nroot; r++) VISIT(roots[r]);
        if (fim >= 0) VISIT(lab_at[fim]);
        while (nw > 0)
        {
            int i = work[--nw];
            const char *fl = isa[ins[i].op].flow;
            int tg = ins[i].target >= 0 ? lab_at[ins[i].target] : -1;
            if      (strcmp(fl, "jmp") == 0) VISIT(tg);
            else if (strcmp(fl, "jz") == 0 || strcmp(fl, "call") == 0) { VISIT(tg); VISIT(i + 1); }
            else if (strcmp(fl, "ret") != 0) VISIT(i + 1);
        }
        for (int i = 0; i < n; i++)                        // a block whose label nothing reaches goes too
        {
            int r = ins[i].region;
            if (r < 0) continue;
            int at = lab_at[reg_lab[r]];
            if (at < 0 || at >= n || !live[at]) live[i] = 0;
        }
    }
    #undef VISIT

    // every label a kept jump needs keeps an instruction to sit on
    needed    = calloc(labels.n + 1, 1);
    next_live = malloc((n + 1) * sizeof(int));
    next_live[n] = -1;
    for (int i = n - 1; i >= 0; i--) next_live[i] = live[i] ? i : next_live[i + 1];
    for (int i = 0; i < n; i++) if (live[i] && ins[i].target >= 0) needed[ins[i].target] = 1;
    if (fim >= 0) needed[fim] = 1;
    for (int li = 0; li < labels.n && ok; li++)
        if (needed[li] && lab_at[li] >= 0 && lab_at[li] < n && next_live[lab_at[li]] < 0) ok = 0;
    if (!ok) memset(live, 1, n);                           // anything unclear: keep every instruction

    for (int i = 0; i < n; i++) if (live[i] && ins[i].name >= 0) used[ins[i].name] = 1;
    int dead = 0, arrays_gone = 0;
    for (int i = 0; i < n; i++) dead += !live[i];
    for (int ln = 0; ln < lines.n && ok; ln++) if (L[ln].kind == RL_ARRAY && !used[L[ln].name]) arrays_gone++;
    if (dead == 0 && arrays_gone == 0 && !markers) goto done;

    // rewrite: dead lines, the arrays only they used and the #IFLIVE/#ENDLIVE
    // markers leave; a label a kept jump needs moves to the next kept instruction

    f = fopen(asm_path, "w");
    if (!f) { removed = -1; goto done; }
    carry = malloc((lof_n + 1) * sizeof(int));
    int nc = 0;
    for (int ln = 0; ln < lines.n; ln++)
    {
        rl_line *l = &L[ln];
        char *text = lines.s[ln];
        int drop = (l->kind == RL_INS && !live[l->ins]) || l->kind == RL_IFLIVE || l->kind == RL_ENDLIVE
                || (l->kind == RL_ARRAY && ok && !used[l->name]);
        int move = drop || (l->kind != RL_INS && l->nlab > 0 && l->ins < n && !live[l->ins]);
        if (move)
            for (int k = 0; k < l->nlab; k++)
            {
                int li = lab_of[l->lab0 + k];
                if (needed[li] || (l->ins < n && live[l->ins])) carry[nc++] = li;
            }
        if (drop) continue;
        if (move)
        {
            const char *r = text + l->rest;
            while (*r == ' ' || *r == '\t') r++;
            if (*r != '\n' && *r != '\r' && *r != 0) fputs(r, f);   // else the line only held labels
            continue;
        }
        if (l->kind == RL_INS && nc > 0)
        {
            for (int k = 0; k < nc; k++) fprintf(f, "@%s ", labels.s[carry[k]]);
            nc = 0;
        }
        fputs(text, f);
    }
    fclose(f);

    if (pc.n > 0 && dead > 0)
    {
        f = fopen(pc_path, "w");
        if (!f) { removed = -1; goto done; }
        for (int i = 0; i < pc.n; i++) { if (live[i]) fputs(pc.s[i], f); else removed++; }
        fclose(f);
    }
    if (dead > 0) printf("Info: %d unreachable instructions removed\n", dead);

done:
    free(carry); free(needed); free(next_live); free(work); free(used); free(live);
    free(L); free(ins); free(lab_at); free(lab_of); free(reg_lab); free(roots);
    st_free(&labels); st_free(&names); st_free(&lines); st_free(&pc);
    return removed;
}

// ---- asm_depth: how deep the two stacks get -----------------------------------------
// See asm_share.h. A routine is the code a CAL target reaches until its RETs
// (plus the entry at address 0 and the interrupt point). For each one, once
// the routines it calls are known: the data-stack depth it reaches above its
// entry, its net effect at RET (a callee pops its arguments: negative), and
// how many calls deep it goes.

typedef struct {
    int  n;            // instructions
    int *op, *tgt;     // row of isa[]; instruction a jump or call goes to (-1: none)
    int *entry;        // instruction -> routine index (-1: not an entry)
    int *state;        // routine: 0 not yet, 1 being walked (a call back to it: recursion), 2 done
    int *net, *maxd, *maxc;
    const char *why;   // why the depths could not be worked out (NULL: they could)
} dp_ctx;

static void dp_walk(dp_ctx *c, int r, int start)
{
    if (c->why) return;
    if (c->state[r] == 1) { c->why = "a routine calls itself back (recursion)"; return; }
    if (c->state[r] == 2) return;
    c->state[r] = 1;

    int n = c->n, unset = INT_MIN;
    int *dep  = malloc((n + 1) * sizeof(int));
    int *work = malloc((n + 1) * sizeof(int)), nw = 0;
    for (int i = 0; i < n; i++) dep[i] = unset;
    int net = unset, maxd = 0, maxc = 0;

    #define GO(k, v) do { int k_ = (k), v_ = (v);                                     \
        if (k_ < 0 || k_ >= n)   { c->why = "the code runs off the end"; break; }      \
        if (dep[k_] == unset)    { dep[k_] = v_; work[nw++] = k_; }                    \
        else if (dep[k_] != v_)  { c->why = "two paths reach one instruction with different stack depths"; } \
    } while (0)

    GO(start, 0);
    while (nw > 0 && !c->why)
    {
        int i = work[--nw], d = dep[i];
        const as_isa *x = &isa[c->op[i]];
        if      (strcmp(x->stk, "push") == 0) d++;
        else if (strcmp(x->stk, "pop")  == 0) d--;
        if (d > maxd) maxd = d;
        if      (strcmp(x->flow, "jmp") == 0) GO(c->tgt[i], d);
        else if (strcmp(x->flow, "jz")  == 0) { GO(c->tgt[i], d); GO(i + 1, d); }
        else if (strcmp(x->flow, "call") == 0)
        {
            int q = c->entry[c->tgt[i]];
            dp_walk(c, q, c->tgt[i]);
            if (c->why) break;
            if (d + c->maxd[q] > maxd) maxd = d + c->maxd[q];
            if (1 + c->maxc[q] > maxc) maxc = 1 + c->maxc[q];
            GO(i + 1, d + c->net[q]);
        }
        else if (strcmp(x->flow, "ret") == 0)
        {
            if (net == unset) net = d;
            else if (net != d) c->why = "two RETs of one routine leave different stack depths";
        }
        else GO(i + 1, d);
    }
    #undef GO

    c->net[r]  = net == unset ? 0 : net;
    c->maxd[r] = maxd;
    c->maxc[r] = maxc;
    c->state[r] = 2;
    free(dep); free(work);
}

int asm_depth(const char *asm_path, int *sdepth, int *ddepth)
{
    FILE *f = fopen(asm_path, "r");
    if (!f) return -1;
    strtab lines = {0}, labels = {0};
    read_lines(f, &lines);
    fclose(f);

    int *lab_at = NULL, lab_cap = 0, n = 0, itr = -1, ret = 0;
    int *op = NULL, *tgl = NULL, tgl_cap = 0, op_cap = 0;
    const char *why = NULL;
    *sdepth = *ddepth = 0;

    for (int ln = 0; ln < lines.n && !why; ln++)       // parse: labels, #ITRAD, instructions
    {
        char *text = lines.s[ln];
        int   end  = (int)strlen(text);
        char *cm   = strstr(text, "//");
        if (cm) end = (int)(cm - text);
        int ts[8], tl[8], nt = 0;
        for (int p = 0; p < end && nt < 8; )
        {
            while (p < end && isspace((unsigned char)text[p])) p++;
            if (p >= end) break;
            ts[nt] = p;
            while (p < end && !isspace((unsigned char)text[p])) p++;
            tl[nt] = p - ts[nt]; nt++;
        }
        char tok[256];
        int t = 0;
        for (; t < nt && text[ts[t]] == '@'; t++)
        {
            snprintf(tok, sizeof(tok), "%.*s", tl[t] - 1, text + ts[t] + 1);
            int li = lab_add(&labels, &lab_at, &lab_cap, tok);   // may move lab_at
            lab_at[li] = n;
        }
        if (t >= nt) continue;
        snprintf(tok, sizeof(tok), "%.*s", tl[t], text + ts[t]);
        if (tok[0] == '#') { if (strcmp(tok, "#ITRAD") == 0) itr = n; continue; }

        op  = grow(op,  &op_cap,  n, sizeof(int));
        tgl = grow(tgl, &tgl_cap, n, sizeof(int));
        op[n] = isa_find(tok); tgl[n] = -1;
        if (op[n] < 0) { why = "an instruction it does not know"; break; }
        if (strcmp(isa[op[n]].cls, "code") == 0 && t + 1 < nt)
        {
            snprintf(tok, sizeof(tok), "%.*s", tl[t + 1], text + ts[t + 1]);
            tgl[n] = lab_add(&labels, &lab_at, &lab_cap, tok);
        }
        n++;
    }

    dp_ctx c = {0};
    c.n = n;
    c.op = op;
    c.tgt = malloc((n + 1) * sizeof(int));
    c.entry = malloc((n + 1) * sizeof(int));
    for (int i = 0; i < n && !why; i++)
    {
        c.tgt[i] = tgl[i] >= 0 ? lab_at[tgl[i]] : -1;
        if (tgl[i] >= 0 && (c.tgt[i] < 0 || c.tgt[i] >= n)) why = "a jump to a label that is not there";
        c.entry[i] = -1;
    }
    int nr = 0;
    if (!why && n > 0)
    {
        c.entry[0] = nr++;
        if (itr >= 0 && itr < n && c.entry[itr] < 0) c.entry[itr] = nr++;
        for (int i = 0; i < n; i++)
            if (strcmp(isa[op[i]].flow, "call") == 0 && c.entry[c.tgt[i]] < 0) c.entry[c.tgt[i]] = nr++;
    }
    c.state = calloc(nr + 1, sizeof(int));
    c.net   = calloc(nr + 1, sizeof(int));
    c.maxd  = calloc(nr + 1, sizeof(int));
    c.maxc  = calloc(nr + 1, sizeof(int));
    c.why   = why;
    if (!c.why && n > 0)
    {
        dp_walk(&c, 0, 0);
        if (itr >= 0 && itr < n) dp_walk(&c, c.entry[itr], itr);
    }
    why = c.why;

    if (!why && n > 0)
    {
        int d = c.maxd[0], s = c.maxc[0];
        if (itr >= 0 && itr < n) { d += c.maxd[c.entry[itr]]; s += c.maxc[c.entry[itr]]; }  // on top of any point
        // one word more than the deepest point: the simulation's overflow flag
        // (core.v, stack) fires when the pointer reaches DEPTH; at least 2, the
        // stack's pointer has $clog2(DEPTH) bits
        *ddepth = d + 1 < 2 ? 2 : d + 1;
        *sdepth = s + 1 < 2 ? 2 : s + 1;
        ret = 1;
    }
    else if (why) printf("Info: stack depths not worked out from the program, the default stays: %s\n", why);

    free(c.tgt); free(c.entry); free(c.state); free(c.net); free(c.maxd); free(c.maxc);
    free(op); free(tgl); free(lab_at);
    st_free(&labels); st_free(&lines);
    return ret;
}
