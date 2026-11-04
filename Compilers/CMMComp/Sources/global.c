// ----------------------------------------------------------------------------
// global functions and variables of the compiler -----------------------------
// ----------------------------------------------------------------------------

// global includes
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include  <ctype.h>

// local includes
#include "../Headers/convert.h"
#include "../Headers/macros.h"
#include "../Headers/global.h"
#include "../Headers/directives.h"
#include "../Headers/symbols.h"
#include "../Headers/messages.h"
#include "../Headers/dirac.h"
#include "../Headers/functions.h"
#include "../Headers/diag.h"
#include "../../common/asm_share.h"
#include "../../common/exe_dir.h"
#include "../../common/xstr.h"

// ----------------------------------------------------------------------------
// global variable definitions ------------------------------------------------
// ----------------------------------------------------------------------------

const char *dir_macro = ""; // Macros directory (main's arguments: they live to the end)
const char *dir_tmp   = ""; // Temp directory
static const char *cmm_src = ""; // the .cmm the parser reads (re-read for src_lines.txt)

parse_ctx pctx = {.fname = ""};   // the state of the parse (context.h)
emit_ctx  ectx = {.emit_line = 1, .emit_fid = -1, .emit_fname = ""};   // the state of the emission

// state variables
int  sim_arr  = 0;    // tells whether the array is simulated or not


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
static void replace_brackets(char *str)
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
// the file); the block is the caller's. Byte by byte: a NUL in the line, which
// fgets + strlen took for its end, cut the line and shifted every later one
// (#108 A19); it is kept as a space, since the table is text
static char *read_src_line(FILE *f)
{
    char  *line = NULL;
    size_t len  = 0, cap = 0;
    int    c;
    while ((c = getc(f)) != EOF)
    {
        if (len + 2 > cap) {cap = cap ? 2 * cap : 256; line = xrealloc(line, cap);}
        line[len++] = c ? (char)c : ' ';
        if (c == '\n') break;
    }
    if (line) line[len] = '\0';
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

void parse_init(char *f_name, char *d_proc, char *d_macro, char *d_tmp, const char *d_array)
{
    // pick up the arguments --------------------------------------------------

    // the .cmm, by its path
    cmm_src = f_name;
    yyin = fopen(cmm_src, "r");
    if (yyin  == NULL) fatal(MSG_ERR_CANT_OPEN_FILE, cmm_src);

    fatal_hook(out_remove);       // from here on, an error removes what this run wrote
    xstr_oom_hook(out_remove);    // and so does running out of memory

    char *asm_file = xasprintf("%s/%s.asm", d_proc, cfg.prname); // output .asm file name
    ectx.f_asm = fopen(asm_file, "w"); // create the .asm file
    if (ectx.f_asm == NULL) fatal(MSG_ERR_CANT_OPEN_FILE, asm_file);
    out_track(asm_file, &ectx.f_asm);

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

    char *path = xasprintf("%s/prog_log.txt", dir_tmp); ectx.f_log = fopen(path,"w"); // log with info for the assembler and gtkwave
    if (ectx.f_log == NULL) fatal(MSG_ERR_CANT_OPEN_FILE, path);
    out_track(path, &ectx.f_log);
    path = xasprintf("%s/pc_%s_mem.txt", dir_tmp, cfg.prname); ectx.f_lin = fopen(path,"w"); // each instruction's source line, for wave_map.py and the waveform
    if (ectx.f_lin == NULL) fatal(MSG_ERR_CANT_OPEN_FILE, path);
    out_track(path, &ectx.f_lin);

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
    add_sinst( 0, "#PRNAME %s\n", cfg.prname);   // the processor's name: the source file's

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
    warn(MSG_INFO_INS_GENERATED, ectx.num_ins);

    // close the files --------------------------------------------------------

    fclose(ectx.f_asm); ectx.f_asm = NULL; // assembly code
    fclose(ectx.f_lin); ectx.f_lin = NULL; // assembly translator

    // check whether macros need to be appended to the .asm file --------------

    char *asm_file = xasprintf("%s/%s.asm", d_proc, cfg.prname);

	// the area the Dirac loops share, when a statement needed it ------------

	dirac_finish(asm_file); // (dirac.c)

	// a function that calls itself back would run over its own locals ------

	check_rec(); // (functions.c)

	mac_copy(asm_file);

	// code no path reaches leaves the program, and with it its hardware -----

	char *pc_file = xasprintf("%s/pc_%s_mem.txt", dir_tmp, cfg.prname);

	// two words the ISA writes as one (#38)

	int  fused = asm_fuse(asm_file, pc_file);
	if  (fused > 0) ectx.num_ins -= fused;

	int  gone = asm_reach(asm_file, pc_file);
	if  (gone > 0) ectx.num_ins -= gone;

	// a division by a constant stays one only when the float divider is there anyway

	int  kept = asm_divk(asm_file, pc_file);
	if  (kept > 0) ectx.num_ins -= kept;

	// every division becomes <div> x; NOP...; <read>: the dividers take five / four cycles

	int  more = asm_divseq(asm_file, pc_file);
	if  (more > 0) ectx.num_ins += more;

	// a variable written and never read leaves the program; check_var says so

	int  unread = asm_unread(asm_file, pc_file, unread_gone, unread_keep);
	if  (unread > 0) ectx.num_ins -= unread;

	// an argument the called function never reads is not computed
	int  noarg = asm_callarg(asm_file, pc_file);
	if  (noarg > 0) ectx.num_ins -= noarg;

	// variables that are never alive at the same time share a data word ------

	asm_share(asm_file);
	free(asm_file); free(pc_file);

	// check consistency of all variables and functions -----------------------

	check_var(); // (symbols.c)

    // finalize the cmm log file ----------------------------------------------

    fprintf(ectx.f_log, "num_ins %d\n", ectx.num_ins); // number of instructions (excluding final macros)
    char *src = dir_of(cmm_src);             // the folder of the source: asmcomp reads the user's
    if (src) fprintf(ectx.f_log, "srcdir %s\n", src); // files there (array inits, input_N.txt, cade_k.txt)
    fprintf(ectx.f_log, "lang C+-\n");            // the source language: the waveform's source track takes its name
    fclose (ectx.f_log); ectx.f_log = NULL;

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
        replace_brackets(texto);
        fprintf(output, "%d %s", cnt++, texto);
        free(texto);
    }

    fclose(input );
    fclose(output); f_src = NULL;

    // compiled: what this run wrote stays
    fatal_hook(NULL);
    xstr_oom_hook(NULL);
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
        if (gap) {fputc(' ', ectx.f_asm); gap = 0;}
        fputc(*s, ectx.f_asm);
    }
    fputc('\n', ectx.f_asm);
}

