// ----------------------------------------------------------------------------
// global functions and variables of the compiler -----------------------------
// ----------------------------------------------------------------------------

// global includes
#include <stdarg.h>
#include <string.h>
#include  <ctype.h>

// local includes
#include "..\Headers\t2t.h"
#include "..\Headers\emit.h"
#include "..\Headers\macros.h"
#include "..\Headers\global.h"
#include "..\Headers\diretivas.h"
#include "..\Headers\variaveis.h"
#include "..\Headers\messages.h"

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
int  line_num = 0;    // line number being parsed
int  num_ins  = 0;    // number of instructions parsed
int  sim_arr  = 0;    // tells whether the array is simulated or not

// One-instruction window for the SET-then-LOD peephole. Holds the most
// recent string passed to add_instr so the next call can detect a redundant
// load. Cleared by every control-flow boundary (labels via add_sinst, capture
// push/pop, macro start/end) so the drop only fires within a basic block.
static char last_str[256] = "";

void emit_peephole_reset(void)
{
    last_str[0] = '\0';
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

    char cmm_file[1024]; sprintf(cmm_file, "%s/Software/%s"    , d_proc, f_name); // input .cmm file name
    char asm_file[1024]; sprintf(asm_file, "%s/Software/%s.asm", d_proc, prname); // output .asm file name

    yyin  = fopen(cmm_file, "r"); // open the .cmm file
    f_asm = fopen(asm_file, "w"); // create the .asm file

    sprintf(dir_soft , "%s/Software", d_proc ); // record the Software directory
    strcpy (dir_macro,                d_macro); // record the Macro directory
    strcpy (dir_tmp  ,                d_tmp  ); // record the Tmp directory

    sim_arr = strcmp(d_array, "1") == 0;

    // create helper files ----------------------------------------------------

    char path[2048];

    snprintf(path, sizeof(path),   "%s/cmm_log.txt", dir_tmp        ); f_log = fopen(path,"w"); // log with info for the assembler and gtkwave
    snprintf(path, sizeof(path), "%s/pc_%s_mem.txt", dir_tmp, prname); f_lin = fopen(path,"w"); // memory in pc.v that bridges asm to cmm

    // emit a NOP instruction at the start (try to remove this) ---------------

    add_sinst(-1,"NOP\n");
}

void parse_end(char *prname, char *d_proc)
{
    printf(MSG_INFO_INS_GENERATED, num_ins);

    // close the files --------------------------------------------------------

    fclose(f_asm); // assembly code
    fclose(f_lin); // assembly translator

    // check whether macros need to be appended to the .asm file --------------

    char asm_file[1024]; sprintf(asm_file, "%s/Software/%s.asm", d_proc, prname);

	mac_copy(asm_file);

	// check consistency of all variables and functions -----------------------

	check_var(); // (variaveis.c)

    // finalize the cmm log file ----------------------------------------------

    fprintf(f_log, "num_ins %d\n", num_ins); // number of instructions (excluding final macros)
    fclose (f_log);

    // generate the translation file for the cmm code -------------------------

    char     path[2048]; snprintf(path, sizeof(path), "%s/%s", dir_tmp, "trad_cmm.txt");
    char cmm_file[1024]; sprintf(cmm_file, "%s/Software/%s.cmm", d_proc, prname);

    FILE *output = fopen(path    , "w");
    FILE *input  = fopen(cmm_file, "r");

    char linha[1001], texto[1001] = "";
    fputs("-1 INTERNAL\n"    , output); // code for the file start
    fputs("-2 void main();\n", output); // code for CAL main
    fputs("-3 END\n"         , output); // code for @fim JMP fim
    fputs("-4 User Macro\n"  , output); // user-supplied asm code (#USEMAC)

    int cnt = 1;
    while(fgets(texto, 1001, input) != NULL)
    {
        substituir_braket(texto);
        sprintf(linha, "%d %s", cnt++, texto);
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
    // peephole: drop "LOD x" right after "SET x" -----------------------------
    // ------------------------------------------------------------------------
    // SET stores acc to memory and leaves acc unchanged (instr_dec.v: opcode 4
    // -> ula_op 0). So reloading the just-stored value is a no-op. Only fires
    // when both lines are the plain SET / LOD opcodes (not SET_P / F_SET /
    // P_LOD / etc.) and the variable names match. last_str is reset by
    // add_sinst, the capture stack, and the macro hooks - so this never
    // crosses a label, a basic-block boundary, or an opaque macro emit.

    if (mac_using == 0 &&
        strncmp(str,      "LOD ", 4) == 0 &&
        strncmp(last_str, "SET ", 4) == 0)
    {
        char lod_var[100], set_var[100];
        if (sscanf(str      + 4, "%99s", lod_var) == 1 &&
            sscanf(last_str + 4, "%99s", set_var) == 1 &&
            strcmp(lod_var, set_var) == 0)
        {
            last_str[0] = '\0';
            return;
        }
    }

    // ------------------------------------------------------------------------
    // append the instruction -------------------------------------------------
    // ------------------------------------------------------------------------
    // emit_is_capturing() is true when a parent construct (e.g. an if body)
    // is buffering its contents to be wrapped into an ast_node later.
    // In that case the asm text goes into the capture buffer, not f_asm.
    // f_lin / num_ins bookkeeping stays immediate either way.

    if (mac_using == 0)
    {
        if (emit_is_capturing()) emit_append(str);
        else                     fprintf(f_asm, "%s", str);
    }

    // table for the gtkwave assembly translator ------------------------------

    if (mac_using == 0) num_ins++;
    if (mac_using == 0) fprintf(f_lin, "%s\n", itob(line_num+1,20));

    // ------------------------------------------------------------------------
    // check whether the instruction needs a special macro --------------------
    // ------------------------------------------------------------------------

    if (find_opc("float_sqrt", str)) mac_add("fsqrt");
    if (find_opc("float_atan", str)) mac_add("fatan");
    if (find_opc("float_sin" , str)) mac_add("fsin" );

    // remember this emit for the next peephole check (only when we actually
    // appended to f_asm or a capture; otherwise the window stays as it was)
    if (mac_using == 0)
    {
        strncpy(last_str, str, sizeof(last_str) - 1);
        last_str[sizeof(last_str) - 1] = '\0';
    }
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

    // append the instruction (capture buffer or f_asm, see add_instr) --------

    if (mac_using == 0)
    {
        if (emit_is_capturing()) emit_append(str);
        else                     fprintf(f_asm, "%s", str);
    }

    // table for the gtkwave assembly translator ------------------------------

    if (type != 0)
    {
        if (mac_using == 0) num_ins++;
        if (mac_using == 0) fprintf(f_lin, "%s\n", itob(type,20));
    }
}
