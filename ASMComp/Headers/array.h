// ----------------------------------------------------------------------------
// assembly array handling ----------------------------------------------------
// ----------------------------------------------------------------------------

#include <stdio.h>

// adds an array to data memory
// for a regular array (f_name = ""), fills it with zeros
// for an initialized array, calls fill_mem to populate it
void arr_add(int size, int type, char *f_name, FILE *f_data);
