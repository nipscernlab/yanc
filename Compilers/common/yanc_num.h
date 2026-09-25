// ****************************************************************************
// yanc_num -- exact decimal -> SAPHO float encoder (TODO.md item 5) ----------
// ****************************************************************************
//
// The SAPHO float is value = m * 2^e: m an explicit NBMANT-bit magnitude with
// its top bit set (no hidden bit), e a two's-complement NBEXPO-bit exponent
// (no bias), sign-magnitude; the word is {s, e, m}, sign on top. Zero is the
// word with only the exponent's sign bit set: {0, 100..0, 0..0}.
//
// yn_encode() turns decimal text (`-12.5e-3`, `.5`, `4194304.0`) into that
// word EXACTLY: the text is read as a ratio of big integers, divided bit by
// bit, and rounded to nearest, ties to even, on the remainder -- no host
// float or double on the way, so NBMANT above 23 gains real bits and nothing
// is rounded twice. A value below the smallest normal number is encoded with
// the exponent held at its minimum and the mantissa shifted (rounded once,
// from the exact value); at #FROUND >= 1 it is flushed to zero instead,
// because the ALU there assumes normalised operands.
// ****************************************************************************

#ifndef YANC_NUM_H
#define YANC_NUM_H

#define YN_MAXBITS 128          // widest word yn_encode can write

typedef struct {
    int  ok;                    // 0: the text is not a decimal number
    int  overflow;              // too large for the format (word left at zero)
    int  underflow;             // nonzero value that encoded as zero
    int  denormal;              // below the smallest normal, kept shifted
    int  flushed;               // below the smallest normal, flushed (#FROUND >= 1)
    int  inexact;               // rounding happened
    int  s;                     // sign
    long long e;                // exponent (as encoded)
    unsigned long long m;       // mantissa (as encoded; NBMANT <= 63)
    char bits[YN_MAXBITS + 1];  // the word, MSB first, NUBITS characters
    double delta;               // encoded - exact, approximate (diagnostics only)
} yn_float;

// nubits must equal nbmant + nbexpo + 1; nbmant in 2..63, nbexpo in 2..62.
// Returns r.ok.
int yn_encode(const char *text, int nbmant, int nbexpo, int fround, yn_float *r);

#endif
