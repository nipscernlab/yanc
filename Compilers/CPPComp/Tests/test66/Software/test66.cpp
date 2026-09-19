#pragma yanc prname test66
#include <cstdint>
// Signed and unsigned on a signed-only ALU. The ULA compares, divides and
// converts as signed; cppcomp compensates for `unsigned`: comparisons flip
// bit 31 of both operands before the signed compare, `/` and `%` call the
// software divider (udivmod), `>>` picks SHR (logical) over SRS (arithmetic),
// and int -> unsigned mixes follow C's usual arithmetic conversions. Expected
// values come from host g++ (built with -Wno-narrowing: two initializer lists
// below convert on purpose). Also signed bitfields, which were read without
// sign extension (`int s : 4` holding -1 read 15), and every conversion site:
// argument, return, cast, initializer, assignment, aggregate, bitfield store,
// template instance. Arguments were never converted (a float passed to an int
// parameter arrived as its raw bits), unsigned <-> float went through the
// signed I2F/F2I, and bool held whatever was stored in it.
// The exact-width 8- and 16-bit types (int8_t ... uint16_t) wrap to their
// width when a value is stored, and are ints in arithmetic, as C++ promotes
// them; they were plain 32-bit words (uint8_t 255 + 1 gave 256). And ++/-- on
// a bitfield stepped the whole word, carrying into the neighbouring fields.
// Not here yet (TODO.md item 11): the display of unsigned words at or above
// 2^31.
struct BF { int s : 4; unsigned u : 4; int one : 1; int mid : 7; };
union  BU { unsigned raw; BF f; };
struct BB { bool flag : 1; int n : 8; };

float    twice_f(float x)  { return x * 2.0f; }
int      plus1(int x)      { return x + 1; }
bool     pass_b(bool b)    { return b; }
bool     ret_b(void)       { return 7; }
unsigned ret_u(float x)    { return x; }
float    ret_f(unsigned u) { return u; }
template <class T> float to_f(T v) { return v; }
std::uint8_t next_u8(std::uint8_t v) { return v + 1; }
struct Pkt { std::uint8_t len; std::int16_t temp; };
struct BF2 { unsigned a : 4; unsigned b : 4; int s : 4; };

