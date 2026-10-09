// test90: user names spelt like the compiler's own names in the .asm. Before
// those moved under `__` (607bff4) this printed 4 1 1 7 105 6: the unsigned
// division wrote its remainder into the user's global `udm_r`. `_str0` (the
// string literal's word), `Lwh1` and `mal_loop` (labels) stay as guards.
// Expected (hand-computed): 9/2 = 4, then udm_r 2, _str0 1, Lwh1 + k = 4 + 3,
// s[1] 'i' = 105, mal_loop 6.
#pragma yanc prname test90

int udm_r = 2;
int _str0 = 1;
int Lwh1 = 4;
int mal_loop = 6;

void main(void) {
    unsigned a = 9, b = 2;
    out(0, (int)(a / b));                       // 4
    out(0, udm_r);                              // 2
    const char *s = "hi";
    out(0, _str0);                              // 1
    int k = 0;
    while (k < 3) k = k + 1;
    out(0, Lwh1 + k);                           // 7
    out(0, s[1]);                               // 105
    int *p = new int[2];
    p[0] = mal_loop;
    out(0, p[0]);                               // 6
    delete[] p;
}
