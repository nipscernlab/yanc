// Arbitrary-precision factorial (big-number arithmetic). The value can't fit in
// one 32-bit word, so it's stored as an array of decimal digits (little-endian)
// and grown by repeated multiply-with-carry. Exercises carry propagation and
// integer division/modulo. 20! = 2432902008176640000 (19 digits).
#pragma yanc prname tst56
#define D 40

int digits[D];
int ndig;

void bn_set(int v) {
    ndig = 0;
    if (v == 0) { digits[0] = 0; ndig = 1; return; }
    while (v > 0) { digits[ndig] = v % 10; ndig = ndig + 1; v = v / 10; }
}

void bn_mul(int m) {
    int carry = 0;
    for (int i = 0; i < ndig; i = i + 1) {
        int p = digits[i] * m + carry;
        digits[i] = p % 10;
        carry = p / 10;
    }
    while (carry > 0) { digits[ndig] = carry % 10; ndig = ndig + 1; carry = carry / 10; }
}

void main(void) {
    bn_set(1);
    for (int i = 2; i <= 20; i = i + 1) bn_mul(i);

    out(0, ndig);                                  // 19
    for (int i = ndig - 1; i >= 0; i = i - 1)      // digits, most significant first
        out(0, digits[i]);                         // 2 4 3 2 9 0 2 0 0 8 1 7 6 6 4 0 0 0 0
}
