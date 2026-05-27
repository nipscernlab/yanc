// TEST48 LOCAL constants.cpp -- replaces the user's hosted constants.cpp.
// The user file defines LAMBDA_MULTIPLIERS (7 floats) and uses a C++14
// lambda-IIFE (`= []() { ... }()`) to build TRIANGLE_INIT. CPPComp doesn't
// yet have lambda IIFE support, so this copy:
//   - shrinks LAMBDA_MULTIPLIERS to match the reduced LAMBDA_GRID_LEN=2 in
//     this test48/blind_deconv.hpp
//   - replaces the lambda IIFE for TRIANGLE_INIT with an inline array
//     literal that gives the same shape (the triangle [0.25,0.5,1,0.5,0.25]
//     centered at C_PEAK=5 within a M_PULSE=15 zero-padded array).
// The other 6 user files in this directory remain unmodified.

#include "blind_deconv.hpp"

namespace blind {

// Note: std::array<T,N> is a class with an inner T[N] field, so its aggregate
// init in CPPComp needs double braces (no brace elision support yet).
const std::array<float, LAMBDA_GRID_LEN> LAMBDA_MULTIPLIERS = {{
    1.0f, 2.0f
}};

// Direct array literal -- pre-computed triangle centered at C_PEAK=5:
// indices 3,4,5,6,7 get 0.25, 0.5, 1.0, 0.5, 0.25; everything else is 0.
const std::array<float, M_PULSE> TRIANGLE_INIT = {{
    0.0f, 0.0f, 0.0f, 0.25f, 0.5f, 1.0f, 0.5f, 0.25f,
    0.0f, 0.0f, 0.0f,  0.0f, 0.0f, 0.0f, 0.0f
}};

} // namespace blind
