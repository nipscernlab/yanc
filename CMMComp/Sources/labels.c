// ----------------------------------------------------------------------------
// generates assembly labels for the jump instructions ------------------------
// ----------------------------------------------------------------------------

#define LABMAX 99999

#include  <stdio.h>
#include <stdlib.h>

#include "..\Headers\messages.h"

// ----------------------------------------------------------------------------
// local variables ------------------------------------------------------------
// ----------------------------------------------------------------------------

int stk_ind = 0;
int lab_cnt = 0;
int lab_stk[LABMAX];
int lab_typ[LABMAX]; // 0 for if/else and 1 for while

// ----------------------------------------------------------------------------
// interface functions --------------------------------------------------------
// ----------------------------------------------------------------------------

int push_lab(int typ)
{
    lab_cnt++;

    if (lab_cnt == LABMAX)
    {
        fprintf (stderr, MSG_ERR_TOO_MANY_LABELS, LABMAX);
        exit(EXIT_FAILURE);
    }

    lab_stk[stk_ind] = lab_cnt;
    lab_typ[stk_ind] = typ;
    stk_ind++;
    return lab_cnt;
}

int pop_lab()
{
    stk_ind--;
    return lab_stk[stk_ind];
}

int get_lab()
{
    return lab_stk[stk_ind-1];
}

// returns the index of the most recent while on the stack (or 0 if none)
int get_while()
{
    int i = stk_ind-1;
    while ((lab_typ[i] != 1) && (i >= 0)) i--;
    return i+1;
}

// returns the index of the most recent if/else on the stack (or 0 if none)
int get_if()
{
    int i = stk_ind-1;
    while ((lab_typ[i] != 0) && (i >= 0)) i--;
    return i+1;
}
