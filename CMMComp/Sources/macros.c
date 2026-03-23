// ----------------------------------------------------------------------------
// funcoes e variaveis pra criacao e utilizacao de macros ---------------------
// ----------------------------------------------------------------------------

/*
TODO:
1- implementar mais funcoes nao-lineares
2- ver no TCC do Tiago Falcao qual metodo eh melhor pra cada caso
*/

// includes globais
#include <math.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

// includes locais
#include "..\Headers\t2t.h"
#include "..\Headers\global.h"
#include "..\Headers\funcoes.h"
#include "..\Headers\diretivas.h"
#include "..\Headers\variaveis.h"

// ----------------------------------------------------------------------------
// gerenciamento de macros criadas pelo usuario -------------------------------
// ----------------------------------------------------------------------------

// redeclaracao de variaveis globais ------------------------------------------

int mac_using = 0; // se estiver lendo uma macro, nao deve escrever o assembler durante o parse

// nao deixa o parser escrever no arquivo assembler ---------------------------
// ao inves disso, copia o codigo de uma macro
// id_num eh o terceiro argumento da macro, que deve ser o numero de instrucoes
// tirar essa necessidade de passar o id_num, pois o numero de instrucoes eh fixo
void mac_use(int ids, int global, int id_num)
{
    // checa consistencia -----------------------------------------------------

    if (mac_using == 1)
        {fprintf(stderr, "Erro na linha %d: tá chamando uma macro dentro da outra. você é uma pessoa confusa!\n", line_num+1); exit(EXIT_FAILURE);}

    printf("Info: replacing C± code by user macro %s at line %d\n", v_name[ids], line_num+1);

    // se for global, tem q ver se tem que chamar a funcao main ainda ---------
    if ((mainok == 0) && (global == 1))
    {
        add_sinst(-2, "JMP main\n");

        mainok = 1; // questao da funcao main foi resolvida
    }

    // remover as aspas da string (trabalho da porra!) ------------------------
    // mudar para um codigo mais simples como o que ta em array.c -------------

    char f_name[64];
    strcpy(f_name, v_name[ids]);
    char file_name[512];
    int  tamanho = strlen(f_name); // tamanho da string
    int idxToDel = tamanho-1;      // indice para deletar, nesse caso o ultimo, as aspas.
    strcpy ( file_name, "");
    memmove(&f_name[idxToDel], &f_name[idxToDel+1], 1); // deletando de fato as ultimas aspas
    strcat ( file_name, f_name+1); // agora copia, tirando as primeiras aspas

    char    mac_name[1024];
    sprintf(mac_name, "%s/%s", dir_soft, file_name);

    // copia o codigo do arquivo asm ------------------------------------------

    FILE *f_macro;
    char a;
        f_macro  =    fopen  (mac_name, "r");
    if (f_macro == 0){fprintf(stderr, "Erro na linha %d: cadê a macro %s? Tinha que estar na pasta Software!\n", line_num+1, file_name); exit(EXIT_FAILURE);}
	do {      a  =    fgetc  (f_macro); if (a != EOF) fputc(a,f_asm);} while (a != EOF);
                      fputc  ('\n',f_asm);
	                  fclose (f_macro);

    // preenche tabela de codigo cmm com -1 (INTERNO) -------------------------
    
    int n = atoi(v_name[id_num]);
    for (int i = 0; i < n; i++)
    {
        num_ins++;
        fprintf(f_lin, "%s\n", itob(-4,20));
    }

    mac_using = 1;
}

// libera o parser pra salvar no arquivo assembler ----------------------------

void mac_end()
{
    if (mac_using == 0) {fprintf(stderr, "Erro na linha %d: não estou achando o começo da macro\n", line_num+1); exit(EXIT_FAILURE);}
    mac_using = 0;
}

// ----------------------------------------------------------------------------
// funcoes auxiliares para geracao de macros pre-definidas --------------------
// ----------------------------------------------------------------------------

// concatena conteudo do arquivo read no arquivo write
void fcat2end(char *n_read, char *n_write)
{
    FILE *f_in  = fopen(n_read , "r");
    FILE *f_out = fopen(n_write, "a");

    char a;
    do {a = fgetc(f_in); if (a != EOF) fputc(a, f_out);} while (a != EOF);

    fclose(f_in );
    fclose(f_out);
}

// ----------------------------------------------------------------------------
// gerenciamento de macros pre-definidas --------------------------------------
// ----------------------------------------------------------------------------

// variaveis locais -----------------------------------------------------------

int fatan = 0; // se vai precisar de macro pra arco tangente
int fsqrt = 0; // se vai precisar de macro pra raiz quadrada
int fsin  = 0; // se vai precisar de macro pra seno

// adiciona flag de uma macro pre-definida ------------------------------------

void mac_add(char *name)
{
         if (strcmp(name, "fsqrt") == 0) fsqrt = 1; // raiz quadrada
    else if (strcmp(name, "fatan") == 0) fatan = 1; // arco tangente
    else if (strcmp(name, "fsin" ) == 0) fsin  = 1; // seno
}

// copia as macros pre-definidas no final arquivo assembler -------------------

void mac_copy(char *fasm)
{
    // se nao tiver nada pra fazer, sai! --------------------------------------

    if (!(fsqrt || fatan || fsin)) return;

    // copia o que precisa no final do asm ------------------------------------

    char tasm[1024]; sprintf(tasm, "%s/%s", dir_tmp, "tasm.txt");

    if (fsqrt)
    {
        printf("Info: adding assembly macro for root square computation\n");
        sprintf(tasm, "%s/float_sqrt.asm", dir_macro);
        fcat2end(tasm,fasm);
    }

    if (fatan)
    {
        printf("Info: adding assembly macro for arc-tangent computation\n");
        sprintf(tasm, "%s/float_atan.asm", dir_macro);
        fcat2end(tasm,fasm);
    }

    if (fsin)
    {
        printf("Info: adding assembly macro for sin computation\n");
        sprintf(tasm, "%s/float_sin.asm", dir_macro);
        fcat2end(tasm,fasm);
    }
}

// ----------------------------------------------------------------------------
// backup do codigo em c+- das macros pre-definidas ---------------------------
// ----------------------------------------------------------------------------

// arco-tg para float (float_atan.asm)
/*float float_atan(float x)
{
    float ax = abs(x);

    if (ax == 0.0) return 0.0;
    if (ax > 1.02) return sign(x, 1.5707963268) - my_atan(1.0/x);
    if (ax > 0.98)
    {
        float xm1 = ax-1;
        return sign(x, 0.7853981634 + xm1*0.5 - xm1*xm1*0.25); 
    }

    float termo      = x;
    float x2         = x*x;
    float resultado  = termo;
    float tolerancia = 0.000008/x2;

    int indiceX = 3;

    while ((abs(termo) > tolerancia) && (indiceX < 100)) {
        termo = termo * (- x2 * (indiceX - 2)) / indiceX;

        resultado = resultado + termo;
        indiceX = indiceX + 2;
    }

    return resultado;
}*/

// seno para float (float_sin.asm)
/*float sin(float x)
{
    if (x == 0) return 0.0;
    
    while (abs(x) > 3.141592654) x = x - sign(x, 6.283185307);

    float termo      = x;
    float x2         = x * x;
    float resultado  = termo;
    float tolerancia = 0.000008/x2;

    int indiceX = 3;

    while (abs(termo) > tolerancia) {
        termo = termo * (- x2) / ((indiceX - 1) * indiceX);
        resultado = resultado + termo;
        indiceX = indiceX + 2;
    }

    return resultado;
}*/