#pragma yanc prname test36
// FISTA blind sparse deconvolution. The constants, globals and every function
// from project() through fit() are KEPT VERBATIM from the original program
// (CPPComp/.work/fista_ref.cpp) — same static/std:: declarations, same algorithm.
// The original's two platform-specific pieces are necessarily replaced because
// this target has no filesystem and no IEEE-754 floats:
//   * fbits() (reinterpret an IEEE bit-pattern) and the FILE reads/writes  ->  the
//     vector y arrives as INTEGERS scaled by GAIN_IN over in(), reconstructed with
//     (float)b / GAIN_IN, and h_hat is reported scaled by GAIN_OUT over out().
//   * int main(argc,argv)  ->  void main(void).
// GAIN_IN stays below 2^22 so int->float (I2F) — limited to a 23-bit magnitude on
// this core — does not wrap. Nothing numerical changed; output matches a gcc
// reference on the same vector to within float rounding.
#include <cmath>
#include <vector>

static const float GAIN_IN  = 30000.0f;
static const float GAIN_OUT = 1000000.0f;

static const int   M = 7, C = 3;
static const float LAM = 1.0f;
static const int   OUTER = 40, XIT = 20, HIT = 5, BT = 15;
static const float HSTEP = 1e-5f;

static int N, LX;
static std::vector<float> y, x_hat, s_z, s_x_prev, s_grad_x, s_conv;
static float h_hat[M], s_h_cand[M], s_h_best[M], s_grad_h[M];

static void project(float *h) {
    for (int i = 0; i < M; ++i) { if (h[i] < 0.0f) h[i] = 0.0f; if (h[i] > 1.0f) h[i] = 1.0f; }
    h[C] = 1.0f;
}

static void conv_full(const float *src, const float *kern) {
    for (int n = 0; n < N; ++n) s_conv[n] = 0.0f;
    for (int i = 0; i < LX; ++i) {
        float xi = src[i];
        for (int j = 0; j < M; ++j) s_conv[i + j] += xi * kern[j];
    }
}
static void residual_sub_y() { for (int n = 0; n < N; ++n) s_conv[n] -= y[n]; }
static float sum_residual_sq() { float s = 0.0f; for (int n = 0; n < N; ++n) s += s_conv[n] * s_conv[n]; return s; }
static void correlate_h() {
    for (int n = 0; n < LX; ++n) { float s = 0.0f; for (int k = 0; k < M; ++k) s += s_conv[n + k] * h_hat[k]; s_grad_x[n] = s; }
}
static void correlate_x() {
    for (int n = 0; n < M; ++n) { float s = 0.0f; for (int k = 0; k < LX; ++k) s += s_conv[n + k] * x_hat[k]; s_grad_h[n] = s; }
}
static float lipschitz() { float s = 0.0f; for (int k = 0; k < M; ++k) s += h_hat[k]; float L = s * s; return L < 1e-12f ? 1e-12f : L; }

static void fista() {
    for (int i = 0; i < LX; ++i) { s_z[i] = x_hat[i]; s_x_prev[i] = x_hat[i]; }
    float t = 1.0f, L = lipschitz(), step = 1.0f / L, step_lam = step * LAM;
    for (int it = 0; it < XIT; ++it) {
        conv_full(s_z.data(), h_hat); residual_sub_y(); correlate_h();
        for (int i = 0; i < LX; ++i) {
            float v = s_z[i] - step * s_grad_x[i] - step_lam;
            if (v < 0.0f) v = 0.0f;
            x_hat[i] = v;
        }
        float tn = 0.5f * (1.0f + std::sqrt(1.0f + 4.0f * t * t));
        float mom = (t - 1.0f) / tn;
        for (int i = 0; i < LX; ++i) {
            float xi = x_hat[i];
            s_z[i] = xi + mom * (xi - s_x_prev[i]);
            s_x_prev[i] = xi;
        }
        t = tn;
    }
}

static void update_h() {
    project(h_hat);
    for (int it = 0; it < HIT; ++it) {
        conv_full(x_hat.data(), h_hat); residual_sub_y();
        float dt = 0.5f * sum_residual_sq();
        correlate_x(); s_grad_h[C] = 0.0f;
        float best_dt = dt;
        for (int i = 0; i < M; ++i) s_h_best[i] = h_hat[i];
        float trial = HSTEP;
        for (int bt = 0; bt < BT; ++bt) {
            for (int i = 0; i < M; ++i) s_h_cand[i] = h_hat[i] - trial * s_grad_h[i];
            project(s_h_cand);
            conv_full(x_hat.data(), s_h_cand); residual_sub_y();
            float cdt = 0.5f * sum_residual_sq();
            if (cdt < best_dt) { best_dt = cdt; for (int i = 0; i < M; ++i) s_h_best[i] = s_h_cand[i]; }
            trial *= 0.5f;
        }
        for (int i = 0; i < M; ++i) h_hat[i] = s_h_best[i];
    }
    project(h_hat);
}

static void fit() {
    for (int i = 0; i < LX; ++i) x_hat[i] = 0.0f;
    for (int i = 0; i < M; ++i) h_hat[i] = 0.0f;
    h_hat[C - 1] = 0.5f; h_hat[C] = 1.0f; h_hat[C + 1] = 0.5f;
    for (int it = 0; it < OUTER; ++it) { fista(); update_h(); }
}

void main(void) {
    N = in(0);
    LX = N - M + 1;
    y.resize(N); x_hat.resize(LX); s_z.resize(LX); s_x_prev.resize(LX);
    s_grad_x.resize(LX); s_conv.resize(N);
    for (int n = 0; n < N; ++n) {
        int b = in(0);
        y[n] = (float)b / GAIN_IN;
    }

    fit();

    for (int j = 0; j < M; ++j) out(0, (int)(h_hat[j] * GAIN_OUT + 0.5f));
}
