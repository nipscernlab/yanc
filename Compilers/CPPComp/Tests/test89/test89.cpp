// test89: #IFLIVE keeps the recursion stack when ANY recursive function is
// reached. fact is never called; fib is, through a function pointer. The
// global array comes first, so __cstk does not start at data address 0:
// without the stack set-up (__sp = __cstk) fib's frames would start at 0,
// on top of keep[], and the last line would not be 10.
#pragma yanc prname test89

int keep[4] = {1, 2, 3, 4};

int fact(int n) { if (n < 2) return 1; return n * fact(n - 1); }
int fib(int n)  { if (n < 2) return n; return fib(n - 1) + fib(n - 2); }

void main(void) {
    int (*f)(int) = fib;
    out(0, f(10));                              // 55
    out(0, fib(7));                             // 13
    out(0, keep[0] + keep[1] + keep[2] + keep[3]);   // 10
}
