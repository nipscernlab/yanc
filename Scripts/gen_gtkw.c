// ============================================================================
// gen_gtkw.c -- generate a pre-formatted GTKWave .gtkw save file from a VCD
// header, replacing the runtime Tcl formatter (gtk_proc_init.tcl /
// gtk_proj_init.tcl). Ports the proven logic of Aurora's js/wave/
// gtkw_proc_writer.js (the format flags below were calibrated against a real
// GTKWave save, not guessed).
//
// Handles single AND multiple processors: every VCD scope that owns both a
// `valr2` and a `linetabs` signal is a processor instance; each gets its own
// formatted section. Single-proc is just the N=1 case.
//
// Why a static .gtkw instead of --script: the nipscern GTKWave v4 ignores
// --script, so the formatting has to live in the save file. GTKWave opens it
// with:  gtkwave <vcd> -a <out.gtkw>
//
// Usage:
//   gen_gtkw <in.vcd> <out.gtkw> <base> [<comp2gtkw_exe>]
//     <base>          the folder holding each processor's output folder; the
//                     translate files are read from
//                     <base>/<proc_type>/trad_opcode.txt and src_lines.txt,
//                     or from <base>/ when it is the output folder itself
//     <comp2gtkw_exe> path to comp2gtkw.exe (process filter for complex signals)
//
// Built by the Makefile (`make gen_gtkw`), with exe_dir.c, yanc_lang.c and
// xstr.c. A name, a scope or a path has any length: each lives in a block of
// its own size (xstr.h), never in a char[N] (issue #66).
// ============================================================================

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <dirent.h>

#include "../Compilers/common/exe_dir.h"
#include "../Compilers/common/yanc_lang.h"
#include "../Compilers/common/xstr.h"

// every message a user sees, in Portuguese and in English (AGENTS.md, rule 4):
// English unless --lang or YANC_LANG says pt
#define M(pt, en) (yanc_lang_en ? (en) : (pt))

#define MSG_ERR_OPEN_VCD  M("gen_gtkw: não foi possível abrir o VCD '%s'\n", \
                            "gen_gtkw: cannot open VCD '%s'\n")
#define MSG_ERR_MEMORY    M("gen_gtkw: memória insuficiente\n", \
                            "gen_gtkw: out of memory\n")
#define MSG_ERR_WRITE     M("gen_gtkw: não foi possível escrever '%s'\n", \
                            "gen_gtkw: cannot write '%s'\n")
#define MSG_INFO_DONE     M("gen_gtkw: %d sinais, %d processador(es) -> %s\n", \
                            "gen_gtkw: %d signals, %d processor(s) -> %s\n")
#define MSG_WARN_NO_TYPE  M("gen_gtkw: não achei a pasta do processador de '%s' em '%s';\n" \
                            "  as faixas de instrução e de código ficam em números\n", \
                            "gen_gtkw: cannot find the processor folder of '%s' in '%s';\n" \
                            "  the instruction and source tracks stay as numbers\n")
#define MSG_CLI_USAGE \
    M("uso: gen_gtkw <entrada.vcd> <saida.gtkw> <base> [<comp2gtkw>]\n" \
      "  escreve a vista formatada do GTKWave (saida.gtkw) de um dump de simulação;\n" \
      "  <base> guarda a pasta de saída de cada processador, <base>/<proc>/,\n" \
      "  ou é a própria pasta de saída, com os arquivos de tradução\n" \
      "  (trad_opcode.txt, src_lines.txt);\n" \
      "  o comp2gtkw padrão é o que está ao lado do gen_gtkw\n" \
      "  --lang=en|pt: a língua das mensagens (também YANC_LANG); padrão en\n", \
      "usage: gen_gtkw <in.vcd> <out.gtkw> <base> [<comp2gtkw>]\n" \
      "  writes the formatted GTKWave view (out.gtkw) of a simulation dump;\n" \
      "  <base> holds the output folder of each processor, <base>/<proc>/,\n" \
      "  or is the output folder itself, with the translate files\n" \
      "  (trad_opcode.txt, src_lines.txt);\n" \
      "  comp2gtkw defaults to the one next to gen_gtkw\n" \
      "  --lang=en|pt: the language of the messages (also YANC_LANG); default en\n")

