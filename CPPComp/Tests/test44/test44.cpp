#pragma yanc prname test44
// Probe: compile iter_mad.cpp UNMODIFIED via the testN/ folder layout.
// iter_mad.cpp pulls in blind_deconv.hpp + dsp_kernels.hpp + <cmath>.
// Goal at this stage is just to see what cppcomp errors on first.
//
// Driver: estimate (mu, sigma) of a 16-sample input with two outliers.
// The exact numeric result isn't the point yet; we want the chain to
// build before we worry about numerics.

#include "iter_mad.cpp"
#include "dsp_kernels.cpp"

// CPPComp namespaces are transparent (see CPPComp.y translation_unit rules),
// so blind::iter_mad / blind::MadResult are visible at file scope as bare
// names. No using-declaration needed in the driver.

void main(void) {
    float y[16]      = { 0.0f, 1.0f, -1.0f, 0.5f, -0.5f, 0.2f, -0.2f, 0.1f,
                         -0.1f, 0.3f, -0.3f, 0.4f, -0.4f, 50.0f, -50.0f, 0.0f };
    float scratch[16];

    // Call with all three defaults (clip_k=3, max_iter=10, tol=1e-3).
    MadResult r = iter_mad(y, 16, scratch);
    out(0, (int)(r.mu    * 1000.0f));   // mu in milli-units
    out(0, (int)(r.sigma * 1000.0f));   // sigma in milli-units
}
