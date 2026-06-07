// ----------------------------------------------------------------------------
// global functions and variables of the compiler -----------------------------
// ----------------------------------------------------------------------------

// global includes
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include  <ctype.h>

// local includes
#include "../Headers/t2t.h"
#include "../Headers/macros.h"
#include "../Headers/global.h"
#include "../Headers/diretivas.h"
#include "../Headers/variaveis.h"
#include "../Headers/messages.h"

// ----------------------------------------------------------------------------
// global variable definitions ------------------------------------------------
// ----------------------------------------------------------------------------

FILE *f_asm;          // output file
FILE *f_log;          // log file holding project information (name, nbmant, etc.)
FILE *f_lin;          // line file for the cmm program

char dir_macro[1024]; // Macros directory
char dir_tmp  [1024]; // Temp directory
char dir_soft [1024]; // Software directory

// state variables
int  acc_ok   = 0;    // 0 -> acc empty (use LOD)  , 1 -> acc loaded (use P_LOD)
int  line_num = 0;    // parser-time: line the lexer is currently reading
int  emit_line= 1;    // emit-time: line the AST walker tags onto each instruction
char emit_fname[512] = ""; // emit-time: name of the function whose body the AST
                           // walker is in, for rem_fname() in diagnostics only
                           // (exec_id keeps using the global fname). "" = global
int  num_ins  = 0;    // number of instructions parsed
int  sim_arr  = 0;    // tells whether the array is simulated or not

// One-instruction window for the SET-then-LOD peephole. Holds the most
// recent string passed to add_instr so the next call can detect a redundant
// load. Cleared by every control-flow boundary (labels via add_sinst, capture
// push/pop, macro start/end) so the drop only fires within a basic block.
static char last_str[256] = "";

// Name of the variable/constant currently sitting in the accumulator, or "" when
// unknown. A plain LOD/P_LOD/SET of an operand leaves that operand's value in the
// acc; every other op clobbers it. add_instr drops a "LOD x" whenever acc_name is
// already x (the acc holds it), which generalises the old "LOD x after SET x"
// peephole. Cleared at every basic-block boundary alongside last_str.
static char acc_name[100] = "";

void emit_peephole_reset(void)
{
    last_str[0] = '\0';
    acc_name[0] = '\0';
}

// True when the accumulator is known to currently hold variable `name`. Used by
// the codegen (e.g. the comp c²+d² in oper_divi / exec_mod2) to square the
// acc-resident half first, so its LOD is dropped by the peephole above.
int acc_holds(const char *name)
{
    return name && name[0] && strcmp(acc_name, name) == 0;
}

// ----------------------------------------------------------------------------
// helper functions -----------------------------------------------------------
// ----------------------------------------------------------------------------

// helper that finds the right instruction inside add_instr
int find_opc(const char *opc, const char *str)
{
    size_t len = strlen(opc);
    const char *p = str;

    while ((p = strstr(p, opc)) != NULL)
    {
        char before = (p == str) ? '\0' : *(p - 1);
        char after = *(p + len);

        int before_ok = (p == str) || before == '\0' || isspace((unsigned char)before);
        int  after_ok =                after == '\0' || isspace((unsigned char) after);

        if (before_ok && after_ok) return 1;

        p += len;
    }

    return 0;
}

// Replaces ⟨ with < and ⟩ with >
// so they can be displayed in gtkwave
void substituir_braket(char *str)
{
    unsigned char *src = (unsigned char *)str;
    unsigned char *dst = (unsigned char *)str;

    while (*src)
    {
        // ⟨ = 0xE2 0x9F 0xA8
        if (src[0] == 0xE2 && src[1] == 0x9F && src[2] == 0xA8)
        {
            *dst++ = '<';
            src += 3;
        }
        // ⟩ = 0xE2 0x9F 0xA9
        else if (src[0] == 0xE2 && src[1] == 0x9F && src[2] == 0xA9)
        {
            *dst++ = '>';
            src += 3;
        }
        else *dst++ = *src++;
    }
    *dst = '\0';
}

// ----------------------------------------------------------------------------
// parser start and end functions ---------------------------------------------
// ----------------------------------------------------------------------------

