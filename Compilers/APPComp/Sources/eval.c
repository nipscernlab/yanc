// ----------------------------------------------------------------------------
// routines used during lexical analysis of the assembly code -----------------
// ----------------------------------------------------------------------------

// global includes
#include  <stdio.h>
#include <string.h>
#include <stdlib.h>

// local includes
#include "../Headers/variaveis.h"
#include "../Headers/messages.h"

// ----------------------------------------------------------------------------
// local variable declarations ------------------------------------------------
// ----------------------------------------------------------------------------

// helper variables for the array lexer
int   tam_arr;      // array size
char name_arr[128]; // name of the array currently being read

// state variables
int  n_ins = 0;     // number of instructions
int  state = 0;     // compiler state

FILE *f_log;        // log file

// ----------------------------------------------------------------------------
// routines that act on the lexer ---------------------------------------------
// ----------------------------------------------------------------------------

// runs before the lexer
void eval_init(char *path)
{
    char    file[1001];
    snprintf(file, sizeof(file), "%s/app_log.txt", path);

    f_log = fopen(file, "w");

    if (f_log == NULL) {fprintf(stderr, MSG_ERR_CANT_CREATE_LOG, path); exit(EXIT_FAILURE);}
}

// runs when a directive is found
void eval_direct(int next_state)
{
    // moves to the state that picks up the directive's specific argument
    state = next_state;
}

// runs when the #ITRAD directive is found
void eval_itrad()
{
    // current instruction is registered as the interrupt point
    fprintf(f_log, "itr_addr %d\n", n_ins);
     printf(MSG_INFO_ITR_HANDLING);
}

// runs when the #TOAQUI directive is found
// records the current instruction index so the hardware can compare PC against
// it and pulse the cheguei pin
void eval_toaqui()
{
    fprintf(f_log, "toaqui_addr %d\n", n_ins);
}

// runs when a new opcode is found
void eval_opcode(int next_state)
{
    // next state depends on the opcode type:
    // 0 : no operand
    // 17: operand is a data-memory address
    // 18: operand is an instruction-memory address
    state = next_state;

    // no operand, so we can already count an instruction
    if (state == 0) n_ins++;
}

// runs when a new operand is found
void eval_opernd(char *va)
{
    switch (state)
    {
        case  1: fprintf(f_log, "prname %s\n", va ); state =  0; break; // processor name
        case  2: fprintf(f_log, "nubits %s\n", va ); state =  0; break; // ALU word width (bits)
        case  3: fprintf(f_log, "nbmant %s\n", va ); state =  0; break; // mantissa width (bits)
        case  4: fprintf(f_log, "nbexpo %s\n", va ); state =  0; break; // exponent width (bits)
        case 11: strcpy (name_arr,             va ); state = 12; break; // found an array without initialization
        case 12:                                     state = 13; break; // pick up data type (not needed in app)
        case 13: var_add(name_arr,        atoi(va)); state =  0; break; // declare array without initialization
        case 14: strcpy (name_arr,             va ); state = 15; break; // found an array with initialization
        case 15:                                     state = 16; break; // pick up data type (not needed in app)
        case 16:          tam_arr =       atoi(va ); state = 17; break; // pick up the file-initialized array size
        case 17: var_add(name_arr,         tam_arr); state =  0; break; // fill memory with values from file
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

// runs when a new label is found
void eval_label(char *va)
{
    fprintf(f_log, "%s %d\n", va, n_ins);
}

// runs after the lexer
void eval_finish()
{
    // finalize log file
    fprintf(f_log, "n_ins %d\n", n_ins    );
    fprintf(f_log, "n_dat %d\n", var_cnt());
    fclose (f_log);

    // checks whether data memory can be created
    if (var_cnt() <= 2) {fprintf(stderr, MSG_ERR_USELESS_PROC); exit(EXIT_FAILURE);}

    printf(MSG_INFO_INS_VAR_FOUND, n_ins, var_cnt());
}
