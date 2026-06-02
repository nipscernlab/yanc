// ----------------------------------------------------------------------------
// command-line argument parsing for asmcomp ----------------------------------
// ----------------------------------------------------------------------------

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <limits.h>

#include "../Headers/args.h"
#include "../Headers/messages.h"

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

// parses a non-negative integer option value, bailing out on garbage input
static int opt_int(const char *value, const char *opt)
{
    char *end;
    errno = 0;
    long v = strtol(value, &end, 10);

    if (errno != 0 || end == value || *end != '\0' || v < 0 || v > INT_MAX)
    {
        fprintf(stderr, MSG_ERR_CLI_BAD_INT, opt, value);
        cli_help(stderr);
        exit(EXIT_FAILURE);
    }
    return (int)v;
}

void cli_parse(int argc, char **argv, cli_args *out)
{
    memset(out, 0, sizeof *out);

    for (int i = 1; i < argc; i++)
    {
        char *arg = argv[i];

        if      (!strcmp(arg, "-i") || !strcmp(arg, "--input"))
            out->input = opt_value(argc, argv, &i, arg);
        else if (!strcmp(arg, "-p") || !strcmp(arg, "--proc-dir"))
            out->proc_dir = opt_value(argc, argv, &i, arg);
        else if (!strcmp(arg, "-d") || !strcmp(arg, "--hdl-dir"))
            out->hdl_dir = opt_value(argc, argv, &i, arg);
        else if (!strcmp(arg, "-m") || !strcmp(arg, "--macros-dir"))
            out->macros_dir = opt_value(argc, argv, &i, arg);
        else if (!strcmp(arg, "-t") || !strcmp(arg, "--temp-dir"))
            out->temp_dir = opt_value(argc, argv, &i, arg);
        else if (!strcmp(arg, "-f") || !strcmp(arg, "--freq"))
            out->freq = opt_int(opt_value(argc, argv, &i, arg), arg);
        else if (!strcmp(arg, "-c") || !strcmp(arg, "--clocks"))
            out->clocks = opt_int(opt_value(argc, argv, &i, arg), arg);
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
        else
        {
            fprintf(stderr, MSG_ERR_CLI_UNKNOWN_OPT, arg);
            cli_help(stderr);
            exit(EXIT_FAILURE);
        }
    }

    // every directory/file option is mandatory; -f and -c default to 0 -------

    const char *missing = NULL;
    if      (!out->input)      missing = "-i/--input";
    else if (!out->proc_dir)   missing = "-p/--proc-dir";
    else if (!out->hdl_dir)    missing = "-d/--hdl-dir";
    else if (!out->macros_dir) missing = "-m/--macros-dir";
    else if (!out->temp_dir)   missing = "-t/--temp-dir";

    if (missing)
    {
        fprintf(stderr, MSG_ERR_CLI_MISSING, missing);
        cli_help(stderr);
        exit(EXIT_FAILURE);
    }
}
