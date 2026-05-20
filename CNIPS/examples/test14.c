// nested struct definitions (inline) + nested member access
#pragma yanc prname tst14
#pragma yanc nubits 16

struct box {
    struct dims { int w; int h; } size;   // struct defined inline inside box
    int id;
};

void main(void) {
    struct box b;
    b.size.w = 4;
    b.size.h = 7;
    b.id = 99;
    out(0, b.size.w);             // 4
    out(0, b.size.h);             // 7
    out(0, b.id);                 // 99
    out(0, b.size.w * b.size.h);  // 28

    // a standalone struct dims (the inline tag is visible afterwards)
    struct dims d;
    d.w = 3; d.h = 5;
    out(0, d.w + d.h);            // 8
}
