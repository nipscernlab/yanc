// Recursive-descent parser — the lowest grammar level, in a separate source
// file from rdparse.c. parse_factor reads a parenthesised sub-expression
// (calling back into parse_expr across the file boundary) or a multi-digit
// integer literal off the shared cursor.
#include "rdparse.h"

int parse_factor(void) {
    if (*cur == '(') {
        cur = cur + 1;              // consume '('
        int v = parse_expr();
        cur = cur + 1;              // consume ')'
        return v;
    }
    int n = 0;
    while (*cur >= '0' && *cur <= '9') {
        n = n * 10 + (*cur - '0');
        cur = cur + 1;
    }
    return n;
}
