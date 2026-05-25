#pragma yanc prname test47
// Static methods in class bodies (inline-defined), called as Class::name(args).
// Also `Class<T>::name(args)` via the new template-instantiation call syntax.
// And `std::numeric_limits<float>::infinity()` via the <limits> shim, which
// is the actual blocker for the user's blind_core.cpp.

#include <limits>

class Math {
public:
    static int twice(int x) { return x + x; }
    static int add(int a, int b) { return a + b; }
};

// Class template with a static method, called via Holder<int>::value()
template<class T>
class Holder {
public:
    static T value() { return (T)42; }
};

void main(void) {
    // Plain static method on a non-template class
    out(0, Math::twice(7));                         // 14
    out(0, Math::add(10, 20));                      // 30

    // Class<T>::static_method() -- new grammar + monomorphization
    out(0, Holder<int>::value());                   // 42

    // <limits> shim: numeric_limits<float>::infinity() returns a very large
    // positive float (no IEEE infinity on YANC, see [[yanc-float-format]]).
    // Verify it's larger than a "normal large" value.
    float big = std::numeric_limits<float>::infinity();
    float small = 1.0e10f;
    out(0, (big > small) ? 1 : 0);                  // 1
}
