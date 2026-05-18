// ----------------------------------------------------------------------------
// routines and state variables for the function parser -----------------------
// ----------------------------------------------------------------------------

#include "ast.h"   // expr

// state variables ------------------------------------------------------------

extern char fname[512];             // name of the function currently being parsed
extern int  mainok;                 // main function status: 0 -> undefined, 1 -> resolved (how it will be called)
extern int  fun_id;                 // stores the id of the function being used
extern int  p_test;                 // packed param-type list for the call currently being walked

// type-checks one argument against the expected parameter and emits LOD/SET
// to stage it for the upcoming CAL. Read by the EXPR_FUNC_CALL walker.
void par_check(expr e);

// declaration ----------------------------------------------------------------

void  declar_fun(int id1, int id2); // type and name
void  declar_fst(int id);           // first/only parameter
int   declar_par(int   t, int id ); // next parameter
void     set_par(int id);           // SET on the parameter
void  declar_ret(expr e , int ret); // a return keyword was found
void func_body_begin(void);         // fires at the function's opening '{'
void    func_ret(int id);           // end/return of the function
void    void_ret(      );           // end of a void function

// usage ----------------------------------------------------------------------

void       par_exp    (expr_node *n); // record first  arg's node on the frame
void       par_listexp(expr_node *n); // record next-N arg's node on the frame
void       vcall      (int id);       // void user call: builds node and walks it
expr_node *fcall      (int id);       // value user call: builds and returns node

// argument-stack frames for nested function calls. args_frame_push() is
// called from the grammar mid-rule action right after 'ID ('; par_exp /
// par_listexp push each arg's node onto the frame; fcall / vcall drain the
// frame when ')' is reduced.
void  args_frame_push(void);
