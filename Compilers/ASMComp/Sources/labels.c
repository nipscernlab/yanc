// ----------------------------------------------------------------------------
// label handling routines ----------------------------------------------------
// ----------------------------------------------------------------------------

// global includes
#include <string.h>
#include <stdlib.h>
#include  <stdio.h>

// local includes
#include "../Headers/eval.h"
#include "../Headers/simulacao.h"
#include "../Headers/messages.h"
#include "../../common/xstr.h"

// ----------------------------------------------------------------------------
// local variables ------------------------------------------------------------
// ----------------------------------------------------------------------------

static int   l_cap   = 0;
static char **l_name = NULL;    // names of any length
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

    l_name = xrealloc(l_name, (size_t)new_cap * sizeof(*l_name));
    l_val  = xrealloc(l_val , (size_t)new_cap * sizeof(*l_val ));
    memset(l_name + l_cap, 0, (size_t)(new_cap - l_cap) * sizeof(*l_name));
    memset(l_val  + l_cap, 0, (size_t)(new_cap - l_cap) * sizeof(*l_val ));
    l_cap = new_cap;
}

// appends a label to the label vector
void add_label(char *la, int val)
{
    lab_grow(l_count + 1);
    l_name[l_count] = xstrdup(la);
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
    char *path = xasprintf("%s/app_log.txt", temp_dir);
    FILE *input = fopen(path, "r");
    if (input == NULL) fatal(MSG_ERR_FILE_WHERE, path);
    free(path);

    // scan the log file
    char *linha;
    int   val;
    while ((linha = xgetline(input)) != NULL)
    {
        char *nome = xrealloc(NULL, strlen(linha) + 1);   // no word is longer than its line
        if (sscanf(linha, "@%s %d", nome, &val) == 2)
        {
            add_label(nome, val);                          // register the label
            if (strcmp(nome,"fim") == 0) sim_set_fim(val); // set the @fim address
        }
        free(nome); free(linha);
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
