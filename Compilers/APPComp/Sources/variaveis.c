// ----------------------------------------------------------------------------
// variable table -------------------------------------------------------------
// ----------------------------------------------------------------------------

// global includes
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "../Headers/messages.h"

int          v_count = 0;
static int   v_cap   = 0;
static char (*v_name)[512] = NULL;

static void var_grow(int needed)
{
    if (needed <= v_cap) return;
    int new_cap = v_cap ? v_cap : 256;
    while (new_cap < needed) new_cap *= 2;

    void *t = realloc(v_name, (size_t)new_cap * sizeof(*v_name));
    if (!t) {fprintf(stderr, MSG_ERR_OUT_OF_MEMORY); exit(EXIT_FAILURE);}

    v_name = t;
    memset(v_name + v_cap, 0, (size_t)(new_cap - v_cap) * sizeof(*v_name));
    v_cap = new_cap;
}

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
        var_grow(v_count + size);
        strcpy(v_name[v_count], va);
        v_count += size;
    }
}

// returns the number of variables
int var_cnt(void)
{
    return v_count;
}
