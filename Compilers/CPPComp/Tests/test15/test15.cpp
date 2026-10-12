// Stack construction with arguments T v(a,b) and a virtual destructor
// (delete via a base pointer runs the derived destructor).
#pragma yanc prname test15
int dlog;

class Vec {
public:
    int x, y;
    Vec(int a, int b) { x = a; y = b; }       // ctor with args
    int sum() { return x + y; }
};

class Base {
public:
    virtual ~Base() { dlog = 100; }           // virtual destructor
    virtual int who() { return 1; }
};
class Derived : public Base {
public:
    ~Derived() { dlog = 200; }                // overrides the virtual dtor
    int who() { return 2; }
};

void main(void) {
    Vec v(3, 4);                 // stack construction with arguments
    out(0, v.sum());             // 7
    out(0, v.x);                 // 3

    dlog = 0;
    Base* p = new Derived;
    out(0, p->who());            // 2  (virtual)
    delete p;                    // virtual dtor -> Derived::~Derived
    out(0, dlog);                // 200
}
