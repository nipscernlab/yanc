// function-like macros (preprocessor)
#pragma yanc prname tst8
#pragma yanc nubits 16

#define SQUARE(x)   ((x) * (x))
#define MAX(a, b)   ((a) > (b) ? (a) : (b))
#define ADD3(a,b,c) ((a) + (b) + (c))
#define DBL(x)      ((x) + (x))
#define NEST(x)     SQUARE(x)        /* macro that expands to another macro */

void main(void) {
    out(0, SQUARE(5));          // 25
    out(0, MAX(3, 9));          // 9
    out(0, MAX(9, 3));          // 9
    out(0, ADD3(1, 2, 3));      // 6
    out(0, DBL(7));             // 14
    out(0, NEST(4));            // 16  (nested macro expansion)
    out(0, SQUARE(MAX(2, 5)));  // 25  (macro arg is itself a macro call)
}
