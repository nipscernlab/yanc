// test54 - aggregate `= {}` default member initializer must zero-fill the field
// in the ctor, so a HEAP object whose memory was poisoned by a previous object
// (freed + reused slot) sees zeros, not garbage. Locks the __zerofill path.
//
// Without the fix `Box b = {};` is parse-and-drop, so h2 (reusing h1's freed
// slot) would read h1's 111/222/333/444; with the fix the ctor zero-fills b.
#pragma yanc prname test54

struct Box {
    int data[4];
};

class Holder {
public:
    int tag;
    Box b = {};          // aggregate default member init -> zero-fill at ctor time
    Holder(int t) { tag = t; }
};

void main(void) {
    Holder* h1 = new Holder(1);              // first alloc
    h1->b.data[0] = 111; h1->b.data[1] = 222;
    h1->b.data[2] = 333; h1->b.data[3] = 444; // poison the slot
    delete h1;

    Holder* h2 = new Holder(2);              // reuses h1's freed slot
    out(0, h2->tag);                         // 2
    out(0, h2->b.data[0]);                   // 0 (zero-filled, not 111)
    out(0, h2->b.data[1]);                   // 0
    out(0, h2->b.data[2]);                   // 0
    out(0, h2->b.data[3]);                   // 0
    delete h2;
}
