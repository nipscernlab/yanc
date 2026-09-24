#pragma yanc prname test78
// Value-initialised locals (`T v{};`, `T v = {};`, `T v[N] = {};`) are zeroed
// on EVERY entry: a local keeps fixed storage, so these forms emitted nothing
// and the second call found the first call's writes (TODO 16). probe() runs
// twice, writing garbage into everything after reading it, so only a correct
// value-init prints the same line twice. Also locks in:
//  - a class with a user default constructor still runs it (C{} -> v = 42);
//  - default member initializers apply (D{} -> a = 5, b = 0);
//  - a class with a vtable keeps it (V{} -> f() = 1, w = 0);
//  - a class template's constructor runs, with braces, without, and with an
//    argument (TC<int>{} / TC<int> t2; -> 7, TA<int> u(5) -> 5): its
//    constructors are clones in g_inst, which resolve_ctor did not search;
//  - a float zeroed by a brace initializer equals 0.0f: the float zero is the
//    0.0 constant, not the word 0 (EQU compares words).
// Expected outputs: 111111111 111111111 (nine 1s per call, as one number)

struct P { int x; int y; int z[6]; };
struct C { int v; C() { v = 42; } };
struct D { int a = 5; int b; };
struct V { int w; virtual int f() { return 1; } };
template <class T> struct TC { T v; TC() { v = 7; } };
template <class T> struct TA { T v; TA(T a) { v = a; } };

void probe(int dirt)
{
    int   a[8] = {};
    P     q = {};
    P     p{};
    int   x{};
    float f{};
    float fa[6] = {1.0f};
    C     c{};
    D     d{};
    V     v{};
    TC<int> t{};
    TC<int> t2;
    TA<int> u(5);

    int zs = x;
    for (int k = 0; k < 8; ++k) zs += a[k];
    zs += q.x + q.y + p.x + p.y;
    for (int k = 0; k < 6; ++k) zs += q.z[k] + p.z[k];

    int r = 0;
    r = r * 10 + (zs == 0);
    r = r * 10 + (f == 0.0f);
    r = r * 10 + (fa[3] == 0.0f && fa[0] == 1.0f);
    r = r * 10 + (c.v == 42);
    r = r * 10 + (d.a == 5 && d.b == 0);
    r = r * 10 + (v.f() == 1 && v.w == 0);
    r = r * 10 + (t.v == 7);
    r = r * 10 + (t2.v == 7);
    r = r * 10 + (u.v == 5);
    out(0, r);

    for (int k = 0; k < 8; ++k) a[k] = dirt;
    q.x = dirt; q.y = dirt; p.x = dirt; p.y = dirt;
    for (int k = 0; k < 6; ++k) { q.z[k] = dirt; p.z[k] = dirt; fa[k] = 3.5f; }
    x = dirt; f = 2.5f; c.v = dirt; d.a = dirt; d.b = dirt; v.w = dirt; t.v = dirt; t2.v = dirt; u.v = dirt;
}

void main(void)
{
    probe(1000);
    probe(2000);
}
