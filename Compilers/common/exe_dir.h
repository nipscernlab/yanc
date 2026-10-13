// exe_dir -- the folders a tool finds next to its own executable.
//
// A release unpacks as bin/ beside SAPHO/, Macros/ and Header/; a repository
// build puts the tools in bin/ (or .smoke/bin/, stage/bin/) beside Compilers/
// and SAPHO/. A tool started from anywhere (on the PATH) looks for its data
// folders relative to where its executable is, with no option to name them;
// the driver yanc finds the other tools the same way.

#ifndef EXE_DIR_H
#define EXE_DIR_H

// The first of the candidate folders, each relative to the executable's own
// folder ("../Macros", ...; a NULL ends the list), that exists, as a path with
// '/' separators. NULL when none does. The string is the caller's (malloc).
char *exe_find_dir(const char *const *candidates);

// The executable's own folder, '/' separators, no trailing '/'; "" if the
// system does not say. Static storage.
const char *exe_dir(void);

// Path helpers the tools share for their defaults.
int   make_dir   (const char *path);       // creates a folder; 0 if it exists now
char *path_stem  (const char *path);       // "a/b/prog.cmm" -> "prog" (malloc)
char *abs_path   (const char *path);       // the absolute form, '/' separators (malloc); the path itself if the system can't say
char *dir_of     (const char *path);       // the absolute folder of a file, '/' separators (malloc)

// Where each tool looks, in order: a release (bin/ beside the folder), a
// repository build in bin/, and one in .smoke/bin/ or stage/bin/.
#define EXE_MACROS_DIRS {"../Macros", "../Compilers/CMMComp/Includes", "../../Compilers/CMMComp/Includes", NULL}
#define EXE_HDL_DIRS    {"../SAPHO",  "../../SAPHO", NULL}
#define EXE_HEADER_DIRS {"../Header", "../Compilers/CPPComp/Includes", "../../Compilers/CPPComp/Includes", NULL}

#endif
