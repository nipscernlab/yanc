// ----------------------------------------------------------------------------
// rotinas para manipular variaveis encontradas no arquivo .asm ---------------
// ----------------------------------------------------------------------------

#define NVARMAX 999999 // trocar para arrays dinamicos

// includes globais
#include  <stdio.h>
#include <string.h>
#include <stdlib.h>

// includes locais
#include "..\Headers\t2t.h"
#include "..\Headers\messages.h"

// ----------------------------------------------------------------------------
// variaveis locais -----------------------------------------------------------
// ----------------------------------------------------------------------------

int  v_count = 0;
char v_name[NVARMAX][512];
int  v_val [NVARMAX];

// ----------------------------------------------------------------------------
// rotinas de interface -------------------------------------------------------
// ----------------------------------------------------------------------------

// addiciona uma nova variavel na tabela
// se o operando for uma constante, converte seu valor para binario ...
void var_add(char *var, int is_const)
{
    if (v_count == NVARMAX)
    {
        fprintf(stderr, MSG_ERR_TOO_MANY_VARS, NVARMAX);
        exit(EXIT_FAILURE);
    }

    // transforma char *var pra int val
    int   val;
    float delta;
    switch(is_const)
    {
        case 0: val = 0;                break; // nao eh constante
        case 1: val = atoi(var);        break; // constante tipo int
        case 2: val = f2mf(var,&delta); break; // constante tipo float
    }

    strcpy(v_name [v_count], var);
    v_val [v_count]        = val ;
    v_count++;
}

// ve se uma variavel ja foi usado
// se sim, pega o indice na tabela
// se nao, retorna -1
int var_find(char *val)
{
	int i, ind = -1;

	for (i = 0; i < v_count; i++)
		if (strcmp(val, v_name[i]) == 0)
		{
			ind = i;
			break;
		}
	return ind;
}

void var_inc (int   val){v_count += val             ;} // incrementa o tamanho da memoria (para arrays)
int  var_val (char *var){return v_val[var_find(var)];} // retorna o valor  da variavel
int  var_cnt (         ){return v_count             ;} // retorna o numero de variaveis