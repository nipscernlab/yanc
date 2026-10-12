// iter_mad.cpp - iterative MAD with sigma-clipping.
//
// Robust estimator of (mu, sigma) from a marginal sample. Returns the
// median-of-inliers and 1.4826 * MAD-of-inliers, where the inlier mask is
// |y - mu| < k*sigma, refined for up to max_iter passes.

#include "blind_deconv.hpp"
#include "dsp_kernels.hpp"

#include <cmath>

namespace blind {

static constexpr float MAD_TO_SIGMA = 1.4826f;  // 1 / Phi^{-1}(0.75)

MadResult iter_mad(const float* y, int N, float* scratch,
                   float clip_k, int max_iter, float tol) noexcept {
    MadResult out{0.0f, 0.0f};
    if (N <= 0) return out;

    // Initial mu / sigma over the full sample.
    dsp::copy(scratch, y, N);
    float mu = dsp::median_inplace(scratch, N);

    float mad = dsp::mad(y, N, mu, scratch);
    float sigma = MAD_TO_SIGMA * mad;
    if (!(sigma > 0.0f)) sigma = 1.0f;  // degenerate: all equal

    for (int it = 0; it < max_iter; ++it) {
        const float thr = clip_k * sigma;

        // Pass 1: fill scratch with inliers (raw values) for new mu.
        int count = 0;
        for (int i = 0; i < N; ++i) {
            const float d = y[i] - mu;
            if (d < thr && d > -thr) scratch[count++] = y[i];
        }
        if (count < 10) break;

        const float mu_new = dsp::median_inplace(scratch, count);

        // Pass 2: fill scratch with |inlier - mu_new| for new MAD.
        count = 0;
        for (int i = 0; i < N; ++i) {
            const float d = y[i] - mu;       // same mask as pass 1
            if (d < thr && d > -thr) {
                const float dn = y[i] - mu_new;
                scratch[count++] = (dn >= 0.0f) ? dn : -dn;
            }
        }
        const float mad_new   = dsp::median_inplace(scratch, count);
        const float sigma_new = MAD_TO_SIGMA * mad_new;

        const float abs_sigma = (sigma > 0.0f) ? sigma : 1.0f;
        const float ds = std::fabs(sigma_new - sigma);
        const float dm = std::fabs(mu_new    - mu);
        mu    = mu_new;
        sigma = (sigma_new > 0.0f) ? sigma_new : sigma;

        if (ds < tol * abs_sigma && dm < tol * abs_sigma) break;
    }

    out.mu    = mu;
    out.sigma = sigma;
    return out;
}

} // namespace blind
