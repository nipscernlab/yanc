// const: initialisation and reads are fine; assignment is rejected (see below)
#pragma yanc prname tst15
#pragma yanc nubits 16

const int K = 100;            // const global

void main(void) {
    const int local = 7;      // const local
    out(0, K);                // 100
    out(0, local);            // 7
    out(0, K + local);        // 107

    int v = K;                // reading a const into a mutable is fine
    v = v + 1;
    out(0, v);                // 101
}

// NOTE: `K = 1;` or `local = 9;` now fail to compile with
//   "assignment to const 'K'"  /  "assignment to const 'local'"
