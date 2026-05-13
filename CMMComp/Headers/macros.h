// ----------------------------------------------------------------------------
// functions and variables used to create and use macros ----------------------
// ----------------------------------------------------------------------------

// user macro usage -----------------------------------------------------------

extern int mac_using;                          // while reading a macro, the assembler must not be written during the parse

void mac_use(int ids, int global, int id_num); // enables user macro usage
void mac_end();                                // macro end point

// predefined macro usage -----------------------------------------------------

void mac_add (char *name);                     // adds a flag for a predefined macro
void mac_copy(char *fasm);                     // copies the predefined macros at the end of the assembler file
