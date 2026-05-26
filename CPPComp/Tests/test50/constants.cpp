// TEST50 constants.cpp -- replaces the user's hosted constants.cpp because
// CPPComp does not yet support C++14 lambda-IIFE (`= []() { ... }()`).
// The numeric values are IDENTICAL to the user's hosted file; only the
// lambda-IIFE for TRIANGLE_INIT is unrolled into a direct array literal.
// LAMBDA_MULTIPLIERS holds LAMBDA_GRID_LEN entries (was 7 for the host
// validation; now 1 to keep YANC sim <20s in regress).

#include "blind_deconv.hpp"

namespace blind {

// First entry of the user's grid is enough to smoke-test the path.
const std::array<float, LAMBDA_GRID_LEN> LAMBDA_MULTIPLIERS = {{ 0.5f }};

// TRIANGLE_INIT = [0.25, 0.5, 1.0, 0.5, 0.25] placed so the peak (1.0)
// lands at index C_PEAK=5 in a 15-tap zero-padded array. The user's source
// builds this via a lambda-IIFE; we inline it as a direct literal.
const std::array<float, M_PULSE> TRIANGLE_INIT = {{
    0.0f, 0.0f, 0.0f, 0.25f, 0.5f, 1.0f, 0.5f, 0.25f,
    0.0f, 0.0f, 0.0f,  0.0f, 0.0f, 0.0f, 0.0f
}};

} // namespace blind
