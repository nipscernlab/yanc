#pragma yanc prname test70
// Every object is constructed. cppcomp ran a constructor only for a plain local
// object and `new T`: global objects, static locals, array elements, new T[n]
// and member objects stayed all zero, and a class without a constructor got no
// implicit one, so its default member initializers were never applied. Now:
// the implicit default constructor is synthesized when there is something to
// run; every constructor first builds its base (unless its member-init list
// does) and its member objects (one the list names, with the list's
// arguments: `: inner(5)` used to copy the object at address 5); globals are
// constructed at program start after every global value. A static local is
// built the first time control reaches it (test76); this test only compares
// how many objects were built, which that timing does not change.
// g_made counts constructions, so a missing or a doubled one shows. A braced
// initializer gives what it leaves out its default member initializer, or
// zero -- also on a second call, when a local's fixed storage still holds the
// first call's values.
int g_made = 0;
struct Cnt { int id; Cnt() { g_made = g_made + 1; id = g_made; } };

struct Cfg { int gain = 5; float tau = 0.5f; int n = 3; };          // no ctor: implicit one
class Box { public: int k = 7; Cnt c; Box() { k = k + 1; } };       // user ctor, member object
struct Pair { Cnt a; Cnt b; };                                      // member objects only
struct Base { int b = 11; };
struct Derived : Base { int d = 22; };                              // base built implicitly
struct DerivedN : Base { int d; DerivedN() : Base() { d = b + 1; } }; // base named: built once
struct Plain { int q; };
struct DerivedP : Plain { int d; DerivedP() : Plain() { d = 4; } };   // a base with nothing to run
struct In { int v; In() { v = 1; g_made = g_made + 1; } In(int x) { v = x; g_made = g_made + 1; } };
class Out { public: In inner; int z; Out() : inner(5) { z = 2; } };   // member built once, with 5
struct P2 { int x; int y; };
class HasP { public: P2 pt; HasP(P2 p) : pt(p) {} };                // no ctor takes it: a copy
class Poly { public: virtual int v() { return 1; } };
class PolyD : public Poly { public: virtual int v() { return 2; } };

Cfg   g_cfg;
Box   g_box;
Cnt   g_arr[3];
PolyD g_poly;                                   // a global's vptr is set too

int agg(int v) {
    int a[4] = {v};                             // omitted elements are zero on every call
    Cfg c = {v};                                // omitted members take their defaults
    int r = a[1] + a[3] + c.n * 100 + (int)(c.tau * 10.0f);
    a[1] = 7; a[3] = 8; c.n = 9; c.tau = 9.0f;  // dirty them for the next call
    return r;
}

int stat_id(void) { static Cnt s; return s.id; }

void main(void) {
    out(0, g_cfg.gain + g_cfg.n);               // 8
    out(0, (int)(g_cfg.tau * 10.0f));           // 5
    out(0, g_box.k);                            // 8
    out(0, g_box.c.id);                         // 1: the first object built
    out(0, g_arr[2].id);                        // 4
    Poly *pp = &g_poly;
    out(0, pp->v());                            // 2
    Pair p;
    out(0, p.b.id - p.a.id);                    // 1
    Box bl[2];
    out(0, bl[1].k);                            // 8
    Derived dv;
    out(0, dv.b + dv.d);                        // 33
    DerivedN dn;
    out(0, dn.d);                               // 12
    DerivedP dp;
    out(0, dp.d);                               // 4
    Out ou;
    out(0, ou.inner.v);                         // 5: the member-init list's argument
    P2 q = {3, 4};
    HasP hp(q);
    out(0, hp.pt.x + hp.pt.y);                  // 7
    Cfg *hc = new Cfg;
    out(0, hc->gain);                           // 5
    Box *hb = new Box[2];
    out(0, hb[1].k);                            // 8
    out(0, stat_id() == stat_id());             // 1: built once
    out(0, agg(1));                             // 305
    out(0, agg(2));                             // 305
    out(0, g_made);                             // 12 constructions, none twice
}
