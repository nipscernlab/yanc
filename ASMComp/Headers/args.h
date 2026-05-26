// ----------------------------------------------------------------------------
// command-line argument parsing for asmcomp ----------------------------------
// ----------------------------------------------------------------------------
// usage:
//   cli_args a;
//   parse_lang_flag(&argc, argv);  // must run first (selects the message language)
//   cli_parse(argc, argv, &a);     // fills 'a' or exits with a usage message
// ----------------------------------------------------------------------------

#ifndef ARGS_H
#define ARGS_H

#include "../../yanc_version.h"   // version string reported by --version

// parsed command-line arguments for asmcomp
typedef struct
{
    char *input;      // -i  assembly source file (full path)
    char *proc_dir;   // -p  processor directory
    char *hdl_dir;    // -d  HDL directory
    char *macros_dir; // -m  Macros directory
    char *temp_dir;   // -t  Temp directory
    int   freq;       // -f  processor operating frequency in MHz
    int   clocks;     // -c  number of clocks to simulate
} cli_args;

// parses argv into 'out'. on a missing/unknown/incomplete/non-numeric option
// it prints a usage message and exits; --help and --version print and exit(0).
void cli_parse(int argc, char **argv, cli_args *out);

#endif
