#pragma yanc prname test71
// `T x(v);` -- a direct-initialized object whose first argument starts with a
// variable -- was a syntax error: after `T x(`, an identifier could still have
// begun a namespace-qualified parameter type (`std::uint8_t`), so the parser
// took the prototype path. The lexer now marks a name followed by `::` as a
// token of its own, and a plain identifier there starts the arguments. The
// qualified names keep working (namespace members, an enum class, std types).
#include <cstdint>

namespace geo { int scale = 3; int twice(int v) { return 2 * v; } }
namespace outer { namespace inner { int k2 = 7; } }
enum class Color { Red, Green, Blue };
struct P2 { int x; int y; };

class Filter {
public:
    int gain; int bias;
    Filter(int g) { gain = g; bias = 0; }
    Filter(int g, int b) { gain = g; bias = b; }
    Filter(P2 p) { gain = p.x; bias = p.y; }
    int apply(int v) { return gain * v + bias; }
};

int g = 4;
Filter g_f(g);                          // a global, a variable argument

void main(void) {
    int k = 5;
    Filter f(k);                        // a local, a variable argument
    out(0, f.apply(2));                 // 10
    Filter f2(k, g);                    // two variables
    out(0, f2.apply(1));                // 9
    P2 p = {6, 7};
    Filter f3(p);                       // an object
    out(0, f3.apply(1));                // 13
    Filter f4(k + 1);                   // an expression that starts with a variable
    out(0, f4.apply(1));                // 6
    Filter f5(f.apply(1));              // a call on another object
    out(0, f5.apply(1));                // 5
    out(0, g_f.apply(3));               // 12
    out(0, geo::twice(geo::scale));     // 6: qualified names still parse
    out(0, outer::inner::k2);           // 7: a chain of namespaces
    Color c = Color::Green;
    out(0, (int)c);                     // 1
    std::uint8_t b = 200;
    out(0, (int)b);                     // 200
}
