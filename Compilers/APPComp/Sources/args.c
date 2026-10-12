// ----------------------------------------------------------------------------
// command-line argument parsing for appcomp ----------------------------------
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
            out->input = arg;               // the .asm, without -i
        else
        {
            fprintf(stderr, MSG_ERR_CLI_UNKNOWN_OPT, arg);
            cli_help(stderr);
            exit(EXIT_FAILURE);
        }
    }

    // app_log.txt goes to the folder of the .asm, the output folder of the
    // program (exe_dir.h); the input is mandatory ---------------------------

    if (out->input) out->temp_dir = dir_of(out->input);

    const char *missing = NULL;
    if (!out->input) missing = "<file.asm>";

    if (missing)
    {
        fprintf(stderr, MSG_ERR_CLI_MISSING, missing);
        cli_help(stderr);
        exit(EXIT_FAILURE);
    }
}
