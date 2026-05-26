// blind_deconv.hpp - top-level API for the embedded blind deconvolver.
//
// Target: Cortex-A bare-metal, C++14, no exceptions, no RTTI, no heap.
// Compile with:
//     -std=c++14 -fno-exceptions -fno-rtti -fno-threadsafe-statics
//     -ffast-math -O3 -mcpu=cortex-a53 -mfpu=neon-fp-armv8
//
// Zero dynamic allocation. All buffers are caller-provided or have static
// fixed-size storage in classes.

#ifndef BLIND_DECONV_HPP
#define BLIND_DECONV_HPP

#include <array>
#include <cstddef>
#include <cstdint>

namespace blind {

// ---------------------------------------------------------------------------
// Compile-time constants. Change M / WINDOW_LEN here and recompile.
// ---------------------------------------------------------------------------

constexpr int M_PULSE          = 15;   // FIR length of h
constexpr int C_PEAK           =  5;   // suggested peak index in h
constexpr int L_INVERSE        = 15;   // length of inverse FIR g (= M)
// TEST48 LOCAL: reduced from 1000 -- test50 covers numerical validation
// against the IEEE-754 host gcc build. test48 only exercises the
// blind_deconvolve compile path on YANC.
constexpr int WINDOW_LEN       = 100;  // user-original: 1000
constexpr int X_LEN            = WINDOW_LEN - M_PULSE + 1;
constexpr int CONV_FULL_LEN    = WINDOW_LEN + M_PULSE - 1;
constexpr int CONV_GH_LEN      = L_INVERSE + M_PULSE - 1;

// Algorithm tunables (kept tight; profiled to give safe convergence).
// TEST48 LOCAL HACK: iteration counts dropped from the user's production
// values so the YANC Verilator simulation finishes in seconds instead of
// hours. The user's hosted source uses
//     OUTER=120, X_FISTA=200, H_GRAD=60, BT=30, LAMBDA=7
// which adds up to ~75G clocks on YANC (~25 min of Verilator wall time).
// This copy in examples/test48/ is purely a runtime-cost workaround and
// MUST NOT propagate back to the user's original tree. The other 6 files
// in examples/test48/ are unmodified.
constexpr int    OUTER_ITER_MAX        = 1;     // user-original: 120
constexpr int    X_FISTA_ITER_MAX      = 2;     // user-original: 200
constexpr int    H_GRAD_ITER_MAX       = 1;     // user-original: 60
constexpr int    H_BACKTRACK_STEPS     = 2;     // user-original: 30
constexpr float  OUTER_TOL             = 1e-4f;
constexpr float  FISTA_TOL             = 1e-4f;
constexpr float  H_INITIAL_STEP        = 1e-5f;
constexpr float  H_STEP_GROWTH         = 1.05f;
constexpr float  ACTIVE_X_THRESHOLD    = 1e-3f;

// Lambda grid as multipliers of sigma_init.
constexpr int    LAMBDA_GRID_LEN       = 1;     // user-original: 7
extern const std::array<float, LAMBDA_GRID_LEN> LAMBDA_MULTIPLIERS;

// Inverse FIR regularization scaling: gamma = GAMMA_COEFF * sigma_init^2.
constexpr float  GAMMA_COEFF           = 0.005f;

// Triangle initialization for h (peak at C_PEAK).
extern const std::array<float, M_PULSE> TRIANGLE_INIT;

// ---------------------------------------------------------------------------
// Public result types
// ---------------------------------------------------------------------------

struct BlindResult {
    std::array<float, M_PULSE>      h_hat;
    std::array<float, X_LEN>        x_hat;
    std::array<float, WINDOW_LEN>   y_hat;       // baseline restored
    float    sigma_init;
    float    baseline_mu;
    float    lambda_selected;
    float    mse_y;
    float    sigma_self;
    float    ratio;                 // (sigma_self / sigma_init)^2
    int      active_samples;
    int      K_params;
    bool     converged;             // false if no lambda was good
};

// ---------------------------------------------------------------------------
// Iterative MAD (baseline mu and noise sigma).
// ---------------------------------------------------------------------------

struct MadResult {
    float sigma;
    float mu;
};

// y must point to N floats. Uses scratch[0..N-1] internally (provided by
// caller to avoid dynamic alloc).
MadResult iter_mad(const float* y, int N, float* scratch,
                   float clip_k = 3.0f, int max_iter = 10,
                   float tol = 1e-3f) noexcept;

// ---------------------------------------------------------------------------
// Blind deconvolution on a single window.
//
// scratch_floats   : working memory; caller supplies. Required size is
//                    given by BLIND_SCRATCH_FLOATS.
// ---------------------------------------------------------------------------

constexpr std::size_t BLIND_SCRATCH_FLOATS =
      WINDOW_LEN              // y_centered
    + WINDOW_LEN              // residual / y_hat
    + X_LEN                   // x_prev (FISTA momentum)
    + X_LEN                   // z (FISTA accel)
    + X_LEN                   // gradient_x  (also reused as x_new)
    + M_PULSE                 // gradient_h
    + M_PULSE                 // h_candidate
    + M_PULSE                 // h_working   (current h during the fit)
    + M_PULSE                 // h_warm      (carry across lambdas)
    + CONV_FULL_LEN           // conv buffer (candidate residual)
    + WINDOW_LEN              // sigma / mad scratch (also reused)
    + 32;                     // padding for safety

void blind_deconvolve(const float* y_window,   // [WINDOW_LEN]
                      BlindResult& out,
                      float* scratch_floats    // [BLIND_SCRATCH_FLOATS]
                      ) noexcept;

// ---------------------------------------------------------------------------
// Inverse FIR (Tikhonov) calibrated so max(g * h) = 1.
// ---------------------------------------------------------------------------

struct InverseFIR {
    std::array<float, L_INVERSE> g;
    int   delta_pos;        // index in (g*h) where the calibrated peak lands
    float noise_gain;       // ||g||_2; std on x = sigma * noise_gain
    float gamma;            // regularization used
};

// scratch: ((L+M-1)*L + (L+M-1) + L*L + L) floats. ~  ~2200 floats for L=M=15.
constexpr std::size_t INVERSE_FIR_SCRATCH_FLOATS =
      static_cast<std::size_t>(CONV_GH_LEN) * L_INVERSE  // H matrix
    + CONV_GH_LEN                                        // e_0
    + static_cast<std::size_t>(L_INVERSE) * L_INVERSE    // H^T H
    + L_INVERSE                                          // H^T e_0
    + 32;

void inverse_tikhonov_calibrated(const float* h,         // [M_PULSE]
                                 float gamma,
                                 InverseFIR& out,
                                 float* scratch_floats   // [INVERSE_FIR_SCRATCH_FLOATS]
                                 ) noexcept;

// Apply g to y. x_tilde must have at least WINDOW_LEN entries. Subtracts
// baseline_mu inside.
void apply_inverse_fir(const float* y_window,
                       float baseline_mu,
                       const InverseFIR& fir,
                       float* x_tilde            // [WINDOW_LEN]
                       ) noexcept;

// ---------------------------------------------------------------------------
// Online averaging of h_hat with anomaly rejection.
// ---------------------------------------------------------------------------

class OnlineHAverager {
public:
    static constexpr int RECENT_BUFFER_SIZE = 50;

