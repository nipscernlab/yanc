// Forms the manual promises that gave an error (#131). Each line printed is
// what C++ gives:
//
// out(0, n.next->v)         -> 3    class Node; declared, then defined: one type
// out(0, x.m[1][2])         -> 7    a field of two dimensions
// out(0, local_sum())       -> 9    a struct defined inside a function
// out(0, local_method())    -> 8    a class with a method, inside a function
// out(0, s[3]), s[5]        -> 100 70  "ab" "cd" "e\x46" is one string, over two lines
// out(0, at(7).x)           -> 7    a member of the struct a call returns
// out(0, use(make()))       -> 15   make()'s result initializes use's parameter
//                                   with no copy (C++17); `return a;` copies once
//                                   (C++ may leave that copy out, and gcc does: 5)
// out(0, b.name()[0])       -> 98   a method that returns const char*
// out(0, local_t())         -> 4    a typedef inside a function
// out(0, k), twice          -> 3 3  two loops of one block both declare k

class Node;
Node *head = 0;
class Node { public: int v; Node *next; };

struct M { int m[2][3]; };

int local_sum(void)
{
    struct P { int a; int b; };
    P p; p.a = 4; p.b = 5;
    return p.a + p.b;
}

int local_method(void)
{
    class C { public: int k; int dbl() { return 2 * k; } };
    C c; c.k = 4;
    return c.dbl();
}

struct Pt { int x; int y; };
Pt at(int v) { Pt p; p.x = v; p.y = v + 1; return p; }

class A { public: int v; A() { v = 1; } A(const A &o) { v = o.v + 10; } };
A make(void) { A a; a.v = 5; return a; }
int use(A a) { return a.v; }

struct B { const char *name() { return "b"; } };

int local_t(void) { typedef int T; T t = 4; return t; }

void main(void)
{
    Node first, second;
    second.v = 3; second.next = 0;
    first.v = 1; first.next = &second;
    head = &first;
    out(0, head->next->v);
    M x; x.m[1][2] = 7;
    out(0, x.m[1][2]);
    out(0, local_sum());
    out(0, local_method());
    const char *s = "ab" "cd"
                    "e\x46";
    out(0, s[3]); out(0, s[5]);
    out(0, at(7).x);
    out(0, use(make()));
    B b;
    out(0, b.name()[0]);
    out(0, local_t());
    int k = 0;
    for (int k = 0; k < 3; k++) { }
    for (int k = 0; k < 3; k++) { }
    k = 3;
    out(0, k); out(0, k);
}
