// Mini-STL written in the language itself: a templated Vector<T> (heap-backed,
// RAII, grow-on-push, operator[]) and a unique_ptr<T> (RAII owning pointer).
#pragma yanc prname test21
#include "ministl.hpp"

void main(void) {
    Vector v;                 // type-erased Vector<int>, default-constructed
    v.push(10);
    v.push(20);
    v.push(30);
    v.push(40);
    v.push(50);               // forces a grow (cap 4 -> 8)
    out(0, v.size());         // 5
    out(0, v[0]);             // 10
    out(0, v[4]);             // 50
    int sum = 0;
    for (int i = 0; i < v.size(); i = i + 1) sum = sum + v.get(i);
    out(0, sum);              // 150
    v.set(2, 99);
    out(0, v[2]);             // 99

    {
        unique_ptr up(new int);   // owns a heap int
        int* raw = up.get();
        *raw = 42;
        out(0, *raw);             // 42
    }                             // up's destructor frees the int here
    out(0, 7);                    // 7 (reached after the smart pointer's scope)
}                             // v's destructor frees the heap buffer
