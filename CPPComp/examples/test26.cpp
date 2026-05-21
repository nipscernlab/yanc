#pragma yanc prname test26
// Realistic embedded example: Q8.8 fixed-point arithmetic (no FPU).
// A value is stored as an int scaled by 256. Exercises overloaded constructors
// (default + converting), operator overloading including the compound operator
// +=, and a method returning a reference (operator+= returns *this).

class Fixed {
    int raw;                       // value * 256
public:
    Fixed() : raw(0) {}
    Fixed(int w) : raw(w * 256) {} // from a whole number

    Fixed operator+(Fixed o) { Fixed r; r.raw = raw + o.raw; return r; }
    Fixed operator*(Fixed o) { Fixed r; r.raw = (raw * o.raw) / 256; return r; }
    Fixed& operator+=(Fixed o) { raw = raw + o.raw; return *this; }
    bool  operator==(Fixed o) { return raw == o.raw; }

    int whole() { return raw / 256; }
};

void main(void) {
    Fixed a(3);
    Fixed b(2);

    Fixed c = a + b;          // 5
    out(0, c.whole());        // 5

    Fixed d = a * b;          // 6
    out(0, d.whole());        // 6

    Fixed acc(0);
    for (int i = 0; i < 4; i = i + 1) acc += a;   // 3*4 = 12
    out(0, acc.whole());      // 12

    Fixed e(5);
    out(0, c == e);           // 1 (5 == 5)
}
