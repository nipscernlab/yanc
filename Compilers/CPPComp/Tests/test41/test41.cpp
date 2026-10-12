#pragma yanc prname test41
// 'constexpr' treated as 'const' for storage purposes. Integer constexprs go
// through the existing constant-folding path (usable as array bounds and
// template arguments). Float constexprs behave as runtime-initialised consts.
#include <array>

constexpr int   N       = 4;
constexpr int   M_PULSE = 7;
constexpr float SCALE   = 100.0f;

// constexpr int used as array bound (constant folded)
int arr[N] = {1, 2, 3, 4};

void main(void) {
    int sum = 0;
    for (int i = 0; i < N; i = i + 1) sum = sum + arr[i];
    out(0, sum);                        // 1+2+3+4 = 10

    // constexpr int as template argument
    std::array<int, M_PULSE> a;
    for (int i = 0; i < M_PULSE; i = i + 1) a[i] = i + 10;
    out(0, a[0]);                       // 10
    out(0, a[M_PULSE - 1]);             // 16
    out(0, a.size());                   // 7

    // constexpr float at runtime
    float v = 3.5f;
    out(0, (int)(v * SCALE));           // 350
}
