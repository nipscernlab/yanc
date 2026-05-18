// ----------------------------------------------------------------------------
// AST nodes for statements (see ast.h for the design notes) ------------------
// ----------------------------------------------------------------------------

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "..\Headers\ast.h"
#include "..\Headers\emit.h"
#include "..\Headers\global.h"
#include "..\Headers\labels.h"
#include "..\Headers\variaveis.h"   // v_name[] for the dump
#include "..\Headers\messages.h"

// needed by the expression-tree walker (ast_emit_expr) below
#include "..\Headers\oper.h"
#include "..\Headers\stdlib.h"
#include "..\Headers\array_index.h"
#include "..\Headers\data_use.h"
#include "..\Headers\funcoes.h"

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
// No callers yet. The bison stack still carries the flat `expr` POD; this
// only puts the building blocks in place so a follow-up commit can start
// returning expr_node * from terminal / exp reductions.

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
// expression tree walker (codegen via tree) ----------------------------------
// ----------------------------------------------------------------------------
// Recreates the emit that the grammar actions currently do inline. Calls the
// same oper_*/exec_*/arr_*/pplus_*/num2exp/id2exp/par_exp/fcall helpers, so
// running the walker over a built tree produces byte-identical assembly to
// the inline path. NOT invoked yet - turning it on is a future step.

expr ast_emit_expr(expr_node *n)
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
            args_frame_push();
            for (int i = 0; i < n->n_args; i++) {
                expr a = ast_emit_expr(n->args[i]);
                if (i == 0) par_exp    (a);
                else        par_listexp(a);
            }
            return fcall(n->id);
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
// helpers --------------------------------------------------------------------
// ----------------------------------------------------------------------------

static ast_node *node_new(ast_kind k)
{
    ast_node *n = calloc(1, sizeof(*n));
    if (!n) {fprintf(stderr, MSG_ERR_OUT_OF_MEMORY); exit(EXIT_FAILURE);}
    n->kind = k;
    n->line = line_num + 1;
    return n;
}

// ----------------------------------------------------------------------------
// constructors ---------------------------------------------------------------
// ----------------------------------------------------------------------------

ast_node *ast_raw(const char *text)
{
    ast_node *n = node_new(AST_RAW);
    size_t len  = strlen(text);
    n->text = malloc(len + 1);
    if (!n->text) {fprintf(stderr, MSG_ERR_OUT_OF_MEMORY); exit(EXIT_FAILURE);}
    memcpy(n->text, text, len + 1);
    return n;
}

ast_node *ast_block(void)
{
    return node_new(AST_BLOCK);
}

void ast_block_push(ast_node *blk, ast_node *kid)
{
    if (blk->kids_n + 1 > blk->kids_cap)
    {
        int new_cap = blk->kids_cap ? blk->kids_cap * 2 : 8;
        ast_node **t = realloc(blk->kids, (size_t)new_cap * sizeof(*t));
        if (!t) {fprintf(stderr, MSG_ERR_OUT_OF_MEMORY); exit(EXIT_FAILURE);}
        blk->kids     = t;
        blk->kids_cap = new_cap;
    }
    blk->kids[blk->kids_n++] = kid;
}

ast_node *ast_if(int label, ast_node *body, ast_node *els)
{
    ast_node *n = node_new(AST_IF);
    n->label = label;
    n->body  = body;
    n->els   = els;
    return n;
}

ast_node *ast_while(int label, ast_node *body)
{
    ast_node *n = node_new(AST_WHILE);
    n->label = label;
    n->body  = body;
    return n;
}

ast_node *ast_break(void)
{
    return node_new(AST_BREAK);
}

ast_node *ast_switch(int swit_id, int case_max, ast_node *body)
{
    ast_node *n = node_new(AST_SWITCH);
    n->label    = swit_id;
    n->case_max = case_max;
    n->body     = body;
    return n;
}

// ----------------------------------------------------------------------------
// destructor -----------------------------------------------------------------
// ----------------------------------------------------------------------------

void ast_free(ast_node *n)
{
    if (!n) return;

    free(n->text);
    ast_free(n->cond);
    ast_free(n->body);
    ast_free(n->els );

    for (int i = 0; i < n->kids_n; i++) ast_free(n->kids[i]);
    free(n->kids);

    free(n);
}

// ----------------------------------------------------------------------------
// emit walker ----------------------------------------------------------------
// ----------------------------------------------------------------------------
//
// Each node kind is migrated one at a time. The migrated cases turn the node
// back into assembly. The default case aborts so we notice if an un-migrated
// kind ever reaches here.

void ast_emit(ast_node *n)
{
    if (!n) return;

    switch (n->kind)
    {
        case AST_RAW:
            // already-counted text: replay without re-doing f_lin / num_ins
            emit_raw(n->text);
            break;

        case AST_BLOCK:
            for (int i = 0; i < n->kids_n; i++) ast_emit(n->kids[i]);
            break;

        case AST_IF:
            ast_emit(n->body); // replay then
            if (n->els)
            {
                add_instr("JMP Lif%dend\n",  n->label);
                add_sinst(0, "@Lif%delse ",  n->label);
                ast_emit(n->els);
                add_sinst(0, "@Lif%dend ",   n->label);
            }
            else
            {
                add_sinst(0, "@Lif%delse ",  n->label);
            }
            break;

        case AST_WHILE:
            ast_emit(n->body); // replay body
            add_instr("JMP Lwh%d\n",     n->label);
            add_sinst(0, "@Lwh%dend ",   n->label);
            break;

        case AST_BREAK:
            add_instr("JMP Lwh%dend\n", get_while());
            break;

        case AST_SWITCH:
            ast_emit(n->body);
            add_sinst(0, "@sw_case_%d_%d ", n->label, n->case_max + 1);
            add_sinst(0, "@switch_end_%d ", n->label);
            break;
    }
}
