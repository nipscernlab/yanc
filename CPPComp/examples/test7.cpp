// Namespaces (transparent on this target): namespace blocks, N::name qualified
// calls, a using-directive, and a namespace-qualified type N::Vec with a method.
#pragma yanc prname test7
namespace math {
    int square(int x) { return x * x; }
    int cube(int x) { return x * x * x; }
    class Vec { public: int x, y; int sum() { return x + y; } };
}

namespace util {
    int doubler(int x) { return x + x; }
}

using namespace util;

void main(void) {
    out(0, math::square(5));    // 25
    out(0, math::cube(3));      // 27
    out(0, doubler(7));         // 14  (brought in by using)
    out(0, util::doubler(8));   // 16  (qualified)

    math::Vec v;                // namespace-qualified type
    v.x = 3; v.y = 4;
    out(0, v.sum());            // 7
}
