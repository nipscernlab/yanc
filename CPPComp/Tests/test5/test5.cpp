// Constructors / destructors wired into new/delete, plus bool/true/false.
// new Point(3,4) allocates then runs the ctor; delete runs the dtor then frees.
#pragma yanc prname test5
int dtor_log;

class Point {
public:
    int x, y;
    Point(int a, int b) { x = a; y = b; }
    ~Point() { dtor_log = x + y; }
    int norm2() { return x * x + y * y; }
};

void main(void) {
    Point* p = new Point(3, 4);
    out(0, p->x);          // 3
    out(0, p->y);          // 4
    out(0, p->norm2());    // 25
    dtor_log = 0;
    delete p;              // dtor: dtor_log = 3+4
    out(0, dtor_log);      // 7
    bool flag = true;
    out(0, flag);          // 1
    out(0, false);         // 0
}
