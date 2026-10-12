#pragma yanc prname test74
// `T x(N::v);` used to parse as a function prototype. After `T x(` the parser
// had one token of lookahead and a namespace-qualified name could still start
// a parameter TYPE (`Filter f(std::uint8_t v)`), so it took the prototype path
// and the declaration was a syntax error. The lexer now classifies the LAST
// component of a qualified chain, so the first name already says which it is:
// a chain ending in a type keeps NS_IDENT, one ending in a value gets
// NS_VIDENT. `T x(v)` with a plain variable has worked since test71.
// TODO item 11(e).
#include <cstdint>

namespace geo { int scale = 3; int twice(int v) { return 2 * v; } }
namespace outer { namespace inner { int k = 7; int deep = 11; } }
enum class Color { Red, Green, Blue };

class Filter {
public:
    int gain; int bias;
    Filter(int g) { gain = g; bias = 0; }
    Filter(int g, int b) { gain = g; bias = b; }
    int apply(int v);                       // defined out of class, below
};

int Filter::apply(int v) { return gain * v + bias; }   // Class::method still parses

int g_plain = 5;
Filter g_q(geo::scale);                     // a global, qualified argument

void main(void) {
    Filter a(geo::scale);                   // the case that failed
    out(0, a.apply(2));                     // 6

    Filter b(outer::inner::k);              // a chain of namespaces
    out(0, b.apply(2));                     // 14

    Filter c(geo::scale, outer::inner::k);  // qualified in both arguments
    out(0, c.apply(2));                     // 13

    Filter d(geo::twice(geo::scale));       // a qualified call, qualified argument
    out(0, d.apply(2));                     // 12

    Filter e(g_plain);                      // a plain variable still works
    out(0, e.apply(2));                     // 10

    out(0, g_q.apply(2));                   // 6: the global built the same way

    // qualified names keep working everywhere else
    int s = geo::scale + outer::inner::deep;
    out(0, s);                              // 14
    out(0, geo::twice(outer::inner::k));    // 14
    Color col = Color::Green;
    out(0, (int)col);                       // 1: enum class is not a namespace
    std::uint8_t w = 200;
    out(0, (int)w);                         // 200: a qualified TYPE is still a type
    out(0, geo::scale);                     // 3: a qualified value on its own
}
