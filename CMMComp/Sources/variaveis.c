// ----------------------------------------------------------------------------
// implementacao da tabela de indentificadores --------------------------------
// ----------------------------------------------------------------------------

// includes globais
#include <string.h>
#include <stdlib.h>
#include   <math.h>

// includes locais
#include "..\Headers\global.h"
#include "..\Headers\funcoes.h"
#include "..\Headers\variaveis.h"
#include "..\Headers\diretivas.h"
#include "..\Headers\messages.h"

int  v_count = 0;          // guarda o tamanho da tabela
char v_name[NVARMAX][512]; // nome da variavel ou funcao
int  v_isar[NVARMAX];      // se variavel eh um array
int  v_type[NVARMAX];      // 0 -> nao identificada, 1 -> int, 2 -> float
int  v_fnid[NVARMAX];      // ID da funcao a qual a variavel pertence
int  v_used[NVARMAX];      // se ID ja foi usado
int  v_isco[NVARMAX];      // se variavel eh uma constante
int  v_fpar[NVARMAX];      // se ID eh uma funcao, diz a lista de parametros
int  v_size[NVARMAX];      // tamanho do array (caso seja array)
int  v_siz2[NVARMAX];      // tamanho da dimensao j (caso seja matriz)

// procura variavel na tabela (-1 se nao achar)
int find_var(char *val)
{
	int i, ind = -1;

	for (i = 0; i < v_count; i++)
	{
		if (strcmp(val, v_name[i]) == 0)
		{
			ind = i;
			break;
		}
	}
	return ind;
}

// adiciona variavel na tabela
void add_var(char *var)
{
    if (v_count == NVARMAX)
    {
        fprintf (stderr, MSG_ERR_TOO_MANY_VARS, NVARMAX);
        exit(EXIT_FAILURE);
    }
    else
    {
        strcpy(v_name[v_count], var);
        v_count++;
    }
}

// checa quais variaveis e funcoes foram usadas (no final do parse)
void check_var()
{
    if (mainok == 0) {fprintf (stderr, MSG_ERR_NO_MAIN); exit(EXIT_FAILURE);}

    // varre toda a tabela de variaveis
    for (int i = 0; i < v_count; i++)
    {
        // checa se variavel foi declarada e nao foi usada ...
        if (((v_type[i] == 1) || (v_type[i] == 2) || (v_type[i] == 3)) && (v_used[i] == 0))
        {
            // checa se eh ou nao global
            if (strcmp(v_name[v_fnid[i]], "") == 0)
                fprintf (stdout, MSG_WARN_UNUSED_GLOBAL_VAR, v_name[i]);
            else
                fprintf (stdout, MSG_WARN_UNUSED_LOCAL_VAR, rem_fname(v_name[i], v_name[v_fnid[i]]), v_name[v_fnid[i]]);
        }

        // checa se a funcao foi declarada e nao foi usada
        if (((v_type[i] == 5) || (v_type[i] == 6) || (v_type[i] == 7)) && v_used[i] == 0)
            fprintf (stdout, MSG_WARN_UNUSED_FUNCTION, v_name[i]);
    }
}

// tira o nome da funcao da variavel
char* rem_fname(char *var, char *fname)
{
    if (strcmp(fname,"") == 0) return var;
    int    ind = 0;
    while (var[ind] == fname[ind]) ind++;
    // se ind != strlen(fname) eh pq a variavel nao
    // contem todo o nome da funcao no comeco
    // entao melhor nao remover nada (deve ser global)
    return (ind == strlen(fname)) ? var + ind + 1 : var; // o +1 eh pra tirar o '_'
}

// usado quando o lexer acha um ID
int exec_id(char *text)
{
    if (strcmp(text,"i") == 0)
        {fprintf (stderr, MSG_ERR_RESERVED_I, line_num+1); exit(EXIT_FAILURE);}

    char var_name[64];

    if (find_var(text) == -1)                                     // primeiro ve se tem uma variavel global
    {
            strcpy  (var_name, fname);
        if (strcmp  (var_name, "") != 0) strcat (var_name, "_");  // se nao, coloca o nome da funcao atual antes
            strcat  (var_name, text);
        if (find_var(var_name)    == -1) add_var(var_name);       // cria a variavel local, caso ela ainda nao exista
    }
    else
    {
        strcpy(var_name, text);
    }
    return find_var(var_name);
}

