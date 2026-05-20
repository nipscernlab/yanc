// Recursive-descent parser — cursor + the two higher grammar levels.
// expr and term recurse (expr->term->factor->expr is a cycle spanning all
// three functions and BOTH source files), so the compiler gives them stack
// frames; their scalar locals `v`/`op` each get a per-level frame slot.
#include "rdparse.h"

static const char *cur;   // shared parse cursor (single translation unit)

int parse_eval(const char *s) {
    cur = s;
    return parse_expr();
}

int parse_expr(void) {
    int v = parse_term();
    while (*cur == '+' || *cur == '-') {
        char op = *cur;
        cur = cur + 1;
        if (op == '+') v = v + parse_term();
        else           v = v - parse_term();
    }
    return v;
}

int parse_term(void) {
    int v = parse_factor();
    while (*cur == '*' || *cur == '/') {
        char op = *cur;
        cur = cur + 1;
        if (op == '*') v = v * parse_factor();
        else           v = v / parse_factor();
    }
    return v;
}
