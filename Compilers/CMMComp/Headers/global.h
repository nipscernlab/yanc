// ----------------------------------------------------------------------------
// global functions and variables of the compiler -----------------------------
// ----------------------------------------------------------------------------

#ifndef CMM_GLOBAL_H
#define CMM_GLOBAL_H

#include <stdio.h>
#include "context.h"
#include "acc.h"       // what the accumulator holds (emit_peephole_reset, acc_holds, acc_now, ...)

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
void parse_init(char *f_name, char *d_proc, char *d_macro, char *d_tmp, const char *d_array);
void prog_emit (void);                              // fires from `fim : prog` action after parsing completes
void parse_end (char *d_proc);

// registers the assembly instructions
void add_instr(const char *inst, ...);           // standard
void add_sinst(int type, const char *inst, ...); // special

#endif // CMM_GLOBAL_H
