// ----------------------------------------------------------------------------
// instructions as tables of templates (step P10 of #93) -----------------------
// ----------------------------------------------------------------------------
// An operand is in one of six places: an int, a float or a comp, in memory or
// in the acc (a comp constant and a comp variable are both a comp in memory).
// An emitter that writes one sequence per pair of places keeps them in a
// PL_N x PL_N table and calls emit_tpl; tpl.c says what a template may hold.
// ----------------------------------------------------------------------------

#ifndef CMM_TPL_H
#define CMM_TPL_H

#include "ast.h"   // expr

enum { PL_IM, PL_IA, PL_FM, PL_FA, PL_CM, PL_CA, PL_N };

int  place_of    (expr e);                                   // the place of an operand
void emit_tpl    (const char *t, expr e1, expr e2);          // emits template t
void emit_tpl_ops(const char *t, expr e1, expr e2, const char *const *ops); // with $1..$9

#endif // CMM_TPL_H
