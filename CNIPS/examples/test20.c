// struct assignment by value: whole-struct copy (a = b), independence of the
// copy, and chained assignment c = b = a (nested copy temporaries).
#pragma yanc prname tst20
#pragma yanc nubits 16

struct P { int x; int y; int z; };

void main(void) {
    struct P a;
    a.x = 1; a.y = 2; a.z = 3;

    struct P b;
    b = a;                 // whole-struct copy
    out(0, b.x);           // 1
    out(0, b.y);           // 2
    out(0, b.z);           // 3

    b.x = 99;
    out(0, a.x);           // 1  (a is independent of b)

    struct P c;
    a.x = 7;
    c = b = a;             // chained: both end up as a's current values
    out(0, c.x);           // 7
    out(0, b.x);           // 7
}
