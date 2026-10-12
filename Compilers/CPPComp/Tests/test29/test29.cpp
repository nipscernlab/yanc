#pragma yanc prname test29
// Realistic embedded example: a 3D vector for physics/IMU math.
// Exercises overloaded constructors, binary operators (+ - * scalar), a unary
// negation operator, and a dot-product method, all over IEEE floats.

class Vec3 {
    float x, y, z;
public:
    Vec3() : x(0.0f), y(0.0f), z(0.0f) {}
    Vec3(float a, float b, float c) : x(a), y(b), z(c) {}

    Vec3 operator+(Vec3 o) { return Vec3(x + o.x, y + o.y, z + o.z); }
    Vec3 operator-(Vec3 o) { return Vec3(x - o.x, y - o.y, z - o.z); }
    Vec3 operator*(float s) { return Vec3(x * s, y * s, z * s); }
    Vec3 operator-()        { return Vec3(-x, -y, -z); }   // unary negate

    float dot(Vec3 o) { return x * o.x + y * o.y + z * o.z; }

    float getx() { return x; }
    float gety() { return y; }
    float getz() { return z; }
};

void main(void) {
    Vec3 a(1.0f, 2.0f, 3.0f);
    Vec3 b(4.0f, 5.0f, 6.0f);

    Vec3 s = a + b;             // (5,7,9)
    out(0, (int)s.getx());      // 5
    out(0, (int)s.gety());      // 7
    out(0, (int)s.getz());      // 9

    Vec3 d = b - a;             // (3,3,3)
    out(0, (int)d.getx());      // 3

    Vec3 t = a * 10.0f;         // (10,20,30)
    out(0, (int)t.getz());      // 30

    Vec3 n = -a;                // (-1,-2,-3)
    out(0, (int)n.getx());      // -1
    out(0, (int)n.getz());      // -3

    out(0, (int)a.dot(b));      // 1*4+2*5+3*6 = 32
}
