// test53 - using-declaration `using N::name;`. Namespaces are transparent on
// this target, so the declaration is a no-op (the name already resolves
// unqualified); the point is that it PARSES without error and the program runs.
// Locks the `KW_USING qualified_id ';'` grammar rule. Its several locals date
// from when the assembler refused a program of 2 data words or fewer.

namespace ns {
    int twice(int x) { return x + x; }
    int inc(int x)   { return x + 1; }
}

using ns::twice;        // using-declaration (the feature under test)
using ns::inc;          // a second one

void main(void) {
    int a = twice(21);  // 42
    int b = inc(a);     // 43
    int c = twice(b);   // 86
    out(0, a);
    out(0, b);
    out(0, c);
}
