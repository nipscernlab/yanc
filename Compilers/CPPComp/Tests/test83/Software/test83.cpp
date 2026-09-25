#pragma yanc prname test83
// Constant folding (codegen.c cfold): an int expression of literals only is
// computed at compile time in the word's arithmetic -- 32-bit two's
// complement, wrapping; / and % truncate; >> of a signed int is arithmetic.
// Division by zero, INT_MIN / -1 and out-of-range shifts are left to the ALU;
// unsigned literals are not folded. Every line is an expression the folder
// sees; the same code on host gcc -fwrapv gives the expected values.
constexpr int L = 15, FL = 29;

void main(void)
{
    int k = in(0);                              // 0: keeps the runtime path alive
    out(0, L >> 1);                             // 7
    out(0, FL - 1);                             // 28
    out(0, 3 - 5);                              // -2
    out(0, 2147483647 + 1);                     // wraps to INT_MIN
    out(0, -7 / 2);                             // -3
    out(0, -7 % 2);                             // -1
    out(0, (1 << 31) >> 4);                     // arithmetic: -134217728
    out(0, (0 - 2147483647 - 1) / -1);          // INT_MIN / -1: the ALU's wrap
    out(0, (5 < 3) + (5 > 3) * 2 + (5 == 5) * 4 + (5 != 5) * 8);   // 6
    out(0, (2 && 0) + (0 || 3) * 2 + !0 * 4 + ~5);                 // 2 + 4 - 6 = 0
    out(0, 100000 * 100000);                    // wraps: 1410065408
    out(0, k < FL - 1);                         // 1: a folded bound in a compare
    out(0, (int)(3000000000u / 2u));            // unsigned: not folded, 1500000000
}
