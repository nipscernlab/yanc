// TEST50 constants.cpp -- replaces the user's hosted constants.cpp because
// CPPComp does not yet support C++14 lambda-IIFE (`= []() { ... }()`).
// The numeric values are IDENTICAL to the user's hosted file; only the
// lambda-IIFE for TRIANGLE_INIT is unrolled into a direct array literal.
// LAMBDA_MULTIPLIERS keeps all 7 entries (test50 uses the FULL grid for
// numerical comparison against the reference IEEE-754 host build).

#include "blind_deconv.hpp"

namespace blind {

// Lambda multipliers from the user's constants.cpp (verbatim values, just
// with the double-braces required by std::array aggregate init in CPPComp).
const std::array<float, LAMBDA_GRID_LEN> LAMBDA_MULTIPLIERS = {{
    0.5f, 1.0f, 1.5f, 2.0f, 2.5f, 3.0f, 4.0f
}};

// TRIANGLE_INIT = [0.25, 0.5, 1.0, 0.5, 0.25] placed so the peak (1.0)
// lands at index C_PEAK=5 in a 15-tap zero-padded array. The user's source
// builds this via a lambda-IIFE; we inline it as a direct literal.
const std::array<float, M_PULSE> TRIANGLE_INIT = {{
    0.0f, 0.0f, 0.0f, 0.25f, 0.5f, 1.0f, 0.5f, 0.25f,
    0.0f, 0.0f, 0.0f,  0.0f, 0.0f, 0.0f, 0.0f
}};

} // namespace blind
