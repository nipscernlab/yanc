#pragma yanc prname test66
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
// Not here yet (TODO.md item 11): the display of unsigned words at or above
// 2^31 and the 8/16-bit integer types. Each joins this test when it lands.
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
}
