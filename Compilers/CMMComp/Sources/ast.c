// ----------------------------------------------------------------------------
// AST nodes for statements (see ast.h for the design notes) ------------------
// ----------------------------------------------------------------------------

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../Headers/ast.h"
#include "../Headers/global.h"
#include "../Headers/variaveis.h"   // v_table[].name for the dump
#include "../Headers/diretivas.h"   // nbmant / nbexpo for the constant-fold range
#include "../Headers/messages.h"

// needed by the expression-tree walker (ast_emit_expr) below
#include "../Headers/oper.h"
#include "../Headers/stdlib.h"
#include "../Headers/array_index.h"
#include "../Headers/data_use.h"
#include "../Headers/funcoes.h"

// needed by the stmt_node walker
#include "../Headers/data_assign.h"
#include "../Headers/data_declar.h"  // declar_arr_*_emit for STMT_DECLAR_* walker
#include "../Headers/itr.h"
#include "../Headers/t2t.h"      // get_cmp_cst for complex cond loads

// ----------------------------------------------------------------------------
// expressions ----------------------------------------------------------------
// ----------------------------------------------------------------------------

expr expr_make(int type, int id)
{
    expr e = {type, id};
    return e;
}

// ----------------------------------------------------------------------------
// expression AST constructors / destructor -----------------------------------
// ----------------------------------------------------------------------------
// Every `exp` reduction in CMMComp.y calls one of these to build the subtree
// it just parsed. Codegen happens later in the walker.

static expr_node *enode_new(expr_kind k)
{
    expr_node *n = calloc(1, sizeof(*n));
    if (!n) {fprintf(stderr, MSG_ERR_OUT_OF_MEMORY); exit(EXIT_FAILURE);}
    n->kind = k;
    n->line = line_num + 1;
    return n;
}

expr_node *expr_lit(int type, int id)
{
    expr_node *n = enode_new(EXPR_LITERAL);
    n->type = type;
    n->id   = id;
    return n;
}

expr_node *expr_var(int type, int id)
{
    expr_node *n = enode_new(EXPR_VAR);
    n->type = type;
    n->id   = id;
    return n;
}

// type is intentionally not a parameter: composite nodes carry n->type = 0
// out of enode_new (calloc), and typecheck_expr fills it bottom-up before
// codegen runs.
expr_node *expr_binop(int op, expr_node *left, expr_node *right)
{
    expr_node *n = enode_new(EXPR_BINOP);
    n->op    = op;
    n->left  = left;
    n->right = right;
    return n;
}

expr_node *expr_unop(int op, expr_node *operand)
{
    expr_node *n = enode_new(EXPR_UNOP);
    n->op   = op;
    n->left = operand;
    return n;
}

expr_node *expr_array_index(int type, int id, int reversed,
                            expr_node *idx, expr_node *idx2)
{
    expr_node *n = enode_new(EXPR_ARRAY_INDEX);
    n->type  = type;
    n->id    = id;        // array variable index
    n->op    = reversed;  // 0=x[i], 1=x[i)
    n->left  = idx;
    n->right = idx2;      // NULL for 1D
    return n;
}

expr_node *expr_pplus(int type, int id, expr_node *idx, expr_node *idx2)
{
    expr_node *n = enode_new(EXPR_PPLUS);
    n->type  = type;
    n->id    = id;        // variable being post-incremented
    n->left  = idx;       // NULL for scalar id++
    n->right = idx2;      // NULL unless 2D
    return n;
}

expr_node *expr_stdlib(int op, int port, expr_node *a, expr_node *b)
{
    expr_node *n = enode_new(EXPR_STDLIB_CALL);
    n->op    = op;
    n->id    = port;      // INUM port for IN/FIN/OUT, 0 for compute-only calls
    n->left  = a;
    n->right = b;
    return n;
}

expr_node *expr_func_call(int type, int id, expr_node **args, int n_args)
{
    expr_node *n = enode_new(EXPR_FUNC_CALL);
    n->type   = type;
    n->id     = id;       // function's v_table index
    n->args   = args;     // takes ownership
    n->n_args = n_args;
    return n;
}

expr_node *expr_inner(expr_node *a, expr_node *b)
{
    expr_node *n = enode_new(EXPR_INNER);
    n->left  = a;
    n->right = b;
    return n;
}

void expr_free(expr_node *n)
{
    if (!n) return;
    expr_free(n->left);
    expr_free(n->right);
    for (int i = 0; i < n->n_args; i++) expr_free(n->args[i]);
    free(n->args);
    free(n);
}

// ----------------------------------------------------------------------------
// debugging dump -------------------------------------------------------------
// ----------------------------------------------------------------------------

static const char *kind_name(expr_kind k)
{
    switch (k) {
        case EXPR_LITERAL:     return "LITERAL";
        case EXPR_VAR:         return "VAR";
        case EXPR_BINOP:       return "BINOP";
        case EXPR_UNOP:        return "UNOP";
        case EXPR_ARRAY_INDEX: return "ARRAY_INDEX";
        case EXPR_PPLUS:       return "PPLUS";
        case EXPR_STDLIB_CALL: return "STDLIB_CALL";
        case EXPR_FUNC_CALL:   return "FUNC_CALL";
        case EXPR_INNER:       return "INNER";
    }
    return "?";
}

static const char *type_name(int t)
{
    switch (t) {
        case 0: return "void";
        case 1: return "int";
        case 2: return "float";
        case 3: return "comp";
        case 4: return "comp_im";
        case 5: return "comp_const";
    }
    return "?";
}

static void expr_dump_at(expr_node *n, int depth)
{
    if (!n) return;
    for (int i = 0; i < depth; i++) fputs("  ", stderr);
    fprintf(stderr, "%s type=%s line=%d", kind_name(n->kind), type_name(n->type), n->line);
    switch (n->kind) {
        case EXPR_LITERAL:
        case EXPR_VAR:
            fprintf(stderr, " id=%d (%s)", n->id, n->id ? v_table[n->id].name : "<acc>");
            break;
        case EXPR_BINOP:
        case EXPR_UNOP:
            fprintf(stderr, " op=%d", n->op);
            break;
        case EXPR_ARRAY_INDEX:
            fprintf(stderr, " id=%d (%s) reversed=%d", n->id, v_table[n->id].name, n->op);
            break;
        case EXPR_PPLUS:
            fprintf(stderr, " id=%d (%s)", n->id, v_table[n->id].name);
            break;
        case EXPR_STDLIB_CALL:
            fprintf(stderr, " op=%d port=%d", n->op, n->id);
            break;
        case EXPR_FUNC_CALL:
            fprintf(stderr, " id=%d (%s) n_args=%d", n->id, v_table[n->id].name, n->n_args);
            break;
        case EXPR_INNER:
            /* left/right (vector refs) get printed by the recursive walk */
            break;
    }
    fputc('\n', stderr);
    expr_dump_at(n->left,  depth + 1);
    expr_dump_at(n->right, depth + 1);
    for (int i = 0; i < n->n_args; i++) expr_dump_at(n->args[i], depth + 1);
}

void expr_dump(expr_node *n)
{
    expr_dump_at(n, 0);
}

