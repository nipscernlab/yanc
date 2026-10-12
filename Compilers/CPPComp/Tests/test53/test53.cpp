// test53 - using-declaration `using N::name;`. Namespaces are transparent on
// this target, so the declaration is a no-op (the name already resolves
// unqualified); the point is that it PARSES without error and the program runs.
// Locks the `KW_USING qualified_id ';'` grammar rule. Uses several locals so the
// program clears appcomp's "needs a data memory" floor (var_cnt > 2).
#pragma yanc prname test53

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
