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
//   gen_gtkw <in.vcd> <out.gtkw> <tmp_base> <comp2gtkw_exe>
//     <tmp_base>      base temp dir; per-proc translate files are read from
//                     <tmp_base>/<proc_type>/trad_opcode.txt and trad_cmm.txt
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
    char scope[600];   // hierarchical scope path, e.g. tb.proc
    char name[256];    // leaf signal name
    int  width;
    char range[64];    // e.g. "22:0" or "" if scalar
} Sig;

static Sig  g_sig[60000];
static int  g_nsig = 0;

// a detected processor instance
typedef struct {
    char inst[600];    // scope owning valr2 + linetabs (mirrors live here)
    char core[600];    // <inst>...p_<type>.core (clk/rst, sp/isp/ula live below)
    char type[128];    // module type, for <tmp_base>/<type>/trad_*.txt
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

static int read_token(char *out, size_t cap) {
    while (g_pos < g_len && isspace((unsigned char)g_buf[g_pos])) g_pos++;
    if (g_pos >= g_len) return 0;
    size_t s = g_pos;
    if (g_buf[g_pos] == '[') {
        size_t p = g_pos + 1;
        while (p < g_len && g_buf[p] != ']' && !isspace((unsigned char)g_buf[p])) p++;
        if (p < g_len && g_buf[p] == ']') p++;
        else p = g_pos + 1;
        size_t n = p - s; if (n >= cap) n = cap - 1;
        memcpy(out, g_buf + s, n); out[n] = 0; g_pos = p; return 1;
    }
    while (g_pos < g_len && !isspace((unsigned char)g_buf[g_pos])) g_pos++;
    size_t n = g_pos - s; if (n >= cap) n = cap - 1;
    memcpy(out, g_buf + s, n); out[n] = 0; return 1;
}

static char g_stack[128][600];
static int  g_stack_mod[128];
static int  g_depth = 0;

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
    // we only need the header; read in chunks and stop after $enddefinitions so a
    // multi-GB full-sim VCD doesn't get slurped whole.
    g_buf = (char *)malloc(sz + 1);
    if (!g_buf) { fprintf(stderr, "gen_gtkw: out of memory\n"); exit(2); }
    g_len = fread(g_buf, 1, sz, f); g_buf[g_len] = 0; fclose(f);
    char *ed = strstr(g_buf, "$enddefinitions");
    if (ed) g_len = (size_t)(ed - g_buf);

    char t[700];
    while (read_token(t, sizeof t)) {
        if (strcmp(t, "$scope") == 0) {
            char stype[64] = "", sname[256] = "";
            read_token(stype, sizeof stype);
            read_token(sname, sizeof sname);
            skip_to_end();
            if (strcmp(stype, "module") == 0 && g_depth < 128) {
                const char *par = nearest_module_path();
                char tmp[600];
                if (par) snprintf(tmp, sizeof tmp, "%s.%s", par, sname);
                else     snprintf(tmp, sizeof tmp, "%s", sname);
                snprintf(g_stack[g_depth], 600, "%s", tmp);
                g_stack_mod[g_depth] = 1; g_depth++;
            } else if (g_depth < 128) {
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
            if (read_token(maybe, sizeof maybe)) {
                if (maybe[0] == '[' && strcmp(maybe, "$end") != 0) {
                    size_t L = strlen(maybe);
                    if (L >= 2 && maybe[L-1] == ']') { memcpy(rng, maybe+1, L-2); rng[L-2] = 0; }
                    skip_to_end();
                } else if (strcmp(maybe, "$end") != 0) {
                    skip_to_end();
                }
            }
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

// extract func/var from a mangled name: ..._f_<func>_v_<var>_e_  (unanchored)
static int split_func_var(const char *name, char *func, char *var) {
    const char *f = strstr(name, "_f_"); if (!f) return 0;
    const char *v = strstr(f + 3, "_v_"); if (!v) return 0;
    const char *e = strstr(v + 3, "_e_"); if (!e) return 0;
    size_t fl = (size_t)(v - (f + 3)); memcpy(func, f + 3, fl); func[fl] = 0;
    size_t vl = (size_t)(e - (v + 3)); memcpy(var, v + 3, vl); var[vl] = 0;
    return 1;
}
static void func_label(const char *func, char *out) {
    if (strcmp(func, "global") == 0) strcpy(out, "global");
    else snprintf(out, 256, "%s()", func);
}
static void rng_suffix(const Sig *s, char *out) {
    if (s->range[0]) snprintf(out, 80, "[%s]", s->range);
    else out[0] = 0;
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
    char suf[80]; rng_suffix(s, suf);
    if (alias && alias[0]) fprintf(g_out, "+{%s} %s.%s%s\n", alias, s->scope, s->name, suf);
    else                   fprintf(g_out, "%s.%s%s\n", s->scope, s->name, suf);
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
        char func[256], var[256], fl[256], alias[600];
        if (!split_func_var(g_sig[i].name, func, var)) continue;
        func_label(func, fl);
        snprintf(alias, sizeof alias, "%s %s in %s", tlabel, var, fl);
        emit_signal(&g_sig[i], flag, COL_ORANGE, alias, NULL, procFilter);
    }
}

static int has_4digit_suffix(const char *nm, char *base, int *idx) {
    size_t L = strlen(nm);
    if (L < 4) return 0;
    for (int k = 0; k < 4; k++) if (!isdigit((unsigned char)nm[L-1-k])) return 0;
    size_t bl = L - 4; memcpy(base, nm, bl); base[bl] = 0;
    *idx = atoi(nm + L - 4);
    return 1;
}

// array vars (arr_me1_/arr_me2_/comp_arr_me3_) within `inst`, grouped by base
static void emit_typed_arrays(const char *inst, const char *prefix, unsigned flag,
                              const char *tlabel, const char *procFilter) {
    char seen[256][256]; int nseen = 0;
    for (int i = 0; i < g_nsig; i++) {
        if (strcmp(g_sig[i].scope, inst) != 0) continue;
        if (!starts_with(g_sig[i].name, prefix)) continue;
        char base[256]; int dummy;
        if (!has_4digit_suffix(g_sig[i].name, base, &dummy)) continue;
        int known = 0; for (int k = 0; k < nseen; k++) if (strcmp(seen[k], base) == 0) { known = 1; break; }
        if (known) continue;
        if (nseen < 256) strcpy(seen[nseen++], base);

        char func[256], var[256], fl[256], glabel[600], vv[256];
        if (split_func_var(base, func, var)) { func_label(func, fl); snprintf(glabel, sizeof glabel, "%s %s in %s", tlabel, var, fl); strcpy(vv, var); }
        else { snprintf(glabel, sizeof glabel, "%s %s", tlabel, base); strcpy(vv, base); }
        emit_group_begin(glabel);

        int mi[8192], mn = 0;
        for (int j = 0; j < g_nsig; j++) {
            if (strcmp(g_sig[j].scope, inst) != 0) continue;
            if (!starts_with(g_sig[j].name, prefix)) continue;
            char b2[256]; int ix;
            if (!has_4digit_suffix(g_sig[j].name, b2, &ix)) continue;
            if (strcmp(b2, base) != 0) continue;
            if (mn < 8192) mi[mn++] = j;
        }
        for (int a = 1; a < mn; a++) {  // sort by suffix index
            int ia = mi[a], ja = a - 1; char tb[256]; int va; has_4digit_suffix(g_sig[ia].name, tb, &va);
            char tb2[256]; int vb;
            while (ja >= 0 && (has_4digit_suffix(g_sig[mi[ja]].name, tb2, &vb), vb) > va) { mi[ja+1] = mi[ja]; ja--; }
            mi[ja+1] = ia;
        }
        for (int a = 0; a < mn; a++) {
            char al[320]; snprintf(al, sizeof al, "%s %d", vv, a);
            emit_signal(&g_sig[mi[a]], flag, COL_ORANGE, al, NULL, procFilter);
        }
        emit_group_end(glabel);
    }
}

// emit a flag signal if present, matched under `inst` by sub-scope suffix
static void emit_flag(const char *inst, const char *suffix, const char *name, unsigned flag, const char *alias) {
    int i = find_under_suffix(inst, suffix, name);
    if (i >= 0) emit_signal(&g_sig[i], flag, COL_NORMAL, alias, NULL, NULL);
}

static void emit_proc_section(const Proc *p) {
    char banner[700];
    const char *instname = strrchr(p->inst, '.'); instname = instname ? instname + 1 : p->inst;
    snprintf(banner, sizeof banner, "###### %s", instname);
    emit_comment(banner);

    // clk / rst / itr -- prefer the proc's core (project case: wrapper drives
    // core.clk); fall back to the tb top (single-proc generated tb: clk/rst are
    // testbench regs at the root scope).
    char tbroot[600]; snprintf(tbroot, sizeof tbroot, "%s", p->inst);
    char *dot = strchr(tbroot, '.'); if (dot) *dot = 0;
    const char *cs = p->core[0] ? p->core : p->inst;
    int ci;
    if ((ci = find_sig(cs, "clk")) < 0) ci = find_sig(tbroot, "clk");
    if (ci >= 0) emit_signal(&g_sig[ci], FMT_BIN, COL_NORMAL, NULL, NULL, NULL);
    if ((ci = find_sig(cs, "rst")) < 0) ci = find_sig(tbroot, "rst");
    if (ci >= 0) emit_signal(&g_sig[ci], FMT_BIN, COL_NORMAL, NULL, NULL, NULL);
    if ((ci = find_sig(cs, "itr")) < 0) ci = find_sig(tbroot, "itr");
    if (ci >= 0) emit_signal(&g_sig[ci], FMT_BIN, COL_NORMAL, NULL, NULL, NULL);

    emit_comment("I/O ****************");
    emit_io(p->inst, "req_in_sim_", FMT_BIN,        "req_in");
    emit_io(p->inst, "in_sim_",     FMT_SIGNED_DEC, "input ");
    emit_io(p->inst, "out_en_sim_", FMT_BIN,        "out_en");
    emit_io(p->inst, "out_sig_",    FMT_SIGNED_DEC, "output");

    // per-proc translate files at <tmp_base>/<type>/ (forward slashes work on
    // both Windows and POSIX, and GTKWave accepts them in the .gtkw paths).
    char tradop[800] = "", tradcmm[800] = "";
    if (p->type[0]) {
        snprintf(tradop,  sizeof tradop,  "%s/%s/trad_opcode.txt", g_tmp_base, p->type);
        snprintf(tradcmm, sizeof tradcmm, "%s/%s/trad_cmm.txt",    g_tmp_base, p->type);
    }
    emit_comment("Instructions *******");
    int vi = find_sig(p->inst, "valr2");
    if (vi >= 0) emit_signal(&g_sig[vi], FMT_DEC, COL_INDIGO, "Assembly", tradop[0]?tradop:NULL, NULL);
    int li = find_sig(p->inst, "linetabs");
    if (li >= 0) emit_signal(&g_sig[li], FMT_SIGNED_DEC, COL_VIOLET, "C+-", tradcmm[0]?tradcmm:NULL, NULL);

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
        emit_flag(p->inst, ".sp",  "pointeri", FMT_ANALOG_SIGNED, "Data Stack Pointer");
        emit_flag(p->inst, ".sp",  "fl_max",   FMT_DEC,           "Data Stack Max");
        emit_flag(p->inst, ".sp",  "fl_full",  FMT_BIN,           "Data Stack Overflow");
        emit_flag(p->inst, ".isp", "pointeri", FMT_ANALOG_SIGNED, "Inst Stack Pointer");
        emit_flag(p->inst, ".isp", "fl_max",   FMT_DEC,           "Inst Stack Max");
        emit_flag(p->inst, ".isp", "fl_full",  FMT_BIN,           "Inst Stack Overflow");
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
static void derive_core_and_type(Proc *p) {
    // core = the ".core" scope under inst. Single-proc tbs dump no signal AT the
    // core scope itself (only core.sp/core.ula/...), so derive the core path by
    // truncating any under-inst signal scope that contains ".core" at that point.
    p->core[0] = 0; p->type[0] = 0;
    for (int i = 0; i < g_nsig; i++) {
        const char *sc = g_sig[i].scope;
        if (!under(sc, p->inst)) continue;
        const char *c = strstr(sc, ".core");
        if (c && (c[5] == '.' || c[5] == 0)) {
            size_t len = (size_t)(c - sc) + 5;              // include ".core"
            if (len < sizeof p->core) { memcpy(p->core, sc, len); p->core[len] = 0; }
            break;
        }
    }
    // type from the "p_<type>.core" segment of the core path
    if (p->core[0]) {
        size_t L = strlen(p->core);
        const char *seg_end = p->core + L - 5;            // points at ".core"
        const char *seg_start = seg_end;
        while (seg_start > p->core && *(seg_start - 1) != '.') seg_start--;
        // seg holds a (short) module-type name; size it to match p->type so the
        // snprintf below cannot truncate (keeps -Wformat-truncation quiet).
        char seg[sizeof ((Proc*)0)->type]; size_t n = (size_t)(seg_end - seg_start);
        if (n >= sizeof seg) n = sizeof seg - 1;
        memcpy(seg, seg_start, n); seg[n] = 0;
        if (strncmp(seg, "p_", 2) == 0) snprintf(p->type, sizeof p->type, "%s", seg + 2);
        else                            snprintf(p->type, sizeof p->type, "%s", seg);
    } else {
        // fall back: instance name with a trailing _inst stripped
        const char *nm = strrchr(p->inst, '.'); nm = nm ? nm + 1 : p->inst;
        snprintf(p->type, sizeof p->type, "%s", nm);
        char *u = strstr(p->type, "_inst"); if (u && u[5] == 0) *u = 0;
    }
}

static void detect_procs(void) {
    for (int i = 0; i < g_nsig && g_nproc < 32; i++) {
        if (strcmp(g_sig[i].name, "valr2") != 0) continue;
        if (find_sig(g_sig[i].scope, "linetabs") < 0) continue;
        int dup = 0;
        for (int k = 0; k < g_nproc; k++) if (strcmp(g_proc[k].inst, g_sig[i].scope) == 0) { dup = 1; break; }
        if (dup) continue;
        // inst and scope are the same fixed size and scope is NUL-terminated,
        // so a fixed-size copy is safe and avoids -Wformat-truncation.
        memcpy(g_proc[g_nproc].inst, g_sig[i].scope, sizeof g_proc[g_nproc].inst);
        derive_core_and_type(&g_proc[g_nproc]);
        g_nproc++;
    }
}

// ---- main ------------------------------------------------------------------
int main(int argc, char **argv) {
    if (argc < 5) {
        fprintf(stderr, "usage: %s <in.vcd> <out.gtkw> <tmp_base> <comp2gtkw_exe>\n", argv[0]);
        return 1;
    }
    const char *vcd = argv[1], *out = argv[2];
    g_tmp_base = argv[3];
    g_comp2gtkw = argv[4];

    parse_vcd_header(vcd);
    detect_procs();

    g_out = fopen(out, "wb");
    if (!g_out) { fprintf(stderr, "gen_gtkw: cannot write '%s'\n", out); return 2; }

    fprintf(g_out, "[*]\n[*] Generated by gen_gtkw (yanc)\n[*]\n");
    fprintf(g_out, "[dumpfile] \"%s\"\n", vcd);
    fprintf(g_out, "[savefile] \"%s\"\n", out);
    fprintf(g_out, "[timestart] 0\n");

    for (int i = 0; i < g_nproc; i++) emit_proc_section(&g_proc[i]);

    fclose(g_out);
    fprintf(stderr, "gen_gtkw: %d signals, %d processor(s) -> %s\n", g_nsig, g_nproc, out);
    for (int i = 0; i < g_nproc; i++)
        fprintf(stderr, "  proc[%d] inst=%s  type=%s\n", i, g_proc[i].inst, g_proc[i].type);
    return 0;
}