    OnlineHAverager(float ratio_min      = 0.7f,
                    float ratio_max      = 1.5f,
                    float cos_sim_min    = 0.95f,
                    int   bootstrap_n    = 3) noexcept;

    // Returns true if the result was accepted into the running mean.
    bool feed(const BlindResult& res) noexcept;

    int   count()       const noexcept { return count_; }
    int   rejected()    const noexcept { return rejected_; }
    int   seen()        const noexcept { return seen_; }
    const std::array<float, M_PULSE>& h_avg() const noexcept { return h_avg_; }
    bool  has_h_avg()   const noexcept { return count_ > 0; }

    // Coefficient-wise std across the recent ring buffer. Returns false if
    // fewer than 2 windows accepted (no std defined yet).
    bool recent_std(std::array<float, M_PULSE>& out) const noexcept;

private:
    void align_to_peak(const std::array<float, M_PULSE>& h_raw,
                       std::array<float, M_PULSE>& h_aligned) const noexcept;
    float cosine_sim(const std::array<float, M_PULSE>& a,
                     const std::array<float, M_PULSE>& b) const noexcept;

    float ratio_min_;
    float ratio_max_;
    float cos_sim_min_;
    int   bootstrap_n_;

    std::array<float, M_PULSE>   sum_h_      = {};
    std::array<float, M_PULSE>   h_avg_      = {};
    int                           count_      = 0;
    int                           rejected_   = 0;
    int                           seen_       = 0;

    // Ring buffer of recent accepted h_hats (aligned).
    std::array<std::array<float, M_PULSE>, RECENT_BUFFER_SIZE> recent_ = {};
    int recent_head_  = 0;
    int recent_count_ = 0;
};

// ---------------------------------------------------------------------------
// Online pipeline: blind -> h_avg -> g (recomputed on demand).
// ---------------------------------------------------------------------------

class OnlineBlindPipeline {
public:
    OnlineBlindPipeline(float g_change_threshold = 0.01f) noexcept;

    struct StepOutput {
        const BlindResult*  result;     // pointer to internal result (lives until next feed)
        const InverseFIR*   g;          // pointer to current inverse filter, or nullptr
        bool                accepted;
        bool                g_updated;
        int                 g_updates_total;
    };

    // Process one window. The output references internal storage and is
    // valid until the next call to feed().
    StepOutput feed(const float* y_window) noexcept;

private:
    void maybe_update_g_(float sigma_typical) noexcept;

    float                  g_change_threshold_;
    BlindResult            last_result_       = {};
    InverseFIR             current_g_         = {};
    OnlineHAverager        averager_;
    std::array<float, M_PULSE>  h_avg_at_last_g_ = {};
    bool                   has_g_              = false;
    int                    g_updates_          = 0;

    // Scratch buffers (allocated once with the object).
    std::array<float, BLIND_SCRATCH_FLOATS>      blind_scratch_;
    std::array<float, INVERSE_FIR_SCRATCH_FLOATS> fir_scratch_;
};

} // namespace blind

#endif // BLIND_DECONV_HPP
