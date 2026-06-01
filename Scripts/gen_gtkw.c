// ============================================================================
// gen_gtkw.c -- generate a pre-formatted GTKWave .gtkw save file from a VCD
// header, replacing the runtime Tcl formatter (gtk_proc_init.tcl). Phase A:
// single processor. Ports the proven logic of Aurora's js/wave/gtkw_proc_writer.js
// (the format flags below were calibrated against a real GTKWave save, not guessed).
//
// Why a static .gtkw instead of --script: the nipscern GTKWave v4 ignores
// --script, so the formatting has to live in the save file. GTKWave opens it
// with:  gtkwave <vcd> -a <out.gtkw>
//
// Usage:
//   gen_gtkw <in.vcd> <out.gtkw> <trad_dir> <comp2gtkw_exe>
//     <in.vcd>        a VCD whose header lists the signals (full sim or header-only)
//     <trad_dir>      folder holding trad_opcode.txt and trad_cmm.txt for this proc
//     <comp2gtkw_exe> path to comp2gtkw.exe (process filter for complex signals)
//
// Build (sibling of comp2gtkw.c):  gcc -O2 -o gen_gtkw.exe gen_gtkw.c
// ============================================================================

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

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
#define FLAG_COMMENT      (TR_BLANK)                                   // 0x200
#define FLAG_GRP_BEGIN    (TR_BLANK | TR_GRP_BEGIN | TR_CLOSED)               // 0xc00200
#define FLAG_GRP_END      (TR_BLANK | TR_GRP_END | TR_CLOSED | TR_COLLAPSED)  // 0x1401200

// colors (the index after `[color]`)
#define COL_NORMAL 0
#define COL_ORANGE 2
#define COL_YELLOW 3
#define COL_INDIGO 6
#define COL_VIOLET 7

// ---- signal model ----------------------------------------------------------
typedef struct {
    char scope[600];   // hierarchical scope path, e.g. proc_fft_tb.proc
    char name[256];    // leaf signal name
    int  width;
    char range[64];    // e.g. "22:0" or "" if scalar
} Sig;

static Sig   g_sig[40000];
static int   g_nsig = 0;

static FILE *g_out = NULL;
static int   g_file_id = 0;   // running id for file filters  (^N)
static int   g_proc_id = 0;   // running id for process filters (^>N)

// args
static const char *g_trad_dir = "";
static const char *g_comp2gtkw = "";

// ---- VCD header parsing ----------------------------------------------------
// Tokenizer mirroring Aurora's /\[[^\]\s]+\]|\S+/g: a [..] range (no inner
// whitespace) is one token; otherwise a run of non-whitespace. The bracket rule
// matters because iverilog uses '[' as a $var id symbol.

static char *g_buf = NULL;
static size_t g_pos = 0, g_len = 0;

static int read_token(char *out, size_t cap) {
    while (g_pos < g_len && isspace((unsigned char)g_buf[g_pos])) g_pos++;
    if (g_pos >= g_len) return 0;
    size_t s = g_pos;
    if (g_buf[g_pos] == '[') {
        // [^\]\s]+]
        size_t p = g_pos + 1;
        while (p < g_len && g_buf[p] != ']' && !isspace((unsigned char)g_buf[p])) p++;
        if (p < g_len && g_buf[p] == ']') { p++; }           // include ']'
        else { p = g_pos + 1; }                               // lone '[' -> single char token
        size_t n = p - s; if (n >= cap) n = cap - 1;
        memcpy(out, g_buf + s, n); out[n] = 0; g_pos = p; return 1;
    }
    while (g_pos < g_len && !isspace((unsigned char)g_buf[g_pos])) g_pos++;
    size_t n = g_pos - s; if (n >= cap) n = cap - 1;
    memcpy(out, g_buf + s, n); out[n] = 0; return 1;
}

