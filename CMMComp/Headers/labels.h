// ----------------------------------------------------------------------------
// generates assembly labels for the jump instructions ------------------------
// ----------------------------------------------------------------------------

int push_if   (); // pushes a new if/else  label onto the if    stack
int  pop_if   (); // pops  the latest      label from the if    stack
int  get_if   (); // peeks the latest      label of   the if    stack (0 if empty)

int push_while(); // pushes a new while    label onto the while stack
int  pop_while(); // pops  the latest      label from the while stack
int  get_while(); // peeks the latest      label of   the while stack (0 if empty)
