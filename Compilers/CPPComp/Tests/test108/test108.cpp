// Forms that compiled and that no test ran (#134). Each line printed is what
// C++ gives:
//
// out(0, k)              -> 3     goto out of a loop, at k == 3
// out(0, CAT(1, 2))      -> 12    ## pastes two tokens into one number
// out(0, SUM3(1, 2, 3))  -> 6     __VA_ARGS__ passes the rest of the call
// out(0, FIRST(7))       -> 7     , ##__VA_ARGS__ drops the comma when empty
// out(0, once_count)     -> 0     once.h included twice: #pragma once, one copy
// out(0, d->who())       -> 2     dynamic_cast to the derived class
// out(0, *q)             -> 5     reinterpret_cast of an int pointer
// out(0, sizeof(S))      -> 2     static_assert that holds compiles to nothing
//
// It also checks two warnings (warnings.txt): #warning, and an unknown
// #pragma yanc setting.

#include "once.h"
#include "once.h"

#warning this line is a warning on purpose
#pragma yanc nosuch 1

#define CAT(a, b) a ## b
#define SUM3(...) add3(__VA_ARGS__)
#define FIRST(x, ...) pick(x, ##__VA_ARGS__)

int add3(int a, int b, int c) { return a + b + c; }
int pick(int x) { return x; }

struct S { int a; int b; };
static_assert(sizeof(S) == 2, "a struct of two ints takes two words");

class Base { public: virtual int who() { return 1; } };
class Derived : public Base { public: int who() { return 2; } };

void main(void)
{
    int k = 0;
    for (;;) {
        k = k + 1;
        if (k == 3) goto done;
    }
done:
    out(0, k);
    out(0, CAT(1, 2));
    out(0, SUM3(1, 2, 3));
    out(0, FIRST(7));
    out(0, once_count);
    Derived dv;
    Base *b = &dv;
    Derived *d = dynamic_cast<Derived *>(b);
    out(0, d->who());
    int five = 5;
    int *q = reinterpret_cast<int *>(&five);
    out(0, *q);
    out(0, sizeof(S));
}
