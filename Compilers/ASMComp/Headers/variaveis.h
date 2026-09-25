// ----------------------------------------------------------------------------
// routines for handling variables found in the .asm file ---------------------
// ----------------------------------------------------------------------------

void var_add (char *var, int val);  // adds a variable to the table
void var_add_with_val(char *var, int val); // adds a variable with explicit value (for LEA pseudo)
void var_inc (int   val);           // increments the memory size (for arrays)
int  var_find(char *val);           // returns the variable's index in the table
int  var_val (char *var);           // returns the variable's value
int  var_cnt ();                    // returns the number of variables
void  var_share (char *var, char *home); // #SHARE: var lives in home's word
char *var_home  (char *var);             // the name whose word var uses (var itself if not shared)
int   var_shared(char *var);             // var is in a #SHARE group (as a name or as a home)
