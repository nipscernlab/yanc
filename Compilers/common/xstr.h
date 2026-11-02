// ----------------------------------------------------------------------------
// Strings and tables of any length, for every YANC tool ----------------------
// ----------------------------------------------------------------------------
// A name, a line or a path comes from the user's program or command line and
// has no length the tool can know in advance: it lives in a block of its own
// exact size, never in a char[N]. Each function here either succeeds or ends
// the tool with a message (out of memory); none returns NULL on failure, so a
// caller has nothing to check.
// ----------------------------------------------------------------------------

#ifndef XSTR_H
#define XSTR_H

#include <stdio.h>
#include <stddef.h>

void *xrealloc (void *p, size_t n);                  // realloc, or the tool ends
char *xstrdup  (const char *s);                      // a copy of s
char *xasprintf(const char *fmt, ...)                // the formatted text
#if defined(__GNUC__)
    __attribute__((format(printf, 1, 2)))
#endif
    ;
char *xgetline (FILE *f);                            // the next line, any length,
                                                     // without its \r\n; NULL at
                                                     // the end of the file; the
                                                     // block is the caller's
#endif
