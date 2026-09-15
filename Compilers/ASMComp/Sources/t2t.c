// ----------------------------------------------------------------------------
// data-type conversion routines ----------------------------------------------
// ----------------------------------------------------------------------------

// global includes
#include <string.h>
#include <stdlib.h>
#include  <stdio.h>
#include   <math.h>

// local includes
#include "../Headers/eval.h"

// converts the integer x to a binary string of length w
// could be revised to support ints wider than 32 bits
// one way would be to switch x to an ascii representation as well
char *itob(int x, int w)
{
	int z;
    char *b = (char *) malloc(w+1);
    b[0] = '\0';

	int s = (w > 31) ? 31 : w;
	if (w > 31)
    {
        for (z = 0; z < w- 31; z++)
            if (x < 0) strcat(b,"1");
            else       strcat(b,"0");
    }

    for (z = pow(2,s-1); z > 0; z >>= 1)
		strcat(b, ((x & z) == z) ? "1" : "0");

    return b;
}

// converts an IEEE-754 32-bit float to "my float"
// could be revised to convert 64-bit floats too
unsigned int f2mf(char *va, float *delta)
{
    float f = atof(va);

    if (f == 0.0) return 1 << (nbmant + nbexpo -1);

    int *ifl = (int*)&f;

    // unpack standard IEEE ---------------------------------------------------

    int s =  (*ifl >> 31) & 0x00000001;
    int e = ((*ifl >> 23) & 0xFF) - 127 - 22;
    int m = ((*ifl & 0x007FFFFF) + 0x00800000) >> 1;

    // sign -------------------------------------------------------------------

    s = s << (nbmant + nbexpo);

    // exponent ---------------------------------------------------------------

    e = e + (23-nbmant);

    int sh = 0;
    while (e < -pow(2, nbexpo-1))
    {
        e   = e+1;
        sh = sh+1;
    }

    // mantissa ---------------------------------------------------------------
    // one right shift takes the 23-bit mantissa down to nbmant bits plus `sh`
    // more for a value below the smallest normal number (a denormal). The
    // shift used to be skipped for nbmant == 23, so a tiny constant kept its
    // full mantissa at the clamped exponent (1e-33 was encoded as 1.6e-32);
    // and a shift of 32+ bits is undefined in C (garbage instead of 0).

    int sht = (23-nbmant) + sh;
    if (sht == 0)
    {
        if (*ifl & 0x00000001) m = m+1; // round on the IEEE bit dropped above
    }
    else if (sht > 24)
    {
        m = 0;                          // below half the smallest denormal
    }
    else
    {
        int carry = (m >> (sht-1)) & 0x00000001; // rounding carry
        m = m >> sht;
        if (carry) m = m+1; // round
    }

    // renormalize a rounding carry out of the nbmant-bit mantissa -------------
    // For the largest value just below a power of two the mantissa is all ones
    // (0x7FFFFF) and rounding pushes it to 0x800000, i.e. one bit past the
    // field. Left as-is, that bit bleeds into the exponent in `s + e + m` below
    // and zeroes the mantissa, encoding e.g. 2 - 2^-23 as 0.0. Shift it back
    // and bump the exponent instead.
    if (m >> nbmant) { m = m >> 1; e = e + 1; }

    // residual ---------------------------------------------------------------

    float num = (atof(va)<0.0) ? -atof(va) : atof(va); // absolute value of the number

    // underflow ----------------------------------------------------------------
    // nothing left, or a denormal at #FROUND >= 1: encode the canonical zero.
    // At those levels the ALU flushes every result below the smallest normal
    // number to zero and its F_MLT/F_DIV assume normalised operands, so a
    // denormal constant must not reach it.

    if (m == 0 || (fround >= 1 && m < (1 << (nbmant-1))))
    {
        if (delta) *delta = -num;
        return 1 << (nbmant + nbexpo -1);
    }

    *delta = m*pow(2,e)-num;

    // assemble ---------------------------------------------------------------

    e = e & ((int)(pow(2,nbexpo)-1));
    e = e << nbmant;

    return s + e + m;
}

// converts "my float" (as ascii) back to float
float mf2f(char *ifl)
{
    // sign -------------------------------------------------------------------

    int s = ifl[0] == '1';

    // exponent ---------------------------------------------------------------

    char exb[64]; for (int i=0;i<nbexpo;i++) exb[i] = ifl[i+1]; exb[nbexpo]=0;

    int es = exb[0] == '1';
    if (es) for (int i=0;i<nbexpo;i++) exb[i] = (exb[i] == '1') ? '0' : '1';

    char *endp;
    int e = strtol(exb,&endp,2);
    if (es) e = -(e+1);

    // mantissa ---------------------------------------------------------------

    char mab[64]; for (int i=0;i<nbmant;i++) mab[i] = ifl[nbexpo+1+i]; mab[nbmant]=0;

    int  m = strtol(mab,&endp,2);

    // build the float --------------------------------------------------------

    float  f = m * pow(2,e);
    if (s) f = -f;

    return f;
}
