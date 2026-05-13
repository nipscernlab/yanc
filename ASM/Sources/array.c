// ----------------------------------------------------------------------------
// tratamento de arrays em assembly -------------------------------------------
// ----------------------------------------------------------------------------

// includes globais
#include <string.h>
#include <stdlib.h>
#include  <ctype.h>
#include   <math.h>

// includes locais
#include "..\Headers\t2t.h"
#include "..\Headers\eval.h"
#include "..\Headers\array.h"
#include "..\Headers\variaveis.h"
#include "..\Headers\messages.h"

// ----------------------------------------------------------------------------
// funcoes auxiliares ---------------------------------------------------------
// ----------------------------------------------------------------------------

// funcao auxiliar para remover aspas de uma string
void rem_aspas(char *str)
{
    int j = 0; char c;
    for (int i = 0; (c = str[i]) != '\0'; i++) if (c != '"') str[j++] = c;
    str[j] = '\0';
}

// funcao auxiliar para verificar se o conteudo da linha de um arquivo eh um int valido
int linha_e_inteiro(char *linha, int idx, char *f_name)
{
    // primeiro checa formatacao ----------------------------------------------

    // ignora espaços em branco no começo
    while (isspace((unsigned char)*linha)) linha++ ;
    // se linha ta vazia, nao eh um inteiro valido
    if (*linha == '\0' || *linha == '\n') {fprintf(stderr, MSG_ERR_EMPTY_LINE, idx, f_name); exit(EXIT_FAILURE);}
    // passa pelos caracteres dos numeros (incluindo sinal de negativo)
    char *endptr; strtol(linha, &endptr, 10);
    // verifica se o restante da string é só espaços
    while (isspace((unsigned char)*endptr)) endptr++;
    // eh inteiro se não sobrou mais nada
    if (*endptr != '\0') {fprintf(stderr, MSG_ERR_INVALID_INT, idx, f_name); exit(EXIT_FAILURE);}
    
    // checa se o valor cabe no numero de bits --------------------------------

    int max = (int) ( pow(2,nbmant+nbexpo+1-1)-1);
    int min = (int) (-pow(2,nbmant+nbexpo+1-1)  );
    int num = atoi(linha);

    if (num > max) {fprintf(stderr, MSG_ERR_INT_OVER, idx, f_name, max); exit(EXIT_FAILURE);}
    if (num < min) {fprintf(stderr, MSG_ERR_INT_UNDER, idx, f_name, min); exit(EXIT_FAILURE);}

    return num;
}

// funcao auxiliar para verificar se uma linha representa um float valido
int linha_e_float(char *linha, int idx, char *f_name, float *delta)
{
    // checa formatacao -------------------------------------------------------
    
    // ignora espaços em branco iniciais
    while (isspace((unsigned char)*linha)) linha++;
    // linha vazia
    if (*linha == '\0' || *linha == '\n') {fprintf(stderr, MSG_ERR_EMPTY_LINE, idx, f_name); exit(EXIT_FAILURE);}
    // passa pelos caracteres dos numeros (incluindo sinal de negativo)
    char *endptr; strtof(linha, &endptr);
    // verifica se o restante da string é só espaços
    while (isspace((unsigned char)*endptr)) endptr++;
    // eh float se nao sobrou mais nada
    if (*endptr != '\0') {fprintf(stderr, MSG_ERR_INVALID_FLOAT, idx, f_name); exit(EXIT_FAILURE);}

    // verifica limites -------------------------------------------------------

    float max = (float)((pow(2,nbmant)-1) * pow(2, pow(2,nbexpo-1)-1)); // maior valor possivel em modulo
    float min = (float)(                    pow(2,-pow(2,nbexpo-1)  )); // menor valor possivel em modulo
    float num = (atof(linha)<0.0) ? -atof(linha) : atof(linha);         //       valor do num   em modulo

    if (num < min && num != 0.0) {fprintf(stderr, MSG_ERR_FLOAT_UNDER, idx, f_name, min); exit(EXIT_FAILURE);}
    if (num > max)               {fprintf(stderr, MSG_ERR_FLOAT_OVER, idx, f_name, max); exit(EXIT_FAILURE);}

    // converte e calcula residuo ---------------------------------------------

    return f2mf(linha,delta);
}

