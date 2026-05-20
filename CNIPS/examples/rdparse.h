// Recursive-descent arithmetic parser — public API. The grammar:
//   expr   := term   (('+' | '-') term)*
//   term   := factor (('*' | '/') factor)*
//   factor := '(' expr ')' | number
// Evaluates a NUL-terminated expression string left-to-right with correct
// precedence and parenthesis nesting. Implemented across two .c files; the
// shared cursor lives in rdparse.c.
#ifndef RDPARSE_H
#define RDPARSE_H

int parse_eval(const char *s);   // evaluate the whole expression string

// internal mutual-recursion entry points (declared so the two .c files that
// implement the grammar can call across the file boundary)
int parse_expr(void);
int parse_term(void);
int parse_factor(void);

#endif
