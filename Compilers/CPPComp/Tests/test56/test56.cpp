// test56 - access a static data member through an instance (`obj.s`) and through
// a pointer (`ptr->s`), for both read and write. A static is one shared global
// (Class__s), so a write through one object is visible through another and
// through Class::s. Locks the obj.staticmember / ptr->staticmember resolution.
#pragma yanc prname test56

class Counter {
public:
    static int total = 100;   // static data member (in-class init = definition)
    int id;
    Counter(int x) { id = x; }
};

void main(void) {
    Counter a(1);
    Counter b(2);
    out(0, a.total);          // 100  (read static via instance)
    out(0, Counter::total);   // 100  (read via Class:: — control)
    a.total = 250;            // write static via instance
    out(0, b.total);          // 250  (shared: b sees a's write)
    out(0, Counter::total);   // 250
    Counter* p = &a;
    out(0, p->total);         // 250  (read static via pointer ->)
    p->total = 7;             // write via pointer
    out(0, a.total);          // 7
}
