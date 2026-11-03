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
#include "../Headers/dirac.h"
#include "../Headers/funcoes.h"
#include "../Headers/diag.h"
#include "../../common/asm_share.h"
#include "../../common/exe_dir.h"
#include "../../common/xstr.h"

// ----------------------------------------------------------------------------
// global variable definitions ------------------------------------------------
// ----------------------------------------------------------------------------

FILE *f_asm;          // output file
FILE *f_log;          // log file holding project information (name, nbmant, etc.)
FILE *f_lin;          // line file for the cmm program

const char *dir_macro = ""; // Macros directory (main's arguments: they live to the end)
const char *dir_tmp   = ""; // Temp directory
static const char *cmm_src = ""; // the .cmm the parser reads (re-read for src_lines.txt)

// state variables
int  acc_ok   = 0;    // 0 -> acc empty (use LOD)  , 1 -> acc loaded (use P_LOD)
int  line_num = 0;    // parser-time: line the lexer is currently reading
int  emit_line= 1;    // emit-time: line the AST walker tags onto each instruction
int  emit_fid = -1;   // emit-time: id of the function whose body the walker is in (-1: none)
const char *emit_fname = ""; // emit-time: name of the function whose body the AST
                           // walker is in, for rem_fname() in diagnostics only
                           // (exec_id keeps using the global fname). "" = global
int  num_ins  = 0;    // number of instructions parsed
int  sim_arr  = 0;    // tells whether the array is simulated or not

// One-instruction window for the SET-then-LOD peephole. Holds the most
// recent string passed to add_instr so the next call can detect a redundant
// load. Cleared by every control-flow boundary (labels via add_sinst, capture
// push/pop, macro start/end) so the drop only fires within a basic block.
static char *last_str = NULL;           // NULL: none

// Name of the variable/constant currently sitting in the accumulator, or "" when
// unknown. A plain LOD/P_LOD/SET of an operand leaves that operand's value in the
// acc; every other op clobbers it. add_instr drops a "LOD x" whenever x is among
// acc_names (the acc holds it), which generalises the old "LOD x after SET x"
// peephole. Cleared at every basic-block boundary alongside last_str. A SET
// adds its name to the ones the acc already holds: after LOD 0.0; SET a; SET b;
// SET c the acc is 0.0, a, b and c, and a fourth "LOD 0.0" is dropped too.
#define ACC_NAMES 32
static char *acc_names[ACC_NAMES];       // each in a block of its own
static int   acc_n = 0;                  // how many of them the acc holds

static void acc_clear(void)
{
    for (int i = 0; i < acc_n; i++) free(acc_names[i]);
    acc_n = 0;
}

static void acc_only(const char *name)   // the acc holds this one name ("": none)
{
    acc_clear();
    if (name && name[0]) acc_names[acc_n++] = xstrdup(name);
}

// a store leaves the acc as it is, but an element or an index key ("v[k]",
// "[ii+j]") goes, since the store may write a word the key reads
static void acc_drop_keys(void)
{
    int k = 0;
    for (int i = 0; i < acc_n; i++)
        if (!strchr(acc_names[i], '[')) acc_names[k++] = acc_names[i];
        else free(acc_names[i]);
    acc_n = k;
}

// a SET of `name`: the acc keeps what it held, and is `name` too
static void acc_add(const char *name)
{
    acc_drop_keys();
    int k = 0;
    for (int i = 0; i < acc_n; i++)
        if (strcmp(acc_names[i], name) != 0) acc_names[k++] = acc_names[i];
        else free(acc_names[i]);
    acc_n = k;
    if (acc_n == ACC_NAMES)              // full: the oldest goes
    {
        free(acc_names[0]);
        memmove(acc_names, acc_names + 1, (size_t)(acc_n - 1) * sizeof acc_names[0]);
        acc_n--;
    }
    acc_names[acc_n++] = xstrdup(name);
}

void emit_peephole_reset(void)
{
    free(last_str); last_str = NULL;
    acc_clear();
}

// True when the accumulator is known to currently hold variable `name`. Used by
// the codegen (e.g. the comp c²+d² in oper_div / exec_mod2) to square the
// acc-resident half first, so its LOD is dropped by the peephole above.
int acc_holds(const char *name)
{
    if (!name || !name[0]) return 0;
    for (int i = 0; i < acc_n; i++) if (strcmp(acc_names[i], name) == 0) return 1;
    return 0;
}

