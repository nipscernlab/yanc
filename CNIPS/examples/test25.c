// nested-brace aggregate initializers: 2D arrays, arrays of structs, nested
// structs, partial inner braces (zero-fill), and a designated nested element.
#pragma yanc prname tst25
#pragma yanc nubits 16

struct P { int x; int y; };
struct Line { struct P a; struct P b; };

int m[2][3] = { {1,2,3}, {4,5,6} };
struct P pts[2] = { {10,20}, {30,40} };
struct Line L = { {1,2}, {3,4} };

void main(void) {
    out(0, m[0][0]);   // 1
    out(0, m[0][2]);   // 3
    out(0, m[1][0]);   // 4
    out(0, m[1][2]);   // 6

    out(0, pts[0].x);  // 10
    out(0, pts[1].y);  // 40

    out(0, L.a.x);     // 1
    out(0, L.a.y);     // 2
    out(0, L.b.x);     // 3
    out(0, L.b.y);     // 4

    int q[2][3] = { {7}, {8,9} };   // partial inner braces -> zero-fill
    out(0, q[0][0]);   // 7
    out(0, q[0][1]);   // 0
    out(0, q[1][0]);   // 8
    out(0, q[1][1]);   // 9

    struct P d[2] = { [1] = { .y = 99 } };   // designated nested element
    out(0, d[0].x);    // 0
    out(0, d[1].y);    // 99
    out(0, d[1].x);    // 0
}
