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

#endif
