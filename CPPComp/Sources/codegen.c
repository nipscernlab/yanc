// ----------------------------------------------------------------------------
// CNIPS — AST -> YANC .asm code generator ------------------------------------
// ----------------------------------------------------------------------------
// Conventions:
//   - Accumulator (racc) carries every expression result.
//   - Data stack is the scratch for compound expressions; PSH before recursing
//     into the right side of a binop, S_<op> to combine.
//   - All locals get the asm name "<func>_<var>"; globals keep their raw name.
//   - Caller of f(a,b,c) pushes args LEFT-to-RIGHT; callee pops them in REVERSE.
//   - Conditional jumps run through LIN;LIN to normalise the value to {0,1}
//     unless the expression's top operator is already comparison/logic.
//   - Lvalue accesses (arr[i], s.f, *p, p->f) compute &lv into the accumulator
//     and use LDA/STA to dereference. Simple scalar idents use LOD/SET fast.
// ----------------------------------------------------------------------------

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>

#include "..\Headers\codegen.h"
#include "..\Headers\config.h"
#include "..\Headers\symtab.h"
#include "..\Headers\messages.h"

// ---- output ----------------------------------------------------------------

static FILE *out_f;
static int   ins_count = 0;
static int   label_n   = 0;
static int   g_nubits  = 32;     // effective word width (for unsigned compares)
static int   g_uses_udiv = 0;    // an unsigned / or % was emitted -> emit _udivmod
static int   g_any_recursive = 0; // some function needs stack frames
static int   g_uses_heap = 0;    // malloc/free/new/delete used -> emit heap runtime
#ifndef CFG_HEAPSZ
#define CFG_HEAPSZ 2048
#endif
static int   cur_fn_recursive = 0; // emitting a recursive function right now
static int   cur_frame_cursor = 0; // next free frame offset while emitting it
#define CSTK_SIZE 1024            // software call-stack depth (words)
static char *cur_func_name = NULL;
static type *cur_func_ret  = NULL;
static type *cur_method_class = NULL;   // class type when emitting a method body
static unit *cg_unit = NULL;            // the translation unit (for overload resolution)
static type *infer_type(expr *e);       // (also forward-declared below; needed early here)

// inside a method, an unqualified name that isn't a local/param but IS a data
// member of the enclosing class resolves to `this->member`. Returns the field
// (and tells the caller to address it relative to the `this` param).
static strct_field *cur_class_member(const char *name)
{
    if (!cur_method_class) return NULL;
    if (st_find_local(name)) return NULL;     // a local/param shadows a member
    return t_struct_find(cur_method_class, name);
}

// mangle Class::name -> "Class__name" (must match the parser's scheme)
static char *cg_mangle_method(const char *cls, const char *name)
{
    char *m = malloc(strlen(cls) + strlen(name) + 3);
    sprintf(m, "%s__%s", cls, name);
    return m;
}

// virtual-method bookkeeping: the polymorphic classes whose vtables we emit
static type *poly_cls[64]; static int n_poly = 0;

// slot of virtual method `name` in class `ct` (vtbl already includes inherited
// slots), or -1 if `name` is not virtual.
static int vtbl_slot(type *ct, const char *name)
{
    if (!ct) return -1;
    for (int i = 0; i < ct->n_vtbl; i++) if (!strcmp(ct->vtbl[i], name)) return i;
    return -1;
}

// find method `m` in `cls` or an ancestor (single inheritance); returns its
// mangled asm name (malloc'd, = the SK_FUNC's name) or NULL.
static char *resolve_method(type *cls, const char *m)
{
    for (type *c = cls; c; c = c->base_class) {
        if (!c->tag) continue;
        char *mm = cg_mangle_method(c->tag, m);
        sym *s = st_find(mm);
        if (s && s->kind == SK_FUNC) return mm;
        free(mm);
    }
    return NULL;
}

// overloadable binary operator -> method name (matches the parser's op_name)
static const char *op_method_name(op_kind op)
{
    switch (op) {
        case OP_ADD: return "op_add"; case OP_SUB: return "op_sub";
        case OP_MUL: return "op_mul"; case OP_DIV: return "op_div";
        case OP_EQ:  return "op_eq";  case OP_NE:  return "op_ne";
        case OP_LT:  return "op_lt";  case OP_GT:  return "op_gt";
        case OP_LE:  return "op_le";  case OP_GE:  return "op_ge";
        default: return NULL;
    }
}

// Assign each function its emitted label. A name shared by more than one
// definition is overloaded -> append `_O` + per-param type codes (kept to the
// [A-Za-z0-9_] charset the assembler accepts); unique names stay plain.
static void assign_overload_labels(unit *u)
{
    for (int i = 0; i < u->n_funcs; i++) {
        func *f = u->funcs[i];
        int dup = 0;
        for (int j = 0; j < u->n_funcs; j++)
            if (j != i && !strcmp(u->funcs[j]->name, f->name)) { dup = 1; break; }
        if (!dup) { f->asm_label = f->name; continue; }
        char *lbl = malloc(strlen(f->name) + f->n_params + 3);
        int k = sprintf(lbl, "%s_O", f->name);
        decl *p = f->params;
        for (int a = 0; a < f->n_params && p; a++, p = p->next) lbl[k++] = type_code(p->dtype);
        lbl[k] = 0;
        f->asm_label = lbl;
    }
}

// pick the best-matching overload of `name` for the argument types (exact code
// match scores higher than an int<->float conversion); NULL if no such function.
static func *resolve_overload(const char *name, expr **args, int nargs)
{
    if (!cg_unit) return NULL;
    func *best = NULL; int best_score = -1;
    for (int i = 0; i < cg_unit->n_funcs; i++) {
        func *f = cg_unit->funcs[i];
        if (f->method_of || strcmp(f->name, name)) continue;
        int required = 0;                       // params without a default (assumed leading)
        for (decl *p = f->params; p; p = p->next) if (!p->init) required++;
        if (nargs < required || nargs > f->n_params) continue;   // default args fill the rest
        int score = 0, ok = 1;
        decl *p = f->params;
        for (int a = 0; a < nargs && p; a++, p = p->next) {
            char pc = type_code(p->dtype), ac = type_code(infer_type(args[a]));
            if (pc == ac) score += 2;
            else if ((pc == 'i' && ac == 'f') || (pc == 'f' && ac == 'i')) score += 1;
            else { ok = 0; break; }
        }
        if (ok && score > best_score) { best_score = score; best = f; }
    }
    return best;
}

static void emit(const char *fmt, ...)
{
    if (fmt[0] != '#') ins_count++;
    va_list ap; va_start(ap, fmt);
    vfprintf(out_f, fmt, ap);
    va_end(ap);
    fputc('\n', out_f);
}

static char *fresh_label(const char *prefix)
{
    char buf[64];
    snprintf(buf, sizeof(buf), "L%s%d", prefix, ++label_n);
    char *s = malloc(strlen(buf)+1); strcpy(s, buf); return s;
}

static char *mangle_local(const char *name)
{
    if (!cur_func_name) return strdup(name);
    size_t n = strlen(cur_func_name) + strlen(name) + 2;
    char *r = malloc(n);
    snprintf(r, n, "%s_%s", cur_func_name, name);
    return r;
}

// ---- variable-log (for asmcomp's cmm_log.txt) ------------------------------

typedef struct { char *func; char *var; int type_code; int size; } varlog_e;
#define VARLOG_MAX 4096
static varlog_e varlog[VARLOG_MAX];
static int      varlog_n = 0;

static void log_var(const char *func, const char *var, int type_code, int size)
{
    if (varlog_n >= VARLOG_MAX) msg_internal("too many variables");
    varlog[varlog_n].func = strdup(func);
    varlog[varlog_n].var  = strdup(var);
    varlog[varlog_n].type_code = type_code;
    varlog[varlog_n].size = size;
    varlog_n++;
}

static int type_code_for(const type *t)
{
    if (!t) return 1;
    if (t->kind == TY_FLOAT) return 2;
    return 1;        // int, ptr, char all type-code 1 for asmcomp's simulator
}

// type code of the innermost scalar element (peels array nesting)
static int innermost_code(const type *t)
{
    while (t && t->kind == TY_ARRAY) t = t->base;
    return type_code_for(t);
}

// ---- loop-context stack (break/continue) -----------------------------------

typedef struct { char *cont_l; char *break_l; } loop_ctx;
#define MAX_LOOPS 32
static loop_ctx loop_stk[MAX_LOOPS];
static int      loop_top = 0;

// unified break-target stack: BOTH loops and switches push here in nesting
// order, so `break` targets the innermost enclosing loop-or-switch (a `break`
// inside a switch nested in a loop must leave the switch, not the loop).
static char *brk_stk[MAX_LOOPS * 2];
static int   brk_top = 0;
static void  brk_push(char *b) { if (brk_top < MAX_LOOPS*2) brk_stk[brk_top++] = b; }
static void  brk_pop (void)    { if (brk_top > 0) brk_top--; }

static void loop_push(char *c, char *b)
{
    if (loop_top >= MAX_LOOPS) msg_internal("loop nesting too deep");
    loop_stk[loop_top].cont_l = c; loop_stk[loop_top].break_l = b; loop_top++;
    brk_push(b);
}
static void loop_pop(void) { loop_top--; brk_pop(); }

// ---- string-literal pool ---------------------------------------------------
// Each "..." becomes a global char array _str<N> (1 word per char + NUL),
// declared up front and filled at main entry. The expression value is the
// array's base address (C array-to-pointer decay).
typedef struct { char *label; char *bytes; int len; } strlit;
#define STRLIT_MAX 256
static strlit strtab[STRLIT_MAX];
static int    strtab_n = 0;

// ---- function-pointer dispatch table ---------------------------------------
// Every address-taken function gets an integer ID = its index here. A function
// pointer holds that ID; an indirect call sets _fp_id and CALs _dispatch, which
// CALs the matching function. (CAL takes an immediate target, hence the table.)
#define FPTAB_MAX 256
static char *fptab[FPTAB_MAX];
static int   fptab_n = 0;

static int fp_id_of(const char *name)
{
    for (int i = 0; i < fptab_n; i++) if (strcmp(fptab[i], name) == 0) return i;
    return -1;
}
static void fp_add(const char *name)
{
    if (fp_id_of(name) >= 0) return;
    if (fptab_n >= FPTAB_MAX) msg_internal("too many address-taken functions");
    fptab[fptab_n++] = strdup(name);
}

// emit the inlined function-ID dispatch (assumes _fp_id is set and the args,
// including any `this`, are already on the data stack)
static void emit_dispatch_chain(void)
{
    char *done = fresh_label("ic_done");
    for (int i = 0; i < fptab_n; i++) {
        char *skip = fresh_label("ic_s");
        emit("LOD _fp_id"); emit("EQU %d", i); emit("JIZ %s", skip);
        emit("CAL %s", fptab[i]); emit("JMP %s", done);
        emit("@%s NOP", skip); free(skip);
    }
    emit("@%s NOP", done); free(done);
}

// ---- forward decls ---------------------------------------------------------

static void gen_expr (expr *e);
static void emit_initz(const char *base, int off, type *t, initz *z);
static void emit_fn_return(void);
static void gen_addr (expr *e);
static void gen_store(expr *lv, expr *val);
static void gen_stmt (stmt *s);
static void gen_bool (expr *e, const char *jz_target);
static type *infer_type(expr *e);
static func *resolve_overload(const char *name, expr **args, int nargs);
static void  emit_vtable_inits(void);
static void  emit_dispatch_chain(void);

// ---- string-literal pre-scan (assigns each "..." a global label) -----------

static void scan_strings_stmt(stmt *s);
static void scan_strings_initz(initz *z);

static void scan_strings_expr(expr *e)
{
    if (!e) return;
    if (e->kind == E_STRING_LIT && !e->member) {
        if (strtab_n >= STRLIT_MAX) msg_internal("too many string literals");
        char lbl[32]; snprintf(lbl, sizeof(lbl), "_str%d", strtab_n);
        e->member = strdup(lbl);
        strtab[strtab_n].label = e->member;
        strtab[strtab_n].bytes = e->sval;
        strtab[strtab_n].len   = e->slen;
        strtab_n++;
    }
    if (e->kind == E_COMPOUND) { scan_strings_initz(e->cinit); return; }
    scan_strings_expr(e->a);
    scan_strings_expr(e->b);
    scan_strings_expr(e->c);
    for (int i = 0; i < e->n_args; i++) scan_strings_expr(e->args[i]);
}

static void scan_strings_initz(initz *z)
{
    if (!z) return;
    if (!z->is_list) { scan_strings_expr(z->e); return; }
    for (int i = 0; i < z->n; i++) scan_strings_initz(z->items[i]);
}

