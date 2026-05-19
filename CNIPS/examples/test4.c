// unions + multidimensional arrays
#pragma yanc prname tst4
#pragma yanc nubits 16

union ix { int i; int x; };

void main(void) {
    // ---- union: write one member, read the (aliased) other ----
    union ix u;
    u.i = 77;
    out(0, u.x);                 // expect 77 (same word)

    // ---- 2-D array ----
    int m[3][4];
    int r;
    int c;
    for (r = 0; r < 3; r++)
        for (c = 0; c < 4; c++)
            m[r][c] = r * 10 + c;

    out(0, m[0][0]);             // expect 0
    out(0, m[1][2]);             // expect 12
    out(0, m[2][3]);             // expect 23

    // sum the whole matrix
    int sum = 0;
    for (r = 0; r < 3; r++)
        for (c = 0; c < 4; c++)
            sum = sum + m[r][c];
    out(0, sum);                 // 0..23 grid: sum = 138
}
