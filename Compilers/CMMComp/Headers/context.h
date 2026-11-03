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

#endif // CMM_CONTEXT_H
