// Taylor-series exp() and sin() (numerical methods). Exercises IEEE-754 float
// add/sub/mul/div in loops, int->float casts, and unary negation. Results are
// printed scaled by 1000 (truncated) so the golden is deterministic.
#pragma yanc prname tst36
#pragma yanc nubits 32
#pragma yanc nbmant 23
#pragma yanc nbexpo 8

float my_exp(float x) {
    float term = 1.0, sum = 1.0;
    for (int n = 1; n <= 12; n = n + 1) {
        term = term * x / (float)n;
        sum = sum + term;
    }
    return sum;
}

float my_sin(float x) {
    float term = x, sum = x;
    for (int k = 1; k <= 9; k = k + 1) {
        int d = (2 * k) * (2 * k + 1);
        term = term * (-(x * x)) / (float)d;   // next term
        sum = sum + term;
    }
    return sum;
}

void main(void) {
    out(0, (int)(my_exp(0.0) * 1000.0));   // 1000
    out(0, (int)(my_exp(1.0) * 1000.0));   // e   ~ 2718
    out(0, (int)(my_exp(2.0) * 1000.0));   // e^2 ~ 7389

    out(0, (int)(my_sin(0.0) * 1000.0));          // 0
    out(0, (int)(my_sin(0.5235988) * 1000.0));    // sin(pi/6) ~ 500
    out(0, (int)(my_sin(1.5707963) * 1000.0));    // sin(pi/2) ~ 1000
}
