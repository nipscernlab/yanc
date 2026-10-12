// test51 - regression test for the cur_base-clobbering bug fixed in
// CPPComp.y. Before the fix, a cast inside a local variable's initializer
// would reduce its own `base_type` non-terminal mid-parse, clobbering the
// global `cur_base` that the declarator's action later consults to set
// the variable's type. So `float* b = scratch + static_cast<int>(L) * L;`
// would register `b` as int* (cur_base = int from the cast), causing
// b[k] reads/writes to emit int semantics (F2I/I2F around stores) and
// silently corrupting downstream computation -- exactly the test50
// inverse_tikhonov_calibrated palindromic-garbage symptom.
//
// Fix: capture cur_base via a mid-rule action before parsing the
// initializer, just like the direct-init form already does.
//
// This test exercises the exact buggy pattern and verifies the round-trip
// of floats through a pointer-arithmetic-derived `float*` works.

#pragma yanc prname test51

constexpr int L = 15;

static float scratch[400] = { 0.0f };
static float src[L] = {
    1.5f, -2.5f, 3.5f, -4.5f, 5.5f,
   -6.5f,  7.5f, -8.5f, 9.5f, -10.5f,
   11.5f,-12.5f, 13.5f,-14.5f, 15.5f
};
static float dst[L] = { 0.0f };

// Mimics inverse_fir.cpp:23 -- scratch-pointer arithmetic with a cast on a
// constexpr int. Reads/writes go through b as float*.
void roundtrip(const float* in, float* out, float* scratch_floats) noexcept {
    float* b = scratch_floats + static_cast<int>(L) * L;
    for (int k = 0; k < L; k = k + 1) b[k] = in[k];
    for (int k = 0; k < L; k = k + 1) out[k] = b[k];
}

void main(void) {
    roundtrip(src, dst, scratch);
    for (int k = 0; k < L; k = k + 1) out(0, (int)(dst[k] * 1000.0f));
}