// ---- GTKWave trace-format flag bits (see Aurora gtkw_proc_writer.js) --------
#define TR_HEX        0x2
#define TR_DEC        0x4
#define TR_BIN        0x8
#define TR_RJUSTIFY   0x20
#define TR_BLANK      0x200
#define TR_SIGNED     0x400
#define TR_COLLAPSED  0x1000
#define TR_CLOSED     0x400000
#define TR_FTRANSLATED 0x2000   // file filter (translate table)
#define TR_PTRANSLATED 0x4000   // process filter (executable transducer)
#define TR_ANALOG_STEP 0x8000
#define TR_REAL       0x40000
#define TR_GRP_BEGIN  0x800000
#define TR_GRP_END    0x1000000

// format presets (calibrated against a real save -- do NOT add "obvious" bits)
#define FMT_BIN           (TR_RJUSTIFY | TR_BIN)                       // 0x28
#define FMT_DEC           (TR_RJUSTIFY | TR_DEC)                       // 0x24
#define FMT_SIGNED_DEC    (TR_RJUSTIFY | TR_SIGNED)                    // 0x420  (signed alone = decimal)
#define FMT_REAL          (TR_RJUSTIFY | TR_REAL)                      // 0x40020 (BitsToReal = real alone)
#define FMT_ANALOG_SIGNED (TR_RJUSTIFY | TR_SIGNED | TR_ANALOG_STEP)   // 0x8420
#define FMT_ANALOG_HEX    (TR_RJUSTIFY | TR_HEX | TR_ANALOG_STEP)      // 0x8022
#define FLAG_COMMENT      (TR_BLANK)                                            // 0x200
#define FLAG_GRP_BEGIN    (TR_BLANK | TR_GRP_BEGIN | TR_CLOSED)                 // 0xc00200
#define FLAG_GRP_END      (TR_BLANK | TR_GRP_END | TR_CLOSED | TR_COLLAPSED)    // 0x1401200

// colors (the index after `[color]`)
#define COL_NORMAL 0
#define COL_ORANGE 2
#define COL_YELLOW 3
#define COL_INDIGO 6
#define COL_VIOLET 7

// ---- signal model ----------------------------------------------------------
typedef struct {
    char *scope;       // hierarchical scope path, e.g. tb.proc
    char *name;        // leaf signal name
    int   width;
    char *range;       // e.g. "22:0" or "" if scalar
} Sig;

static Sig *g_sig = NULL;
static int  g_nsig = 0, g_capsig = 0;

// a detected processor instance
typedef struct {
    char *inst;        // scope owning valr2 + linetabs (mirrors live here)
    char *core;        // <inst>...p_<type>.core (clk/rst, sp/isp/ula live below), "" if none
    char *type;        // module type, for <tmp_base>/<type>/trad_*.txt
} Proc;
static Proc g_proc[32];
static int  g_nproc = 0;

static FILE *g_out = NULL;
static int   g_file_id = 0;   // running id for file filters  (^N)
static int   g_proc_id = 0;   // running id for process filters (^>N)

static const char *g_tmp_base = "";
static const char *g_comp2gtkw = "";

// ---- VCD header parsing ----------------------------------------------------
static char  *g_buf = NULL;
static size_t g_pos = 0, g_len = 0;

// the next token of the header, of any length (NULL at its end); the block is
// the caller's
static char *read_token(void) {
    while (g_pos < g_len && isspace((unsigned char)g_buf[g_pos])) g_pos++;
    if (g_pos >= g_len) return NULL;
    size_t s = g_pos;
    if (g_buf[g_pos] == '[') {
        size_t p = g_pos + 1;
        while (p < g_len && g_buf[p] != ']' && !isspace((unsigned char)g_buf[p])) p++;
        if (p < g_len && g_buf[p] == ']') p++;
        else p = g_pos + 1;
        g_pos = p;
        return xasprintf("%.*s", (int)(p - s), g_buf + s);
    }
    while (g_pos < g_len && !isspace((unsigned char)g_buf[g_pos])) g_pos++;
    return xasprintf("%.*s", (int)(g_pos - s), g_buf + s);
}

static char *g_stack[128];        // each a block of its own ("" for a scope that is no module)
static int   g_stack_mod[128];
static int   g_depth = 0;

static const char *nearest_module_path(void) {
    for (int i = g_depth - 1; i >= 0; i--) if (g_stack_mod[i]) return g_stack[i];
    return NULL;
}
static void skip_to_end(void) {
    char *t;
    while ((t = read_token()) != NULL) {
        int end = strcmp(t, "$end") == 0;
        free(t);
        if (end) return;
    }
}

