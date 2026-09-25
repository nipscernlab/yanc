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
#include "../Headers/messages.h"
#include "../../common/yanc_num.h"

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

// encodes decimal text as a "my float" word: Compilers/common/yanc_num.c
// reads it exactly (big integers, no host float) and rounds to nearest, ties
// to even; below the smallest normal the exponent is held at its minimum and
// the mantissa shifted, flushed to zero at #FROUND >= 1 (the ALU there
// assumes normalised operands). This used to go through atof: a 24-bit host
// float first, then a half-up rounding of one more bit -- rounded twice, and
// one unit high in the last bit for about 1 constant in 6 (pi/2 at 23 bits).
// *delta (if given) = encoded - exact, for the precision warnings.
unsigned int f2mf(char *va, float *delta)
{
    yn_float r;
    if (!yn_encode(va, nbmant, nbexpo, fround, &r)) {
        fprintf(stderr, MSG_ERR_BAD_FLOAT, va);
        exit(EXIT_FAILURE);
    }
    if (r.overflow) {
        fprintf(stderr, MSG_ERR_FLOAT_OVERFLOW, va, nbmant, nbexpo);
        exit(EXIT_FAILURE);
    }
    if (delta) *delta = (float)r.delta;
    return (unsigned int)strtoul(r.bits, NULL, 2);
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
