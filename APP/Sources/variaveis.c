// ----------------------------------------------------------------------------
// tabela de variaveis --------------------------------------------------------
// ----------------------------------------------------------------------------

// includes globais
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "..\Headers\messages.h"

#define NVARMAX 999999 // mudar pra array dinamico

int  v_count = 0;
char v_name[NVARMAX][512];

// funcoes auxiliares ---------------------------------------------------------

// ve se uma variavel ja foi usada
// se sim, pega o indice na tabela
// se nao, retorna -1
int var_find(char *val)
{
	int ind = -1;

	for (int i = 0; i < v_count; i++)
		if (strcmp(val, v_name[i]) == 0) {ind = i; break;}
        
	return ind;
}

// funcoes de interface global ------------------------------------------------

// addiciona uma nova variavel na tabela
// pode ser um vetor com size > 1
void var_add(char *va, int size)
{
    if (var_find(va) == -1)
    {
        strcpy(v_name[v_count], va);
        v_count += size;
    }

    if (v_count > NVARMAX) {fprintf(stderr, MSG_ERR_TOO_MANY_VARS, NVARMAX); exit(EXIT_FAILURE);}
}

// retorna o numero de variaveis
int var_cnt(void)
{
    return v_count;
}
