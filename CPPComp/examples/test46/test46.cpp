#pragma yanc prname test46
// Probe: compile inverse_fir.cpp UNMODIFIED. Driver builds an h with a known
// peak, calls inverse_tikhonov_calibrated, dumps a couple of fields of the
// resulting InverseFIR via YANC port I/O.

#include "inverse_fir.cpp"
#include "dsp_kernels.cpp"

void main(void) {
    // h: delta at index 7 (middle of M=15) -> easy reference case.
    float h[15] = {0.0f};
    h[7] = 1.0f;

    InverseFIR fir{};
    float scratch[2400] = {0.0f};      // INVERSE_FIR_SCRATCH_FLOATS ~ 2200

    inverse_tikhonov_calibrated(h, 0.01f, fir, scratch);

    out(0, fir.delta_pos);                       // expected: 7 + 15/2 = 14
    out(0, (int)(fir.noise_gain * 1000.0f));     // ||g||_2 in milli
    out(0, (int)(fir.gamma      * 10000.0f));    // 0.01 -> 100
    out(0, (int)(fir.g[fir.delta_pos % 15] * 1000.0f));   // sample one g coeff
}