static void parse_vcd_header(const char *path) {
    FILE *f = fopen(path, "rb");
    if (!f) { fprintf(stderr, MSG_ERR_OPEN_VCD, path); exit(2); }
    fseek(f, 0, SEEK_END); long sz = ftell(f); fseek(f, 0, SEEK_SET);
    // we only need the header; read in chunks and stop after $enddefinitions so a
    // multi-GB full-sim VCD doesn't get slurped whole.
    g_buf = (char *)malloc(sz + 1);
    if (!g_buf) { fprintf(stderr, MSG_ERR_MEMORY); exit(2); }
    g_len = fread(g_buf, 1, sz, f); g_buf[g_len] = 0; fclose(f);
    char *ed = strstr(g_buf, "$enddefinitions");
    if (ed) g_len = (size_t)(ed - g_buf);

    char *t;
    int   stop = 0;
    while (!stop && (t = read_token()) != NULL) {
        if (strcmp(t, "$scope") == 0) {
            char *stype = read_token(), *sname = read_token();
            if (!stype) stype = xstrdup("");
            if (!sname) sname = xstrdup("");
            skip_to_end();
            if (strcmp(stype, "module") == 0 && g_depth < 128) {
                const char *par = nearest_module_path();
                g_stack[g_depth] = par ? xasprintf("%s.%s", par, sname) : xstrdup(sname);
                g_stack_mod[g_depth] = 1; g_depth++;
            } else if (g_depth < 128) {
                g_stack[g_depth] = xstrdup(""); g_stack_mod[g_depth] = 0; g_depth++;
            }
            free(stype); free(sname);
        } else if (strcmp(t, "$upscope") == 0) {
            skip_to_end();
            if (g_depth > 0) free(g_stack[--g_depth]);
        } else if (strcmp(t, "$var") == 0) {
            char *vtype = read_token(), *vwidth = read_token(), *vid = read_token(), *vname = read_token();
            char *rng = xstrdup(""), *maybe = read_token();
            if (maybe) {
                if (maybe[0] == '[' && strcmp(maybe, "$end") != 0) {
                    size_t L = strlen(maybe);
                    if (L >= 2 && maybe[L-1] == ']') { free(rng); rng = xasprintf("%.*s", (int)(L - 2), maybe + 1); }
                    skip_to_end();
                } else if (strcmp(maybe, "$end") != 0) {
                    skip_to_end();
                }
            }
            if (g_depth > 0 && g_stack_mod[g_depth-1]) {
                if (g_nsig == g_capsig) {
                    g_capsig = g_capsig ? 2 * g_capsig : 1024;
                    g_sig = xrealloc(g_sig, (size_t)g_capsig * sizeof *g_sig);
                }
                Sig *s = &g_sig[g_nsig++];
                s->scope = xstrdup(g_stack[g_depth-1]);
                s->name  = xstrdup(vname ? vname : "");
                s->width = vwidth ? atoi(vwidth) : 0;
                s->range = rng; rng = NULL;
            }
            free(vtype); free(vwidth); free(vid); free(vname); free(maybe); free(rng);
        } else if (strcmp(t, "$enddefinitions") == 0) {
            stop = 1;
        }
        free(t);
    }
}

// ---- string helpers --------------------------------------------------------
static int starts_with(const char *s, const char *p) { return strncmp(s, p, strlen(p)) == 0; }
static int scope_ends_with(const char *scope, const char *suf) {
    size_t L = strlen(scope), M = strlen(suf);
    return L >= M && strcmp(scope + L - M, suf) == 0;
}
// is signal s at/under scope `sc` ?
static int under(const char *scope, const char *sc) {
    size_t L = strlen(sc);
    return strncmp(scope, sc, L) == 0 && (scope[L] == 0 || scope[L] == '.');
}

// extract func/var from a mangled name: ..._f_<func>_v_<var>_e_  (unanchored);
// *func and *var are blocks the caller frees (only when it returns 1)
static int split_func_var(const char *name, char **func, char **var) {
    const char *f = strstr(name, "_f_"); if (!f) return 0;
    const char *v = strstr(f + 3, "_v_"); if (!v) return 0;
    const char *e = strstr(v + 3, "_e_"); if (!e) return 0;
    *func = xasprintf("%.*s", (int)(v - (f + 3)), f + 3);
    *var  = xasprintf("%.*s", (int)(e - (v + 3)), v + 3);
    return 1;
}
// "global", or "<func>()" (the block is the caller's)
static char *func_label(const char *func) {
    return strcmp(func, "global") == 0 ? xstrdup("global") : xasprintf("%s()", func);
}

