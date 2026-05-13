// ----------------------------------------------------------------------------
// routines and state variables for the function parser -----------------------
// ----------------------------------------------------------------------------

// state variables ------------------------------------------------------------

extern char fname[512];             // name of the function currently being parsed
extern int  mainok;                 // main function status: 0 -> undefined, 1 -> resolved (how it will be called)
extern int  fun_id;                 // stores the id of the function being used

// declaration ----------------------------------------------------------------

void  declar_fun(int id1, int id2); // type and name
void  declar_fst(int id);           // first/only parameter
int   declar_par(int   t, int id ); // next parameter
void     set_par(int id);           // SET on the parameter
void  declar_ret(int  et, int ret); // a return keyword was found
void    func_ret(int id);           // end/return of the function
void    void_ret(      );           // end of a void function

// usage ----------------------------------------------------------------------

void  par_exp    (int et );         // loads the first parameter (if any)
void  par_listexp(int et );         // loads the remaining parameters (if any)
void  vcall      (int id );         // CAL of a function with return value
int   fcall      (int id );         // CAL of a void function