// scope stack: each entry is a module-scope path, or empty string for a
// non-module scope (task/function/fork) -- a placeholder that drops its vars.
static char  g_stack[64][600];
static int   g_stack_mod[64];   // 1 = real module scope, 0 = placeholder
static int   g_depth = 0;

static const char *nearest_module_path(void) {
    for (int i = g_depth - 1; i >= 0; i--) if (g_stack_mod[i]) return g_stack[i];
    return NULL;
}

static void skip_to_end(void) {
    char t[700];
    while (read_token(t, sizeof t)) if (strcmp(t, "$end") == 0) return;
}

static void parse_vcd_header(const char *path) {
    FILE *f = fopen(path, "rb");
    if (!f) { fprintf(stderr, "gen_gtkw: cannot open VCD '%s'\n", path); exit(2); }
    fseek(f, 0, SEEK_END); long sz = ftell(f); fseek(f, 0, SEEK_SET);
    g_buf = (char *)malloc(sz + 1);
    g_len = fread(g_buf, 1, sz, f); g_buf[g_len] = 0; fclose(f);
    // only parse up to $enddefinitions
    char *ed = strstr(g_buf, "$enddefinitions");
    if (ed) g_len = (size_t)(ed - g_buf);

    char t[700];
    while (read_token(t, sizeof t)) {
        if (strcmp(t, "$scope") == 0) {
            char stype[64] = "", sname[256] = "";
            read_token(stype, sizeof stype);
            read_token(sname, sizeof sname);
            skip_to_end();
            if (strcmp(stype, "module") == 0 && g_depth < 64) {
                const char *par = nearest_module_path();
                if (par) snprintf(g_stack[g_depth], 600, "%s.%s", par, sname);
                else     snprintf(g_stack[g_depth], 600, "%s", sname);
                g_stack_mod[g_depth] = 1; g_depth++;
            } else if (g_depth < 64) {
                g_stack[g_depth][0] = 0; g_stack_mod[g_depth] = 0; g_depth++;
            }
        } else if (strcmp(t, "$upscope") == 0) {
            skip_to_end();
            if (g_depth > 0) g_depth--;
        } else if (strcmp(t, "$var") == 0) {
            char vtype[32], vwidth[16], vid[64], vname[256], maybe[64];
            read_token(vtype, sizeof vtype);
            read_token(vwidth, sizeof vwidth);
            read_token(vid, sizeof vid);
            read_token(vname, sizeof vname);
            char rng[64] = "";
            // optional [range] before $end
            if (read_token(maybe, sizeof maybe)) {
                if (maybe[0] == '[' && strcmp(maybe, "$end") != 0) {
                    size_t L = strlen(maybe);
                    if (L >= 2 && maybe[L-1] == ']') { memcpy(rng, maybe+1, L-2); rng[L-2] = 0; }
                    skip_to_end();
                } else if (strcmp(maybe, "$end") != 0) {
                    skip_to_end();
                }
            }
            // attribute to the immediate enclosing module scope; drop otherwise
            if (g_depth > 0 && g_stack_mod[g_depth-1] && g_nsig < (int)(sizeof g_sig / sizeof g_sig[0])) {
                Sig *s = &g_sig[g_nsig++];
                snprintf(s->scope, sizeof s->scope, "%s", g_stack[g_depth-1]);
                snprintf(s->name, sizeof s->name, "%s", vname);
                s->width = atoi(vwidth);
                snprintf(s->range, sizeof s->range, "%s", rng);
            }
        } else if (strcmp(t, "$enddefinitions") == 0) {
            break;
        }
    }
}

// ---- helpers ---------------------------------------------------------------

