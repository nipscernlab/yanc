// Calls and objects (#128): each line printed is what C++ gives.
//
// out(0, p->f(p->g(5)))   -> 207  a virtual call in the argument of another
// out(0, k + q->h())      -> 46   a virtual call with a default argument
// out(0, r->f())          -> 1    B b = d keeps B's vtable (slicing)
// out(0, s->f())          -> 1    b = d, the same through an assignment
// out(0, 1)               -> 1    delete of a null pointer runs no destructor
//                                 (a 99 before it would be wrong)
// out(0, cnt)             -> 4    { C arr[3]; C one; } runs 4 destructors
// out(0, cnt)             -> 6    an array of 2 left by a return: 2 more
// out(0, gg), counter()   -> 1, 2 int gg = counter(); keeps counter's static

struct B { virtual int f(int x) { return x; } virtual int g(int x) { return x; } };
struct D : B { int f(int x) { return x + 200; } int g(int x) { return x + 2; } };

struct B2 { virtual int h(int a = 5) { return a; } };
struct D2 : B2 { int h(int a = 5) { return a + 40; } };

struct B3 { virtual int f() { return 1; } };
struct D3 : B3 { int f() { return 2; } };

struct A { ~A() { out(0, 99); } };

int cnt = 0;
struct C { ~C() { cnt = cnt + 1; } };

int leave()
{
    C a2[2];
    return 0;
}

int counter()
{
    static int c = 0;
    c = c + 1;
    return c;
}

int gg = counter();

void main(void)
{
    D d; B *p = &d;
    out(0, p->f(p->g(5)));

    D2 d2; B2 *q = &d2; int k = 1;
    out(0, k + q->h());

    D3 d3;
    B3 b3 = d3; B3 *r = &b3;
    out(0, r->f());
    B3 b4; b4 = d3; B3 *s = &b4;
    out(0, s->f());

    A *pa = 0;
    delete pa;
    out(0, 1);

    { C arr[3]; C one; }
    out(0, cnt);
    leave();
    out(0, cnt);

    out(0, gg);
    out(0, counter());
}
