#pragma yanc prname test68
// Runtime helpers used only inside template instances: udivmod (unsigned /),
// u2f (unsigned -> float) and f2u (float -> unsigned). Instances are emitted
// after main, and the helpers used to be emitted before them, so a helper that
// only an instance needed was never emitted: the CAL went to an undefined
// label and the program printed nothing. main itself uses none of the three.
template <class T> T half(T v) { return v / 2u; }
template <class T> float to_f(T v) { return v; }
template <class T> T from_f(T like, float f) { return f; }

void main(void) {
    unsigned x = 4000000000u;
    out(0, (int)(half(x) >> 16));             // 30517: 2000000000 >> 16
    out(0, (int)(to_f(x) * 0.001f));          // 4000000
    out(0, (int)(from_f(x, 3.5e9f) >> 16));   // 53405: 3500000000 >> 16
}