static void scan_strings_stmt(stmt *s)
{
    if (!s) return;
    scan_strings_expr(s->e1); scan_strings_expr(s->e2); scan_strings_expr(s->e3);
    scan_strings_stmt(s->body); scan_strings_stmt(s->body2); scan_strings_stmt(s->init_stmt);
    for (int i = 0; i < s->n_items; i++) scan_strings_stmt(s->items[i]);
    for (decl *d = s->decls; d; d = d->next) {
        scan_strings_expr(d->init);
        scan_strings_initz(d->binit);
    }
}

// ---- address-taken-function pre-scan ---------------------------------------
// Any function name used as a value (not as the immediate callee of a direct
// call) is address-taken and needs a dispatch-table slot.

static void scan_fp_stmt(stmt *s);
static void scan_fp_initz(initz *z);

static void scan_fp_expr(expr *e)
{
    if (!e) return;
    if (e->kind == E_COMPOUND) { scan_fp_initz(e->cinit); return; }
    if (e->kind == E_CALL) {
        // a direct call's callee (a plain function name) is NOT address-taken
        if (e->a && e->a->kind == E_IDENT) {
            sym *s = st_find(e->a->sval);
            if (!s || s->kind != SK_FUNC) scan_fp_expr(e->a);   // indirect target var
        } else {
            scan_fp_expr(e->a);
        }
        for (int i = 0; i < e->n_args; i++) scan_fp_expr(e->args[i]);
        return;
    }
    if (e->kind == E_IDENT) {
        sym *s = st_find(e->sval);
        if (s && s->kind == SK_FUNC) fp_add(e->sval);
        return;
    }
    scan_fp_expr(e->a); scan_fp_expr(e->b); scan_fp_expr(e->c);
    for (int i = 0; i < e->n_args; i++) scan_fp_expr(e->args[i]);
}

static void scan_fp_initz(initz *z)
{
    if (!z) return;
    if (!z->is_list) { scan_fp_expr(z->e); return; }
    for (int i = 0; i < z->n; i++) scan_fp_initz(z->items[i]);
}

static void scan_fp_stmt(stmt *s)
{
    if (!s) return;
    scan_fp_expr(s->e1); scan_fp_expr(s->e2); scan_fp_expr(s->e3);
    scan_fp_stmt(s->body); scan_fp_stmt(s->body2); scan_fp_stmt(s->init_stmt);
    for (int i = 0; i < s->n_items; i++) scan_fp_stmt(s->items[i]);
    for (decl *d = s->decls; d; d = d->next) {
        scan_fp_expr(d->init);
        scan_fp_initz(d->binit);
    }
}

// ---- heap-usage pre-scan ---------------------------------------------------
// new/delete desugar to malloc/free calls, so spotting a call to either (in any
// function body or initializer) tells us to declare __heap and emit the runtime
// — this must be known before the header is emitted, hence a pre-pass.

static void scan_heap_stmt(stmt *s);
static void scan_heap_initz(initz *z);

static void scan_heap_expr(expr *e)
{
    if (!e) return;
    if (e->kind == E_COMPOUND) { scan_heap_initz(e->cinit); return; }
    if (e->kind == E_NEW || e->kind == E_DELETE) g_uses_heap = 1;
    if (e->kind == E_CALL && e->a && e->a->kind == E_IDENT &&
        (!strcmp(e->a->sval, "malloc") || !strcmp(e->a->sval, "free")))
        g_uses_heap = 1;
    scan_heap_expr(e->a); scan_heap_expr(e->b); scan_heap_expr(e->c);
    for (int i = 0; i < e->n_args; i++) scan_heap_expr(e->args[i]);
}

static void scan_heap_initz(initz *z)
{
    if (!z) return;
    if (!z->is_list) { scan_heap_expr(z->e); return; }
    for (int i = 0; i < z->n; i++) scan_heap_initz(z->items[i]);
}

static void scan_heap_stmt(stmt *s)
{
    if (!s) return;
    scan_heap_expr(s->e1); scan_heap_expr(s->e2); scan_heap_expr(s->e3);
    scan_heap_stmt(s->body); scan_heap_stmt(s->body2); scan_heap_stmt(s->init_stmt);
    for (int i = 0; i < s->n_items; i++) scan_heap_stmt(s->items[i]);
    for (decl *d = s->decls; d; d = d->next) {
        scan_heap_expr(d->init);
        scan_heap_initz(d->binit);
    }
}

// ---- static-local initializers ---------------------------------------------
// A `static` local has fixed storage (like every local on this target), but its
// initializer must run ONCE at program start, not on each function entry. We
// collect them in a pre-pass and emit them at main's entry, like globals. The
// stored base name is already mangled (<func>_<var>), matching declare_local.
typedef struct { char *base; type *t; expr *init; initz *binit; } stinit_e;
#define MAX_STINIT 256
static stinit_e stinits[MAX_STINIT];
static int      n_stinit = 0;

static void collect_statics_stmt(const char *fn, stmt *s)
{
    if (!s) return;
    collect_statics_stmt(fn, s->body);
    collect_statics_stmt(fn, s->body2);
    collect_statics_stmt(fn, s->init_stmt);
    for (int i = 0; i < s->n_items; i++) collect_statics_stmt(fn, s->items[i]);
    for (decl *d = s->decls; d; d = d->next) {
        if (d->sclass == SC_STATIC && (d->init || d->binit) && n_stinit < MAX_STINIT) {
            size_t n = strlen(fn) + strlen(d->name) + 2;
            char *base = malloc(n); snprintf(base, n, "%s_%s", fn, d->name);
            stinits[n_stinit].base  = base;
            stinits[n_stinit].t     = d->dtype;
            stinits[n_stinit].init  = d->init;
            stinits[n_stinit].binit = d->binit;
            n_stinit++;
        }
    }
}

// ---- _Generic selection ----------------------------------------------------
// type compatibility for _Generic: same kind, matching signedness for ints and
// matching pointee for pointers. (No integer promotion; arrays don't decay.)
static int type_match(type *a, type *b)
{
    if (!a || !b) return 0;
    if (a->kind != b->kind) return 0;
    switch (a->kind) {
    case TY_INT:   return a->is_signed == b->is_signed;
    case TY_PTR:
    case TY_ARRAY: return type_match(a->base, b->base);
    case TY_STRUCT:return a == b;
    default:       return 1;        // float, char, void
    }
}

// pick the association whose type matches the controlling expression's type,
// else the `default:` association. Errors if neither exists.
static expr *generic_select(expr *e)
{
    type *ct = infer_type(e->a);
    expr *deflt = NULL;
    for (int i = 0; i < e->n_args; i++) {
        if (!e->gtypes[i]) { deflt = e->args[i]; continue; }
        if (type_match(ct, e->gtypes[i])) return e->args[i];
    }
    if (deflt) return deflt;
    msg_error(e->line, "_Generic: no association matches the controlling type");
    return e->args[0];
}

// ---- type inference (lightweight; fills e->etype bottom-up) ----------------

static type *infer_type(expr *e)
{
    if (!e) return NULL;
    if (e->etype) return e->etype;
    switch (e->kind) {
    case E_INT_LIT: case E_CHAR_LIT: e->etype = t_int(); break;
    case E_FLOAT_LIT: e->etype = t_float(); break;
    case E_STRING_LIT: e->etype = t_ptr(t_char()); break;
    case E_IDENT: {
        strct_field *mf = cur_class_member(e->sval);
        if (mf) { e->etype = mf->ftype; break; }    // unqualified data member
        sym *s = st_find(e->sval);
        if (!s) msg_error(e->line, "undefined '%s'", e->sval);
        if (s->stype && s->stype->is_ref) e->etype = s->stype->base;   // peel reference
        else                              e->etype = s->stype;
        break;
    }
    case E_BINOP: {
        type *lt = infer_type(e->a);
        type *rt = infer_type(e->b);
        if (lt && lt->kind == TY_STRUCT) {       // overloaded operator -> its return type
            const char *opm = op_method_name(e->op);
            char *masm = opm ? resolve_method(lt, opm) : NULL;
            if (masm) { sym *ms = st_find(masm); free(masm);
                        e->etype = (ms && ms->kind == SK_FUNC) ? ms->stype : t_int(); break; }
        }
        switch (e->op) {
            case OP_EQ: case OP_NE: case OP_LT: case OP_GT: case OP_LE: case OP_GE:
            case OP_LAND: case OP_LOR:
                e->etype = t_int(); break;
            case OP_MOD: case OP_BAND: case OP_BOR: case OP_BXOR: case OP_SHL: case OP_SHR:
                e->etype = t_int(); break;
            default:
                if (lt && rt && (type_is_float(lt) || type_is_float(rt))) e->etype = t_float();
                else if (type_is_ptr_or_arr(lt)) e->etype = lt;
                else if (type_is_ptr_or_arr(rt)) e->etype = rt;
                else e->etype = t_int();
                break;
        }
        break;
    }
    case E_UNOP: {
        type *t = infer_type(e->a);
        if (e->op == OP_LNOT || e->op == OP_BNOT) e->etype = t_int();
        else e->etype = t;
        break;
    }
    case E_ASSIGN:
        infer_type(e->b);
        e->etype = infer_type(e->a);
        break;
    case E_INDEX: {
        type *bt = infer_type(e->a);
        infer_type(e->b);
        if (bt && (bt->kind == TY_PTR || bt->kind == TY_ARRAY)) e->etype = bt->base;
        else { msg_error(e->line, "subscript on non-array/non-pointer"); }
        break;
    }
    case E_CALL: {
        for (int i = 0; i < e->n_args; i++) infer_type(e->args[i]);
        if (e->a->kind == E_MEMBER || e->a->kind == E_PMEMBER) {
            // method call: return type comes from the mangled method symbol
            type *ct = infer_type(e->a->a);
            if (e->a->kind == E_PMEMBER) ct = ct ? ct->base : NULL;
            if (ct && ct->kind == TY_STRUCT && ct->tag) {
                char *masm = cg_mangle_method(ct->tag, e->a->member);
                sym *s = st_find(masm); free(masm);
                e->etype = (s && s->kind == SK_FUNC) ? s->stype : t_int();
            } else e->etype = t_int();
            break;
        }
        if (e->a->kind == E_IDENT) {
            const char *fn = e->a->sval;
            // builtins: in() returns int, out() returns void; both are not "declared" identifiers
            if (!strcmp(fn, "in"))       { e->etype = t_int();  break; }
            if (!strcmp(fn, "out"))      { e->etype = t_void(); break; }
            if (!strcmp(fn, "malloc"))   { e->etype = t_ptr(t_void()); break; }
            if (!strcmp(fn, "free"))     { e->etype = t_void(); break; }
            func *ov = resolve_overload(fn, e->args, e->n_args);
            if (ov) { e->etype = ov->ret; break; }   // pick the matching overload's return type
            sym *s = st_find(fn);
            if (s) e->etype = s->stype;
            else { msg_error(e->line, "undefined function '%s'", fn); }
        } else {
            // indirect call via (*fp)(...) — the callee holds a function ID, not
            // a dereferenceable pointer, so don't type-check the deref itself.
            e->etype = t_int();
        }
        break;
    }
    case E_ADDR:   e->etype = t_ptr(infer_type(e->a)); break;
    case E_DEREF: {
        type *t = infer_type(e->a);
        if (!t || (t->kind != TY_PTR && t->kind != TY_ARRAY)) msg_error(e->line, "deref on non-pointer");
        e->etype = t->base;
        break;
    }
    case E_MEMBER: {
        type *t = infer_type(e->a);
        if (!t || t->kind != TY_STRUCT) msg_error(e->line, "'.' on non-struct");
        strct_field *f = t_struct_find(t, e->member);
        if (!f) msg_error(e->line, "no such field '%s'", e->member);
        e->etype = f->ftype;
        break;
    }
    case E_PMEMBER: {
        type *t = infer_type(e->a);
        if (!t || t->kind != TY_PTR || !t->base || t->base->kind != TY_STRUCT)
            msg_error(e->line, "'->' on non-pointer-to-struct");
        strct_field *f = t_struct_find(t->base, e->member);
        if (!f) msg_error(e->line, "no such field '%s'", e->member);
        e->etype = f->ftype;
        break;
    }
    case E_PREINC: case E_PREDEC: case E_POSTINC: case E_POSTDEC:
        e->etype = infer_type(e->a); break;
    case E_CAST: infer_type(e->a); e->etype = e->target_t; break;
    case E_NEW:    e->etype = t_ptr(e->target_t); break;
    case E_DELETE: infer_type(e->a); e->etype = t_void(); break;
    case E_COMPOUND: e->etype = e->target_t; break;
    case E_GENERIC: e->etype = infer_type(generic_select(e)); break;
    case E_SIZEOF_T: e->etype = t_int(); break;
    case E_SIZEOF_E: infer_type(e->a); e->etype = t_int(); break;
    case E_TERNARY: infer_type(e->a); infer_type(e->b); infer_type(e->c); e->etype = e->b->etype; break;
    case E_COMMA: infer_type(e->a); e->etype = infer_type(e->b); break;
    }
    return e->etype;
}

