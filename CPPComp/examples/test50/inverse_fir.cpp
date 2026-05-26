// inverse_fir.cpp - Tikhonov-regularized inverse FIR, calibrated so the
// peak of (g * h) equals 1.

#include "blind_deconv.hpp"
#include "dsp_kernels.hpp"

#include <cmath>

namespace blind {

void inverse_tikhonov_calibrated(const float* h, float gamma,
                                 InverseFIR& out,
                                 float* scratch_floats) noexcept {
    constexpr int M  = M_PULSE;
    constexpr int L  = L_INVERSE;
    constexpr int FL = CONV_GH_LEN;       // L + M - 1

    // Scratch layout:
    //   [0          .. L*L         )  A    (Gram + gamma*I)
    //   [L*L        .. L*L + L     )  b    (RHS, becomes solution g)
    //   [L*L + L    .. L*L + L + FL)  gh   (conv(g, h))
    float* A  = scratch_floats;
    float* b  = scratch_floats + static_cast<std::size_t>(L) * L;
    float* gh = b + L;

    // delta_pos = argmax|h| + L/2, clamped to [0, FL-1].
    int peak_h    = dsp::argmax_abs(h, M);
    int delta_pos = peak_h + (L >> 1);
    if (delta_pos < 0)        delta_pos = 0;
    if (delta_pos >= FL)      delta_pos = FL - 1;

    // Autocorrelation r[d] = sum_{k=0..M-1-d} h[k] * h[k+d], d = 0..M-1.
    float r[M];
    for (int d = 0; d < M; ++d) {
        float s = 0.0f;
        const int kmax = M - d;
        for (int k = 0; k < kmax; ++k) s += h[k] * h[k + d];
        r[d] = s;
    }

    // A = H^T H + gamma * I.  H^T H is Toeplitz with entries r[|i-j|].
    for (int i = 0; i < L; ++i) {
        for (int j = 0; j < L; ++j) {
            const int d = (i >= j) ? (i - j) : (j - i);
            float v = (d < M) ? r[d] : 0.0f;
            if (i == j) v += gamma;
            A[i * L + j] = v;
        }
    }

    // b = H^T e_{delta_pos}: b[j] = h[delta_pos - j] if in [0,M), else 0.
    for (int j = 0; j < L; ++j) {
        const int idx = delta_pos - j;
        b[j] = (idx >= 0 && idx < M) ? h[idx] : 0.0f;
    }

    // Solve A g = b in place (b becomes g_raw).
    const bool ok = dsp::solve_spd(A, b, L);
    if (!ok) {
        for (int k = 0; k < L; ++k) out.g[k] = 0.0f;
        out.delta_pos  = 0;
        out.noise_gain = 0.0f;
        out.gamma      = gamma;
        return;
    }

    // gh = conv(g_raw, h), length FL.
    dsp::convolve_full(b, L, h, M, gh);
    const int peak  = dsp::argmax_abs(gh, FL);
    const float pk  = gh[peak];

    // Calibrate g so that gh[peak] = 1.
    if (std::fabs(pk) > 1e-12f) {
        const float inv = 1.0f / pk;
        for (int k = 0; k < L; ++k) out.g[k] = b[k] * inv;
    } else {
        for (int k = 0; k < L; ++k) out.g[k] = b[k];
    }

    out.delta_pos = peak;
    out.gamma     = gamma;
    out.noise_gain = dsp::l2_norm(out.g.data(), L);
}

void apply_inverse_fir(const float* y_window, float baseline_mu,
                       const InverseFIR& fir, float* x_tilde) noexcept {
    constexpr int L = L_INVERSE;
    const int dp = fir.delta_pos;

    // x_tilde[i] = sum_k (y[k] - mu) * g[dp + i - k]
    // valid k: max(0, dp+i-L+1) .. min(WINDOW_LEN-1, dp+i).
    for (int i = 0; i < WINDOW_LEN; ++i) {
        const int n = dp + i;
        int k_lo = n - (L - 1);
        if (k_lo < 0) k_lo = 0;
        int k_hi = n;
        if (k_hi > WINDOW_LEN - 1) k_hi = WINDOW_LEN - 1;
        float s = 0.0f;
        for (int k = k_lo; k <= k_hi; ++k) {
            s += (y_window[k] - baseline_mu) * fir.g[n - k];
        }
        x_tilde[i] = s;
    }
}

} // namespace blind
