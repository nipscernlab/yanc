// ----------------------------------------------------------------------------
// CPPComp — compiler entry point -----------------------------------------------
// ----------------------------------------------------------------------------
// usage: cppcomp [-o <dir>] <input.cpp>
// input is assumed to be already preprocessed (run cpppp first).
//
// Mirrors cmmcomp: the processor takes the name of the source file (the
// original one, which cpppp names in `#pragma yanc source`), and
// <name>.asm and the tables go to one output folder, -o or ./<name>.
// ----------------------------------------------------------------------------

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/stat.h>

#include "../Headers/config.h"
#include "../Headers/ast.h"
#include "../Headers/symtab.h"
#include "../Headers/codegen.h"
#include "../Headers/messages.h"
#include "../../common/asm_share.h"
#include "../../common/console_utf8.h"
#include "../../common/exe_dir.h"

/* asm_reach dropped `gone` instructions the pc map covered: cmm_log.txt's
 * "num_ins" line, already written by codegen, drops by as much (a negative
 * `gone` raises it: asm_divseq's added words). */
static void lower_num_ins(const char *tmp_dir, int gone) {
    char path[2048]; snprintf(path, sizeof(path), "%s/cmm_log.txt", tmp_dir);
    FILE *f = fopen(path, "r");
    if (!f) return;
    char *text = NULL; size_t len = 0, cap = 0; char line[1024];
    while (fgets(line, sizeof(line), f)) {
        int v;
        if (sscanf(line, "num_ins %d", &v) == 1) snprintf(line, sizeof(line), "num_ins %d\n", v - gone);
        size_t k = strlen(line);
        if (len + k + 1 > cap) { cap = 2 * (len + k + 1); text = realloc(text, cap); }
        memcpy(text + len, line, k + 1); len += k;
    }
    fclose(f);
    if ((f = fopen(path, "w"))) { if (text) fputs(text, f); fclose(f); }
    free(text);
}

extern int yyparse(void);
extern FILE *yyin;
extern unit *g_unit;

static void usage(void)
{
    fprintf(stderr,
        "cppcomp %s — C++ compiler for the SAPHO processor\n"
        "usage: cppcomp [-o <dir>] <input.cpp>\n"
        "  <input.cpp>             the .cpp preprocessed by cpppp; the processor\n"
        "                          takes the name of the original source file\n"
        "  -o, --output   <dir>    output folder (default: ./<name>): <name>.asm and\n"
        "                          the tables asmcomp and the waveform read\n"
        "  -h, --help              this help\n"
        "  -V, --version           show version and exit\n"
        "\n"
        "build-time defaults (override with -D when building cppcomp.exe):\n"
        "  NUBITS=%d  NBMANT=%d  NBEXPO=%d  NUGAIN=%d\n"
        "  NUIOIN=%d  NUIOOU=%d  FFTSIZ=%d\n"
        "stack depths (NDSTAC, SDEPTH): worked out by asmcomp unless a pragma sets them\n",
        YANC_VERSION,
        CFG_NUBITS, CFG_NBMANT, CFG_NBEXPO, CFG_NUGAIN,
        CFG_NUIOIN, CFG_NUIOOU, CFG_FFTSIZ);
    exit(1);
}

static char *derive_name(const char *path, const char *new_ext)
{
    const char *base = path;
    for (const char *p = path; *p; p++) if (*p == '/' || *p == '\\') base = p + 1;
    const char *dot = strrchr(base, '.');
    size_t blen = dot ? (size_t)(dot - base) : strlen(base);
    size_t elen = strlen(new_ext);
    char *r = malloc(blen + elen + 1);
    memcpy(r, base, blen);
    memcpy(r + blen, new_ext, elen);
    r[blen + elen] = 0;
    return r;
}

int main(int argc, char **argv)
{
    console_utf8();                 // the help text is UTF-8 (Windows console)
    const char *in_path  = NULL;
    const char *out_dir  = NULL;

    for (int i = 1; i < argc; i++) {
        if      ((!strcmp(argv[i], "-o") || !strcmp(argv[i], "--output"  )) && i+1 < argc) out_dir  = argv[++i];
        else if ( !strcmp(argv[i], "-h") || !strcmp(argv[i], "--help"    )) usage();
        else if ( !strcmp(argv[i], "-V") || !strcmp(argv[i], "--version" )) {
            printf("cppcomp %s\n", YANC_VERSION);
            return 0;
        }
        else if (argv[i][0] != '-' && !in_path) in_path = argv[i];   // the source
        else { fprintf(stderr, "cppcomp: unknown option '%s'\n", argv[i]); usage(); }
    }
    if (!in_path) usage();

    msg_set_file(in_path);

    yyin = fopen(in_path, "r");
    if (!yyin) { fprintf(stderr, "cppcomp: cannot open '%s'\n", in_path); exit(1); }

    g_unit = ast_unit();
    st_init();

    if (yyparse() != 0) { fprintf(stderr, "cppcomp: parse failed\n"); exit(2); }
    fclose(yyin);

    if (msg_error_count() > 0) return 2;
    // the name of the original source (cpppp's `#pragma yanc source`), else
    // of the input
    g_unit->prname = derive_name(g_unit->srcdir ? g_unit->srcdir : in_path, "");

    /* Output: everything in one folder, -o or ./<prname> (created, all
     * levels): <prname>.asm and the tables asmcomp and the waveform read. */
    char *dflt = NULL;
    if (!out_dir) {
        dflt = malloc(strlen(g_unit->prname) + 3);
        sprintf(dflt, "./%s", g_unit->prname);
        out_dir = dflt;
    }
    make_dir(out_dir);
    const char *tmp_dir = out_dir;
    char *outp = malloc(strlen(out_dir) + strlen(g_unit->prname) + 8);
    sprintf(outp, "%s/%s.asm", out_dir, g_unit->prname);
    FILE *fo = fopen(outp, "w");
    if (!fo) { fprintf(stderr, "cppcomp: cannot open '%s' for writing\n", outp); exit(1); }

    codegen(fo, g_unit, tmp_dir, in_path);

    fclose(fo);
    if (msg_error_count() > 0) return 2;

    // code no path reaches leaves the program, and its hardware with it (the
    // pc_<proc>_mem.txt map and cmm_log's num_ins follow)
    if (tmp_dir) {
        char pc[2048]; snprintf(pc, sizeof(pc), "%s/pc_%s_mem.txt", tmp_dir, g_unit->prname);
        int gone = asm_reach(outp, pc);
        if (gone > 0) lower_num_ins(tmp_dir, gone);
        // every division becomes `<div> x; NOP; <read>` (the dividers take
        // three cycles); the count grows by the words added
        int more = asm_divseq(outp, pc);
        if (more > 0) lower_num_ins(tmp_dir, -more);
    } else {
        asm_reach(outp, NULL);
        asm_divseq(outp, NULL);
    }

    // scalars never alive together share one data word (#SHARE lines ahead of
    // the code; the instructions and the pc_<proc>_mem.txt map are untouched)
    asm_share(outp);

    // the folder of the original source, where asmcomp reads the user's own
    // files (input_N.txt, cade_k.txt): cpppp's `#pragma yanc source`, else the
    // input
    char log[2048]; snprintf(log, sizeof(log), "%s/cmm_log.txt", tmp_dir);
    FILE *fl = fopen(log, "a");
    if (fl) {
        char *src = dir_of(g_unit->srcdir ? g_unit->srcdir : in_path);
        if (src) fprintf(fl, "srcdir %s\n", src);
        fclose(fl);
    }
    return 0;
}
