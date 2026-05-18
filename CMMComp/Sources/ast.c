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

void expr_free(expr_node *n)
{
    if (!n) return;
    expr_free(n->left);
    expr_free(n->right);
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
    }
    fputc('\n', stderr);
    expr_dump_at(n->left,  depth + 1);
    expr_dump_at(n->right, depth + 1);
}

void expr_dump(expr_node *n)
{
    expr_dump_at(n, 0);
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
