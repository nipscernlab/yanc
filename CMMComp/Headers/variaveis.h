// ----------------------------------------------------------------------------
// identifier table -----------------------------------------------------------
// ----------------------------------------------------------------------------

// table elements (dynamically allocated, grow on demand) ---------------------

extern char (*v_name)[512];              // name of the variable or function
extern int  *v_type;                     // 0 -> unidentified, 1 -> int, 2 -> float, 3 -> comp, 5 -> const comp
extern int  *v_used;                     // whether the ID has already been used
extern int  *v_fpar;                     // if the ID is a function, holds the parameter list
extern int  *v_fnid;                     // ID of the function the variable belongs to
extern int  *v_isar;                     // whether the variable is an array
extern int  *v_isco;                     // whether the variable is a constant
extern int  *v_size;                     // array size (when it is an array)
extern int  *v_siz2;                     // size of the j dimension (when it is a matrix)

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
