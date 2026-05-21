#pragma yanc prname test35
// std::vector<float> via the new <vector> header: real type-parameter
// monomorphization, so operator[] returns float& and element-vs-element
// arithmetic uses float ops (the case that type-erasure got wrong).
#include <vector>

using namespace std;

static vector<float> a, b;          // GLOBAL vectors: zero-init = valid empty

void main(void) {
    a.resize(4);
    b.resize(4);
    for (int i = 0; i < 4; i = i + 1) {
        a[i] = (float)(i + 1);      // 1 2 3 4
        b[i] = 0.5f;
    }

    // element-vs-element float subtract + multiply (erasure would do int ops)
    float dot = 0.0f;
    for (int i = 0; i < 4; i = i + 1) {
        a[i] -= b[i];               // 0.5 1.5 2.5 3.5
        dot += a[i] * a[i];         // 0.25 + 2.25 + 6.25 + 12.25 = 21.0
    }
    out(0, (int)(dot * 2.0f));      // 42

    // data() raw-pointer access
    float* p = a.data();
    out(0, (int)(p[3] * 2.0f));     // 3.5 * 2 = 7

    // push_back + size + grow
    vector<float> v;                // local
    for (int i = 0; i < 5; i = i + 1) v.push_back((float)(i * i));
    out(0, v.size());               // 5
    out(0, (int)v[4]);              // 16
    out(0, (int)(v[2] + v[3]));     // 4 + 9 = 13
}
