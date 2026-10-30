// ----------------------------------------------------------------------------
// identifier table -----------------------------------------------------------
// ----------------------------------------------------------------------------

#include <stddef.h>

// One row of the symbol table.
typedef struct symbol {
    char name[512];   // name of the variable or function
    int  type;        // 0 -> unidentified, 1 -> int, 2 -> float, 3 -> comp, 5 -> const comp
    int  used;        // whether the ID has already been used
    int  npar;        // if the ID is a function, how many parameters it has
    int *ptyp;        // and the type of each one (1, 2 or 3), npar entries
    int  fnid;        // ID of the function the variable belongs to
    int  isar;        // whether the variable is an array
    int  isco;        // whether the variable is a constant
    int  size;        // array size (when it is an array)
    int  siz2;        // size of the j dimension (when it is a matrix)
} symbol;

// the longest name a program may write. The longest name the compiler
// builds from it, a local comp's imaginary part <function>_<name>_i, is
// 2 * CMM_NAME_MAX + 3 = 103 characters: it must go through asmcomp, whose
// buffers of 128 stopped a global name of 121 characters (measured
// 2026-10-30; 120 went through), and fits name[512] here with room to spare;
// GitHub issue #66 is to take names of any length and drop this limit
#define CMM_NAME_MAX 50

extern symbol *v_table;                  // AoS storage, indexed by ID 0..v_count

// table element manipulation -------------------------------------------------

int    find_var(char *val);              // searches the table for the ID (-1 if not found)
void    add_var(char *var);              // adds an ID to the table
void  check_var();                       // checks what was used in the table (at the end of the parser)
char *rem_fname(char *var, char *fname); // removes the function name from the variable
void  check_name(int id, int is_fun);     // a declared name must be free in the .asm and not an instruction

// used in the lexer to pick up variables and constants -----------------------

int exec_id  (char *text);               // an ID was found
int cnum_join(const char *re, int sub, int imid); // re +/- an imaginary literal: the comp literal
int cnum_is_imag(int id);                // a comp literal whose real part is zero
const char *neg_text(int id);            // "-" and the text of literal id
int cnum_imag_fnum(int id, int neg);     // the imaginary part of a comp literal, as a float literal
int find_id  (const char *text);         // the ID as exec_id would resolve it, without creating it (-1 if none)
int exec_inum(char *text);               // an int constant was found
int inum_kind(const char *text);         // 0 INUM, 1 NINUM (a negative bit pattern), 2 NIMIN (the most negative), 3 DIMIN (its magnitude in decimal)
int exec_inum_neg(char *text);           // the magnitude of a NINUM / NIMIN / DIMIN
void inum_overflow(void);                // a decimal int past the word: the message, and stop
int inum_word(int id);                   // an int literal used as a value: it must fit the word
int arr_size(int id);                    // an array size: a warning if an int index cannot reach its end
int inum_min_id(void);                   // 2^(NUBITS-1), for a NIMIN in a brace list and for a DIMIN
int exec_fnum(char *text);               // a float constant was found
int exec_cnum(char *text);               // a comp constant was found
void fnum_text (const char *in, char *out, size_t n); // a float literal as the .asm writes it
void cnum_split(const char *txt, char *re, char *im); // a comp literal's two parts
