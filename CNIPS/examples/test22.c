// C99 designated initializers: .field / [index], mixed with positional, with
// a running cursor; unnamed slots stay zero.
#pragma yanc prname tst22
#pragma yanc nubits 16

struct Cfg { int a; int b; int c; int d; };

int arr[6] = { [0]=1, [5]=9, [2]=7 };     // {1,0,7,0,0,9}
struct Cfg g = { .c = 30, .a = 10 };       // {10,0,30,0}

void main(void) {
    out(0, arr[0]);   // 1
    out(0, arr[2]);   // 7
    out(0, arr[5]);   // 9
    out(0, arr[1]);   // 0

    out(0, g.a);      // 10
    out(0, g.b);      // 0
    out(0, g.c);      // 30

    // mixed positional + designator: a=100, then .c=300, then positional d=301
    struct Cfg h = { 100, .c = 300, 301 };
    out(0, h.a);      // 100
    out(0, h.b);      // 0
    out(0, h.c);      // 300
    out(0, h.d);      // 301

    // array: designator then positional continues from there
    int m[5] = { [1]=10, 11, 12 };
    out(0, m[0]);     // 0
    out(0, m[1]);     // 10
    out(0, m[3]);     // 12
    out(0, m[4]);     // 0
}
