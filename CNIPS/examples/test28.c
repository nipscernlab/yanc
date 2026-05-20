// C99 compound literals: (T){ init } as an unnamed object — assigned, used as
// an initializer, passed by value as an argument, designated, and scalar.
#pragma yanc prname tst28
#pragma yanc nubits 16

struct P { int x; int y; };

int sumP(struct P p) { return p.x + p.y; }

void main(void) {
    struct P a;
    a = (struct P){10, 20};            // compound literal assigned
    out(0, a.x);                       // 10
    out(0, a.y);                       // 20

    struct P b = (struct P){3, 4};     // as an initializer
    out(0, b.x + b.y);                 // 7

    out(0, sumP((struct P){5, 6}));    // 11  (by-value argument)

    struct P c = (struct P){.y = 99};  // designated
    out(0, c.x);                       // 0
    out(0, c.y);                       // 99

    int n = (int){42};                 // scalar compound literal
    out(0, n);                         // 42
}
