#pragma yanc prname test40
// <array>: fixed-size std::array<T, N>. Uses the same non-type-template
// monomorphization the grammar already had for `template<class T, int N>`.
// Each (T, N) pair instantiates to a distinct concrete class with inline T[N]
// storage, so operator[] returns a true T&.
#include <array>

void main(void) {
    // int array: write the squares, read them back.
    std::array<int, 5> a;
    for (int i = 0; i < 5; i = i + 1) a[i] = i * i;
    for (int i = 0; i < 5; i = i + 1) out(0, a[i]);     // 0 1 4 9 16

    // size() returns N
    out(0, a.size());                                    // 5

    // float instantiation: <T, N> monomorphizes so the element math is float
    std::array<float, 3> v;
    v[0] = 1.5f; v[1] = 2.5f; v[2] = 3.5f;
    float s = v[0] + v[1] + v[2];                        // 7.5
    out(0, (int)(s * 100.0f));                           // 750

    // .data() now returns the field's actual address (was previously emitting
    // an extra LDA, returning the value at offset 0 instead). Writes through
    // the returned pointer must propagate back to the array.
    float* p = v.data();
    p[1] = 9.0f;
    out(0, (int)v[1]);                                   // 9
}
