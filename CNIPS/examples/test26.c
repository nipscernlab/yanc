// static local variables: initialised once, persist across calls (unlike an
// auto local, whose initializer re-runs each entry).
#pragma yanc prname tst26
#pragma yanc nubits 16

int next_id(void) {
    static int c = 10;     // init once
    c = c + 1;
    return c;
}

int auto_reset(void) {
    int a = 100;           // re-init every call
    a = a + 1;
    return a;
}

int accum(int x) {
    static int sum = 0;
    sum = sum + x;
    return sum;
}

void main(void) {
    out(0, next_id());     // 11
    out(0, next_id());     // 12
    out(0, next_id());     // 13

    out(0, auto_reset());  // 101
    out(0, auto_reset());  // 101  (auto local resets)

    out(0, accum(5));      // 5
    out(0, accum(7));      // 12
    out(0, accum(3));      // 15
}
