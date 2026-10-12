#pragma yanc prname test27
// Overloaded base-class constructors selected from a derived class's member-
// initializer list: `: Base()` and `: Base(a)` must each dispatch to the right
// constructor overload (a label, not the bare mangled name).

class Base {
protected:
    int v;
public:
    Base() : v(100) {}        // default
    Base(int x) : v(x) {}     // converting
    int get() { return v; }
};

class Derived : public Base {
    int w;
public:
    Derived() : Base(), w(1) {}              // selects Base()
    Derived(int a, int b) : Base(a), w(b) {} // selects Base(int)
    int sum() { return get() + w; }
};

void main(void) {
    Derived d1(10, 5);
    out(0, d1.sum());     // 10 + 5 = 15

    Derived d2;
    out(0, d2.get());     // 100  (default base ctor ran)
    out(0, d2.sum());     // 100 + 1 = 101
}
