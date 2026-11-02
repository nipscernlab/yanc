// ----------------------------------------------------------------------------
// routines for simulation with iverilog/gtkwave ------------------------------
// ----------------------------------------------------------------------------

// global includes
#include  <stdio.h>
#include <string.h>
#include <stdlib.h>

// local includes
#include "../Headers/eval.h"
#include "../Headers/variaveis.h"
#include "../Headers/simulacao.h"
#include "../Headers/messages.h"
#include "../../common/xstr.h"

// ----------------------------------------------------------------------------
// local variables ------------------------------------------------------------
// ----------------------------------------------------------------------------

// simulation parameters
int clk_frq;           // simulation clock frequency
int clk_num;           // max number of clocks to simulate

// translation file
FILE *f_tran;          // opcode translation file
int  sim_n_opc = 0;    // number of instructions in the translation file
int  sim_fim;          // address of the end of the program

// user-declared variables: as many as the program has, names of any length
typedef struct
{
    char *name;        // its signal in the testbench (me<type>_f_<function>_v_<name>_e_)
    char *raw;         // its name as the .asm spells it
    int   addr;        // address of its word
    int   type;        // type
    int   shar;        // in a #SHARE group: followed by its writes, not its address
    int  *st;          // instruction indices of the SETs that write it (if shared)
    int   nst;         // how many
} sim_var;
static sim_var *sv;
static int      s_count = 0, s_cap = 0;

// user-declared arrays
typedef struct
{
    char *name;        // its signals' prefix in the testbench
    int   addr;        // address of its first word
    int   type;        // type
    int   size;        // words
} sim_arr;
static sim_arr *sa;
static int      s_count_arr = 0, sa_cap = 0;

// ----------------------------------------------------------------------------
// helper functions -----------------------------------------------------------
// ----------------------------------------------------------------------------

// searches prog_log.txt for the variable (arr = 0: lines "<function> <name>
// <type>") or the array (arr = 1: "<function> <name> <type> <size>") that the
// .asm calls va (a global by its name, a local as <function>_<name>); on a
// match, returns 1, its type (and size) and, in *nome, "<function>_v_<name>"
static int sim_find(const char *va, int arr, int *tipo, int *size, int *is_global, char **nome)
{
    char path[2048]; snprintf(path, sizeof(path), "%s/prog_log.txt", temp_dir);
    FILE *input = fopen(path, "r");
    if (input == NULL) return 0;   // an .asm written by hand: no source names

    int ok = 0;
    char *texto;
    while (!ok && (texto = xgetline(input)) != NULL)
    {
        size_t n = strlen(texto) + 1;               // no word is longer than its line
        char *funcao = xrealloc(NULL, n), *variav = xrealloc(NULL, n);
        int t, z = 0;
        int got = arr ? sscanf(texto, "%s %s %d %d", funcao, variav, &t, &z)
                      : sscanf(texto, "%s %s %d"   , funcao, variav, &t);
        if (got == (arr ? 4 : 3))
        {
            int global = strcmp(funcao, "global") == 0;
            char *asm_name = global ? xstrdup(variav) : xasprintf("%s_%s", funcao, variav);
            if (strcmp(asm_name, va) == 0)
            {
                *tipo = t; if (size) *size = z;
                *is_global = global;
                *nome = xasprintf("%s_v_%s", funcao, variav);
                ok = 1;
            }
            free(asm_name);
        }
        free(funcao); free(variav); free(texto);
    }

    fclose(input);
    return ok;
}

// ----------------------------------------------------------------------------
// routines to access simulation parameters -----------------------------------
// ----------------------------------------------------------------------------

int sim_clk    (){return clk_frq;} // returns the simulation clock frequency
int sim_clk_num(){return clk_num;} // returns the number of clocks to simulate

// ----------------------------------------------------------------------------
// routines for translation-file operations -----------------------------------
// ----------------------------------------------------------------------------

// creates the opcode translation file
// and initializes the simulation variables
void sim_init(int clk, int clk_n)
{
    // open the opcode translation file in the Temp folder
    char path[2048];
    snprintf(path, sizeof(path), "%s/trad_opcode.txt", temp_dir);
    f_tran  = fopen(path, "w");

    clk_frq = clk;   // simulation clock frequency
    clk_num = clk_n; // number of clocks to simulate
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
    return sim_regi_at(va, var_cnt()-1);
}

