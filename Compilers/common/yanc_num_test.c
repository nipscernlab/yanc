// yanc_num_test: reads lines "nbmant nbexpo fround text" and prints, for each,
// "<bits> <flags>" (flags: o overflow, u underflow, d denormal, f flushed,
// i inexact, - none) or "ERR" when the text is not a number. Driven by
// Scripts/check_yanc_num.py, which compares against an exact Python model.
#include <stdio.h>
#include <string.h>
#include "yanc_num.h"

int main(void)
{
    char line[512], text[400];
    int nbmant, nbexpo, fround;
    while (fgets(line, sizeof line, stdin)) {
        if (sscanf(line, "%d %d %d %399s", &nbmant, &nbexpo, &fround, text) != 4) continue;
        yn_float r;
        if (!yn_encode(text, nbmant, nbexpo, fround, &r)) { puts("ERR"); continue; }
        char f[8]; int n = 0;
        if (r.overflow)  f[n++] = 'o';
        if (r.underflow) f[n++] = 'u';
        if (r.denormal)  f[n++] = 'd';
        if (r.flushed)   f[n++] = 'f';
        if (r.inexact)   f[n++] = 'i';
        if (!n) f[n++] = '-';
        f[n] = 0;
        printf("%s %s\n", r.bits, f);
    }
    return 0;
}
