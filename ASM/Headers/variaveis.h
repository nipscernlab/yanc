// ----------------------------------------------------------------------------
// routines for handling variables found in the .asm file ---------------------
// ----------------------------------------------------------------------------

void var_add (char *var, int val);  // adds a variable to the table
void var_add_with_val(char *var, int val); // adds a variable with explicit value (for LEA pseudo)
void var_inc (int   val);           // increments the memory size (for arrays)
int  var_find(char *val);           // returns the variable's index in the table
int  var_val (char *var);           // returns the variable's value
int  var_cnt ();                    // returns the number of variables