// ---- literal emission helpers ----------------------------------------------

static void emit_load_int(long v)   { emit("LOD %ld", v); }
static void emit_load_float(double v)
{
    char buf[64]; snprintf(buf, sizeof(buf), "%.15g", v);
    int has_dot = 0; for (char *p = buf; *p; p++) if (*p == '.' || *p == 'e' || *p == 'E') { has_dot = 1; break; }
    if (!has_dot) strcat(buf, ".0");
    emit("LOD %s", buf);
}

// ---- conditional branch ----------------------------------------------------
// JIZ tests the whole accumulator word (HDL: if_acc = |ula_out), so any value's
// C truthiness is honoured directly. No LIN;LIN normalisation needed — emit the
// expression and branch.

static void gen_bool(expr *e, const char *jz_target)
{
    // short-circuit && / || directly into the branch (no 0/1 materialisation):
    //   a && b  : jump to jz if either side is false
    //   a || b  : jump to jz only if BOTH sides are false
    if (e->kind == E_BINOP && e->op == OP_LAND) {
        gen_bool(e->a, jz_target);
        gen_bool(e->b, jz_target);
        return;
    }
    if (e->kind == E_BINOP && e->op == OP_LOR) {
        char *tcont = fresh_label("or_t");   // condition-true continuation
        char *chkb  = fresh_label("or_b");
        gen_expr(e->a);
        emit("JIZ %s", chkb);                // a == 0 -> must test b
        emit("JMP %s", tcont);               // a != 0 -> whole OR is true
        emit("@%s NOP", chkb);
        gen_bool(e->b, jz_target);           // b == 0 -> jz; else fall through
        emit("@%s NOP", tcont);
        free(tcont); free(chkb);
        return;
    }
    gen_expr(e);
    emit("JIZ %s", jz_target);
}

// ---- lvalue address: leaves &lv in accumulator -----------------------------

// load the `this` pointer's VALUE (the current object's address) into acc
static void gen_load_this(void)
{
    sym *th = st_find("this");
    if (!th) { msg_internal("`this` outside a method"); return; }
    if (th->is_frame) { emit("LOD __fp"); if (th->frame_off) emit("ADD %d", th->frame_off); emit("LDA"); }
    else emit("LOD %s", th->asm_name);
}

static void gen_addr(expr *e)
{
    switch (e->kind) {
    case E_IDENT: {
        strct_field *mf = cur_class_member(e->sval);
        if (mf) { gen_load_this(); if (mf->offset > 0) emit("ADD %d", mf->offset); return; }
        sym *s = st_find(e->sval);
        if (!s) msg_error(e->line, "undefined '%s'", e->sval);
        // reference: its lvalue address is the address it stores (the referent)
        if (s->stype && s->stype->is_ref) {
            if (s->is_frame) { emit("LOD __fp"); if (s->frame_off) emit("ADD %d", s->frame_off); emit("LDA"); }
            else emit("LOD %s", s->asm_name);
            return;
        }
        // frame local/param of a recursive function: address is __fp + offset
        if (s->is_frame) {
            emit("LOD __fp"); if (s->frame_off) emit("ADD %d", s->frame_off);
            return;
        }
        // array/struct params hold the address of the object, so loading the
        // variable's value yields that address; everything else is addressed
        // by name via LEA.
        if (s->kind == SK_PARAM && s->stype &&
            (s->stype->kind == TY_ARRAY || s->stype->kind == TY_STRUCT))
            emit("LOD %s", s->asm_name);
        else
            emit("LEA %s", s->asm_name);
        return;
    }
    case E_DEREF:
        // *p as lvalue: the address is the value of p
        gen_expr(e->a);
        return;
    case E_INDEX: {
        type *bt = infer_type(e->a);
        int elem_sz = bt ? type_size_words(bt->base) : 1;
        // compute base address into acc
        if (bt && bt->kind == TY_ARRAY && e->a->kind == E_IDENT) {
            sym *s = st_find(e->a->sval);
            if (s && s->kind == SK_PARAM) emit("LOD %s", s->asm_name);
            else                          emit("LEA %s", s->asm_name);
        } else {
            // pointer: its value IS the base address
            gen_expr(e->a);
        }
        emit("PSH");                              // stack: base
        gen_expr(e->b);                           // acc: index
        if (elem_sz > 1) { emit("PSH"); emit("LOD %d", elem_sz); emit("S_MLT"); }
        emit("S_ADD");                            // acc = base + idx*sz
        return;
    }
    case E_MEMBER: {
        gen_addr(e->a);
        type *st = infer_type(e->a);
        strct_field *f = t_struct_find(st, e->member);
        if (!f) msg_error(e->line, "no field '%s'", e->member);
        if (f->offset > 0) emit("ADD %d", f->offset);
        return;
    }
    case E_PMEMBER: {
        gen_expr(e->a);                           // ptr value
        type *st = infer_type(e->a);
        if (!st || !st->base) msg_error(e->line, "bad -> target");
        strct_field *f = t_struct_find(st->base, e->member);
        if (!f) msg_error(e->line, "no field '%s'", e->member);
        if (f->offset > 0) emit("ADD %d", f->offset);
        return;
    }
    default:
        msg_error(e->line, "expression is not an lvalue");
    }
}

// returns the struct field a member-access refers to, or NULL
static strct_field *member_field(expr *e)
{
    if (e->kind == E_MEMBER) {
        type *st = infer_type(e->a);
        if (st && st->kind == TY_STRUCT) return t_struct_find(st, e->member);
    } else if (e->kind == E_PMEMBER) {
        type *pt = infer_type(e->a);
        if (pt && pt->base && pt->base->kind == TY_STRUCT) return t_struct_find(pt->base, e->member);
    }
    return NULL;
}

// implicit int<->float conversion of the value already in acc, given its source
// type `have` and whether the destination wants float.
static void coerce_acc(type *have, int want_float)
{
    int hf = have && have->kind == TY_FLOAT;
    if (want_float && !hf)      emit("I2F");
    else if (!want_float && hf) emit("F2I");
}

// evaluate e into acc, then coerce to float (want_float) or int.
static void gen_expr_num(expr *e, int want_float)
{
    gen_expr(e);
    coerce_acc(infer_type(e), want_float);
}

// copy an n-word struct: dest is an lvalue, src an expression that evaluates to
// the struct's base address (a struct rvalue decays to its address). Leaves the
// destination address in acc so a struct assignment yields the assigned object.
// Src is evaluated first so a nested struct copy inside it can't clobber our
// (freshly allocated) address temporaries.
static void gen_struct_copy(expr *dest, expr *src, int nwords)
{
    char dn[32], sn[32];
    snprintf(dn, sizeof(dn), "_scd%d", ++label_n);
    snprintf(sn, sizeof(sn), "_scs%d", ++label_n);
    char *dt = mangle_local(dn), *st = mangle_local(sn);
    const char *fn = cur_func_name ? cur_func_name : "global";
    st_add(SK_LOCAL_VAR, dn, dt, t_int()); log_var(fn, dn, 1, 0);
    st_add(SK_LOCAL_VAR, sn, st, t_int()); log_var(fn, sn, 1, 0);

    gen_expr(src);  emit("SET %s", st);     // st = &src
    gen_addr(dest); emit("SET %s", dt);     // dt = &dest
    for (int i = 0; i < nwords; i++) {
        emit("LOD %s", dt); if (i) emit("ADD %d", i); emit("PSH");
        emit("LOD %s", st); if (i) emit("ADD %d", i); emit("LDA");
        emit("STA");
    }
    emit("LOD %s", dt);                      // result = &dest
    free(dt); free(st);
}

// copy n words from src (which evaluates to a base address) into the named
// block `dest` (declared via #array) at offsets 0..n-1.
static void copy_to_block(const char *dest, expr *src, int n)
{
    char sn[32]; snprintf(sn, sizeof(sn), "_cbs%d", ++label_n);
    char *st = mangle_local(sn);
    const char *fn = cur_func_name ? cur_func_name : "global";
    st_add(SK_LOCAL_VAR, sn, st, t_int()); log_var(fn, sn, 1, 0);
    gen_expr(src); emit("SET %s", st);       // st = &src
    for (int i = 0; i < n; i++) {
        emit("LOD %d", i); emit("PSH");      // offset on stack
        emit("LOD %s", st); if (i) emit("ADD %d", i); emit("LDA"); // acc = src[i]
        emit("STI %s", dest);                // mem[dest+i] = acc
    }
    free(st);
}

// push one call argument. A struct is passed by value: copy it into a fresh
// per-call-site block and push that block's address (the callee treats a struct
// param like an array param — it holds the address). Scalars push their value.
static void gen_push_arg(expr *arg)
{
    type *at = infer_type(arg);
    if (at && at->kind == TY_STRUCT) {
        int w = type_size_words(at);
        char nm[32]; snprintf(nm, sizeof(nm), "_argt%d", ++label_n);
        char *an = mangle_local(nm);
        emit("#array %s 1 %d", an, w);
        copy_to_block(an, arg, w);
        emit("LEA %s", an); emit("PSH");
        free(an);
    } else {
        gen_expr(arg); emit("PSH");
    }
}

// push one call argument, honouring a reference parameter (T&): pass the
// address of the argument lvalue rather than its value.
static void gen_arg(expr *arg, type *ptype)
{
    if (ptype && ptype->is_ref) { gen_addr(arg); emit("PSH"); }
    else gen_push_arg(arg);
}

// ---- store: lv = val -------------------------------------------------------

static void gen_store(expr *lv, expr *val)
{
    // reject assignment to a const variable (direct, by-name)
    if (lv->kind == E_IDENT) {
        sym *cs = st_find(lv->sval);
        if (cs && cs->is_const) msg_error(lv->line, "assignment to const '%s'", lv->sval);
    }
    // whole-struct assignment: copy all words (not a single STA)
    {
        type *lvt = infer_type(lv);
        if (lvt && lvt->kind == TY_STRUCT) {
            gen_struct_copy(lv, val, type_size_words(lvt));
            return;
        }
    }
    // bitfield store: read-modify-write the containing word
    {
        strct_field *bf = (lv->kind == E_MEMBER || lv->kind == E_PMEMBER) ? member_field(lv) : NULL;
        if (bf && bf->is_bitfield) {
            long mask  = (1L << bf->bit_width) - 1;
            long clear = ~(mask << bf->bit_pos);
            char tn[64]; snprintf(tn, sizeof(tn), "_bf%d", ++label_n);
            char *ta = mangle_local(tn);
            st_add(SK_LOCAL_VAR, tn, ta, t_int());
            log_var(cur_func_name ? cur_func_name : "global", tn, 1, 0);
            gen_addr(lv);  emit("SET %s", ta);     // ta = &word
            emit("LOD %s", ta); emit("PSH");        // stack: [&word]  (STA address)
            emit("LOD %s", ta); emit("LDA");        // acc = old word
            emit("AND %ld", clear);                 // clear the field's bits
            emit("PSH");                            // stack: [&word, cleared]
            gen_expr(val);
            emit("AND %ld", mask);                  // value & field-mask
            if (bf->bit_pos > 0) {                  // shift left by bit_pos (stack form)
                emit("PSH"); emit("LOD %d", bf->bit_pos); emit("S_SHL");
            }
            emit("S_ORR");                          // cleared | shifted
            emit("STA");                            // mem[&word] = result
            free(ta);
            return;
        }
    }
    // fast path: simple scalar IDENT
    if (lv->kind == E_IDENT) {
        sym *s = st_find(lv->sval);
        if (s && !s->is_frame && s->stype && !s->stype->is_ref &&
            s->stype->kind != TY_ARRAY && s->stype->kind != TY_STRUCT) {
            gen_expr_num(val, s->stype->kind == TY_FLOAT);
            emit("SET %s", s->asm_name);
            return;
        }
    }
    // fast path: regular array element with constant base
    if (lv->kind == E_INDEX && lv->a->kind == E_IDENT) {
        sym *s = st_find(lv->a->sval);
        if (s && s->stype && s->stype->kind == TY_ARRAY && s->kind != SK_PARAM) {
            type *bt = s->stype;
            int elem_sz = type_size_words(bt->base);
            if (elem_sz == 1) {
                gen_expr(lv->b);              // index → acc
                emit("PSH");
                gen_expr_num(val, bt->base && bt->base->kind == TY_FLOAT);
                emit("STI %s", s->asm_name);
                return;
            }
        }
    }
    // general path via STA
    {
        type *lvt = infer_type(lv);
        gen_addr(lv);
        emit("PSH");
        gen_expr_num(val, lvt && lvt->kind == TY_FLOAT);
        emit("STA");
    }
}

