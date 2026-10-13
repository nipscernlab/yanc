// ----------------------------------------------------------------------------
// command-line argument parsing for cmmcomp ----------------------------------
// ----------------------------------------------------------------------------
// usage:
//   cli_args a;
//   parse_lang_flag(&argc, argv);  // must run first (selects the message language)
//   cli_parse(argc, argv, &a);     // fills 'a' or exits with a usage message
// ----------------------------------------------------------------------------

#ifndef ARGS_H
#define ARGS_H

#include "../../yanc_version.h"   // version string reported by --version

// parsed command-line arguments for cmmcomp
typedef struct
{
    char *input;      //     the C± source (the argument without an option)
    char *name;       //     processor name: the source file's (base name of the .asm)
    char *proc_dir;   // -o  output folder (default ./<name>): the .asm and the tables
    char *macros_dir; //     the Macros folder next to the tool
    char *temp_dir;   //     the same folder (the tables)
    int   array_sim;  // --wave-arrays: each array element as its own GTKWave signal
} cli_args;

// parses argv into 'out'. on a missing/unknown/incomplete option it prints a
// usage message and exits; --help and --version also print and exit(0).
void cli_parse(int argc, char **argv, cli_args *out);

#endif
