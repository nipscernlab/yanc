// YANC port of test_main.cpp's blind_deconvolve call. Mirrors test36's I/O
// pattern: file I/O on the host becomes in(0)/out(0) port I/O here, with
// integer scaling around the YANC float boundary (see [[yanc-float-format]];
// I2F wraps at 2^22, so GAIN_IN * max|y| must stay below 4194304).
//
// Source files compiled UNMODIFIED:
//   blind_core.cpp + iter_mad.cpp + inverse_fir.cpp + dsp_kernels.cpp
//   blind_deconv.hpp + dsp_kernels.hpp
//
// Pipeline:
//   1. read WINDOW_LEN integers from in(0) -- pre-scaled by GAIN_IN on the host
//   2. reconstruct floats as (float)b / GAIN_IN
//   3. call blind::blind_deconvolve(y_window, result, scratch)
//   4. emit a summary of result via out(0, ...) -- key fields scaled to ints
//
// host preprocessor for test48.in (1000 lines, window [7000, 8000)):
//   awk 'NR>=7001 && NR<=8000 { printf "%d\n", int($1 * 20000 + 0.5) }' \
//       pzc_out_first10k.txt > test48.in
//
// Memory note: BLIND_SCRATCH_FLOATS ~= 8000 floats, BlindResult ~= 4000, plus
// the 1000-float window. Static allocation puts this in YANC's data memory.
// If the target's data memory is configured below ~14k words this WILL fail
// at asmcomp; the proper fix is to size the processor's RAM at synthesis.

#pragma yanc prname test48

#include "blind_core.cpp"
#include "iter_mad.cpp"
#include "inverse_fir.cpp"
#include "dsp_kernels.cpp"
#include "constants.cpp"

static const float GAIN_IN  = 20000.0f;     // max|y| in [7000,8000) ~ 143 -> 2.9M < 2^22
static const float GAIN_OUT = 1000000.0f;   // emit floats * 1e6 as ints

static float y_window[WINDOW_LEN];
static BlindResult result;
static float scratch[BLIND_SCRATCH_FLOATS];

void main(void) {
    // Read pre-scaled window from the testbench input port.
    for (int i = 0; i < WINDOW_LEN; i = i + 1) {
        int b = in(0);                             // [-2^22, 2^22) per host-side scaling
        y_window[i] = (float)b / GAIN_IN;          // (float)b first then divide -- avoid the (float)in() codegen bug
    }

    // Sentinel: input loop done; iter_mad probe next.
    out(0, 7777);

    // Probe iter_mad alone first (it's what blind_deconvolve calls first).
    MadResult mad = iter_mad(y_window, WINDOW_LEN, scratch);
    out(0, 1111);                              // iter_mad returned
    out(0, (int)(mad.mu    * 1000.0f));
    out(0, (int)(mad.sigma * 1000.0f));

    blind_deconvolve(y_window, result, scratch);

    // Sentinel: blind_deconvolve returned.
    out(0, 8888);

    // Summary: integer fields raw, float fields scaled by GAIN_OUT and quantized.
    out(0, result.converged ? 1 : 0);
    out(0, (int)(result.sigma_init      * GAIN_OUT));
    out(0, (int)(result.baseline_mu     * GAIN_OUT));
    out(0, (int)(result.lambda_selected * GAIN_OUT));
    out(0, (int)(result.mse_y           * GAIN_OUT));
    out(0, result.active_samples);
    out(0, result.K_params);

    // First few h_hat coefficients (M_PULSE = 15 total).
    for (int k = 0; k < M_PULSE; k = k + 1) {
        out(0, (int)(result.h_hat[k] * GAIN_OUT));
    }
}
