// ----------------------------------------------------------------------------
// CPPComp — compiler entry point -----------------------------------------------
// ----------------------------------------------------------------------------
// usage: cppcomp -i <input.c> [-o <output.asm>] [-n <prname>] [-t <tmp>]
// input is assumed to be already preprocessed (run cpppp first).
// ----------------------------------------------------------------------------

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "..\Headers\config.h"
#include "..\Headers\ast.h"
#include "..\Headers\symtab.h"
#include "..\Headers\codegen.h"
#include "..\Headers\messages.h"

extern int yyparse(void);
extern FILE *yyin;
extern unit *g_unit;

static void usage(void)
{
    fprintf(stderr,
        "CPPComp %s — C compiler for the YANC processor\n"
        "usage: cppcomp -i <input.c> [-o <output.asm>] [-n <prname>] [-t <tmp>]\n"
        "  -i <file>   input (preprocessed) .c file\n"
        "  -o <file>   output .asm file (default: <input>.asm)\n"
        "  -n <name>   #PRNAME embedded in output (default: input basename)\n"
        "  -t <dir>    temp directory; cppcomp writes cmm_log.txt there for asmcomp\n"
        "  -h          this help\n"
        "\n"
        "build-time defaults (override with -D when building cppcomp.exe):\n"
        "  NUBITS=%d  NBMANT=%d  NBEXPO=%d  NUGAIN=%d\n"
        "  NDSTAC=%d  SDEPTH=%d  NUIOIN=%d  NUIOOU=%d  FFTSIZ=%d\n",
        CPPCOMP_VERSION,
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
    const char *out_path = NULL;
    const char *prname   = NULL;
    const char *tmp_dir  = NULL;

    for (int i = 1; i < argc; i++) {
        if      (!strcmp(argv[i], "-i") && i+1 < argc) in_path  = argv[++i];
        else if (!strcmp(argv[i], "-o") && i+1 < argc) out_path = argv[++i];
        else if (!strcmp(argv[i], "-n") && i+1 < argc) prname   = argv[++i];
        else if (!strcmp(argv[i], "-t") && i+1 < argc) tmp_dir  = argv[++i];
        else if (!strcmp(argv[i], "-h") || !strcmp(argv[i], "--help")) usage();
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

    if (!g_unit->prname) {
        if (prname) g_unit->prname = strdup(prname);
        else        g_unit->prname = derive_name(in_path, "");
    }

    char *outp = out_path ? strdup(out_path) : derive_name(in_path, ".asm");
    FILE *fo = fopen(outp, "w");
    if (!fo) { fprintf(stderr, "cppcomp: cannot open '%s' for writing\n", outp); exit(1); }

    codegen(fo, g_unit, tmp_dir);

    fclose(fo);
    if (msg_error_count() > 0) return 2;
    return 0;
}
