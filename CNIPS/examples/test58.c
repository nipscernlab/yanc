// Duff's device (the famous loop-unrolling trick): switch case labels
// interleaved with a do-while loop body. A real stress test for the switch
// code generator — case labels must be reachable anywhere in the body, with
// fall-through into the loop.
#pragma yanc prname tst58

void copyn(int *to, int *from, int count) {
    int n = (count + 7) / 8;
    int i = 0;
    switch (count % 8) {
    case 0: do { to[i] = from[i]; i = i + 1;
    case 7:      to[i] = from[i]; i = i + 1;
    case 6:      to[i] = from[i]; i = i + 1;
    case 5:      to[i] = from[i]; i = i + 1;
    case 4:      to[i] = from[i]; i = i + 1;
    case 3:      to[i] = from[i]; i = i + 1;
    case 2:      to[i] = from[i]; i = i + 1;
    case 1:      to[i] = from[i]; i = i + 1;
            } while (--n > 0);
    }
}

void main(void) {
    int src[13];
    int dst[13];
    for (int k = 0; k < 13; k = k + 1) { src[k] = (k + 1) * 100; dst[k] = -1; }

    copyn(dst, src, 13);          // copies all 13 (13 % 8 == 5 entry)

    out(0, dst[0]);    // 100
    out(0, dst[7]);    // 800
    out(0, dst[12]);   // 1300
    int sum = 0;
    for (int k = 0; k < 13; k = k + 1) sum = sum + dst[k];
    out(0, sum);       // 100+200+...+1300 = 9100
}
