// test50: numerical validation of the YANC blind_deconvolve port against
// the user's hosted reference IEEE-754 g++ build. Runs the FULL algorithm
// (OUTER=120, X_FISTA=200, etc) on the same input as the reference, then
// emits the 6 output arrays (y_input, h_hat, x_hat, y_hat, g, x_tilde) as
// scaled integers via out(0). A post-step descales and compares against
// 03_reference_outputs/ using the user's compare_outputs.py.
//
// Cycle budget is large -- the FULL FISTA over a 1000-sample window with
// 7-lambda grid costs ~75G clocks on YANC, ~18-25 min Verilator wall time.
//
// The driver mirrors test_main.cpp's flow (file I/O replaced by in/out
// ports). Step 1: read window from in(0). Step 2: blind_deconvolve.
// Step 3: inverse_tikhonov_calibrated + apply_inverse_fir for g, x_tilde.
// Step 4: dump all 6 arrays via out(0, scaled_int).

#pragma yanc prname test50

#include "blind_core.cpp"
#include "iter_mad.cpp"
#include "inverse_fir.cpp"
#include "dsp_kernels.cpp"
#include "constants.cpp"

static const float GAIN_IN  = 20000.0f;     // matches test48.in scaling
static const float GAIN_OUT = 1000000.0f;   // 6-digit precision in the int

static float        y_window[WINDOW_LEN];
static BlindResult  result;
static float        scratch[BLIND_SCRATCH_FLOATS];
static InverseFIR   fir;
static float        fir_scratch[INVERSE_FIR_SCRATCH_FLOATS];
static float        x_tilde[WINDOW_LEN];

