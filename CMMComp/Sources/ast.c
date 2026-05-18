// ----------------------------------------------------------------------------
// AST nodes for statements (see ast.h for the design notes) ------------------
// ----------------------------------------------------------------------------

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "..\Headers\ast.h"
#include "..\Headers\emit.h"
#include "..\Headers\global.h"
#include "..\Headers\variaveis.h"   // v_name[] for the dump
#include "..\Headers\messages.h"

// needed by the expression-tree walker (ast_emit_expr) below
#include "..\Headers\oper.h"
#include "..\Headers\stdlib.h"
#include "..\Headers\array_index.h"
#include "..\Headers\data_use.h"
#include "..\Headers\funcoes.h"

// needed by the stmt_node walker
#include "..\Headers\data_assign.h"
#include "..\Headers\itr.h"
#include "..\Headers\t2t.h"      // get_cmp_cst for complex cond loads

// ----------------------------------------------------------------------------
// expressions ----------------------------------------------------------------
// ----------------------------------------------------------------------------

expr expr_make(int type, int id)
{
    expr e = {type, id, NULL};
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

expr_node *expr_binop(int op, int type, expr_node *left, expr_node *right)
{
    expr_node *n = enode_new(EXPR_BINOP);
    n->op    = op;
    n->type  = type;
    n->left  = left;
    n->right = right;
    return n;
}

expr_node *expr_unop(int op, int type, expr_node *operand)
{
    expr_node *n = enode_new(EXPR_UNOP);
    n->op   = op;
    n->type = type;
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

expr_node *expr_stdlib(int op, int type, int port, expr_node *a, expr_node *b)
{
    expr_node *n = enode_new(EXPR_STDLIB_CALL);
    n->op    = op;
    n->type  = type;
    n->id    = port;      // INUM port for IN/FIN/OUT, 0 for compute-only calls
    n->left  = a;
    n->right = b;
    return n;
}

expr_node *expr_func_call(int type, int id, expr_node **args, int n_args)
{
    expr_node *n = enode_new(EXPR_FUNC_CALL);
    n->type   = type;
    n->id     = id;       // function's v_name index
    n->args   = args;     // takes ownership
    n->n_args = n_args;
    return n;
}

expr_node *expr_inner(int type, expr_node *a, expr_node *b)
{
    expr_node *n = enode_new(EXPR_INNER);
    n->type  = type;
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
            fprintf(stderr, " id=%d (%s)", n->id, n->id ? v_name[n->id] : "<acc>");
            break;
        case EXPR_BINOP:
        case EXPR_UNOP:
            fprintf(stderr, " op=%d", n->op);
            break;
        case EXPR_ARRAY_INDEX:
            fprintf(stderr, " id=%d (%s) reversed=%d", n->id, v_name[n->id], n->op);
            break;
        case EXPR_PPLUS:
            fprintf(stderr, " id=%d (%s)", n->id, v_name[n->id]);
            break;
        case EXPR_STDLIB_CALL:
            fprintf(stderr, " op=%d port=%d", n->op, n->id);
            break;
        case EXPR_FUNC_CALL:
            fprintf(stderr, " id=%d (%s) n_args=%d", n->id, v_name[n->id], n->n_args);
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
// "in the accumulator", id > 0 means "in v_name[id]". The public wrapper
// stamps the returned POD with the original node pointer so callers can
// keep chasing the tree through the result.

static expr ast_emit_expr_impl(expr_node *n);

// Idempotent: a second call over the same node returns the first result
// instead of re-emitting. This guards against double-emission if a tree ever
// gets walked twice (shouldn't happen in the grammar today, but cheap to
// keep as a safety net).
expr ast_emit_expr(expr_node *n)
{
    if (n && n->emitted) return n->cached;
    expr e = ast_emit_expr_impl(n);
    e.node = n;
    if (n) {
        n->emitted = 1;
        n->cached  = e;
    }
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
            expr a = ast_emit_expr(n->left );
            expr b = ast_emit_expr(n->right);
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
                case OP_STD_REAL: { expr a = ast_emit_expr(n->left); return exec_real(a); }
                case OP_STD_IMAG: { expr a = ast_emit_expr(n->left); return exec_imag(a); }
                case OP_STD_COMP: { expr a = ast_emit_expr(n->left); expr b = ast_emit_expr(n->right); return exec_comp(a, b); }
                case OP_STD_FASE: { expr a = ast_emit_expr(n->left); return exec_fase(a); }
                case OP_STD_MOD2: { expr a = ast_emit_expr(n->left); return exec_mod2(a); }
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

            add_instr("CAL %s\n", v_name[n->id]);
            v_used[n->id] = 1;
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
// it to the current body's STMT_BLOCK via stmt_emit_inline. The walker fires
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

stmt_node *stmt_raw(const char *text)
{
    stmt_node *n = snode_new(STMT_RAW);
    size_t len = strlen(text);
    n->text = malloc(len + 1);
    if (!n->text) {fprintf(stderr, MSG_ERR_OUT_OF_MEMORY); exit(EXIT_FAILURE);}
    memcpy(n->text, text, len + 1);
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

    if ((e.type == 1) && (e.id != 0)) add_instr("LOD %s\n", v_name[e.id]);
    // int acc: nothing

    if ((e.type == 2) && (e.id != 0)) {
        fprintf(stdout, msg_float, line_num+1);
        add_instr("F2I_M %s\n", v_name[e.id]);
    }
    if ((e.type == 2) && (e.id == 0)) {
        fprintf(stdout, msg_float, line_num+1);
        add_instr("F2I\n");
    }
    if (e.type == 5) {
        fprintf(stdout, msg_comp, line_num+1);
        expr etr, eti;
        get_cmp_cst(e, &etr, &eti);
        add_instr("F2I_M %s\n", v_name[etr.id]);
    }
    if ((e.type == 3) && (e.id != 0)) {
        fprintf(stdout, msg_comp, line_num+1);
        add_instr("F2I_M %s\n", v_name[e.id]);
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

            add_instr("JMP Lwh%d\n",   n->id);
            add_sinst(0, "@Lwh%dend ", n->id);
            break;
        }

        case STMT_SWITCH: {
            // ensure the implicit `switch_exp` var exists and matches the
            // cond's type; case_test rebuilds an expr against it later.
            expr cond_e = ast_emit_expr(n->cond);
            if (find_var("switch_exp") == -1) add_var("switch_exp");
            int sw_id = find_var("switch_exp");
            v_type[sw_id] = cond_e.type;
            v_used[sw_id] = 0;

            emit_cond_int_load(cond_e, 1);
            add_instr("SET switch_exp\n");
            acc_ok = 0;

            stmt_emit(n->body);

            add_sinst(0, "@sw_case_%d_%d ", n->id, n->id2 + 1);
            add_sinst(0, "@switch_end_%d ", n->id);
            break;
        }

        case STMT_CASE_LABEL: {
            // n->id  = swit_id, n->id2 = case_idx, n->id3 = val_id, n->id4 = val_type
            add_sinst(0, "@sw_case_%d_%d ", n->id, n->id2);
            expr e1 = num2exp(n->id3, n->id4);
            expr e2 = id2exp (find_var("switch_exp"));
            oper_cmp(e1, e2, 2);
            add_instr("JIZ sw_case_%d_%d\n", n->id, n->id2 + 1);
            acc_ok = 0;
            break;
        }

        case STMT_DEFAULT_LABEL:
            add_sinst(0, "@sw_case_%d_%d ", n->id, n->id2);
            break;

        case STMT_SWITCH_BREAK:
            add_instr("JMP switch_end_%d\n", n->id);
            break;

        case STMT_BREAK_WHILE:
            add_instr("JMP Lwh%dend\n", n->id);
            break;

        case STMT_RAW:
            // already-counted assembly text captured at parse time
            emit_raw(n->text);
            break;
    }

    n->emitted = 1;
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
    free(n->text);
    free(n);
}

void stmt_emit_inline(stmt_node *n)
{
    if (stmt_list_stack_n > 0)
    {
        // A body is open: defer codegen by parking the node on the top
        // STMT_BLOCK. The body's closer (func_ret / if_fim / while_stmt /
        // end_switch ...) walks the block and emits each kid in source order.
        stmt_block_push(stmt_list_stack[stmt_list_stack_n - 1], n);
    }
    else
    {
        // No body active (a stmt reduces outside any function context): walk
        // immediately so the emit reaches f_asm at parse time. Not exercised
        // by the current grammar but cheap to keep as a safety net.
        stmt_emit(n);
        stmt_free(n);
    }
}
