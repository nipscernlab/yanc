#pragma yanc prname test84
// Float constants far from 1 survive the trip to the assembler (TODO 5, step
// 3). cppcomp printed them with %.20f while the assembler's lexer rejected an
// exponent: 1e-25f became "0.00000000000000000000" = 0 and anything above
// ~1e75 overflowed the buffer. Now it prints %.17g ("1e-25") and the lexers
// read the exponent, which yanc_num rounds once. Only robust results are
// printed (compares, +0.5 roundings), not last-bit values.
// Expected outputs: 1 1 1 1 3 7 25 1
void main(void)
{
    int k = in(0);
    float tiny = 1e-25f, big = 1e30f;
    out(0, tiny > 0.0f);                          // 1: was 0
    out(0, tiny < 1e-24f);                        // 1
    out(0, big > 1e29f);                          // 1
    out(0, big * 1e-30f > 0.5f);                  // 1: ~1.0
    out(0, (int)(3.0e-6f * 1e6f + 0.5f));         // 3
    out(0, (int)(tiny * 7e25f + 0.5f));           // 7
    out(0, (int)(2.5e-3f * 1e4f + 0.5f));         // 25
    out(0, k + 1);                                // 1
}
