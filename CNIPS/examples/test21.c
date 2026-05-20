// struct by value: parameters (callee mutation must not affect caller) and
// return-by-value, including struct-returning calls nested as arguments.
#pragma yanc prname tst21
#pragma yanc nubits 16

struct V { int a; int b; };

int sumv(struct V p) {
    p.a = p.a + 100;          // local mutation, must be discarded
    return p.a + p.b;
}

struct V make(int x, int y) {
    struct V r;
    r.a = x;
    r.b = y;
    return r;                 // return by value
}

struct V bump(struct V p) {
    p.a = p.a + 1;
    p.b = p.b + 1;
    return p;
}

void main(void) {
    struct V v;
    v.a = 3; v.b = 4;
    out(0, sumv(v));          // (3+100)+4 = 107
    out(0, v.a);              // 3  (caller's copy untouched)

    struct V w = make(10, 20);
    out(0, w.a);              // 10
    out(0, w.b);              // 20

    struct V z;
    z = bump(make(5, 6));     // make->{5,6}; bump->{6,7}
    out(0, z.a);              // 6
    out(0, z.b);              // 7
}
