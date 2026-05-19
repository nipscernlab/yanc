// minimal smoke: int loop with out()
#pragma yanc prname tst1
#pragma yanc nubits 16

void main(void) {
    int i;
    for (i = 0; i < 5; i++) {
        out(0, i * 2);
    }
}
