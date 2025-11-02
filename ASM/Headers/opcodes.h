// ----------------------------------------------------------------------------
// rotinas de manipulacao de opcodes ------------------------------------------
// ----------------------------------------------------------------------------

void  opc_add (char *mne); // cadastra opcode
char* opc_get (int i);     // retorna o nome   do opcode  cadastrado
int   opc_cnt ();          // retorna o numero de opcodes cadastrados
int   opc_ucnt();          // pega numero de operacoes da ula cadastrados
int   opc_inn ();          // verifica se tem instrucao INN
int   opc_out ();          // verifica se tem instrucao OUT
int   opc_cal ();          // verifica se tem instrucao CAL