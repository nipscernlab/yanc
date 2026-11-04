// ----------------------------------------------------------------------------
// the accumulator: what it holds, in one place (step P9 of #93) ---------------
// ----------------------------------------------------------------------------

#ifndef CMM_ACC_H
#define CMM_ACC_H

// 1 when the acc holds a value an expression still needs: the next operand
// load pushes it (P_LOD) instead of overwriting it (LOD)
extern int acc_live;

// forget what the acc holds: at every basic-block boundary (labels, capture
// push/pop, macro use/end), so the drop of a LOD never crosses control flow
void emit_peephole_reset(void);

// true when the acc is known to hold `name`, a variable's or a key
int  acc_holds(const char *name);

// after an indexed store or read, names the acc by the element, "v[k]" (ast.c)
void acc_holds_elem(const char *key);

// the acc holds this value now, and nothing else: a variable's name, or a key
// of a value that is no variable. A key in brackets, "[k]" (an index, ast.c)
// or "[dk0+dd1]" (a sum of the Dirac loops' variables, dirac.c), goes at the
// next SET, which may change what it is made of
void acc_now(const char *key);

// add_instr's two questions: is this instruction a LOD of what the acc holds
// (then it is dropped), and, once it is emitted, what does the acc hold
int  acc_has_load(const char *str);
void acc_after(char *str);

#endif // CMM_ACC_H
