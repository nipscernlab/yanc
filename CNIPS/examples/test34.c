// Hamming(7,4) single-error-correcting code (communications). Exercises
// bitfields, a union for type-punning (word <-> bits), XOR and shifts, and
// passing a union by value. Verifies that every single-bit error is corrected.
#pragma yanc prname tst34
#pragma yanc nubits 32
#pragma yanc nbmant 23
#pragma yanc nbexpo 8

// codeword bit layout (position 1..7): p1 p2 d1 p3 d2 d3 d4
struct bits7 { unsigned p1:1, p2:1, d1:1, p3:1, d2:1, d3:1, d4:1; };
union cw { unsigned w; struct bits7 b; };

unsigned encode(int d1, int d2, int d3, int d4) {
    union cw c;
    c.w = 0;
    c.b.d1 = d1; c.b.d2 = d2; c.b.d3 = d3; c.b.d4 = d4;
    c.b.p1 = d1 ^ d2 ^ d4;     // parity over positions 1,3,5,7
    c.b.p2 = d1 ^ d3 ^ d4;     //                       2,3,6,7
    c.b.p3 = d2 ^ d3 ^ d4;     //                       4,5,6,7
    return c.w;
}

int syndrome(union cw c) {
    int s1 = c.b.p1 ^ c.b.d1 ^ c.b.d2 ^ c.b.d4;
    int s2 = c.b.p2 ^ c.b.d1 ^ c.b.d3 ^ c.b.d4;
    int s3 = c.b.p3 ^ c.b.d2 ^ c.b.d3 ^ c.b.d4;
    return s1 + s2 * 2 + s3 * 4;   // 0 = clean, else 1-based error position
}

int decode(unsigned word) {
    union cw c;
    c.w = word;
    int e = syndrome(c);
    if (e != 0) c.w = c.w ^ (1u << (e - 1));   // flip the bad bit
    return c.b.d1 | (c.b.d2 << 1) | (c.b.d3 << 2) | (c.b.d4 << 3);
}

void main(void) {
    int data = 1 | (0 << 1) | (1 << 2) | (1 << 3);   // d=1011 -> 13
    unsigned enc = encode(1, 0, 1, 1);

    out(0, (int)enc);          // the 7-bit codeword (deterministic)
    out(0, decode(enc));       // 13 (no error)

    int allok = 1;
    for (int p = 1; p <= 7; p = p + 1) {
        unsigned bad = enc ^ (1u << (p - 1));   // single-bit error at position p
        if (decode(bad) != data) allok = 0;
    }
    out(0, allok);             // 1 — every single-bit error corrected
}
