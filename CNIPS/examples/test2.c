// pointers + struct + switch + typedef
#pragma yanc prname tst2
#pragma yanc nubits 16

typedef int word;

struct point { int x; int y; };

// double via pointer
void dbl(word *p) {
    *p = *p * 2;
}

// returns x+y of a point passed by pointer
int sum_xy(struct point *q) {
    return q->x + q->y;
}

void main(void) {
    word v = 7;
    dbl(&v);
    out(0, v);                  // expect 14

    struct point p;
    p.x = 3; p.y = 5;
    out(0, sum_xy(&p));         // expect 8

    int n = 3;
    switch (n) {
        case 1: out(0, 100); break;
        case 2: out(0, 200); break;
        case 3: out(0, 300); break;
        default: out(0, 999); break;
    }
    // expect 300

    // ternary + comma
    int t = (n > 2) ? 42 : 0;
    out(0, t);                  // expect 42
}
