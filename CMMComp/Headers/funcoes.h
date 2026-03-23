// ----------------------------------------------------------------------------
// rotinas e variaveis de estado para parser de funcoes -----------------------
// ----------------------------------------------------------------------------

// variaveis de estado --------------------------------------------------------

extern char fname[512];             // nome da funcao atual sendo parseada
extern int  mainok;                 // status da funcao main: 0 -> indefinido, 1 -> resolvido (como sera chamada)
extern int  fun_id;                 // guarda id da funcao sendo usada

// declaracao -----------------------------------------------------------------

void  declar_fun(int id1, int id2); // tipo e nome
void  declar_fst(int id);           // primeiro/unico parametro
int   declar_par(int   t, int id ); // proximo parametro
void     set_par(int id);           // SET no parametro
void  declar_ret(int  et, int ret); // achou uma palavra-chave return
void    func_ret(int id);           // fim/return da funcao
void    void_ret(      );           // fim de uma funcao void

// utilizacao -----------------------------------------------------------------

void  par_exp    (int et );         // carrega primeiro parametro (se houver)
void  par_listexp(int et );         // carrega os proximos parametros (se houver)
void  vcall      (int id );         // CAL de funcao com retorno
int   fcall      (int id );         // CAL de funcao void