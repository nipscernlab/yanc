// ----------------------------------------------------------------------------
// CPPComp — AST constructors ---------------------------------------------------
// ----------------------------------------------------------------------------

#include <stdlib.h>
#include <string.h>

#include "../Headers/ast.h"
#include "../Headers/messages.h"

static void *xcalloc(size_t n) { void *p = calloc(1, n); if (!p) msg_internal("oom"); return p; }

static expr *new_expr(expr_kind k, int line)
{
    expr *e = (expr*)xcalloc(sizeof(expr));
    e->kind = k; e->line = line;
    return e;
}

expr *ast_int_lit  (long v, int line)            { expr *e = new_expr(E_INT_LIT,    line); e->ival = v; return e; }
expr *ast_float_lit(double v, int line)          { expr *e = new_expr(E_FLOAT_LIT,  line); e->fval = v; return e; }
expr *ast_char_lit (long v, int line)            { expr *e = new_expr(E_CHAR_LIT,   line); e->ival = v; return e; }
expr *ast_string_lit(char *bytes, int len, int line) { expr *e = new_expr(E_STRING_LIT, line); e->sval = bytes; e->slen = len; return e; }
expr *ast_ident    (char *n, int line)           { expr *e = new_expr(E_IDENT,      line); e->sval = n; return e; }
expr *ast_assign   (expr *l, expr *r, int line)  { expr *e = new_expr(E_ASSIGN,     line); e->a = l; e->b = r; e->op = OP_NONE; return e; }
expr *ast_binop    (op_kind o, expr *l, expr *r, int line) { expr *e = new_expr(E_BINOP, line); e->op = o; e->a = l; e->b = r; return e; }
/* compound `l OP= r`: keep the desugared `l = l OP r` shape (so scalars and the
   default struct path are unchanged) but remember OP so codegen can dispatch to
   a user-defined operatorOP= when the lhs is a class that overloads it. */
expr *ast_assign_op(op_kind o, expr *l, expr *r, int line) { expr *e = new_expr(E_ASSIGN, line); e->a = l; e->b = ast_binop(o, l, r, line); e->op = o; return e; }
expr *ast_unop     (op_kind o, expr *o2, int line) { expr *e = new_expr(E_UNOP, line); e->op = o; e->a = o2; return e; }
expr *ast_index    (expr *arr, expr *idx, int line) { expr *e = new_expr(E_INDEX, line); e->a = arr; e->b = idx; return e; }
expr *ast_call     (expr *callee, expr **args, int nargs, int line)
{
    expr *e = new_expr(E_CALL, line); e->a = callee; e->args = args; e->n_args = nargs; return e;
}
expr *ast_member  (expr *base, char *name, int line) { expr *e = new_expr(E_MEMBER,  line); e->a = base; e->member = name; return e; }
expr *ast_pmember (expr *base, char *name, int line) { expr *e = new_expr(E_PMEMBER, line); e->a = base; e->member = name; return e; }
expr *ast_addr    (expr *x, int line) { expr *e = new_expr(E_ADDR,  line); e->a = x; return e; }
expr *ast_deref   (expr *p, int line) { expr *e = new_expr(E_DEREF, line); e->a = p; return e; }
expr *ast_xfix    (expr_kind k, expr *o, int line) { expr *e = new_expr(k, line); e->a = o; return e; }
expr *ast_cast    (type *t, expr *e2, int line) { expr *e = new_expr(E_CAST, line); e->target_t = t; e->a = e2; return e; }
expr *ast_sizeof_t(type *t, int line) { expr *e = new_expr(E_SIZEOF_T, line); e->target_t = t; return e; }
expr *ast_sizeof_e(expr *e2, int line) { expr *e = new_expr(E_SIZEOF_E, line); e->a = e2; return e; }
expr *ast_ternary (expr *c, expr *a, expr *b, int line) { expr *e = new_expr(E_TERNARY, line); e->a = c; e->b = a; e->c = b; return e; }
expr *ast_comma   (expr *a, expr *b, int line) { expr *e = new_expr(E_COMMA, line); e->a = a; e->b = b; return e; }
expr *ast_compound(type *t, initz *z, int line) { expr *e = new_expr(E_COMPOUND, line); e->target_t = t; e->cinit = z; return e; }
expr *ast_generic (expr *ctrl, type **ts, expr **es, int n, int line)
{
    expr *e = new_expr(E_GENERIC, line);
    e->a = ctrl; e->gtypes = ts; e->args = es; e->n_args = n; return e;
}
// new T / new T(args) / new T[count]: target_t=elem type, args=ctor args,
// a=array-count expr (NULL for a scalar new).
expr *ast_new(type *t, expr **args, int nargs, expr *count, int line)
{
    expr *e = new_expr(E_NEW, line);
    e->target_t = t; e->args = args; e->n_args = nargs; e->a = count; return e;
}
// T(args) temporary: target_t = class type, args = ctor arguments.
expr *ast_tempobj(type *t, expr **args, int nargs, int line)
{
    expr *e = new_expr(E_TEMPOBJ, line);
    e->target_t = t; e->args = args; e->n_args = nargs; return e;
}
// delete p / delete[] p: a=operand, ival=1 for the array form.
expr *ast_delete(expr *p, int is_array, int line)
{
    expr *e = new_expr(E_DELETE, line);
    e->a = p; e->ival = is_array; return e;
}

