// Recursion (#128): each line printed is what C++ gives.
//
// out(0, a.sum())     -> 111  a recursive method, next->sum(), over the list
//                             1, 10, 100: its `this` and locals need a frame
// out(0, fact(5))     -> 120  a recursive function template
// out(0, f(q, 2))     -> 11   a struct passed by value to a recursive function:
//                             3 + 4 + 4

struct Node {
    int v;
    Node *next;
    int sum() { int r = 0; if (next) r = next->sum(); return v + r; }
};

template <typename T> T fact(T n)
{
    if (n <= 1) return 1;
    return n * fact(n - 1);
}

struct P { int a; int b; };

int f(P p, int n)
{
    if (n == 0) return p.a;
    return f(p, n - 1) + p.b;
}

void main(void)
{
    Node c; c.v = 100; c.next = 0;
    Node b; b.v = 10;  b.next = &c;
    Node a; a.v = 1;   a.next = &b;
    out(0, a.sum());
    out(0, fact(5));
    P q; q.a = 3; q.b = 4;
    out(0, f(q, 2));
}
