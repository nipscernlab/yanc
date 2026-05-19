// raw-value truthiness: the case that used to emit LIN;LIN and (on old HW) misbehave
#pragma yanc prname tst5
#pragma yanc nubits 16

void main(void) {
    int n = 3;
    while (n) {              // counting loop on a raw multi-bit value
        out(0, n);          // expect 3, 2, 1
        n = n - 1;
    }

    int x = 4;
    if (x) out(0, 99);      // 4 is truthy -> expect 99

    if (n) out(0, 0); else out(0, 7);   // n==0 -> else -> expect 7
}
