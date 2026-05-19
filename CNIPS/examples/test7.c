// aggregate initializers
#pragma yanc prname tst7
#pragma yanc nubits 16

struct point { int x; int y; int z; };

int g[3] = {100, 200, 300};      // global array initializer

void main(void) {
    int a[4] = {5, 6, 7, 8};      // local array initializer
    out(0, a[0]);                 // 5
    out(0, a[3]);                 // 8

    struct point p = {11, 22, 33};
    out(0, p.x);                  // 11
    out(0, p.y);                  // 22
    out(0, p.z);                  // 33

    out(0, g[1]);                 // 200 (global init)

    // partial init: remaining slots keep their .mif default (0)
    int b[3] = {9};
    out(0, b[0]);                 // 9
    out(0, b[2]);                 // 0
}