// same, for a variable whose word is at addr (a #SHARE name uses its home's)
int sim_regi_at(char *va, int addr)
{
    int   tipo = 0;
    int   is_global;
    char *var_name;

    if (sim_find(va, 0, &tipo, NULL, &is_global, &var_name))
    {
        if (s_count == s_cap) sv = xrealloc(sv, (size_t)(s_cap = s_cap ? 2 * s_cap : 64) * sizeof(*sv));
        sim_var *v = &sv[s_count++];
        v->name = is_global ? xasprintf("me%d_f_global_v_%s_e_", tipo, va)
                            : xasprintf("me%d_f_%s_e_", tipo, var_name);
        v->raw  = xstrdup(va);
        v->addr = addr;
        v->type = tipo;
        v->shar = var_shared(va);
        v->st   = NULL;
        v->nst  = 0;
        free(var_name);
    }

    return tipo;
}

// whether va is already registered
int sim_has(char *va)
{
    for (int i = 0; i < s_count; i++) if (strcmp(sv[i].raw, va) == 0) return 1;
    return 0;
}

// the instruction about to be added (sim_n_opc) writes va
void sim_store(char *va)
{
    for (int i = 0; i < s_count; i++) if (strcmp(sv[i].raw, va) == 0)
    {
        sv[i].st = xrealloc(sv[i].st, (size_t)(sv[i].nst + 1) * sizeof(int));
        sv[i].st[sv[i].nst++] = sim_n_opc;
        return;
    }
}

// checks prog_log.txt to see whether it's an array declared in the .cmm code
// if so, registers it for display in the simulator
void sim_regi_arr(char *va)
{
    int   tipo, size;
    int   is_global;
    char *var_name;

    if (sim_find(va, 1, &tipo, &size, &is_global, &var_name))
    {
        if (s_count_arr == sa_cap) sa = xrealloc(sa, (size_t)(sa_cap = sa_cap ? 2 * sa_cap : 16) * sizeof(*sa));
        sim_arr *r = &sa[s_count_arr++];
        r->name = is_global ? xasprintf("arr_me%d_f_global_v_%s_e_", tipo, va)
                            : xasprintf("arr_me%d_f_%s_e_", tipo, var_name);
        r->addr = var_cnt()-1;
        r->type = tipo;
        r->size = size;
        free(var_name);
    }
}

// the word at addr of the data memory, as its binary digits (the caller frees it)
char *sim_mem(int addr)
{
    char aux[2048]; snprintf(aux, sizeof(aux), "%s/%s_data.mif", proc_dir, prname);
    FILE *f_data = fopen(aux, "r");
    if (f_data == NULL) {fprintf(stderr, MSG_ERR_CANT_OPEN_FILE, aux); exit(EXIT_FAILURE);}

    char *line, *val = NULL;
    for (int i = 0; (line = xgetline(f_data)) != NULL; i++)
    {
        if (i == addr) {val = line; break;}
        free(line);
    }
    fclose(f_data);
    return val ? val : xstrdup("0");
}

char* sim_name    (int i){return sv[i].name   ;} // returns the variable name
int   sim_addr    (int i){return sv[i].addr   ;} // returns the variable address
int   sim_type    (int i){return sv[i].type   ;} // returns the variable type
int   sim_cont    (     ){return s_count      ;} // returns the number of registered variables
int   sim_shared  (int i){return sv[i].shar   ;} // returns whether the variable is in a #SHARE group
int   sim_nstore  (int i){return sv[i].nst    ;} // returns how many SETs write it
int   sim_store_at(int i, int k){return sv[i].st[k];} // returns the instruction index of the k-th

char* sim_name_arr(int i){return sa[i].name   ;} // returns the array name
int   sim_addr_arr(int i){return sa[i].addr   ;} // returns the array address
int   sim_type_arr(int i){return sa[i].type   ;} // returns the array type
int   sim_cont_arr(     ){return s_count_arr  ;} // returns the number of registered arrays
int   sim_size_arr(int i){return sa[i].size   ;} // returns the array size
