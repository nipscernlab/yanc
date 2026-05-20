// preprocessor macro operators: # (stringize), ## (token paste),
// __VA_ARGS__ variadic macros, and the GNU `, ##__VA_ARGS__` comma elision.
#pragma yanc prname tst23
#pragma yanc nubits 16

#define PASTE(a, b)     a ## b
#define VAR(n)          var ## n
#define STR(x)          #x
#define CALL(f, ...)    f(__VA_ARGS__)
#define ONE(first, ...) sink(first , ## __VA_ARGS__)

int add(int a, int b)        { return a + b; }
int add3(int a, int b, int c){ return a + b + c; }
int sink(int n)              { return n; }

void main(void) {
    out(0, PASTE(1, 2));         // 12  (paste forms the number 12)

    int var7 = 99;
    out(0, VAR(7));              // 99  (paste forms identifier var7)

    char *s = STR(hi);           // "hi"
    out(0, s[0]);                // 'h' = 104
    out(0, s[1]);                // 'i' = 105

    out(0, CALL(add, 3, 4));     // 7
    out(0, CALL(add3, 1, 2, 3)); // 6

    out(0, ONE(5));              // sink(5) = 5  (comma elided, empty __VA_ARGS__)
}
