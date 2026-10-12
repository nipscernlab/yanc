#pragma yanc prname test61
// Lambda IIFE as a global initializer: the user's constants.cpp TRIANGLE_INIT, verbatim.
#include <array>

constexpr int M_PULSE = 15;
constexpr int C_PEAK  = 5;

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

// And the scalar form (return type deduced from the returned local).
const int MAGIC = []() {
    int acc = 0;
    for (int k = 1; k <= 4; ++k) acc = acc + k * k;
    return acc;
}();

void main(void) {
    for (int k = 0; k < M_PULSE; k = k + 1)
        out(0, (int)(TRIANGLE_INIT[k] * 1000.0f));
    out(0, MAGIC);    // expect 30 (1+4+9+16)
}