// ---- lookups ---------------------------------------------------------------
// find a signal named `name` whose scope == `scope`
static int find_sig(const char *scope, const char *name) {
    for (int i = 0; i < g_nsig; i++)
        if (strcmp(g_sig[i].name, name) == 0 && strcmp(g_sig[i].scope, scope) == 0) return i;
    return -1;
}
// find `name` under `inst` whose scope ends with `suffix` (e.g. ".sp"/".isp"/".ula")
static int find_under_suffix(const char *inst, const char *suffix, const char *name) {
    for (int i = 0; i < g_nsig; i++)
        if (strcmp(g_sig[i].name, name) == 0 &&
            under(g_sig[i].scope, inst) && scope_ends_with(g_sig[i].scope, suffix)) return i;
    return -1;
}

// ---- .gtkw emission --------------------------------------------------------
static void emit_signal(const Sig *s, unsigned flag, int color,
                        const char *alias, const char *fileFilter, const char *procFilter) {
    if (fileFilter) flag |= TR_FTRANSLATED;
    if (procFilter) flag |= TR_PTRANSLATED;
    fprintf(g_out, "@%x\n", flag);
    if (color != COL_NORMAL) fprintf(g_out, "[color] %d\n", color);
    if (fileFilter) fprintf(g_out, "^%d %s\n", ++g_file_id, fileFilter);
    if (procFilter) fprintf(g_out, "^>%d %s\n", ++g_proc_id, procFilter);
    const char *ob = s->range[0] ? "[" : "", *cb = s->range[0] ? "]" : "";
    if (alias && alias[0]) fprintf(g_out, "+{%s} %s.%s%s%s%s\n", alias, s->scope, s->name, ob, s->range, cb);
    else                   fprintf(g_out, "%s.%s%s%s%s\n", s->scope, s->name, ob, s->range, cb);
}
static void emit_comment(const char *text)     { fprintf(g_out, "@%x\n-%s\n", FLAG_COMMENT,   text); }
static void emit_group_begin(const char *name) { fprintf(g_out, "@%x\n-%s\n", FLAG_GRP_BEGIN, name); }
static void emit_group_end(const char *name)   { fprintf(g_out, "@%x\n-%s\n", FLAG_GRP_END,   name); }

// numbered I/O pair list within `inst`: <prefix>N -> alias "<label> i"
static void emit_io(const char *inst, const char *prefix, unsigned flag, const char *label) {
    int idx[256], n = 0;
    for (int i = 0; i < g_nsig; i++) {
        if (strcmp(g_sig[i].scope, inst) != 0) continue;
        if (starts_with(g_sig[i].name, prefix) && n < 256) idx[n++] = i;
    }
    for (int a = 1; a < n; a++) {  // insertion sort by numeric suffix
        int ia = idx[a], ja = a - 1;
        long va = atol(g_sig[ia].name + strlen(prefix));
        while (ja >= 0 && atol(g_sig[idx[ja]].name + strlen(prefix)) > va) { idx[ja+1] = idx[ja]; ja--; }
        idx[ja+1] = ia;
    }
    for (int a = 0; a < n; a++) {
        char al[64]; snprintf(al, sizeof al, "%s %d", label, a);
        emit_signal(&g_sig[idx[a]], flag, COL_YELLOW, al, NULL, NULL);
    }
}

// scalar typed vars (me1_/me2_/comp_me3_) within `inst`
static void emit_typed_vars(const char *inst, const char *prefix, unsigned flag,
                            const char *tlabel, const char *procFilter) {
    for (int i = 0; i < g_nsig; i++) {
        if (strcmp(g_sig[i].scope, inst) != 0) continue;
        if (!starts_with(g_sig[i].name, prefix)) continue;
        char *func, *var;
        if (!split_func_var(g_sig[i].name, &func, &var)) continue;
        char *fl = func_label(func);
        char *alias = xasprintf("%s %s in %s", tlabel, var, fl);
        emit_signal(&g_sig[i], flag, COL_ORANGE, alias, NULL, procFilter);
        free(func); free(var); free(fl); free(alias);
    }
}

// a name that ends in 4 digits: *base_len is the length before them
static int has_4digit_suffix(const char *nm, size_t *base_len, int *idx) {
    size_t L = strlen(nm);
    if (L < 4) return 0;
    for (int k = 0; k < 4; k++) if (!isdigit((unsigned char)nm[L-1-k])) return 0;
    *base_len = L - 4;
    *idx = atoi(nm + L - 4);
    return 1;
}

