// ----------------------------------------------------------------------------
// generates assembly labels for the jump instructions ------------------------
// ----------------------------------------------------------------------------

#include  <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../Headers/messages.h"

// ----------------------------------------------------------------------------
// local variables ------------------------------------------------------------
// ----------------------------------------------------------------------------

static int  if_cnt    = 0;          // label counter for the if    stack
static int  if_ind    = 0;          // top index   of the if    stack
static int  if_cap    = 0;          // capacity    of the if    stack
static int *if_stk    = NULL;

static int  wh_cnt    = 0;          // label counter for the while stack
static int  wh_ind    = 0;          // top index   of the while stack
static int  wh_cap    = 0;          // capacity    of the while stack
static int *wh_stk    = NULL;

// ----------------------------------------------------------------------------
// helpers --------------------------------------------------------------------
// ----------------------------------------------------------------------------

static void stk_grow(int **stk, int *cap, int needed)
{
    if (needed <= *cap) return;
    int new_cap = *cap ? *cap : 64;
    while (new_cap < needed) new_cap *= 2;

    int *t = realloc(*stk, (size_t)new_cap * sizeof(**stk));
    if (!t) {fprintf(stderr, MSG_ERR_OUT_OF_MEMORY); exit(EXIT_FAILURE);}

    *stk = t;
    memset(*stk + *cap, 0, (size_t)(new_cap - *cap) * sizeof(**stk));
    *cap = new_cap;
}

// ----------------------------------------------------------------------------
// if/else stack --------------------------------------------------------------
// ----------------------------------------------------------------------------

int push_if()
{
    if_cnt++;
    stk_grow(&if_stk, &if_cap, if_ind + 1);
    if_stk[if_ind++] = if_cnt;
    return if_cnt;
}

int pop_if()
{
    return if_stk[--if_ind];
}

// returns the top of the if stack (or 0 if empty)
int get_if()
{
    if (if_ind == 0) return 0;
    return if_stk[if_ind-1];
}

// ----------------------------------------------------------------------------
// while stack ----------------------------------------------------------------
// ----------------------------------------------------------------------------

int push_while()
{
    wh_cnt++;
    stk_grow(&wh_stk, &wh_cap, wh_ind + 1);
    wh_stk[wh_ind++] = wh_cnt;
    return wh_cnt;
}

int pop_while()
{
    return wh_stk[--wh_ind];
}

// returns the top of the while stack (or 0 if empty)
int get_while()
{
    if (wh_ind == 0) return 0;
    return wh_stk[wh_ind-1];
}
