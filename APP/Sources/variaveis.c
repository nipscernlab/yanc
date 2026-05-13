// ----------------------------------------------------------------------------
// variable table -------------------------------------------------------------
// ----------------------------------------------------------------------------

// global includes
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "..\Headers\messages.h"

#define NVARMAX 999999 // switch to a dynamic array later

int  v_count = 0;
char v_name[NVARMAX][512];

// helper functions -----------------------------------------------------------

// checks whether a variable has already been used
// if so, returns its index in the table
// if not, returns -1
int var_find(char *val)
{
	int ind = -1;

	for (int i = 0; i < v_count; i++)
		if (strcmp(val, v_name[i]) == 0) {ind = i; break;}

	return ind;
}

// global interface functions -------------------------------------------------

// adds a new variable to the table
// may be a vector with size > 1
void var_add(char *va, int size)
{
    if (var_find(va) == -1)
    {
        strcpy(v_name[v_count], va);
        v_count += size;
    }

    if (v_count > NVARMAX) {fprintf(stderr, MSG_ERR_TOO_MANY_VARS, NVARMAX); exit(EXIT_FAILURE);}
}

// returns the number of variables
int var_cnt(void)
{
    return v_count;
}