// extract func/var from a mangled name: ..._f_<func>_v_<var>_e_  (unanchored)
static int split_func_var(const char *name, char *func, char *var) {
    const char *f = strstr(name, "_f_"); if (!f) return 0;
    const char *v = strstr(f + 3, "_v_"); if (!v) return 0;
    const char *e = strstr(v + 3, "_e_"); if (!e) return 0;
    size_t fl = (size_t)(v - (f + 3)); memcpy(func, f + 3, fl); func[fl] = 0;
    size_t vl = (size_t)(e - (v + 3)); memcpy(var, v + 3, vl); var[vl] = 0;
    return 1;
}
// "global" stays bare, otherwise append "()"
static void func_label(const char *func, char *out) {
    if (strcmp(func, "global") == 0) strcpy(out, "global");
    else snprintf(out, 256, "%s()", func);
}

static void rng_suffix(const Sig *s, char *out) {
    if (s->range[0]) snprintf(out, 80, "[%s]", s->range);
    else out[0] = 0;
}

// emit one signal line. fileFilter / procFilter are paths or NULL.
static void emit_signal(const Sig *s, unsigned flag, int color,
                        const char *alias, const char *fileFilter, const char *procFilter) {
    if (fileFilter) flag |= TR_FTRANSLATED;
    if (procFilter) flag |= TR_PTRANSLATED;
    fprintf(g_out, "@%x\n", flag);
    if (color != COL_NORMAL) fprintf(g_out, "[color] %d\n", color);
    if (fileFilter) fprintf(g_out, "^%d %s\n", ++g_file_id, fileFilter);
    if (procFilter) fprintf(g_out, "^>%d %s\n", ++g_proc_id, procFilter);
    char suf[80]; rng_suffix(s, suf);
    if (alias && alias[0]) fprintf(g_out, "+{%s} %s.%s%s\n", alias, s->scope, s->name, suf);
    else                   fprintf(g_out, "%s.%s%s\n", s->scope, s->name, suf);
}

static void emit_comment(const char *text) {
    fprintf(g_out, "@%x\n-%s\n", FLAG_COMMENT, text);
}
static void emit_group_begin(const char *name) { fprintf(g_out, "@%x\n-%s\n", FLAG_GRP_BEGIN, name); }
static void emit_group_end(const char *name)   { fprintf(g_out, "@%x\n-%s\n", FLAG_GRP_END, name); }

// find a signal by scope+exact-name; returns index or -1
static int find_sig(const char *scope, const char *name) {
    for (int i = 0; i < g_nsig; i++)
        if (strcmp(g_sig[i].name, name) == 0 && strcmp(g_sig[i].scope, scope) == 0) return i;
    return -1;
}
// find first signal by exact leaf name anywhere (clk/rst/itr live at the tb top,
// not under the proc -- so match by leaf, like the Tcl getVar substring search)
static int find_by_leaf(const char *name) {
    for (int i = 0; i < g_nsig; i++) if (strcmp(g_sig[i].name, name) == 0) return i;
    return -1;
}
static int scope_ends_with(const char *scope, const char *suf) {
    size_t L = strlen(scope), M = strlen(suf);
    return L >= M && strcmp(scope + L - M, suf) == 0;
}
// find leaf name whose scope ends with a given segment (e.g. ".sp"/".isp"/".ula").
// The flag blocks live deep (core.sp, core.instr_fetch.isp, core.ula) so we match
// by the stack/alu sub-scope suffix instead of a fixed depth.
static int find_by_suffix_leaf(const char *suffix, const char *name) {
    for (int i = 0; i < g_nsig; i++)
        if (strcmp(g_sig[i].name, name) == 0 && scope_ends_with(g_sig[i].scope, suffix)) return i;
    return -1;
}

// ---- single-proc emission --------------------------------------------------

static char g_inst[600] = "";   // processor instance scope (has valr2 + linetabs)
static char g_core[600] = "";   // .../core scope (clk/rst/itr, sp/isp/ula live below)

