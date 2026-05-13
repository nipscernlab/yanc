// ----------------------------------------------------------------------------
// generates assembly labels for the jump instructions ------------------------
// ----------------------------------------------------------------------------

#include  <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "..\Headers\messages.h"

// ----------------------------------------------------------------------------
// local variables ------------------------------------------------------------
// ----------------------------------------------------------------------------

int  stk_ind  = 0;
int  lab_cnt  = 0;
static int  stk_cap  = 0;          // capacity of lab_stk / lab_typ
static int *lab_stk  = NULL;
static int *lab_typ  = NULL;       // 0 for if/else and 1 for while

// ----------------------------------------------------------------------------
// helpers --------------------------------------------------------------------
// ----------------------------------------------------------------------------

static void stk_grow(int needed)
{
    if (needed <= stk_cap) return;
    int new_cap = stk_cap ? stk_cap : 64;
    while (new_cap < needed) new_cap *= 2;

    int *t1 = realloc(lab_stk, (size_t)new_cap * sizeof(*lab_stk));
    int *t2 = realloc(lab_typ, (size_t)new_cap * sizeof(*lab_typ));
    if (!t1 || !t2) {fprintf(stderr, MSG_ERR_OUT_OF_MEMORY); exit(EXIT_FAILURE);}

    lab_stk = t1;
    lab_typ = t2;
    memset(lab_stk + stk_cap, 0, (size_t)(new_cap - stk_cap) * sizeof(*lab_stk));
    memset(lab_typ + stk_cap, 0, (size_t)(new_cap - stk_cap) * sizeof(*lab_typ));
    stk_cap = new_cap;
}

// ----------------------------------------------------------------------------
// interface functions --------------------------------------------------------
// ----------------------------------------------------------------------------

int push_lab(int typ)
{
    lab_cnt++;

    stk_grow(stk_ind + 1);

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
