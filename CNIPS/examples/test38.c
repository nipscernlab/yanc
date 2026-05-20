// Number-theory toolkit (numerical / integer). Exercises 32-bit signed and
// unsigned division/modulo, loops, the full int range (Fibonacci), and modular
// exponentiation. All outputs are exact, well-known values.
#pragma yanc prname tst38
#pragma yanc nubits 32
#pragma yanc nbmant 23
#pragma yanc nbexpo 8

int isqrt(int n) {
    if (n <= 0) return 0;
    int x = n, y = (x + 1) / 2;
    while (y < x) { x = y; y = (x + n / x) / 2; }
    return x;
}

int gcd(int a, int b) {
    while (b != 0) { int t = b; b = a % b; a = t; }
    return a;
}

int fib(int n) {
    int a = 0, b = 1;
    for (int i = 0; i < n; i = i + 1) { int t = a + b; a = b; b = t; }
    return a;
}

int collatz(int n) {
    int s = 0;
    while (n != 1) {
        if (n % 2 == 0) n = n / 2;
        else            n = 3 * n + 1;
        s = s + 1;
    }
    return s;
}

unsigned int modpow(unsigned int b, unsigned int e, unsigned int m) {
    unsigned int r = 1;
    b = b % m;
    while (e > 0) {
        if (e & 1) r = (r * b) % m;
        e = e >> 1;
        b = (b * b) % m;
    }
    return r;
}

void main(void) {
    out(0, isqrt(2000000));            // 1414
    out(0, isqrt(144));                // 12
    out(0, gcd(1071, 462));            // 21
    out(0, fib(10));                   // 55
    out(0, fib(46));                   // 1836311903 (near 2^31)
    out(0, collatz(27));               // 111
    out(0, (int)modpow(7, 256, 13));   // 9
    out(0, (int)modpow(2, 10, 1000));  // 1024 % 1000 = 24
}
