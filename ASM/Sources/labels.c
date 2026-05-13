// ----------------------------------------------------------------------------
// label handling routines ----------------------------------------------------
// ----------------------------------------------------------------------------

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

static int   l_cap   = 0;
static char (*l_name)[512] = NULL;
static int  *l_val         = NULL;
int  l_count = 0;

// ----------------------------------------------------------------------------
// helper functions -----------------------------------------------------------
// ----------------------------------------------------------------------------

static void lab_grow(int needed)
{
    if (needed <= l_cap) return;
    int new_cap = l_cap ? l_cap : 128;
    while (new_cap < needed) new_cap *= 2;

    void *t1 = realloc(l_name, (size_t)new_cap * sizeof(*l_name));
    void *t2 = realloc(l_val , (size_t)new_cap * sizeof(*l_val ));
    if (!t1 || !t2) {fprintf(stderr, MSG_ERR_OUT_OF_MEMORY); exit(EXIT_FAILURE);}

    l_name = t1;
    l_val  = t2;
    memset(l_name + l_cap, 0, (size_t)(new_cap - l_cap) * sizeof(*l_name));
    memset(l_val  + l_cap, 0, (size_t)(new_cap - l_cap) * sizeof(*l_val ));
    l_cap = new_cap;
}

// appends a label to the label vector
void add_label(char *la, int val)
{
    lab_grow(l_count + 1);
    strcpy(l_name[l_count], la);
    l_val[l_count] = val;
    l_count++;
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
