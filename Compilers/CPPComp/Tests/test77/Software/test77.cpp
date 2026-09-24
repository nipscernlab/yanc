#pragma yanc prname test77
// The zero-fill loop of a local brace initializer (emit_zero_words), at the
// three shapes of its exit test: the run starting at word 0 (a leading array
// member the designated list leaves out), at word 1 (one item given) and at
// word 2 (two items given). A local keeps fixed storage, so the second call
// finds the first call's writes there and only a correct fill brings the sums
// back; a store one word before the run or one past its end changes them too.
struct Q { int z[6]; int x; };

void probe(int dirt) {
    int guard = 3;
    Q q = {.x = 4};          // run 0..5
    int b[9] = {7, 8};       // run 2..8
    int c[6] = {5};          // run 1..5
    int s = guard + q.x;
    for (int j = 0; j < 6; ++j) s += q.z[j];
    for (int j = 0; j < 9; ++j) s += b[j] * 10;
    for (int j = 0; j < 6; ++j) s += c[j] * 100;
    out(0, s);               // 3 + 4 + 150 + 500 = 657
    for (int j = 0; j < 6; ++j) q.z[j] = dirt;
    for (int j = 0; j < 9; ++j) b[j] = dirt;
    for (int j = 0; j < 6; ++j) c[j] = dirt;
    q.x = dirt; guard = dirt;
}

void main(void) {
    probe(1000);
    probe(2000);             // 657 again: every run was zeroed
}
