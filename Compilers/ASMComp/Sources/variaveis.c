// ----------------------------------------------------------------------------
// routines for handling variables found in the .asm file ---------------------
// ----------------------------------------------------------------------------

// global includes
#include  <stdio.h>
#include <string.h>
#include <stdlib.h>

// local includes
#include "../Headers/t2t.h"
#include "../Headers/messages.h"

// ----------------------------------------------------------------------------
// local variables ------------------------------------------------------------
// ----------------------------------------------------------------------------

int          v_count = 0;
static int   v_cap   = 0;
static char (*v_name)[512] = NULL;
static int  *v_val         = NULL;
static int  *v_addr        = NULL;  // the address (operand) of each entry

// PROTOTYPE (array-unit.md, step 4a): two memories, each numbered from 0
int          arr_mode = 0;
int          arr_sbit = 0;
static int   n_norm   = 0;          // words of the normal memory
static int   n_arr    = 0;          // words of the array memory

// ----------------------------------------------------------------------------
// helpers --------------------------------------------------------------------
// ----------------------------------------------------------------------------

static void var_grow(int needed)
{
    if (needed <= v_cap) return;
    int new_cap = v_cap ? v_cap : 256;
    while (new_cap < needed) new_cap *= 2;

    void *t1 = realloc(v_name, (size_t)new_cap * sizeof(*v_name));
    void *t2 = realloc(v_val , (size_t)new_cap * sizeof(*v_val ));
    void *t3 = realloc(v_addr, (size_t)new_cap * sizeof(*v_addr));
    if (!t1 || !t2 || !t3) {fprintf(stderr, MSG_ERR_OUT_OF_MEMORY); exit(EXIT_FAILURE);}

    v_name = t1;
    v_val  = t2;
    v_addr = t3;
    memset(v_name + v_cap, 0, (size_t)(new_cap - v_cap) * sizeof(*v_name));
    memset(v_val  + v_cap, 0, (size_t)(new_cap - v_cap) * sizeof(*v_val ));
    for (int i = v_cap; i < new_cap; i++) v_addr[i] = i;
    v_cap = new_cap;
}

// ----------------------------------------------------------------------------
// interface routines ---------------------------------------------------------
// ----------------------------------------------------------------------------

// adds a new variable to the table
// if the operand is a constant, converts its value to binary ...
void var_add(char *var, int is_const)
{
    var_grow(v_count + 1);

    // turn char *var into int val. val = 0 covers any is_const outside
    // the {0,1,2} domain so we never propagate uninitialized memory.
    int   val = 0;
    float delta;
    switch(is_const)
    {
        case 0: val = 0;                break; // not a constant
        case 1: val = atoi(var);        break; // int constant
        case 2: val = f2mf(var,&delta); break; // float constant
    }

    strcpy(v_name [v_count], var);
    v_val [v_count]        = val ;
    v_addr[v_count]        = arr_mode ? n_norm++ : v_count;
    v_count++;
}

// the index of a variable in the table, or -1
static int var_idx(char *val)
{
	for (int i = 0; i < v_count; i++)
		if (strcmp(val, v_name[i]) == 0) return i;
	return -1;
}

// checks whether a variable has already been used
// if so, returns its address (the index in the table, or with the array
// memory on, its address in its memory, arr_sbit set for an array)
// if not, returns -1
int var_find(char *val)
{
	int i = var_idx(val);
	return (i < 0) ? -1 : v_addr[i];
}

// PROTOTYPE: an array's first word in the array memory; var_inc adds the rest
void var_add_arr(char *var)
{
    var_grow(v_count + 1);
    strcpy(v_name[v_count], var);
    v_val [v_count] = 0;
    v_addr[v_count] = (1 << arr_sbit) | n_arr++;
    v_count++;
}

int var_is_arr (char *var){int i = var_idx(var); return arr_mode && i >= 0 && (v_addr[i] >> arr_sbit) & 1;}
int var_arr_cnt(         ){return n_arr;}

void var_inc (int   val){if (arr_mode) {n_arr += val; return;} var_grow(v_count + val); v_count += val;} // increments the memory size (for arrays)
int  var_val (char *var){return v_val[var_idx(var)];}              // returns the variable's value
int  var_cnt (         ){return arr_mode ? n_norm : v_count;}      // returns the number of variables

// #SHARE <name> <home>: <name> has no word of its own, it uses <home>'s
// (asm_share, in Compilers/common, decides which variables can). The code
// keeps using <name>; only the address it resolves to is shared.
static int    sh_count = 0;
static char (*sh_name)[512] = NULL;
static char (*sh_home)[512] = NULL;

void var_share(char *var, char *home)
{
    sh_name = realloc(sh_name, (size_t)(sh_count + 1) * sizeof(*sh_name));
    sh_home = realloc(sh_home, (size_t)(sh_count + 1) * sizeof(*sh_home));
    if (!sh_name || !sh_home) {fprintf(stderr, MSG_ERR_OUT_OF_MEMORY); exit(EXIT_FAILURE);}
    snprintf(sh_name[sh_count], sizeof(*sh_name), "%s", var );
    snprintf(sh_home[sh_count], sizeof(*sh_home), "%s", home);
    sh_count++;
}

char *var_home(char *var)
{
    for (int i = 0; i < sh_count; i++) if (strcmp(var, sh_name[i]) == 0) return sh_home[i];
    return var;
}

int var_shared(char *var)
{
    for (int i = 0; i < sh_count; i++)
        if (strcmp(var, sh_name[i]) == 0 || strcmp(var, sh_home[i]) == 0) return 1;
    return 0;
}

// adds a variable whose initial value is provided explicitly (rather than
// derived from its name). used by LEA to materialise &target as a constant.
void var_add_with_val(char *var, int val)
{
    var_grow(v_count + 1);
    strcpy(v_name [v_count], var);
    v_val [v_count] = val;
    v_addr[v_count] = arr_mode ? n_norm++ : v_count;
    v_count++;
}
