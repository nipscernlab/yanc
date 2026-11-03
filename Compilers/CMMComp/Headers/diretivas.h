// ----------------------------------------------------------------------------
// gathers functions and variables driven by directives -----------------------
// ----------------------------------------------------------------------------

// state the cmm compiler keeps from the directives

#ifndef CMM_DIRETIVAS_H
#define CMM_DIRETIVAS_H

extern const char *prname; // processor name
extern int  nubits;      // word width (bits)
extern int  nbmant;      // mantissa width (bits)
long long mant_pow2(void); // 2^NBMANT, defined for any #NBMANT
extern int  nbexpo;      // exponent width (bits)
extern int  nuioin;      // number of input ports
extern int  nuioou;      // number of output ports
extern int  fround;      // float rounding level (#FROUND)
extern int  fftsiz;      // low bits of an index v[k) reverses (#FFTSIZ)

// directive parsing
// what a configuration directive sets in cmmcomp (DIR_ASM_ONLY: nothing,
// only asmcomp reads it: #NDSTAC, #SDEPTH)
typedef enum {
    DIR_ASM_ONLY, DIR_NBMANT, DIR_NBEXPO, DIR_NUIOIN, DIR_NUIOOU,
    DIR_FROUND, DIR_NUGAIN, DIR_NUBITS, DIR_FFTSIZ
} dir_code;

void dire_exec (char *dir, int id, dir_code t);

#endif // CMM_DIRETIVAS_H
