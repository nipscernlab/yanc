// ----------------------------------------------------------------------------
// variable table -------------------------------------------------------------
// ----------------------------------------------------------------------------

void var_add(char *va, int size); // adds variables and arrays
int  var_cnt();                   // returns the total number of variables
void var_share(char *va, char *home); // #SHARE: va lives in home's word
