// relu, sigmoid, tanh, softmax and argmax, and exp and tanh of a number, as
// cppcomp built-ins (docs/design/nn-functions.md). The first 42 values are
// those of the C± test cmm_nn, from the same arrays; the last five are exp,
// std::exp, tanh, std::tanh and sigmoid of a number. Scripts/nn_ref.py
// computes them in Python and holds golden.txt to them.
#include <cmath>
float v[5]    = {-2.0f, -0.5f, 0.0f, 0.75f, 3.0f};
int   iv[4]   = {-3, 5, -1, 2};
float m[2][3] = {{-1.0f, 2.0f, -3.0f}, {4.0f, -5.0f, 6.0f}};
float g[3]    = {-1.0f, 0.0f, 1.0f};
float t[3]    = {-1.0f, 0.0f, 1.0f};
float s[5]    = {1.0f, 2.0f, 3.0f, 2.0f, 1.0f};
float big[3]  = {200.0f, 199.0f, 0.0f};
float one[1]  = {-3.0f};
float tie[4]  = {1.0f, 5.0f, 5.0f, -2.0f};
float neg[3]  = {-4.0f, -1.0f, -2.0f};
int   cnt[3]  = {0, 0, 9};

int best(int n) { float loc[3]; loc[0] = 0.5f; loc[1] = -0.5f; loc[2] = 2.5f; return argmax(loc) * n; }

int main() {
    relu(v);   for (int k = 0; k < 5; k++) out(0, (int)(v[k] * 1000.0f));
    relu(iv);  for (int k = 0; k < 4; k++) out(0, iv[k]);
    relu(m);   for (int r = 0; r < 2; r++) for (int c = 0; c < 3; c++) out(0, (int)(m[r][c] * 1000.0f));
    sigmoid(g); for (int k = 0; k < 3; k++) out(0, (int)(g[k] * 1000000.0f));
    tanh(t);    for (int k = 0; k < 3; k++) out(0, (int)(t[k] * 1000000.0f));
    softmax(s); for (int k = 0; k < 5; k++) out(0, (int)(s[k] * 1000000.0f));
    softmax(big); for (int k = 0; k < 3; k++) out(0, (int)(big[k] * 1000000.0f));
    softmax(one); out(0, (int)(one[0] * 1000.0f));
    out(0, argmax(s)); out(0, argmax(iv)); out(0, argmax(tie)); out(0, argmax(one)); out(0, argmax(neg));
    int k = 0;
    while (argmax(cnt) != 0) { cnt[0] = cnt[0] + 5; k++; }
    out(0, k);
    out(0, best(10));
    out(0, argmax(tie) * 10 + argmax(neg));
    float x = 0.5f;
    out(0, (int)(sigmoid(x) * 1000000.0f));
    out(0, (int)(relu(-x) * 1000.0f));
    out(0, relu(-7)); out(0, relu(7));
    out(0, (int)(exp(1.0f) * 1000000.0f));
    out(0, (int)(std::exp(-1.0f) * 1000000.0f));
    out(0, (int)(tanh(0.5f) * 1000000.0f));
    out(0, (int)(std::tanh(x) * 1000000.0f));
    out(0, (int)(sigmoid(2) * 1000000.0f));
    return 0;
}
