// function pointers (dispatch-table calls)
#pragma yanc prname tst10
#pragma yanc nubits 16

int add1(int x) { return x + 1; }
int dbl (int x) { return x + x; }
int sq  (int x) { return x * x; }

// callback: apply a function pointer to a value
int apply(int (*f)(int), int v) {
    return f(v);
}

void main(void) {
    int (*fp)(int);

    fp = add1;
    out(0, fp(10));            // 11
    out(0, (*fp)(20));         // 21  (explicit-deref call form)

    fp = dbl;
    out(0, fp(7));             // 14

    fp = sq;
    out(0, fp(6));             // 36

    // pass a function pointer to another function
    out(0, apply(add1, 100));  // 101
    out(0, apply(sq, 9));      // 81
}
