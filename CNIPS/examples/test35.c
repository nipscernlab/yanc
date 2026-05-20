// Mandelbrot escape-time (numerical / fractals). Exercises IEEE-754 float
// arithmetic, float comparison, and break inside a float loop. Each output is
// the iteration count at which the point escapes (or maxit if it stays bounded).
#pragma yanc prname tst35
#pragma yanc nubits 32
#pragma yanc nbmant 23
#pragma yanc nbexpo 8

int mandel(float cr, float ci, int maxit) {
    float zr = 0.0, zi = 0.0;
    int i;
    for (i = 0; i < maxit; i = i + 1) {
        float zr2 = zr * zr - zi * zi + cr;
        float zi2 = 2.0 * zr * zi + ci;
        zr = zr2;
        zi = zi2;
        if (zr * zr + zi * zi > 4.0) break;
    }
    return i;
}

void main(void) {
    out(0, mandel(0.0, 0.0, 50));     // origin: never escapes -> 50
    out(0, mandel(-1.0, 0.0, 50));    // in the set -> 50
    out(0, mandel(2.0, 0.0, 50));     // escapes immediately -> 0
    out(0, mandel(0.5, 0.0, 50));     // escapes after a few -> small count
    out(0, mandel(-0.5, 0.5, 50));    // interior-ish
    out(0, mandel(0.3, 0.6, 50));     // boundary -> some count
    out(0, mandel(-0.75, 0.1, 50));   // near boundary
}