// ---- main expression dispatcher -------------------------------------------

static void gen_expr(expr *e)
{
    if (!e) return;
    infer_type(e);

    switch (e->kind) {

    case E_INT_LIT: case E_CHAR_LIT: emit_load_int(e->ival); return;
    case E_FLOAT_LIT:                emit_load_float(e->fval); return;
    case E_STRING_LIT:
        // decays to the base address of its materialised global char array
        if (!e->member) msg_internal("string literal not pre-scanned");
        emit("LEA %s", e->member);
        return;

    case E_IDENT: {
        strct_field *mf = cur_class_member(e->sval);
        if (mf) {                              // unqualified data member -> this->member
            gen_load_this();
            if (mf->offset > 0) emit("ADD %d", mf->offset);
            emit("LDA");
            return;
        }
        sym *s = st_find(e->sval);
        if (!s) msg_error(e->line, "undefined '%s'", e->sval);
        if (s->stype && s->stype->is_ref) { gen_addr(e); emit("LDA"); return; }  // auto-deref
        if (s->kind == SK_FUNC) {
            // function used as a value -> its dispatch-table ID (function ptr)
            int id = fp_id_of(e->sval);
            if (id < 0) msg_internal("function '%s' not in dispatch table", e->sval);
            emit("LOD %d", id);
            return;
        }
        // frame scalar/pointer local: load mem[__fp + off] (arrays/structs in a
        // recursive function are rejected at declaration time)
        if (s->is_frame) { gen_addr(e); emit("LDA"); return; }
        if (s->stype && s->stype->kind == TY_ARRAY) {
            // array decays to base address
            if (s->kind == SK_PARAM) emit("LOD %s", s->asm_name);
            else                     emit("LEA %s", s->asm_name);
            return;
        }
        if (s->stype && s->stype->kind == TY_STRUCT) {
            // a struct rvalue decays to the base address of its storage block;
            // a by-value struct param holds that address (caller's copy)
            if (s->kind == SK_PARAM) emit("LOD %s", s->asm_name);
            else                     emit("LEA %s", s->asm_name);
            return;
        }
        emit("LOD %s", s->asm_name);
        return;
    }

    case E_ADDR:
        gen_addr(e->a);
        return;

    case E_DEREF:
        // a struct/array rvalue decays to its address (no load), like a[i] below
        if (e->etype && (e->etype->kind == TY_STRUCT || e->etype->kind == TY_ARRAY)) {
            gen_addr(e);
            return;
        }
        gen_expr(e->a);
        emit("LDA");
        return;

    case E_INDEX: {
        // array decay: a[i] whose element is itself an array (multi-dim) or a
        // struct produces the *address* of the sub-object, not a loaded word.
        if (e->etype && (e->etype->kind == TY_ARRAY || e->etype->kind == TY_STRUCT)) {
            gen_addr(e);
            return;
        }
        // fast path: regular scalar-array with scalar element
        type *bt = infer_type(e->a);
        if (bt && bt->kind == TY_ARRAY && type_size_words(bt->base) == 1
            && e->a->kind == E_IDENT) {
            sym *s = st_find(e->a->sval);
            if (s && s->kind != SK_PARAM) {
                gen_expr(e->b);
                emit("LDI %s", s->asm_name);
                return;
            }
        }
        gen_addr(e);
        emit("LDA");
        return;
    }

    case E_MEMBER:
    case E_PMEMBER: {
        // bitfield read: load the word, shift the field down, mask it off
        strct_field *bf = member_field(e);
        if (bf && bf->is_bitfield) {
            gen_addr(e);                    // &word (gen_addr adds the word offset)
            emit("LDA");                    // word value
            if (bf->bit_pos > 0) {          // logical shift right by bit_pos (stack form)
                emit("PSH"); emit("LOD %d", bf->bit_pos); emit("S_SHR");
            }
            emit("AND %ld", (1L << bf->bit_width) - 1);
            return;
        }
        // a field of array/struct type decays to its address (no load)
        if (e->etype && (e->etype->kind == TY_ARRAY || e->etype->kind == TY_STRUCT)) {
            gen_addr(e);
            return;
        }
        gen_addr(e);
        emit("LDA");
        return;
    }

    case E_ASSIGN:
        gen_store(e->a, e->b);
        // assignment expression yields the stored value; for v1 we leave acc
        // as it is after the store (which for SET-path is the value, for
        // STA-path is also the value)
        return;

    case E_BINOP: {
        op_kind op = e->op;
        type *lt = infer_type(e->a);
        type *rt = infer_type(e->b);
        // operator overloading: lhs is a class with operator<OP> -> a.op(b)
        if (lt && lt->kind == TY_STRUCT) {
            const char *opm = op_method_name(op);
            char *masm = opm ? resolve_method(lt, opm) : NULL;
            if (masm) {
                sym *ms = st_find(masm);
                gen_addr(e->a); emit("PSH");                  // this = &lhs
                type *pt = (ms && ms->param_types && ms->n_params > 1) ? ms->param_types[1] : NULL;
                gen_arg(e->b, pt);                            // the rhs operand
                emit("CAL %s", masm);
                free(masm);
                return;
            }
        }
        int lf = lt && lt->kind == TY_FLOAT;
        int rf = rt && rt->kind == TY_FLOAT;
        int is_float = lf || rf;     // operate in float if EITHER operand is float
        // operand signedness (for >> and comparisons; only when not float)
        int lhs_uns = lt && lt->kind == TY_INT && !lt->is_signed;
        int uns_cmp = !is_float && ((lhs_uns) || (rt && rt->kind == TY_INT && !rt->is_signed));

        // unsigned comparison: map to signed order by flipping the sign bit of
        // both operands (XOR with 1<<(NUBITS-1)), then do the signed compare.
        if (uns_cmp && !is_float &&
            (op == OP_LT || op == OP_GT || op == OP_LE || op == OP_GE)) {
            long sbit = 1L << (g_nubits - 1);
            gen_expr(e->a); emit("XOR %ld", sbit); emit("PSH");
            gen_expr(e->b); emit("XOR %ld", sbit);
            switch (op) {
                case OP_LT: emit("S_LES"); break;
                case OP_GT: emit("S_GRE"); break;
                case OP_LE: emit("S_GRE"); emit("LIN"); break;
                case OP_GE: emit("S_LES"); emit("LIN"); break;
                default: break;
            }
            return;
        }

        // logical &&/|| as a value: short-circuit, result normalised to 0/1.
        if (op == OP_LAND) {
            char *lz = fresh_label("and_z");
            char *le = fresh_label("and_e");
            gen_expr(e->a); emit("JIZ %s", lz);   // a == 0 -> 0
            gen_expr(e->b); emit("JIZ %s", lz);   // b == 0 -> 0
            emit("LOD 1"); emit("JMP %s", le);
            emit("@%s NOP", lz); emit("LOD 0");
            emit("@%s NOP", le);
            free(lz); free(le);
            return;
        }
        if (op == OP_LOR) {
            char *l1 = fresh_label("or_1");
            char *lz = fresh_label("or_z");
            char *le = fresh_label("or_e");
            gen_expr(e->a); emit("JIZ %s", l1);   // a == 0 -> test b; else true
            emit("LOD 1"); emit("JMP %s", le);
            emit("@%s NOP", l1);
            gen_expr(e->b); emit("JIZ %s", lz);   // b == 0 -> 0
            emit("LOD 1"); emit("JMP %s", le);
            emit("@%s NOP", lz); emit("LOD 0");
            emit("@%s NOP", le);
            free(l1); free(lz); free(le);
            return;
        }

        // rhs is a simple scalar identifier → use the memory-operand form.
        // ONLY for ops where mem-as-in1 / acc-as-in2 gives the right answer:
        // commutative ops, and the comparisons (verified below). Non-commutative
        // ops (SUB, DIV, MOD, SHL, SHR) compute `mem OP acc` = `rhs OP lhs` in
        // this form, which is reversed — they fall through to the stack path.
        if (e->b->kind == E_IDENT && lf == rf) {   // mem-form needs matching types
            sym *r = st_find(e->b->sval);
            if (r && !r->is_frame && r->stype && r->stype->kind != TY_ARRAY && r->stype->kind != TY_STRUCT) {
                gen_expr(e->a);
                switch (op) {
                    case OP_ADD: emit(is_float ? "F_ADD %s" : "ADD %s", r->asm_name); return;
                    case OP_MUL: emit(is_float ? "F_MLT %s" : "MLT %s", r->asm_name); return;
                    case OP_BAND: emit("AND %s", r->asm_name); return;
                    case OP_BOR:  emit("ORR %s", r->asm_name); return;
                    case OP_BXOR: emit("XOR %s", r->asm_name); return;
                    case OP_LT:   emit(is_float ? "F_GRE %s" : "GRE %s", r->asm_name); return; // mem>acc == lhs<rhs
                    case OP_GT:   emit(is_float ? "F_LES %s" : "LES %s", r->asm_name); return; // mem<acc == lhs>rhs
                    case OP_LE:   emit(is_float ? "F_LES %s" : "LES %s", r->asm_name); emit("LIN"); return;
                    case OP_GE:   emit(is_float ? "F_GRE %s" : "GRE %s", r->asm_name); emit("LIN"); return;
                    case OP_EQ:   emit("EQU %s", r->asm_name); return;
                    case OP_NE:   emit("EQU %s", r->asm_name); emit("LIN"); return;
                    default: break;   // SUB/DIV/MOD/SHL/SHR -> stack path (correct order)
                }
            }
        }
        // general path (coerce each operand to float when operating in float)
        gen_expr_num(e->a, is_float); emit("PSH"); gen_expr_num(e->b, is_float);
        switch (op) {
            case OP_ADD: emit(is_float ? "SF_ADD" : "S_ADD"); return;
            // stack=lhs (in1), acc=rhs (in2). SF_SU2 inverts in2 -> in1-in2 = lhs-rhs.
            // (SF_SU1 inverts in1 -> rhs-lhs, which is reversed.)
            case OP_SUB: if (is_float) emit("SF_SU2"); else { emit("NEG"); emit("S_ADD"); } return;
            case OP_MUL: emit(is_float ? "SF_MLT" : "S_MLT"); return;
            // unsigned div/mod: stack=[n], acc=d -> push d, call the software
            // unsigned divider (the ULA's DIV/MOD are signed). q in acc, r in _ud_r.
            case OP_DIV:
                if (is_float) { emit("SF_DIV"); return; }
                if (uns_cmp)  { emit("PSH"); emit("CAL udivmod"); g_uses_udiv = 1; return; }
                emit("S_DIV"); return;
            case OP_MOD:
                if (uns_cmp)  { emit("PSH"); emit("CAL udivmod"); emit("LOD udm_r"); g_uses_udiv = 1; return; }
                emit("S_MOD"); return;
            case OP_BAND: emit("S_AND"); return;
            case OP_BOR:  emit("S_ORR"); return;
            case OP_BXOR: emit("S_XOR"); return;
            case OP_SHL:  emit("S_SHL"); return;
            case OP_SHR:  emit(lhs_uns ? "S_SHR" : "S_SRS"); return;  // unsigned: logical; signed: arithmetic
            case OP_LT:   emit(is_float ? "SF_LES" : "S_LES"); return;
            case OP_GT:   emit(is_float ? "SF_GRE" : "S_GRE"); return;
            case OP_LE:   emit(is_float ? "SF_GRE" : "S_GRE"); emit("LIN"); return;
            case OP_GE:   emit(is_float ? "SF_LES" : "S_LES"); emit("LIN"); return;
            case OP_EQ:   emit("S_EQU"); return;
            case OP_NE:   emit("S_EQU"); emit("LIN"); return;
            default: msg_internal("unhandled binop %d", op);
        }
        return;
    }

    case E_UNOP:
        gen_expr(e->a);
        switch (e->op) {
            case OP_POS: return;
            case OP_NEG: emit(e->etype && e->etype->kind == TY_FLOAT ? "F_NEG" : "NEG"); return;
            case OP_BNOT: emit("INV"); return;
            case OP_LNOT: emit("LIN"); return;
            default: msg_internal("unhandled unop");
        }
        return;

    case E_PREINC: case E_PREDEC: {
        // ++lv / --lv : modify in place, result is the NEW value.
        expr *lv = e->a;
        int is_float = lv->etype && lv->etype->kind == TY_FLOAT;
        // step: 1 for scalars; sizeof(pointee) for pointers (pointer arithmetic)
        int step = (lv->etype && lv->etype->kind == TY_PTR) ? type_size_words(lv->etype->base) : 1;
        int delta = (e->kind == E_PREDEC) ? -step : step;
        // fast path: simple scalar identifier
        if (lv->kind == E_IDENT) {
            sym *s = st_find(lv->sval);
            if (s && !s->is_frame && s->stype && s->stype->kind != TY_ARRAY && s->stype->kind != TY_STRUCT) {
                emit("LOD %s", s->asm_name);
                if (is_float) emit("F_ADD %s", delta < 0 ? "-1.0" : "1.0");
                else          emit("ADD %d", delta);
                emit("SET %s", s->asm_name);
                return;
            }
        }
        // general lvalue: &lv computed once, read-modify-write via LDA/STA
        gen_addr(lv);                                    // acc = &lv
        emit("PSH");                                     // stack: [&lv]
        emit("LDA");                                     // acc = *lv
        if (is_float) emit("F_ADD %s", delta < 0 ? "-1.0" : "1.0");
        else          emit("ADD %d", delta);             // acc = new
        emit("STA");                                     // mem[&lv] = new ; acc = new
        return;
    }
    case E_POSTINC: case E_POSTDEC: {
        // lv++ / lv-- : modify in place, result is the OLD value.
        expr *lv = e->a;
        int is_float = lv->etype && lv->etype->kind == TY_FLOAT;
        int step = (lv->etype && lv->etype->kind == TY_PTR) ? type_size_words(lv->etype->base) : 1;
        int delta = (e->kind == E_POSTDEC) ? -step : step;
        // fast path: simple scalar identifier
        if (lv->kind == E_IDENT) {
            sym *s = st_find(lv->sval);
            if (s && !s->is_frame && s->stype && s->stype->kind != TY_ARRAY && s->stype->kind != TY_STRUCT) {
                emit("LOD %s", s->asm_name);
                emit("PSH");                             // stack: [old]  (result)
                if (is_float) emit("F_ADD %s", delta < 0 ? "-1.0" : "1.0");
                else          emit("ADD %d", delta);
                emit("SET %s", s->asm_name);
                emit("POP");                             // acc = old
                return;
            }
        }
        if (!is_float) {
            // integer: recover old by undoing the delta (exact in integer math)
            gen_addr(lv);                                // acc = &lv
            emit("PSH");                                 // stack: [&lv]
            emit("LDA");                                 // acc = old
            emit("ADD %d", delta);                       // acc = new
            emit("STA");                                 // mem[&lv] = new ; acc = new
            emit("ADD %d", -delta);                      // acc = new - delta = old
            return;
        }
        // float: re-add would round; preserve old in a temp instead
        {
            char tn[64]; snprintf(tn, sizeof(tn), "_pf%d", ++label_n);
            char *ta = mangle_local(tn);
            st_add(SK_LOCAL_VAR, tn, ta, t_int());
            log_var(cur_func_name ? cur_func_name : "global", tn, 1, 0);
            gen_addr(lv);                                // acc = &lv
            emit("SET %s", ta);                          // _pf = &lv
            emit("LDA");                                 // acc = old (mem[&lv])
            emit("PSH");                                 // stack: [old]  (result)
            emit("LOD %s", ta); emit("PSH");             // stack: [old, &lv]
            emit("LOD %s", ta); emit("LDA");             // acc = old (re-read)
            emit("F_ADD %s", delta < 0 ? "-1.0" : "1.0");// acc = new
            emit("STA");                                 // mem[&lv] = new ; stack: [old]
            emit("POP");                                 // acc = old
            free(ta);
            return;
        }
    }

    case E_CAST: {
        gen_expr(e->a);
        type *from = infer_type(e->a);
        type *to   = e->target_t;
        if (from && to) {
            if (type_is_int(from) && type_is_float(to)) emit("I2F");
            else if (type_is_float(from) && type_is_int(to)) emit("F2I");
            /* other casts are bit-reinterpret: no-op */
        }
        return;
    }

    case E_NEW: {
        type *t = e->target_t;
        int words = type_size_words(t);
        if (e->a) {                                   // new T[n] -> malloc(n*words)
            gen_expr(e->a);
            if (words != 1) { emit("PSH"); emit("LOD %d", words); emit("S_MLT"); }
            emit("PSH"); emit("CAL malloc");
            return;
        }
        emit("LOD %d", words); emit("PSH"); emit("CAL malloc");   // acc = ptr
        int poly = (t->kind == TY_STRUCT && t->n_vtbl > 0);
        char *ctor = (t->kind == TY_STRUCT && t->tag) ? resolve_method(t, "ctor") : NULL;
        sym  *cs   = ctor ? st_find(ctor) : NULL;
        int has_ctor = cs && cs->kind == SK_FUNC;
        if (!poly && !has_ctor) return;                            // plain alloc, ptr in acc
        emit("SET __new_p");                                       // stash the new object
        if (poly) {                                                // mem[obj+0] = &vtable
            emit("LOD __new_p"); emit("PSH"); emit("LEA %s__vtable", t->tag); emit("STA");
        }
        if (has_ctor) {
            emit("LOD __new_p"); emit("PSH");                      // result (kept on stack bottom)
            emit("LOD __new_p"); emit("PSH");                      // this (arg 0)
            for (int i = 0; i < e->n_args; i++) {
                type *pt = (cs->param_types && i + 1 < cs->n_params) ? cs->param_types[i + 1] : NULL;
                gen_arg(e->args[i], pt);
            }
            emit("CAL %s", ctor);
            emit("POP");                                           // acc = result
        } else {
            emit("LOD __new_p");                                   // acc = result
        }
        free(ctor);
        return;
    }
    case E_DELETE: {
        type *pt = infer_type(e->a);
        type *t = pt ? pt->base : NULL;
        if (!e->ival && t && t->kind == TY_STRUCT && t->tag) {
            char *dtor = resolve_method(t, "dtor");
            sym *ds = dtor ? st_find(dtor) : NULL;
            if (ds && ds->kind == SK_FUNC) { gen_expr(e->a); emit("PSH"); emit("CAL %s", dtor); }
            free(dtor);
        }
        gen_expr(e->a); emit("PSH"); emit("CAL free");
        return;
    }

    case E_SIZEOF_T: emit_load_int(type_size_words(e->target_t)); return;
    case E_SIZEOF_E: emit_load_int(type_size_words(infer_type(e->a))); return;

    case E_GENERIC: gen_expr(generic_select(e)); return;  // controlling expr not evaluated

    case E_COMPOUND: {
        // materialise an unnamed block, initialise it, and yield it: aggregates
        // decay to the block's address; a scalar literal yields its value.
        type *t = e->target_t;
        char nm[32]; snprintf(nm, sizeof(nm), "_cl%d", ++label_n);
        char *an = mangle_local(nm);
        if (t->kind == TY_ARRAY) emit("#array %s %d %d", an, innermost_code(t), type_size_words(t));
        else                     emit("#array %s 1 %d", an, type_size_words(t));
        emit_initz(an, 0, t, e->cinit);
        emit("LEA %s", an);
        if (t->kind != TY_ARRAY && t->kind != TY_STRUCT) emit("LDA");  // scalar value
        free(an);
        return;
    }

    case E_TERNARY: {
        char *else_l = fresh_label("ter_else");
        char *end_l  = fresh_label("ter_end");
        gen_bool(e->a, else_l);
        gen_expr(e->b);
        emit("JMP %s", end_l);
        emit("@%s NOP", else_l);
        gen_expr(e->c);
        emit("@%s NOP", end_l);
        free(else_l); free(end_l);
        return;
    }

    case E_COMMA:
        gen_expr(e->a);                            // value discarded
        gen_expr(e->b);
        return;

    case E_CALL: {
        // method call: obj.m(args) -> Class__m(&obj, args); p->m(args) -> Class__m(p, args)
        if (e->a->kind == E_MEMBER || e->a->kind == E_PMEMBER) {
            expr *obj = e->a->a;
            type *ct = infer_type(obj);
            if (e->a->kind == E_PMEMBER) ct = ct ? ct->base : NULL;
            if (!ct || ct->kind != TY_STRUCT || !ct->tag)
                msg_error(e->line, "method call on non-class value");
            char *masm = resolve_method(ct, e->a->member);
            if (!masm) msg_error(e->line, "no method '%s' in class '%s'", e->a->member, ct->tag);
            sym *ms = st_find(masm);
            int vslot = vtbl_slot(ct, e->a->member);
            if (vslot >= 0) {
                // virtual dispatch: fid = mem[mem[this] + slot], then indirect-call
                if (e->a->kind == E_MEMBER) gen_addr(obj); else gen_expr(obj);  // acc = this
                emit("PSH");                                  // this -> arg 0 (kept on stack)
                emit("LDA");                                  // acc = vptr = mem[this]
                if (vslot) emit("ADD %d", vslot);
                emit("LDA");                                  // acc = fid = mem[vptr+slot]
                emit("SET _fp_id");
                for (int i = 0; i < e->n_args; i++) {
                    type *pt = (ms && ms->param_types && i + 1 < ms->n_params) ? ms->param_types[i + 1] : NULL;
                    gen_arg(e->args[i], pt);
                }
                emit_dispatch_chain();
                free(masm);
                return;
            }
            if (e->a->kind == E_MEMBER) gen_addr(obj); else gen_expr(obj);  // this
            emit("PSH");
            for (int i = 0; i < e->n_args; i++) {            // param i+1 (this is param 0)
                type *pt = (ms && ms->param_types && i + 1 < ms->n_params) ? ms->param_types[i + 1] : NULL;
                gen_arg(e->args[i], pt);
            }
            emit("CAL %s", masm);
            free(masm);
            return;
        }
        // direct call to a named function (or in()/out() builtins)
        if (e->a->kind == E_IDENT) {
            const char *fn = e->a->sval;
            if (!strcmp(fn, "in")) {
                if (e->n_args != 1) msg_error(e->line, "in(port) takes 1 arg");
                if (e->args[0]->kind != E_INT_LIT) msg_error(e->line, "in() port must be a literal int");
                emit("INN %ld", e->args[0]->ival); return;
            }
            if (!strcmp(fn, "out")) {
                if (e->n_args != 2) msg_error(e->line, "out(port,val) takes 2 args");
                if (e->args[0]->kind != E_INT_LIT) msg_error(e->line, "out() port must be a literal int");
                gen_expr(e->args[1]);
                emit("OUT %ld", e->args[0]->ival); return;
            }
            if (!strcmp(fn, "malloc")) {       // malloc(nwords) -> payload addr (0=OOM)
                if (e->n_args != 1) msg_error(e->line, "malloc(nwords) takes 1 arg");
                gen_expr(e->args[0]); emit("PSH"); emit("CAL malloc");
                g_uses_heap = 1; return;
            }
            if (!strcmp(fn, "free")) {         // free(ptr)
                if (e->n_args != 1) msg_error(e->line, "free(ptr) takes 1 arg");
                gen_expr(e->args[0]); emit("PSH"); emit("CAL free");
                g_uses_heap = 1; return;
            }
            func *ov = resolve_overload(fn, e->args, e->n_args);
            if (ov) {                                  // resolved (overloaded or unique)
                decl *p = ov->params;
                for (int i = 0; i < e->n_args; i++) {
                    gen_arg(e->args[i], p ? p->dtype : NULL);
                    if (p) p = p->next;
                }
                for (; p; p = p->next) gen_arg(p->init, p->dtype);   // fill default arguments
                emit("CAL %s", ov->asm_label);
                return;
            }
            sym *s = st_find(fn);
            if (s && s->kind == SK_FUNC) {
                for (int i = 0; i < e->n_args; i++) {
                    type *pt = (s->param_types && i < s->n_params) ? s->param_types[i] : NULL;
                    gen_arg(e->args[i], pt);
                }
                emit("CAL %s", s->asm_name);
                return;
            }
            // fall through: callee is a variable holding a function ID -> indirect
        }
        // indirect call: callee is a function-pointer value (a variable, or *fp).
        // Inline the dispatch as a chain of direct CALs guarded by EQU/JIZ. Each
        // CAL is a depth-1 call straight from this caller (the same shape as a
        // normal call), so the target's RET returns here — avoiding the nested
        // CAL-under-conditional that the prefetch mishandles.
        {
            expr *fpv = (e->a->kind == E_DEREF) ? e->a->a : e->a;
            for (int i = 0; i < e->n_args; i++) gen_push_arg(e->args[i]);
            gen_expr(fpv);                          // acc = function ID
            emit("SET _fp_id");
            emit_dispatch_chain();
            return;
        }
    }
    }
    msg_internal("unhandled expr kind %d", e->kind);
}

