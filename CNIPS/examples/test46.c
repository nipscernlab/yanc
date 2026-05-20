// Monte Carlo estimate of pi (numerical / probabilistic). Exercises a 32-bit
// xorshift PRNG, unsigned bit ops, int->float conversion, and a float loop.
// Deterministic seed -> deterministic count -> deterministic pi estimate.
#pragma yanc prname tst46

unsigned int rng = 12345;

unsigned int xs(void) {
    unsigned int x = rng;
    x = x ^ (x << 13);
    x = x ^ (x >> 17);
    x = x ^ (x << 5);
    rng = x;
    return x;
}

float frand(void) {
    return (float)(xs() & 0xFFFF) / 65536.0;   // [0,1), mask is cheap (no divider)
}

void main(void) {
    int inside = 0;
    int N = 1000;
    for (int i = 0; i < N; i = i + 1) {
        float x = frand();
        float y = frand();
        if (x * x + y * y <= 1.0) inside = inside + 1;
    }
    out(0, inside);                                       // deterministic hit count
    out(0, (int)(4.0 * (float)inside / (float)N * 1000.0)); // ~3140 (pi*1000)
}
