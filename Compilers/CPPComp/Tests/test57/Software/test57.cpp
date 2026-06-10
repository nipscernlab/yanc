// test57 - an array-of-function-pointers as a STRUCT FIELD:
// `int (*fns[N])(int)` inside a struct (the field rule previously lacked the
// array_suffix). Each element holds a function id; calling through an indexed
// element dispatches. Locks the struct-field array-of-funcptr grammar.
#pragma yanc prname test57

int f0(int x) { return x; }
int f1(int x) { return x + 10; }
int f2(int x) { return x + 20; }

struct Table {
    int (*fns[3])(int);     // array-of-function-pointers field
    int tag;
};

void main(void) {
    Table t;
    t.tag = 7;
    t.fns[0] = f0;
    t.fns[1] = f1;
    t.fns[2] = f2;
    out(0, t.fns[0](5));    // 5
    out(0, t.fns[1](5));    // 15
    out(0, t.fns[2](5));    // 25
    out(0, t.tag);          // 7
}
