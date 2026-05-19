// ----------------------------------------------------------------------------
// identifier table -----------------------------------------------------------
// ----------------------------------------------------------------------------

// One row of the symbol table. Currently allocated in parallel with the
// legacy SoA arrays below (v_name/v_type/...); cutover happens field-by-field
// in subsequent commits, after which v_table becomes the sole storage.
typedef struct symbol {
    char name[512];   // name of the variable or function
    int  type;        // 0 -> unidentified, 1 -> int, 2 -> float, 3 -> comp, 5 -> const comp
    int  used;        // whether the ID has already been used
    int  fpar;        // if the ID is a function, holds the parameter list
    int  fnid;        // ID of the function the variable belongs to
    int  isar;        // whether the variable is an array
    int  isco;        // whether the variable is a constant
    int  size;        // array size (when it is an array)
    int  siz2;        // size of the j dimension (when it is a matrix)
} symbol;

extern symbol *v_table;                  // AoS storage, indexed by ID 0..v_count

// Legacy SoA pointers (being retired field-by-field). The matching
// struct member already lives in v_table; what remains here is what
// hasn't been cut over yet.

extern char (*v_name)[512];              // name of the variable or function
extern int  *v_type;                     // 0 -> unidentified, 1 -> int, 2 -> float, 3 -> comp, 5 -> const comp

// table element manipulation -------------------------------------------------

int    find_var(char *val);              // searches the table for the ID (-1 if not found)
void    add_var(char *var);              // adds an ID to the table
void  check_var();                       // checks what was used in the table (at the end of the parser)
char *rem_fname(char *var, char *fname); // removes the function name from the variable

// used in the lexer to pick up variables and constants -----------------------

int exec_id  (char *text);               // an ID was found
int exec_inum(char *text);               // an int constant was found
int exec_fnum(char *text);               // a float constant was found
int exec_cnum(char *text);               // a comp constant was found
