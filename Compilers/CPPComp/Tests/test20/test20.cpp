// C++ quick wins: type alias (using T = U), static_cast<T>(e), and scoped
// enumerations (enum class E { ... } accessed as E::name).
#pragma yanc prname test20
using Int = int;                          // type alias
enum class Color { Red, Green, Blue };    // scoped enum (Red=0, Green=1, Blue=2)

void main(void) {
    Int x = 40;                           // alias used as a type
    float f = 3.9;
    int n = static_cast<int>(f);          // C++ cast -> 3 (truncates toward zero)
    out(0, x + n);                        // 43
    out(0, (int)Color::Green);            // 1  (scoped enumerator)
    out(0, (int)Color::Blue);             // 2
    out(0, static_cast<int>(2.5 * 4.0));  // 10
}