void main(void) {
    // ---- 1. read window ----
    for (int i = 0; i < WINDOW_LEN; i = i + 1) {
        int b = in(0);
        y_window[i] = (float)b / GAIN_IN;
    }

    // ---- 2. blind_deconvolve ----
    blind_deconvolve(y_window, result, scratch);

    // ---- 3. inverse FIR (g, x_tilde) ----
    // gamma = GAMMA_COEFF * sigma_init^2  (matches test_main.cpp line 138)
    const float gamma = GAMMA_COEFF * result.sigma_init * result.sigma_init;
    inverse_tikhonov_calibrated(result.h_hat.data(), gamma, fir, fir_scratch);
    apply_inverse_fir(y_window, result.baseline_mu, fir, x_tilde);

    // ---- 4. emit all 6 arrays as scaled ints, in the order the post-step
    // script expects: y_input, h_hat, x_hat, y_hat, g, x_tilde.

    // y_input: 1000 floats (the window itself, post-scaling roundtrip)
    for (int i = 0; i < WINDOW_LEN; i = i + 1)
        out(0, (int)(y_window[i] * GAIN_OUT));

    // h_hat: M_PULSE = 15 floats
    for (int k = 0; k < M_PULSE; k = k + 1)
        out(0, (int)(result.h_hat[k] * GAIN_OUT));

    // x_hat: X_LEN = 986 floats
    for (int k = 0; k < X_LEN; k = k + 1)
        out(0, (int)(result.x_hat[k] * GAIN_OUT));

    // y_hat: WINDOW_LEN = 1000 floats
    for (int n = 0; n < WINDOW_LEN; n = n + 1)
        out(0, (int)(result.y_hat[n] * GAIN_OUT));

    // g: L_INVERSE = 15 floats
    for (int k = 0; k < L_INVERSE; k = k + 1)
        out(0, (int)(fir.g[k] * GAIN_OUT));

    // x_tilde: 1000 floats
    for (int n = 0; n < WINDOW_LEN; n = n + 1)
        out(0, (int)(x_tilde[n] * GAIN_OUT));

    // ---- 5. LOCALIZER PROBE for the inverse_fir divergence (test50 only) ----
    // YANC's calibrated g[] is palindromic+positive vs the host's mixed-sign
    // result. We reconstruct the same A and b that inverse_tikhonov_calibrated
    // builds, snapshot them before the solve, then call solve_spd on a fresh
    // copy and dump the raw solution. host_test_main.cpp emits the same three
    // probe arrays (probe_A_row_0, probe_b_orig, probe_g_raw) so we can
    // compare them array-by-array. Whichever array first diverges localizes
    // the bug (A: autocorrelation; b: argmax/h indexing; g_raw: solve_spd).
    {
        const int Mp  = M_PULSE;
        const int Lp  = L_INVERSE;
        const int FLp = CONV_GH_LEN;
        const float gamma_p = gamma;

        float r[M_PULSE];
        for (int d = 0; d < Mp; d = d + 1) {
            float s = 0.0f;
            int kmax = Mp - d;
            for (int k = 0; k < kmax; k = k + 1)
                s = s + result.h_hat[k] * result.h_hat[k + d];
            r[d] = s;
        }

        static float probe_A[L_INVERSE * L_INVERSE];
        for (int i = 0; i < Lp; i = i + 1) {
            for (int j = 0; j < Lp; j = j + 1) {
                int d = (i >= j) ? (i - j) : (j - i);
                float v = (d < Mp) ? r[d] : 0.0f;
                if (i == j) v = v + gamma_p;
                probe_A[i * Lp + j] = v;
            }
        }

        int peak_h = blind::dsp::argmax_abs(result.h_hat.data(), Mp);
        int delta_pos = peak_h + (Lp >> 1);
        if (delta_pos < 0) delta_pos = 0;
        if (delta_pos >= FLp) delta_pos = FLp - 1;

        static float probe_b[L_INVERSE];
        for (int j = 0; j < Lp; j = j + 1) {
            int idx = delta_pos - j;
            probe_b[j] = (idx >= 0 && idx < Mp) ? result.h_hat[idx] : 0.0f;
        }

        // Snapshot before solve (solve overwrites A and b).
        static float probe_A_row_0[L_INVERSE];
        static float probe_b_orig[L_INVERSE];
        for (int j = 0; j < Lp; j = j + 1) {
            probe_A_row_0[j] = probe_A[j];     // row 0
            probe_b_orig[j]  = probe_b[j];
        }

        bool ok = blind::dsp::solve_spd(probe_A, probe_b, Lp);

        // ---- Stage 2 of probe: re-run the calibration externally to check
        // if a parallel implementation gets the same wrong answer as
        // inverse_tikhonov_calibrated (deterministic codegen bug) or the
        // correct one (suggesting the bug lives inside that function's
        // scratch/static layout).
        static float probe_gh[CONV_GH_LEN];
        blind::dsp::convolve_full(probe_b, Lp,
                                  result.h_hat.data(), Mp,
                                  probe_gh);
        int   probe_peak = blind::dsp::argmax_abs(probe_gh, FLp);
        float probe_pk   = probe_gh[probe_peak];
        float probe_inv  = 1.0f / probe_pk;
        static float probe_g_calibrated[L_INVERSE];
        for (int k = 0; k < Lp; k = k + 1)
            probe_g_calibrated[k] = probe_b[k] * probe_inv;

        // Scale scalars by GAIN_OUT too so the post-step's descale-by-1e6
        // works uniformly across array and scalar entries.
        out(0, (int)((float)delta_pos * GAIN_OUT));    // 1 int
        out(0, (int)((ok ? 1.0f : 0.0f) * GAIN_OUT));  // 1 int
        for (int j = 0; j < Lp; j = j + 1)
            out(0, (int)(probe_A_row_0[j] * GAIN_OUT));   // L=15 ints
        for (int j = 0; j < Lp; j = j + 1)
            out(0, (int)(probe_b_orig[j] * GAIN_OUT));    // L=15 ints
        for (int j = 0; j < Lp; j = j + 1)
            out(0, (int)(probe_b[j] * GAIN_OUT));         // L=15 ints (= g_raw)
        // Stage-2 dumps:
        for (int n = 0; n < FLp; n = n + 1)
            out(0, (int)(probe_gh[n] * GAIN_OUT));        // FL=29 ints
        out(0, (int)((float)probe_peak * GAIN_OUT));      // 1 int
        out(0, (int)(probe_pk * GAIN_OUT));               // 1 int
        for (int k = 0; k < Lp; k = k + 1)
            out(0, (int)(probe_g_calibrated[k] * GAIN_OUT)); // L=15 ints
    }

    // Total: 100+15+86+100+15+100 + 2+3*15 + 29+1+1+15 = 509 outputs.
}
