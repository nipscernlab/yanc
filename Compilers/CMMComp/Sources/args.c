// ----------------------------------------------------------------------------
// command-line argument parsing for cmmcomp ----------------------------------
// ----------------------------------------------------------------------------

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../Headers/args.h"
#include "../Headers/messages.h"
#include "../../common/exe_dir.h"

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

        if      (!strcmp(arg, "-i") || !strcmp(arg, "--input"))
            out->input = opt_value(argc, argv, &i, arg);
        else if (!strcmp(arg, "-n") || !strcmp(arg, "--name"))
            out->name = opt_value(argc, argv, &i, arg);
        else if (!strcmp(arg, "-o") || !strcmp(arg, "--output"))
            out->proc_dir = opt_value(argc, argv, &i, arg);
        else if (!strcmp(arg, "-m") || !strcmp(arg, "--macros-dir"))
            out->macros_dir = opt_value(argc, argv, &i, arg);
        else if (!strcmp(arg, "-A") || !strcmp(arg, "--array"))
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
            out->input = arg;               // the source file, without -i
        else
        {
            fprintf(stderr, MSG_ERR_CLI_UNKNOWN_OPT, arg);
            cli_help(stderr);
            exit(EXIT_FAILURE);
        }
    }

    // defaults (exe_dir.h): -m the Macros/ folder next to the tool, -n the
    // source file's name, -o the folder ./<name>, where everything goes (the
    // .asm and the tables); the source file is mandatory --------------------

    static const char *const macros_dirs[] = EXE_MACROS_DIRS;
    if (!out->macros_dir) out->macros_dir = exe_find_dir(macros_dirs);
    if (!out->name && out->input) out->name = path_stem(out->input);
    if (!out->proc_dir && out->name)
    {
        out->proc_dir = malloc(strlen(out->name) + 3);
        if (out->proc_dir) sprintf(out->proc_dir, "./%s", out->name);
    }
    if (out->proc_dir) {make_dir(out->proc_dir); out->temp_dir = out->proc_dir;}

    const char *missing = NULL;
    if      (!out->input)      missing = "<file.cmm>";
    else if (!out->macros_dir) missing = "-m/--macros-dir";

    if (missing)
    {
        fprintf(stderr, MSG_ERR_CLI_MISSING, missing);
        cli_help(stderr);
        exit(EXIT_FAILURE);
    }
}
