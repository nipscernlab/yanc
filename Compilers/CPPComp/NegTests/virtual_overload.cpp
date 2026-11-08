// #131: an overloaded virtual method called a label that did not exist
class B { public: virtual int f(int a) { return a; } virtual int f(float a) { return 100; } };
class D : public B { public: int f(int a) { return a + 1; } int f(float a) { return 200; } };
void main(void) { D d; B *p = &d; out(0, p->f(1)); out(0, p->f(1.5f)); }
