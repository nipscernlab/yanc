// ----------------------------------------------------------------------------
// routines for simulation with iverilog/gtkwave ------------------------------
// ----------------------------------------------------------------------------

// global includes
#include  <stdio.h>
#include <string.h>

// local includes
#include "..\Headers\eval.h"
#include "..\Headers\variaveis.h"

// ----------------------------------------------------------------------------
// local variables ------------------------------------------------------------
// ----------------------------------------------------------------------------

// simulation parameters
int clk_frq;           // simulation clock frequency
int clk_num;           // max number of clocks to simulate
int sim_typ;           // simulation type (single-proc or multicore)

// translation file
FILE *f_tran;          // opcode translation file
int  sim_n_opc = 0;    // number of instructions in the translation file
int  sim_fim;          // address of the end of the program

// user-declared variables
int  s_count  = 0;     // number of registered variables
char s_name[1000][64]; // name     of the found variable
int  s_addr[1000];     // address  of the found variable
int  s_type[1000];     // type     of the found variable

// user-declared arrays
int  s_count_arr  = 0;     // number of registered arrays
char s_name_arr[1000][64]; // name     of the found array
int  s_addr_arr[1000];     // address  of the found array
int  s_type_arr[1000];     // type     of the found array
int  s_size_arr[1000];     // size     of the found array

// ----------------------------------------------------------------------------
// helper functions -----------------------------------------------------------
// ----------------------------------------------------------------------------

// helper that searches cmm_log.txt to check whether ...
// char *va points to a variable declared in the .cmm
// also returns the type of the variable found
int sim_is_var(char *va, int *tipo, int *is_global, char *nome)
{
    // open the log file ------------------------------------------------------

    char path[2048]; snprintf(path, sizeof(path), "%s/cmm_log.txt", temp_dir);
    FILE *input = fopen(path, "r");

    // scan the file lines looking for the variable ---------------------------

    char texto[1001];
    char funcao[128];
    char variav[128];

    int ok = 0;
    while(fgets(texto, 1001, input) != NULL)
    {
        // pick up the variable's 3 parameters --------------------------------

        if (sscanf (texto, "%s %s %d", funcao, variav, tipo) != 3) continue;

        // build the variable name from the log data --------------------------

        // global variables don't include the function name
        if (strcmp(funcao,"global")==0)
        {
            *is_global = 1;
            sprintf(nome , "%s", variav);
        }
        else
        {
            *is_global = 0;
            sprintf(nome , "%s_%s", funcao, variav);
        }

        // if it matches the wanted variable, build its name and break --------

        if (strcmp(nome,va) == 0)
        {
            sprintf(nome , "%s_v_%s", funcao, variav);
            ok = 1; break;
        }
    }

    fclose(input);

    return ok;
}

// helper that searches cmm_log.txt to check whether ...
// char *va points to an array declared in the .cmm
// also returns the type and size of the array found
int sim_is_arr(char *va, int *tipo, int *size, int *is_global, char *nome)
{
    // open the log file ------------------------------------------------------

    char path[2048]; snprintf(path, sizeof(path), "%s/cmm_log.txt", temp_dir);
    FILE *input = fopen(path, "r");

    // scan the file lines looking for the variable --------------------------

    char texto[1001];
    char funcao[128];
    char variav[128];

    int ok = 0;
    while(fgets(texto, 1001, input) != NULL)
    {
        // pick up the array's 4 parameters -----------------------------------

        if (sscanf (texto, "%s %s %d %d", funcao, variav, tipo, size) != 4) continue;

        // build the array name from the log data -----------------------------

        // global arrays don't include the function name
        if (strcmp(funcao,"global")==0)
        {
            *is_global = 1;
            sprintf(nome , "%s", variav);
        }
        else
        {
            *is_global = 0;
            sprintf(nome , "%s_%s", funcao, variav);
        }

        // if it matches the wanted array, build its name and break -----------

        if (strcmp(nome,va) == 0)
        {
            sprintf(nome , "%s_v_%s", funcao, variav);
            ok = 1; break;
        }
    }

    fclose(input);

    return ok;
}

