// ----------------------------------------------------------------------------
// cmmcomp's state, in three structs (step P8 of #93) -------------------------
// ----------------------------------------------------------------------------
// What used to be some twenty globals, grouped by who sets them: the target the
// directives describe, the state of the parse, and the state of the emission.
// They are global instances: a unit test sets their fields before it calls a
// helper (UnitTest/cmm_unit_test.c). Passing them by pointer was planned
// with #93 P8 and not done.
// ----------------------------------------------------------------------------

#ifndef CMM_CONTEXT_H
#define CMM_CONTEXT_H

#include <stdio.h>

// the processor the program is compiled for: the directives set it
typedef struct {
    const char *prname;   // processor name: the source file's (main sets it)
    int nubits;           // word width (bits): an int literal in hex, octal or binary is a bit pattern of this word
    int nbmant;           // mantissa width (bits)
    int nbexpo;           // exponent width (bits)
    int nuioin;           // number of input ports
    int nuioou;           // number of output ports
    int fround;           // float rounding level (#FROUND)
    int fftsiz;           // low bits of an index v[k) reverses (#FFTSIZ; asmcomp's default too)
} target_cfg;

extern target_cfg cfg;

// the state of the parse: where the lexer is and what it is inside of
typedef struct {
    int         line_num;   // line the lexer is reading (the messages say line_num + 1)
    const char *fname;      // name of the function being parsed (a v_table name, or "")
    int         fun_parse;  // function being walked (set by the STMT_FUNC walker; declare_ret reads it)
    int         type_tmp;   // the type of the declaration being read (CMMComp.l)
    int         mainok;     // 1 once a function named main is declared (check_var refuses a program without it)
    int         ret_ok;     // set by the STMT_RETURN walker when a `return` fires
} parse_ctx;

extern parse_ctx pctx;

// the state of the emission: the files written and what the walker is in
typedef struct {
    FILE       *f_asm;      // the .asm
    FILE       *f_log;      // prog_log.txt, the project's information for asmcomp and the waveform
    FILE       *f_lin;      // pc_<name>_mem.txt, the source line of each instruction
    int         emit_line;  // line of the AST node the current instruction comes from (add_instr)
    int         emit_fid;   // id of the function whose body the walker is in (-1: none)
    const char *emit_fname; // its name, for strip_fn_prefix in messages ("" outside a function)
    int         num_ins;    // instructions emitted (the final macros not counted)
    int         assign_idx_reload; // an index variable assign_array may read again (0: none; ast.c)
    int         fnum_quiet; // 1: lex_fnum says nothing of an approximation (a constant the compiler made)
} emit_ctx;

extern emit_ctx ectx;

#endif // CMM_CONTEXT_H
