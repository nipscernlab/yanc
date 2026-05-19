// bitfields packed into a word
#pragma yanc prname tst12
#pragma yanc nubits 16

struct flags {
    unsigned a : 3;     // bits 0..2
    unsigned b : 4;     // bits 3..6
    unsigned c : 1;     // bit 7
};

void main(void) {
    struct flags f;
    f.a = 5;            // 0b101
    f.b = 9;            // 0b1001
    f.c = 1;
    out(0, f.a);        // 5
    out(0, f.b);        // 9
    out(0, f.c);        // 1

    f.a = 2;            // rewrite a; b and c must be untouched
    out(0, f.a);        // 2
    out(0, f.b);        // 9
    out(0, f.c);        // 1

    f.c = 6;            // 6 & 1-bit-mask = 0
    out(0, f.c);        // 0
}
