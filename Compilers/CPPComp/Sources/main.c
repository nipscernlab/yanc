// ----------------------------------------------------------------------------
// CPPComp — compiler entry point -----------------------------------------------
// ----------------------------------------------------------------------------
// usage: cppcomp -i <input.c> -p <proc_dir> [-n <prname>] [-t <tmp>]
// input is assumed to be already preprocessed (run cpppp first).
//
// Mirrors the cmmcomp pipeline layout: the .asm lands at
// <proc_dir>/Software/<prname>.asm, with Software/ created if missing.
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

/* Create a directory if it doesn't already exist. Used by the -p
 * proc-folder mode to mkdir <proc>/Software when missing. Tolerates
 * EEXIST so reruns don't fail. */
static void ensure_dir(const char *path) {
#ifdef _WIN32
    if (mkdir(path) != 0 && errno != EEXIST) {
        fprintf(stderr, "cppcomp: warning: could not create %s (errno=%d)\n",
                path, errno);
    }
#else
    if (mkdir(path, 0755) != 0 && errno != EEXIST) {
        fprintf(stderr, "cppcomp: warning: could not create %s (errno=%d)\n",
                path, errno);
    }
#endif
}

extern int yyparse(void);
extern FILE *yyin;
extern unit *g_unit;

static void usage(void)
{
    fprintf(stderr,
        "CPPComp %s — C compiler for the YANC processor\n"
        "usage: cppcomp -i <input.c> -p <proc_dir> [-n <prname>] [-t <tmp>]\n"
        "  -i, --input    <file>   input (preprocessed) .c file\n"
        "  -p, --proc-dir <dir>    writes .asm to <dir>/Software/<prname>.asm\n"
        "                          (creates Software/ if missing). Required.\n"
        "  -n, --name     <name>   #PRNAME embedded in output (default: input basename)\n"
        "  -t, --temp-dir <dir>    temp directory; cppcomp writes cmm_log.txt there for asmcomp\n"
        "  -h, --help              this help\n"
        "  -V, --version           show version and exit\n"
        "\n"
        "build-time defaults (override with -D when building cppcomp.exe):\n"
        "  NUBITS=%d  NBMANT=%d  NBEXPO=%d  NUGAIN=%d\n"
        "  NDSTAC=%d  SDEPTH=%d  NUIOIN=%d  NUIOOU=%d  FFTSIZ=%d\n",
        YANC_VERSION,
        CFG_NUBITS, CFG_NBMANT, CFG_NBEXPO, CFG_NUGAIN,
        CFG_NDSTAC, CFG_SDEPTH, CFG_NUIOIN, CFG_NUIOOU, CFG_FFTSIZ);
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
    const char *in_path  = NULL;
    const char *proc_dir = NULL;
    const char *prname   = NULL;
    const char *tmp_dir  = NULL;

    for (int i = 1; i < argc; i++) {
        if      ((!strcmp(argv[i], "-i") || !strcmp(argv[i], "--input"   )) && i+1 < argc) in_path  = argv[++i];
        else if ((!strcmp(argv[i], "-p") || !strcmp(argv[i], "--proc-dir")) && i+1 < argc) proc_dir = argv[++i];
        else if ((!strcmp(argv[i], "-n") || !strcmp(argv[i], "--name"    )) && i+1 < argc) prname   = argv[++i];
        else if ((!strcmp(argv[i], "-t") || !strcmp(argv[i], "--temp-dir")) && i+1 < argc) tmp_dir  = argv[++i];
        else if ( !strcmp(argv[i], "-h") || !strcmp(argv[i], "--help"    )) usage();
        else if ( !strcmp(argv[i], "-V") || !strcmp(argv[i], "--version" )) {
            printf("cppcomp %s\n", YANC_VERSION);
            return 0;
        }
        else { fprintf(stderr, "cppcomp: unknown option '%s'\n", argv[i]); usage(); }
    }
    if (!in_path || !proc_dir) usage();

    msg_set_file(in_path);

    yyin = fopen(in_path, "r");
    if (!yyin) { fprintf(stderr, "cppcomp: cannot open '%s'\n", in_path); exit(1); }

    g_unit = ast_unit();
    st_init();

    if (yyparse() != 0) { fprintf(stderr, "cppcomp: parse failed\n"); exit(2); }
    fclose(yyin);

    if (!g_unit->prname) {
        if (prname) g_unit->prname = strdup(prname);
        else        g_unit->prname = derive_name(in_path, "");
    }

    /* Output: <proc_dir>/Software/<prname>.asm, creating Software/ if missing.
     * Mirrors the cmmcomp pipeline layout. */
    size_t plen = strlen(proc_dir);
    size_t nlen = strlen(g_unit->prname);
    char *outp = malloc(plen + nlen + 32);   // "/Software/" + ".asm" + slack
    sprintf(outp, "%s/Software", proc_dir);
    ensure_dir(outp);
    sprintf(outp, "%s/Software/%s.asm", proc_dir, g_unit->prname);
    FILE *fo = fopen(outp, "w");
    if (!fo) { fprintf(stderr, "cppcomp: cannot open '%s' for writing\n", outp); exit(1); }

    codegen(fo, g_unit, tmp_dir);

    fclose(fo);
    if (msg_error_count() > 0) return 2;
    return 0;
}