void parse_init(char *f_name, char *prname, char *d_proc, char *d_macro, char *d_tmp, char *d_array)
{
    // pick up the arguments --------------------------------------------------

    char cmm_file[1024]; snprintf(cmm_file, sizeof(cmm_file), "%s/Software/%s"    , d_proc, f_name); // input .cmm file name
    char asm_file[1024]; snprintf(asm_file, sizeof(asm_file), "%s/Software/%s.asm", d_proc, prname); // output .asm file name

    yyin  = fopen(cmm_file, "r"); // open the .cmm file
    if (yyin  == NULL) { fprintf(stderr, MSG_ERR_CANT_OPEN_FILE, cmm_file); exit(EXIT_FAILURE); }
    f_asm = fopen(asm_file, "w"); // create the .asm file
    if (f_asm == NULL) { fprintf(stderr, MSG_ERR_CANT_OPEN_FILE, asm_file); exit(EXIT_FAILURE); }

    snprintf(dir_soft, sizeof(dir_soft), "%s/Software", d_proc); // record the Software directory
    strcpy (dir_macro,                d_macro); // record the Macro directory
    strcpy (dir_tmp  ,                d_tmp  ); // record the Tmp directory

    sim_arr = strcmp(d_array, "1") == 0;

    // create helper files ----------------------------------------------------

    char path[2048];

    snprintf(path, sizeof(path),   "%s/cmm_log.txt", dir_tmp        ); f_log = fopen(path,"w"); // log with info for the assembler and gtkwave
    if (f_log == NULL) { fprintf(stderr, MSG_ERR_CANT_OPEN_FILE, path); exit(EXIT_FAILURE); }
    snprintf(path, sizeof(path), "%s/pc_%s_mem.txt", dir_tmp, prname); f_lin = fopen(path,"w"); // memory in pc.v that bridges asm to cmm
    if (f_lin == NULL) { fprintf(stderr, MSG_ERR_CANT_OPEN_FILE, path); exit(EXIT_FAILURE); }

    // Open the top-level stmt_list collector. Every directive, declaration
    // and function reduce appends a stmt_node here; nothing reaches f_asm
    // until prog_emit() runs the walker at `fim`.
    stmt_list_open();
}

// Fires from the `fim : prog` action once parsing is complete. The whole
// program is now an AST in the global stmt_list; this is where it gets
// flattened into assembly. NOP is emitted as a fixed prefix (the processor
// expects instruction address 0 to be a no-op) and then every top-level
// stmt is walked in source order: directives, declarations, function bodies.
void prog_emit(void)
{
    add_sinst(-1, "NOP\n");

    stmt_node *prog = stmt_list_close();
    stmt_emit(prog);
    stmt_free(prog);
}

void parse_end(char *prname, char *d_proc)
{
    printf(MSG_INFO_INS_GENERATED, num_ins);

    // close the files --------------------------------------------------------

    fclose(f_asm); // assembly code
    fclose(f_lin); // assembly translator

    // check whether macros need to be appended to the .asm file --------------

    char asm_file[1024]; snprintf(asm_file, sizeof(asm_file), "%s/Software/%s.asm", d_proc, prname);

	mac_copy(asm_file);

	// check consistency of all variables and functions -----------------------

	check_var(); // (variaveis.c)

    // finalize the cmm log file ----------------------------------------------

    fprintf(f_log, "num_ins %d\n", num_ins); // number of instructions (excluding final macros)
    fclose (f_log);

    // generate the translation file for the cmm code -------------------------

    char     path[2048]; snprintf(path, sizeof(path), "%s/%s", dir_tmp, "trad_cmm.txt");
    char cmm_file[1024]; snprintf(cmm_file, sizeof(cmm_file), "%s/Software/%s.cmm", d_proc, prname);

    FILE *output = fopen(path    , "w");
    if (output == NULL) { fprintf(stderr, MSG_ERR_CANT_OPEN_FILE, path);     exit(EXIT_FAILURE); }
    FILE *input  = fopen(cmm_file, "r");
    if (input  == NULL) { fprintf(stderr, MSG_ERR_CANT_OPEN_FILE, cmm_file); exit(EXIT_FAILURE); }

    char linha[1001], texto[1001] = "";
    fputs("-1 INTERNAL\n"    , output); // code for the file start
    fputs("-2 void main();\n", output); // code for CAL main
    fputs("-3 END\n"         , output); // code for @fim JMP fim

    int cnt = 1;
    while(fgets(texto, 1001, input) != NULL)
    {
        substituir_braket(texto);
        snprintf(linha, sizeof(linha), "%d %s", cnt++, texto);
        fputs(linha, output);
        memset(texto, 0, sizeof(char) * 1001);
    }

    fclose(input );
    fclose(output);
}

