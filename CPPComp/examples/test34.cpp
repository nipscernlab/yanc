#pragma yanc prname test34
// Standard-library foundation: #include of bundled headers, fixed-width integer
// aliases, std::bit_cast (explicit template arguments) and std::sqrt/fabs.
#include <cstdint>
#include <cmath>
#include <bit>

template<class T> T maxv(T a, T b) { return a > b ? a : b; }

void main(void) {
    int32_t a = 7;
    std::int32_t b = 9;
    out(0, maxv(a, b));               // 9   (deduced function template)

    out(0, (int)std::sqrt(144.0f));   // 12
    out(0, (int)std::fabs(-8.5f));    // 8

    // bit_cast round-trip: reinterpret 2.0f as its int bits and back
    float x = 2.0f;
    int bits = std::bit_cast<int>(x); // explicit template argument <int>
    float y = std::bit_cast<float>(bits);
    out(0, (int)y);                   // 2
    float z = std::bit_cast<float>(std::bit_cast<int>(4.0f));
    out(0, (int)z);                   // 4
}
