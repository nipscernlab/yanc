// ----------------------------------------------------------------------------
// label handling routines ----------------------------------------------------
// ----------------------------------------------------------------------------

#define NLABMAX 99999 // switch to dynamic arrays later

// global includes
#include <string.h>
#include <stdlib.h>
#include  <stdio.h>

// local includes
#include "..\Headers\eval.h"
#include "..\Headers\simulacao.h"
#include "..\Headers\messages.h"

// ----------------------------------------------------------------------------
// local variables ------------------------------------------------------------
// ----------------------------------------------------------------------------

char l_name[NLABMAX][512];
int  l_val [NLABMAX];
int  l_count;

// ----------------------------------------------------------------------------
// helper functions -----------------------------------------------------------
// ----------------------------------------------------------------------------

// appends a label to the label vector
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
// interface functions --------------------------------------------------------
// ----------------------------------------------------------------------------

// reads every label from the log file
void lab_reg()
{
    // open the log file
    char path[1024];
    sprintf(path, "%s/app_log.txt", temp_dir);
    FILE *input = fopen(path, "r");

    // scan the log file
    char linha[1001];
    char nome [128];
    int  val;
    while (fgets(linha, sizeof(linha), input))
    {
        if (sscanf(linha, "@%s %d", nome, &val) == 2)
        {
            add_label(nome, val);                          // register the label
            if (strcmp(nome,"fim") == 0) sim_set_fim(val); // set the @fim address
        }
    }

    fclose(input);
}

// returns the label's index
int lab_find(char *la)
{
	for (int i = 0; i < l_count; i++)
        if (strcmp(la, l_name[i]) == 0)
            return l_val[i];

	return -1;
}
