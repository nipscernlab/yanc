// asm_share -- scalars whose lives never overlap share one data word. See asm_share.h.

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <ctype.h>
#include "asm_share.h"

// ---- what each mnemonic does to its operand and to the flow -----------------
// A copy of four columns of isa.tsv (mnemonic, operand class, effect on the
// word the operand names, flow); Scripts/check_isa.py holds it to the table.

typedef struct { const char *mn, *cls, *opnd, *flow; } as_isa;

static const as_isa isa[] = {
    {"LOD", "data", "r", "-"},
    {"P_LOD", "data", "r", "-"},
    {"LDI", "data", "base_r", "-"},
    {"ILI", "data", "base_r", "-"},
    {"SET", "data", "w", "-"},
    {"SET_P", "data", "w", "-"},
    {"STI", "data", "base_w", "-"},
    {"ISI", "data", "base_w", "-"},
    {"PSH", "none", "-", "-"},
    {"POP", "none", "-", "-"},
    {"INN", "in", "-", "-"},
    {"F_INN", "in", "-", "-"},
    {"P_INN", "in", "-", "-"},
    {"PF_INN", "in", "-", "-"},
    {"OUT", "out", "-", "-"},
    {"JMP", "code", "-", "jmp"},
    {"JIZ", "code", "-", "jz"},
    {"CAL", "code", "-", "call"},
    {"RET", "none", "-", "ret"},
    {"ADD", "data", "r", "-"},
    {"S_ADD", "none", "-", "-"},
    {"F_ADD", "data", "r", "-"},
    {"SF_ADD", "none", "-", "-"},
    {"MLT", "data", "r", "-"},
    {"S_MLT", "none", "-", "-"},
    {"F_MLT", "data", "r", "-"},
    {"SF_MLT", "none", "-", "-"},
    {"DIV", "data", "r", "-"},
    {"S_DIV", "none", "-", "-"},
    {"F_DIV", "data", "r", "-"},
    {"SF_DIV", "none", "-", "-"},
    {"MOD", "data", "r", "-"},
    {"S_MOD", "none", "-", "-"},
    {"SGN", "data", "r", "-"},
    {"S_SGN", "none", "-", "-"},
    {"F_SGN", "data", "r", "-"},
    {"SF_SGN", "none", "-", "-"},
    {"NEG", "none", "-", "-"},
    {"NEG_M", "data", "r", "-"},
    {"P_NEG_M", "data", "r", "-"},
    {"F_NEG", "none", "-", "-"},
    {"F_NEG_M", "data", "r", "-"},
    {"PF_NEG_M", "data", "r", "-"},
    {"ABS", "none", "-", "-"},
    {"ABS_M", "data", "r", "-"},
    {"P_ABS_M", "data", "r", "-"},
    {"F_ABS", "none", "-", "-"},
    {"F_ABS_M", "data", "r", "-"},
    {"PF_ABS_M", "data", "r", "-"},
    {"PST", "none", "-", "-"},
    {"PST_M", "data", "r", "-"},
    {"P_PST_M", "data", "r", "-"},
    {"F_PST", "none", "-", "-"},
    {"F_PST_M", "data", "r", "-"},
    {"PF_PST_M", "data", "r", "-"},
    {"NRM", "none", "-", "-"},
    {"NRM_M", "data", "r", "-"},
    {"P_NRM_M", "data", "r", "-"},
    {"I2F", "none", "-", "-"},
    {"I2F_M", "data", "r", "-"},
    {"P_I2F_M", "data", "r", "-"},
    {"F2I", "none", "-", "-"},
    {"F2I_M", "data", "r", "-"},
    {"P_F2I_M", "data", "r", "-"},
    {"AND", "data", "r", "-"},
    {"S_AND", "none", "-", "-"},
    {"ORR", "data", "r", "-"},
    {"S_ORR", "none", "-", "-"},
    {"XOR", "data", "r", "-"},
    {"S_XOR", "none", "-", "-"},
    {"INV", "none", "-", "-"},
    {"INV_M", "data", "r", "-"},
    {"P_INV_M", "data", "r", "-"},
    {"LAN", "data", "r", "-"},
    {"S_LAN", "none", "-", "-"},
    {"LOR", "data", "r", "-"},
    {"S_LOR", "none", "-", "-"},
    {"LIN", "none", "-", "-"},
    {"LIN_M", "data", "r", "-"},
    {"P_LIN_M", "data", "r", "-"},
    {"LES", "data", "r", "-"},
    {"S_LES", "none", "-", "-"},
    {"F_LES", "data", "r", "-"},
    {"SF_LES", "none", "-", "-"},
    {"GRE", "data", "r", "-"},
    {"S_GRE", "none", "-", "-"},
    {"F_GRE", "data", "r", "-"},
    {"SF_GRE", "none", "-", "-"},
    {"EQU", "data", "r", "-"},
    {"S_EQU", "none", "-", "-"},
    {"SHL", "data", "r", "-"},
    {"S_SHL", "none", "-", "-"},
    {"SHR", "data", "r", "-"},
    {"S_SHR", "none", "-", "-"},
    {"SRS", "data", "r", "-"},
    {"S_SRS", "none", "-", "-"},
    {"NOP", "none", "-", "-"},
    {"F_ROT", "none", "-", "-"},
    {"F_SU1", "data", "r", "-"},
    {"F_SU2", "data", "r", "-"},
    {"SF_SU1", "none", "-", "-"},
    {"SF_SU2", "none", "-", "-"},
    {"F_SCL", "data", "r", "-"},
    {"SF_SCL", "none", "-", "-"},
    {"XPO", "none", "-", "-"},
    {"XPO_M", "data", "r", "-"},
    {"LEA", "lea", "addr", "-"},
    {"LOD_V", "offset", "r", "-"},
    {"P_LOD_V", "offset", "r", "-"},
    {"SET_V", "offset", "w", "-"},
    {"ADD_V", "offset", "r", "-"},
    {"F_ADD_V", "offset", "r", "-"},
    {"MLT_V", "offset", "r", "-"},
    {"F_MLT_V", "offset", "r", "-"},
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
