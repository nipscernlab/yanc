#pragma yanc prname test69
// A tagged struct is a class: methods, constructors, a base, access labels, a
// nested struct with a method, a template. C++ tells struct and class apart
// only by the default access; cppcomp accepted nothing but fields in a struct.
// The C uses keep working: a self-referential struct, a typedef'd anonymous
// struct. Constructors run here only where cppcomp already calls them (local
// objects and `new`); TODO.md item 11 lists the rest.
struct Point {
    int x, y;
    Point() { x = 0; y = 0; }
    Point(int a, int b) { x = a; y = b; }
    int sum() { return x + y; }
    void scale(int k) { x = x * k; y = y * k; }
};

struct Point3 : Point {
    int z;
    Point3(int a, int b, int c) : Point(a, b) { z = c; }
    int sum3() { return sum() + z; }
};

class Shape {
public:
    struct Box { int w, h; int area() { return w * h; } };
    Box b;
    int area() { return b.area(); }
};

template <class T> struct Pair {
    T first, second;
    T larger() { return first > second ? first : second; }
};

struct Counter {
private:
    int n;
public:
    Counter() { n = 0; }
    void inc() { n = n + 1; }
    int get() { return n; }
};

struct Node { int v; struct Node *next; };
typedef struct { int a; int b; } AB;

void main(void) {
    Point p;             out(0, p.sum());            // 0
    Point q(3, 4);       out(0, q.sum());            // 7
    q.scale(2);          out(0, q.x);                // 6
    Point3 r(1, 2, 3);   out(0, r.sum3());           // 6
    Shape s; s.b.w = 4; s.b.h = 5;
    out(0, s.area());                                // 20
    Pair<int> pi; pi.first = 9; pi.second = 4;
    out(0, pi.larger());                             // 9
    Pair<float> pf; pf.first = 1.5f; pf.second = 2.5f;
    out(0, (int)(pf.larger() * 2.0f));               // 5
    Counter c; c.inc(); c.inc();
    out(0, c.get());                                 // 2
    Node n2; n2.v = 2; n2.next = 0;
    Node n1; n1.v = 1; n1.next = &n2;
    out(0, n1.next->v);                              // 2
    AB ab; ab.a = 5; ab.b = 6;
    out(0, ab.a * ab.b);                             // 30
    Point *hp = new Point(7, 8);
    out(0, hp->sum());                               // 15
}
