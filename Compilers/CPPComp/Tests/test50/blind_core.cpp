// blind_core.cpp - alternating minimization with lambda sweep.
//
// Mirror of blind_deconv_production.blind_deconvolve() with the patched
// triangle init, "free left / h>=0 right" projection, max-normalization,
// warm-start across lambdas, and sigma-self early-stop.

#include "blind_deconv.hpp"
#include "dsp_kernels.hpp"

#include <cmath>
#include <limits>

namespace blind {

// ---------------------------------------------------------------------------
// scratch layout helpers
// ---------------------------------------------------------------------------

struct Scratch {
    float* y_centered;     // WINDOW_LEN
    float* residual;       // WINDOW_LEN
    float* x_prev;         // X_LEN
    float* z;              // X_LEN
    float* grad_x;         // X_LEN  (also reused as x_new)
    float* grad_h;         // M_PULSE
    float* h_cand;         // M_PULSE
    float* h_work;         // M_PULSE
    float* h_warm;         // M_PULSE
    float* conv_buf;       // CONV_FULL_LEN
    float* mad_scratch;    // WINDOW_LEN  (also reused for h-update conv etc.)
};

static Scratch layout_(float* base) noexcept {
    Scratch s{};
    float* p = base;
    s.y_centered  = p;  p += WINDOW_LEN;
    s.residual    = p;  p += WINDOW_LEN;
    s.x_prev      = p;  p += X_LEN;
    s.z           = p;  p += X_LEN;
    s.grad_x      = p;  p += X_LEN;
    s.grad_h      = p;  p += M_PULSE;
    s.h_cand      = p;  p += M_PULSE;
    s.h_work      = p;  p += M_PULSE;
    s.h_warm      = p;  p += M_PULSE;
    s.conv_buf    = p;  p += CONV_FULL_LEN;
    s.mad_scratch = p;  // 32-float pad after this
    return s;
}

// ---------------------------------------------------------------------------
// helpers shared by x-FISTA and h-update
// ---------------------------------------------------------------------------

// Compute residual = conv(x, h) - y_centered, length WINDOW_LEN.
// conv_dest must hold at least CONV_FULL_LEN floats; the function writes the
// convolution into conv_dest and then stores residual into out_residual.
static void compute_residual_(const float* x, const float* h,
                              const float* y_centered,
                              float* conv_dest, float* out_residual) noexcept {
    dsp::convolve_full(x, X_LEN, h, M_PULSE, conv_dest);
    // conv(x, h) length = X_LEN + M - 1 = WINDOW_LEN.
    for (int n = 0; n < WINDOW_LEN; ++n) {
        out_residual[n] = conv_dest[n] - y_centered[n];
    }
}

// Lipschitz upper bound for the gradient of 0.5 ||H x - y||^2 with kernel h.
// Uses ||H||_2^2 <= ||h||_1^2 (Young's inequality, tight for non-negative h).
static float lipschitz_from_h_(const float* h, int M) noexcept {
    float s = 0.0f;
    for (int k = 0; k < M; ++k) s += std::fabs(h[k]);
    return s * s;
}

// Project h: free for k < C_PEAK, max(., 0) for k >= C_PEAK.
static void project_h_(float* h) noexcept {
    for (int k = C_PEAK; k < M_PULSE; ++k) {
        if (h[k] < 0.0f) h[k] = 0.0f;
    }
}

// Max-normalize h so max(h) = 1, in place. If max <= eps, leave untouched.
static void maxnorm_h_(float* h) noexcept {
    float m = h[0];
    for (int k = 1; k < M_PULSE; ++k) if (h[k] > m) m = h[k];
    if (m > 1e-12f) {
        const float inv = 1.0f / m;
        for (int k = 0; k < M_PULSE; ++k) h[k] *= inv;
    }
}

// 0.5 * ||conv(x,h) - y_centered||^2. Uses conv_dest as scratch.
static float data_term_(const float* x, const float* h,
                        const float* y_centered,
                        float* conv_dest) noexcept {
    dsp::convolve_full(x, X_LEN, h, M_PULSE, conv_dest);
    float acc = 0.0f;
    for (int n = 0; n < WINDOW_LEN; ++n) {
        const float r = conv_dest[n] - y_centered[n];
        acc += r * r;
    }
    return 0.5f * acc;
}

// ---------------------------------------------------------------------------
// x-FISTA: non-negative L1 (lambda > 0).
//
// On entry x_prev holds the current x (warm start). On exit x_prev holds
// the new x. The buffers z and grad_x are working scratch. Returns the
// final objective value (data + lambda * sum x).
// ---------------------------------------------------------------------------

static float fista_x_(const float* y_centered, const float* h,
                      float lambda_value,
                      float* x_prev, float* z, float* grad_x,
                      float* conv_buf) noexcept {
    // Step size from a Lipschitz upper bound.
    const float L  = lipschitz_from_h_(h, M_PULSE);
    const float step = (L > 0.0f) ? (1.0f / L) : 1.0f;

    // Initialise z to current x.
    for (int k = 0; k < X_LEN; ++k) z[k] = x_prev[k];
    float t = 1.0f;

    float prev_obj = std::numeric_limits<float>::infinity();

    for (int it = 0; it < X_FISTA_ITER_MAX; ++it) {
        // gradient at z: grad = correlate(conv(z,h) - y, h, valid).
        dsp::convolve_full(z, X_LEN, h, M_PULSE, conv_buf);
        for (int n = 0; n < WINDOW_LEN; ++n) {
            conv_buf[n] -= y_centered[n];
        }
        dsp::correlate_valid(conv_buf, WINDOW_LEN, h, M_PULSE,
                             grad_x, X_LEN);

        // x_new = max(0, z - step*grad - step*lambda). Reuse grad_x as x_new.
        const float shift = step * lambda_value;
        for (int k = 0; k < X_LEN; ++k) {
            const float v = z[k] - step * grad_x[k] - shift;
            grad_x[k] = (v > 0.0f) ? v : 0.0f;
        }

        // Nesterov acceleration.
        const float t_next = 0.5f * (1.0f + std::sqrt(1.0f + 4.0f * t * t));
        const float beta   = (t - 1.0f) / t_next;
        for (int k = 0; k < X_LEN; ++k) {
            z[k] = grad_x[k] + beta * (grad_x[k] - x_prev[k]);
        }

        // Commit x_new -> x_prev.
        for (int k = 0; k < X_LEN; ++k) x_prev[k] = grad_x[k];
        t = t_next;

        // Objective (data + lambda * sum x_prev, x >= 0 so |x|_1 = sum x).
        const float dt = data_term_(x_prev, h, y_centered, conv_buf);
        float sx = 0.0f;
        for (int k = 0; k < X_LEN; ++k) sx += x_prev[k];
        const float obj = dt + lambda_value * sx;

        if (std::isfinite(prev_obj)) {
            const float denom = (std::fabs(prev_obj) > 1.0f)
                                ? std::fabs(prev_obj) : 1.0f;
            const float rel = std::fabs(prev_obj - obj) / denom;
            if (rel < FISTA_TOL) return obj;
        }
        prev_obj = obj;
    }
    return prev_obj;
}

// ---------------------------------------------------------------------------
// h-update: projected gradient with backtracking, then max-normalize.
// Mirrors _update_h_maxnorm_signed in Python. Uses lambda = 0 inside (no L1).
// ---------------------------------------------------------------------------

static void update_h_(const float* y_centered, const float* x,
                      float* h_work, float* grad_h, float* h_cand,
                      float* residual,    // WINDOW_LEN
                      float* conv_buf     // CONV_FULL_LEN
                      ) noexcept {
    // Initial projection.
    project_h_(h_work);

    float step = H_INITIAL_STEP;

    for (int it = 0; it < H_GRAD_ITER_MAX; ++it) {
        // Current residual + data term.
        compute_residual_(x, h_work, y_centered, conv_buf, residual);
        float dt = 0.0f;
        for (int n = 0; n < WINDOW_LEN; ++n) dt += residual[n] * residual[n];
        dt *= 0.5f;

        // grad_h = correlate(residual, x, valid)[:M]. Length M.
        dsp::correlate_valid(residual, WINDOW_LEN, x, X_LEN,
                             grad_h, M_PULSE);

        // Backtracking.
        bool accepted = false;
        float trial = step;
        for (int bt = 0; bt < H_BACKTRACK_STEPS; ++bt) {
            for (int k = 0; k < M_PULSE; ++k) {
                h_cand[k] = h_work[k] - trial * grad_h[k];
            }
            project_h_(h_cand);

            const float cdt = data_term_(x, h_cand, y_centered, conv_buf);
            if (std::isfinite(cdt) && cdt <= dt) {
                for (int k = 0; k < M_PULSE; ++k) h_work[k] = h_cand[k];
                const float grown = trial * H_STEP_GROWTH;
                step = (grown < H_INITIAL_STEP) ? grown : H_INITIAL_STEP;
                accepted = true;
                break;
            }
            trial *= 0.5f;
        }
        if (!accepted) break;
    }

    // Max-normalize at end (peak in raw h, then normalize).
    maxnorm_h_(h_work);
}

// ---------------------------------------------------------------------------
// fit for one lambda: alternating x-FISTA + h-update.
// Writes the final x and h into x_prev / h_work. Returns final objective.
// Also writes:
//   *out_mse_y       : final mean square error of y_hat
//   *out_active      : number of x samples above ACTIVE_X_THRESHOLD
// ---------------------------------------------------------------------------

static float fit_one_lambda_(const float* y_centered, float lambda_value,
                             Scratch& s,
                             float* out_mse_y, int* out_active) noexcept {
    float prev_obj = std::numeric_limits<float>::infinity();
    float obj = prev_obj;

    for (int outer = 0; outer < OUTER_ITER_MAX; ++outer) {
        // x update for current h_work.
        fista_x_(y_centered, s.h_work, lambda_value,
                 s.x_prev, s.z, s.grad_x, s.conv_buf);

        // h update for current x_prev.
        update_h_(y_centered, s.x_prev,
                  s.h_work, s.grad_h, s.h_cand,
                  s.residual, s.conv_buf);

        // Outer convergence check via full objective.
        const float dt = data_term_(s.x_prev, s.h_work, y_centered,
                                    s.conv_buf);
        float sx = 0.0f;
        for (int k = 0; k < X_LEN; ++k) sx += s.x_prev[k];
        obj = dt + lambda_value * sx;

        if (std::isfinite(prev_obj)) {
            const float denom = (std::fabs(prev_obj) > 1.0f)
                                ? std::fabs(prev_obj) : 1.0f;
            const float rel = std::fabs(prev_obj - obj) / denom;
            if (rel < OUTER_TOL) break;
        }
        prev_obj = obj;
    }

    // Final MSE_y and active count.
    compute_residual_(s.x_prev, s.h_work, y_centered, s.conv_buf, s.residual);
    float sq = 0.0f;
    for (int n = 0; n < WINDOW_LEN; ++n) sq += s.residual[n] * s.residual[n];
    *out_mse_y = sq / static_cast<float>(WINDOW_LEN);

    int active = 0;
    for (int k = 0; k < X_LEN; ++k) {
        if (s.x_prev[k] > ACTIVE_X_THRESHOLD) ++active;
    }
    *out_active = active;

    return obj;
}

// ---------------------------------------------------------------------------
// Top-level blind_deconvolve.
// ---------------------------------------------------------------------------

void blind_deconvolve(const float* y_window,
                      BlindResult& out,
                      float* scratch_floats) noexcept {
    Scratch s = layout_(scratch_floats);

    // Step 1: iter-MAD for sigma_init and baseline_mu.
    const MadResult mad = iter_mad(y_window, WINDOW_LEN, s.mad_scratch);
    float sigma_init = mad.sigma;
    if (!(sigma_init > 0.0f)) {
        // Degenerate case: cannot proceed. Mark as not converged.
        for (int k = 0; k < M_PULSE; ++k)  out.h_hat[k] = 0.0f;
        for (int k = 0; k < X_LEN; ++k)    out.x_hat[k] = 0.0f;
        for (int n = 0; n < WINDOW_LEN; ++n) out.y_hat[n] = y_window[n];
        out.sigma_init      = 0.0f;
        out.baseline_mu     = mad.mu;
        out.lambda_selected = 0.0f;
        out.mse_y           = 0.0f;
        out.sigma_self      = 0.0f;
        out.ratio           = 0.0f;
        out.active_samples  = 0;
        out.K_params        = 0;
        out.converged       = false;
        return;
    }
    const float baseline_mu = mad.mu;

    for (int n = 0; n < WINDOW_LEN; ++n) {
        s.y_centered[n] = y_window[n] - baseline_mu;
    }

    // Step 2: seed h_warm with the triangle init (used for the first lambda).
    for (int k = 0; k < M_PULSE; ++k) s.h_warm[k] = TRIANGLE_INIT[k];

    // Track the best (smallest Q = |sigma_self - sigma_init|).
    float best_Q       = std::numeric_limits<float>::infinity();
    float best_mse     = 0.0f;
    float best_sigself = 0.0f;
    float best_lambda  = 0.0f;
    int   best_active  = 0;
    int   best_K       = 0;
    bool  any_done     = false;

    for (int li = 0; li < LAMBDA_GRID_LEN; ++li) {
        const float lam = sigma_init * LAMBDA_MULTIPLIERS[li];

        // Init h_work from h_warm (triangle for first; previous h_hat after).
        for (int k = 0; k < M_PULSE; ++k) s.h_work[k] = s.h_warm[k];
        // Init x to zeros.
        for (int k = 0; k < X_LEN; ++k)   s.x_prev[k] = 0.0f;

        float mse_y = 0.0f;
        int active = 0;
        (void) fit_one_lambda_(s.y_centered, lam, s, &mse_y, &active);

        const int K = active + (M_PULSE - 1);
        float sigma_self;
        if (K < WINDOW_LEN) {
            const float denom = 1.0f - static_cast<float>(K) /
                                       static_cast<float>(WINDOW_LEN);
            sigma_self = std::sqrt(mse_y / denom);
        } else {
            sigma_self = std::numeric_limits<float>::infinity();
        }
        const float Q = std::fabs(sigma_self - sigma_init);

        if (Q < best_Q) {
            best_Q       = Q;
            best_mse     = mse_y;
            best_sigself = sigma_self;
            best_lambda  = lam;
            best_active  = active;
            best_K       = K;
            // Copy current (x, h) into out.x_hat / out.h_hat.
            for (int k = 0; k < X_LEN; ++k)   out.x_hat[k] = s.x_prev[k];
            for (int k = 0; k < M_PULSE; ++k) out.h_hat[k] = s.h_work[k];
            any_done = true;
        }

        // Warm-start next lambda with the just-found h_hat.
        for (int k = 0; k < M_PULSE; ++k) s.h_warm[k] = s.h_work[k];

        // Early-stop: sigma_self is monotone increasing in lambda.
        if (sigma_self >= sigma_init) break;
    }

    // Compose final y_hat = conv(x_best, h_best) + baseline_mu.
    dsp::convolve_full(out.x_hat.data(), X_LEN,
                       out.h_hat.data(), M_PULSE, s.conv_buf);
    for (int n = 0; n < WINDOW_LEN; ++n) {
        out.y_hat[n] = s.conv_buf[n] + baseline_mu;
    }

    out.sigma_init      = sigma_init;
    out.baseline_mu     = baseline_mu;
    out.lambda_selected = best_lambda;
    out.mse_y           = best_mse;
    out.sigma_self      = best_sigself;
    if (sigma_init > 0.0f) {
        const float r = best_sigself / sigma_init;
        out.ratio = r * r;
    } else {
        out.ratio = 0.0f;
    }
    out.active_samples = best_active;
    out.K_params       = best_K;
    out.converged      = any_done;
}

} // namespace blind
