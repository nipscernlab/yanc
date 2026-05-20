// Recursion (hybrid stack frames). These functions form call-graph cycles, so
// the compiler gives them software stack frames; non-recursive functions keep
// fast fixed-address locals. The return-address stack must be deep enough.
#pragma yanc prname tst59
#pragma yanc sdepth 128    // return-address stack: >= max recursion depth
#pragma yanc ndstac 128    // data stack: pending operands accumulate per level

int fact(int n) {
    if (n <= 1) return 1;
    return n * fact(n - 1);
}

int fib(int n) {
    if (n < 2) return n;
    return fib(n - 1) + fib(n - 2);
}

int gcd(int a, int b) {
    if (b == 0) return a;
    return gcd(b, a % b);          // tail recursion
}

int ack(int m, int n) {           // Ackermann — deeply nested recursion
    if (m == 0) return n + 1;
    if (n == 0) return ack(m - 1, 1);
    return ack(m - 1, ack(m, n - 1));
}

void main(void) {
    out(0, fact(5));      // 120
    out(0, fact(10));     // 3628800
    out(0, fib(10));      // 55
    out(0, fib(15));      // 610
    out(0, gcd(48, 36));  // 12
    out(0, ack(2, 3));    // 9
    out(0, ack(3, 2));    // 29
}
