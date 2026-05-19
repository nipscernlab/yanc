// preprocessor smoke: #include, #define, #ifdef
#pragma yanc prname tst3
#pragma yanc nubits 16

#include "inc_math.h"

#define HAVE_TRIPLE 1
#define FACTOR 10

int triple(int x) {
    return x + x + x;
}

void main(void) {
    int v = SCALE * FACTOR;     // 4 * 10 = 40 (both macros expanded)
    out(0, v);                  // expect 40

#ifdef HAVE_TRIPLE
    out(0, triple(7));          // expect 21
#else
    out(0, 0);
#endif

#ifndef NOT_DEFINED
    out(0, 99);                 // expect 99
#else
    out(0, 0);
#endif
}
