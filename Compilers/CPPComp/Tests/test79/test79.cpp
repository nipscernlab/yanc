#pragma yanc prname test79
// A reference used where cppcomp takes a memory-operand shortcut: its word
// holds the referent's ADDRESS, so `z + r` added that address (104 for 105:
// &x was 4) and `++r` stepped it, leaving x alone and the reference pointing
// elsewhere. The binop, ++ and -- fast paths now skip references, as the
// store path already did.
// Expected outputs: 105 500 0 140 9 6 7 39
int g = 40;

void main(void)
{
    int x = 5;
    int& r = x;
    int& q = g;
    int z = 100;
    out(0, z + r);          // 105
    out(0, z * r);          // 500
    out(0, z < r);          // 0
    out(0, z + q);          // 140
    int a[4] = {7, 8, 9, 10};
    int i = 2;
    int& ri = i;
    out(0, a[ri]);          // 9
    ++r;
    out(0, x);              // 6
    r++;
    out(0, x);              // 7
    --q;
    out(0, g);              // 39
}
