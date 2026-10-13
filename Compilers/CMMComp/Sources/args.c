// ----------------------------------------------------------------------------
// command-line argument parsing for cmmcomp ----------------------------------
// ----------------------------------------------------------------------------

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../Headers/args.h"
#include "../Headers/messages.h"
#include "../../common/exe_dir.h"
#include "../../common/proc_name.h"

// prints the full help text to 'stream'
static void cli_help(FILE *stream)
{
    fprintf(stream, MSG_CLI_HELP, YANC_VERSION);
}

// returns the value attached to an option, consuming the next argv slot.
// bails out with a usage message when the value is missing.
static char *opt_value(int argc, char **argv, int *i, const char *opt)
{
    if (*i + 1 >= argc)
    {
        fprintf(stderr, MSG_ERR_CLI_NEEDS_VALUE, opt);
        cli_help(stderr);
        exit(EXIT_FAILURE);
    }
    return argv[++(*i)];
}

void cli_parse(int argc, char **argv, cli_args *out)
{
    memset(out, 0, sizeof *out);

    for (int i = 1; i < argc; i++)
    {
        char *arg = argv[i];

        if      (!strcmp(arg, "-o") || !strcmp(arg, "--output"))
            out->proc_dir = opt_value(argc, argv, &i, arg);
        else if (!strcmp(arg, "--wave-arrays"))
            out->array_sim = 1;
        else if (!strcmp(arg, "-h") || !strcmp(arg, "--help"))
        {
            cli_help(stdout);
            exit(EXIT_SUCCESS);
        }
        else if (!strcmp(arg, "-V") || !strcmp(arg, "--version"))
        {
            printf(MSG_CLI_VERSION, YANC_VERSION);
            exit(EXIT_SUCCESS);
        }
        else if (arg[0] != '-' && !out->input)
            out->input = arg;               // the source file
        else
        {
            fprintf(stderr, MSG_ERR_CLI_UNKNOWN_OPT, arg);
            cli_help(stderr);
            exit(EXIT_FAILURE);
        }
    }

    // the processor takes the name of the source file; the Macros/ folder is
    // the one next to the tool (exe_dir.h); -o defaults to the folder
    // ./<name>, where everything goes (the .asm and the tables) -------------

    static const char *const macros_dirs[] = EXE_MACROS_DIRS;
    out->macros_dir = exe_find_dir(macros_dirs);
    if (out->input) out->name = path_stem(out->input);
    // the name becomes a Verilog module's: refuse one that cannot be
    if (out->name)
    {
        int bad = proc_name_problem(out->name);
        const char *why = bad == 1 ? MSG_PROC_NAME_CHARS   : bad == 2 ? MSG_PROC_NAME_DIGIT :
                          bad == 3 ? MSG_PROC_NAME_UNDERSC : MSG_PROC_NAME_KEYWORD;
        if (bad) {fprintf(stderr, MSG_ERR_PROC_NAME, out->input, out->name, why); exit(EXIT_FAILURE);}
    }
    if (!out->proc_dir && out->name)
    {
        out->proc_dir = malloc(strlen(out->name) + 3);
        if (out->proc_dir) sprintf(out->proc_dir, "./%s", out->name);
    }
    if (out->proc_dir) {make_dir(out->proc_dir); out->temp_dir = out->proc_dir;}

    if (!out->input)
    {
        fprintf(stderr, MSG_ERR_CLI_MISSING, "<file.cmm>");
        cli_help(stderr);
        exit(EXIT_FAILURE);
    }
    if (!out->macros_dir) {fprintf(stderr, MSG_ERR_NO_TOOL_DIR, "Macros"); exit(EXIT_FAILURE);}
}
