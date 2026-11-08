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
#include "../../common/xstr.h"
#include "../../common/asm_share.h"
#include "../../common/console_utf8.h"
#include "../../common/exe_dir.h"
#include "../../common/yanc_lang.h"
#include "../../common/proc_name.h"

/* asm_reach dropped `gone` instructions the pc map covered: prog_log.txt's
 * "num_ins" line, already written by codegen, drops by as much (a negative
 * `gone` raises it: asm_divseq's added words). */
static void lower_num_ins(const char *tmp_dir, int gone) {
    char *path = xasprintf("%s/prog_log.txt", tmp_dir);
    FILE *f = fopen(path, "r");
    if (!f) { free(path); return; }
    char *text = NULL, *line; size_t len = 0, cap = 0;
    while ((line = xgetline(f)) != NULL) {
        int v;
        char *out = sscanf(line, "num_ins %d", &v) == 1 ? xasprintf("num_ins %d\n", v - gone)
                                                         : xasprintf("%s\n", line);
        size_t k = strlen(out);
        if (len + k + 1 > cap) { cap = 2 * (len + k + 1); text = xrealloc(text, cap); }
        memcpy(text + len, out, k + 1); len += k;
        free(out); free(line);
    }
    fclose(f);
    if ((f = fopen(path, "w"))) { if (text) fputs(text, f); fclose(f); }
    free(text); free(path);
}

/* The files of this processor in its output folder. A run that stops on an
 * error, by msg_error's exit() or by a return from main, removes them (#94
 * N): a compilation that failed leaves no .asm or table that a later step,
 * or the user, could take for the result. */
static char *out_files[4];
static int   out_done = 0;

static void remove_outputs(void)
{
    if (out_done) return;
#ifdef _WIN32
    // an error exits with the .asm and the line map still open, and the C
    // library closes them only after the atexit handlers: Windows does not
    // remove an open file, so they stayed (#117). The process is ending, so
    // every stream but the standard ones can close here
    _fcloseall();
#endif
    for (int i = 0; i < 4; i++) if (out_files[i]) remove(out_files[i]);
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
    char *r = xmalloc(blen + elen + 1);
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
        // what went wrong, said as it is (#132: both were "unknown option")
        else if (argv[i][0] != '-') { fprintf(stderr, MSG_ERR_TWO_SOURCES, in_path, argv[i]); usage(1); }
        else if (!strcmp(argv[i], "-o") || !strcmp(argv[i], "--output") || !strcmp(argv[i], "-I"))
            { fprintf(stderr, MSG_ERR_OPTION_VALUE, argv[i]); usage(1); }
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
        if (bad) msg_fatal(2, MSG_ERR_FILE_NAME, in_path, nm, why[bad]);
        free(nm);
    }
    yy_scan_string(text);

    g_unit = ast_unit();
    st_init();

    // an error stops the run where it is found (yyerror and msg_error exit,
    // bison's "memory exhausted" too), so the parse returns only on success
    if (yyparse() != 0) msg_internal("the parser failed without a message");

    g_unit->prname = derive_name(in_path, "");   // the source file's name

    /* Output: everything in one folder, -o or ./<prname> (created, all
     * levels): <prname>.asm and the tables asmcomp and the waveform read. */
    if (!out_dir) out_dir = xasprintf("./%s", g_unit->prname);
    make_dir(out_dir);
    const char *tmp_dir = out_dir;
    char *outp = xasprintf("%s/%s.asm", out_dir, g_unit->prname);
    {
        const char *fmt[4] = {"%s/%s.asm", "%s/pc_%s_mem.txt", "%s/prog_log.txt%.0s", "%s/src_lines.txt%.0s"};
        for (int i = 0; i < 4; i++) {
            size_t n = strlen(out_dir) + strlen(g_unit->prname) + 32;
            out_files[i] = xmalloc(n);
            snprintf(out_files[i], n, fmt[i], out_dir, g_unit->prname);
        }
        atexit(remove_outputs);
        msg_on_internal(remove_outputs);   // abort skips atexit
    }
    FILE *fo = fopen(outp, "w");
    if (!fo) msg_fatal(1, MSG_ERR_CANT_WRITE, outp);

    codegen(fo, g_unit, tmp_dir, text);

    fclose(fo);

    // the passes on the .asm (asm_share.h, in the order they need), the same
    // as cmmcomp's but asm_fuse (the peephole fuses), asm_divk and asm_tail;
    // pc_<proc>_mem.txt and prog_log's num_ins follow what they remove
    char *pc = xasprintf("%s/pc_%s_mem.txt", tmp_dir, g_unit->prname);
    int less = asm_passes(outp, pc, 0, NULL, NULL);
    if (less != 0) lower_num_ins(tmp_dir, less);
    free(pc);

    // the folder of the source, where asmcomp reads the user's own files
    // (input_N.txt, cade_k.txt)
    char *log = xasprintf("%s/prog_log.txt", tmp_dir);
    FILE *fl = fopen(log, "a");
    free(log);
    if (fl) {
        char *src = dir_of(in_path);
        if (src) fprintf(fl, "srcdir %s\n", src);
        fprintf(fl, "lang C++\n");       // the waveform's source track takes its name
        fclose(fl);
    }
    out_done = 1;                        // compiled: what this run wrote stays
    return 0;
}
