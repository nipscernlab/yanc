// ----------------------------------------------------------------------------
// cmmcomp's state, in three structs (step P8 of #93) -------------------------
// ----------------------------------------------------------------------------
// What used to be some twenty globals, grouped by who sets them: the target the
// directives describe, the state of the parse, and the state of the emission.
// They are global instances for now; the next step passes them by pointer,
// which is what lets a unit test build a context of its own.
// ----------------------------------------------------------------------------

#ifndef CMM_CONTEXT_H
#define CMM_CONTEXT_H

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

#endif // CMM_CONTEXT_H
