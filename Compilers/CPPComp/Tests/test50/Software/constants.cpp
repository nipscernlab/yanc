// TEST50 constants.cpp -- the user's hosted constants.cpp with ONE remaining
// shim: LAMBDA_MULTIPLIERS uses double braces (CPPComp lacks brace elision for
// std::array single-brace init) and holds 1 entry instead of 7 (was 7 for the
// host validation; 1 keeps the YANC sim <20s in regress -- LAMBDA_GRID_LEN in
// blind_deconv.hpp matches). TRIANGLE_INIT is the user's lambda-IIFE VERBATIM
// (supported since test61).

#include "blind_deconv.hpp"

namespace blind {

// First entry of the user's grid is enough to smoke-test the path.
const std::array<float, LAMBDA_GRID_LEN> LAMBDA_MULTIPLIERS = {{ 0.5f }};

// Triangle init [0.25, 0.5, 1.0, 0.5, 0.25] placed so the peak (1.0) lands
// at index C_PEAK (= 5 by default). The rest is zero.
const std::array<float, M_PULSE> TRIANGLE_INIT = []() {
    std::array<float, M_PULSE> h = {};
    constexpr float base[5] = { 0.25f, 0.5f, 1.0f, 0.5f, 0.25f };
    constexpr int base_peak = 2;
    for (int k = 0; k < 5; ++k) {
        int idx = C_PEAK + (k - base_peak);
        if (idx >= 0 && idx < M_PULSE) {
            h[idx] = base[k];
        }
    }
    return h;
}();

} // namespace blind
