#pragma yanc prname test62
// Regression for the std::array<float,N> zero-fill bug: a value-initialised
// (`= {}`) or partially brace-initialised float aggregate must zero-fill its
// unwritten slots with the YANC float encoding of 0.0 (f2mf(0.0)), NOT all-zero
// bits. An all-zero "zero" reads ~0 through the multiplier but is mis-decoded by
// the float ADDER, so it silently corrupts any later float summation (it slipped
// past test61, which only multiplies the zero slots). cppcomp emitted struct
// #arrays with type code 1 (int), so asmcomp filled std::array<float> with
// int-0; agg_fill_code() now emits code 2 for homogeneous-float aggregates.
#include <array>

void main(void) {
    // (1) value-init = {} then one explicit element; unwritten slots = 0.0f.
    std::array<float, 5> a = {};
    a[2] = 1.0f;
    float sa = 0.0f;
    for (int k = 0; k < 5; k = k + 1) sa = sa + a[k];
    out(0, (int)(sa * 1000.0f));            // 1000  (bug: != 1000)

    // (2) partial brace-init; trailing slots default to 0.0f.
    std::array<float, 4> b = {{ 2.5f }};
    float sb = 0.0f;
    for (int k = 0; k < 4; k = k + 1) sb = sb + b[k];
    out(0, (int)(sb * 1000.0f));            // 2500  (bug: != 2500)

    // (3) the raw bits of a default-zero slot are the float-0.0 encoding.
    union { float f; int i; } z; z.f = a[0];
    out(0, z.i);                            // f2mf(0.0) under 32/23/8 = 1073741824
}