// função auxiliar para verificar e extrair um float válido
// usado pra ler variaveis tipo comp de arquivo
int parse_float(const char *str, float *out_value, const char **out_end)
{
    char *endptr;

    float val = strtof(str, &endptr); // tenta converter
    if (str  == endptr)     return 0; // falha na conversão
    *out_value =    val;              // pega o valor convertido
    *out_end   = endptr;              // aponta para o proximo caractere da linha
                            return 1; // conversao ok
}

// funcao auxiliar para separar a parte real e imaginaria de um comp
void separar_complexo(const char *entrada, char *real, char *imag) {
    char buffer[100];
    int j = 0;

    // Remover espaços
    for (int i = 0; entrada[i] != '\0'; i++) {
        if (!isspace((unsigned char)entrada[i])) {
            buffer[j++] = entrada[i];
        }
    }
    buffer[j] = '\0';

    // Procurar último '+' ou '-' que separa real de imaginário
    int pos = -1;
    for (int i = 1; buffer[i] != '\0'; i++) { // pular índice 0 para não confundir sinal da parte real
        if (buffer[i] == '+' || buffer[i] == '-') {
            pos = i;
        }
    }

    if (pos == -1) {
        fprintf(stderr, MSG_ERR_BAD_FORMAT);
        real[0] = '\0';
        imag[0] = '\0';
        return;
    }

    // Copiar parte real
    strncpy(real, buffer, pos);
    real[pos] = '\0';

    // Copiar parte imaginária (sem o 'i')
    strcpy(imag, buffer + pos);
    size_t len = strlen(imag);
    if (len > 0 && imag[len - 1] == 'i') {
        imag[len - 1] = '\0';
    }
}

// funcao auxiliar para verificar se uma linha contém um comp valido
int linha_e_comp(const char *linha, int idx, char *f_name, float *delta)
{
    const char *p = linha;
    float f1, f2;

    // checa formatacao -------------------------------------------------------

    // Ignora espaços
    while (isspace((unsigned char)*p)) p++;
    // Primeiro float
    if (!parse_float(p, &f1, &p)) {fprintf(stderr, MSG_ERR_INVALID_COMP, idx, f_name); exit(EXIT_FAILURE);}
    // Ignora espaços
    while (isspace((unsigned char)*p)) p++;
    // Segundo float
    if (!parse_float(p, &f2, &p)) {fprintf(stderr, MSG_ERR_INVALID_COMP, idx, f_name); exit(EXIT_FAILURE);}
    // se o proximo caractere nao for a letra i, retorna
    if (*p != 'i')                {fprintf(stderr, MSG_ERR_INVALID_COMP, idx, f_name); exit(EXIT_FAILURE);}
    // incrementa o ponteiro do ´i´
    p++;
    // Ignora espaços
    while (isspace((unsigned char)*p)) p++;
    // se sobrou algo na linha, nao eh comp valido
    if (*p != '\0')               {fprintf(stderr, MSG_ERR_INVALID_COMP, idx, f_name); exit(EXIT_FAILURE);}

    // verifica limites -------------------------------------------------------

    float max = (float)((pow(2,nbmant)-1) * pow(2, pow(2,nbexpo-1)-1)); // maior valor possivel em modulo
    float min = (float)(                    pow(2,-pow(2,nbexpo-1)  )); // menor valor possivel em modulo
    char  real [64], imag[64];

    separar_complexo(linha, real, imag);

    // parte real
    float num = (atof(real)<0.0) ? -atof(real) : atof(real); // valor da parte realem modulo
    if (num < min && num != 0.0) {fprintf(stderr, MSG_ERR_COMP_REAL_UNDER, idx, f_name, min); exit(EXIT_FAILURE);}
    if (num > max)               {fprintf(stderr, MSG_ERR_COMP_REAL_OVER, idx, f_name, max); exit(EXIT_FAILURE);}

    // parte imaginaria
        num = (atof(imag)<0.0) ? -atof(imag) : atof(imag); // valor da parte imaginaria em modulo
    if (num < min && num != 0.0) {fprintf(stderr, MSG_ERR_COMP_IMAG_UNDER, idx, f_name, min); exit(EXIT_FAILURE);}
    if (num > max)               {fprintf(stderr, MSG_ERR_COMP_IMAG_OVER, idx, f_name, max); exit(EXIT_FAILURE);}

    // converte e calcula residuo da parte real -------------------------------

    return f2mf(real,delta);
}