// ---- statement codegen -----------------------------------------------------

// emit stores for an aggregate initializer into block `base` (declared via
// #array) at word offset `off`, recursing into nested brace lists. `t` is the
// type of the sub-object being initialised. A scalar slot takes a value; an
// array/struct slot initialised by a non-braced expression is copied word-by-
// word (the expression yields the object's address). Items carry an optional
// single-level designator (.field / [index]); positional items use a running
// cursor that a designator resets. Slots not written keep the .mif zero default.
static void emit_initz(const char *base, int off, type *t, initz *z)
{
    if (!z) return;
    if (!z->is_list) {
        infer_type(z->e);
        int agg = t && (t->kind == TY_ARRAY || t->kind == TY_STRUCT);
        if (!agg) {
            emit("LOD %d", off); emit("PSH");
            gen_expr_num(z->e, t && t->kind == TY_FLOAT);   // int<->float per slot
            emit("STI %s", base);
        } else {
            int n = type_size_words(t);
            char sn[32]; snprintf(sn, sizeof(sn), "_izs%d", ++label_n);
            char *st = mangle_local(sn);
            st_add(SK_LOCAL_VAR, sn, st, t_int());
            log_var(cur_func_name ? cur_func_name : "global", sn, 1, 0);
            gen_expr(z->e); emit("SET %s", st);          // st = &src
            for (int i = 0; i < n; i++) {
                emit("LOD %d", off + i); emit("PSH");
                emit("LOD %s", st); if (i) emit("ADD %d", i); emit("LDA");
                emit("STI %s", base);
            }
            free(st);
        }
        return;
    }
    if (t && t->kind == TY_ARRAY) {
        int esz = type_size_words(t->base);
        int cursor = 0;
        for (int k = 0; k < z->n; k++) {
            int idx = cursor;
            if (z->desigs[k].kind == DESIG_FIELD)
                msg_error(z->line, "field designator in array initializer");
            if (z->desigs[k].kind == DESIG_INDEX) idx = z->desigs[k].idx;
            cursor = idx + 1;
            emit_initz(base, off + idx * esz, t->base, z->items[k]);
        }
    } else if (t && t->kind == TY_STRUCT) {
        strct_field *f = t->fields;
        for (int k = 0; k < z->n; k++) {
            strct_field *target;
            if (z->desigs[k].kind == DESIG_INDEX)
                msg_error(z->line, "array designator in struct initializer");
            if (z->desigs[k].kind == DESIG_FIELD) {
                target = t_struct_find(t, z->desigs[k].name);
                if (!target) msg_error(z->line, "no field '%s' in initializer", z->desigs[k].name);
                f = target->next;
            } else {
                if (!f) msg_error(z->line, "too many initializers");
                target = f; f = f->next;
            }
            emit_initz(base, off + target->offset, target->ftype, z->items[k]);
        }
    } else {
        // braced list wrapping a scalar, e.g. `{ x }` -> take the first element
        if (z->n > 0) emit_initz(base, off, t, z->items[0]);
    }
}