// ----------------------------------------------------------------------------
// expression tree walker (codegen) -------------------------------------------
// ----------------------------------------------------------------------------
// Single entry point for expression codegen. Dispatches on n->kind, recurses
// into children, and calls the existing oper_*/exec_*/arr_*/pplus_*/num2exp/
// id2exp/par_check helpers in left-to-right post-order. The traversal order
// matches what the parser used to reduce, so the emit stream is byte-identical
// to the previous inline-emit compiler.
//
// Result POD carries the freshly-emitted value's location: id == 0 means
// "in the accumulator", id > 0 means "in v_table[id].name".

static expr ast_emit_expr_impl(expr_node *n);

// True iff n is an EXPR_LITERAL whose numeric value equals `want` (e.g. 0 or
// 1). Used by the algebraic-identity folder in ast_emit_expr_impl. The
// literal text is whatever the lexer matched ("0", "0.0", "1", "1.0", ...);
// atof() gives an exact result for these inputs, so == comparison is safe.
static int is_const_value(expr_node *n, double want)
{
    if (!n || n->kind != EXPR_LITERAL) return 0;
    // Only a genuine scalar (int/float) literal can be the algebraic 0 or 1. A
    // comp literal (type 3/4/5) is NOT -- e.g. 1+2i has real part 1 but is not
    // the constant 1 -- so matching its real half here would wrongly fold
    // 1*x / x/1 / x+0 and drop the operation (e.g. (1+2i)*(3+4i) -> (3+4i)).
    if (n->type > 2) return 0;
    return atof(v_table[n->id].name) == want;
}

// Typecheck pass: walks bottom-up and annotates n->type on composite nodes
// whose type the parser leaves at 0 (BINOP / UNOP today; INNER / STDLIB_CALL
// pending follow-up commits). Leaves (LITERAL / VAR / ARRAY_INDEX / PPLUS /
// FUNC_CALL) already carry the correct type, so the early-return makes the
// walk idempotent: a second call over the same subtree is O(1) per node.
//
// Codegen does not consume the annotation yet - this commit just stages the
// data. Subsequent commits cut oper_* / exec_* helpers over to trust
// n->type, removing the per-helper "compute result type" tail.
void typecheck_expr(expr_node *n)
{
    if (!n)           return;
    if (n->type != 0) return;  // already typed (leaf, or prior pass)

    typecheck_expr(n->left);
    typecheck_expr(n->right);
    for (int i = 0; i < n->n_args; i++) typecheck_expr(n->args[i]);

    int lt = n->left  ? n->left->type  : 0;
    int rt = n->right ? n->right->type : 0;

    switch (n->kind)
    {
        case EXPR_BINOP:
            switch (n->op)
            {
                // arithmetic: result type is max(left, right) where comp(3) > float(2) > int(1)
                case OP_ADD: case OP_SUB: case OP_MUL: case OP_DIV: case OP_MOD:
                    n->type = (lt > rt) ? lt : rt; break;
                // comparisons / logical / bitwise / shifts all yield int
                case OP_LT:  case OP_GT:  case OP_EQ:
                case OP_GE:  case OP_LE:  case OP_NE:
                case OP_LAN: case OP_LOR:
                case OP_SHL: case OP_SHR: case OP_SSHR:
                case OP_AND: case OP_OR:  case OP_XOR:
                    n->type = 1; break;
                default: break;
            }
            break;

        case EXPR_UNOP:
            switch (n->op)
            {
                case OP_NEG:              n->type = lt; break;   // preserves operand type
                case OP_LIN: case OP_INV: n->type = 1;  break;   // int result
                default: break;
            }
            break;

        case EXPR_STDLIB_CALL:
            switch (n->op)
            {
                case OP_STD_IN:                                  n->type = 1;  break;
                case OP_STD_FIN:                                 n->type = 2;  break;
                case OP_STD_PST: case OP_STD_ABS:                n->type = lt; break;
                case OP_STD_SIGN:                                n->type = rt; break;
                case OP_STD_NRM:                                 n->type = 1;  break;
                // these accept comp: comp argument -> comp result, real arg -> float
                case OP_STD_SQRT: case OP_STD_ATAN:
                case OP_STD_SIN:  case OP_STD_COS:  case OP_STD_TAN:
                case OP_STD_EXP:  case OP_STD_LOG:               n->type = (lt > 2) ? 3 : 2;  break;
                // real-only (reject comp) -> always float
                case OP_STD_COSH: case OP_STD_SINH: case OP_STD_TANH:
                case OP_STD_POW:
                case OP_STD_FLOOR: case OP_STD_CEIL: case OP_STD_ROUND: n->type = 2;  break;
                case OP_STD_REAL: case OP_STD_IMAG:
                case OP_STD_FASE: case OP_STD_MOD2:              n->type = 2;  break;
                case OP_STD_COMP: case OP_STD_CONJ:              n->type = 3;  break;
                default: break;
            }
            break;

        case EXPR_INNER:
            // <a|b> reduces via exec_vtv, which returns the element type of
            // the first operand. left is an EXPR_VAR whose type already
            // carries v_table[id].type, so just propagate.
            n->type = lt;
            break;

        default: break;
    }
}

// Sethi-Ullman labelling: how many hardware-stack slots the subtree needs
// while being evaluated. A leaf (var/literal) is a memory operand and needs
// none; a binop with two complex children needs one extra slot to hold the
// first result while the second is computed. Used only to choose the
// evaluation order of a commutative op (heavier child first -> fewer pushes /
// spills on the shallow NDSTAC stack). A heuristic: correctness never depends
// on it, because reordering is restricted to commutative ops.
static int is_expr_leaf(const expr_node *n)
{
    return n && (n->kind == EXPR_LITERAL || n->kind == EXPR_VAR);
}

static int stack_need(expr_node *n)
{
    if (!n) return 0;
    switch (n->kind) {
        case EXPR_LITERAL:
        case EXPR_VAR:
            return 0;
        case EXPR_UNOP:
            return stack_need(n->left);
        case EXPR_BINOP: {
            // A leaf operand is a memory operand (free); a non-leaf child must
            // go through the acc, so two complex children force one slot to
            // hold the first result while the second is computed -- even when
            // each child's own need is 0 (e.g. (c+d)+(e+f) needs 1).
            int leafL = is_expr_leaf(n->left), leafR = is_expr_leaf(n->right);
            if (leafL && leafR) return 0;            // a OP b
            if (leafR)          return stack_need(n->left);   // complexL OP leafR
            if (leafL)          return stack_need(n->right);  // leafL OP complexR
            int l = stack_need(n->left), r = stack_need(n->right);
            return (l == r) ? l + 1 : (l > r ? l : r);
        }
        default:            // array index / pplus / stdlib / func call / Dirac
            return 1;       // complex value -> conservatively one slot
    }
}

// Commutative integer ops whose acc+acc codegen is a *symmetric* stack op
// (S_ADD / S_MLT), so swapping operand evaluation order is value-preserving.
// Deliberately NOT float (reassociation would change rounding) and NOT the
// other commutative ops until their acc+acc paths are likewise confirmed.
static int commutative_int_binop(const expr_node *n)
{
    return n->type == 1 && (n->op == OP_ADD || n->op == OP_MUL);
}

