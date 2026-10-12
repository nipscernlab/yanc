// test85: code no path reaches costs no hardware (asm_reach, asm_share.h).
// Both headers are included and nothing of them is used: their inline
// functions and <vector>'s heap (the 2048-word arena and its set-up in main)
// leave the program. fdead and mdead are never called: no float divider, no
// multiplier. What a virtual call or a function pointer reaches stays, since
// both are chains of direct CALs. hardware.txt holds the hardware to that.
// Also the function-pointer declarator with an initializer, global and local,
// and `&f` (it gave 0: a LEA of the function's name as if it were data).
#pragma yanc prname test85
#include <cmath>
#include <vector>

float fdead(float u, float v) { return u / v; }
int   mdead(int u, int v)     { return u * v; }

class Shape {
public:
    virtual int grow(int s) { return s + 1; }
};
class Twice : public Shape {
public:
    int grow(int s) { return s + s; }
};

int add3(int u) { return u + 3; }
int sub2(int u) { return u - 2; }

int (*gfp)(int) = sub2;

void main(void) {
    Twice t;
    Shape* p = &t;
    out(0, p->grow(5));        // 10  (virtual: Twice::grow)
    int (*fp)(int) = add3;
    out(0, fp(4));             // 7   (local pointer, initialized)
    out(0, gfp(9));            // 7   (global pointer, initialized)
    int (*fq)(int) = &sub2;
    out(0, fq(20));            // 18  (&f is f)
    out(0, add3(1));           // 4
}