static void declare_local(decl *d)
{
    // `auto x = init;` — deduce the variable's type from its initializer
    if (d->dtype && d->dtype->is_auto && d->init) {
        type *it = infer_type(d->init);
        if (it) d->dtype = it;
    }
    // recursive function: the local lives in the stack frame at __fp + offset
    if (cur_fn_recursive && d->sclass != SC_STATIC) {
        if (d->dtype && (d->dtype->kind == TY_ARRAY || d->dtype->kind == TY_STRUCT))
            msg_error(d->line, "array/struct local in a recursive function is not supported");
        sym *ls = st_add(SK_LOCAL_VAR, d->name, NULL, d->dtype);
        ls->is_const = d->is_const;
        ls->is_frame = 1; ls->frame_off = cur_frame_cursor;
        cur_frame_cursor += d->dtype ? type_size_words(d->dtype) : 1;
        if (d->init) {                                  // store init into the frame slot
            emit("LOD __fp"); if (ls->frame_off) emit("ADD %d", ls->frame_off); emit("PSH");
            if (d->dtype && d->dtype->is_ref) gen_addr(d->init);   // bind reference to address
            else gen_expr_num(d->init, d->dtype && d->dtype->kind == TY_FLOAT);
            emit("STA");
        }
        return;
    }

    char *aname = mangle_local(d->name);
    { sym *ls = st_add(SK_LOCAL_VAR, d->name, aname, d->dtype); ls->is_const = d->is_const; }
    int arr_words = (d->dtype && d->dtype->kind == TY_ARRAY) ? type_size_words(d->dtype) : 0;
    log_var(cur_func_name ? cur_func_name : "global", d->name,
            innermost_code(d->dtype), arr_words);
    // `static` locals keep fixed storage but are initialised ONCE at program
    // start (collected pre-pass, emitted at main entry) — skip the inline init.
    int is_static = (d->sclass == SC_STATIC);
    if (d->dtype && d->dtype->kind == TY_ARRAY) {
        // multi-dim arrays flatten to total word count; element type is the innermost scalar
        if (d->init_file) emit("#arrays %s %d %d \"%s\"", aname, innermost_code(d->dtype), arr_words, d->init_file);
        else              emit("#array %s %d %d",         aname, innermost_code(d->dtype), arr_words);
        if (!is_static && d->binit) emit_initz(aname, 0, d->dtype, d->binit);
    } else if (d->dtype && d->dtype->kind == TY_STRUCT) {
        emit("#array %s 1 %d", aname, type_size_words(d->dtype));
        if (is_static)          { /* deferred to program start */ }
        else if (d->binit)      emit_initz(aname, 0, d->dtype, d->binit);
        else if (d->init)       copy_to_block(aname, d->init, type_size_words(d->dtype)); // = struct expr
    } else if (d->init && !is_static) {
        if (d->dtype && d->dtype->is_ref) gen_addr(d->init);   // bind reference to address
        else gen_expr_num(d->init, d->dtype && d->dtype->kind == TY_FLOAT);
        emit("SET %s", aname);
    }
    free(aname);
}

// ---- switch dispatch -------------------------------------------------------
// A switch's case/default labels may appear ANYWHERE in its body (even nested
// inside a loop, e.g. Duff's device), so we walk the body, mint a label per
// case (stored on the node), and emit the EQU/JIZ dispatch chain. We do NOT
// descend into a nested switch — its cases belong to it.
static stmt *g_sw_default;
static void switch_dispatch(stmt *s, const char *tmp_name)
{
    if (!s || s->kind == S_SWITCH) return;
    if (s->kind == S_CASE) {
        s->label = fresh_label("case");
        emit("LOD %s", tmp_name);
        emit("EQU %ld", s->e1->ival);
        emit("LIN");
        emit("JIZ %s", s->label);      // jump to this case when discriminant matches
        switch_dispatch(s->body, tmp_name);
        return;
    }
    if (s->kind == S_DEFAULT) {
        s->label = fresh_label("default");
        g_sw_default = s;
        switch_dispatch(s->body, tmp_name);
        return;
    }
    switch_dispatch(s->body, tmp_name);
    switch_dispatch(s->body2, tmp_name);
    switch_dispatch(s->init_stmt, tmp_name);
    for (int i = 0; i < s->n_items; i++) switch_dispatch(s->items[i], tmp_name);
}

static void gen_stmt(stmt *s)
{
    if (!s) return;
    switch (s->kind) {
    case S_NULL: return;
    case S_EXPR: if (s->e1) { infer_type(s->e1); gen_expr(s->e1); } return;
    case S_BLOCK:
        st_push_scope();
        for (int i = 0; i < s->n_items; i++) gen_stmt(s->items[i]);
        st_pop_scope();
        return;

    case S_DECL:
        for (decl *d = s->decls; d; d = d->next) declare_local(d);
        return;

    case S_IF: {
        infer_type(s->e1);
        char *else_l = fresh_label("if_else");
        char *end_l  = fresh_label("if_end");
        gen_bool(s->e1, s->body2 ? else_l : end_l);
        gen_stmt(s->body);
        if (s->body2) {
            emit("JMP %s", end_l);
            emit("@%s NOP", else_l);
            gen_stmt(s->body2);
        }
        emit("@%s NOP", end_l);
        free(else_l); free(end_l);
        return;
    }

    case S_WHILE: {
        char *top = fresh_label("wh_top");
        char *end = fresh_label("wh_end");
        emit("@%s NOP", top);
        infer_type(s->e1);
        gen_bool(s->e1, end);
        loop_push(top, end);
        gen_stmt(s->body);
        loop_pop();
        emit("JMP %s", top);
        emit("@%s NOP", end);
        free(top); free(end);
        return;
    }

    case S_DOWHILE: {
        char *top = fresh_label("do_top");
        char *cont= fresh_label("do_cont");
        char *end = fresh_label("do_end");
        emit("@%s NOP", top);
        loop_push(cont, end);
        gen_stmt(s->body);
        loop_pop();
        emit("@%s NOP", cont);
        infer_type(s->e1);
        gen_bool(s->e1, end);
        emit("JMP %s", top);
        emit("@%s NOP", end);
        free(top); free(cont); free(end);
        return;
    }

    case S_FOR: {
        char *top  = fresh_label("for_top");
        char *cont = fresh_label("for_cont");
        char *end  = fresh_label("for_end");
        st_push_scope();
        if (s->init_stmt) gen_stmt(s->init_stmt);
        emit("@%s NOP", top);
        if (s->e1) { infer_type(s->e1); gen_bool(s->e1, end); }
        loop_push(cont, end);
        gen_stmt(s->body);
        loop_pop();
        emit("@%s NOP", cont);
        if (s->e2) { infer_type(s->e2); gen_expr(s->e2); }
        emit("JMP %s", top);
        emit("@%s NOP", end);
        st_pop_scope();
        free(top); free(cont); free(end);
        return;
    }

    case S_BREAK:
        if (brk_top > 0) emit("JMP %s", brk_stk[brk_top-1]);
        else msg_error(s->line, "break outside of loop/switch");
        return;
    case S_CONTINUE:
        if (loop_top == 0) msg_error(s->line, "continue outside of loop");
        emit("JMP %s", loop_stk[loop_top-1].cont_l);
        return;

    case S_RETURN:
        if (s->e1) {
            if (cur_func_ret && cur_func_ret->kind == TY_VOID)
                msg_error(s->line, "return value in void function");
            infer_type(s->e1);
            if (cur_func_ret && cur_func_ret->kind == TY_STRUCT) {
                // write the struct into this function's static result block and
                // return its address (a struct rvalue == its base address)
                char rb[80]; snprintf(rb, sizeof(rb), "_ret_%s", cur_func_name);
                copy_to_block(rb, s->e1, type_size_words(cur_func_ret));
                emit("LEA %s", rb);
            } else {
                gen_expr_num(s->e1, cur_func_ret && cur_func_ret->kind == TY_FLOAT);
            }
        }
        if (cur_func_name && strcmp(cur_func_name, "main") == 0) emit("JMP fim");
        else emit_fn_return();
        return;

    case S_GOTO:
        emit("JMP _l_%s_%s", cur_func_name ? cur_func_name : "g", s->label);
        return;

    case S_LABEL:
        emit("@_l_%s_%s NOP", cur_func_name ? cur_func_name : "g", s->label);
        gen_stmt(s->body);
        return;

    case S_ASM: {
        // emit the inline-asm text verbatim, one .asm line per '\n'
        const char *t = s->label ? s->label : "";
        const char *start = t;
        for (const char *p = t; ; p++) {
            if (*p == '\n' || *p == 0) {
                int n = (int)(p - start);
                if (n > 0) {
                    char buf[512];
                    if (n >= (int)sizeof(buf)) n = sizeof(buf) - 1;
                    memcpy(buf, start, n); buf[n] = 0;
                    emit("%s", buf);     // "%s" -> no format-injection from user text
                }
                if (*p == 0) break;
                start = p + 1;
            }
        }
        return;
    }

    case S_SWITCH: {
        // evaluate the discriminant once into a temp, emit the dispatch chain
        // (cases may be nested anywhere — Duff's device), then emit the body
        // normally; each case/default emits its minted label inline.
        infer_type(s->e1);
        char tmp[64]; snprintf(tmp, sizeof(tmp), "_sw_%d", ++label_n);
        char *tmp_name = mangle_local(tmp);
        st_add(SK_LOCAL_VAR, tmp, tmp_name, t_int());
        log_var(cur_func_name ? cur_func_name : "global", tmp, 1, 0);
        gen_expr(s->e1);
        emit("SET %s", tmp_name);

        char *end = fresh_label("sw_end");
        brk_push(end);

        stmt *saved_default = g_sw_default;
        g_sw_default = NULL;
        switch_dispatch(s->body, tmp_name);            // mint labels + EQU/JIZ chain
        if (g_sw_default) emit("JMP %s", g_sw_default->label);
        else              emit("JMP %s", end);
        g_sw_default = saved_default;

        st_push_scope();
        gen_stmt(s->body);                             // body; cases emit @label inline
        st_pop_scope();

        emit("@%s NOP", end);
        brk_pop();
        free(end);
        free(tmp_name);
        return;
    }

    case S_CASE: case S_DEFAULT:
        // reached while emitting a switch body: emit the label minted by
        // switch_dispatch, then the labelled statement (fall-through is natural).
        if (s->label) { emit("@%s NOP", s->label); gen_stmt(s->body); }
        else          msg_error(s->line, "case/default outside switch");
        return;
    }
}

