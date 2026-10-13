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

        if      (!strcmp(arg, "-f") || !strcmp(arg, "--freq"))
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
        else if (arg[0] != '-' && !out->input)
            out->input = arg;               // the .asm
        else
        {
            fprintf(stderr, MSG_ERR_CLI_UNKNOWN_OPT, arg);
            cli_help(stderr);
            exit(EXIT_FAILURE);
        }
    }

    // everything goes to the folder of the .asm (absolute: the testbench names
    // its files there); the SAPHO/ and Macros/ folders are the ones next to
    // the tool (exe_dir.h); -f and -c default to a testbench that runs ------

    static const char *const hdl_dirs[]    = EXE_HDL_DIRS;
    static const char *const macros_dirs[] = EXE_MACROS_DIRS;
    out->hdl_dir    = exe_find_dir(hdl_dirs);
    out->macros_dir = exe_find_dir(macros_dirs);
    if (out->input) out->proc_dir = out->temp_dir = dir_of(out->input);
    if (out->freq   <= 0) out->freq   = 100;     // MHz
    if (out->clocks <= 0) out->clocks = 10000;   // clocks to simulate

    if (!out->input)
    {
        fprintf(stderr, MSG_ERR_CLI_MISSING, "<file.asm>");
        cli_help(stderr);
        exit(EXIT_FAILURE);
    }
    // the .asm names the processor, as the source does for the compilers
    // (whose .asm has the source's name): it has to be a Verilog module name
    out->name = path_stem(out->input);
    {
        int bad = proc_name_problem(out->name);
        const char *why = bad == 1 ? MSG_PROC_NAME_CHARS   : bad == 2 ? MSG_PROC_NAME_DIGIT :
                          bad == 3 ? MSG_PROC_NAME_UNDERSC : MSG_PROC_NAME_KEYWORD;
        if (bad) {fprintf(stderr, MSG_ERR_PROC_NAME, out->input, out->name, why); exit(EXIT_FAILURE);}
    }
    if (!out->hdl_dir)    {fprintf(stderr, MSG_ERR_NO_TOOL_DIR, "SAPHO" ); exit(EXIT_FAILURE);}
    if (!out->macros_dir) {fprintf(stderr, MSG_ERR_NO_TOOL_DIR, "Macros"); exit(EXIT_FAILURE);}
}
