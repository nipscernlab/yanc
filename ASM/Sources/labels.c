// ----------------------------------------------------------------------------
// rotinas para tratamento de labels ------------------------------------------
// ----------------------------------------------------------------------------

#define NLABMAX 99999 // trocar para arrays dinamicos

// includes globais
#include <string.h>
#include <stdlib.h>
#include  <stdio.h>

// includes locais
#include "..\Headers\eval.h"
#include "..\Headers\simulacao.h"
#include "..\Headers\messages.h"

// ----------------------------------------------------------------------------
// variaveis locais -----------------------------------------------------------
// ----------------------------------------------------------------------------

char l_name[NLABMAX][512];
int  l_val [NLABMAX];
int  l_count;

// ----------------------------------------------------------------------------
// funcoes auxiliares ---------------------------------------------------------
// ----------------------------------------------------------------------------

// adiciona labels ao vetor de labels
void add_label(char *la, int val)
{
    if (l_count == NLABMAX)
    {
        fprintf(stderr, MSG_ERR_TOO_MANY_LABELS, NLABMAX);
        exit(EXIT_FAILURE);
    }
    else
    {
        strcpy(l_name[l_count], la);
        l_val[l_count] = val;
        l_count++;
    }
}

// ----------------------------------------------------------------------------
// funcoes de interface -------------------------------------------------------
// ----------------------------------------------------------------------------

// pega todos os labels no arquivo de log
void lab_reg()
{
    // abre o arquivo de log
    char path[1024];
    sprintf(path, "%s/app_log.txt", temp_dir);
    FILE *input = fopen(path, "r");
    
    // faz a varredura no arquivo de log
    char linha[1001];
    char nome [128];
    int  val;
    while (fgets(linha, sizeof(linha), input))
    {
        if (sscanf(linha, "@%s %d", nome, &val) == 2)
        {
            add_label(nome, val);                          // cadastra label
            if (strcmp(nome,"fim") == 0) sim_set_fim(val); // define endereco de @fim
        }
    }

    fclose(input);
}

// pega o indice da label
int lab_find(char *la)
{
	for (int i = 0; i < l_count; i++) 
        if (strcmp(la, l_name[i]) == 0)
            return l_val[i];

	return -1;
}