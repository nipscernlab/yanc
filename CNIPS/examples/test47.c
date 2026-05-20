// Projectile motion by explicit Euler integration (physics simulation).
// Exercises float state updated in a loop with a float loop condition and
// float subtraction (velocity decay under gravity).
#pragma yanc prname tst47

void main(void) {
    float x = 0.0, y = 0.0;
    float vx = 10.0, vy = 20.0;
    float g = 9.8, dt = 0.1;
    int steps = 0;

    // integrate until the projectile falls back to the ground (y < 0)
    while (y >= 0.0) {
        x  = x + vx * dt;
        y  = y + vy * dt;
        vy = vy - g * dt;
        steps = steps + 1;
        if (steps > 200) break;       // safety bound
    }

    out(0, steps);              // steps to land (~41)
    out(0, (int)x);             // horizontal range (~40)
    out(0, (int)(vy * 10.0));   // final vertical velocity *10 (negative)
}
