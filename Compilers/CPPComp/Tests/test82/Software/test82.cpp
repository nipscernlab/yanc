#pragma yanc prname test82
// The zero-fill of a local brace initializer stores four words a turn from
// 16 words up (emit_zero_words): the n % 4 lowest words singly, then groups of
// four top-down. One array per remainder (0..3) and per starting word, int
// and float, around a guard word; probe() runs twice with garbage written in
// between, so only a fill that covers every word -- and nothing next to it --
// prints the same line twice.
// Expected outputs: 1111111 1111111
int probe(int dirt)
{
    int   g1 = 11;
    int   a[16] = {};              // words 0..15:  n 16, remainder 0
    int   b[18] = {5};             // words 1..17:  n 17, remainder 1
    float f[21] = {1.0f, 2.0f};    // words 2..20:  n 19, remainder 3
    int   d[21] = {1, 2, 3};       // words 3..20:  n 18, remainder 2
    int   g2 = 22;

    int sa = 0, sb = 0, sd = 0, zf = 1;
    for (int k = 0; k < 16; ++k) sa += a[k];
    for (int k = 0; k < 18; ++k) sb += b[k];
    for (int k = 0; k < 21; ++k) sd += d[k];
    for (int k = 2; k < 21; ++k) if (!(f[k] == 0.0f)) zf = 0;

    int r = 0;
    r = r * 10 + (sa == 0);
    r = r * 10 + (sb == 5 && b[0] == 5);
    r = r * 10 + (sd == 6 && d[2] == 3);
    r = r * 10 + (zf && f[0] == 1.0f && f[1] == 2.0f);
    r = r * 10 + (g1 == 11);
    r = r * 10 + (g2 == 22);
    r = r * 10 + (a[15] == 0 && b[17] == 0 && d[20] == 0);
    out(0, r);

    for (int k = 0; k < 16; ++k) a[k] = dirt;
    for (int k = 0; k < 18; ++k) b[k] = dirt;
    for (int k = 0; k < 21; ++k) { d[k] = dirt; f[k] = 3.5f; }
    g1 = dirt; g2 = dirt;
    return r;
}

void main(void)
{
    probe(1000);
    probe(2000);
}
