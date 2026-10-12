// References (T&): swap/inc by reference, a reference local, ref-to-global,
// and a class reference used for method + member access.
#pragma yanc prname test6
void swap(int& a, int& b) { int t = a; a = b; b = t; }
void inc(int& x) { x = x + 1; }

class Box { public: int v; int get() { return v; } };
int peek(Box& b) { return b.get() + b.v; }   // method + member through a class ref

int g;
void main(void) {
    int p = 3, q = 9;
    swap(p, q);
    out(0, p);     // 9
    out(0, q);     // 3

    int n = 5;
    int& ref = n;  // reference local binds to n
    ref = 42;      // writes through to n
    out(0, n);     // 42
    ref = ref + 1;
    out(0, n);     // 43

    g = 10;
    inc(g);
    out(0, g);     // 11

    Box bx; bx.v = 7;
    out(0, peek(bx));  // 7 + 7 = 14
}
