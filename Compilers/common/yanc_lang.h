// ----------------------------------------------------------------------------
// The message language of every YANC tool ------------------------------------
// ----------------------------------------------------------------------------
// English by default; the environment variable YANC_LANG (en or pt) changes
// the default, and `--lang=en|pt` (or `--lang en|pt`) on the command line
// wins over both.
// ----------------------------------------------------------------------------

#ifndef YANC_LANG_H
#define YANC_LANG_H

// reads YANC_LANG and takes every --lang option out of argv (the rest keeps
// its order); returns 1 for English, 0 for Portuguese. A value other than en
// or pt is an error: the tool named `tool` says so and exits.
int yanc_lang(int *argc, char **argv, const char *tool);

// what yanc_lang() chose (1 English, 0 Portuguese), for code shared by the
// tools that prints on its own (asm_share.c)
extern int yanc_lang_en;

#endif
