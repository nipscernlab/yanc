#pragma yanc prname test39
// First multi-file test (folder layout): exercises the user's blind::dsp
// kernels in dsp_kernels.cpp end-to-end (cpppp amalgamation -> cppcomp ->
// appcomp -> asmcomp -> simulation). Integer-only outputs to avoid float
// rounding in the golden.
#include "dsp_kernels.cpp"

void main(void) {
    // 1) convolve_full: out_len = 4 + 3 - 1 = 6
    //    a = {1,2,3,4}, b = {1,0,-1} -> c = {1, 2, 2, 2, -3, -4}
    float a[4] = {1.0f, 2.0f, 3.0f, 4.0f};
    float b[3] = {1.0f, 0.0f, -1.0f};
    float c[6];
    blind::dsp::convolve_full(a, 4, b, 3, c);
    for (int i = 0; i < 6; i = i + 1) out(0, (int)c[i]);

    // 2) argmax / argmax_abs
    out(0, blind::dsp::argmax    (a, 4));     // 3 (value 4)
    out(0, blind::dsp::argmax_abs(b, 3));     // 0 (first |1|)

    // 3) dot
    float p[3] = {1.0f, 2.0f, 3.0f};
    float q[3] = {1.0f, 0.0f, -1.0f};
    out(0, (int)blind::dsp::dot(p, q, 3));    // 1 - 3 = -2

    // 4) fill + axpy: r <- 5.0; r += 2.0*a -> {7,9,11,13}
    float r[4];
    blind::dsp::fill(r, 4, 5.0f);
    blind::dsp::axpy(r, a, 4, 2.0f);
    for (int i = 0; i < 4; i = i + 1) out(0, (int)r[i]);

    // 5) l2_norm: ||{3,4}||_2 = 5 exactly (no irrationals -> bit-clean)
    float v[2] = {3.0f, 4.0f};
    out(0, (int)blind::dsp::l2_norm(v, 2));   // 5

    // 6) median_inplace: median of {5,1,3,2,4} = 3
    float med[5] = {5.0f, 1.0f, 3.0f, 2.0f, 4.0f};
    out(0, (int)blind::dsp::median_inplace(med, 5));   // 3
}
