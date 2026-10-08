// ----------------------------------------------------------------------------
// opcode handling routines ---------------------------------------------------
// ----------------------------------------------------------------------------

void  opc_add (char *mne); // registers an opcode
char* opc_get (int i);     // returns the registered opcode name
int   opc_cnt ();          // returns the number of registered opcodes
int   opc_ucnt();          // returns the number of registered ALU operations
int   opc_total();         // returns how many opcode parameters exist
int   opc_utotal();        // returns how many ALU blocks exist
int   opc_inn ();          // checks whether the INN instruction is present
int   opc_out ();          // checks whether the OUT instruction is present
int   opc_tom ();          // checks whether the TOM instruction is present (toma, docs/toma-and-cade.md)
int   opc_cad ();          // checks whether the CAD instruction is present (cade)
int   opc_cal ();          // checks whether the CAL instruction is present
