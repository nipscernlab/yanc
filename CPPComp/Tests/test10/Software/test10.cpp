// Virtual functions + vtables: dynamic dispatch through a base-class pointer.
// Each polymorphic object carries a vptr at offset 0; the ctor/new sets it to
// the class vtable; p->area() loads the slot from the vtable and dispatches.
#pragma yanc prname test10
class Shape {
public:
    virtual int area()  { return 0; }
    virtual int sides() { return 0; }
};
class Square : public Shape {
public:
    int s;
    int area()  { return s * s; }   // override (implicitly virtual)
    int sides() { return 4; }
};
class Triangle : public Shape {
public:
    int b, h;
    int area()  { return b * h / 2; }
    int sides() { return 3; }
};

void main(void) {
    Square* sq = new Square;   sq->s = 5;
    Triangle* tr = new Triangle; tr->b = 6; tr->h = 4;

    Shape* p = sq;             // base pointer to a Square
    out(0, p->area());         // 25  (dynamic dispatch -> Square::area)
    out(0, p->sides());        // 4
    p = tr;                    // base pointer to a Triangle
    out(0, p->area());         // 12  (-> Triangle::area)
    out(0, p->sides());        // 3
    out(0, sq->area());        // 25  (virtual via the derived pointer)
}
