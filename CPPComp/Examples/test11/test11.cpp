// Member-initializer lists (Ctor():x(a),y(b){}) and operator overloading
// (operator+ / operator== as member functions).
#pragma yanc prname test11
class Vec {
public:
    int x, y;
    Vec(int a, int b) : x(a), y(b) {}            // member-initializer list
    int operator+(Vec o)  { return x + o.x + y + o.y; }
    int operator==(Vec o) { return x == o.x && y == o.y; }
};

void main(void) {
    Vec* a = new Vec(3, 4);    // ctor init list -> x=3, y=4
    Vec* b = new Vec(1, 2);
    out(0, a->x);              // 3
    out(0, a->y);              // 4
    out(0, (*a) + (*b));       // 3+1+4+2 = 10  (operator+)
    out(0, (*a) == (*b));      // 0
    Vec* c = new Vec(3, 4);
    out(0, (*a) == (*c));      // 1
}
