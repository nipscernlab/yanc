#pragma yanc prname test43
// Default function arguments. The prototype carries the defaults; the
// out-of-line definition omits them. Calls with fewer args than params
// get the missing trailing values from the prototype's stored defaults.

float scaled_sum(float x, float k = 2.0f, int n = 3);

void main(void) {
    // all defaults: 10*2 + 3 = 23
    out(0, (int)scaled_sum(10.0f));

    // override one default: 10*5 + 3 = 53
    out(0, (int)scaled_sum(10.0f, 5.0f));

    // override all: 10*5 + 9 = 59
    out(0, (int)scaled_sum(10.0f, 5.0f, 9));
}

float scaled_sum(float x, float k, int n) {
    return x * k + (float)n;
}
