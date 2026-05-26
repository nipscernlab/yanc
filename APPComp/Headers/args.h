// ----------------------------------------------------------------------------
// command-line argument parsing for appcomp ----------------------------------
// ----------------------------------------------------------------------------
// usage:
//   cli_args a;
//   parse_lang_flag(&argc, argv);  // must run first (selects the message language)
//   cli_parse(argc, argv, &a);     // fills 'a' or exits with a usage message
// ----------------------------------------------------------------------------

#ifndef ARGS_H
#define ARGS_H

#include "../../yanc_version.h"   // version string reported by --version

// parsed command-line arguments for appcomp
typedef struct
{
    char *input;    // -i  assembly source file (full path)
    char *temp_dir; // -t  Temp directory (holds app_log.txt)
} cli_args;

// parses argv into 'out'. on a missing/unknown/incomplete option it prints a
// usage message and exits; --help and --version also print and exit(0).
void cli_parse(int argc, char **argv, cli_args *out);

#endif
