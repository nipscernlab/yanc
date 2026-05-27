#pragma yanc prname test49
// Isolated probe: does iter_mad scale? We invoke it with N=100 (not 1000) to
// see if the algorithm finishes well within budget. If even N=100 fails, the
// bottleneck is fundamental, not size-driven. We still read 1000 ints from
// the .in file (matches the testbench protocol), but only feed 100 to
// iter_mad.

#include "iter_mad.cpp"
#include "dsp_kernels.cpp"

static const float GAIN_IN  = 20000.0f;
static const float GAIN_OUT = 1000000.0f;

// Reduced from 1000 -- test50 covers full-scale numerical validation now.
static const int N_PROBE = 100;
static float y_window[N_PROBE];
static float scratch[N_PROBE];

void main(void) {
    // Read all 1000 samples.
    for (int i = 0; i < WINDOW_LEN; i = i + 1) {
        int b = in(0);
        if (i < N_PROBE) y_window[i] = (float)b / GAIN_IN;
    }
    out(0, 7777);

    MadResult mad = iter_mad(y_window, N_PROBE, scratch);

    out(0, 1111);
    out(0, (int)(mad.mu    * GAIN_OUT));
    out(0, (int)(mad.sigma * GAIN_OUT));
}
