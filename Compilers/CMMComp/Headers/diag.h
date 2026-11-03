// ----------------------------------------------------------------------------
// diagnostics: the one way cmmcomp stops on an error -------------------------
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

#endif // CMM_DIAG_H
