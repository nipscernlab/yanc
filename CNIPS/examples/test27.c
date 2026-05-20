// _Static_assert (C11): compile-time checks over constant expressions
// (literals, sizeof(type) — in words on this target, enum constants, arithmetic).
#pragma yanc prname tst27
#pragma yanc nubits 16

struct P { int x; int y; int z; };
enum { A = 2, B = 5 };

_Static_assert(1, "always true");
_Static_assert(sizeof(struct P) == 3, "P is three words");
_Static_assert(B - A == 3, "enum arithmetic");
static_assert(sizeof(int) == 1, "int is one word here");   // C23 / <assert.h> spelling

void main(void) {
    _Static_assert(2 + 2 == 4, "block scope");
    out(0, sizeof(struct P));   // 3
    out(0, B);                  // 5
    out(0, A * B);              // 10
}
