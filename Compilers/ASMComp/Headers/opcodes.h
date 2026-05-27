// ----------------------------------------------------------------------------
// opcode handling routines ---------------------------------------------------
// ----------------------------------------------------------------------------

void  opc_add (char *mne); // registers an opcode
char* opc_get (int i);     // returns the registered opcode name
int   opc_cnt ();          // returns the number of registered opcodes
int   opc_ucnt();          // returns the number of registered ALU operations
int   opc_inn ();          // checks whether the INN instruction is present
int   opc_out ();          // checks whether the OUT instruction is present
int   opc_cal ();          // checks whether the CAL instruction is present
