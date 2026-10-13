// ----------------------------------------------------------------------------
// asmcomp's first pass: the scanner's actions and its data-word table ---------
// ----------------------------------------------------------------------------
// (it was the appcomp program: eval.c and variaveis.c). pass1.l calls these;
// the result is <folder of the .asm>/app_log.txt, read by the second pass.

// global includes
#include  <stdio.h>
#include <string.h>
#include <stdlib.h>
#include  <ctype.h>

// local includes
#include "../Headers/pass1.h"
#include "../Headers/messages.h"

// ----------------------------------------------------------------------------
// the data-word table --------------------------------------------------------
// ----------------------------------------------------------------------------

static int    v_count = 0;
static int    v_cap   = 0;
static char (*v_name)[512] = NULL;

static void var_grow(int needed)
{
    if (needed <= v_cap) return;
    int new_cap = v_cap ? v_cap : 256;
    while (new_cap < needed) new_cap *= 2;

    void *t = realloc(v_name, (size_t)new_cap * sizeof(*v_name));
    if (!t) {fprintf(stderr, MSG_ERR_OUT_OF_MEMORY); exit(EXIT_FAILURE);}

    v_name = t;
    memset(v_name + v_cap, 0, (size_t)(new_cap - v_cap) * sizeof(*v_name));
    v_cap = new_cap;
}

// the index of a variable already in the table, or -1
static int var_find(char *val)
{
    for (int i = 0; i < v_count; i++)
        if (strcmp(val, v_name[i]) == 0) return i;
    return -1;
}

// #SHARE <name> <home>: <name> has no word of its own, it uses <home>'s
// (asm_share, in Compilers/common, decides which variables can)
static int    sh_count = 0;
static char (*sh_name)[512] = NULL;
static char (*sh_home)[512] = NULL;

static char *var_home(char *va)
{
    for (int i = 0; i < sh_count; i++)
        if (strcmp(va, sh_name[i]) == 0) return sh_home[i];
    return va;
}

// records that va shares home's word
static void var_share(char *va, char *home)
{
    sh_name = realloc(sh_name, (size_t)(sh_count + 1) * sizeof(*sh_name));
    sh_home = realloc(sh_home, (size_t)(sh_count + 1) * sizeof(*sh_home));
    if (!sh_name || !sh_home) {fprintf(stderr, MSG_ERR_OUT_OF_MEMORY); exit(EXIT_FAILURE);}
    snprintf(sh_name[sh_count], sizeof(*sh_name), "%s", va  );
    snprintf(sh_home[sh_count], sizeof(*sh_home), "%s", home);
    sh_count++;
}

// adds a variable, or an array of `size` words
static void var_add(char *va, int size)
{
    va = var_home(va); // a shared name takes its home's word

    if (var_find(va) == -1)
    {
        var_grow(v_count + size);
        strcpy(v_name[v_count], va);
        v_count += size;
    }
}

// ----------------------------------------------------------------------------
// the scanner's actions ------------------------------------------------------
// ----------------------------------------------------------------------------

static int   tam_arr;       // array size
static char  name_arr[128]; // name of the array currently being read
static char  name_sh [512]; // name of the #SHARE variable currently being read

static int   n_ins = 0;     // number of instructions
static int   state = 0;     // scanner state

static FILE *f_log;         // app_log.txt

// runs before the scanner; name is the processor's (the .asm's file name)
void p1_init(const char *dir, const char *name)
{
    char    file[1001];
    snprintf(file, sizeof(file), "%s/app_log.txt", dir);

    f_log = fopen(file, "w");

    if (f_log == NULL) {fprintf(stderr, MSG_ERR_CANT_CREATE_LOG, dir); exit(EXIT_FAILURE);}

    fprintf(f_log, "prname %s\n", name);
}

// a directive: the next operand is its argument
void p1_direct(int next_state)
{
    state = next_state;
}

// #ITRAD: the current instruction is the interrupt point
void p1_itrad(void)
{
    fprintf(f_log, "itr_addr %d\n", n_ins);
     printf(MSG_INFO_ITR_HANDLING);
}

// #TOAQUI: the hardware compares the PC against this instruction and pulses
// the cheguei pin
void p1_toaqui(void)
{
    fprintf(f_log, "toaqui_addr %d\n", n_ins);
}

// an opcode; next_state says what its operand is:
// 0 : no operand (the instruction counts already)
// 18: a data-memory address
// 19: an instruction-memory address
void p1_opcode(int next_state)
{
    state = next_state;

    if (state == 0) n_ins++;
}

// an operand
void p1_opernd(char *va)
{
    switch (state)
    {
        case  1:                                     state =  0; break; // #PRNAME: the compilers' copy of the name; the file's wins
        case  2: fprintf(f_log, "nubits %s\n", va ); state =  0; break; // ALU word width (bits)
        case  3: fprintf(f_log, "nbmant %s\n", va ); state =  0; break; // mantissa width (bits)
        case  4: fprintf(f_log, "nbexpo %s\n", va ); state =  0; break; // exponent width (bits)
        case 25: fprintf(f_log, "fround %s\n", va ); state =  0; break; // float rounding level
        case 26: snprintf(name_sh, sizeof(name_sh), "%s", va); state = 27; break; // #SHARE: the name
        case 27: var_share(name_sh,            va ); state =  0; break; // #SHARE: the home it uses
        case 28: if (!isdigit((unsigned char)va[0]) && va[0] != '-' && va[0] != '+')
                     var_add(va,1);                                          // LDI/STI <array>: a data operand
                 n_ins++;                            state =  0; break; // LDI/STI <number>: raw base, no data word
        case 11: strcpy (name_arr,             va ); state = 12; break; // found an array without initialization
        case 12:                                     state = 13; break; // its data type (not needed here)
        case 13: var_add(name_arr,        atoi(va)); state =  0; break; // declare array without initialization
        case 14: strcpy (name_arr,             va ); state = 15; break; // found an array with initialization
        case 15:                                     state = 16; break; // its data type (not needed here)
        case 16:          tam_arr =       atoi(va ); state = 17; break; // the file-initialized array's size
        case 17: var_add(name_arr,         tam_arr); state =  0; break; // its words
        case 18: var_add(va,1);             n_ins++; state =  0; break; // ALU operations
        case 19:                            n_ins++; state =  0; break; // jump operations
        case 20:                            n_ins++; state =  0; break; // input operations
        case 21:                            n_ins++; state =  0; break; // output operations
        case 22:                                     state = 23; break; // prepare constant offset
        case 23:                            n_ins++; state =  0; break; // instr with constant offset
        case 24: {                                                       // LEA <name>: target + synthetic addr-of constant
            var_add(va,1);
            char syn[600]; snprintf(syn, sizeof(syn), "__addr_%s", va);
            var_add(syn,1);
            n_ins++;
            state = 0;
            break;
        }
    }
}

// a label: its address
void p1_label(char *va)
{
    fprintf(f_log, "%s %d\n", va, n_ins);
}

// runs after the scanner
void p1_finish(void)
{
    fprintf(f_log, "n_ins %d\n", n_ins  );
    fprintf(f_log, "n_dat %d\n", v_count);
    fclose (f_log);

    // no minimum here: a program with 2 or fewer data words (a button-to-LED
    // loop) is valid, and the second pass pads the data memory to the 2 words
    // the hardware needs (eval_init)

    printf(MSG_INFO_INS_VAR_FOUND, n_ins, v_count);
}
