// ----------------------------------------------------------------------------
// identifier table -----------------------------------------------------------
// ----------------------------------------------------------------------------

#ifndef CMM_SYMBOLS_H
#define CMM_SYMBOLS_H

#include "types.h"

#include <stddef.h>

// One row of the symbol table.
typedef struct symbol {
    char *name;         // name of the variable or function, in a block of its own
                        // (add_var's xstrdup), never moved or freed: code may keep
                        // the pointer while the table grows
    int  type;          // a TY_* code of types.h: TY_NONE until declared, a value type, or a function's
    int  used;          // whether the ID has already been used
    int  n_params;      // if the ID is a function, how many parameters it has
    int *param_types;   // and the type of each one (TY_INT, TY_FLOAT or TY_COMP), n_params entries
    int  owner;         // ID of the function the variable belongs to
    int  dims;          // AR_NONE, AR_1D or AR_2D (types.h): a scalar, a vector, a matrix
    int  is_const;      // whether the variable is a constant
    int  size;          // array size (when it is an array)
    int  cols;          // size of the j dimension (when it is a matrix)
    int  wave;          // #WAVE: kept for the waveform even if never read
} symbol;

// the longest name a program may write (issue #66, Luciano 2026-11-03). It
// is a rule of the language, not the size of a buffer: every name lives in a
// block of its own exact size. The limit comes from the tools after YANC: the
// longest name built from a user name, a local comp's imaginary part
// <function>_<name>_i, becomes a Verilog identifier of about 2 * CMM_NAME_MAX
// + 15 characters (me1_f_<function>_v_<name>_e_), and iverilog takes up to
// 16382 (measured 2026-11-03, #66); 1000 leaves room for that eight times
#define CMM_NAME_MAX 1000

extern symbol *v_table;                  // AoS storage, indexed by ID 0..v_count
extern int     v_count;                  // how many entries v_table holds

// table element manipulation -------------------------------------------------

int    find_var(const char *val);             // searches the table for the ID (-1 if not found)
void    add_var(const char *var);             // adds an ID to the table
void  check_var(void);                       // checks what was used in the table (at the end of the parser)
char *strip_fn (char *var, const char *func); // <func>_<name> -> <name>; any other name as it is
char *strip_fn_prefix(char *var);              // the same, for the function being compiled
void  check_name(int id, int is_fun);     // a declared name must be free in the .asm and not an instruction
void  directive_wave (int id);                // #WAVE name: the variable stays even if never read

// used in the lexer to pick up variables and constants -----------------------

int lex_id  (char *text);               // an ID was found
int cnum_join(const char *re, int sub, int imid); // re +/- an imaginary literal: the comp literal
int cnum_is_imag(int id);                // a comp literal whose real part is zero
char *neg_text(int id);                  // "-" and the text of literal id (the block is the caller's)
int cnum_imag_fnum(int id, int neg);     // the imaginary part of a comp literal, as a float literal
int find_id  (const char *text);         // the ID as lex_id would resolve it, without creating it (-1 if none)
int lex_inum(char *text);               // an int constant was found
int inum_kind(const char *text);         // 0 INUM, 1 NINUM (a negative bit pattern), 2 NIMIN (the most negative), 3 DIMIN (its magnitude in decimal)
int lex_inum_neg(char *text);           // the magnitude of a NINUM / NIMIN / DIMIN
void inum_overflow(void);                // a decimal int past the word: the message, and stop
int inum_word(int id);                   // an int literal used as a value: it must fit the word
int arr_size(int id);                    // an array size: a warning if an int index cannot reach its end
int inum_min_id(void);                   // 2^(NUBITS-1), for a NIMIN in a brace list and for a DIMIN
int lex_fnum(char *text);               // a float constant was found
extern int fnum_quiet;                   // 1: lex_fnum says nothing of an approximation (a constant the compiler made)
int lex_cnum(char *text);               // a comp constant was found
char *fnum_text (const char *in);       // a float literal as the .asm writes it (the block is the caller's)
void cnum_split(const char *txt, char **re, char **im); // a comp literal's two parts (the blocks are the caller's)

#endif // CMM_SYMBOLS_H
