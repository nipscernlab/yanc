// Names and scopes (#128): each line printed is what C++ gives, and what
// cppcomp gave before is in the comment.
//
// out(0, x)            -> 1    an inner block's x hid the outer one (was 2)
// out(0, k + s)        -> 8    a for's k hid the outer k = 5 (was 6)
// out(0, p.x)          -> 7    P(int x) { this->x = x; } (was garbage)
// out(0, s.f())        -> 7    a local v in a nested block hid the member (was 1)
// out(0, t.get())      -> 12   a[n] with a member n and a global n (was 10)
// out(0, t.add(100))   -> 102  x + n, the same (was 100)
// out(0, t.n), out(0, n) -> 5, 10   n = 5 in a method writes the member
// out(0, u.get(1))     -> 6    an inlined method reads the global g, not the
//                              caller's local g (was 101)

int n = 10;
int g = 5;

struct P { int x; P(int x) { this->x = x; } };

struct S {
    int v = 1;
    int f() { int r = 0; int v = 7; if (v > 0) { r = v; } return r; }
};

struct T {
    int n = 2;
    int a[4];
    T() { a[0] = 10; a[1] = 11; a[2] = 12; a[3] = 13; }
    int get() { return a[n]; }
    int add(int x) { return x + n; }
    void setn() { n = 5; }
};

struct U { int get(int x) { return x + g; } };

void main(void)
{
    int x = 1;
    { int x = 2; x = x + 1; }
    out(0, x);

    int k = 5, s = 0;
    for (int k = 0; k < 3; k++) s += k;
    out(0, k + s);

    P p(7);
    out(0, p.x);

    S sv;
    out(0, sv.f());

    T t;
    out(0, t.get());
    out(0, t.add(100));
    t.setn();
    out(0, t.n);
    out(0, n);

    U u;
    { int g = 100; out(0, u.get(1)); g = g + 1; }
}
