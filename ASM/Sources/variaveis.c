// ----------------------------------------------------------------------------
// routines for handling variables found in the .asm file ---------------------
// ----------------------------------------------------------------------------

#define NVARMAX 999999 // switch to dynamic arrays later

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

int  v_count = 0;
char v_name[NVARMAX][512];
int  v_val [NVARMAX];

// ----------------------------------------------------------------------------
// interface routines ---------------------------------------------------------
// ----------------------------------------------------------------------------

// adds a new variable to the table
// if the operand is a constant, converts its value to binary ...
void var_add(char *var, int is_const)
{
    if (v_count == NVARMAX)
    {
        fprintf(stderr, MSG_ERR_TOO_MANY_VARS, NVARMAX);
        exit(EXIT_FAILURE);
    }

    // turn char *var into int val
    int   val;
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

void var_inc (int   val){v_count += val             ;} // increments the memory size (for arrays)
int  var_val (char *var){return v_val[var_find(var)];} // returns the variable's value
int  var_cnt (         ){return v_count             ;} // returns the number of variables