// usado quando o lexer acha uma constante int
int exec_inum(char *text)
{
    // verifica limites -------------------------------------------------------
    // nessa funcao nunca entra constante negativa

    int max = (int) (pow(2,nbmant+nbexpo+1-1)-1);
    int num = atoi(text);

    if (num > max) {fprintf (stderr, MSG_ERR_INT_MAX_OVERFLOW, line_num+1, max); exit(EXIT_FAILURE);}

    // adiciona na tabela -----------------------------------------------------

    if (find_var(text) == -1) add_var(text);

    int id = find_var(text);

    v_isco[id] = 1;

    return id;
}

// converte float ieee 32 bits para meu float
// tentar mudar pra converter float de 64 bits
void f2mf(char *va, int *s, int *m, int *e)
{
    float f = atof(va);

    if (f == 0.0) {*m = 0; *e = 0;}

    int *ifl = (int*)&f;

    // desempacota padrao IEEE ------------------------------------------------

    *s = ( *ifl >> 31) & 0x00000001;
    *e = ((*ifl >> 23) & 0xFF) - 127 - 22;
    *m = ((*ifl & 0x007FFFFF) + 0x00800000) >> 1;

    // expoente ---------------------------------------------------------------

    *e = *e + (23-nbmant);

    int sh = 0;
    while (*e < -pow(2, nbexpo-1)) {*e = *e+1; sh = sh+1;}

    // mantissa ---------------------------------------------------------------

    if (nbmant == 23)
    {
        if (*ifl & 0x00000001) *m = *m+1; // arredonda
    }
    else
    {
        sh = 23-nbmant+sh;
        int carry = (*m >> (sh-1)) & 0x00000001; // carry de arredondamento
        *m = *m >> sh;
        if (carry) *m = *m+1; // arredonda
    }
}

// usado quando o lexer acha uma constante float
int exec_fnum(char *text)
{
    // verifica limites -------------------------------------------------------
    // nunca entra o sinal negativo da constante aqui

    float max = (float)((pow(2,nbmant)-1) * pow(2, pow(2,nbexpo-1)-1)); // maior valor possivel em modulo
    float min = (float)(                    pow(2,-pow(2,nbexpo-1)  )); // menor valor possivel em modulo
    float num = atof(text);                                             //       valor do num   em modulo
    float abs = (num < 0.0) ? -num : num;                               //       valor do num   em modulo

    if (abs < min && abs != 0.0) {fprintf (stderr, MSG_ERR_FLOAT_MIN, line_num+1, min); exit(EXIT_FAILURE);}
    if (abs > max)               {fprintf (stderr, MSG_ERR_FLOAT_MAX, line_num+1, max); exit(EXIT_FAILURE);}

    // calcula residuo --------------------------------------------------------

    int s,m,e; f2mf(text,&s,&m,&e);
    
    float mf    = (s) ? -m*pow(2,e) : m*pow(2,e);
    float delta = mf-num;

    if (delta != 0.0 && num != 0.0) printf(MSG_INFO_CONST_APPROX,text,line_num+1,mf,delta);

    // adiciona na tabela -----------------------------------------------------

    if (find_var(text) == -1) add_var(text);

    int id = find_var(text);

    v_isco[id] = 1;

    return id;
}

// usado quando o lexer acha uma constante comp
int exec_cnum(char *text)
{
    // remova espacos em branco -----------------------------------------------

    int i = 0, j = 0;
    char temp[strlen(text) + 1];
    strcpy(temp, text);
    while (temp[i] != '\0')
    {
        while (temp[i] == ' ') i++;
        text[j] = temp[i];
        i++;
        j++;
    }
    text[j] = '\0';

    // adiciona na tabela -----------------------------------------------------

    if (find_var(text) == -1) add_var(text);
    return find_var(text);
}