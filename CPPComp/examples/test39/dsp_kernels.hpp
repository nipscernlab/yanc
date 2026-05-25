// dsp_kernels.hpp - low-level numeric primitives.

#ifndef BLIND_DSP_KERNELS_HPP
#define BLIND_DSP_KERNELS_HPP

#include <cstddef>

namespace blind {
namespace dsp {

// Full convolution: out[n] = sum_k a[k] * b[n-k].
// Output length is la + lb - 1. Caller provides out buffer of that length.
void convolve_full(const float* a, int la,
                   const float* b, int lb,
                   float* out) noexcept;

// Correlation gradient: gradient_x[k] = sum_n residual[n+k] * h[k_h],
// equivalent to np.correlate(residual, h, mode="valid").
// Returns the first `out_len` correlation samples. The 'full' correlation
// has la - lb + 1 valid samples; if out_len exceeds that, the rest is 0.
void correlate_valid(const float* a, int la,
                     const float* b, int lb,
                     float* out, int out_len) noexcept;

// Sample median of a buffer. The buffer IS modified (partial sort).
// Returns the float median (average of middle two for even n).
float median_inplace(float* buf, int n) noexcept;

// Median absolute deviation of |x - mu|. Uses scratch[0..n-1].
float mad(const float* x, int n, float mu, float* scratch) noexcept;

// Solve the symmetric positive-definite linear system A x = b in-place.
// A is provided row-major in `a` of size n*n (will be overwritten with the
// Cholesky factor L). b will be overwritten with the solution x.
// Returns true on success, false if A is not SPD (zero or negative pivot).
bool solve_spd(float* a, float* b, int n) noexcept;

// Vector ops.
float dot(const float* a, const float* b, int n) noexcept;
float l2_norm(const float* a, int n) noexcept;
void  scale(float* a, int n, float s) noexcept;
void  axpy(float* y, const float* x, int n, float a) noexcept; // y += a*x
void  copy(float* dst, const float* src, int n) noexcept;
void  fill(float* dst, int n, float v) noexcept;
int   argmax_abs(const float* a, int n) noexcept;
int   argmax(const float* a, int n) noexcept;
float max_abs(const float* a, int n) noexcept;

} // namespace dsp
} // namespace blind

#endif // BLIND_DSP_KERNELS_HPP
