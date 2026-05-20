// CRC-32 and xorshift32 PRNG (hashing / pseudo-random). Exercises 32-bit
// unsigned arithmetic, logical shifts, XOR, bitwise-not, string literals, and
// static state. CRC-32("123456789") has the well-known value 0xCBF43926.
#pragma yanc prname tst33
#pragma yanc nubits 32
#pragma yanc nbmant 23
#pragma yanc nbexpo 8

unsigned int crc32(char *s, int len) {
    unsigned int crc = 0xFFFFFFFF;
    for (int i = 0; i < len; i = i + 1) {
        crc = crc ^ (unsigned int)s[i];
        for (int j = 0; j < 8; j = j + 1) {
            unsigned int m = -(crc & 1);          // 0x00000000 or 0xFFFFFFFF
            crc = (crc >> 1) ^ (0xEDB88320 & m);
        }
    }
    return ~crc;
}

unsigned int rng_state = 2463534242;

unsigned int xorshift32(void) {
    unsigned int x = rng_state;
    x = x ^ (x << 13);
    x = x ^ (x >> 17);
    x = x ^ (x << 5);
    rng_state = x;
    return x;
}

void main(void) {
    out(0, (int)crc32("123456789", 9));   // 0xCBF43926 -> -873187034
    out(0, (int)crc32("", 0));            // 0 (empty)

    out(0, (int)xorshift32());            // deterministic PRNG sequence
    out(0, (int)xorshift32());
    out(0, (int)xorshift32());

    // a value in [0,99] via unsigned modulo
    out(0, (int)(xorshift32() % 100u));
}
