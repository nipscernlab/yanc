// ----------------------------------------------------------------------------
// the Dirac notation computed in loops (dirac.c) ------------------------------
// ----------------------------------------------------------------------------
// docs/design/dirac-expressions.md. The expression comes checked for shape
// and type (ast.c); these turn it into loops.

#ifndef YANC_DIRAC_H
#define YANC_DIRAC_H

#include "ast.h"

void dirac_loops    (int dst, expr_node *r, int line);           // dst # r;
void dirac_loops_out(int port, expr_node *r, int fout, int line); // out(p, r) / fout(p, r), r a vector
expr dirac_number   (expr_node *n);                              // <a|b>, <a|M|b> inside an expression

// the walker's cases for the nodes dirac.c builds
expr dirac_elem_emit(expr_node *n);
expr dirac_sum_emit (expr_node *n);

// after the walker: declares __dirac_tmp in the .asm when a statement used it,
// and prints the summary
void dirac_finish(const char *asm_file);

#endif
