// Stack-object RAII: default construction at declaration + destruction at
// scope exit (ctor/dtor track a live-object count in a global).
#pragma yanc prname test13
int alive;        // live-object count (ctor++ / dtor--)

class Counter {
public:
    int id;
    Counter()  { id = 0; alive = alive + 1; }   // default ctor
    ~Counter() { alive = alive - 1; }            // destructor
};

void scope_test(void) {
    Counter a;            // ctor -> alive++
    Counter b;            // ctor -> alive++
    out(0, alive);        // 3 (c + a + b)
}                         // block exit -> b,a dtors

void main(void) {
    alive = 0;
    out(0, alive);        // 0
    Counter c;            // ctor -> alive = 1
    out(0, alive);        // 1
    scope_test();
    out(0, alive);        // 1 (a,b destroyed; c still alive)
}