// array vars (arr_me1_/arr_me2_/comp_arr_me3_) within `inst`, grouped by base
static void emit_typed_arrays(const char *inst, const char *prefix, unsigned flag,
                              const char *tlabel, const char *procFilter) {
    char **seen = NULL; int nseen = 0;
    for (int i = 0; i < g_nsig; i++) {
        if (strcmp(g_sig[i].scope, inst) != 0) continue;
        if (!starts_with(g_sig[i].name, prefix)) continue;
        size_t bl; int dummy;
        if (!has_4digit_suffix(g_sig[i].name, &bl, &dummy)) continue;
        char *base = xasprintf("%.*s", (int)bl, g_sig[i].name);
        int known = 0; for (int k = 0; k < nseen; k++) if (strcmp(seen[k], base) == 0) { known = 1; break; }
        if (known) { free(base); continue; }
        seen = xrealloc(seen, (size_t)(nseen + 1) * sizeof *seen);
        seen[nseen++] = base;

        char *func, *var, *glabel, *vv;
        if (split_func_var(base, &func, &var)) {
            char *fl = func_label(func);
            glabel = xasprintf("%s %s in %s", tlabel, var, fl);
            vv = var;
            free(func); free(fl);
        }
        else { glabel = xasprintf("%s %s", tlabel, base); vv = xstrdup(base); }
        emit_group_begin(glabel);

        int mi[8192], mn = 0;
        for (int j = 0; j < g_nsig; j++) {
            if (strcmp(g_sig[j].scope, inst) != 0) continue;
            if (!starts_with(g_sig[j].name, prefix)) continue;
            size_t b2; int ix;
            if (!has_4digit_suffix(g_sig[j].name, &b2, &ix)) continue;
            if (b2 != bl || strncmp(g_sig[j].name, base, bl) != 0) continue;
            if (mn < 8192) mi[mn++] = j;
        }
        for (int a = 1; a < mn; a++) {  // sort by suffix index
            int ia = mi[a], ja = a - 1; size_t tb; int va; has_4digit_suffix(g_sig[ia].name, &tb, &va);
            size_t tb2; int vb;
            while (ja >= 0 && (has_4digit_suffix(g_sig[mi[ja]].name, &tb2, &vb), vb) > va) { mi[ja+1] = mi[ja]; ja--; }
            mi[ja+1] = ia;
        }
        for (int a = 0; a < mn; a++) {
            char *al = xasprintf("%s %d", vv, a);
            emit_signal(&g_sig[mi[a]], flag, COL_ORANGE, al, NULL, procFilter);
            free(al);
        }
        emit_group_end(glabel);
        free(glabel); free(vv);
    }
    for (int k = 0; k < nseen; k++) free(seen[k]);
    free(seen);
}

// emit a flag signal if present, matched under `inst` by sub-scope suffix
static void emit_flag(const char *inst, const char *suffix, const char *name, unsigned flag, const char *alias) {
    int i = find_under_suffix(inst, suffix, name);
    if (i >= 0) emit_signal(&g_sig[i], flag, COL_NORMAL, alias, NULL, NULL);
}

