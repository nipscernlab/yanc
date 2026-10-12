#pragma yanc prname test67
// Types wider than the 32-bit word (long long, unsigned long long, int64_t,
// uint64_t, double, long double) get one word on YANC. cppcomp warns at each
// variable declared with one -- global, local, loop variable, parameter,
// field, array field, static member, through a typedef -- that the requested
// size is ignored; warnings.txt lists exactly the warnings expected. A
// pointer, a typedef, a cast, sizeof and `long` (32 bits is what C++ asks of
// it) do not warn. The values stay inside 32 bits, so the outputs match host
// g++.
#include <cstdint>

long long g_total = 1000;                      // warns: global
typedef long long wide_t;                      // no warning: not a variable

struct Acc {
    std::int64_t sum;                          // warns: field
    double scale;                              // warns: field
    long long hist[2];                         // warns: array field
};

class Counter {
public:
    static const long long ticks = 7;          // warns: static member
};

long long twice(long long v) { return v * 2; } // warns: parameter

void main(void) {
    std::uint64_t u = 5;                       // warns: local
    long double ld = 0.25;                     // warns: local
    wide_t w = 3;                              // warns: through the typedef
    long plain = 100000;                       // no warning
    long long *p = &g_total;                   // no warning: a pointer
    Acc a;
    a.sum = 40; a.scale = 1.5; a.hist[0] = 1; a.hist[1] = 2;
    int acc = 0;
    for (long long i = 0; i < 4; i++) acc = acc + (int)i;   // warns: loop variable

    out(0, (int)g_total);                      // 1000
    out(0, (int)twice(21));                    // 42
    out(0, (int)(u + w));                      // 8
    out(0, (int)(ld * 8.0));                   // 2
    out(0, (int)(a.sum + a.hist[0] + a.hist[1]));   // 43
    out(0, (int)(a.scale * 4.0));              // 6
    out(0, (int)*p);                           // 1000
    out(0, (int)Counter::ticks);               // 7
    out(0, acc);                               // 6
    out(0, (int)((long long)plain / 1000));    // 100: a cast does not warn
    out(0, (int)(sizeof(long long) > 0));      // 1: nor does sizeof
}