// ----------------------------------------------------------------------------
// functions for registering instructions -------------------------------------
// ----------------------------------------------------------------------------

// adds an instruction to the asm file
void add_instr(char *inst, ...)
{
    va_list  args;
    va_start(args , inst);

    char     str[100];
    vsprintf(str, inst, args);
    va_end  (args);

    // ------------------------------------------------------------------------
    // peephole: drop "LOD x" when the accumulator already holds x -------------
    // ------------------------------------------------------------------------
    // acc_name tracks the operand currently in the acc: a plain LOD / P_LOD /
    // SET of an operand leaves its value there (SET stores acc to memory and
    // leaves acc unchanged -- instr_dec.v opcode 4 -> ula_op 0); every other op
    // clobbers it. So a "LOD x" while acc_name == x is a no-op and is dropped.
    // This subsumes the old "LOD x after SET x" case. Only the plain LOD is
    // dropped -- P_LOD also pushes, so it must always run. acc_name is cleared
    // at every basic-block boundary (emit_peephole_reset), so the drop never
    // crosses a label or control-flow merge.

    if (strncmp(str, "LOD ", 4) == 0)
    {
        char lod_var[100];
        if (sscanf(str + 4, "%99s", lod_var) == 1 && strcmp(lod_var, acc_name) == 0)
            return;   // acc already holds it -- acc_name / last_str stay valid
    }

    // ------------------------------------------------------------------------
    // append the instruction -------------------------------------------------
    // ------------------------------------------------------------------------

    fprintf(f_asm, "%s", str);

    // table for the gtkwave assembly translator ------------------------------

    num_ins++;
    fprintf(f_lin, "%s\n", itob(emit_line,20));

    // ------------------------------------------------------------------------
    // check whether the instruction needs a special macro --------------------
    // ------------------------------------------------------------------------

    if (find_opc("float_sqrt", str)) mac_add("fsqrt");
    if (find_opc("float_atan", str)) mac_add("fatan");
    if (find_opc("float_sin" , str)) mac_add("fsin" );
    if (find_opc("float_tan" , str)) mac_add("ftan" );
    if (find_opc("float_exp" , str)) mac_add("fexp" );
    if (find_opc("float_log" , str)) mac_add("flog" );

    // track what is now in the accumulator: only a plain LOD / P_LOD / SET of an
    // operand leaves that operand's value there; anything else clobbers it.
    if      (strncmp(str, "LOD ", 4) == 0 || strncmp(str, "SET ", 4) == 0)
        sscanf(str + 4, "%99s", acc_name);
    else if (strncmp(str, "P_LOD ", 6) == 0)
        sscanf(str + 6, "%99s", acc_name);
    else
        acc_name[0] = '\0';

    // remember this emit for the next peephole check
    strncpy(last_str, str, sizeof(last_str) - 1);
    last_str[sizeof(last_str) - 1] = '\0';
}

// adds special instructions
// type =  0 -> comment
// type = -1 -> INTERNAL
// type = -2 -> void main();
// type = -3 -> END
void add_sinst(int type, char *inst, ...)
{
    // Labels, directives and inline comments are control-flow / scaffolding
    // boundaries: a label may be a jump target, so dropping a LOD that
    // follows it would be unsafe. Reset the peephole window before emitting.
    emit_peephole_reset();

    va_list  args;
    va_start(args , inst);

    char     str[256];
    vsprintf(str, inst, args);
    va_end  (args);

    // append the instruction -------------------------------------------------

    fprintf(f_asm, "%s", str);

    // table for the gtkwave assembly translator ------------------------------

    if (type != 0)
    {
        num_ins++;
        fprintf(f_lin, "%s\n", itob(type,20));
    }
}