// Constant folding. const_eval recursively evaluates a subtree as a
// compile-time int constant: it succeeds (returns 1, sets *val) only for an int
// literal, or +/-/*/&/|/^ of foldable constants whose result -- AT EVERY LEVEL
// -- stays non-negative and within the signed NUBITS-bit range. That guard is
// what makes the C-side arithmetic match the hardware: if any intermediate
// would overflow or go negative, we bail and let the runtime op handle the
// NUBITS wraparound / negation (so e.g. (30000+30000) is NOT folded). Float is
// never folded (the YANC float is not IEEE-bit-compatible); div/mod/shift/
// comparison/logical ops are left to the runtime too.
static int const_eval(expr_node *n, long *val)
{
    if (!n) return 0;
    // Range that keeps the result both representable and emittable: a positive
    // result becomes a literal, a negative one a NEG_M of its magnitude (a
    // literal, so |r| must be <= max). max = signed NUBITS-bit max; min = -max.
    long max = ((long)1 << (nbmant + nbexpo)) - 1;
    long min = -max;

    if (n->kind == EXPR_LITERAL && n->type == 1) {
        *val = atol(v_table[n->id].name);
        return 1;
    }
    if (n->kind == EXPR_UNOP && n->op == OP_NEG) {       // -(const)
        long a;
        if (!const_eval(n->left, &a)) return 0;
        long r = -a;
        if (r < min || r > max) return 0;
        *val = r;
        return 1;
    }
    if (n->kind == EXPR_BINOP) {
        long a, b, r;
        if (!const_eval(n->left, &a) || !const_eval(n->right, &b)) return 0;
        switch (n->op) {
            case OP_ADD: r = a + b; break;
            case OP_SUB: r = a - b; break;
            case OP_MUL: r = a * b; break;
            // bitwise only on non-negative operands -- a negative 2's-complement
            // operand's high bits beyond NUBITS would not match the hardware
            case OP_AND: if (a < 0 || b < 0) return 0; r = a & b; break;
            case OP_OR:  if (a < 0 || b < 0) return 0; r = a | b; break;
            case OP_XOR: if (a < 0 || b < 0) return 0; r = a ^ b; break;
            default: return 0;
        }
        if (r < min || r > max) return 0;
        *val = r;
        return 1;
    }
    return 0;
}

// Idempotent: a second call over the same node returns the first result
// instead of re-emitting. This guards against double-emission if a tree ever
// gets walked twice (shouldn't happen in the grammar today, but cheap to
// keep as a safety net).
expr ast_emit_expr(expr_node *n)
{
    if (n && n->emitted) return n->cached;
    // save/restore emit_line so that when this call returns, the caller's
    // emit_line is what it set before recursing. That way an operator like
    //   exec_add(ast_emit_expr(L), ast_emit_expr(R))
    // emits the ADD with the binop's line, not whatever the last sub-walk
    // left behind.
    int saved_emit_line = emit_line;
    if (n) emit_line = n->line;
    // line_num drives the warning / MSG_INFO prints inside the emit helpers
    // (oper_*, exec_*, arr_*, par_check, ...). Keep it in lock-step with the
    // node's source line during the walk so those messages report the right line
    // instead of the lexer's EOF position. The helpers print `line_num + 1` and
    // n->line was captured as `line_num + 1` at parse time, so set n->line - 1 to
    // reproduce exactly what a parse-time print would have shown. Restored on the
    // way out so parse-time diagnostics keep using the real lexer line.
    int saved_line_num = line_num;
    if (n) line_num = n->line - 1;
    typecheck_expr(n);  // annotate composite n->type before codegen
    expr e = ast_emit_expr_impl(n);
    if (n) {
        n->emitted = 1;
        n->cached  = e;
    }
    emit_line = saved_emit_line;
    line_num  = saved_line_num;
    return e;
}

