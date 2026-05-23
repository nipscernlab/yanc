// ----------------------------------------------------------------------------
// routines for generating the memories' .mif files ... -----------------------
// as the lexer scans the .asm ------------------------------------------------
// ----------------------------------------------------------------------------

#define NBITS_OPC 7 // must match the verilog (in proc.v)

// global includes
#include   <math.h>
#include  <stdio.h>
#include <string.h>
#include <stdlib.h>

// local includes
#include "..\Headers\t2t.h"
#include "..\Headers\hdl.h"
#include "..\Headers\eval.h"
#include "..\Headers\array.h"
#include "..\Headers\labels.h"
#include "..\Headers\opcodes.h"
#include "..\Headers\variaveis.h"
#include "..\Headers\simulacao.h"
#include "..\Headers\messages.h"

// ----------------------------------------------------------------------------
// global variable definitions ------------------------------------------------
// ----------------------------------------------------------------------------

// directories used to access the files
char proc_dir[1024];    // processor directory
char temp_dir[1024];    // Tmp folder directory
char  hdl_dir[1024];    // HDL folder directory
char  mac_dir[1024];    // Macros folder directory

// stores the directive values
char prname   [128];    // processor name
int  nubits    = 23;    // ALU word width (bits)
int  nbmant    = 16;    // mantissa width (bits)
int  nbexpo    =  6;    // exponent width (bits)
int  ddepth    = 10;    // data stack depth
int  sdepth    = 10;    // subroutine stack depth
int  nuioin    =  1;    // number of input ports
int  nuioou    =  1;    // number of output ports
int  nugain    = 64;    // division constant
int  fftsiz    =  8;    // FFT size (bits)

// ----------------------------------------------------------------------------
// local variables ------------------------------------------------------------
// ----------------------------------------------------------------------------

FILE *f_data, *f_instr; // .mif of the data and instruction memories

// state variables
int  state =   0 ;      // tracks the compiler state
char opc_name[64];      // stores the current opcode name
char  va_name[64];      // stores the current variable name
int  opc_idx;           // stores the current opcode index
int  arr_typ;           // stores the array type
int  arr_tam;           // stores the array size

// helper variables
int  n_ins	  = 0;      // number of instructions added
int  n_dat    = 0;      // number of variables added
int  i_used[256];       // marks which input was used
int  o_used[256];       // marks which output was used
int  itr_addr    = 0;   // interrupt address (#INTERPOINT)
int  toaqui_addr = 0;   // #TOAQUI marker address (cheguei pin trigger)
int  nbopr;             // number of operand bits

// ----------------------------------------------------------------------------
// helper functions -----------------------------------------------------------
// ----------------------------------------------------------------------------

// fetches a parameter from the log file
int eval_get(char *fname, char *var, char *val)
{
    // open the log file
    char path[2048]; snprintf(path, sizeof(path), "%s/%s", temp_dir, fname);
    FILE *input = fopen(path , "r");
    if   (input == NULL) {fprintf(stderr, MSG_ERR_FILE_WHERE, path); exit(EXIT_FAILURE);}

    char linha[1001];
    char nome [128 ];
    while (fgets (linha,sizeof(linha),input)) if (sscanf(linha,"%s %s", nome, val) == 2) if (strcmp(nome,var) == 0)
    {
        fclose(input);
        return 1; // variable found
    }

    fclose(input);
    return 0; // variable not found
}

// registers an ALU instruction
// adds the variable to data memory
// finally, places the instruction in instruction memory
void instr_ula(char *va, int is_const)
{
    // if it's the first time the var shows up, register it
    if (var_find(va) == -1)
    {
        var_add (va, is_const);  // adds the variable to the table
        int type = sim_regi(va); // registers the variable in the simulator (if it belongs to the user)

        // if it's a float variable, initialize with zero
        if (!is_const && type > 1)
            fprintf (f_data, "%s\n", itob(f2mf("0.0",NULL), nubits)); // adds variable to data memory
        else
            fprintf (f_data, "%s\n", itob(var_val(va     ), nubits)); // adds variable to data memory
    }

    // write the new instruction
    fprintf(f_instr, "%s%s\n" , itob(opc_idx,NBITS_OPC), itob(var_find(va),nbopr));
    // also register it in the simulation translator
    sim_add(opc_name,va);
}

// registers jump instructions
void instr_salto(char *va)
{
    // write the new instruction
    fprintf(f_instr, "%s%s\n" , itob(opc_idx,NBITS_OPC), itob(lab_find(va),nbopr));
    // also register it in the simulation translator
    sim_add(opc_name,va);
}

