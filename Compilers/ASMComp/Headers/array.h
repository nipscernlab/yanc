// ----------------------------------------------------------------------------
// assembly array handling ----------------------------------------------------
// ----------------------------------------------------------------------------

#include <stdio.h>

// adds an array to data memory
// for a regular array (f_name = ""), fills it with zeros
// for an initialized array, calls fill_mem to populate it
void arr_add(int size, int type, char *f_name, FILE *f_data);

// #arrayd <name> <type> <size> <n> <v0> ... <v(n-1)>: an array whose first n
// words are given on the line (the compilers' brace initializers and string
// literals), the rest zero. arrd_start takes the header, arrd_value each
// value; it returns 1 while values are still expected.
int arrd_start(char *name, int size, int type, int n, FILE *f_data);
int arrd_value(char *va, FILE *f_data);
