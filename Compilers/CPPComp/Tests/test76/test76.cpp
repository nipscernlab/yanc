#pragma yanc prname test76
// A `static` local is initialised the first time control reaches its
// declaration. cppcomp used to build every one of them at program start, with
// the globals: a function that was never called still ran its static's
// constructor, and the construction order did not match C++. `test70` only
// compared how MANY objects were built, so it never saw this.
//
// Each static now carries a `__once` word, zeroed at main entry and set after
// the initialiser runs. TODO item 11(d).

int g_made = 0;
struct Cnt { int id; Cnt() { g_made = g_made + 1; id = g_made; } };

int never_called(void) { static Cnt dead; return dead.id; }   // never constructed
int late(void)         { static Cnt s;    return s.id; }      // built on the first call
int counter(void)      { static int k = 100; k = k + 1; return k; }
int from_global(void)  { static int base = g_made * 10; return base; }

int guarded(int go) {
    if (go) { static Cnt c; return c.id; }    // only when the branch is taken
    return 0;
}

int tbl(int k) { static int a[4] = {2, 3}; a[2] = a[2] + 1; return a[k]; }

void main(void) {
    out(0, g_made);          // 0: nothing is constructed before main runs

    Cnt first;               // a plain local: the first object of the program
    out(0, first.id);        // 1

    out(0, late());          // 2: built now, after the local
    out(0, g_made);          // 2
    out(0, late());          // 2: not rebuilt

    out(0, counter());       // 101
    out(0, counter());       // 102: initialised once, kept across calls
    out(0, counter());       // 103

    out(0, guarded(0));      // 0: the branch is not taken, nothing is built
    out(0, g_made);          // 2
    out(0, guarded(1));      // 3: built the first time the branch runs
    out(0, guarded(1));      // 3: not rebuilt
    out(0, guarded(0));      // 0

    out(0, from_global());   // 30: the initialiser reads g_made AT FIRST USE
    out(0, from_global());   // 30

    out(0, tbl(0));          // 2: brace initialiser, once
    out(0, tbl(2));          // 2: {2,3} leaves a[2] zero, incremented twice
    out(0, tbl(3));          // 0

    out(0, g_made);          // 3: dead was never built
}
