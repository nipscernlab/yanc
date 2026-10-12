// test87: <cstring>. It did not compile (`(const int*)src` was a syntax error)
// and no test included it. Sizes are in words here (sizeof(int) == 1), so
// memcpy / memset count words; the header used to take n / 4 words, so
// memcpy(a, b, sizeof(a)) copied a quarter of the array. Also the two casts
// with a const type the fix added: `(const T*)e` and static_cast<const T*>.
#pragma yanc prname test87
#include <cstring>

struct Pt { int x; int y; float w; };

void main(void) {
    int a[4] = {1, 2, 3, 4};
    int b[4] = {0, 0, 0, 0};
    std::memcpy(b, a, sizeof(a));
    out(0, b[0] + b[1] + b[2] + b[3]);     // 10: all four words copied

    std::memset(b, 0, sizeof(b));
    out(0, b[0] + b[1] + b[2] + b[3]);     // 0

    std::memset(b, 7, 2 * sizeof(int));
    out(0, b[0] + b[1] + b[2] + b[3]);     // 14: only the first two words

    float f[3] = {1.5, 2.5, 4.0};
    float g[3];
    std::memcpy(g, f, sizeof(f));
    out(0, (int)(g[0] + g[1] + g[2]));     // 8

    Pt p; p.x = 5; p.y = 6; p.w = 0.5;
    Pt q;
    std::memcpy(&q, &p, sizeof(Pt));
    out(0, q.x * 10 + q.y);                // 56
    out(0, (int)(q.w * 4.0));              // 2

    const int* r = (const int*)a;
    out(0, r[3]);                          // 4
    const int* t = static_cast<const int*>(b);
    out(0, t[1]);                          // 7
}