// An indexed store (STI, ISI) or read (LDI) leaves the element in the acc: the
// walker names it by the element, "v[k]", which no variable name can be, so
// that a read of the same element after it takes the acc (ast.c). Only when
// the store or read is the last instruction emitted; an instruction after it
// that keeps the acc (OUT, PSH, TOM, JIZ) keeps the name, any other forgets
// it, and a SET does too, since it may write an index
void acc_holds_elem(const char *key)
{
    if (last_str && (strncmp(last_str, "STI ", 4) == 0 || strncmp(last_str, "ISI ", 4) == 0
                  || strncmp(last_str, "LDI ", 4) == 0))   // a read leaves the element too
        acc_only(key);
}

// The index of an indexed store, computed into the acc: the walker names it
// "[ii+j]" so that a read at the same index inside the right-hand side pushes
// it with PSH instead of computing it again (ast.c). As above, any instruction
// but a SET forgets the name, and a SET does too
void acc_holds_index(const char *key)
{
    acc_only(key);
}

// ----------------------------------------------------------------------------
// helper functions -----------------------------------------------------------
// ----------------------------------------------------------------------------

// helper that finds the right instruction inside add_instr
static int find_opc(const char *opc, const char *str)
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
static void substituir_braket(char *str)
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

// the next line of f with its end of line, of any length (NULL at the end of
// the file); the block is the caller's
static char *read_src_line(FILE *f)
{
    char   chunk[256];
    char  *line = NULL;
    size_t len  = 0;
    while (fgets(chunk, sizeof chunk, f) != NULL)
    {
        size_t k = strlen(chunk);
        line = xrealloc(line, len + k + 1);
        memcpy(line + len, chunk, k + 1);
        len += k;
        if (k > 0 && chunk[k - 1] == '\n') break;
    }
    return line;
}

// the files this run writes. An error that stops cmmcomp removes them (#94
// N): a compilation that failed leaves no .asm or table that a later step,
// or the user, could take for the result
#define OUT_MAX 4
static char  *out_path[OUT_MAX];
static FILE **out_file[OUT_MAX];
static int    out_n = 0;

static void out_track(char *path, FILE **f)   // takes the block path
{
    out_path[out_n] = path;
    out_file[out_n] = f;
    out_n++;
}

static void out_remove(void)
{
    for (int i = 0; i < out_n; i++)
    {
        if (*out_file[i]) {fclose(*out_file[i]); *out_file[i] = NULL;}
        remove(out_path[i]);
        free(out_path[i]);
    }
    out_n = 0;
}

static FILE *f_src = NULL;            // src_lines.txt, while parse_end writes it

// ----------------------------------------------------------------------------
// parser start and end functions ---------------------------------------------
// ----------------------------------------------------------------------------

void parse_init(char *f_name, char *d_proc, char *d_macro, char *d_tmp, char *d_array)
{
    // pick up the arguments --------------------------------------------------

    // the .cmm, by its path
    cmm_src = f_name;
    yyin = fopen(cmm_src, "r");
    if (yyin  == NULL) fatal(MSG_ERR_CANT_OPEN_FILE, cmm_src);

    fatal_hook(out_remove);       // from here on, an error removes what this run wrote

    char *asm_file = xasprintf("%s/%s.asm", d_proc, prname); // output .asm file name
    f_asm = fopen(asm_file, "w"); // create the .asm file
    if (f_asm == NULL) fatal(MSG_ERR_CANT_OPEN_FILE, asm_file);
    out_track(asm_file, &f_asm);

    dir_macro = d_macro; // record the Macro directory
    dir_tmp   = d_tmp  ; // record the Tmp directory

    sim_arr = strcmp(d_array, "1") == 0;

    // reserve entry 0 of the symbol table. An operand whose id is 0 means "the
    // value is already in the accumulator" (expr_make(type, 0), oper.c), so no
    // real variable or constant may ever get id 0. #PRNAME's name used to sit
    // there; without it the first literal took it (the 32 of #NUBITS 32) and
    // s = s + 32 came out as ADD s. The name starts with __, which the user
    // can't write, so it never appears in an expression
    add_var("__acc");

    // create helper files ----------------------------------------------------

    char *path = xasprintf("%s/prog_log.txt", dir_tmp); f_log = fopen(path,"w"); // log with info for the assembler and gtkwave
    if (f_log == NULL) fatal(MSG_ERR_CANT_OPEN_FILE, path);
    out_track(path, &f_log);
    path = xasprintf("%s/pc_%s_mem.txt", dir_tmp, prname); f_lin = fopen(path,"w"); // memory in pc.v that bridges asm to cmm
    if (f_lin == NULL) fatal(MSG_ERR_CANT_OPEN_FILE, path);
    out_track(path, &f_lin);

    // Open the top-level stmt_list collector. Every directive, declaration
    // and function reduce appends a stmt_node here; nothing reaches f_asm
    // until prog_emit() runs the walker at `fim`.
    stmt_list_open();
}

