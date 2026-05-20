// _Generic (C11): compile-time type-generic selection. The controlling
// expression is not evaluated; the matching association's value is the result.
// Note: on this target `char` is just a signed one-word int (no distinct type),
// so _Generic distinguishes int / unsigned / float / pointer but not char.
#pragma yanc prname tst29
#pragma yanc nubits 16

#define KIND(x) _Generic((x), int: 1, unsigned int: 2, float: 3, default: 0)

void main(void) {
    int        i = 0;
    unsigned   u = 0;
    float      f = 0.0;
    int       *p = 0;

    out(0, KIND(i));    // 1
    out(0, KIND(u));    // 2
    out(0, KIND(f));    // 3
    out(0, KIND(p));    // 0  (pointer -> default)

    // the result can be any expression; only the selected one is evaluated
    out(0, _Generic(i, int: 10 + 5, default: 0));   // 15
    out(0, _Generic(f, int: 0, float: 7 * 3));       // 21
}
