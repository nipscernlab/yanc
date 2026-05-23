// ----------------------------------------------------------------------------
// routines used during lexical analysis of the assembly code -----------------
// ----------------------------------------------------------------------------

void eval_init  (char      *path); // runs before the lexer
void eval_direct(int  next_state); // runs when a directive    is found
void eval_itrad (               ); // runs when an interrupt   is found
void eval_toaqui(               ); // runs when a #TOAQUI marker is found (cheguei pin trigger)
void eval_opcode(int  next_state); // runs when an opcode      is found
void eval_opernd(char        *va); // runs when an operand     is found
void eval_label (char        *va); // runs when a label        is found
void eval_finish(               ); // runs after the lexer