// registers input instructions
void instr_inn(char *va)
{
    // write the new instruction
    fprintf(f_instr, "%s%s\n" , itob(opc_idx,NBITS_OPC), itob(atoi(va),nbopr));
    // register it in the simulation translator
    sim_add(opc_name,va);
    // mark the input index as used
    i_used[atoi(va)] = 1;
}

// registers output instructions
void instr_out(char *va)
{
    // write the new instruction
    fprintf(f_instr, "%s%s\n" , itob(opc_idx,NBITS_OPC), itob(atoi(va),nbopr));
    // also register it in the simulation translator
    sim_add(opc_name,va);
    // mark the output index as used
    o_used[atoi(va)] = 1;
}

// registers array declarations
void instr_arr(char *va)
{
         var_add(va ,0); // adds the array to the table
    sim_regi_arr(va   ); // registers the array in the simulator (looks up info in cmm_log.txt)
}

// LEA <name>: load-effective-address pseudo-instruction
// resolves <name> to its data-memory address, materialises that address as a
// numeric constant cell, then emits a plain LOD pointing to that cell. The
// callee at runtime gets the array's base as a value in the accumulator and
// can use ADD + LDA / STA on it. opc_idx must already be 0 (LOD) by the time
// this is called (set by eval_opcode("LEA", ...)).
void instr_lea(char *va)
{
    int addr = var_find(va);
    if (addr == -1)
    {
        // address-taken-only variable (e.g. `int v; f(&v);` where v is never
        // read/written by name): allocate it now exactly as instr_ula would —
        // table entry + a zero-initialised data cell (asmcomp is single-pass,
        // so the cell must be appended to the .mif in first-seen order).
        var_add(va, 0);
        int type = sim_regi(va);
        if (type > 1) fprintf(f_data, "%s\n", itob(f2mf("0.0", NULL), nubits)); // float
        else          fprintf(f_data, "%s\n", itob(0, nubits));                 // int
        addr = var_find(va);
    }
    // materialise &va as a constant cell named "__addr_<va>". using a name
    // distinct from any plain integer literal avoids the dedup collision
    // that the literal "<addr>" would have with a later LOD <addr>, which
    // would make asmcomp and appcomp disagree on n_dat.
    char syn[600];
    snprintf(syn, sizeof(syn), "__addr_%s", va);
    if (var_find(syn) == -1)
    {
        var_add_with_val(syn, addr);                  // value = address of va
        sim_regi(syn);                                // pass through the sim registry, harmless if not a user var
        fprintf(f_data, "%s\n", itob(addr, nubits));  // emit the constant value into the data .mif
    }
    // emit LOD <syn> (opc_idx is already 0 — LOD — set by eval_opcode("LEA",0,...))
    fprintf(f_instr, "%s%s\n", itob(opc_idx, NBITS_OPC), itob(var_find(syn), nbopr));
    sim_add(opc_name, va); // log "LEA va" in the trace
}

// generates an instruction with address va_name + va (pseudo instruction)
void instr_oft(char *va)
{
    // write the new instruction
    fprintf(f_instr, "%s%s\n" , itob(opc_idx,NBITS_OPC), itob(var_find(va_name)+atoi(va),nbopr));

    // also register it in the simulation translator
    strcat ( va_name, " ");
    strcat ( va_name, va );
    sim_add(opc_name, va_name);
}

// ----------------------------------------------------------------------------
// global functions -----------------------------------------------------------
// ----------------------------------------------------------------------------

// number of instructions
int get_n_ins()
{
    char aux[256]; eval_get("cmm_log.txt","num_ins", aux);

    return atoi(aux);
}

int inn_used(int i) {return i_used[i];} // tells whether input port  i was used
int out_used(int i) {return o_used[i];} // tells whether output port i was used

// ----------------------------------------------------------------------------
// lexer evolution functions --------------------------------------------------
// ----------------------------------------------------------------------------