// ---- top-level emission ----------------------------------------------------

static int has_main = 0;

// IEEE-754 float layout for a given word width: the standard interchange
// formats for 16/32/64 bits, and a sensible split otherwise (overridable per
// source with `#pragma yanc nbmant/nbexpo`). The compiler is width-parametric,
// so picking nubits selects the matching float format automatically — a program
// with no pragmas defaults to 32-bit / IEEE-754 single.
static void derive_ieee(int w, int *mant, int *expo)
{
    switch (w) {
    case 16: *expo = 5;  *mant = 10; return;   // binary16 (half)
    case 32: *expo = 8;  *mant = 23; return;   // binary32 (single)
    case 64: *expo = 11; *mant = 52; return;   // binary64 (double)
    default:
        *expo = (w >= 32) ? 8 : 5;             // best effort for odd widths
        *mant = w - 1 - *expo;
        if (*mant < 1) { *mant = 1; *expo = w - 2; }
        return;
    }
}

static void emit_header(unit *u)
{
    int nb = u->nubits >= 0 ? u->nubits : CFG_NUBITS;
    int mant, expo;
    derive_ieee(nb, &mant, &expo);
    if (u->nbmant >= 0) mant = u->nbmant;      // explicit pragma overrides
    if (u->nbexpo >= 0) expo = u->nbexpo;

    emit("NOP");
    emit("#PRNAME %s", u->prname ? u->prname : "prog");
    emit("#NUBITS %d", nb);
    emit("#NDSTAC %d", u->ndstac >= 0 ? u->ndstac : CFG_NDSTAC);
    emit("#SDEPTH %d", u->sdepth >= 0 ? u->sdepth : CFG_SDEPTH);
    emit("#NUIOIN %d", u->nuioin >= 0 ? u->nuioin : CFG_NUIOIN);
    emit("#NUIOOU %d", u->nuioou >= 0 ? u->nuioou : CFG_NUIOOU);
    emit("#NBMANT %d", mant);
    emit("#NBEXPO %d", expo);
    emit("#NUGAIN %d", u->nugain >= 0 ? u->nugain : CFG_NUGAIN);
}

static void emit_global_arrays(unit *u)
{
    for (int i = 0; i < u->n_globals; i++) {
        decl *d = u->globals[i];
        if (!d->dtype) continue;
        if (d->dtype->kind == TY_ARRAY) {
            int arr_words = type_size_words(d->dtype);
            if (d->init_file)
                emit("#arrays %s %d %d \"%s\"", d->name, innermost_code(d->dtype), arr_words, d->init_file);
            else
                emit("#array %s %d %d", d->name, innermost_code(d->dtype), arr_words);
        } else if (d->dtype->kind == TY_STRUCT) {
            emit("#array %s 1 %d", d->name, type_size_words(d->dtype));
        }
    }
}

// declare a global char array per string literal (1 word/char + NUL)
static void emit_string_arrays(void)
{
    for (int i = 0; i < strtab_n; i++)
        emit("#array %s 1 %d", strtab[i].label, strtab[i].len + 1);
}

// fill each string array at main entry: byte-by-byte stores + NUL terminator
static void emit_string_inits(void)
{
    for (int i = 0; i < strtab_n; i++) {
        for (int k = 0; k < strtab[i].len; k++) {
            emit("LOD %d", k);
            emit("PSH");
            emit("LOD %d", (unsigned char)strtab[i].bytes[k]);
            emit("STI %s", strtab[i].label);
        }
        // NUL terminator at index len (memory defaults to 0, but be explicit)
        emit("LOD %d", strtab[i].len);
        emit("PSH");
        emit("LOD 0");
        emit("STI %s", strtab[i].label);
    }
}

static void emit_global_scalar_inits(unit *u)
{
    for (int i = 0; i < u->n_globals; i++) {
        decl *d = u->globals[i];
        if (!d->dtype) continue;
        if (d->dtype->kind == TY_ARRAY || d->dtype->kind == TY_STRUCT) {
            if (d->binit) emit_initz(d->name, 0, d->dtype, d->binit);
            continue;
        }
        if (!d->init) continue;
        gen_expr_num(d->init, d->dtype->kind == TY_FLOAT);
        emit("SET %s", d->name);
    }
}

// emit the one-time initializers for `static` locals (collected pre-pass)
static void emit_static_inits(void)
{
    for (int i = 0; i < n_stinit; i++) {
        stinit_e *e = &stinits[i];
        if (e->binit) { emit_initz(e->base, 0, e->t, e->binit); continue; }
        if (!e->init) continue;
        if (e->t && (e->t->kind == TY_ARRAY || e->t->kind == TY_STRUCT)) {
            copy_to_block(e->base, e->init, type_size_words(e->t));   // static aggregate = expr
        } else {
            gen_expr_num(e->init, e->t && e->t->kind == TY_FLOAT);
            emit("SET %s", e->base);
        }
    }
}

// total words of all local declarations in a body (for sizing a stack frame)
static int count_frame_locals(stmt *s)
{
    if (!s) return 0;
    int sum = 0;
    for (decl *d = s->decls; d; d = d->next) sum += d->dtype ? type_size_words(d->dtype) : 1;
    sum += count_frame_locals(s->body);
    sum += count_frame_locals(s->body2);
    sum += count_frame_locals(s->init_stmt);
    for (int i = 0; i < s->n_items; i++) sum += count_frame_locals(s->items[i]);
    return sum;
}

// emit a function return: for a recursive function, restore the caller's frame
// (SP=FP; FP=mem[FP]) around the return value, then RET.
static void emit_fn_return(void)
{
    if (cur_fn_recursive) {
        emit("SET __ret");                                  // preserve the return value
        emit("LOD __fp"); emit("SET __sp");                 // free this frame: SP = FP
        emit("LOD __fp"); emit("LDA"); emit("SET __fp");    // FP = caller FP (saved at frame base)
        emit("LOD __ret");
    }
    emit("RET");
}

static void emit_function(func *f, unit *u, int is_main)
{
    if (!f->asm_label) f->asm_label = f->name;
    cur_func_name    = f->asm_label;   // unique label keeps overloads' locals distinct
    cur_func_ret     = f->ret;
    cur_fn_recursive = f->is_recursive;
    cur_method_class = f->method_of;
    st_enter_func(f->asm_label);
    st_push_scope();

    decl *plist[64]; int np = 0;
    for (decl *p = f->params; p && np < 64; p = p->next) plist[np++] = p;

    if (cur_fn_recursive) {
        // frame layout: [0] = saved FP, [1..np] = params, [np+1..] = locals
        f->frame_size = 1 + np + count_frame_locals(f->body);
        cur_frame_cursor = 1 + np;
        for (int i = 0; i < np; i++) {
            sym *s = st_add(SK_PARAM, plist[i]->name, NULL, plist[i]->dtype);
            s->is_frame = 1; s->frame_off = 1 + i;
        }
    } else {
        for (int i = 0; i < np; i++) {
            char *aname = mangle_local(plist[i]->name);
            st_add(SK_PARAM, plist[i]->name, aname, plist[i]->dtype);
            log_var(f->name, plist[i]->name, type_code_for(plist[i]->dtype),
                    plist[i]->dtype && plist[i]->dtype->kind == TY_ARRAY ? plist[i]->dtype->arr_size : 0);
            free(aname);
        }
    }

    emit("@%s NOP", f->asm_label);

    if (!is_main && f->ret && f->ret->kind == TY_STRUCT)
        emit("#array _ret_%s 1 %d", f->asm_label, type_size_words(f->ret));

    if (cur_fn_recursive) {
        // prologue: push a new frame
        emit("LOD __sp"); emit("PSH"); emit("LOD __fp"); emit("STA");      // mem[SP] = FP
        emit("LOD __sp"); emit("SET __fp");                                // FP = SP
        emit("LOD __sp"); emit("ADD %d", f->frame_size); emit("SET __sp"); // SP += frame_size
        for (int i = np - 1; i >= 0; i--) {                                // args -> frame slots
            sym *sy = st_find(plist[i]->name);
            emit("POP"); emit("SET __argv");
            emit("LOD __fp"); if (sy->frame_off) emit("ADD %d", sy->frame_off); emit("PSH");
            emit("LOD __argv"); emit("STA");
        }
    } else if (np > 0) {
        for (int i = np - 1; i >= 0; i--) {
            sym *sy = st_find(plist[i]->name);
            emit("POP"); emit("SET %s", sy->asm_name);
        }
    }

    if (is_main) {
        if (g_any_recursive) { emit("LEA __cstk"); emit("SET __sp"); }   // init the call stack
        if (g_uses_heap) {                                               // init the heap
            emit("LEA __heap"); emit("SET __hp");                        // bump = base
            emit("LEA __heap"); emit("ADD %d", CFG_HEAPSZ); emit("SET __hend");
            emit("LOD 0"); emit("SET __flist");                          // empty free list
        }
        emit_vtable_inits();                                             // fill class vtables
        emit_string_inits(); emit_global_scalar_inits(u); emit_static_inits();
    }

    gen_stmt(f->body);

    if (is_main)        ; /* falls through to @fim */
    else                emit_fn_return();   // fall-off-end return (void)

    st_pop_scope();
    st_leave_func();
    cur_func_name = NULL;
    cur_method_class = NULL;
    cur_func_ret  = NULL;
    cur_fn_recursive = 0;
}

// software unsigned divide: caller pushes n then d; returns quotient in acc and
// leaves the remainder in the global udm_r. Shift-subtract, NUBITS iterations.
// The ULA's DIV/MOD are signed, so this is the unsigned path. `>=u` uses the
// sign-bit-flip trick (XOR sb, then signed compare).
// NOTE: the assembler's label/identifier grammar rejects a leading underscore
// (it is silently dropped), so jump/call targets must start with a letter.
static void emit_udivmod(void)
{
    long sb = 1L << (g_nubits - 1);
    emit("@udivmod NOP");
    emit("POP"); emit("SET udm_d");          // divisor
    emit("POP"); emit("SET udm_n");          // dividend
    emit("LOD 0"); emit("SET udm_q");
    emit("LOD 0"); emit("SET udm_r");
    emit("LOD 0"); emit("SET udm_c");
    emit("@udm_top NOP");
    emit("LOD udm_c"); emit("PSH"); emit("LOD %d", g_nubits); emit("S_LES"); emit("JIZ udm_end"); // while c < NUBITS
    // r = (r<<1) | (n >> (NUBITS-1))
    emit("LOD udm_r"); emit("PSH"); emit("LOD 1"); emit("S_SHL"); emit("PSH");           // (r<<1) on stack
    emit("LOD udm_n"); emit("PSH"); emit("LOD %d", g_nubits - 1); emit("S_SHR");          // top bit of n
    emit("S_ORR"); emit("SET udm_r");
    // n <<= 1 ; q <<= 1
    emit("LOD udm_n"); emit("PSH"); emit("LOD 1"); emit("S_SHL"); emit("SET udm_n");
    emit("LOD udm_q"); emit("PSH"); emit("LOD 1"); emit("S_SHL"); emit("SET udm_q");
    // if (r >=u d) { r -= d; q += 1 }
    emit("LOD udm_r"); emit("XOR %ld", sb); emit("PSH");
    emit("LOD udm_d"); emit("XOR %ld", sb); emit("S_LES"); emit("LIN"); emit("JIZ udm_noif"); // !(r<u d) -> r>=u d
    emit("LOD udm_d"); emit("NEG"); emit("ADD udm_r"); emit("SET udm_r");   // r = r - d
    emit("LOD udm_q"); emit("ADD 1"); emit("SET udm_q");                    // q += 1 (low bit was 0)
    emit("@udm_noif NOP");
    emit("LOD udm_c"); emit("ADD 1"); emit("SET udm_c");
    emit("JMP udm_top");
    emit("@udm_end NOP");
    emit("LOD udm_q");                       // return quotient
    emit("RET");
}

