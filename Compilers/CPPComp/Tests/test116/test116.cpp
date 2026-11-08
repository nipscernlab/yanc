// A division by zero (#34), which C++ leaves undefined and SAPHO defines
// (docs/reference/instructions.md): an int divided by zero gives -1, or 1
// when the dividend is negative; the remainder is the dividend. An unsigned
// one gives the largest unsigned, which out() shows as -1. The divisor comes
// from in(0), so nothing is folded at compile time, and then as a constant,
// which the compiler folds. Expected, from that definition:
//
// 7 / z, -7 / z, 0 / z      -> -1 1 -1
// 7 % z, -7 % z             -> 7 -7
// INT_MIN / z, INT_MIN % z  -> 1 -2147483648
// 7u / 0u, 7u % 0u          -> -1 7
// 7 / 0, -7 / 0, 7 % 0      -> -1 1 7

int main()
{
    int z = in(0);                         // 0, from input_0.txt
    int m = -2147483647 - 1;
    out(0, 7 / z); out(0, -7 / z); out(0, 0 / z);
    out(0, 7 % z); out(0, -7 % z);
    out(0, m / z); out(0, m % z);
    unsigned u = 7u, uz = (unsigned)z;
    out(0, (int)(u / uz)); out(0, (int)(u % uz));
    out(0, 7 / 0); out(0, -7 / 0); out(0, 7 % 0);
    return 0;
}
