// 3D vector math (graphics / robotics). Exercises float structs passed and
// returned by value, compound literals, float arithmetic (incl. subtraction),
// and a Newton-Raphson square root.
#pragma yanc prname tst37
#pragma yanc nubits 32
#pragma yanc nbmant 23
#pragma yanc nbexpo 8

struct vec3 { float x, y, z; };

float dot(struct vec3 a, struct vec3 b) {
    return a.x * b.x + a.y * b.y + a.z * b.z;
}

struct vec3 cross(struct vec3 a, struct vec3 b) {
    return (struct vec3){ a.y * b.z - a.z * b.y,
                          a.z * b.x - a.x * b.z,
                          a.x * b.y - a.y * b.x };
}

float fsqrt(float x) {
    if (x <= 0.0) return 0.0;
    float g = x;
    for (int i = 0; i < 20; i = i + 1) g = 0.5 * (g + x / g);
    return g;
}

float length(struct vec3 v) { return fsqrt(dot(v, v)); }

void main(void) {
    struct vec3 a = { 1.0, 2.0, 3.0 };
    struct vec3 b = { 4.0, 5.0, 6.0 };

    out(0, (int)dot(a, b));                 // 32

    struct vec3 c = cross(a, b);            // (-3, 6, -3)
    out(0, (int)c.x);                       // -3
    out(0, (int)c.y);                       // 6
    out(0, (int)c.z);                       // -3

    out(0, (int)(length((struct vec3){ 3.0, 4.0, 0.0 }) * 100.0));  // 500
    out(0, (int)(fsqrt(2.0) * 1000.0));     // 1414
}
