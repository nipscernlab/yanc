#pragma yanc prname test66
// Signed and unsigned on a signed-only ALU. The ULA compares, divides and
// converts as signed; cppcomp compensates for `unsigned`: comparisons flip
// bit 31 of both operands before the signed compare, `/` and `%` call the
// software divider (udivmod), `>>` picks SHR (logical) over SRS (arithmetic),
// and int -> unsigned mixes follow C's usual arithmetic conversions. Expected
// values come from host g++. Also signed bitfields, which were read without
// sign extension (`int s : 4` holding -1 read 15).
// Not here yet (TODO.md item 11): unsigned <-> float at or above 2^31, the
// display of unsigned words at or above 2^31, 8/16/64-bit integer types and
// bool normalisation. Each joins this test when it lands.
struct BF { int s : 4; unsigned u : 4; int one : 1; int mid : 7; };
union  BU { unsigned raw; BF f; };

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
}
