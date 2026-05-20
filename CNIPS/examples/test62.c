// Recursive-descent expression evaluator (parsing) split across three files:
// rdparse.h (grammar API), rdparse.c (cursor + expr/term), rdfactor.c (factor).
// The grammar's mutual recursion forms a call-graph cycle that SPANS the two
// source units, so the transitive-closure detector must flag expr/term/factor
// across the file boundary. Distinct from test51's iterative shunting-yard:
// here precedence and parenthesis nesting come from the call structure itself.
#pragma yanc prname tst62
#pragma yanc sdepth 128
#pragma yanc ndstac 128

#include "rdparse.h"
#include "rdparse.c"
#include "rdfactor.c"

void main(void) {
    out(0, parse_eval("2+3*4"));           // 14
    out(0, parse_eval("(2+3)*4"));         // 20
    out(0, parse_eval("100-2*(3+4)"));     // 86
    out(0, parse_eval("((1+2)*(3+4))"));   // 21
    out(0, parse_eval("12/3+5"));          // 9
    out(0, parse_eval("2*3+4*5-6"));       // 20
}
