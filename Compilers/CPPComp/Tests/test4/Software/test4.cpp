#pragma yanc prname test4
// Tier-1 classes: data members, methods with implicit `this`, unqualified
// member access, and calls via both `.` and `->` (incl. on a heap object).

class Vec2 {
public:
    int x;
    int y;
    int dot(Vec2* o) { return x * o->x + y * o->y; }   // unqualified x,y == this->x,y
    void scale(int k) { x = x * k; y = y * k; }
    int sumxy() { return x + y; }
};

int main_v;
void main(void) {
    Vec2 a;
    a.x = 3; a.y = 4;
    out(0, a.sumxy());        // 7

    Vec2 b;
    b.x = 5; b.y = 6;
    out(0, a.dot(&b));        // 3*5 + 4*6 = 39

    a.scale(10);
    out(0, a.x);              // 30
    out(0, a.y);              // 40

    Vec2* p = new Vec2;
    p->x = 1; p->y = 2;
    out(0, p->sumxy());       // 3
    out(0, p->dot(&a));       // 1*30 + 2*40 = 110
    delete p;
}
