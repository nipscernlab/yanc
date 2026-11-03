// ----------------------------------------------------------------------------
// diagnostics: errors that stop cmmcomp, warnings and notes ------------------
// ----------------------------------------------------------------------------
// fmt is a MSG_* of messages.h; its arguments (the line number among them)
// are the ones the message takes, in its order.
// ----------------------------------------------------------------------------

#ifndef CMM_DIAG_H
#define CMM_DIAG_H

// prints the message on stderr and ends cmmcomp with EXIT_FAILURE
void fatal(const char *fmt, ...)
#if defined(__GNUC__)
    __attribute__((noreturn, format(printf, 1, 2)))
#endif
    ;

// prints a warning or a note on stdout (where golden_map.txt records it)
void warn(const char *fmt, ...)
#if defined(__GNUC__)
    __attribute__((format(printf, 1, 2)))
#endif
    ;

#endif // CMM_DIAG_H