static void emit_proc_section(const Proc *p) {
    const char *instname = strrchr(p->inst, '.'); instname = instname ? instname + 1 : p->inst;
    char *banner = xasprintf("###### %s", instname);
    emit_comment(banner);
    free(banner);

    // clk / rst / itr -- prefer the proc's core (project case: wrapper drives
    // core.clk); fall back to the tb top (single-proc generated tb: clk/rst are
    // testbench regs at the root scope).
    char *tbroot = xstrdup(p->inst);
    char *dot = strchr(tbroot, '.'); if (dot) *dot = 0;
    const char *cs = p->core[0] ? p->core : p->inst;
    int ci;
    if ((ci = find_sig(cs, "clk")) < 0) ci = find_sig(tbroot, "clk");
    if (ci >= 0) emit_signal(&g_sig[ci], FMT_BIN, COL_NORMAL, NULL, NULL, NULL);
    if ((ci = find_sig(cs, "rst")) < 0) ci = find_sig(tbroot, "rst");
    if (ci >= 0) emit_signal(&g_sig[ci], FMT_BIN, COL_NORMAL, NULL, NULL, NULL);
    if ((ci = find_sig(cs, "itr")) < 0) ci = find_sig(tbroot, "itr");
    if (ci >= 0) emit_signal(&g_sig[ci], FMT_BIN, COL_NORMAL, NULL, NULL, NULL);
    free(tbroot);

    emit_comment("I/O ****************");
    emit_io(p->inst, "req_in_sim_", FMT_BIN,        "req_in");
    emit_io(p->inst, "in_sim_",     FMT_SIGNED_DEC, "input ");
    emit_io(p->inst, "out_en_sim_", FMT_BIN,        "out_en");
    emit_io(p->inst, "out_sig_",    FMT_SIGNED_DEC, "output");

    // per-proc translate files in each processor's output folder: <base>/<type>/
    // when the folder is named after the processor (the default ./<name>),
    // else <base> itself, when it is the output folder (yanc -o, an .asm in a
    // folder of another name). Forward slashes work on both Windows and
    // POSIX, and GTKWave accepts them in the .gtkw paths. A file that is not
    // there (src_lines.txt of an .asm written by hand) is left out.
    char *tradop = xstrdup(""), *tradcmm = xstrdup("");
    char *lang = xstrdup("C+-");  // the source track's name: prog_log.txt's `lang`
    if (p->type[0]) {
        char *dir = xasprintf("%s/%s", g_tmp_base, p->type);
        free(tradop); tradop = xasprintf("%s/trad_opcode.txt", dir);
        FILE *t = fopen(tradop, "r");
        if (!t) {
            char *own = xasprintf("%s/trad_opcode.txt", g_tmp_base);
            if ((t = fopen(own, "r")) != NULL) { free(dir); dir = xstrdup(g_tmp_base); }
            free(own);
        }
        if (t) fclose(t);
        free(tradop);  tradop  = xasprintf("%s/trad_opcode.txt", dir);
        free(tradcmm); tradcmm = xasprintf("%s/src_lines.txt",    dir);
        FILE *s = fopen(tradcmm, "r");
        if (s) fclose(s); else tradcmm[0] = 0;
        char *logp = xasprintf("%s/prog_log.txt", dir);
        FILE *lf = fopen(logp, "r");
        free(logp); free(dir);
        if (lf) {
            char *ln;
            while ((ln = xgetline(lf)) != NULL) {
                int is_lang = strncmp(ln, "lang", 4) == 0 && (ln[4] == ' ' || ln[4] == '\t');
                char *v = ln + 4;
                while (is_lang && (*v == ' ' || *v == '\t')) v++;
                size_t n = strcspn(v, " \t");
                if (is_lang && n > 0) { free(lang); lang = xasprintf("%.*s", (int)n, v); }
                free(ln);
                if (is_lang && n > 0) break;
            }
            fclose(lf);
        }
    }
    emit_comment("Instructions *******");
    int vi = find_sig(p->inst, "valr2");
    if (vi >= 0) emit_signal(&g_sig[vi], FMT_DEC, COL_INDIGO, "Assembly", tradop[0]?tradop:NULL, NULL);
    int li = find_sig(p->inst, "linetabs");
    if (li >= 0) emit_signal(&g_sig[li], FMT_SIGNED_DEC, COL_VIOLET, lang, tradcmm[0]?tradcmm:NULL, NULL);
    free(tradop); free(tradcmm); free(lang);

    emit_comment("Variables **********");
    emit_typed_vars(p->inst, "me1_",      FMT_SIGNED_DEC, "int",   NULL);
    emit_typed_vars(p->inst, "me2_",      FMT_REAL,       "float", NULL);
    emit_typed_vars(p->inst, "comp_me3_", FMT_BIN,        "comp",  g_comp2gtkw);
    emit_typed_arrays(p->inst, "arr_me1_",      FMT_SIGNED_DEC, "int",   NULL);
    emit_typed_arrays(p->inst, "arr_me2_",      FMT_REAL,       "float", NULL);
    emit_typed_arrays(p->inst, "comp_arr_me3_", FMT_BIN,        "comp",  g_comp2gtkw);

    // flags (Stack / ALU) live deep under core; matched by sub-scope suffix.
    // Absent under Verilator (fenced out) -- that's fine.
    int any_stack = find_under_suffix(p->inst, ".sp", "pointeri") >= 0 ||
                    find_under_suffix(p->inst, ".isp", "pointeri") >= 0;
    int any_ula   = find_under_suffix(p->inst, ".ula", "delta_int") >= 0 ||
                    find_under_suffix(p->inst, ".ula", "delta_float") >= 0;
    if (any_stack || any_ula) emit_comment("Flags **************");
    if (any_stack) {
        emit_group_begin("Stack");
        // pointeri is the words the stack holds, fl_top 1 while it is full
        // (a program at its deepest point), fl_overflow 1 from the first push
        // past full or pop of an empty stack (core.v, stack; #145)
        emit_flag(p->inst, ".sp",  "pointeri", FMT_ANALOG_SIGNED, "Data Stack Words");
        emit_flag(p->inst, ".sp",  "fl_max",   FMT_DEC,           "Data Stack Max");
        emit_flag(p->inst, ".sp",  "fl_top",   FMT_BIN,           "Data Stack Full");
        emit_flag(p->inst, ".sp",  "fl_overflow",  FMT_BIN,           "Data Stack Overflow");
        emit_flag(p->inst, ".isp", "pointeri", FMT_ANALOG_SIGNED, "Inst Stack Words");
        emit_flag(p->inst, ".isp", "fl_max",   FMT_DEC,           "Inst Stack Max");
        emit_flag(p->inst, ".isp", "fl_top",   FMT_BIN,           "Inst Stack Full");
        emit_flag(p->inst, ".isp", "fl_overflow",  FMT_BIN,           "Inst Stack Overflow");
        emit_group_end("Stack");
    }
    if (any_ula) {
        emit_group_begin("ALU");
        emit_flag(p->inst, ".ula", "delta_int",   FMT_ANALOG_HEX, "Rounding Error (int)");
        emit_flag(p->inst, ".ula", "delta_float", FMT_ANALOG_HEX, "Rounding Error (float)");
        emit_group_end("ALU");
    }
}