stmt *ast_stmt(stmt_kind k, int line)
{
    stmt *s = (stmt*)xcalloc(sizeof(stmt));
    s->kind = k; s->line = line; return s;
}

decl *ast_decl(type *t, char *n, expr *init, int line)
{
    decl *d = (decl*)xcalloc(sizeof(decl));
    d->dtype = t; d->name = n; d->init = init; d->line = line; return d;
}

initz *ast_initz_expr(expr *e, int line)
{
    initz *z = (initz*)xcalloc(sizeof(initz));
    z->is_list = 0; z->e = e; z->line = line; return z;
}

initz *ast_initz_list(int line)
{
    initz *z = (initz*)xcalloc(sizeof(initz));
    z->is_list = 1; z->line = line; return z;
}

void initz_add(initz *z, initz *item, init_desig d)
{
    z->items  = (initz**)realloc(z->items,  sizeof(initz*)    * (z->n + 1));
    z->desigs = (init_desig*)realloc(z->desigs, sizeof(init_desig) * (z->n + 1));
    z->items[z->n] = item; z->desigs[z->n] = d; z->n++;
}

func *ast_func(type *ret, char *n, decl *params, int np, stmt *body, int line)
{
    func *f = (func*)xcalloc(sizeof(func));
    f->ret = ret; f->name = n; f->params = params; f->n_params = np; f->body = body; f->line = line; return f;
}

unit *ast_unit(void)
{
    unit *u = (unit*)xcalloc(sizeof(unit));
    u->nubits = -1; u->nbmant = -1; u->nbexpo = -1; u->nugain = -1;
    u->ndstac = -1; u->sdepth = -1; u->nuioin = -1; u->nuioou = -1; u->fftsiz = -1; u->itradd = -1;
    return u;
}

const char *op_name(op_kind o)
{
    switch (o) {
        case OP_ADD: return "+";   case OP_SUB: return "-";
        case OP_MUL: return "*";   case OP_DIV: return "/";   case OP_MOD: return "%";
        case OP_BAND: return "&";  case OP_BOR: return "|";   case OP_BXOR: return "^";
        case OP_SHL: return "<<";  case OP_SHR: return ">>";
        case OP_EQ: return "==";   case OP_NE: return "!=";
        case OP_LT: return "<";    case OP_GT: return ">";    case OP_LE: return "<="; case OP_GE: return ">=";
        case OP_LAND: return "&&"; case OP_LOR: return "||";
        case OP_NEG: return "-";   case OP_BNOT: return "~";  case OP_LNOT: return "!"; case OP_POS: return "+";
        case OP_NONE: return "?";
    }
    return "?";
}
