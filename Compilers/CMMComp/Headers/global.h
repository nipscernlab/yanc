// ----------------------------------------------------------------------------
// global functions and variables of the compiler -----------------------------
// ----------------------------------------------------------------------------

#ifndef CMM_GLOBAL_H
#define CMM_GLOBAL_H

#include <stdio.h>
#include "context.h"

// ----------------------------------------------------------------------------
// global variables -----------------------------------------------------------
// ----------------------------------------------------------------------------

extern FILE *yyin ;              // the .cmm needs to be linked to this bison pointer

extern const char *dir_macro;    // Macros directory
extern const char *dir_tmp;      // Temp directory

// ----------------------------------------------------------------------------
// state variables ------------------------------------------------------------
// ----------------------------------------------------------------------------

extern int  sim_arr ;            // tells whether the array is simulated or not

// ----------------------------------------------------------------------------
// interface functions --------------------------------------------------------
// ----------------------------------------------------------------------------

// parser start and end
void parse_init(char *f_name, char *d_proc, char *d_macro, char *d_tmp, char *d_array);
void prog_emit (void);                              // fires from `fim : prog` action after parsing completes
void parse_end (char *d_proc);

// registers the assembly instructions
void add_instr(char *inst, ...);           // standard
void add_sinst(int type, char *inst, ...); // special

// clears the peephole state inside add_instr (last_str + acc_names). Called at
// every basic-block boundary (labels, capture push/pop, macro use/end) so the
// "drop LOD x while the acc already holds x" optimization never crosses control
// flow.
void emit_peephole_reset(void);

// true when the accumulator is known to currently hold variable `name` -- lets
// codegen square the acc-resident half of c²+d² first so its LOD is dropped.
int  acc_holds(const char *name);

// after an indexed store, names the acc by the element stored, "v[k]" (ast.c)
void acc_holds_elem(const char *key);

// names the acc by the index of an indexed store just computed, "[k]" (ast.c)
void acc_holds_index(const char *key);

#endif // CMM_GLOBAL_H
