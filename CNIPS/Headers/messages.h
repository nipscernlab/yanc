// ----------------------------------------------------------------------------
// CNIPS — diagnostics --------------------------------------------------------
// ----------------------------------------------------------------------------

#ifndef CNIPS_MESSAGES_H
#define CNIPS_MESSAGES_H

void msg_set_file(const char *name);
void msg_error    (int line, const char *fmt, ...);    // prints and exits(1)
void msg_warning  (int line, const char *fmt, ...);
void msg_internal (const char *fmt, ...);              // compiler-internal bug; abort
int  msg_error_count(void);

#endif