static int starts_with(const char *s, const char *p) { return strncmp(s, p, strlen(p)) == 0; }
static int in_scope(const Sig *s, const char *scope) { return strcmp(s->scope, scope) == 0; }
// is s anywhere at/under scope?
static int under_scope(const Sig *s, const char *scope) {
    size_t L = strlen(scope);
    return strncmp(s->scope, scope, L) == 0 && (s->scope[L] == 0 || s->scope[L] == '.');
}

static void detect_proc(void) {
    // instance = scope that owns both valr2 and linetabs
    for (int i = 0; i < g_nsig && !g_inst[0]; i++) {
        if (strcmp(g_sig[i].name, "valr2") == 0) {
            if (find_sig(g_sig[i].scope, "linetabs") >= 0)
                snprintf(g_inst, sizeof g_inst, "%s", g_sig[i].scope);
        }
    }
    // core = a scope ending in ".core" that sits under the instance (or anywhere)
    for (int i = 0; i < g_nsig; i++) {
        const char *sc = g_sig[i].scope; size_t L = strlen(sc);
        if (L >= 5 && strcmp(sc + L - 5, ".core") == 0) {
            if (!g_inst[0] || under_scope(&g_sig[i], g_inst) || starts_with(sc, g_inst)) {
                snprintf(g_core, sizeof g_core, "%s", sc); break;
            }
        }
    }
}