// funcao auxiliar para preencher array na memoria de dados
// usado com inicializacao de array (ex: int x[10] "nome do arquivo")
// f_name  -> nome do arquivo a ser lido
// tam     -> tamanho do arquivo
// fil_typ -> tipo de dado
// f_data  -> arquivo da memoria de dados
void fill_mem(char *f_name, int tam, int fil_typ, FILE *f_data)
{
    // informa que o array vai ser preenchido ---------------------------------

    if (fil_typ != 4) printf(MSG_INFO_FILL_ARRAY, tam, f_name);

    // abre o arquivo para leitura -------------------------------------------
    
    rem_aspas(f_name);

    char path[2048];
    // verifica se é LUT na pasta Macros
    if (f_name[0]=='$')
        sprintf(path, "%s/%s"          , mac_dir, f_name+1);
    else
        sprintf(path, "%s/Software/%s", proc_dir, f_name  );

    FILE *f_file =        fopen  (path  , "r");
    if   (f_file == NULL){fprintf(stderr, MSG_ERR_CANT_OPEN_FILE, path); exit(EXIT_FAILURE);}

    // agora le o arquivo -----------------------------------------------------

    int  val;
    char linha[128];
    char real [64], imag[64];

    int   i     =   0;
    int   idx   =   0;
    float delta = 0.0;
    float dmax  = 0.0;
    while (fgets (linha, sizeof(linha), f_file))
    {
        i++;

        // se for tipo int
        if (fil_typ == 1) val = linha_e_inteiro(linha,i,f_name);

        // se for tipo float
        if (fil_typ == 2)
        {
            val = linha_e_float(linha,i,f_name,&delta);
            // guarda o maior erro de aproximacao
            if (fabs(delta) > fabs(dmax)) {dmax = delta; idx = i;}
        }

        // se for a parte real de um comp
        if (fil_typ == 3)
        {
            val = linha_e_comp(linha,i,f_name,&delta);
            // guarda o maior erro de aproximacao
            if (fabs(delta) > fabs(dmax)) {dmax = delta; idx = i;}
        }

        // se for a parte imaginaria de um comp
        if (fil_typ == 4)
        {
            separar_complexo(linha, real, imag);
            val = f2mf(imag,&delta);
            // guarda o maior erro de aproximacao
            if (fabs(delta) > fabs(dmax)) {dmax = delta; idx = i;}
        }

        // se tem mais dados do que o necessario, da um warning e sai
        if (i > tam)
        {
            fprintf(stdout, MSG_WARN_EXTRA_LINES, i-tam, f_name);
            break;
        }
        // senao, preenche a memoria com o novo valor
        else
            fprintf(f_data, "%s\n", itob(val,nubits));
    }

    // se tem menos dados do que o necessario, gera um erro
    if ((i < tam) && (fil_typ != 4))
        {fprintf(stderr, MSG_ERR_MISSING_LINES, tam-i, f_name); exit(EXIT_FAILURE);}

    // informa o maior erro de aproximacao pra float
    if (fil_typ == 2 && dmax != 0.0)
        printf(MSG_INFO_APPROX_ERR, f_name, dmax, idx);

    // informa o maior erro de aproximacao pra real do comp
    if (fil_typ == 3 && dmax != 0.0)
        printf(MSG_INFO_APPROX_ERR_REAL, f_name, dmax, idx);

    // informa o maior erro de aproximacao pra imag do comp
    if (fil_typ == 4 && dmax != 0.0)
        printf(MSG_INFO_APPROX_ERR_IMAG, f_name, dmax, idx);

    fclose(f_file);
}

// ----------------------------------------------------------------------------
// funcoes de manipulacao de arrays -------------------------------------------
// ----------------------------------------------------------------------------

// adiciona array na memoria de dados
// se for array normal (f_name = ""), completa com zero
// se for array inicializado, chama fill_mem para preencher
void arr_add(int size, int type, char *f_name, FILE *f_data)
{
    // incrementa o tamanho da memoria de acordo
    var_inc(size-1);
    // se nao tem arquivo, preenche com zero
    if (strcmp(f_name, "") == 0)
        for (int i = 0; i < size; i++)
        {
            if (type > 1)
                fprintf(f_data, "%s\n", itob(f2mf("0.0",NULL), nubits)); // se for float ou comp, inicializa com 0.0
            else
                fprintf(f_data, "%s\n", itob(0,nubits));                 // se for int, inicializa com 0
        }
    else
        fill_mem(f_name, size, type, f_data);
}