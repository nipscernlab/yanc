// unsigned division / modulo (software shift-subtract; ULA DIV/MOD are signed)
#pragma yanc prname tst17
#pragma yanc nubits 16

void main(void) {
    unsigned int a = 40000;     // > INT_MAX: signed division would be wrong
    unsigned int b = 7;
    out(0, a / b);              // 40000 / 7 = 5714
    out(0, a % b);              // 40000 % 7 = 2

    unsigned int c = 100;
    out(0, c / 6);             // 16
    out(0, c % 6);             // 4

    int s = -20;               // signed division path is unchanged
    out(0, s / 3);             // -6
}