static expr ast_emit_expr_impl(expr_node *n)
{
    if (!n) return expr_make(0, 0);

    switch (n->kind)
    {
        case EXPR_LITERAL: return num2exp(n->id, n->type);
        case EXPR_VAR:     return id2exp (n->id);

        case EXPR_BINOP: {
            // Algebraic identity folding (constant operand drops out, surviving
            // subtree is emitted directly). Only fire when the surviving
            // operand's type matches the binop's result type - otherwise a
            // hidden int->float / float->comp promotion would be lost.
            switch (n->op) {
                case OP_ADD:
                    if (is_const_value(n->right, 0.0) && n->left ->type == n->type) return ast_emit_expr(n->left );
                    if (is_const_value(n->left , 0.0) && n->right->type == n->type) return ast_emit_expr(n->right);
                    break;
                case OP_SUB:
                    if (is_const_value(n->right, 0.0) && n->left ->type == n->type) return ast_emit_expr(n->left );
                    break;
                case OP_MUL:
                    if (is_const_value(n->right, 1.0) && n->left ->type == n->type) return ast_emit_expr(n->left );
                    if (is_const_value(n->left , 1.0) && n->right->type == n->type) return ast_emit_expr(n->right);
                    break;
                case OP_DIV:
                    if (is_const_value(n->right, 1.0) && n->left ->type == n->type) return ast_emit_expr(n->left );
                    break;
            }

            // Constant folding: a fully-constant int subtree collapses to one
            // literal. A non-negative result is just that literal (num2exp,
            // no emit); a negative one has no literal form, so it becomes a
            // NEG_M of its magnitude via oper_neg (e.g. 3-5 -> NEG_M 2).
            {
                long cval;
                if (const_eval(n, &cval)) {
                    char buf[32];
                    if (cval >= 0) {
                        snprintf(buf, sizeof buf, "%ld", cval);
                        return num2exp(exec_inum(buf), 1);
                    }
                    snprintf(buf, sizeof buf, "%ld", -cval);
                    return oper_neg(num2exp(exec_inum(buf), 1));
                }
            }

            // Sethi-Ullman ordering: for a commutative int op, evaluate the
            // heavier subtree first so its result spends less time pushed on
            // the stack (fewer pushes / spills). a/b stay bound to left/right,
            // so the dispatch below is unchanged; S_ADD / S_MLT are symmetric
            // so the swapped acc+acc order yields the same value.
            expr a, b;
            if (commutative_int_binop(n) && stack_need(n->right) > stack_need(n->left)) {
                b = ast_emit_expr(n->right);
                a = ast_emit_expr(n->left );
            } else {
                a = ast_emit_expr(n->left );
                b = ast_emit_expr(n->right);
            }
            switch (n->op) {
                case OP_ADD:  return oper_soma (a, b);
                case OP_SUB:  return oper_subt (a, b);
                case OP_MUL:  return oper_mult (a, b);
                case OP_DIV:  return oper_divi (a, b);
                case OP_MOD:  return oper_mod  (a, b);
                case OP_LT:   return oper_cmp  (a, b, 0);
                case OP_GT:   return oper_cmp  (a, b, 1);
                case OP_EQ:   return oper_cmp  (a, b, 2);
                case OP_GE:   return oper_greq (a, b);
                case OP_LE:   return oper_leeq (a, b);
                case OP_NE:   return oper_dife (a, b);
                case OP_LAN:  return oper_lanor(a, b, 0);
                case OP_LOR:  return oper_lanor(a, b, 1);
                case OP_SHL:  return oper_shift(a, b, 0);
                case OP_SHR:  return oper_shift(a, b, 1);
                case OP_SSHR: return oper_shift(a, b, 2);
                case OP_AND:  return oper_bitw (a, b, 0);
                case OP_OR:   return oper_bitw (a, b, 1);
                case OP_XOR:  return oper_bitw (a, b, 2);
                default: break;
            }
            break;
        }

        case EXPR_UNOP: {
            expr a = ast_emit_expr(n->left);
            switch (n->op) {
                case OP_NEG: return oper_neg(a);
                case OP_LIN: return oper_lin(a);
                case OP_INV: return oper_inv(a);
                default: break;
            }
            break;
        }

        case EXPR_ARRAY_INDEX: {
            // Constant 1D forward index into an int array: read directly with
            // LOD_V (offset baked into the address), skipping the index
            // materialise + indirect LDI. Other shapes (2D, reversed, float /
            // comp array) fall through to the indirect path.
            if (!n->right && n->op == 0
                && n->left->kind == EXPR_LITERAL && v_table[n->id].type == 1)
            {
                typecheck_expr(n->left);
                if (n->left->type == 1)
                    return arr_1d2exp_const(n->id, atoi(v_table[n->left->id].name));
            }
            expr idx1 = ast_emit_expr(n->left);
            if (n->right) {
                expr idx2 = ast_emit_expr(n->right);
                return arr_2d2exp(n->id, idx1, idx2);
            }
            return arr_1d2exp(n->id, idx1, n->op);  // op = reversed flag
        }

        case EXPR_PPLUS: {
            if (!n->left)  return pplus2exp(n->id);
            if (!n->right) {
                expr idx = ast_emit_expr(n->left);
                return pplus1d2exp(n->id, idx);
            }
            expr i1 = ast_emit_expr(n->left );
            expr i2 = ast_emit_expr(n->right);
            return pplus2d2exp(n->id, i1, i2);
        }

        case EXPR_STDLIB_CALL: {
            switch (n->op) {
                case OP_STD_IN:   return exec_in (n->id);
                case OP_STD_FIN:  return exec_fin(n->id);
                case OP_STD_PST:  { expr a = ast_emit_expr(n->left); return exec_pst (a); }
                case OP_STD_ABS:  { expr a = ast_emit_expr(n->left); return exec_abs (a); }
                case OP_STD_SIGN: { expr a = ast_emit_expr(n->left); expr b = ast_emit_expr(n->right); return exec_sign(a, b); }
                case OP_STD_NRM:  { expr a = ast_emit_expr(n->left); return exec_norm(a); }
                case OP_STD_SQRT: { expr a = ast_emit_expr(n->left); return exec_sqrt(a); }
                case OP_STD_ATAN: { expr a = ast_emit_expr(n->left); return exec_atan(a); }
                case OP_STD_SIN:  { expr a = ast_emit_expr(n->left); return exec_sin (a); }
                case OP_STD_COS:  { expr a = ast_emit_expr(n->left); return exec_cos (a); }
                case OP_STD_TAN:  { expr a = ast_emit_expr(n->left); return exec_tan (a); }
                case OP_STD_COSH: { expr a = ast_emit_expr(n->left); return exec_cosh(a); }
                case OP_STD_SINH: { expr a = ast_emit_expr(n->left); return exec_sinh(a); }
                case OP_STD_TANH: { expr a = ast_emit_expr(n->left); return exec_tanh(a); }
                case OP_STD_EXP:  { expr a = ast_emit_expr(n->left); return exec_exp (a); }
                case OP_STD_LOG:  { expr a = ast_emit_expr(n->left); return exec_log (a); }
                case OP_STD_POW:  { expr a = ast_emit_expr(n->left); expr b = ast_emit_expr(n->right); return exec_pow (a, b); }
                case OP_STD_FLOOR:{ expr a = ast_emit_expr(n->left); return exec_floor(a); }
                case OP_STD_CEIL: { expr a = ast_emit_expr(n->left); return exec_ceil (a); }
                case OP_STD_ROUND:{ expr a = ast_emit_expr(n->left); return exec_round(a); }
                case OP_STD_REAL: { expr a = ast_emit_expr(n->left); return exec_real(a); }
                case OP_STD_IMAG: { expr a = ast_emit_expr(n->left); return exec_imag(a); }
                case OP_STD_COMP: { expr a = ast_emit_expr(n->left); expr b = ast_emit_expr(n->right); return exec_comp(a, b); }
                case OP_STD_FASE: { expr a = ast_emit_expr(n->left); return exec_fase(a); }
                case OP_STD_MOD2: { expr a = ast_emit_expr(n->left); return exec_mod2(a); }
                case OP_STD_CONJ: { expr a = ast_emit_expr(n->left); return exec_conj(a); }
                default: break;
            }
            break;
        }

        case EXPR_FUNC_CALL: {
            // save/restore the per-call globals so nested calls behave
            int saved_fun_id = fun_id;
            int saved_p_test = p_test;
            fun_id = n->id;
            p_test = 0;

            for (int i = 0; i < n->n_args; i++) {
                expr a = ast_emit_expr(n->args[i]);
                p_test = p_test * 10 + a.type;
                par_check(a);
                acc_ok = 1;
            }

            add_instr("CAL %s\n", v_table[n->id].name);
            v_table[n->id].used = 1;
            acc_ok = (n->type == 0) ? 0 : 1;

            fun_id = saved_fun_id;
            p_test = saved_p_test;
            return expr_make(n->type, 0);
        }

        case EXPR_INNER: {
            // left / right are EXPR_VAR children carrying the vector ids
            return exec_vtv(n->left->id, n->right->id);
        }
    }
    fprintf(stderr, "ast_emit_expr: unhandled kind=%d op=%d at line %d\n",
            n->kind, n->op, n->line);
    exit(EXIT_FAILURE);
}

// ----------------------------------------------------------------------------
// statement AST --------------------------------------------------------------
// ----------------------------------------------------------------------------
//
// Each top-level statement reduces into a stmt_node and the grammar appends
// it to the current body's STMT_BLOCK via stmt_append. The walker fires
// at body close (func_ret / if_stmt / if_fim / while_stmt / end_switch) and
// emits every kid in source order.

static stmt_node *snode_new(stmt_kind k)
{
    stmt_node *n = calloc(1, sizeof(*n));
    if (!n) {fprintf(stderr, MSG_ERR_OUT_OF_MEMORY); exit(EXIT_FAILURE);}
    n->kind = k;
    n->line = line_num + 1;
    return n;
}

stmt_node *stmt_assign(int id, expr_node *rhs)
{
    stmt_node *n = snode_new(STMT_ASSIGN);
    n->id  = id;
    n->rhs = rhs;
    return n;
}

stmt_node *stmt_pplus(int id, expr_node *idx, expr_node *idx2)
{
    stmt_node *n = snode_new(STMT_PPLUS);
    n->id   = id;
    n->idx  = idx;
    n->idx2 = idx2;
    return n;
}

