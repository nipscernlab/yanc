// ----------------------------------------------------------------------------
// generates assembly labels for the jump instructions ------------------------
// ----------------------------------------------------------------------------

int  push_lab(int type); // pushes a label onto the stack
int   pop_lab();         // pops the latest label from the stack
int   get_lab();         // reads the latest label index
int get_while();         // gets the latest while index
int get_if   ();         // gets the latest if    index
