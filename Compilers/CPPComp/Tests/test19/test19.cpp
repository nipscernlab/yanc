// Copy constructor: Buf(Buf&) coexists with Buf(int) (distinct mangling) and
// runs on copy-initialization, producing an independent copy.
#pragma yanc prname test19
int copies;
class Buf {
public:
    int v;
    Buf(int x)  { v = x; }                          // regular ctor
    Buf(Buf& o) { v = o.v; copies = copies + 1; }   // copy ctor (distinct mangling)
};
void main(void) {
    copies = 0;
    Buf a(5);          // Buf__ctor(this, 5)
    Buf b = a;         // copy-init -> Buf__copyctor(&b, &a)
    out(0, b.v);       // 5
    out(0, copies);    // 1
    a.v = 9;           // mutate the original
    out(0, b.v);       // 5  (b is an independent copy)
}