stmt_node *stmt_array_assign(int id, expr_node *idx, expr_node *idx2,
                             expr_node *rhs, int fft)
{
    stmt_node *n = snode_new(STMT_ARRAY_ASSIGN);
    n->id   = id;
    n->idx  = idx;
    n->idx2 = idx2;
    n->rhs  = rhs;
    n->op   = fft;
    return n;
}

stmt_node *stmt_return(expr_node *rhs)
{
    stmt_node *n = snode_new(STMT_RETURN);
    n->rhs = rhs;
    return n;
}

stmt_node *stmt_out(int port, expr_node *rhs, int fout_flag)
{
    stmt_node *n = snode_new(STMT_OUT);
    n->id  = port;
    n->rhs = rhs;
    n->op  = fout_flag;
    return n;
}

stmt_node *stmt_copy(expr_node *rhs, int dst_id)
{
    stmt_node *n = snode_new(STMT_COPY);
    n->id  = dst_id;
    n->rhs = rhs;
    return n;
}

stmt_node *stmt_vout(int port, expr_node *rhs, int vector_id)
{
    stmt_node *n = snode_new(STMT_VOUT);
    n->id  = port;
    n->id2 = vector_id;
    n->rhs = rhs;
    return n;
}

stmt_node *stmt_void_call(expr_node *call)
{
    stmt_node *n = snode_new(STMT_VOID_CALL);
    n->rhs = call;
    return n;
}

stmt_node *stmt_dire_inter(void)
{
    return snode_new(STMT_DIRE_INTER);
}

stmt_node *stmt_dire_toaqui(void)
{
    return snode_new(STMT_DIRE_TOAQUI);
}

// ----------------------------------------------------------------------------
// Dirac linear-algebra constructors ------------------------------------------
// ----------------------------------------------------------------------------

static stmt_node *dirac_new(dirac_op op)
{
    stmt_node *n = snode_new(STMT_DIRAC);
    n->op = op;
    return n;
}

stmt_node *stmt_dirac_Mv(int dst, int M, int a)
{
    stmt_node *n = dirac_new(DIRAC_MV);
    n->id  = dst;
    n->id2 = M;
    n->id3 = a;
    return n;
}

stmt_node *stmt_dirac_cv(int dst, expr_node *c, int b)
{
    stmt_node *n = dirac_new(DIRAC_CV);
    n->id  = dst;
    n->id2 = b;
    n->rhs = c;
    return n;
}

stmt_node *stmt_dirac_apcb(int dst, int b, expr_node *c, int d)
{
    stmt_node *n = dirac_new(DIRAC_APCB);
    n->id  = dst;
    n->id2 = b;
    n->id3 = d;
    n->rhs = c;
    return n;
}

stmt_node *stmt_dirac_vvt(int dst, int a, int b)
{
    stmt_node *n = dirac_new(DIRAC_VVT);
    n->id  = dst;
    n->id2 = a;
    n->id3 = b;
    return n;
}

stmt_node *stmt_dirac_Mmvvt(int dst, int B, int a, int b)
{
    stmt_node *n = dirac_new(DIRAC_MMVVT);
    n->id  = dst;
    n->id2 = B;
    n->id3 = a;
    n->id4 = b;
    return n;
}

stmt_node *stmt_dirac_cM(int dst, expr_node *c, int M)
{
    stmt_node *n = dirac_new(DIRAC_CM);
    n->id  = dst;
    n->id2 = M;
    n->rhs = c;
    return n;
}

stmt_node *stmt_dirac_cI(int dst, expr_node *c)
{
    stmt_node *n = dirac_new(DIRAC_CI);
    n->id  = dst;
    n->rhs = c;
    return n;
}

stmt_node *stmt_dirac_v0(int dst)
{
    stmt_node *n = dirac_new(DIRAC_V0);
    n->id  = dst;
    return n;
}

stmt_node *stmt_dirac_cvin(int dst, expr_node *c, int port)
{
    stmt_node *n = dirac_new(DIRAC_CVIN);
    n->id  = dst;
    n->id2 = port;
    n->rhs = c;
    return n;
}

stmt_node *stmt_dirac_shift(int dst, expr_node *c, int a)
{
    stmt_node *n = dirac_new(DIRAC_SHIFT);
    n->id  = dst;
    n->id2 = a;
    n->rhs = c;
    return n;
}

// ----------------------------------------------------------------------------
// STMT_BLOCK + stmt_list scaffold stack --------------------------------------
// ----------------------------------------------------------------------------

stmt_node *stmt_block(void)
{
    return snode_new(STMT_BLOCK);
}

void stmt_block_push(stmt_node *blk, stmt_node *kid)
{
    if (blk->kids_n + 1 > blk->kids_cap)
    {
        int new_cap = blk->kids_cap ? blk->kids_cap * 2 : 8;
        stmt_node **t = realloc(blk->kids, (size_t)new_cap * sizeof(*t));
        if (!t) {fprintf(stderr, MSG_ERR_OUT_OF_MEMORY); exit(EXIT_FAILURE);}
        blk->kids     = t;
        blk->kids_cap = new_cap;
    }
    blk->kids[blk->kids_n++] = kid;
}

static stmt_node **stmt_list_stack     = NULL;
static int         stmt_list_stack_n   = 0;
static int         stmt_list_stack_cap = 0;

void stmt_list_open(void)
{
    if (stmt_list_stack_n + 1 > stmt_list_stack_cap)
    {
        int new_cap = stmt_list_stack_cap ? stmt_list_stack_cap * 2 : 8;
        stmt_node **t = realloc(stmt_list_stack, (size_t)new_cap * sizeof(*t));
        if (!t) {fprintf(stderr, MSG_ERR_OUT_OF_MEMORY); exit(EXIT_FAILURE);}
        stmt_list_stack     = t;
        stmt_list_stack_cap = new_cap;
    }
    stmt_list_stack[stmt_list_stack_n++] = stmt_block();
}

stmt_node *stmt_list_close(void)
{
    if (stmt_list_stack_n == 0) return NULL;
    return stmt_list_stack[--stmt_list_stack_n];
}

stmt_node *stmt_list_top(void)
{
    if (stmt_list_stack_n == 0) return NULL;
    return stmt_list_stack[stmt_list_stack_n - 1];
}

// ----------------------------------------------------------------------------
// compound control-flow constructors -----------------------------------------
// ----------------------------------------------------------------------------

stmt_node *stmt_if(int label, expr_node *cond, stmt_node *then_body, stmt_node *else_body)
{
    stmt_node *n = snode_new(STMT_IF);
    n->id        = label;
    n->cond      = cond;
    n->then_body = then_body;
    n->else_body = else_body;
    return n;
}

stmt_node *stmt_while(int label, expr_node *cond, stmt_node *body)
{
    stmt_node *n = snode_new(STMT_WHILE);
    n->id   = label;
    n->cond = cond;
    n->body = body;
    return n;
}

stmt_node *stmt_do(int label, expr_node *cond, stmt_node *body)
{
    stmt_node *n = snode_new(STMT_DO);
    n->id   = label;
    n->cond = cond;
    n->body = body;
    return n;
}

