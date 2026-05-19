// unsigned semantics: logical >> and unsigned comparison
#pragma yanc prname tst13
#pragma yanc nubits 16

void main(void) {
    // logical vs arithmetic right shift of a high-bit-set value
    int           s = 0x8000;   // -32768 as signed
    unsigned int  u = 0x8000;   // 32768 as unsigned

    out(0, s >> 4);   // arithmetic: sign-extends -> 0xF800 = -2048
    out(0, u >> 4);   // logical: zero-fills    -> 0x0800 = 2048

    // unsigned comparison: 0x8000 (=32768u) > 1u, but as signed 0x8000 < 1
    if (u > 1)  out(0, 111); else out(0, 222);   // unsigned -> 111
    if (s > 1)  out(0, 333); else out(0, 444);   // signed   -> 444 (negative)

    unsigned int a = 5;
    unsigned int b = 9;
    if (a < b)  out(0, 1); else out(0, 0);       // 1
}