// ---- processor detection ---------------------------------------------------
// whether <base>/<dir>/trad_opcode.txt (or <base>/trad_opcode.txt, dir NULL) exists
static int has_trad(const char *dir) {
    char *path = dir ? xasprintf("%s/%s/trad_opcode.txt", g_tmp_base, dir)
                     : xasprintf("%s/trad_opcode.txt", g_tmp_base);
    FILE *f = fopen(path, "r");
    free(path);
    if (!f) return 0;
    fclose(f);
    return 1;
}

// a whole file as one string, or NULL; the block is the caller's
static char *slurp(const char *path) {
    FILE *f = fopen(path, "rb");
    if (!f) return NULL;
    fseek(f, 0, SEEK_END); long sz = ftell(f); fseek(f, 0, SEEK_SET);
    if (sz < 0) { fclose(f); return NULL; }
    char *b = (char *)malloc((size_t)sz + 1);
    if (!b) { fprintf(stderr, MSG_ERR_MEMORY); exit(2); }
    size_t n = fread(b, 1, (size_t)sz, f); b[n] = 0; fclose(f);
    return b;
}

static int is_mirror(const char *name) {
    static const char *pre[] = { "me1_", "me2_", "comp_me3_", "arr_me1_", "arr_me2_", "comp_arr_me3_" };
    for (size_t k = 0; k < sizeof pre / sizeof pre[0]; k++)
        if (strncmp(name, pre[k], strlen(pre[k])) == 0) return 1;
    return 0;
}

// The folder under <base> that holds this instance's processor: it has a
// trad_opcode.txt and a <dir>.v that declares every variable mirror the dump
// shows at <inst>. Only a single match counts; with no mirrors, only a single
// processor folder. NULL if none or several (the block is the caller's).
static char *type_by_mirrors(const char *inst) {
    DIR *d = opendir(g_tmp_base);
    if (!d) return NULL;
    char *found = NULL;
    int matches = 0;
    struct dirent *e;
    while ((e = readdir(d)) != NULL) {
        if (e->d_name[0] == '.' || !has_trad(e->d_name)) continue;
        char *vpath = xasprintf("%s/%s/%s.v", g_tmp_base, e->d_name, e->d_name);
        char *text = slurp(vpath);
        free(vpath);
        if (!text) continue;
        int ok = 1;
        for (int i = 0; i < g_nsig && ok; i++)
            if (strcmp(g_sig[i].scope, inst) == 0 && is_mirror(g_sig[i].name) &&
                !strstr(text, g_sig[i].name)) ok = 0;
        free(text);
        if (!ok) continue;
        matches++;
        free(found);
        found = xstrdup(e->d_name);
    }
    closedir(d);
    if (matches == 1) return found;
    free(found);
    return NULL;
}