stmt_node *stmt_switch(int swit_id, int case_max, expr_node *cond, stmt_node *body)
{
    stmt_node *n = snode_new(STMT_SWITCH);
    n->id   = swit_id;
    n->id2  = case_max;
    n->cond = cond;
    n->body = body;
    return n;
}

stmt_node *stmt_case_label(int swit_id, int case_idx, int val_id, int val_type)
{
    stmt_node *n = snode_new(STMT_CASE_LABEL);
    n->id  = swit_id;
    n->id2 = case_idx;
    n->id3 = val_id;
    n->id4 = val_type;
    return n;
}

stmt_node *stmt_default_label(int swit_id, int case_idx)
{
    stmt_node *n = snode_new(STMT_DEFAULT_LABEL);
    n->id  = swit_id;
    n->id2 = case_idx;
    return n;
}

stmt_node *stmt_switch_break(int swit_id)
{
    stmt_node *n = snode_new(STMT_SWITCH_BREAK);
    n->id = swit_id;
    return n;
}

stmt_node *stmt_break_while(int while_label)
{
    stmt_node *n = snode_new(STMT_BREAK_WHILE);
    n->id = while_label;
    return n;
}

stmt_node *stmt_continue(int loop_label, int cont_lbl)
{
    stmt_node *n = snode_new(STMT_CONTINUE);
    n->id = loop_label;
    n->op = cont_lbl;   // 1 -> jump to Lwh<n>cont (for step / do-while test); 0 -> loop top
    return n;
}

stmt_node *stmt_declar_arr_1d(int id_var, int id_arg, int id_fname)
{
    stmt_node *n = snode_new(STMT_DECLAR_ARR_1D);
    n->id  = id_var;
    n->id2 = id_arg;
    n->id3 = id_fname;
    return n;
}

stmt_node *stmt_declar_arr_2d(int id_var, int id_x, int id_y, int id_fname)
{
    stmt_node *n = snode_new(STMT_DECLAR_ARR_2D);
    n->id  = id_var;
    n->id2 = id_x;
    n->id3 = id_y;
    n->id4 = id_fname;
    return n;
}

stmt_node *stmt_declar_Mv(int id_var, int id_N, int id_M, int id_v)
{
    stmt_node *n = snode_new(STMT_DECLAR_MV);
    n->id  = id_var;
    n->id2 = id_N;
    n->id3 = id_M;
    n->id4 = id_v;
    return n;
}

stmt_node *stmt_declar_cv(int id_var, int id_N, expr_node *c, int id_v)
{
    stmt_node *n = snode_new(STMT_DECLAR_CV);
    n->id  = id_var;
    n->id2 = id_N;
    n->id3 = id_v;
    n->rhs = c;
    return n;
}

stmt_node *stmt_func_header(int func_id, int needs_jmp_main,
                            int *params, int n_params)
{
    stmt_node *n = snode_new(STMT_FUNC_HEADER);
    n->id       = func_id;
    n->op       = needs_jmp_main;
    n->params   = params;
    n->n_params = n_params;
    return n;
}

stmt_node *stmt_directive(const char *dir, int id)
{
    stmt_node *n = snode_new(STMT_DIRECTIVE);
    n->dir_str  = dir;
    n->id       = id;
    return n;
}

stmt_node *stmt_func(int func_id, stmt_node *body)
{
    stmt_node *n = snode_new(STMT_FUNC);
    n->id       = func_id;
    n->body     = body;
    return n;
}

// ----------------------------------------------------------------------------
// statement walker -----------------------------------------------------------
// ----------------------------------------------------------------------------

// Shared cond-to-int load used by STMT_IF / STMT_WHILE / STMT_SWITCH walker
// cases. Mirrors the if_exp / while_expexp / exec_switch parse-time code from
// the pre-fase-6 saltos.c. The flag selects between MSG_WARN_COND_* (if /
// while) and MSG_WARN_CASE_* (switch) for the float / comp warnings.
static void emit_cond_int_load(expr e, int is_switch)
{
    const char *msg_float = is_switch ? MSG_WARN_CASE_FLOAT : MSG_WARN_COND_FLOAT;
    const char *msg_comp  = is_switch ? MSG_WARN_CASE_COMP  : MSG_WARN_COND_COMP;

    if ((e.type == 1) && (e.id != 0)) add_instr("LOD %s\n", v_table[e.id].name);
    // int acc: nothing

    if ((e.type == 2) && (e.id != 0)) {
        fprintf(stdout, msg_float, line_num+1);
        add_instr("F2I_M %s\n", v_table[e.id].name);
    }
    if ((e.type == 2) && (e.id == 0)) {
        fprintf(stdout, msg_float, line_num+1);
        add_instr("F2I\n");
    }
    if (e.type == 5) {
        fprintf(stdout, msg_comp, line_num+1);
        expr etr, eti;
        get_cmp_cst(e, &etr, &eti);
        add_instr("F2I_M %s\n", v_table[etr.id].name);
    }
    if ((e.type == 3) && (e.id != 0)) {
        fprintf(stdout, msg_comp, line_num+1);
        add_instr("F2I_M %s\n", v_table[e.id].name);
    }
    if ((e.type == 3) && (e.id == 0)) {
        fprintf(stdout, msg_comp, line_num+1);
        add_instr("POP\n");
        add_instr("F2I\n");
    }
}

