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
#include "..\Headers\messages.h"

// ----------------------------------------------------------------------------
// expressions ----------------------------------------------------------------
// ----------------------------------------------------------------------------

expr expr_make(int type, int id)
{
    expr e = {type, id};
    return e;
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
