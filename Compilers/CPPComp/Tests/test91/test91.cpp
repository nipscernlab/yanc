// test91: four C++ rules cppcomp got wrong, found while writing the manual
// (2026-10-13). Before the fixes this printed 0 0 20 0 0 10 and 6 failed to
// compile. Expected (hand-computed):
//   p + 1 and p + 2 over 3-word structs step whole elements: 20, 30; p++: 20
//   range-for over a std::vector visits every element: 1 + 2 + 3 = 6
//   delete[] runs each element's destructor: 3
//   010 is octal: 8
//   a // comment after a #define is not part of it: K + 1 = 6
#include <vector>

#define K 5 // five, not "5 // five"

struct P { int a; int b; int c; };
P arr[3];

int count = 0;
struct D { int x; ~D() { count++; } };

void main(void)
{
    arr[0].a = 10; arr[1].a = 20; arr[2].a = 30;
    P *p = arr;
    P *q = p + 1;
    out(0, q->a);
    out(0, (p + 2)->a);
    p++;
    out(0, p->a);

    std::vector<int> v;
    v.push_back(1); v.push_back(2); v.push_back(3);
    int s = 0;
    for (int x : v) s += x;
    out(0, s);

    D *d = new D[3];
    delete[] d;
    out(0, count);

    out(0, 010);
    out(0, K + 1);
}
