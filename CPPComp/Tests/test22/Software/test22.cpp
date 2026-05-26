#pragma yanc prname test22
// Real monomorphization: maxv/addv instantiate a separate concrete function per
// argument type, so the float instances use float ops (not the int-erased ones).
template<class T> T maxv(T a, T b) { return a > b ? a : b; }
template<class T> T addv(T a, T b) { return a + b; }

void main(void) {
    out(0, maxv(3, 7));                      // 7   (int instance)
    out(0, maxv(9, 2));                      // 9
    out(0, (int)(maxv(1.5, 2.5) * 10.0));    // 25  (float instance: max=2.5)
    out(0, addv(4, 5));                      // 9   (int)
    out(0, (int)(addv(1.5, 2.25) * 100.0));  // 375 (float: 3.75)
}
