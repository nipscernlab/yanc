#pragma yanc prname test80
// Loop-invariant address hoisting (codegen.c, lih_*): in a `for`, p[inv + k]
// with p and the variables of inv not written in the loop becomes t[k], with
// t = p + inv computed once before it. The cases that must NOT be hoisted
// are the point of this test: each one changes inv (or p) inside the loop in
// a way a hoist would miss, so a wrong hoist prints a different number.
// Expected values from the same code compiled on the host (gcc).
int  M[64];
int  G = 3;

void bump(int& v) { v = v + 1; }

int sum_plain(int* a, int n, int i)          // hoisted: a + i*n
{
    int s = 0;
    for (int k = 0; k < n; ++k) s += a[i * n + k];
    return s;
}
int sum_written(int* a, int n, int i)        // i changes in the loop: not hoisted
{
    int s = 0;
    for (int k = 0; k < n; ++k) { s += a[i * n + k]; if (k == 2) i = i + 1; }
    return s;
}
int sum_pointer(int* a, int n, int i)        // i written through a pointer
{
    int s = 0; int* pi = &i;
    for (int k = 0; k < n; ++k) { s += a[i * n + k]; *pi = *pi + (k == 1); }
    return s;
}
int sum_ref(int* a, int n, int i)            // i bumped by reference in the loop
{
    int s = 0;
    for (int k = 0; k < n; ++k) { s += a[i * n + k]; if (k == 3) bump(i); }
    return s;
}
int sum_base(int* a, int n, int i)           // the base pointer moves in the loop
{
    int s = 0;
    for (int k = 0; k < n; ++k) { s += a[i * n + k]; a = a + 1; }
    return s;
}
int sum_empty(int* a, int n, int i)          // zero-trip loop: the hoist runs, harmlessly
{
    int s = 7;
    for (int k = 0; k < 0; ++k) s += a[i * n + k];
    return s;
}
int ident(int v) { return v; }
int sum_byval(int* a, int n, int i)          // i passed BY VALUE in the loop: hoisted
{
    int s = 0;
    for (int k = 0; k < n; ++k) s += a[i * n + k] + ident(i);
    return s;
}
int sum_lone(int* a, int n, int i)           // a lone invariant variable: a + i hoisted
{
    int s = 0;
    for (int k = 0; k < n; ++k) s += a[k + i];
    return s;
}
int sum_nested(int* a, int n)                // a + j*n hoisted out of both loops
{
    int s = 0;
    for (int j = 0; j < 2; ++j)
        for (int r = 0; r < 3; ++r)
            for (int k = 0; k < n; ++k) s += a[j * n + k] * (r + 1);
    return s;
}

void main(void)
{
    for (int q = 0; q < 64; ++q) M[q] = q * q % 17 + G;
    out(0, sum_plain  (M, 6, 2));
    out(0, sum_written(M, 6, 2));
    out(0, sum_pointer(M, 6, 2));
    out(0, sum_ref    (M, 6, 2));
    out(0, sum_base   (M, 6, 2));
    out(0, sum_empty  (M, 6, 2));
    out(0, sum_nested (M, 6));
    out(0, sum_byval  (M, 6, 2));
    out(0, sum_lone   (M, 6, 5));
}
