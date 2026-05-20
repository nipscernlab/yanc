// Multi-file DSP example: a 4-tap moving-average FIR filter split across
// fir.h (interface) and fir.c (implementation), driven from here. Exercises
// #include of a header AND a source unit, #pragma once, float struct with
// float arrays, pointer-to-struct, and a circular buffer.
#pragma yanc prname tst43
// (no width pragma -> default 32-bit / IEEE-754 single)

#include "fir.h"
#include "fir.c"

void main(void) {
    struct fir f;
    float coefs[FIR_N] = { 0.25, 0.25, 0.25, 0.25 };   // 4-tap average
    fir_init(&f, coefs);

    // step input of 4.0 -> output ramps 1,2,3,4 then holds (x100)
    out(0, (int)(fir_step(&f, 4.0) * 100.0));   // 100
    out(0, (int)(fir_step(&f, 4.0) * 100.0));   // 200
    out(0, (int)(fir_step(&f, 4.0) * 100.0));   // 300
    out(0, (int)(fir_step(&f, 4.0) * 100.0));   // 400
    out(0, (int)(fir_step(&f, 4.0) * 100.0));   // 400 (steady state)
    out(0, (int)(fir_step(&f, 0.0) * 100.0));   // 300 (one zero enters)
}
