// ----------------------------------------------------------------------------
// routines for handling variables found in the .asm file ---------------------
// ----------------------------------------------------------------------------

// global includes
#include  <stdio.h>
#include <string.h>
#include <stdlib.h>

// local includes
#include "..\Headers\t2t.h"
#include "..\Headers\messages.h"

// ----------------------------------------------------------------------------
// local variables ------------------------------------------------------------
// ----------------------------------------------------------------------------

int          v_count = 0;
static int   v_cap   = 0;
static char (*v_name)[512] = NULL;
static int  *v_val         = NULL;

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
    if (!t1 || !t2) {fprintf(stderr, MSG_ERR_OUT_OF_MEMORY); exit(EXIT_FAILURE);}

    v_name = t1;
    v_val  = t2;
    memset(v_name + v_cap, 0, (size_t)(new_cap - v_cap) * sizeof(*v_name));
    memset(v_val  + v_cap, 0, (size_t)(new_cap - v_cap) * sizeof(*v_val ));
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
    v_count++;
}

// checks whether a variable has already been used
// if so, returns its index in the table
// if not, returns -1
int var_find(char *val)
{
	int i, ind = -1;

	for (i = 0; i < v_count; i++)
		if (strcmp(val, v_name[i]) == 0)
		{
			ind = i;
			break;
		}
	return ind;
}

void var_inc (int   val){var_grow(v_count + val); v_count += val;} // increments the memory size (for arrays)
int  var_val (char *var){return v_val[var_find(var)];}             // returns the variable's value
int  var_cnt (         ){return v_count             ;}             // returns the number of variables

// adds a variable whose initial value is provided explicitly (rather than
// derived from its name). used by LEA to materialise &target as a constant.
void var_add_with_val(char *var, int val)
{
    var_grow(v_count + 1);
    strcpy(v_name [v_count], var);
    v_val [v_count] = val;
    v_count++;
}
