// Expressions (#128): each line printed is what C++ gives.
//
// out(0, k), out(0, a[0])  -> 1, 5  a[k++] += 5 increments k once
// out(0, (int)(y * 2))     -> 5     float y = c ? 1 : 2.5f, c == 0
// out(0, x)                -> 2     int x = c ? 1 : 2.5f
// out(0, r1), out(0, r2)   -> 0, 0  0.0f && 1, 0.0f || 0
// out(0, b > m)            -> 1     bool true > int -1
// out(0, y2)               -> 5     y2 = (s.b = 5), b a 4-bit field at bit 4
// out(0, 7)                -> 7     switch (0) takes case 0, not case -1
// out(0, pick(1))          -> 9     a local declared after a return in a case

struct BF { int a : 4; int b : 4; };

int pick(int n)
{
    switch (n) {
    case 0: return 1;
        int t[4];
    case 1: t[2] = 9; return t[2];
    }
    return 0;
}

void main(void)
{
    int a[3]; a[0] = 0; a[1] = 0; a[2] = 0;
    int k = 0;
    a[k++] += 5;
    out(0, k);
    out(0, a[0]);

    int c = 0;
    float y = c ? 1 : 2.5f;
    out(0, (int)(y * 2));
    int x = c ? 1 : 2.5f;
    out(0, x);

    float z = 0.0f; int one = 1;
    int r1 = z && one;
    int r2 = z || 0;
    out(0, r1);
    out(0, r2);

    bool b = true; int m = -1;
    out(0, b > m);

    BF s; s.a = 0;
    int y2 = (s.b = 5);
    out(0, y2);

    switch (0) {
    case -1: out(0, 100); break;
    case 0:  out(0, 7);   break;
    }

    out(0, pick(1));
}