void stmt_emit(stmt_node *n)
{
    // Idempotent guard: once a node has been emitted, subsequent walker
    // calls on it are no-ops. Useful when an outer block walker reaches
    // a kid that has already been emitted by a prior pass.
    if (!n || n->emitted) return;

    // Save/restore emit_line so nested ast_emit_expr / stmt_emit calls (which
    // overwrite emit_line for their own sub-emissions) leave the statement's
    // line in place for any direct add_instr / helper call that fires after
    // the sub-walks return. Pre-AST, line_num was the lexer's current line
    // at every grammar action, so f_lin entries automatically tracked source
    // location. Post-AST the walker fires after parsing finishes, so we have
    // to set emit_line explicitly from n->line here.
    int saved_emit_line = emit_line;
    emit_line = n->line;
    int saved_line_num = line_num;   // keep warning / MSG_INFO line in sync (see ast_emit_expr)
    line_num = n->line - 1;

    switch (n->kind)
    {
        case STMT_ASSIGN:
            ass_set(n->id, ast_emit_expr(n->rhs));
            break;

        case STMT_PPLUS:
            if      (!n->idx ) ass_pplus(n->id);
            else if (!n->idx2) ass_aplus(n->id, ast_emit_expr(n->idx));
            else               ass_apl2d(n->id, ast_emit_expr(n->idx),
                                                ast_emit_expr(n->idx2));
            break;

        case STMT_ARRAY_ASSIGN:
            // Constant 1D forward index into an int array with an int RHS:
            // store directly with SET_V (the assembler bakes the offset into a
            // plain SET to arr_base+k), skipping the index materialise + stack
            // push + indirect STI. Every other shape (2D, reversed array, or a
            // float/comp array or RHS) falls through to the indirect path.
            if (!n->idx2 && n->op == 0
                && n->idx->kind == EXPR_LITERAL && v_table[n->id].type == 1)
            {
                typecheck_expr(n->idx);
                typecheck_expr(n->rhs);
                if (n->idx->type == 1 && n->rhs->type == 1) {
                    int k = atoi(v_table[n->idx->id].name);
                    ass_array_const(n->id, k, ast_emit_expr(n->rhs));
                    break;
                }
            }
            // First emit the index calculation (leaves it in acc),
            // then the walker on the RHS pushes that, computes the value,
            // and ass_array emits the indexed-store.
            if (n->idx2) arr_2d_index(n->id, ast_emit_expr(n->idx),
                                              ast_emit_expr(n->idx2));
            else         arr_1d_index(n->id, ast_emit_expr(n->idx));
            ass_array(n->id, ast_emit_expr(n->rhs), n->op);
            break;

        case STMT_RETURN:
            if (n->rhs) declar_ret(ast_emit_expr(n->rhs), 1);
            else        void_ret();
            break;

        case STMT_OUT:
            if (n->op) exec_fout(n->id, ast_emit_expr(n->rhs));
            else       exec_out (n->id, ast_emit_expr(n->rhs));
            break;

        case STMT_COPY:
            exec_copy(ast_emit_expr(n->rhs), n->id);
            break;

        case STMT_VOUT:
            exec_vout(n->id, ast_emit_expr(n->rhs), n->id2);
            break;

        case STMT_DIRAC:
            switch (n->op) {
                case DIRAC_MV:    exec_Mv   (n->id, n->id2, n->id3); break;
                case DIRAC_CV:    exec_cv   (n->id, ast_emit_expr(n->rhs), n->id2); break;
                case DIRAC_APCB:  exec_apcb (n->id, n->id2, ast_emit_expr(n->rhs), n->id3); break;
                case DIRAC_VVT:   exec_vvt  (n->id, n->id2, n->id3); break;
                case DIRAC_MMVVT: exec_Mmvvt(n->id, n->id2, n->id3, n->id4); break;
                case DIRAC_CM:    exec_cM   (n->id, ast_emit_expr(n->rhs), n->id2); break;
                case DIRAC_CI:    exec_cI   (n->id, ast_emit_expr(n->rhs)); break;
                case DIRAC_V0:    exec_v0   (n->id); break;
                case DIRAC_CVIN:  exec_cvin (n->id, ast_emit_expr(n->rhs), n->id2); break;
                case DIRAC_SHIFT: exec_shift(n->id, ast_emit_expr(n->rhs), n->id2); break;
            }
            break;

        case STMT_VOID_CALL:
            ast_emit_expr(n->rhs);  // walks the EXPR_FUNC_CALL: par_check + CAL
            break;

        case STMT_DIRE_INTER:
            dire_inter();
            break;

        case STMT_DIRE_TOAQUI:
            dire_toaqui();
            break;

        case STMT_BLOCK:
            for (int i = 0; i < n->kids_n; i++) stmt_emit(n->kids[i]);
            break;

        case STMT_IF: {
            emit_cond_int_load(ast_emit_expr(n->cond), 0);
            add_instr("JIZ Lif%delse\n", n->id);
            acc_ok = 0;

            stmt_emit(n->then_body);

            if (n->else_body)
            {
                add_instr("JMP Lif%dend\n",  n->id);
                add_sinst(0, "@Lif%delse ",  n->id);
                stmt_emit(n->else_body);
                add_sinst(0, "@Lif%dend ",   n->id);
            }
            else
            {
                add_sinst(0, "@Lif%delse ",  n->id);
            }
            break;
        }

        case STMT_WHILE: {
            add_sinst(0, "@Lwh%d ", n->id);
            emit_cond_int_load(ast_emit_expr(n->cond), 0);
            add_instr("JIZ Lwh%dend\n", n->id);
            acc_ok = 0;

            stmt_emit(n->body);

            // For a `for`, the step lives in else_body and is emitted here, after
            // the body and before the back-edge -- this is where `continue`
            // lands so the step still runs. The Lwh<n>cont label is only emitted
            // when a continue actually targets this loop (op set by
            // exec_continue), so loops without continue stay byte-identical.
            if (n->op)        add_sinst(0, "@Lwh%dcont ", n->id);
            if (n->else_body) stmt_emit(n->else_body);

            add_instr("JMP Lwh%d\n",   n->id);
            add_sinst(0, "@Lwh%dend ", n->id);
            break;
        }

        case STMT_DO: {
            // do { body } while (cond); -- body runs first, condition tested at
            // the bottom, so the body always runs at least once. Shares the
            // Lwh<n> label namespace, so break (-> Lwh<n>end) and continue (->
            // Lwh<n>cont) ride the same stacks as while/for. continue lands at
            // Lwh<n>cont, just before the test, so it re-evaluates the condition.
            add_sinst(0, "@Lwh%d ", n->id);
            acc_ok = 0;

            stmt_emit(n->body);

            if (n->op) add_sinst(0, "@Lwh%dcont ", n->id);   // continue target
            emit_cond_int_load(ast_emit_expr(n->cond), 0);
            add_instr("JIZ Lwh%dend\n", n->id);   // condition false -> exit
            add_instr("JMP Lwh%d\n",    n->id);   // condition true  -> loop
            add_sinst(0, "@Lwh%dend ",  n->id);
            acc_ok = 0;
            break;
        }

        case STMT_SWITCH: {
            // ensure the implicit `switch_exp` var exists and matches the
            // cond's type; the dispatch block below rebuilds an expr against it.
            expr cond_e = ast_emit_expr(n->cond);
            if (find_var("switch_exp") == -1) add_var("switch_exp");
            int sw_id = find_var("switch_exp");
            v_table[sw_id].type = cond_e.type;
            v_table[sw_id].used = 0;

            emit_cond_int_load(cond_e, 1);
            add_instr("SET switch_exp\n");
            acc_ok = 0;

            // The body is a flat block of CASE_LABEL / DEFAULT_LABEL markers and
            // statements in source order. We emit it in two passes so the case
            // bodies sit contiguously and a case without `break` falls through
            // to the next -- real C semantics (the old single-pass scheme
            // re-tested switch_exp at every label, which skipped instead of
            // falling through).
            stmt_node *body = n->body;
            int def_idx = -1;  // case-index of the default label, if present

            // Pass 1 -- dispatch: compare switch_exp against each case value and
            // jump to that case's body when equal. With JIZ only, "jump if
            // equal" is `JIZ <skip>` (taken when NOT equal) + `JMP <body>`.
            for (int i = 0; body && i < body->kids_n; i++)
            {
                stmt_node *kid = body->kids[i];
                if (kid->kind == STMT_CASE_LABEL)
                {
                    expr e1 = num2exp(kid->id3, kid->id4);
                    expr e2 = id2exp(sw_id);
                    oper_cmp(e1, e2, 2);                                 // acc = (val == switch_exp)
                    add_instr("JIZ sw_disp_%d_%d\n", n->id, kid->id2);  // not equal -> next entry
                    add_instr("JMP sw_body_%d_%d\n", n->id, kid->id2);  // equal     -> this body
                    add_sinst(0, "@sw_disp_%d_%d ", n->id, kid->id2);
                    acc_ok = 0;
                }
                else if (kid->kind == STMT_DEFAULT_LABEL)
                {
                    def_idx = kid->id2;
                }
            }
            // nothing matched -> default body (if any), else the switch end.
            if (def_idx >= 0) add_instr("JMP sw_body_%d_%d\n", n->id, def_idx);
            else              add_instr("JMP switch_end_%d\n",  n->id);
            acc_ok = 0;

            // Pass 2 -- bodies, contiguous. A label node only drops its body
            // label; everything else emits in order (statements, breaks, nested
            // switches). No compare here, so control falls from one body into
            // the next exactly like C.
            for (int i = 0; body && i < body->kids_n; i++)
            {
                stmt_node *kid = body->kids[i];
                if (kid->kind == STMT_CASE_LABEL || kid->kind == STMT_DEFAULT_LABEL)
                {
                    add_sinst(0, "@sw_body_%d_%d ", n->id, kid->id2);
                    acc_ok = 0;
                }
                else
                {
                    stmt_emit(kid);
                }
            }
            add_sinst(0, "@switch_end_%d ", n->id);
            acc_ok = 0;
            break;
        }

        // CASE_LABEL / DEFAULT_LABEL are consumed directly by the enclosing
        // STMT_SWITCH two-pass walk above; they never emit on their own.
        case STMT_CASE_LABEL:
        case STMT_DEFAULT_LABEL:
            break;

        case STMT_SWITCH_BREAK:
            add_instr("JMP switch_end_%d\n", n->id);
            break;

        case STMT_CONTINUE:
            // for: jump to the step label (Lwh<n>cont) so the step runs before
            // the next condition test; while: jump straight to the top.
            if (n->op) add_instr("JMP Lwh%dcont\n", n->id);
            else       add_instr("JMP Lwh%d\n",     n->id);
            break;

        case STMT_BREAK_WHILE:
            add_instr("JMP Lwh%dend\n", n->id);
            break;

        case STMT_DECLAR_ARR_1D:
            declar_arr_1d_emit(n->id, n->id2, n->id3);
            break;

        case STMT_DECLAR_ARR_2D:
            declar_arr_2d_emit(n->id, n->id2, n->id3, n->id4);
            break;

        case STMT_DECLAR_MV:
            // base array #array directive(s), then Mv codegen.
            declar_arr_1d_emit(n->id, n->id2, -1);
            exec_Mv(n->id, n->id3, n->id4);
            break;

        case STMT_DECLAR_CV:
            // base array #array directive(s), then walk the scalar coeff
            // and let exec_cv do the c|v⟩ codegen.
            declar_arr_1d_emit(n->id, n->id2, -1);
            exec_cv(n->id, ast_emit_expr(n->rhs), n->id3);
            break;

        case STMT_FUNC_HEADER: {
            // Function prologue. Emits, in order:
            //   - JMP main\n             (only when this is the first non-main
            //                             function: control falls through at
            //                             reset, so we redirect to main)
            //   - @<funcname>            (label)
            //   - SET_P <p[k]>           (last-to-second param, with imag
            //                             half first when the param is comp)
            //   - SET   <p[0]>           (first param)
            // Mirrors the pre-AST pattern that declar_fun / declar_fst /
            // set_par used to do inline at parse time.
            if (n->op) add_sinst(-2, "JMP main\n");
            add_sinst(0, "@%s ", v_table[n->id].name);
            for (int i = n->n_params - 1; i >= 1; i--)
            {
                int pid = n->params[i];
                if (v_table[pid].type > 2)
                    add_instr("SET_P %s\n", v_table[get_img_id(pid)].name);
                add_instr("SET_P %s\n", v_table[pid].name);
            }
            if (n->n_params >= 1)
            {
                int pid = n->params[0];
                if (v_table[pid].type > 2)
                    add_instr("SET_P %s\n", v_table[get_img_id(pid)].name);
                add_instr("SET %s\n", v_table[pid].name);
            }
            // The function entry is a basic-block boundary: the body's first
            // instruction cannot fuse with the parameter SET via the peephole.
            emit_peephole_reset();
            break;
        }

        case STMT_DIRECTIVE:
            // Pre-function configuration line. Goes straight into the asm
            // header (no instruction slot, no f_lin entry).
            add_sinst(0, "%s %s\n", n->dir_str, v_table[n->id].name);
            break;

        case STMT_FUNC: {
            // Full function definition. Resets ret_ok (set by STMT_RETURN
            // walker when a `return` statement fires) and stashes the
            // current function id into the global fun_parse so declar_ret
            // - reached at walker time from STMT_RETURN - can validate the
            // return type against the enclosing function. Then walks the
            // body, validates that a non-void function actually emitted a
            // return somewhere, and finally emits the trailing exit
            // instruction (@fim JMP fim for main, RET for void).
            // Non-void/non-main functions rely on the explicit `return
            // exp;` inside the body to emit their own RET.
            int saved_fun_parse = fun_parse;
            fun_parse = n->id;
            // emit_fname carries this function's name through the body walk so
            // rem_fname() can strip the "<fn>_" prefix from diagnostics. It is
            // DISPLAY-ONLY: exec_id() still keys off the (empty-during-walk)
            // global fname, so the temp-variable names -- and the assembly -- are
            // unchanged. Globals (no prefix) are unaffected by rem_fname.
            char saved_emit_fname[512];
            strcpy(saved_emit_fname, emit_fname);
            strcpy(emit_fname, v_table[n->id].name);
            ret_ok    = 0;
            stmt_emit(n->body);
            strcpy(emit_fname, saved_emit_fname);
            fun_parse = saved_fun_parse;

            if ((v_table[n->id].type != 6) && (ret_ok == 0))
            {
                fprintf(stderr, MSG_ERR_FUNC_NO_RETURN, v_table[n->id].name);
                exit(EXIT_FAILURE);
            }

            if (strcmp(v_table[n->id].name, "main") == 0)
            {
                add_sinst(-3, "@fim JMP fim\n");
                v_table[n->id].used = 1;
            }
            else if (v_table[n->id].type == 6)
            {
                add_instr("RET\n");
            }
            break;
        }
    }

    n->emitted = 1;
    emit_line  = saved_emit_line;
    line_num   = saved_line_num;
}

void stmt_free(stmt_node *n)
{
    if (!n) return;
    expr_free(n->rhs);
    expr_free(n->idx);
    expr_free(n->idx2);
    expr_free(n->cond);
    stmt_free(n->then_body);
    stmt_free(n->else_body);
    stmt_free(n->body);
    for (int i = 0; i < n->kids_n; i++) stmt_free(n->kids[i]);
    free(n->kids);
    free(n->params);
    free(n);
}

// Appends a stmt_node to the currently-open STMT_BLOCK. The stack always
// has at least the global stmt_list opened by parse_init, so the top is
// never empty - every reduce that builds a statement just parks the node
// here and lets the walker (driven by `fim : prog` -> prog_emit, or by
// an enclosing if_fim / while_stmt / end_switch closer) emit it later.
void stmt_append(stmt_node *n)
{
    stmt_block_push(stmt_list_stack[stmt_list_stack_n - 1], n);
}
