// Pure virtual (= 0) abstract base, override/final, and const member functions.
#pragma yanc prname test14
class Shape {
public:
    virtual int area() = 0;                       // pure virtual -> abstract
    virtual int kind() const { return 0; }
};
class Sq : public Shape {
public:
    int s;
    int area() override        { return s * s; }
    int kind() const override  { return 1; }
};
void main(void) {
    Sq* q = new Sq; q->s = 6;
    Shape* p = q;               // base pointer to a Sq
    out(0, p->area());          // 36 (override of pure virtual)
    out(0, p->kind());          // 1
}