static void derive_core_and_type(Proc *p) {
    // core = the ".core" scope under inst. Single-proc tbs dump no signal AT the
    // core scope itself (only core.sp/core.ula/...), so derive the core path by
    // truncating any under-inst signal scope that contains ".core" at that point.
    p->core = xstrdup(""); p->type = NULL;
    for (int i = 0; i < g_nsig; i++) {
        const char *sc = g_sig[i].scope;
        if (!under(sc, p->inst)) continue;
        const char *c = strstr(sc, ".core");
        if (c && (c[5] == '.' || c[5] == 0)) {
            size_t len = (size_t)(c - sc) + 5;              // include ".core"
            free(p->core); p->core = xasprintf("%.*s", (int)len, sc);
            break;
        }
    }
    // type from the "p_<type>.core" segment of the core path
    if (p->core[0]) {
        size_t L = strlen(p->core);
        const char *seg_end = p->core + L - 5;            // points at ".core"
        const char *seg_start = seg_end;
        while (seg_start > p->core && *(seg_start - 1) != '.') seg_start--;
        size_t n = (size_t)(seg_end - seg_start);
        if (n >= 2 && strncmp(seg_start, "p_", 2) == 0) p->type = xasprintf("%.*s", (int)(n - 2), seg_start + 2);
        else                                            p->type = xasprintf("%.*s", (int)n, seg_start);
    } else {
        // fall back: instance name with a trailing _inst stripped
        const char *nm = strrchr(p->inst, '.'); nm = nm ? nm + 1 : p->inst;
        p->type = xstrdup(nm);
        char *u = strstr(p->type, "_inst"); if (u && u[5] == 0) *u = 0;
        // Under Verilator the processor's inside is not traced (tracing_off), so
        // there is no p_<type>.core and the instance name is all there is. In a
        // project it often differs from the processor (DTWv4_inst of ProcDTW),
        // so when neither <base>/<name>/ nor <base>/ holds the translate files,
        // look for the folder whose <dir>.v declares this instance's mirrors.
        if (!has_trad(p->type) && !has_trad(NULL)) {
            char *t = type_by_mirrors(p->inst);
            if (t) { free(p->type); p->type = t; }
            else fprintf(stderr, MSG_WARN_NO_TYPE, p->inst, g_tmp_base);
        }
    }
}

static void detect_procs(void) {
    for (int i = 0; i < g_nsig && g_nproc < 32; i++) {
        if (strcmp(g_sig[i].name, "valr2") != 0) continue;
        if (find_sig(g_sig[i].scope, "linetabs") < 0) continue;
        int dup = 0;
        for (int k = 0; k < g_nproc; k++) if (strcmp(g_proc[k].inst, g_sig[i].scope) == 0) { dup = 1; break; }
        if (dup) continue;
        g_proc[g_nproc].inst = xstrdup(g_sig[i].scope);
        derive_core_and_type(&g_proc[g_nproc]);
        g_nproc++;
    }
}

// ---- main ------------------------------------------------------------------
int main(int argc, char **argv) {
    yanc_lang(&argc, argv, "gen_gtkw");   // --lang and YANC_LANG: the messages' language
    int help = argc >= 2 && (!strcmp(argv[1], "-h") || !strcmp(argv[1], "--help"));
    if (help || argc < 4 || argc > 5) {
        fprintf(help ? stdout : stderr, MSG_CLI_USAGE);
        return help ? 0 : 1;
    }
    const char *vcd = argv[1], *out = argv[2];
    g_tmp_base = argv[3];
#ifdef _WIN32
    char *c2g = xasprintf("%s/comp2gtkw.exe", exe_dir());
#else
    char *c2g = xasprintf("%s/comp2gtkw", exe_dir());
#endif
    g_comp2gtkw = (argc == 5) ? argv[4] : c2g;

    parse_vcd_header(vcd);
    detect_procs();

    g_out = fopen(out, "wb");
    if (!g_out) { fprintf(stderr, MSG_ERR_WRITE, out); return 2; }

    fprintf(g_out, "[*]\n[*] Generated by gen_gtkw (yanc)\n[*]\n");
    fprintf(g_out, "[dumpfile] \"%s\"\n", vcd);
    fprintf(g_out, "[savefile] \"%s\"\n", out);
    fprintf(g_out, "[timestart] 0\n");

    for (int i = 0; i < g_nproc; i++) emit_proc_section(&g_proc[i]);

    fclose(g_out);
    fprintf(stderr, MSG_INFO_DONE, g_nsig, g_nproc, out);
    for (int i = 0; i < g_nproc; i++)
        fprintf(stderr, "  proc[%d] inst=%s  type=%s\n", i, g_proc[i].inst, g_proc[i].type);
    return 0;
}
