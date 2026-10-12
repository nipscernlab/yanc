#pragma yanc prname test81
// The `for` tests its condition at the bottom now (codegen.c S_FOR +
// gen_jump_true): once on entry, then after the step, jumping back when it
// holds. Each case below would print a different number if the bottom test
// had the wrong sense, skipped a turn, ran one too many, or broke `continue`
// / `break`. An int literal bound also takes the memory form (a <= c tests
// a < c+1), except at the end of the range, where c+1 would overflow.
// Expected values from the same code compiled on the host (gcc -fwrapv: a
// SAPHO word wraps at 32 bits, as the last case relies on).
int calls = 0;
int bump(int v) { calls = calls + 1; return v; }

void main(void)
{
    int s, k;

    s = 0; for (k = 0; k < 10; k = k + 1) s += k;                       out(0, s);  // 45
    s = 0; for (k = 3; k <= 7; ++k) s += k;                             out(0, s);  // 25
    s = 0; for (k = 9; k >= 2; --k) s += k;                             out(0, s);  // 44
    s = 0; for (k = 9; k > 2; --k) s += k;                              out(0, s);  // 42
    s = 0; for (k = 0; k != 6; ++k) s += k;                             out(0, s);  // 15
    s = 0; for (k = 0; k == 0; ++k) s += 7;                             out(0, s);  // 7
    s = 0; for (k = 5; k < 5; ++k) s += 100;                            out(0, s + k); // 5: zero trips
    s = 0; for (k = 0; k < 10; ++k) { if (k % 3 == 0) continue; s += k; } out(0, s); // 27
    s = 0; for (k = 0; k < 10; ++k) { if (k == 6) break; s += k; }      out(0, s);  // 15
    s = 0; for (k = 0; k < 8 && s < 10; ++k) s += k;                    out(0, s);  // 10
    s = 0; for (k = 0; k < 3 || s < 20; ++k) s += 4;                    out(0, s);  // 20
    s = 0; calls = 0; for (k = 0; bump(k) < 4; ++k) s += 1;             out(0, s * 10 + calls); // 45
    int n = 4, j;
    s = 0; for (k = 0; k < n; ++k) for (j = k; j < n; ++j) s += j;      out(0, s);  // 20
    float f; int c = 0;
    for (f = 0.5f; f < 3.0f; f = f + 0.5f) c = c + 1;                   out(0, c);  // 5
    int big = 2147483646; c = 0;
    for (k = big; k <= 2147483647 && c < 3; ++k) c = c + 1;             out(0, c);  // 3: ++k wraps to INT_MIN, c < 3 stops it
}