// ----------------------------------------------------------------------------
// routines to access simulation parameters -----------------------------------
// ----------------------------------------------------------------------------

int sim_clk    (){return clk_frq;} // returns the simulation clock frequency
int sim_clk_num(){return clk_num;} // returns the number of clocks to simulate
int sim_multi  (){return sim_typ;} // returns the simulation type (single-proc or multicore)

// ----------------------------------------------------------------------------
// routines for translation-file operations -----------------------------------
// ----------------------------------------------------------------------------

// creates the opcode translation file
// and initializes the simulation variables
void sim_init(int clk, int clk_n, int s_typ)
{
    // open the opcode translation file in the Temp folder
    char path[2048];
    snprintf(path, sizeof(path), "%s/trad_opcode.txt", temp_dir);
    f_tran  = fopen(path, "w");

    clk_frq = clk;   // simulation clock frequency
    clk_num = clk_n; // number of clocks to simulate
    sim_typ = s_typ; // simulation type (single-proc or multicore)
}

// appends an opcode and operand to the translation file
void sim_add(char *opc, char *opr)
{
    fprintf(f_tran , "%d %s %s\n", sim_n_opc++, opc, opr);
}

void sim_set_fim(int fim){sim_fim  = fim;} // sets the @fim address
int  sim_get_fim(       ){return sim_fim;} // returns the @fim address
void sim_finish (       ){fclose(f_tran);} // closes the translation file

// ----------------------------------------------------------------------------
// routines for user-variable operations --------------------------------------
// ----------------------------------------------------------------------------

// checks the log file to see whether it's a variable declared in the .cmm code
// if so, registers it for display in the simulator
int sim_regi(char *va)
{
    int  tipo;
    int  is_global;
    char var_name[128];

    if (sim_is_var(va, &tipo, &is_global, var_name))
    {
        if (is_global)
            sprintf(s_name[s_count], "me%d_f_global_v_%s_e_", tipo, va);
        else
            sprintf(s_name[s_count], "me%d_f_%s_e_", tipo, var_name);
        s_addr[s_count] = var_cnt()-1;
        s_type[s_count] = tipo;
        s_count++;
    }

    return tipo;
}

// checks cmm_log.txt to see whether it's an array declared in the .cmm code
// if so, registers it for display in the simulator
void sim_regi_arr(char *va)
{
    int  tipo, size;
    int  is_global;
    char var_name[128];

    if (sim_is_arr(va, &tipo, &size, &is_global, var_name))
    {
        if (is_global)
            sprintf(s_name_arr[s_count_arr], "arr_me%d_f_global_v_%s_e_", tipo, va);
        else
            sprintf(s_name_arr[s_count_arr], "arr_me%d_f_%s_e_", tipo, var_name);
        s_addr_arr[s_count_arr] = var_cnt()-1;
        s_type_arr[s_count_arr] = tipo;
        s_size_arr[s_count_arr] = size;
        s_count_arr++;
    }
}

// returns the memory contents
void sim_mem(int addr, char *val)
{
    char aux[2048]; snprintf(aux, sizeof(aux), "%s/Hardware/%s_data.mif", proc_dir, prname);
    FILE *f_data = fopen(aux, "r");

    int i = 0;
    while (fgets(aux,sizeof(aux),f_data))
    {
        if (i == addr)
        {
            aux[strcspn(aux, "\r\n")] = 0; // remove newline characters
            strcpy(val, aux);
            break;
        }
        i++;
    }

    fclose(f_data);
}

char* sim_name    (int i){return s_name    [i];} // returns the variable name
int   sim_addr    (int i){return s_addr    [i];} // returns the variable address
int   sim_type    (int i){return s_type    [i];} // returns the variable type
int   sim_cont    (     ){return s_count      ;} // returns the number of registered variables

char* sim_name_arr(int i){return s_name_arr[i];} // returns the array name
int   sim_addr_arr(int i){return s_addr_arr[i];} // returns the array address
int   sim_type_arr(int i){return s_type_arr[i];} // returns the array type
int   sim_cont_arr(     ){return s_count_arr  ;} // returns the number of registered arrays
int   sim_size_arr(int i){return s_size_arr[i];} // returns the array size
