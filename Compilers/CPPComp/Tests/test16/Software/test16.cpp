// Base-class initialization in the member list: Derived(x,y) : Base(x), d(y) {}
#pragma yanc prname test16
class Base {
public:
    int b;
    Base(int x) { b = x; }
};
class Derived : public Base {
public:
    int d;
    Derived(int x, int y) : Base(x), d(y) {}   // base-class init + member init
    int total() { return b + d; }
};
void main(void) {
    Derived* p = new Derived(10, 5);
    out(0, p->b);          // 10 (set by Base's ctor)
    out(0, p->d);          // 5
    out(0, p->total());    // 15
    Derived v(3, 4);       // stack construction
    out(0, v.total());     // 7
}