// runs before the lexer starts
void eval_init(int clk, int clk_n, int s_typ)
{
    // reset used I/O indices -------------------------------------------------

    for (int i = 0; i < 256; i++) {i_used[i] = 0; o_used[i] = 0;}

    // pull parameters from app_log.txt ---------------------------------------

    char aux[2048];

    eval_get("app_log.txt","prname", prname);                     // processor name
    eval_get("app_log.txt","n_ins" ,    aux); n_ins  = atoi(aux); // number of instructions added
    eval_get("app_log.txt","n_dat" ,    aux); n_dat  = atoi(aux); // number of variables added
    eval_get("app_log.txt","nubits",    aux); nubits = atoi(aux); // ALU word width (bits)
    eval_get("app_log.txt","nbmant",    aux); nbmant = atoi(aux); // mantissa width (bits)
    eval_get("app_log.txt","nbexpo",    aux); nbexpo = atoi(aux); // exponent width (bits)

    // if there's an interrupt, pull its address
    if (eval_get("app_log.txt","itr_addr", aux) == 1) itr_addr = atoi(aux); // interrupt address

    // if there's a #TOAQUI marker, pull its address
    if (eval_get("app_log.txt","toaqui_addr", aux) == 1) toaqui_addr = atoi(aux); // cheguei marker address

    lab_reg(); // register labels found in app_log.txt

    // determine the number of address bits for the operand (after the mnemonic)
    // depends on whichever is larger, data or instruction memory ------------

    nbopr = (n_ins > n_dat) ? ceil(log2(n_ins)) : ceil(log2(n_dat));

    // open the .mif files ----------------------------------------------------

    snprintf(aux, sizeof(aux), "%s/Hardware/%s_data.mif", proc_dir, prname); f_data  = fopen(aux, "w");
    snprintf(aux, sizeof(aux), "%s/Hardware/%s_inst.mif", proc_dir, prname); f_instr = fopen(aux, "w");

    // initialize routines for iverilog simulation ----------------------------

    sim_init(clk, clk_n, s_typ);
}

// runs when a directive is found
void eval_direct(int next_state)
{
    // moves to the state that picks up the directive's specific argument
    state = next_state;
}

// runs when a new opcode is found
void eval_opcode(int op, int next_state, char *text, char *nome)
{
    opc_idx = op;          // record the current opcode
    strcpy(opc_name,text); // store the current opcode name for the translation file

    // next state depends on the opcode type:
    // 0 : no operand
    // 18: operand is a data-memory address
    // 19: operand is an instruction-memory address
    // 20: operand is an input address
    // 21: operand is an output address
    // 22: operand is fixed indirect addressing (pseudo instruction)
    state = next_state;

    // no operand, so we can already write the instruction
    if (state == 0)
    {
        fprintf(f_instr, "%s%s\n", itob(op,NBITS_OPC), itob(0,nbopr));
        sim_add(opc_name,"");
    }
    // register the opcode
    opc_add(nome);
}

// runs when an operand is found
void eval_opernd(char *va, int is_const)
{
    switch (state)
    {
        case  5: ddepth =  atoi(va);                    state =  0; break; // data stack depth
        case  6: sdepth =  atoi(va);                    state =  0; break; // instruction stack depth
        case  7: nuioin =  atoi(va);                    state =  0; break; // number of input addresses
        case  8: nuioou =  atoi(va);                    state =  0; break; // number of output addresses
        case  9: nugain =  atoi(va);                    state =  0; break; // normalization value
        case 10: fftsiz =  atoi(va);                    state =  0; break; // number of bits to reverse in the FFT
        case 11: instr_arr     (va);                    state = 12; break; // found an array without initialization
        case 12: arr_typ = atoi(va);                    state = 13; break; // pick up the array type
        case 13: arr_add  (atoi(va),arr_typ,"",f_data); state =  0; break; // declare array without initialization
        case 14: instr_arr     (va);                    state = 15; break; // found an array with initialization
        case 15: arr_typ = atoi(va);                    state = 16; break; // pick up the array type
        case 16: arr_tam = atoi(va);                    state = 17; break; // pick up the file-initialized array size
        case 17: arr_add  (arr_tam,arr_typ,va, f_data); state =  0; break; // fill memory with file values (zeros if no file)
        case 18: instr_ula     (va,is_const);           state =  0; break; // ALU operations
        case 19: instr_salto   (va);                    state =  0; break; // jump operations
        case 20: instr_inn     (va);                    state =  0; break; // input operations
        case 21: instr_out     (va);                    state =  0; break; // output operations
        case 22: strcpy(va_name,va);                    state = 23; break; // prepare constant offset
        case 23: instr_oft     (va);                    state =  0; break; // instr with constant offset
        case 24: instr_lea     (va);                    state =  0; break; // load-effective-address pseudo (LEA -> LOD <const>)
    }
}

// runs after the lexer
void eval_finish()
{
    // close the .mif files ---------------------------------------------------

    fclose(f_instr);
    fclose(f_data );

    // check floating-point consistency ---------------------------------------

    if (nubits != nbmant+nbexpo+1) {fprintf(stderr, MSG_ERR_FP_INCONSISTENT); exit(EXIT_FAILURE);}

    // finalize simulation ----------------------------------------------------

    sim_finish();

    // generate hdl files -----------------------------------------------------

    hdl_vv_file(n_ins,n_dat,nbopr,itr_addr,toaqui_addr); // top-level verilog file for the processor
    hdl_tb_file(itr_addr,toaqui_addr);                   // verilog testbench file
}
