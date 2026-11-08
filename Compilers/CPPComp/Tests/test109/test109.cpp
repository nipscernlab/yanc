// The #pragma yanc settings no test used (#134): a 16-bit processor with a
// 16-bit float (10 + 5 + the sign), two ports each way and the stack depths
// given. Each line printed is what C++ gives on a 16-bit int:
//
// out(0, a + b)                  -> -5536  30000 + 30000 wraps at 16 bits
// out(0, u + 1 == 0)             -> 1      65535u + 1 is 0
// out(0, (int)(f * 4.0f))        -> 6      1.5 * 4 in the 16-bit float
// out(0, sizeof(int))            -> 1      a word
// out(0, big)                    -> 32767  numeric_limits<int>::max() follows nubits
//
// out(1, g) writes a float to port 1, which gives the warning in warnings.txt
// (port 1 is not compared).

#pragma yanc nubits 16
#pragma yanc nbmant 10
#pragma yanc nbexpo 5
#pragma yanc nuioin 2
#pragma yanc nuioou 2
#pragma yanc ndstac 16
#pragma yanc sdepth 8

#include <limits>

void main(void)
{
    int a = 30000, b = 30000;
    out(0, a + b);
    unsigned u = 65535;
    out(0, u + 1 == 0);
    float f = 1.5f;
    out(0, (int)(f * 4.0f));
    out(0, sizeof(int));
    int big = std::numeric_limits<int>::max();
    out(0, big);
    float g = 2.5f;
    out(1, g);
}