void main(void) {
    // comparisons
    unsigned big = 3000000000u;      // bit 31 set
    unsigned five = 5u;
    unsigned hi = 0xFFFFFFFFu;
    unsigned mid = 0x80000000u;
    unsigned low = 0x7FFFFFFFu;
    int m1 = -1;
    out(0, big > five);              // 1
    out(0, hi > mid);                // 1
    out(0, mid < low);               // 0
    out(0, mid >= low);              // 1
    out(0, low <= mid);              // 1
    out(0, m1 < 1);                  // 1: signed
    out(0, m1 < five);               // 0: -1 converts to 0xFFFFFFFF

    // wrap-around and mixes
    unsigned z = 0u;
    z = z - 1u;                      // 0xFFFFFFFF
    out(0, (int)(z >> 1));           // 2147483647
    unsigned one = 1u; int n2 = -2;
    out(0, (int)(one + n2));         // -1

    // division and modulo
    int n7 = -7; int p7 = 7; int p2 = 2;
    out(0, n7 / p2);                 // -3: toward zero
    out(0, n7 % p2);                 // -1: sign of the dividend
    out(0, p7 / n2);                 // -3
    out(0, (int)(big / five));       // 600000000: software divider
    out(0, (int)(big % 7u));         // 4
    out(0, (int)(z / 3u));           // 1431655765

    // right shifts
    int n16 = -16;
    out(0, n16 >> 2);                // -4: arithmetic
    out(0, (int)(mid >> 28));        // 8: logical

    // int <-> float, within the signed range
    int i7 = -7; float f1 = i7;
    out(0, (int)(f1 * 10.0f));       // -70
    float fn = -2.7f; float fp = 2.7f;
    out(0, (int)fn);                 // -2: toward zero
    out(0, (int)fp);                 // 2

    // signed bitfields
    BF x;
    x.s = -1; x.u = 15; x.one = -1; x.mid = -64;
    out(0, x.s);                     // -1
    out(0, x.u);                     // 15
    out(0, x.one);                   // -1: a 1-bit signed field is 0 or -1
    out(0, x.mid);                   // -64
    x.s = 7;  out(0, x.s);           // 7
    x.s = -8; out(0, x.s);           // -8
    x.s++;    out(0, x.s);           // -7
    x.s += 3; out(0, x.s);           // -4
    int k = x.s * 10; out(0, k);     // -40
    out(0, x.s < 0);                 // 1
    out(0, x.u);                     // 15: neighbour untouched
    out(0, x.mid);                   // -64: neighbour untouched
    BU b; b.raw = 0;
    b.f.s = -2;
    out(0, (int)b.raw);              // 14
    b.raw = 10;                      // bits 0..3 = 1010
    out(0, b.f.s);                   // -6

    // conversions
    float f35 = 3.5e9f; int i5 = 5;
    out(0, (int)twice_f(-3));        // -6: int -> float argument
    out(0, plus1(2.9f));             // 3: float -> int argument
    out(0, (int)pass_b(5));          // 1: int -> bool argument
    out(0, (int)ret_b());            // 1: int -> bool return
    out(0, (int)(ret_u(f35) / 1000u));      // 3500000: float -> unsigned return
    out(0, (int)(ret_f(big) / 1000.0f));    // 3000000: unsigned -> float return
    out(0, (int)(to_f(big) / 1000.0f));     // 3000000: in a template instance
    out(0, (int)((float)big / 1000.0f));    // 3000000: casts
    out(0, (int)((unsigned)f35 / 1000u));   // 3500000
    out(0, (int)(bool)i5);         // 1
    out(0, (int)(bool)0.25f);        // 1
    float fz = 0.0f;
    out(0, (int)(bool)fz);           // 0
    float fa = big;
    out(0, (int)(fa / 1000.0f));     // 3000000: initializers and assignment
    unsigned ua = f35;
    out(0, (int)(ua / 1000u));       // 3500000
    ua = 4.0e9f;
    out(0, (int)(ua / 1000u));       // 4000000
    fa = 4294967295u;
    out(0, (int)(fa / 1000.0f));     // 4294967: rounds up to 2^32
    unsigned odd = 2147483649u; float fo = odd;
    out(0, (int)(fo / 1000.0f));     // 2147483: 2^31 + 1 rounds to 2^31
    unsigned small = 16777217u; float fs = small;
    out(0, (int)fs);                 // 16777216: the float rounding, not the conversion
    bool ba = i5;
    out(0, (int)ba);                 // 1
    bool bf = 0.25f;
    out(0, (int)bf);                 // 1
    ba = 9;
    out(0, (int)ba);                 // 1
    ba = i5 > 9;
    out(0, (int)ba);                 // 0
    bool barr[2] = {5, 0};
    out(0, (int)barr[0]);            // 1: aggregates
    out(0, (int)barr[1]);            // 0
    float farr[1] = {3000000000u};
    out(0, (int)(farr[0] / 1000.0f)); // 3000000
    float mix = big * 0.5f;
    out(0, (int)(mix / 1000.0f));    // 1500000: an unsigned operand of a float operation
    BB bb;
    bb.flag = 2; bb.n = 2.7f;
    out(0, (int)bb.flag);            // 1: bitfield stores
    out(0, bb.n);                    // 2

    // exact-width types
    std::uint8_t u8 = 255; u8 = u8 + 1;
    out(0, (int)u8);                 // 0
    u8 = 0; u8--;
    out(0, (int)u8);                 // 255
    u8 = 255; int was = u8++;
    out(0, was);                     // 255
    out(0, (int)u8);                 // 0
    u8 = 250; u8 += 10;
    out(0, (int)u8);                 // 4
    std::int8_t i8 = 127; i8 = i8 + 1;
    out(0, (int)i8);                 // -128
    i8 = -128; i8--;
    out(0, (int)i8);                 // 127
    std::uint16_t u16 = 65535; u16++;
    out(0, (int)u16);                // 0
    std::int16_t i16 = 32767; i16 = i16 + 1;
    out(0, (int)i16);                // -32768
    std::uint8_t a1 = 200, a2 = 100;
    int ssum = a1 + a2;
    out(0, ssum);                    // 300: promoted to int
    std::uint8_t tsum = a1 + a2;
    out(0, (int)tsum);               // 44
    std::uint8_t five8 = 5;
    out(0, five8 > -1);              // 1: an int comparison, not an unsigned one
    std::uint8_t buf8[3] = {0, 0, 0};
    buf8[1] = 300; buf8[1]++; buf8[2] = buf8[1] * 10;
    out(0, (int)buf8[2]);            // 194
    std::uint8_t *pb = buf8; *pb = 511;
    out(0, (int)buf8[0]);            // 255
    out(0, (int)next_u8(255));       // 0: argument and return
    out(0, (int)next_u8(300));       // 45
    out(0, (int)(std::int8_t)200);   // -56: casts
    out(0, (int)(std::int16_t)40000); // -25536
    out(0, (int)(std::uint16_t)-1);  // 65535
    std::uint8_t fu8 = 3.9f;
    out(0, (int)fu8);                // 3
    Pkt pk; pk.len = 260; pk.temp = -40000;
    out(0, (int)pk.len);             // 4
    out(0, (int)pk.temp);            // 25536
    std::uint8_t data8[4] = {200, 100, 50, 25};
    std::uint8_t chk = 0;
    for (int i = 0; i < 4; i++) chk += data8[i];
    out(0, (int)chk);                // 119: a checksum
    std::int8_t neg8 = -1; std::uint8_t un8 = neg8;
    out(0, (int)un8);                // 255

    // ++ and -- on a bitfield
    BF2 w; w.a = 15; w.b = 0; w.s = 0;
    w.b++;
    out(0, w.a);                     // 15: the neighbour untouched
    out(0, w.b);                     // 1
    w.a++;
    out(0, w.a);                     // 0: wraps in its 4 bits
    out(0, w.b);                     // 1
    w.s = 7; w.s++;
    out(0, w.s);                     // -8
    int wold = w.b--;
    out(0, wold);                    // 1
    out(0, w.b);                     // 0
}
