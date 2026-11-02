// ----------------------------------------------------------------------------
// helpers for the predefined sqrt / atan / sin macros ------------------------
// ----------------------------------------------------------------------------

#ifndef CMM_MACROS_H
#define CMM_MACROS_H

void mac_add (char *name);                     // adds a flag for a predefined macro
void mac_copy(char *fasm);                     // copies the predefined macros at the end of the assembler file

#endif // CMM_MACROS_H