// Fires from the `fim : prog` action once parsing is complete. The whole
// program is now an AST in the global stmt_list; this is where it gets
// flattened into assembly. NOP is emitted as a fixed prefix (the processor
// expects instruction address 0 to be a no-op) and then the top-level stmts
// are walked in source order, everything but the functions first: a global's
// initializer runs before main wherever the file declares it, as in C (one
// written after a function used to land after that function's RET, where
// nothing reached it). The second walk skips what the first one emitted.
void prog_emit(void)
{
    add_sinst(-1, "NOP\n");
    add_sinst( 0, "#PRNAME %s\n", prname);   // the processor's name: the source file's

    stmt_node *prog = stmt_list_close();
    for (int i = 0; i < prog->kids_n; i++)
        if (prog->kids[i]->kind != STMT_FUNC) stmt_emit(prog->kids[i]);
    // main right after the globals' initializers, where the reset falls
    // through to it (no JMP main); the other functions follow its @fim
    for (int i = 0; i < prog->kids_n; i++)
        if (prog->kids[i]->kind == STMT_FUNC && strcmp(v_table[prog->kids[i]->id].name, "main") == 0)
            stmt_emit(prog->kids[i]);
    stmt_emit(prog);
    stmt_free(prog);
}

// a variable asm_unread took out of the program counts as not used: check_var
// tells the user (a comp by its real half, which is its name in the table)
static void unread_gone(const char *name)
{
    int id = find_var((char *)name);
    if (id >= 0) v_table[id].used = 0;
}

// a variable the program marked with #WAVE stays, read or not
static int unread_keep(const char *name)
{
    int id = find_var((char *)name);
    return id >= 0 && v_table[id].wave;
}

void parse_end(char *d_proc)
{
    warn(MSG_INFO_INS_GENERATED, num_ins);

    // close the files --------------------------------------------------------

    fclose(f_asm); f_asm = NULL; // assembly code
    fclose(f_lin); f_lin = NULL; // assembly translator

    // check whether macros need to be appended to the .asm file --------------

    char *asm_file = xasprintf("%s/%s.asm", d_proc, prname);

	// the area the Dirac loops share, when a statement needed it ------------

	dirac_finish(asm_file); // (dirac.c)

	// a function that calls itself back would run over its own locals ------

	check_rec(); // (funcoes.c)

	mac_copy(asm_file);

	// code no path reaches leaves the program, and with it its hardware -----

	char *pc_file = xasprintf("%s/pc_%s_mem.txt", dir_tmp, prname);
	int  gone = asm_reach(asm_file, pc_file);
	if  (gone > 0) num_ins -= gone;

	// a division by a constant stays one only when the float divider is there anyway

	int  kept = asm_divk(asm_file, pc_file);
	if  (kept > 0) num_ins -= kept;

	// every division becomes <div> x; NOP...; <read>: the dividers take five / four cycles

	int  more = asm_divseq(asm_file, pc_file);
	if  (more > 0) num_ins += more;

	// a variable written and never read leaves the program; check_var says so

	int  unread = asm_unread(asm_file, pc_file, unread_gone, unread_keep);
	if  (unread > 0) num_ins -= unread;

	// an argument the called function never reads is not computed
	int  noarg = asm_callarg(asm_file, pc_file);
	if  (noarg > 0) num_ins -= noarg;

	// variables that are never alive at the same time share a data word ------

	asm_share(asm_file);
	free(asm_file); free(pc_file);

	// check consistency of all variables and functions -----------------------

	check_var(); // (variaveis.c)

    // finalize the cmm log file ----------------------------------------------

    fprintf(f_log, "num_ins %d\n", num_ins); // number of instructions (excluding final macros)
    char *src = dir_of(cmm_src);             // the folder of the source: asmcomp reads the user's
    if (src) fprintf(f_log, "srcdir %s\n", src); // files there (array inits, input_N.txt, cade_k.txt)
    fprintf(f_log, "lang C+-\n");            // the source language: the waveform's source track takes its name
    fclose (f_log); f_log = NULL;

    // generate the translation file for the cmm code -------------------------

    // (the source the parser read: before, <proc>/Software/<prname>.cmm, which
    // failed when the file was not named after the processor)
    char *path = xasprintf("%s/%s", dir_tmp, "src_lines.txt");

    f_src = fopen(path, "w");
    if (f_src == NULL) fatal(MSG_ERR_CANT_OPEN_FILE, path);
    out_track(path, &f_src);
    FILE *output = f_src;
    FILE *input  = fopen(cmm_src, "r");
    if (input  == NULL) fatal(MSG_ERR_CANT_OPEN_FILE, cmm_src);

    fputs("-1 INTERNAL\n"    , output); // code for the file start
    fputs("-3 END\n"         , output); // code for @fim JMP fim

    // one numbered line per source line, of any length, with its end of line
    // as read (a line of 1000 characters and more used to come out as two)
    int cnt = 1;
    char *texto;
    while ((texto = read_src_line(input)) != NULL)
    {
        substituir_braket(texto);
        fprintf(output, "%d %s", cnt++, texto);
        free(texto);
    }

    fclose(input );
    fclose(output); f_src = NULL;

    // compiled: what this run wrote stays
    fatal_hook(NULL);
    for (int i = 0; i < out_n; i++) free(out_path[i]);
    out_n = 0;
}

