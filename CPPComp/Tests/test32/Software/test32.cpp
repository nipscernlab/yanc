#pragma yanc prname test32
// Realistic embedded example: a fixed-size array container parameterised by a
// non-type template parameter (the element count). Avoids the heap entirely —
// the storage is inline. Classic embedded "static container" idiom.

template<int N>
class Array {
    int data[N];
public:
    void set(int i, int v) { data[i] = v; }
    int  get(int i) { return data[i]; }
    int  size() { return N; }
    int  sum() {
        int s = 0;
        for (int i = 0; i < N; i = i + 1) s = s + data[i];
        return s;
    }
};

void main(void) {
    Array<4> a;
    for (int i = 0; i < 4; i = i + 1) a.set(i, i * 10);
    out(0, a.get(2));      // 20
    out(0, a.size());      // 4
    out(0, a.sum());       // 0+10+20+30 = 60

    Array<3> b;            // a different instantiation
    b.set(0, 5); b.set(1, 7); b.set(2, 9);
    out(0, b.size());      // 3
    out(0, b.sum());       // 21
}