// First-fit free-list allocator over the __heap arena. Word-addressed, so
// malloc(n) reserves n payload words. Block layout at address b: mem[b]=size
// (payload words), payload = b+1..b+size; the returned/handled pointer is the
// PAYLOAD addr p=b+1 (>=1, since b>=0), so 0 is an unambiguous NULL even when
// __heap sits at data address 0. The free list (__flist) chains payload ptrs,
// storing the next-free ptr in each freed block's first payload word (mem[p]).
// __hp bumps for never-yet-allocated space; reuse is first-fit, no splitting or
// coalescing. Returns 0 (NULL) on exhaustion.
static void emit_heap(void)
{
    // ---- malloc(n) : payload words on the data stack -> payload addr in acc --
    emit("@malloc NOP");
    emit("POP"); emit("SET __m_sz");
    emit("LOD 0");       emit("SET __m_prev");         // prev payload ptr (0=none)
    emit("LOD __flist"); emit("SET __m_cur");          // cur payload ptr (0=end)
    emit("@mal_loop NOP");
    emit("LOD __m_cur"); emit("JIZ mal_bump");         // end of list -> bump-allocate
    emit("LOD __m_cur"); emit("PSH"); emit("LOD 1"); emit("NEG"); emit("S_ADD"); emit("LDA");
    emit("SET __m_csz");                               // csz = mem[cur-1] (block size)
    // if (csz >= sz) reuse: !(csz < sz)
    emit("LOD __m_csz"); emit("PSH"); emit("LOD __m_sz"); emit("S_LES"); emit("LIN");
    emit("JIZ mal_next");
    emit("LOD __m_cur"); emit("LDA"); emit("SET __m_lnk");          // link = mem[cur]
    emit("LOD __m_prev"); emit("JIZ mal_head");
    emit("LOD __m_prev"); emit("PSH"); emit("LOD __m_lnk"); emit("STA"); // mem[prev]=link
    emit("JMP mal_reuse");
    emit("@mal_head NOP");
    emit("LOD __m_lnk"); emit("SET __flist");          // __flist = link
    emit("@mal_reuse NOP");
    emit("LOD __m_cur"); emit("RET");                  // return payload (cur)
    emit("@mal_next NOP");
    emit("LOD __m_cur"); emit("SET __m_prev");
    emit("LOD __m_cur"); emit("LDA"); emit("SET __m_cur");          // cur = mem[cur] (next)
    emit("JMP mal_loop");
    // ---- bump-allocate: need = hp + sz + 1 ; require need <= hend -----------
    emit("@mal_bump NOP");
    emit("LOD __hp"); emit("PSH"); emit("LOD __m_sz"); emit("S_ADD"); emit("PSH"); emit("LOD 1"); emit("S_ADD");
    emit("SET __m_need");
    emit("LOD __m_need"); emit("PSH"); emit("LOD __hend"); emit("S_GRE"); emit("LIN"); // need<=hend
    emit("JIZ mal_oom");
    emit("LOD __hp"); emit("PSH"); emit("LOD __m_sz"); emit("STA");      // mem[hp]=sz
    emit("LOD __hp"); emit("ADD 1"); emit("SET __m_ret");                // ret=hp+1 (payload)
    emit("LOD __m_need"); emit("SET __hp");                              // hp=need
    emit("LOD __m_ret"); emit("RET");
    emit("@mal_oom NOP");
    emit("LOD 0"); emit("RET");                                         // NULL

    // ---- free(p) : push payload p onto the free list -----------------------
    emit("@free NOP");
    emit("POP"); emit("SET __m_cur");                  // p (payload addr)
    emit("LOD __m_cur"); emit("JIZ free_ret");         // free(0) is a no-op
    emit("LOD __m_cur"); emit("PSH"); emit("LOD __flist"); emit("STA");  // mem[p]=__flist
    emit("LOD __m_cur"); emit("SET __flist");                            // __flist=p
    emit("@free_ret NOP");
    emit("RET");
}

// collect the polymorphic classes (those with a vtable) and register each
// vtable slot's implementation as address-taken so it joins the dispatch table.
static void register_poly_classes(unit *u)
{
    n_poly = 0;
    for (int i = 0; i < u->n_funcs; i++) {
        type *c = u->funcs[i]->method_of;
        if (!c || c->n_vtbl <= 0) continue;
        int seen = 0;
        for (int j = 0; j < n_poly; j++) if (poly_cls[j] == c) { seen = 1; break; }
        if (!seen && n_poly < 64) poly_cls[n_poly++] = c;
    }
    for (int i = 0; i < n_poly; i++) {
        type *c = poly_cls[i];
        for (int s = 0; s < c->n_vtbl; s++) {
            char *impl = resolve_method(c, c->vtbl[s]);
            if (impl) { fp_add(impl); free(impl); }
        }
    }
}

// fill each class's vtable[slot] with the dispatch ID of its implementation
static void emit_vtable_inits(void)
{
    for (int i = 0; i < n_poly; i++) {
        type *c = poly_cls[i];
        for (int s = 0; s < c->n_vtbl; s++) {
            char *impl = resolve_method(c, c->vtbl[s]);
            int id = impl ? fp_id_of(impl) : 0;
            emit("LEA %s__vtable", c->tag); if (s) emit("ADD %d", s); emit("PSH");
            emit("LOD %d", id); emit("STA");          // mem[vtable+slot] = id
            if (impl) free(impl);
        }
    }
}

static void write_cmm_log(const char *tmp_dir)
{
    char path[2048]; snprintf(path, sizeof(path), "%s/cmm_log.txt", tmp_dir);
    FILE *f = fopen(path, "w");
    if (!f) { fprintf(stderr, "cnips: cannot open %s\n", path); return; }
    for (int i = 0; i < varlog_n; i++) {
        if (varlog[i].size > 0)
            fprintf(f, "%s %s %d %d\n", varlog[i].func, varlog[i].var, varlog[i].type_code, varlog[i].size);
        else
            fprintf(f, "%s %s %d\n", varlog[i].func, varlog[i].var, varlog[i].type_code);
    }
    fprintf(f, "num_ins %d\n", ins_count);
    fclose(f);
}

// ---- recursion analysis (hybrid frames) ------------------------------------
// A function needs a stack frame iff it can reach itself in the call graph.
// Edges: a direct call F->G; an indirect (function-pointer) call in F adds an
// edge F->A for EVERY address-taken function A (conservative: an fp could call
// any of them). Non-recursive functions keep fast fixed-address locals.
static void collect_calls_expr(expr *e, unit *u, char *row, int *indirect)
{
    if (!e) return;
    if (e->kind == E_CALL) {
        if (e->a && e->a->kind == E_IDENT) {
            int hit = 0;
            for (int j = 0; j < u->n_funcs; j++)
                if (strcmp(u->funcs[j]->name, e->a->sval) == 0) { row[j] = 1; hit = 1; break; }
            if (!hit && e->a->sval && strcmp(e->a->sval, "in") && strcmp(e->a->sval, "out"))
                *indirect = 1;                 // a variable holding a function id
        } else {
            *indirect = 1;
            collect_calls_expr(e->a, u, row, indirect);
        }
        for (int i = 0; i < e->n_args; i++) collect_calls_expr(e->args[i], u, row, indirect);
        return;
    }
    collect_calls_expr(e->a, u, row, indirect);
    collect_calls_expr(e->b, u, row, indirect);
    collect_calls_expr(e->c, u, row, indirect);
    for (int i = 0; i < e->n_args; i++) collect_calls_expr(e->args[i], u, row, indirect);
}

static void collect_calls_stmt(stmt *s, unit *u, char *row, int *indirect)
{
    if (!s) return;
    collect_calls_expr(s->e1, u, row, indirect);
    collect_calls_expr(s->e2, u, row, indirect);
    collect_calls_expr(s->e3, u, row, indirect);
    collect_calls_stmt(s->body, u, row, indirect);
    collect_calls_stmt(s->body2, u, row, indirect);
    collect_calls_stmt(s->init_stmt, u, row, indirect);
    for (int i = 0; i < s->n_items; i++) collect_calls_stmt(s->items[i], u, row, indirect);
    for (decl *d = s->decls; d; d = d->next) collect_calls_expr(d->init, u, row, indirect);
}

static void analyze_recursion(unit *u)
{
    int n = u->n_funcs;
    if (n <= 0) return;
    char *adj = calloc((size_t)n * n, 1);
    int  *ind = calloc(n, sizeof(int));
    for (int i = 0; i < n; i++) collect_calls_stmt(u->funcs[i]->body, u, &adj[(size_t)i*n], &ind[i]);
    for (int i = 0; i < n; i++) if (ind[i])
        for (int j = 0; j < n; j++)
            if (fp_id_of(u->funcs[j]->name) >= 0) adj[(size_t)i*n + j] = 1;
    // transitive closure: adj[i][i] becomes 1 iff i lies on a cycle
    for (int k = 0; k < n; k++)
        for (int i = 0; i < n; i++) if (adj[(size_t)i*n + k])
            for (int j = 0; j < n; j++) if (adj[(size_t)k*n + j]) adj[(size_t)i*n + j] = 1;
    for (int i = 0; i < n; i++) u->funcs[i]->is_recursive = adj[(size_t)i*n + i];
    free(adj); free(ind);
}

void codegen(FILE *out_file, unit *u, const char *tmp_dir)
{
    out_f = out_file;
    ins_count = 0; label_n = 0; varlog_n = 0; has_main = 0; strtab_n = 0; fptab_n = 0;
    g_uses_udiv = 0; n_stinit = 0;
    g_nubits = (u->nubits >= 0) ? u->nubits : CFG_NUBITS;

    // file-scope symtab: globals + function signatures
    for (int i = 0; i < u->n_globals; i++) {
        decl *d = u->globals[i];
        { sym *gs = st_add(SK_GLOBAL_VAR, d->name, d->name, d->dtype); gs->is_const = d->is_const; }
        int gw = (d->dtype && d->dtype->kind == TY_ARRAY) ? type_size_words(d->dtype) : 0;
        log_var("global", d->name, innermost_code(d->dtype), gw);
    }
    for (int i = 0; i < u->n_funcs; i++) {
        func *f = u->funcs[i];
        if (!st_find(f->name)) {
            sym *s = st_add(SK_FUNC, f->name, f->name, f->ret);
            s->n_params = f->n_params;
        }
        if (strcmp(f->name, "main") == 0) has_main = 1;
    }
    if (!has_main) msg_error(0, "program has no main() function");

    cg_unit = u;
    assign_overload_labels(u);

    // pre-scan every function body and global initialiser for string literals
    // and address-taken functions before any codegen emits a reference.
    g_uses_heap = 0;
    for (int i = 0; i < u->n_funcs; i++) {
        scan_strings_stmt(u->funcs[i]->body);
        scan_fp_stmt(u->funcs[i]->body);
        scan_heap_stmt(u->funcs[i]->body);
        collect_statics_stmt(u->funcs[i]->name, u->funcs[i]->body);
    }
    for (int i = 0; i < u->n_globals; i++) {
        scan_strings_expr(u->globals[i]->init);
        scan_fp_expr(u->globals[i]->init);
        scan_heap_expr(u->globals[i]->init);
        scan_strings_initz(u->globals[i]->binit);
        scan_fp_initz(u->globals[i]->binit);
    }

    register_poly_classes(u);   // address-take virtual impls before recursion analysis

    analyze_recursion(u);
    g_any_recursive = 0;
    for (int i = 0; i < u->n_funcs; i++) if (u->funcs[i]->is_recursive) g_any_recursive = 1;

    emit_header(u);
    emit_global_arrays(u);
    emit_string_arrays();
    if (g_any_recursive) emit("#array __cstk 0 %d", CSTK_SIZE);  // software call stack
    if (g_uses_heap)     emit("#array __heap 0 %d", CFG_HEAPSZ); // dynamic-alloc arena
    for (int i = 0; i < n_poly; i++)                            // per-class virtual tables
        emit("#array %s__vtable 1 %d", poly_cls[i]->tag, poly_cls[i]->n_vtbl);
    emit("JMP main");

    for (int i = 0; i < u->n_funcs; i++) {
        if (strcmp(u->funcs[i]->name, "main") != 0) emit_function(u->funcs[i], u, 0);
    }
    for (int i = 0; i < u->n_funcs; i++) {
        if (strcmp(u->funcs[i]->name, "main") == 0) emit_function(u->funcs[i], u, 1);
    }

    emit("@fim JMP fim");

    // emitted after @fim so main falls through into the halt loop, not the
    // helper; the helper is only ever entered through CAL.
    if (g_uses_udiv) emit_udivmod();
    if (g_uses_heap) emit_heap();

    if (tmp_dir) write_cmm_log(tmp_dir);
}