// adds an instruction to the asm file
void add_instr(const char *inst, ...)
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
    // leaves acc unchanged; acc.c says which instructions keep it); every other op
    // clobbers it. So a "LOD x" while x is among them is a no-op and is dropped.
    // This subsumes the old "LOD x after SET x" case. Only the plain LOD is
    // dropped -- P_LOD also pushes, so it must always run. acc_names are cleared
    // at every basic-block boundary (emit_peephole_reset), so the drop never
    // crosses a label or control-flow merge.

    if (acc_has_load(str)) {free(str); return;}   // the acc holds it already: what acc.c knows stays

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

    ectx.num_ins++;
    char *bits = int_to_bin(ectx.emit_line,20);
    fprintf(ectx.f_lin, "%s\n", bits);
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

    // what the acc holds after it (acc.c), which takes str
    acc_after(str);
}

// adds special instructions
// type =  0 -> comment
// type = -1 -> INTERNAL
// type = -3 -> END
void add_sinst(int type, const char *inst, ...)
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
    vfprintf(ectx.f_asm, inst, args);
    va_end  (args);

    // table for the gtkwave assembly translator ------------------------------

    if (type != 0)
    {
        ectx.num_ins++;
        char *bits = int_to_bin(type,20);
        fprintf(ectx.f_lin, "%s\n", bits);
        free(bits);
    }
}