// ----------------------------------------------------------------------------
// functions for registering instructions -------------------------------------
// ----------------------------------------------------------------------------

// writes an instruction with one space between its words and none at either
// end, whatever padding its format string had. Only the file gets it: the
// peephole below still reads the text as it was formatted. A leading space
// cannot glue it to a label: add_sinst ends every label with one.
static void put_instr(const char *s)
{
    while (*s == ' ' || *s == '\t') s++;
    int gap = 0;
    for (; *s && *s != '\n'; s++)
    {
        if (*s == ' ' || *s == '\t') {gap = 1; continue;}
        if (gap) {fputc(' ', f_asm); gap = 0;}
        fputc(*s, f_asm);
    }
    fputc('\n', f_asm);
}

// the first word of s, after any blanks (NULL: none); the block is the caller's
static char *word_of(const char *s)
{
    while (*s == ' ' || *s == '\t') s++;
    size_t n = strcspn(s, " \t\r\n");
    return n ? xasprintf("%.*s", (int)n, s) : NULL;
}

// adds an instruction to the asm file
void add_instr(char *inst, ...)
{
    va_list  args;
    va_start(args , inst);

    char    *str = xvasprintf(inst, args);   // an opcode and a name or a literal, of any length
    va_end  (args);

    // ------------------------------------------------------------------------
    // an int literal converted at run time is just a float literal ------------
    // ------------------------------------------------------------------------
    // "I2F_M 3" -> "LOD 3.0" (and the P_ form): same word, no conversion, and a
    // program that converts nothing else no longer needs the I2F block. Only
    // below 2^NBMANT, where the conversion is exact at every #FROUND level.

    {
        int  p = (strncmp(str, "P_I2F_M ", 8) == 0) ? 8 : (strncmp(str, "I2F_M ", 6) == 0) ? 6 : 0;
        long n; char end[8];
        if (p && sscanf(str + p, "%ld%7s", &n, end) == 1 && labs(n) < mant_pow2())
        {
            free(str);
            str = xasprintf("%s %ld.0\n", p == 8 ? "P_LOD" : "LOD", n);
        }
    }

    // ------------------------------------------------------------------------
    // peephole: drop "LOD x" when the accumulator already holds x -------------
    // ------------------------------------------------------------------------
    // acc_names track the operands currently in the acc: a plain LOD / P_LOD /
    // SET of an operand leaves its value there (SET stores acc to memory and
    // leaves acc unchanged -- instr_dec.v opcode 4 -> ula_op 0); every other op
    // clobbers it. So a "LOD x" while x is among them is a no-op and is dropped.
    // This subsumes the old "LOD x after SET x" case. Only the plain LOD is
    // dropped -- P_LOD also pushes, so it must always run. acc_names are cleared
    // at every basic-block boundary (emit_peephole_reset), so the drop never
    // crosses a label or control-flow merge.

    if (strncmp(str, "LOD ", 4) == 0)
    {
        char *lod_var = word_of(str + 4);
        int   held    = lod_var && acc_holds(lod_var);
        free(lod_var);
        if (held) {free(str); return;}   // acc already holds it -- acc_names / last_str stay valid
    }

    // an operation by its identity leaves the acc as it is: "ADD 0" (a matrix
    // index whose column is the constant 0), "MLT 1", and in float "F_ADD 0.0",
    // "F_SU1 0.0" (acc - 0) and "F_MLT 1.0" (the zero or unit part of a comp
    // constant): nothing to emit, and acc_names stay
    {
        char op[8], arg[8], more[2];
        if (sscanf(str, "%7s %7s %1s", op, arg, more) == 2 &&
            ((strcmp(op, "ADD")   == 0 && strcmp(arg, "0")   == 0) ||
             (strcmp(op, "MLT")   == 0 && strcmp(arg, "1")   == 0) ||
             (strcmp(op, "F_ADD") == 0 && strcmp(arg, "0.0") == 0) ||
             (strcmp(op, "F_SU1") == 0 && strcmp(arg, "0.0") == 0) ||
             (strcmp(op, "F_MLT") == 0 && strcmp(arg, "1.0") == 0)))
            {free(str); return;}
    }

    // ------------------------------------------------------------------------
    // append the instruction -------------------------------------------------
    // ------------------------------------------------------------------------

    put_instr(str);

    // table for the gtkwave assembly translator ------------------------------

    num_ins++;
    char *bits = itob(emit_line,20);
    fprintf(f_lin, "%s\n", bits);
    free(bits);

    // ------------------------------------------------------------------------
    // check whether the instruction needs a special macro --------------------
    // ------------------------------------------------------------------------

    if (find_opc("__float_sqrt", str)) mac_add("fsqrt");
    if (find_opc("__float_atan", str)) mac_add("fatan");
    if (find_opc("__float_sin" , str)) mac_add("fsin" );
    if (find_opc("__float_tan" , str)) mac_add("ftan" );
    if (find_opc("__float_exp" , str)) mac_add("fexp" );
    if (find_opc("__float_log" , str)) mac_add("flog" );

    // track what is now in the accumulator: a plain LOD / P_LOD / SET of an
    // operand leaves that operand's value there, OUT, PSH, TOM, JIZ (only
    // read it: isa.tsv; a JIZ's target has a label, which clears all this)
    // and SET_V leave the acc as it is, anything else clobbers it. A SET keeps
    // the names before it too: after LOD y; SET y_n1 the acc is both
    char *nm = strncmp(str, "SET ",   4) == 0 ? word_of(str + 4)
             : strncmp(str, "LOD ",   4) == 0 ? word_of(str + 4)
             : strncmp(str, "P_LOD ", 6) == 0 ? word_of(str + 6) : NULL;
    if      (nm && strncmp(str, "SET ", 4) == 0)
        acc_add(nm);
    else if (nm)
        acc_only(nm);
    else if (strncmp(str, "OUT ", 4) == 0 || strcmp(str, "PSH\n") == 0
          || strncmp(str, "TOM ", 4) == 0 || strncmp(str, "JIZ ", 4) == 0)
        ;                                // they read the acc and leave it as it is
    else if (strncmp(str, "SET_V ", 6) == 0)
        acc_drop_keys();                 // an element store: as a SET, the keys go
    else
        acc_clear();
    free(nm);

    // remember this emit for the next peephole check
    free(last_str);
    last_str = str;
}

// adds special instructions
// type =  0 -> comment
// type = -1 -> INTERNAL
// type = -3 -> END
void add_sinst(int type, char *inst, ...)
{
    // Labels, directives and inline comments are control-flow / scaffolding
    // boundaries: a label may be a jump target, so dropping a LOD that
    // follows it would be unsafe. Reset the peephole window before emitting.
    // An array's declaration (#array, #arrays, #arrayd) is data, no code and
    // no entry: the acc goes through it (a local array declared after the
    // first statement of a function, ArcTan's arctan_LUT)
    if (strncmp(inst, "#array", 6) != 0) emit_peephole_reset();

    va_list  args;
    va_start(args , inst);

    // straight to the file: a #arrayd line carries a whole brace list, of
    // any length (a fixed buffer here overflowed past about 25 values)
    vfprintf(f_asm, inst, args);
    va_end  (args);

    // table for the gtkwave assembly translator ------------------------------

    if (type != 0)
    {
        num_ins++;
        char *bits = itob(type,20);
        fprintf(f_lin, "%s\n", bits);
        free(bits);
    }
}
