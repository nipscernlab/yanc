// Bit-manipulation toolkit (low-level / bit twiddling). Exercises 32-bit
// unsigned bit operations: population count, bit reversal, parity (Brian
// Kernighan), Gray code, and find-first-set.
#pragma yanc prname tst57

int popcount(unsigned int x) {
    int n = 0;
    while (x) { n = n + (int)(x & 1); x = x >> 1; }
    return n;
}

unsigned int bitrev(unsigned int x) {
    unsigned int r = 0;
    for (int i = 0; i < 32; i = i + 1) { r = (r << 1) | (x & 1); x = x >> 1; }
    return r;
}

int parity(unsigned int x) {
    int p = 0;
    while (x) { p = p ^ 1; x = x & (x - 1); }   // clear lowest set bit each step
    return p;
}

unsigned int gray(unsigned int x) { return x ^ (x >> 1); }

int ffs1(unsigned int x) {
    if (x == 0) return 0;
    int i = 1;
    while (!(x & 1)) { i = i + 1; x = x >> 1; }
    return i;
}

void main(void) {
    out(0, popcount(0xFF));          // 8
    out(0, popcount(0xAAAAAAAA));    // 16
    out(0, (int)bitrev(1));          // 0x80000000 -> -2147483648
    out(0, parity(0x7));             // 1
    out(0, parity(0xF));             // 0
    out(0, (int)gray(5));            // 5 ^ 2 = 7
    out(0, ffs1(8));                 // 4
    out(0, ffs1(0));                 // 0
}
