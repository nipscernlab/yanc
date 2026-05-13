// ----------------------------------------------------------------------------
// routines for jump implementation -------------------------------------------
// ----------------------------------------------------------------------------

// if/else --------------------------------------------------------------------

void    if_exp (int et);             // if start
void    if_stmt();                   // before the if stmts
void  else_stmt();                   // before the else stmts
void    if_fim ();                   // end of if/else

// while ----------------------------------------------------------------------

void  while_expp();                  // the while keyword alone - emits a label here
void  while_expexp(int  et);         // evaluates exp and emits a JIZ to decide whether to enter or not
void  while_stmt();                  // end of while. Emits a JMP back to the start and a label for the end right below
void  exec_break();                  // emits a JMP to the end of the while

// switch/case ----------------------------------------------------------------

void   case_test (int id, int type); // tests whether this is the correct case
void defaut_test ();                 // tests whether this is the default
void switch_break();                 // a switch/case break was found
void exec_switch (int et);           // switch/case start
void  end_switch ();                 // switch/case end
