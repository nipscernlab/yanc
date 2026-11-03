// ----------------------------------------------------------------------------
// generates assembly labels for the jump instructions ------------------------
// ----------------------------------------------------------------------------

#include "../Headers/labels.h"

// ----------------------------------------------------------------------------
// label numbers --------------------------------------------------------------
// ----------------------------------------------------------------------------

// each if/else, and each while, for, do and switch, takes the next number of
// its counter for its labels (__Lif<n>..., __Lwh<n>...)
static int if_cnt = 0;
static int wh_cnt = 0;

int push_if   (void) {return ++if_cnt;}
int push_while(void) {return ++wh_cnt;}
