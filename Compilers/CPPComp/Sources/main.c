// ----------------------------------------------------------------------------
// CPPComp — compiler entry point -----------------------------------------------
// ----------------------------------------------------------------------------
// usage: cppcomp [-o <dir>] [-I <dir>]... [-E] <file.cpp>
// Preprocesses the source in memory (cpppp.c), then compiles it, as gcc runs
// cpp and cc1. Mirrors cmmcomp: the processor takes the name of the source
// file, and <name>.asm and the tables go to one output folder, -o or ./<name>.
// -E prints the preprocessed text and stops.
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
#include "../Headers/cpppp.h"
#include "../../common/asm_share.h"
#include "../../common/console_utf8.h"
#include "../../common/exe_dir.h"
#include "../../common/yanc_lang.h"
#include "../../common/proc_name.h"

/* asm_reach dropped `gone` instructions the pc map covered: prog_log.txt's
 * "num_ins" line, already written by codegen, drops by as much (a negative
 * `gone` raises it: asm_divseq's added words). */
static void lower_num_ins(const char *tmp_dir, int gone) {
    char path[2048]; snprintf(path, sizeof(path), "%s/prog_log.txt", tmp_dir);
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
extern struct yy_buffer_state *yy_scan_string(const char *text);   // flex
extern unit *g_unit;

static void usage(int code)
{
    fprintf(code ? stderr : stdout, MSG_CLI_USAGE, YANC_VERSION);
    exit(code);
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
    yanc_lang(&argc, argv, "cppcomp");   // --lang and YANC_LANG: the messages' language (messages.h)
    const char *in_path  = NULL;
    const char *out_dir  = NULL;
    int         only_pp  = 0;

    for (int i = 1; i < argc; i++) {
        if      ((!strcmp(argv[i], "-o") || !strcmp(argv[i], "--output"  )) && i+1 < argc) out_dir  = argv[++i];
        else if ( !strcmp(argv[i], "-I") && i+1 < argc) pp_add_incdir(argv[++i]);
        else if ( !strcmp(argv[i], "-E")) only_pp = 1;
        else if ( !strcmp(argv[i], "-h") || !strcmp(argv[i], "--help"    )) usage(0);
        else if ( !strcmp(argv[i], "-V") || !strcmp(argv[i], "--version" )) {
            printf("cppcomp %s\n", YANC_VERSION);
            return 0;
        }
        else if (argv[i][0] != '-' && !in_path) in_path = argv[i];   // the source
        else { fprintf(stderr, MSG_ERR_UNKNOWN_OPTION, argv[i]); usage(1); }
    }
    if (!in_path) usage(1);

    msg_set_file(in_path);

    const char *text = pp_run(in_path);  // the source, preprocessed
    if (only_pp) { fputs(text, stdout); return 0; }

    // the file's name becomes a Verilog module's: refuse one that cannot be
    {
        char *nm = derive_name(in_path, "");
        const char *const why[] = {"", MSG_WHY_CHARS, MSG_WHY_DIGIT, MSG_WHY_DUNDER,
            MSG_WHY_KEYWORD};
        int bad = proc_name_problem(nm);
        if (bad) {
            fprintf(stderr, MSG_ERR_FILE_NAME,
                    in_path, nm, why[bad]);
            return 2;
        }
        free(nm);
    }
    yy_scan_string(text);

    g_unit = ast_unit();
    st_init();

    if (yyparse() != 0) { fprintf(stderr, MSG_ERR_PARSE_FAILED); exit(2); }

    if (msg_error_count() > 0) return 2;
    g_unit->prname = derive_name(in_path, "");   // the source file's name

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
    if (!fo) { fprintf(stderr, MSG_ERR_CANT_WRITE, outp); exit(1); }

    codegen(fo, g_unit, tmp_dir, text);

    fclose(fo);
    if (msg_error_count() > 0) return 2;

    // code no path reaches leaves the program, and its hardware with it (the
    // pc_<proc>_mem.txt map and prog_log's num_ins follow)
    if (tmp_dir) {
        char pc[2048]; snprintf(pc, sizeof(pc), "%s/pc_%s_mem.txt", tmp_dir, g_unit->prname);
        int gone = asm_reach(outp, pc);
        if (gone > 0) lower_num_ins(tmp_dir, gone);
        // every division becomes `<div> x; NOP...; <read>` (the dividers take
        // five / four cycles); the count grows by the words added
        int more = asm_divseq(outp, pc);
        if (more > 0) lower_num_ins(tmp_dir, -more);
    } else {
        asm_reach(outp, NULL);
        asm_divseq(outp, NULL);
    }

    // scalars never alive together share one data word (#SHARE lines ahead of
    // the code; the instructions and the pc_<proc>_mem.txt map are untouched)
    asm_share(outp);

    // the folder of the source, where asmcomp reads the user's own files
    // (input_N.txt, cade_k.txt)
    char log[2048]; snprintf(log, sizeof(log), "%s/prog_log.txt", tmp_dir);
    FILE *fl = fopen(log, "a");
    if (fl) {
        char *src = dir_of(in_path);
        if (src) fprintf(fl, "srcdir %s\n", src);
        fprintf(fl, "lang C++\n");       // the waveform's source track takes its name
        fclose(fl);
    }
    return 0;
}
