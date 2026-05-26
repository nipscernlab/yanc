#pragma yanc prname test38
// 'noexcept' function specifier: ignored on this target (no exceptions). Must
// parse in every spot it's syntactically valid: free function declarations,
// free function definitions, and template-function definitions.

int adder(int a, int b) noexcept { return a + b; }   // free-fn definition
int squarer(int x) noexcept;                          // declaration
int squarer(int x) noexcept { return x * x; }

template<class T>
T maxv(T a, T b) noexcept { return a > b ? a : b; }   // template-fn definition

void main(void) {
    out(0, adder(3, 4));        // 7
    out(0, squarer(6));         // 36
    out(0, maxv(2, 9));         // 9
    out(0, maxv(-1, -5));       // -1
}