// emit a numbered I/O pair list: <prefix>N -> alias "<label> i"
static void emit_io(const char *prefix, unsigned flag, const char *label) {
    // collect matching leaf names in the instance scope, sorted by trailing number
    int idx[256], n = 0;
    for (int i = 0; i < g_nsig; i++) {
        if (!in_scope(&g_sig[i], g_inst)) continue;
        const char *nm = g_sig[i].name;
        if (starts_with(nm, prefix)) { if (n < 256) idx[n++] = i; }
    }
    // simple insertion sort by numeric suffix
    for (int a = 1; a < n; a++) {
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

// emit scalar typed vars of a given prefix (me1_/me2_/comp_me3_)
static void emit_typed_vars(const char *prefix, unsigned flag, const char *tlabel, const char *procFilter) {
    for (int i = 0; i < g_nsig; i++) {
        if (!in_scope(&g_sig[i], g_inst)) continue;
        const char *nm = g_sig[i].name;
        if (!starts_with(nm, prefix)) continue;
        char func[256], var[256], fl[256], alias[600];
        if (!split_func_var(nm, func, var)) continue;
        func_label(func, fl);
        snprintf(alias, sizeof alias, "%s %s in %s", tlabel, var, fl);
        emit_signal(&g_sig[i], flag, COL_ORANGE, alias, NULL, procFilter);
    }
}

// emit array vars of a given prefix grouped by base (name minus 4-digit suffix)
static int has_4digit_suffix(const char *nm, char *base, int *idx) {
    size_t L = strlen(nm);
    if (L < 4) return 0;
    for (int k = 0; k < 4; k++) if (!isdigit((unsigned char)nm[L-1-k])) return 0;
    size_t bl = L - 4; memcpy(base, nm, bl); base[bl] = 0;
    *idx = atoi(nm + L - 4);
    return 1;
}

static void emit_typed_arrays(const char *prefix, unsigned flag, const char *tlabel, const char *procFilter) {
    char seen[256][256]; int nseen = 0;
    for (int i = 0; i < g_nsig; i++) {
        if (!in_scope(&g_sig[i], g_inst)) continue;
        const char *nm = g_sig[i].name;
        if (!starts_with(nm, prefix)) continue;
        char base[256]; int dummy;
        if (!has_4digit_suffix(nm, base, &dummy)) continue;
        // new base?
        int known = 0; for (int k = 0; k < nseen; k++) if (strcmp(seen[k], base) == 0) { known = 1; break; }
        if (known) continue;
        if (nseen < 256) strcpy(seen[nseen++], base);
        // group label from the base's mangled func/var
        char func[256], var[256], fl[256], glabel[600];
        if (split_func_var(base, func, var)) { func_label(func, fl); snprintf(glabel, sizeof glabel, "%s %s in %s", tlabel, var, fl); }
        else snprintf(glabel, sizeof glabel, "%s %s", tlabel, base);
        emit_group_begin(glabel);
        // members sorted by index
        int mi[4096], mn = 0;
        for (int j = 0; j < g_nsig; j++) {
            if (!in_scope(&g_sig[j], g_inst)) continue;
            if (!starts_with(g_sig[j].name, prefix)) continue;
            char b2[256]; int ix;
            if (!has_4digit_suffix(g_sig[j].name, b2, &ix)) continue;
            if (strcmp(b2, base) != 0) continue;
            if (mn < 4096) mi[mn++] = j;
        }
        for (int a = 1; a < mn; a++) { // sort by suffix idx
            int ia = mi[a], ja = a - 1; char tb[256]; int va; has_4digit_suffix(g_sig[ia].name, tb, &va);
            int vb; char tb2[256];
            while (ja >= 0 && (has_4digit_suffix(g_sig[mi[ja]].name, tb2, &vb), vb) > va) { mi[ja+1] = mi[ja]; ja--; }
            mi[ja+1] = ia;
        }
        for (int a = 0; a < mn; a++) {
            char al[300]; char vv[256]=""; { char f2[256],v2[256]; if (split_func_var(base,f2,v2)) strcpy(vv,v2); else strcpy(vv,base);}
            snprintf(al, sizeof al, "%s %d", vv, a);
            emit_signal(&g_sig[mi[a]], flag, COL_ORANGE, al, NULL, procFilter);
        }
        emit_group_end(glabel);
    }
}

// flags group helper: emit a flag signal if present, matched by sub-scope suffix
static void emit_flag_s(const char *suffix, const char *name, unsigned flag, const char *alias) {
    int i = find_by_suffix_leaf(suffix, name);
    if (i >= 0) emit_signal(&g_sig[i], flag, COL_NORMAL, alias, NULL, NULL);
}

static void emit_proc_section(void) {
    char banner[700];
    const char *instname = strrchr(g_inst, '.'); instname = instname ? instname + 1 : g_inst;
    snprintf(banner, sizeof banner, "###### %s", instname);
    emit_comment(banner);

    // clk / rst / itr -- matched by leaf name (they sit at the tb top, not the proc)
    int ci;
    if ((ci = find_by_leaf("clk")) >= 0) emit_signal(&g_sig[ci], FMT_BIN, COL_NORMAL, NULL, NULL, NULL);
    if ((ci = find_by_leaf("rst")) >= 0) emit_signal(&g_sig[ci], FMT_BIN, COL_NORMAL, NULL, NULL, NULL);
    if ((ci = find_by_leaf("itr")) >= 0) emit_signal(&g_sig[ci], FMT_BIN, COL_NORMAL, NULL, NULL, NULL);

    // I/O
    emit_comment("I/O ****************");
    emit_io("req_in_sim_", FMT_BIN,        "req_in");
    emit_io("in_sim_",     FMT_SIGNED_DEC, "input ");
    emit_io("out_en_sim_", FMT_BIN,        "out_en");
    emit_io("out_sig_",    FMT_SIGNED_DEC, "output");

    // instructions
    emit_comment("Instructions *******");
    int vi = find_sig(g_inst, "valr2");
    if (vi >= 0) {
        char trad[700]; snprintf(trad, sizeof trad, "%s\\trad_opcode.txt", g_trad_dir);
        emit_signal(&g_sig[vi], FMT_DEC, COL_INDIGO, "Assembly", trad, NULL);
    }
    int li = find_sig(g_inst, "linetabs");
    if (li >= 0) {
        char trad[700]; snprintf(trad, sizeof trad, "%s\\trad_cmm.txt", g_trad_dir);
        emit_signal(&g_sig[li], FMT_SIGNED_DEC, COL_VIOLET, "C+-", trad, NULL);
    }

    // variables
    emit_comment("Variables **********");
    emit_typed_vars("me1_",      FMT_SIGNED_DEC, "int",   NULL);
    emit_typed_vars("me2_",      FMT_REAL,       "float", NULL);
    emit_typed_vars("comp_me3_", FMT_BIN,        "comp",  g_comp2gtkw);
    emit_typed_arrays("arr_me1_",      FMT_SIGNED_DEC, "int",   NULL);
    emit_typed_arrays("arr_me2_",      FMT_REAL,       "float", NULL);
    emit_typed_arrays("comp_arr_me3_", FMT_BIN,        "comp",  g_comp2gtkw);

    // flags (Stack / ALU) -- live deep under core (core.sp, core...isp, core.ula);
    // matched by sub-scope suffix. Emit only if present (Verilator fences them
    // out, so they may be absent; that's fine).
    int any_stack = find_by_suffix_leaf(".sp","pointeri")>=0 || find_by_suffix_leaf(".isp","pointeri")>=0;
    int any_ula   = find_by_suffix_leaf(".ula","delta_int")>=0 || find_by_suffix_leaf(".ula","delta_float")>=0;
    if (any_stack || any_ula) emit_comment("Flags **************");
    if (any_stack) {
        emit_group_begin("Stack");
        emit_flag_s(".sp",  "pointeri", FMT_ANALOG_SIGNED, "Data Stack Pointer");
        emit_flag_s(".sp",  "fl_max",   FMT_DEC,           "Data Stack Max");
        emit_flag_s(".sp",  "fl_full",  FMT_BIN,           "Data Stack Overflow");
        emit_flag_s(".isp", "pointeri", FMT_ANALOG_SIGNED, "Inst Stack Pointer");
        emit_flag_s(".isp", "fl_max",   FMT_DEC,           "Inst Stack Max");
        emit_flag_s(".isp", "fl_full",  FMT_BIN,           "Inst Stack Overflow");
        emit_group_end("Stack");
    }
    if (any_ula) {
        emit_group_begin("ALU");
        emit_flag_s(".ula", "delta_int",   FMT_ANALOG_HEX, "Rounding Error (int)");
        emit_flag_s(".ula", "delta_float", FMT_ANALOG_HEX, "Rounding Error (float)");
        emit_group_end("ALU");
    }
}

// ---- main ------------------------------------------------------------------

int main(int argc, char **argv) {
    if (argc < 5) {
        fprintf(stderr, "usage: %s <in.vcd> <out.gtkw> <trad_dir> <comp2gtkw_exe>\n", argv[0]);
        return 1;
    }
    const char *vcd = argv[1], *out = argv[2];
    g_trad_dir = argv[3];
    g_comp2gtkw = argv[4];

    parse_vcd_header(vcd);
    detect_proc();
    if (!g_inst[0]) {
        fprintf(stderr, "gen_gtkw: no processor scope found (no valr2+linetabs) in %s\n", vcd);
        // still emit a minimal file so gtkwave opens something
    }

    g_out = fopen(out, "wb");
    if (!g_out) { fprintf(stderr, "gen_gtkw: cannot write '%s'\n", out); return 2; }

    // header block (paths verbatim, matches GTKWave's canonical Windows save)
    fprintf(g_out, "[*]\n[*] Generated by gen_gtkw (yanc)\n[*]\n");
    fprintf(g_out, "[dumpfile] \"%s\"\n", vcd);
    fprintf(g_out, "[savefile] \"%s\"\n", out);
    fprintf(g_out, "[timestart] 0\n");

    if (g_inst[0]) emit_proc_section();

    fclose(g_out);
    fprintf(stderr, "gen_gtkw: %d signals parsed, inst='%s' core='%s' -> %s\n",
            g_nsig, g_inst, g_core, out);
    return 0;
}
