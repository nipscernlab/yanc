// ----------------------------------------------------------------------------
// CPPComp — the C preprocessor (cpppp.c) ---------------------------------------
// ----------------------------------------------------------------------------

#ifndef CPPPP_H
#define CPPPP_H

// adds an include folder, searched in the order given, before Header/
void        pp_add_incdir(const char *dir);

// preprocesses in_path: the whole text, in memory (exits on an error)
const char *pp_run(const char *in_path);

// the source line that line `line` of the text came from; *file is set to
// its file (left as it was for a line outside the text)
int         pp_where(int line, const char **file);

// line `line` of the text as it was written (macros not expanded, newline
// kept), or NULL outside the text
const char *pp_source(int line);

// a C escape: *p is past the backslash, and ends past the escape. The value
// (0 to 255), PP_ESC_BAD for an escape C does not have (*p left on it), or
// PP_ESC_RANGE for \x or octal past 255. The lexer reads char and string
// literals with it, and #if its char constants, so the two agree.
#define PP_ESC_BAD   (-1)
#define PP_ESC_RANGE (-2)
int         pp_escape(const char **p);

#endif
