// type-specifier aliasing: every C scalar combo folds to the target's
// int / uint / float word; rare qualifiers are swallowed as no-ops.
#pragma yanc prname tst18
#pragma yanc nubits 16

static int g = 100;            // static storage class at file scope

void main(void) {
    short s = 10;  short int si = 20;
    long l = 30;   long int li = 40;   long long ll = 50;
    unsigned short us = 7;
    unsigned long ul = 9;
    unsigned long long ull = 11;
    signed char sc = -5;
    unsigned char uc = 200;
    _Bool b = 1;
    double d = 1.5;
    long double ld = 2.5;

    const int ci = 4;          // const works
    volatile int vi = 5;       // volatile / register swallowed
    register int ri = 6;

    out(0, s + si);            // 30
    out(0, l + li + ll);       // 120
    out(0, us + ull + ul);     // 27
    out(0, sc);                // -5
    out(0, uc);                // 200
    out(0, b);                 // 1
    out(0, (int)(d + ld));     // (int)4.0 = 4
    out(0, ci + vi + ri);      // 15
    out(0, g);                 // 100

    unsigned long big = 40000; // unsigned word -> unsigned divide path
    out(0, big / 7);           // 5714
}
