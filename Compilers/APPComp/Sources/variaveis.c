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

// #SHARE <name> <home>: <name> has no word of its own, it uses <home>'s
// (asm_share, in Compilers/common, decides which variables can)
static int    sh_count = 0;
static char (*sh_name)[512] = NULL;
static char (*sh_home)[512] = NULL;

static char *var_home(char *va)
{
    for (int i = 0; i < sh_count; i++)
        if (strcmp(va, sh_name[i]) == 0) return sh_home[i];
    return va;
}

// global interface functions -------------------------------------------------

// records that va shares home's word
void var_share(char *va, char *home)
{
    sh_name = realloc(sh_name, (size_t)(sh_count + 1) * sizeof(*sh_name));
    sh_home = realloc(sh_home, (size_t)(sh_count + 1) * sizeof(*sh_home));
    if (!sh_name || !sh_home) {fprintf(stderr, MSG_ERR_OUT_OF_MEMORY); exit(EXIT_FAILURE);}
    snprintf(sh_name[sh_count], sizeof(*sh_name), "%s", va  );
    snprintf(sh_home[sh_count], sizeof(*sh_home), "%s", home);
    sh_count++;
}

// adds a new variable to the table
// may be a vector with size > 1
void var_add(char *va, int size)
{
    va = var_home(va); // a shared name takes its home's word

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
