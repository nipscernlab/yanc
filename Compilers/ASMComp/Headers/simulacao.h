// ----------------------------------------------------------------------------
// routines for simulation with iverilog/gtkwave ------------------------------
// ----------------------------------------------------------------------------

// access to simulation parameters --------------------------------------------

int   sim_clk    ();                                // returns the simulation clock frequency
int   sim_clk_num();                                // returns the number of clocks to simulate
// translation file operations ------------------------------------------------

void  sim_init   (int clk, int clk_n);              // creates the translation file
void  sim_add    (char *opc, char *opr);            // adds an opcode and operand
void  sim_set_fim(int   fim);                       // sets the @fim address
int   sim_get_fim();                                // returns the @fim address
void  sim_finish ();                                // closes the translation file

// user variable operations ---------------------------------------------------

int   sim_regi   (char *va);                        // registers a variable
char* sim_name   (int    i);                        // gets the variable name
int   sim_addr   (int    i);                        // gets the variable address
int   sim_type   (int    i);                        // gets the variable type
int   sim_cont   (        );                        // gets the number of registered variables
int   sim_regi_at(char *va, int addr);              // registers a variable whose word is at addr (#SHARE)
int   sim_has    (char *va);                        // whether a variable is already registered
void  sim_store  (char *va);                        // the next instruction writes va (#SHARE groups)
int   sim_shared (int    i);                        // whether the variable is in a #SHARE group
int   sim_nstore (int    i);                        // how many SETs write it
int   sim_store_at(int i, int k);                   // instruction index of the k-th SET
void  sim_mem    (int addr, char *var);             // gets the memory contents

// user array operations ------------------------------------------------------

void  sim_regi_arr(char *va);                       // registers an array
char* sim_name_arr(int    i);                       // gets the array name
int   sim_addr_arr(int    i);                       // gets the array address
int   sim_type_arr(int    i);                       // gets the array type
int   sim_cont_arr(        );                       // gets the number of registered arrays
int   sim_size_arr(int    i);                       // gets the array size
