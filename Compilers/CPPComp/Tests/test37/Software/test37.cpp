#pragma yanc prname test37
// <cstddef>: std::size_t / std::ptrdiff_t aliases. Both fold to int on this
// target (all integer widths are one 32-bit word), so they behave exactly like
// plain int and can be used wherever int can.
#include <cstddef>

void main(void) {
    std::size_t    n = 5;
    std::ptrdiff_t d = -3;
    out(0, (int)n);          // 5
    out(0, (int)d);          // -3
    out(0, (int)(n + d));    // 2

    // exercise std::size_t as a loop counter (the way the embedded code uses it)
    int sum = 0;
    for (std::size_t k = 0; k < n; k = k + 1) sum = sum + (int)k;
    out(0, sum);             // 0+1+2+3+4 = 10
}
