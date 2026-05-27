// dsp_kernels.cpp - implementations of the low-level numeric primitives.

#include "dsp_kernels.hpp"

#include <cmath>

namespace blind {
namespace dsp {

// ---------------------------------------------------------------------------
// Convolution / correlation
// ---------------------------------------------------------------------------

void convolve_full(const float* a, int la,
                   const float* b, int lb,
                   float* out) noexcept {
    const int Lout = la + lb - 1;
    for (int n = 0; n < Lout; ++n) out[n] = 0.0f;
    for (int i = 0; i < la; ++i) {
        const float ai = a[i];
        for (int j = 0; j < lb; ++j) {
            out[i + j] += ai * b[j];
        }
    }
}

void correlate_valid(const float* a, int la,
                     const float* b, int lb,
                     float* out, int out_len) noexcept {
    int valid_len = la - lb + 1;
    if (valid_len < 0) valid_len = 0;
    const int n = (out_len < valid_len) ? out_len : valid_len;
    for (int k = 0; k < n; ++k) {
        float s = 0.0f;
        for (int j = 0; j < lb; ++j) {
            s += a[k + j] * b[j];
        }
        out[k] = s;
    }
    for (int k = n; k < out_len; ++k) out[k] = 0.0f;
}

// ---------------------------------------------------------------------------
// Quickselect-based median. Modifies the input buffer.
// Median-of-three pivot to avoid worst-case on nearly-sorted input.
// ---------------------------------------------------------------------------

static inline void swap_(float& a, float& b) noexcept {
    float t = a; a = b; b = t;
}

static int partition_(float* a, int lo, int hi) noexcept {
    // Median-of-three: pick median of a[lo], a[mid], a[hi], place at hi-1.
    const int mid = lo + ((hi - lo) >> 1);
    if (a[mid] < a[lo]) swap_(a[mid], a[lo]);
    if (a[hi]  < a[lo]) swap_(a[hi],  a[lo]);
    if (a[hi]  < a[mid]) swap_(a[hi], a[mid]);
    // a[lo] <= a[mid] <= a[hi]; use a[mid] as pivot.
    const float pivot = a[mid];
    swap_(a[mid], a[hi - 1]);  // hide pivot at hi-1
    int i = lo;
    int j = hi - 1;
    while (true) {
        while (a[++i] < pivot) {}
        while (a[--j] > pivot) {}
        if (i >= j) break;
        swap_(a[i], a[j]);
    }
    swap_(a[i], a[hi - 1]);    // restore pivot
    return i;
}

static float quickselect_(float* a, int lo, int hi, int k) noexcept {
    while (hi - lo > 2) {
        const int p = partition_(a, lo, hi);
        if (p == k) return a[p];
        if (p < k)  lo = p + 1;
        else        hi = p - 1;
    }
    // Small range: insertion sort then return.
    for (int i = lo + 1; i <= hi; ++i) {
        const float v = a[i];
        int j = i - 1;
        while (j >= lo && a[j] > v) { a[j + 1] = a[j]; --j; }
        a[j + 1] = v;
    }
    return a[k];
}

float median_inplace(float* buf, int n) noexcept {
    if (n <= 0)  return 0.0f;
    if (n == 1)  return buf[0];
    if (n == 2)  return 0.5f * (buf[0] + buf[1]);
    if (n & 1) {
        return quickselect_(buf, 0, n - 1, n >> 1);
    }
    // even n: lower median at n/2-1, upper at n/2.
    const int km = (n >> 1) - 1;
    const float lo_med = quickselect_(buf, 0, n - 1, km);
    // After quickselect for km, all elements at indices > km are >= lo_med.
    float upper_min = buf[km + 1];
    for (int i = km + 2; i < n; ++i) {
        if (buf[i] < upper_min) upper_min = buf[i];
    }
    return 0.5f * (lo_med + upper_min);
}

float mad(const float* x, int n, float mu, float* scratch) noexcept {
    for (int i = 0; i < n; ++i) {
        const float d = x[i] - mu;
        scratch[i] = (d >= 0.0f) ? d : -d;
    }
    return median_inplace(scratch, n);
}

// ---------------------------------------------------------------------------
// Symmetric positive-definite linear solve via Cholesky.
// A is row-major n*n. On return the lower triangle holds L (A = L L^T).
// b is overwritten with the solution.
// ---------------------------------------------------------------------------

bool solve_spd(float* a, float* b, int n) noexcept {
    // Factor: a[j*n+j] = L_jj, a[i*n+j] = L_ij for i > j.
    for (int j = 0; j < n; ++j) {
        float sum = a[j * n + j];
        for (int k = 0; k < j; ++k) {
            const float v = a[j * n + k];
            sum -= v * v;
        }
        if (!(sum > 0.0f)) return false;     // catches NaN and <=0
        const float ljj = std::sqrt(sum);
        a[j * n + j] = ljj;
        const float inv = 1.0f / ljj;
        for (int i = j + 1; i < n; ++i) {
            float s = a[i * n + j];
            for (int k = 0; k < j; ++k) {
                s -= a[i * n + k] * a[j * n + k];
            }
            a[i * n + j] = s * inv;
        }
    }
    // Forward solve L y = b.
    for (int i = 0; i < n; ++i) {
        float s = b[i];
        for (int k = 0; k < i; ++k) s -= a[i * n + k] * b[k];
        b[i] = s / a[i * n + i];
    }
    // Backward solve L^T x = y.
    for (int i = n - 1; i >= 0; --i) {
        float s = b[i];
        for (int k = i + 1; k < n; ++k) s -= a[k * n + i] * b[k];
        b[i] = s / a[i * n + i];
    }
    return true;
}

// ---------------------------------------------------------------------------
// Vector ops.
// ---------------------------------------------------------------------------

float dot(const float* a, const float* b, int n) noexcept {
    float s = 0.0f;
    for (int i = 0; i < n; ++i) s += a[i] * b[i];
    return s;
}

float l2_norm(const float* a, int n) noexcept {
    float s = 0.0f;
    for (int i = 0; i < n; ++i) s += a[i] * a[i];
    return std::sqrt(s);
}

void scale(float* a, int n, float s) noexcept {
    for (int i = 0; i < n; ++i) a[i] *= s;
}

void axpy(float* y, const float* x, int n, float a) noexcept {
    for (int i = 0; i < n; ++i) y[i] += a * x[i];
}

void copy(float* dst, const float* src, int n) noexcept {
    for (int i = 0; i < n; ++i) dst[i] = src[i];
}

void fill(float* dst, int n, float v) noexcept {
    for (int i = 0; i < n; ++i) dst[i] = v;
}

int argmax_abs(const float* a, int n) noexcept {
    int best = 0;
    float bv = std::fabs(a[0]);
    for (int i = 1; i < n; ++i) {
        const float v = std::fabs(a[i]);
        if (v > bv) { bv = v; best = i; }
    }
    return best;
}

int argmax(const float* a, int n) noexcept {
    int best = 0;
    float bv = a[0];
    for (int i = 1; i < n; ++i) {
        if (a[i] > bv) { bv = a[i]; best = i; }
    }
    return best;
}

float max_abs(const float* a, int n) noexcept {
    float m = std::fabs(a[0]);
    for (int i = 1; i < n; ++i) {
        const float v = std::fabs(a[i]);
        if (v > m) m = v;
    }
    return m;
}

} // namespace dsp
} // namespace blind
