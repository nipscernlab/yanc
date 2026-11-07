// The preprocessor and the library headers (#128): each line printed is
// what C++ gives, on the SAPHO float where a float is involved.
//
// out(0, s[1]), out(0, s[2])  -> 44, 98  ID("a,b") keeps the string whole
// out(0, sizeof(arr))         -> 5       N expands after a comment with it's
// out(0, v[1].a + v[1].b)     -> 0       vector<P>::resize value-initializes
// out(0, imax == 2147483647)  -> 1       numeric_limits<int>::max()
// out(0, imin == -imax - 1)   -> 1       numeric_limits<int>::min()
// out(0, (int)(eps * 4194304.0f)) -> 1   epsilon is 2^-22 (22 bits after the point)
// out(0, (int)(sqrt(1e20f) / 1.0e9f + 0.5f))   -> 10   sqrt(1e20) = 1e10
// out(0, (int)(sqrt(1e-20f) * 1.0e11f + 0.5f)) -> 10   sqrt(1e-20) = 1e-10
//                                   (rounded: #FROUND 0 truncates, and 1e10f / 1e9f
//                                   itself gives 9.99)
// out(0, (int)(sqrt(2.0f) * 1000.0f))          -> 1414

#include <vector>
#include <limits>
#include <cmath>

#define ID(x) x
#define N 5

struct P { int a; int b; };

int g0 = 7, g1 = 9;   // the first words of the data memory are not zero

void main(void)
{
    const char *s = ID("a,b");
    out(0, s[1]);
    out(0, s[2]);

    /* it's */ int arr[N];
    out(0, sizeof(arr));

    std::vector<P> v;
    v.resize(2);
    out(0, v[1].a + v[1].b);

    int imax = std::numeric_limits<int>::max();
    int imin = std::numeric_limits<int>::min();
    out(0, imax == 2147483647);
    out(0, imin == -imax - 1);
    float eps = std::numeric_limits<float>::epsilon();
    out(0, (int)(eps * 4194304.0f));

    out(0, (int)(std::sqrt(1e20f) / 1.0e9f + 0.5f));
    out(0, (int)(std::sqrt(1e-20f) * 1.0e11f + 0.5f));
    out(0, (int)(std::sqrt(2.0f) * 1000.0f));
}
