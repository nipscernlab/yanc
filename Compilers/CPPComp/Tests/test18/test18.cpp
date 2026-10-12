// Operator overloading for class types: operator= (custom assignment, replaces
// the default memberwise copy) and operator[] (subscript).
#pragma yanc prname test18
int assign_log;
class Vec {
public:
    int x, y;
    Vec& operator=(Vec o) { x = o.x; y = o.y; assign_log = assign_log + 1; return *this; }
    int  operator[](int i) { return i == 0 ? x : y; }
};
void main(void) {
    Vec a; a.x = 3; a.y = 4;
    Vec b; b.x = 0; b.y = 0;
    assign_log = 0;
    b = a;                  // overloaded operator= (copies + logs)
    out(0, b.x);            // 3
    out(0, b.y);            // 4
    out(0, assign_log);     // 1
    out(0, a[0]);           // 3  (operator[])
    out(0, a[1]);           // 4
}
