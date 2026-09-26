// ----------------------------------------------------------------------------
// CPPComp — AST -> YANC .asm code generator ------------------------------------
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
//     and use LDI 0 / STI 0 to dereference. Simple scalar idents use LOD/SET fast.
// ----------------------------------------------------------------------------

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>

#include "../Headers/codegen.h"
#include "../Headers/config.h"
#include "../Headers/symtab.h"
#include "../Headers/messages.h"

// ---- output ----------------------------------------------------------------

static FILE *out_f;
static int   ins_count = 0;
static int   label_n   = 0;

// pc_<proc>_mem.txt: one 20-bit word per emitted instruction = the source line
// that produced it (negative codes are scaffolding, resolved via trad_cmm.txt).
// f_line is written in lockstep with ins_count from inside emit(); cg_line holds
// the line currently being emitted (-1 = INTERNAL, -3 = END, >=1 = source line).
static FILE *f_line  = NULL;
static int   cg_line = -1;
static int   g_nubits  = 32;     // effective word width (for unsigned compares)
static int   g_uses_udiv = 0;    // an unsigned / or % was emitted -> emit _udivmod
static int   g_uses_u2f  = 0;    // an unsigned -> float conversion was emitted -> emit u2f
static int   g_uses_f2u  = 0;    // a float -> unsigned conversion was emitted -> emit f2u
static int   g_nbmant    = 23;   // mantissa width (a float is zero iff its mantissa is)
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
// instantiated template clones -- a class template's methods and a function
// template's instances -- each with a unique asm_label. Filled by
// get_instance() / instantiate_ctmpl_methods(), emitted after @fim.
static func *g_inst[256]; static int g_n_inst = 0;
// the inliner (defined further down, next to gen_expr) is also used by
// gen_addr, which comes first in this file
static func *func_by_label(const char *lbl);
static expr *inlinable_body(func *f);
static int   inline_call(func *f, expr *body, expr *this_addr, expr **args, int n_args);
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

// inside a method, an unqualified name that is a static data member of the
// enclosing class resolves to the shared global Class__name (unless shadowed by
// a local). Returns that global's symbol, or NULL.
static sym *cur_class_static(const char *name)
{
    if (!cur_method_class || st_find_local(name)) return NULL;
    for (int i = 0; i < cur_method_class->n_statics; i++)
        if (!strcmp(cur_method_class->statics[i], name)) {
            char *gn = cg_mangle_method(cur_method_class->tag, name);
            sym *s = st_find(gn); free(gn);
            return s;
        }
    return NULL;
}

// if `member` is a static data member of struct type `t`, return its global
// symbol (Class__member); else NULL. Lets `obj.member` / `ptr->member` resolve a
// static the same way `Class::member` does (the object/pointer is not addressed).
static sym *struct_static_sym(type *t, const char *member)
{
    if (!t || t->kind != TY_STRUCT || !t->tag) return NULL;
    for (int i = 0; i < t->n_statics; i++)
        if (!strcmp(t->statics[i], member)) {
            char *gn = cg_mangle_method(t->tag, member);
            sym *s = st_find(gn); free(gn);
            return s;
        }
    return NULL;
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

// overloadable unary operator -> method name (matches the parser's op_arity_name)
static const char *op_unary_method_name(op_kind op)
{
    switch (op) {
        case OP_NEG: return "op_neg";
        case OP_POS: return "op_pos";
        default: return NULL;
    }
}

// compound-assignment operator -> method name (matches the parser's op_name)
static const char *op_compound_method_name(op_kind op)
{
    switch (op) {
        case OP_ADD: return "op_addeq"; case OP_SUB: return "op_subeq";
        case OP_MUL: return "op_muleq"; case OP_DIV: return "op_diveq";
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

// pick the constructor overload of `cls` matching the call arguments. Ctors are
// methods named "Class__ctor" with a leading implicit `this`; user args match
// params[1..]. Returns the func (use its asm_label / params), or NULL.
static func *resolve_ctor(type *cls, expr **args, int nargs)
{
    if (!cg_unit || !cls || !cls->tag) return NULL;
    char *mn = cg_mangle_method(cls->tag, "ctor");
    func *best = NULL; int best_score = -1;
    // a class template's constructors are clones in g_inst, not in the unit's
    // function list: without them, `TC<int> t;` and `TA<int> u(5);` ran no
    // constructor at all
    for (int i = 0; i < cg_unit->n_funcs + g_n_inst; i++) {
        func *f = (i < cg_unit->n_funcs) ? cg_unit->funcs[i] : g_inst[i - cg_unit->n_funcs];
        if (!f->method_of || strcmp(f->name, mn)) continue;
        decl *p0 = f->params ? f->params->next : NULL;     // skip `this`
        int nuser = f->n_params - 1;
        int required = 0;
        for (decl *q = p0; q; q = q->next) if (!q->init) required++;
        if (nargs < required || nargs > nuser) continue;
        int score = 0, ok = 1;
        decl *p = p0;
        for (int a = 0; a < nargs && p; a++, p = p->next) {
            char pc = type_code(p->dtype), ac = type_code(infer_type(args[a]));
            if (pc == ac) score += 2;
            else if ((pc == 'i' && ac == 'f') || (pc == 'f' && ac == 'i')) score += 1;
            else { ok = 0; break; }
        }
        if (ok && score > best_score) { best_score = score; best = f; }
    }
    free(mn);
    return best;
}

// 20-bit two's-complement binary (MSB first), matching cmmcomp's f_lin format.
static void emit_pc_line(int v)
{
    if (!f_line) return;
    char b[21];
    unsigned w = (unsigned)v & 0xFFFFF;   // keep low 20 bits
    for (int i = 0; i < 20; i++) b[i] = ((w >> (19 - i)) & 1) ? '1' : '0';
    b[20] = 0;
    fprintf(f_line, "%s\n", b);
}

// Emitted instructions are buffered (one entry per asm line) so a post-emit
// peephole can fuse adjacent ops before they reach the file. nofuse marks a
// line the peephole must leave alone (verbatim inline asm).
typedef struct { char *text; int line; int nofuse; } ibuf_e;
static ibuf_e *g_ibuf = NULL;
static int     g_ibuf_n = 0, g_ibuf_cap = 0;

static void ibuf_push(char *text, int line, int nofuse)
{
    if (g_ibuf_n == g_ibuf_cap) {
        g_ibuf_cap = g_ibuf_cap ? g_ibuf_cap * 2 : 256;
        g_ibuf = realloc(g_ibuf, (size_t)g_ibuf_cap * sizeof(ibuf_e));
    }
    g_ibuf[g_ibuf_n].text = text; g_ibuf[g_ibuf_n].line = line;
    g_ibuf[g_ibuf_n].nofuse = nofuse; g_ibuf_n++;
}

// buffer one emitted asm line (formatted); flushed after the peephole runs.
static void emit(const char *fmt, ...)
{
    va_list ap, ap2;
    va_start(ap, fmt); va_copy(ap2, ap);
    int n = vsnprintf(NULL, 0, fmt, ap);
    va_end(ap);
    char *s = malloc((size_t)n + 1);
    vsnprintf(s, (size_t)n + 1, fmt, ap2);
    va_end(ap2);
    ibuf_push(s, cg_line, 0);
}

// like emit but the line is verbatim user inline asm: never peephole-fused.
static void emit_verbatim(const char *text) { ibuf_push(strdup(text), cg_line, 1); }

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
    if (type_code == 0) return;   // ptr/struct/etc.: not published to cmm_log.txt
    if (varlog_n >= VARLOG_MAX) msg_internal("too many variables");
    varlog[varlog_n].func = strdup(func);
    varlog[varlog_n].var  = strdup(var);
    varlog[varlog_n].type_code = type_code;
    varlog[varlog_n].size = size;
    varlog_n++;
}

// cmm_log.txt type code, used by asmcomp to mirror the variable into the .vcd
// with the right gtkwave formatting. Only the two scalar kinds the dump path
// can render faithfully are published: TY_INT -> 1 (raw word), TY_FLOAT -> 2
// (decoded float). Everything else (ptr, struct, func, void) returns 0 = "do
// not publish" so it never reaches log_var -- a wrong int mirror at a fixed
// address is worse than no mirror. char/bool/unsigned are all TY_INT -> 1;
// double is the same one-word float as float -> 2.
static int type_code_for(const type *t)
{
    if (!t) return 0;
    if (t->kind == TY_INT)   return 1;
    if (t->kind == TY_FLOAT) return 2;
    return 0;        // ptr/struct/func/void: no faithful int|float rendering yet
}

// type code of the innermost scalar element (peels array nesting)
static int innermost_code(const type *t)
{
    while (t && t->kind == TY_ARRAY) t = t->base;
    return type_code_for(t);
}

// Homogeneous scalar fill-code for an aggregate, used as the #array type code so
// asmcomp zero-fills the data .mif with the correct encoding. YANC float 0.0 is
// NOT all-zero bits (it is f2mf(0.0) = 0x40000000 on the 32-bit target), so a
// float aggregate left at the int-0 default reads back as a bogus float — e.g.
// `std::array<float,N> a = {};` (a struct wrapping float[N]) whose unwritten
// slots then corrupt later float arithmetic. Returns 2 (float) only when EVERY
// scalar word is float; int / pointer / mixed fall back to 1, whose all-zero
// fill is a faithful int 0.
static int agg_fill_code(const type *t)
{
    if (!t) return 1;
    if (t->kind == TY_FLOAT) return 2;
    if (t->kind == TY_ARRAY) return agg_fill_code(t->base);
    if (t->kind == TY_STRUCT) {
        int code = 0;
        for (strct_field *f = t->fields; f; f = f->next) {
            int c = agg_fill_code(f->ftype);
            if (!code)       code = c;
            else if (code != c) return 1;   // mixed -> int-0 default
        }
        return code ? code : 1;
    }
    return 1;   // pointer / func / void: int-0 default
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

// live class-typed locals (constructed, pending dtor) for early-exit RAII:
// break/continue/return must run their dtors BEFORE jumping (the normal-exit
// emit_block_dtors sits after the jump in the .asm, so it is skipped at runtime).
// Parallel marks record the live depth at each loop/switch and at function entry
// so each exit unwinds exactly the right span (LIFO).
typedef struct { char *name; char *dtor; } live_local;
#define MAX_LIVE 256
static live_local g_live[MAX_LIVE];
static int        g_live_n = 0;
static int        brk_live[MAX_LOOPS * 2];   // g_live depth at each break boundary
static int        loop_live[MAX_LOOPS];      // g_live depth at each loop (for continue)

static void  brk_push(char *b) { if (brk_top < MAX_LOOPS*2) { brk_live[brk_top] = g_live_n; brk_stk[brk_top++] = b; } }
static void  brk_pop (void)    { if (brk_top > 0) brk_top--; }

static void loop_push(char *c, char *b)
{
    if (loop_top >= MAX_LOOPS) msg_internal("loop nesting too deep");
    loop_live[loop_top] = g_live_n;
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
static void gen_stmt_inner(stmt *s);
static void emit_initz(const char *base, int off, type *t, initz *z);
static void emit_fn_return(void);
static void gen_addr (expr *e);
static void gen_arg  (expr *arg, type *ptype);
static type *array_elems(type *t, int *n);
static void gen_store(expr *lv, expr *val);
static void gen_stmt (stmt *s);
static void gen_bool (expr *e, const char *jz_target);
static type *infer_type(expr *e);
static func *resolve_overload(const char *name, expr **args, int nargs);
static void  emit_vtable_inits(void);
static void  emit_dispatch_chain(void);
static func *find_template(const char *name);
static func *get_instance(func *t, expr **cargs, int ncargs, type **expl, int nexpl);
static type *subst_type(type *t, type **a, int n);
static void  deduce_targs(func *t, expr **cargs, int ncargs, type **targs);

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
typedef struct { char *base; type *t; expr *init; initz *binit; int line; expr **args; int nargs; } stinit_e;
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
        int n_el; type *et = array_elems(d->dtype, &n_el);
        int is_obj = et && et->kind == TY_STRUCT && et->tag;    // constructed at program start
        if (d->sclass == SC_STATIC && (d->init || d->binit || is_obj) && n_stinit < MAX_STINIT) {
            size_t n = strlen(fn) + strlen(d->name) + 2;
            char *base = malloc(n); snprintf(base, n, "%s_%s", fn, d->name);
            stinits[n_stinit].base  = base;
            stinits[n_stinit].t     = d->dtype;
            stinits[n_stinit].init  = d->init;
            stinits[n_stinit].binit = d->binit;
            stinits[n_stinit].line  = d->line;
            stinits[n_stinit].args  = d->ctor_args;
            stinits[n_stinit].nargs = d->n_ctor_args;
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
    case E_INT_LIT:  e->etype = e->is_uns ? t_uint() : t_int(); break;
    case E_CHAR_LIT: e->etype = t_int(); break;
    case E_FLOAT_LIT: e->etype = t_float(); break;
    case E_STRING_LIT: e->etype = t_ptr(t_char()); break;
    case E_IDENT: {
        strct_field *mf = cur_class_member(e->sval);
        if (mf) { e->etype = mf->ftype; break; }    // unqualified data member
        sym *stat = cur_class_static(e->sval);
        sym *s = stat ? stat : st_find(e->sval);
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
        else if (bt && bt->kind == TY_STRUCT && bt->tag) {       // overloaded operator[]
            char *masm = resolve_method(bt, "op_index");
            if (masm) { sym *ms = st_find(masm); free(masm);
                        type *rt = (ms && ms->kind == SK_FUNC) ? ms->stype : t_int();
                        e->etype = (rt && rt->is_ref) ? rt->base : rt; }  // peel reference
            else msg_error(e->line, "subscript on non-array/non-pointer");
        }
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
            if (!strcmp(fn, "__zerofill")) { e->etype = t_void(); break; }   // synth DMI aggregate zero-fill
            if (!strcmp(fn, "__construct_members")) { e->etype = t_void(); break; }  // synth ctor prologue
            if (!strcmp(fn, "__construct_at"))      { e->etype = t_void(); break; }  // synth member ctor call
            func *tm = find_template(fn);            // template call: substituted return type
            if (tm && tm->n_params == e->n_args) {
                type *ta[8] = {0}; deduce_targs(tm, e->args, e->n_args, ta);
                for (int i = 0; i < e->n_gtypes && i < 8; i++) ta[i] = e->gtypes[i];  // explicit args
                e->etype = subst_type(tm->ret, ta, tm->n_tparams); break;
            }
            func *ov = resolve_overload(fn, e->args, e->n_args);
            if (ov) { e->etype = ov->ret; break; }   // pick the matching overload's return type
            sym *s = st_find(fn);
            if (s) { e->etype = s->stype; break; }
            if (cur_method_class) {                  // unqualified sibling-method call -> this->fn()
                char *masm = resolve_method(cur_method_class, fn);
                int vslot = vtbl_slot(cur_method_class, fn);
                if (masm) { sym *ms = st_find(masm); free(masm);
                            e->etype = (ms && ms->kind == SK_FUNC) ? ms->stype : t_int(); break; }
                if (vslot >= 0) { e->etype = t_int(); break; }
            }
            msg_error(e->line, "undefined function '%s'", fn);
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
        sym *stat = struct_static_sym(t, e->member);
        if (stat) { e->etype = stat->stype; break; }     // static data member via instance
        strct_field *f = t_struct_find(t, e->member);
        if (!f) msg_error(e->line, "no such field '%s'", e->member);
        e->etype = f->ftype;
        break;
    }
    case E_PMEMBER: {
        type *t = infer_type(e->a);
        if (!t || t->kind != TY_PTR || !t->base || t->base->kind != TY_STRUCT)
            msg_error(e->line, "'->' on non-pointer-to-struct");
        sym *stat = struct_static_sym(t->base, e->member);
        if (stat) { e->etype = stat->stype; break; }     // static data member via pointer
        strct_field *f = t_struct_find(t->base, e->member);
        if (!f) msg_error(e->line, "no such field '%s'", e->member);
        e->etype = f->ftype;
        break;
    }
    case E_PREINC: case E_PREDEC: case E_POSTINC: case E_POSTDEC:
        e->etype = infer_type(e->a); break;
    case E_CAST: infer_type(e->a); e->etype = e->target_t; break;
    case E_NEW:    e->etype = t_ptr(e->target_t); break;
    case E_TEMPOBJ:
        for (int i = 0; i < e->n_args; i++) infer_type(e->args[i]);
        e->etype = e->target_t; break;
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
    // 17 significant digits reproduce the double exactly, and the assembler
    // now reads an exponent (its encoder, yanc_num, rounds that text once to
    // the target mantissa): %.20f, used while the lexers rejected "1e-05",
    // turned everything below ~1e-20 into 0.0 and overflowed the buffer above
    // ~1e75. A text with neither '.' nor an exponent would read as an int
    // constant, so it gets ".0".
    char buf[40]; snprintf(buf, sizeof(buf), "%.17g", v);
    if (!strpbrk(buf, ".eE")) strcat(buf, ".0");
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

// Constant folding: an int expression of literals only (a constexpr like
// `L >> 1` or `FL - 1` arrives as literals) is computed here, in the word's
// arithmetic -- NUBITS-bit two's complement, wrapping; / and % truncate; >>
// on a signed int is arithmetic. Not folded: unsigned literals, division by
// zero, INT_MIN / -1, a shift out of 0..NUBITS-1. The node itself becomes the
// literal, so a memory-operand form upstream sees a literal too.
static long cf_wrap(long long v)
{
    unsigned long long m = (g_nubits >= 64) ? ~0ULL : ((1ULL << g_nubits) - 1);
    unsigned long long u = (unsigned long long)v & m;
    if (g_nubits < 64 && (u >> (g_nubits - 1)) & 1) return (long)(long long)(u | ~m);
    return (long)u;
}
static int cfold(expr *e, long *v)
{
    if (!e) return 0;
    if (e->kind == E_INT_LIT || e->kind == E_CHAR_LIT) {
        if (e->is_uns) return 0;
        *v = e->ival; return 1;
    }
    long a, b;
    if (e->kind == E_UNOP) {
        if (!cfold(e->a, &a)) return 0;
        switch (e->op) {
            case OP_NEG:  *v = cf_wrap(-(long long)a); return 1;
            case OP_POS:  *v = a;                      return 1;
            case OP_BNOT: *v = cf_wrap(~(long long)a); return 1;
            case OP_LNOT: *v = !a;                     return 1;
            default: return 0;
        }
    }
    if (e->kind != E_BINOP || !cfold(e->a, &a) || !cfold(e->b, &b)) return 0;
    long lmin = -(1L << (g_nubits - 1));
    switch (e->op) {
        case OP_ADD:  *v = cf_wrap((long long)a + b); return 1;
        case OP_SUB:  *v = cf_wrap((long long)a - b); return 1;
        case OP_MUL:  *v = cf_wrap((long long)a * b); return 1;
        case OP_DIV:  if (b == 0 || (a == lmin && b == -1)) return 0; *v = a / b; return 1;
        case OP_MOD:  if (b == 0 || (a == lmin && b == -1)) return 0; *v = a % b; return 1;
        case OP_BAND: *v = a & b; return 1;
        case OP_BOR:  *v = a | b; return 1;
        case OP_BXOR: *v = a ^ b; return 1;
        case OP_SHL:  if (b < 0 || b >= g_nubits) return 0; *v = cf_wrap((long long)((unsigned long long)a << b)); return 1;
        case OP_SHR:  if (b < 0 || b >= g_nubits) return 0; *v = a >> b; return 1;
        case OP_EQ:   *v = a == b; return 1;  case OP_NE: *v = a != b; return 1;
        case OP_LT:   *v = a <  b; return 1;  case OP_GT: *v = a >  b; return 1;
        case OP_LE:   *v = a <= b; return 1;  case OP_GE: *v = a >= b; return 1;
        case OP_LAND: *v = a && b; return 1;  case OP_LOR: *v = a || b; return 1;
        default: return 0;
    }
}
// fold e in place when it is a literal-only int expression
static int cfold_node(expr *e)
{
    long v;
    if (!e || (e->kind != E_BINOP && e->kind != E_UNOP) || !cfold(e, &v)) return 0;
    type *t = infer_type(e);
    if (!t || t->kind != TY_INT) return 0;
    e->kind = E_INT_LIT; e->ival = v; e->is_uns = 0; e->a = e->b = NULL;
    return 1;
}

// Jump to target when e is TRUE (the test at the bottom of a loop). The ISA
// only has JIZ (jump if zero), so the value tested must be zero when e holds:
// an int comparison is inverted (a < b tests a >= b, one instruction with a
// literal bound), a == b tests a ^ b, a != b tests a == b; && and || are
// split; anything else (float, a plain value) tests !e.
static void gen_jump_true(expr *e, const char *target)
{
    if (e->kind == E_BINOP && e->op == OP_LOR) {
        gen_jump_true(e->a, target);
        gen_jump_true(e->b, target);
        return;
    }
    if (e->kind == E_BINOP && e->op == OP_LAND) {
        char *skip = fresh_label("and_f");
        gen_bool(e->a, skip);                   // a false -> the whole && is false
        gen_jump_true(e->b, target);
        emit("@%s NOP", skip);
        free(skip);
        return;
    }
    if (e->kind == E_BINOP) {
        type *lt = infer_type(e->a), *rt = infer_type(e->b);
        int ints = lt && rt && lt->kind == TY_INT && rt->kind == TY_INT;
        int inv = -1;
        switch (e->op) {
            case OP_LT: inv = OP_GE; break;  case OP_GE: inv = OP_LT; break;
            case OP_GT: inv = OP_LE; break;  case OP_LE: inv = OP_GT; break;
            case OP_NE: inv = OP_EQ; break;
            case OP_EQ: inv = OP_BXOR; break;          // zero exactly when a == b
            default: break;
        }
        if (ints && inv >= 0) {
            expr ne = *e; ne.op = (op_kind)inv; ne.etype = NULL;
            gen_bool(&ne, target);                     // jumps when !e is false
            return;
        }
    }
    gen_bool(ast_unop(OP_LNOT, e, e->line), target);
}

// `for (k = c0; k OP c1; ...)` / `for (int k = c0; ...)` with int literals:
// is the condition known to hold on entry? Then the entry test can go (the
// loop tests only at the bottom): smaller and faster.
static int for_entry_holds(stmt *s)
{
    const char *name = NULL; long c0 = 0;
    stmt *in = s->init_stmt;
    if (!in || !s->e1) return 0;
    if (in->kind == S_DECL && in->decls && !in->decls->next && in->decls->init &&
        in->decls->init->kind == E_INT_LIT && in->decls->dtype && in->decls->dtype->kind == TY_INT) {
        name = in->decls->name; c0 = in->decls->init->ival;
    } else if (in->kind == S_EXPR && in->e1 && in->e1->kind == E_ASSIGN && in->e1->op == OP_NONE &&
               in->e1->a && in->e1->a->kind == E_IDENT && in->e1->b && in->e1->b->kind == E_INT_LIT) {
        type *t = infer_type(in->e1->a);
        if (!t || t->kind != TY_INT) return 0;
        name = in->e1->a->sval; c0 = in->e1->b->ival;
    } else return 0;
    expr *c = s->e1;
    if (c->kind != E_BINOP || !c->a || !c->b) return 0;
    int op = c->op; long c1;
    if (c->a->kind == E_IDENT && !strcmp(c->a->sval, name) && c->b->kind == E_INT_LIT) c1 = c->b->ival;
    else if (c->b->kind == E_IDENT && !strcmp(c->b->sval, name) && c->a->kind == E_INT_LIT) {
        c1 = c->a->ival;                                         // c OP k: mirror
        if (op == OP_LT) op = OP_GT; else if (op == OP_GT) op = OP_LT;
        else if (op == OP_LE) op = OP_GE; else if (op == OP_GE) op = OP_LE;
    } else return 0;
    switch (op) {
        case OP_LT: return c0 <  c1;  case OP_LE: return c0 <= c1;
        case OP_GT: return c0 >  c1;  case OP_GE: return c0 >= c1;
        case OP_EQ: return c0 == c1;  case OP_NE: return c0 != c1;
        default:    return 0;
    }
}

// ---- lvalue address: leaves &lv in accumulator -----------------------------

// load the `this` pointer's VALUE (the current object's address) into acc
static void gen_load_this(void)
{
    sym *th = st_find("this");
    if (!th) { msg_internal("`this` outside a method"); return; }
    if (th->is_frame) { emit("LOD __fp"); if (th->frame_off) emit("ADD %d", th->frame_off); emit("LDI 0"); }
    else emit("LOD %s", th->asm_name);
}

static void gen_addr(expr *e)
{
    switch (e->kind) {
    case E_IDENT: {
        strct_field *mf = cur_class_member(e->sval);
        if (mf) { gen_load_this(); if (mf->offset > 0) emit("ADD %d", mf->offset); return; }
        sym *stat = cur_class_static(e->sval);
        sym *s = stat ? stat : st_find(e->sval);
        if (!s) msg_error(e->line, "undefined '%s'", e->sval);
        // reference: its lvalue address is the address it stores (the referent)
        if (s->stype && s->stype->is_ref) {
            if (s->is_frame) { emit("LOD __fp"); if (s->frame_off) emit("ADD %d", s->frame_off); emit("LDI 0"); }
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
        // overloaded subscript returning a reference: the call yields the
        // referent's address, which IS this lvalue's address.
        if (bt && bt->kind == TY_STRUCT && bt->tag) {
            char *masm = resolve_method(bt, "op_index");
            if (masm) {
                sym *ms = st_find(masm);
                {   // the WRITE side of a tiny operator[] (`a[i] = x`): the
                    // expanded body of a reference-returning accessor yields
                    // the element's address, which is this lvalue's address
                    func *cf = func_by_label(masm);
                    expr *bd = cf ? inlinable_body(cf) : NULL;
                    if (bd && cf->ret && cf->ret->is_ref &&
                        inline_call(cf, bd, e->a, &e->b, 1)) { free(masm); return; }
                }
                gen_addr(e->a); emit("PSH");              // this = &obj
                type *pt = (ms && ms->param_types && ms->n_params > 1) ? ms->param_types[1] : NULL;
                gen_arg(e->b, pt);
                emit("CAL %s", masm);
                free(masm);
                return;
            }
            free(masm);
            msg_error(e->line, "expression is not an lvalue");
        }
        int elem_sz = bt ? type_size_words(bt->base) : 1;
        // one-word elements and an index that is a literal or a plain int
        // variable: `base; ADD idx` (the memory-operand form) where the stack
        // path took PSH, the index load and S_ADD
        if (elem_sz == 1 && e->b && (e->b->kind == E_INT_LIT || e->b->kind == E_IDENT)) {
            sym *r = e->b->kind == E_IDENT ? st_find(e->b->sval) : NULL;
            int ok = e->b->kind == E_INT_LIT ||
                     (r && (r->kind == SK_LOCAL_VAR || r->kind == SK_PARAM || r->kind == SK_GLOBAL_VAR) &&
                      !r->is_frame && r->stype && r->stype->kind == TY_INT && !r->stype->is_ref);
            if (ok) {
                if (bt && bt->kind == TY_ARRAY) gen_addr(e->a); else gen_expr(e->a);
                if (e->b->kind == E_INT_LIT) { if (e->b->ival) emit("ADD %ld", e->b->ival); }
                else emit("ADD %s", r->asm_name);
                return;
            }
        }
        // compute base address into acc
        if (bt && bt->kind == TY_ARRAY) {
            // array lvalue: its address is the base (gen_addr handles a class
            // member via implicit this, a global via LEA, a param via LOD)
            gen_addr(e->a);
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
        type *st = infer_type(e->a);
        sym *stat = struct_static_sym(st, e->member);
        if (stat) { emit("LEA %s", stat->asm_name); return; }   // static via instance (object not addressed)
        gen_addr(e->a);
        strct_field *f = t_struct_find(st, e->member);
        if (!f) msg_error(e->line, "no field '%s'", e->member);
        if (f->offset > 0) emit("ADD %d", f->offset);
        return;
    }
    case E_PMEMBER: {
        type *st = infer_type(e->a);
        if (!st || !st->base) msg_error(e->line, "bad -> target");
        sym *stat = struct_static_sym(st->base, e->member);
        if (stat) { emit("LEA %s", stat->asm_name); return; }   // static via pointer (pointer not loaded)
        gen_expr(e->a);                           // ptr value
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

// A bitfield's masks, computed in 64 bits and emitted as the signed 32-bit
// immediate the AND always took. In `long` (32 bits on Windows) a field as wide
// as the word made `1L << 32`, undefined, which came out as mask 0: every
// `unsigned x : 32` read as 0 and every write stored 0.
static unsigned long long bf_ones(const strct_field *bf)
{
    return bf->bit_width >= 64 ? ~0ULL : (1ULL << bf->bit_width) - 1;
}
static long bf_word (unsigned long long v)    { return (long)(int)(unsigned)v; }
static long bf_mask (const strct_field *bf)   { return bf_word(bf_ones(bf)); }                    // value bits, at bit 0
static long bf_clear(const strct_field *bf)   { return bf_word(~(bf_ones(bf) << bf->bit_pos)); }  // all but the field's bits

// Wrap the int in acc to the exact-width type t (int8_t, uint16_t...): keep
// its low bits, sign-extended for a signed type -- (v & m) ^ s, minus s.
static void emit_wrap(const type *t)
{
    unsigned long long m = (1ULL << t->nbits) - 1;
    emit("AND %ld", (long)m);
    if (!t->nbits_uns) {
        long s = 1L << (t->nbits - 1);
        emit("XOR %ld", s);
        emit("ADD %ld", -s);
    }
}

// the value range of an integer type: [lo, hi] (a full word: the int range)
static void int_range(const type *t, long long *lo, long long *hi)
{
    if (t->is_bool)       { *lo = 0; *hi = 1; }
    else if (t->nbits)    { if (t->nbits_uns) { *lo = 0; *hi = (1LL << t->nbits) - 1; }
                            else { *lo = -(1LL << (t->nbits - 1)); *hi = (1LL << (t->nbits - 1)) - 1; } }
    else if (t->is_signed){ *lo = -2147483648LL; *hi = 2147483647LL; }
    else                  { *lo = 0; *hi = 4294967295LL; }
}

// Convert the value in acc from type `have` to type `want` (either NULL:
// nothing to do). The ULA's I2F/F2I are signed, so an unsigned word goes
// through the u2f/f2u helpers (emitted once, only when used). A conversion to
// bool is `!= 0`: an integer or pointer through LIN;LIN, a float by its
// mantissa (a YANC float is zero exactly when its mantissa is).
static void coerce_to(type *have, type *want)
{
    if (!have || !want || want->tparam || have->tparam) return;
    int hf = have->kind == TY_FLOAT, wf = want->kind == TY_FLOAT;
    if (want->is_bool) {
        if (have->is_bool) return;
        if (hf) emit("AND %ld", bf_word((1ULL << g_nbmant) - 1));
        if (hf || have->kind == TY_INT || have->kind == TY_PTR) { emit("LIN"); emit("LIN"); }
        return;
    }
    if (wf && !hf && have->kind == TY_INT) {
        if (!have->is_signed && !have->is_bool) { emit("CAL u2f"); g_uses_u2f = 1; }
        else emit("I2F");
    } else if (!wf && hf && want->kind == TY_INT) {
        if (!want->is_signed) { emit("CAL f2u"); g_uses_f2u = 1; }
        else emit("F2I");
    }
    // an exact-width int8_t/uint16_t/...: wrap, unless every value of `have`
    // already fits
    if (want->kind == TY_INT && want->nbits) {
        long long wl, wh, hl = 1, hh = 0;
        int_range(want, &wl, &wh);
        if (have->kind == TY_INT) int_range(have, &hl, &hh);
        if (!(have->kind == TY_INT && hl >= wl && hh <= wh)) emit_wrap(want);
    }
}

// the old two-way form, for operands: int -> float, or float -> int
static void coerce_acc(type *have, int want_float)
{
    coerce_to(have, want_float ? t_float() : t_int());
}

// evaluate e into acc, then coerce to float (want_float) or int.
static void gen_expr_num(expr *e, int want_float)
{
    gen_expr(e);
    coerce_acc(infer_type(e), want_float);
}

// e already evaluates to 0 or 1: a comparison, a logical operator, a bool
static int expr_is_01(expr *e)
{
    if (e->kind == E_BINOP)
        return e->op == OP_EQ || e->op == OP_NE || e->op == OP_LT || e->op == OP_GT ||
               e->op == OP_LE || e->op == OP_GE || e->op == OP_LAND || e->op == OP_LOR;
    if (e->kind == E_UNOP) return e->op == OP_LNOT;
    type *t = infer_type(e);
    return t && t->is_bool;
}

// evaluate e into acc as a value of type `want` (the destination of an
// assignment, initializer, argument or return)
static void gen_expr_to(expr *e, type *want)
{
    if (want && want->kind == TY_INT && want->nbits && e->kind == E_INT_LIT) {
        long long v = (unsigned long long)e->ival & ((1ULL << want->nbits) - 1);   // folded wrap
        if (!want->nbits_uns && v >= (1LL << (want->nbits - 1))) v -= 1LL << want->nbits;
        emit("LOD %ld", (long)v);
        return;
    }
    if (want && want->is_bool) {
        if (e->kind == E_INT_LIT)   { emit("LOD %d", e->ival != 0);  return; }  // folded
        if (e->kind == E_FLOAT_LIT) { emit("LOD %d", e->fval != 0);  return; }
        gen_expr(e);
        if (!expr_is_01(e)) coerce_to(infer_type(e), want);
        return;
    }
    gen_expr(e);
    coerce_to(infer_type(e), want);
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
        emit("LOD %s", st); if (i) emit("ADD %d", i); emit("LDI 0");
        emit("STI 0");
    }
    emit("LOD %s", dt);                      // result = &dest
    free(dt); free(st);
}

// copy n words from src (which evaluates to a base address) into the named
// block `dest` (declared via #array) at offsets 0..n-1.
static void copy_to_block(const char *dest, expr *src, int n)
{
    // a user copy constructor overrides the default word-by-word copy
    {
        type *st = infer_type(src);
        if (st && st->kind == TY_STRUCT && st->tag) {
            char *cc = resolve_method(st, "copyctor");
            if (cc) {
                emit("LEA %s", dest); emit("PSH");   // this = &dest
                gen_addr(src); emit("PSH");          // o = &src (reference param)
                emit("CAL %s", cc);
                free(cc);
                return;
            }
        }
    }
    char sn[32]; snprintf(sn, sizeof(sn), "_cbs%d", ++label_n);
    char *st = mangle_local(sn);
    const char *fn = cur_func_name ? cur_func_name : "global";
    st_add(SK_LOCAL_VAR, sn, st, t_int()); log_var(fn, sn, 1, 0);
    gen_expr(src); emit("SET %s", st);       // st = &src
    for (int i = 0; i < n; i++) {
        emit("LOD %d", i); emit("PSH");      // offset on stack
        emit("LOD %s", st); if (i) emit("ADD %d", i); emit("LDI 0"); // acc = src[i]
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
    if (ptype && ptype->is_ref) { gen_addr(arg); emit("PSH"); return; }
    type *at = infer_type(arg);
    if (ptype && (ptype->kind == TY_INT || ptype->kind == TY_FLOAT) &&
        at && at->kind != TY_STRUCT) {              // a scalar: convert it to the parameter's type
        gen_expr_to(arg, ptype);
        emit("PSH");
        return;
    }
    gen_push_arg(arg);
}

// ---- object construction ---------------------------------------------------

// a fresh one-word scratch variable of the current function (its asm name)
static char *new_temp(const char *prefix)
{
    char nm[32]; snprintf(nm, sizeof(nm), "_%s%d", prefix, ++label_n);
    char *an = mangle_local(nm);
    st_add(SK_LOCAL_VAR, nm, an, t_int());
    log_var(cur_func_name ? cur_func_name : "global", nm, 1, 0);
    return an;
}

// Where an object is, for constructing it: at a fixed symbol (`LEA name`) or at
// the address a variable holds (`LOD name`).
typedef struct { int in_var; const char *name; } objaddr;
static void emit_objaddr(objaddr a) { emit(a.in_var ? "LOD %s" : "LEA %s", a.name); }

// does creating an object of class t run code (a vptr to set, a ctor to call)?
static int needs_construct(type *t)
{
    return t && t->kind == TY_STRUCT && t->tag && (t->n_vtbl > 0 || resolve_ctor(t, NULL, 0));
}

// call constructor cf on the object at a, with args and then the defaults
static void emit_ctor_call(func *cf, objaddr a, expr **args, int nargs)
{
    emit_objaddr(a); emit("PSH");                             // this
    decl *p = cf->params ? cf->params->next : NULL;           // skip `this`
    for (int i = 0; i < nargs; i++) { gen_arg(args[i], p ? p->dtype : NULL); if (p) p = p->next; }
    for (; p; p = p->next) gen_arg(p->init, p->dtype);        // default args
    emit("CAL %s", cf->asm_label);
}

// Construct one object of class t at a: point a polymorphic object at its
// vtable, then run the constructor its arguments select (the default one
// when there are none), if the class has one.
static void emit_construct(type *t, objaddr a, expr **args, int nargs)
{
    if (!t || t->kind != TY_STRUCT || !t->tag) return;
    if (t->n_vtbl > 0) { emit_objaddr(a); emit("PSH"); emit("LEA %s__vtable", t->tag); emit("STI 0"); }
    func *cf = resolve_ctor(t, args, nargs);
    if (cf) emit_ctor_call(cf, a, args, nargs);
}

// Default-construct the n elements (class t) of the array at a, in a loop. The
// count is n, or the variable n_var when it is only known at run time.
static void emit_construct_n(type *t, objaddr a, int n, const char *n_var)
{
    if (!needs_construct(t) || (!n_var && n <= 0)) return;
    if (!n_var && n == 1) { emit_construct(t, a, NULL, 0); return; }
    int id = ++label_n;
    char *cp = new_temp("cnp"), *ck = new_temp("cnk");
    emit_objaddr(a); emit("SET %s", cp);                      // cp = &a[0]
    if (n_var) emit("LOD %s", n_var); else emit("LOD %d", n);
    emit("SET %s", ck);                                       // ck = n
    emit("@Lcn_t%d NOP", id);
    emit("LOD %s", ck); emit("JIZ Lcn_e%d", id);              // while ck != 0
    objaddr el = { 1, cp };
    emit_construct(t, el, NULL, 0);
    emit("LOD %s", cp); emit("ADD %d", type_size_words(t)); emit("SET %s", cp);   // next element
    emit("LOD %s", ck); emit("ADD -1"); emit("SET %s", ck);
    emit("JMP Lcn_t%d", id);
    emit("@Lcn_e%d NOP", id);
    free(cp); free(ck);
}

// an array type's innermost element type, and how many of them it holds
static type *array_elems(type *t, int *n)
{
    *n = 1;
    while (t && t->kind == TY_ARRAY) { *n *= t->arr_size; t = t->base; }
    return t;
}

// __construct_members' arguments after (this, base_named) name the member
// objects the member-init list constructs itself
static int listed_member(expr *call, const char *name)
{
    for (int i = 2; i < call->n_args; i++)
        if (call->args[i]->kind == E_IDENT && !strcmp(call->args[i]->sval, name)) return 1;
    return 0;
}

// construct a declared object or array of objects (a local at its declaration,
// a global or a static local at program start)
static void emit_construct_decl(type *t, const char *name, expr **args, int nargs)
{
    objaddr a = { 0, name };
    if (t && t->kind == TY_ARRAY) { int n; type *et = array_elems(t, &n); emit_construct_n(et, a, n, NULL); }
    else emit_construct(t, a, args, nargs);
}

// ++/-- on a bitfield: step the field, not the word that holds it (which also
// carried into the neighbouring fields) -- read the field, store it back
// through the masking bitfield store. Returns 0 when e's operand is not one.
static int gen_bitfield_step(expr *e)
{
    expr *lv = e->a;
    strct_field *bf = (lv->kind == E_MEMBER || lv->kind == E_PMEMBER) ? member_field(lv) : NULL;
    if (!bf || !bf->is_bitfield) return 0;
    int d = (e->kind == E_PREINC || e->kind == E_POSTINC) ? 1 : -1;
    expr *nv = ast_binop(OP_ADD, lv, ast_int_lit(d, e->line), e->line);
    if (e->kind == E_PREINC || e->kind == E_PREDEC) {
        gen_store(lv, nv); gen_expr(lv);                 // result: the new value
    } else {
        char *ov = new_temp("bfo");
        gen_expr(lv); emit("SET %s", ov);                // the old value
        gen_store(lv, nv);
        emit("LOD %s", ov);                              // result: the old value
        free(ov);
    }
    return 1;
}

// ---- store: lv = val -------------------------------------------------------

static void gen_store(expr *lv, expr *val)
{
    // reject assignment to a const variable (direct, by-name)
    if (lv->kind == E_IDENT) {
        sym *cs = st_find(lv->sval);
        if (cs && cs->is_const) msg_error(lv->line, "assignment to const '%s'", lv->sval);
    }
    // whole-struct assignment: a user operator= overrides the default memberwise
    // copy; otherwise copy all words (not a single STI 0)
    {
        type *lvt = infer_type(lv);
        if (lvt && lvt->kind == TY_STRUCT) {
            char *masm = lvt->tag ? resolve_method(lvt, "op_assign") : NULL;
            if (masm) {
                sym *ms = st_find(masm);
                gen_addr(lv); emit("PSH");                    // this = &lv
                type *pt = (ms && ms->param_types && ms->n_params > 1) ? ms->param_types[1] : NULL;
                gen_arg(val, pt);                             // rhs operand
                emit("CAL %s", masm);
                free(masm);
                return;
            }
            gen_struct_copy(lv, val, type_size_words(lvt));
            return;
        }
    }
    // bitfield store: read-modify-write the containing word
    {
        strct_field *bf = (lv->kind == E_MEMBER || lv->kind == E_PMEMBER) ? member_field(lv) : NULL;
        if (bf && bf->is_bitfield) {
            long mask  = bf_mask(bf);
            long clear = bf_clear(bf);
            char tn[64]; snprintf(tn, sizeof(tn), "_bf%d", ++label_n);
            char *ta = mangle_local(tn);
            st_add(SK_LOCAL_VAR, tn, ta, t_int());
            log_var(cur_func_name ? cur_func_name : "global", tn, 1, 0);
            gen_addr(lv);  emit("SET %s", ta);     // ta = &word
            emit("LOD %s", ta); emit("PSH");        // stack: [&word]  (STI 0 address)
            emit("LOD %s", ta); emit("LDI 0");        // acc = old word
            emit("AND %ld", clear);                 // clear the field's bits
            emit("PSH");                            // stack: [&word, cleared]
            gen_expr_to(val, bf->ftype);            // e.g. 2.7f -> 2, 5 -> 1 for a bool field
            emit("AND %ld", mask);                  // value & field-mask
            if (bf->bit_pos > 0) {                  // shift left by bit_pos (stack form)
                emit("PSH"); emit("LOD %d", bf->bit_pos); emit("S_SHL");
            }
            emit("S_ORR");                          // cleared | shifted
            emit("STI 0");                            // mem[&word] = result
            free(ta);
            return;
        }
    }
    // fast path: simple scalar IDENT
    if (lv->kind == E_IDENT) {
        sym *s = st_find(lv->sval);
        if (s && !s->is_frame && s->stype && !s->stype->is_ref &&
            s->stype->kind != TY_ARRAY && s->stype->kind != TY_STRUCT) {
            gen_expr_to(val, s->stype);
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
                // constant index -> SET_V base k (offset baked in), no index
                // compute / PSH / indirect STI.
                if (lv->b->kind == E_INT_LIT && lv->b->ival >= 0) {
                    gen_expr_to(val, bt->base);
                    emit("SET_V %s %ld", s->asm_name, lv->b->ival);
                } else {
                    gen_expr(lv->b);              // index → acc
                    emit("PSH");
                    gen_expr_to(val, bt->base);
                    emit("STI %s", s->asm_name);
                }
                return;
            }
        }
    }
    // general path via STI 0
    {
        type *lvt = infer_type(lv);
        gen_addr(lv);
        emit("PSH");
        gen_expr_to(val, lvt);
        emit("STI 0");
    }
}

// ---- main expression dispatcher -------------------------------------------

// e is a plain scalar memory variable that gen_expr would load with a single
// `LOD <asm_name>` (not a class member/static, reference, function, frame,
// enum constant, array or struct) -> its sym, usable with a _M-form opcode
// (NEG_M / ABS_M / I2F_M / ... read the operand straight from memory).
static sym *simple_mem_var(expr *e)
{
    if (!e || e->kind != E_IDENT) return NULL;
    if (cur_class_member(e->sval) || cur_class_static(e->sval)) return NULL;
    sym *s = st_find(e->sval);
    if (!s) return NULL;
    if (s->kind != SK_GLOBAL_VAR && s->kind != SK_LOCAL_VAR && s->kind != SK_PARAM)
        return NULL;
    if (s->is_frame) return NULL;
    if (!s->stype || s->stype->is_ref ||
        s->stype->kind == TY_ARRAY || s->stype->kind == TY_STRUCT) return NULL;
    return s;
}

// ---- inlining a small leaf accessor ---------------------------------------
// A call to a tiny function costs far more than the work it does. `a[i]`
// through an `operator[]` is four instructions at the call site plus six in
// the callee, where the same access on a native array is two. Measured on a
// loop of 3232 accesses: 78991 cycles through the call against 46703 with the
// access written out -- ten cycles each, 1.69x on the whole program.
//
// Only the shape that is safe to paste is taken: a non-virtual, non-recursive
// function whose body is exactly one `return <expr>;`, whose parameters are
// all scalars, and whose expression calls nothing itself. The call then
// becomes: evaluate each argument into the callee's own parameter word (it has
// fixed storage, like every non-recursive local), then emit the return
// expression in the callee's scope. No stack traffic, no CAL, no RET. The
// out-of-line copy is still emitted, because the dispatch chain and any call
// this does not inline still reach it.

static int expr_calls_anything(expr *e)
{
    if (!e) return 0;
    if (e->kind == E_CALL || e->kind == E_NEW || e->kind == E_DELETE) return 1;
    if (expr_calls_anything(e->a) || expr_calls_anything(e->b) ||
        expr_calls_anything(e->c)) return 1;
    for (int i = 0; i < e->n_args; i++)
        if (expr_calls_anything(e->args[i])) return 1;
    return 0;
}

// the one `return <expr>;` that is the whole body, or NULL
static expr *inlinable_body(func *f)
{
    if (!f || f->is_recursive || f->n_tparams > 0 || !f->body) return NULL;
    if (!f->ret || f->ret->kind == TY_VOID || f->ret->kind == TY_STRUCT) return NULL;
    if (f->body->kind != S_BLOCK || f->body->n_items != 1) return NULL;
    stmt *r = f->body->items[0];
    if (!r || r->kind != S_RETURN || !r->e1) return NULL;
    if (expr_calls_anything(r->e1)) return NULL;
    for (decl *p = f->params; p; p = p->next) {
        if (!p->dtype) return NULL;
        if (p->dtype->kind == TY_ARRAY || p->dtype->kind == TY_STRUCT) return NULL;
    }
    return r->e1;
}

static func *func_by_label(const char *lbl)
{
    if (!cg_unit || !lbl) return NULL;
    for (int i = 0; i < cg_unit->n_funcs; i++) {
        func *f = cg_unit->funcs[i];
        if (f->asm_label && !strcmp(f->asm_label, lbl) && f->body) return f;
    }
    // a class template's methods (and function-template instances) are
    // clones kept in g_inst, not in the unit's function list: without this,
    // `std::array`-style code never had its operator[] expanded
    for (int i = 0; i < g_n_inst; i++) {
        func *f = g_inst[i];
        if (f->asm_label && !strcmp(f->asm_label, lbl) && f->body) return f;
    }
    return NULL;
}

// does evaluating e write to anything? (assignment, compound assignment --
// the parser lowers `a += b` to `=` -- or an increment / decrement)
static int expr_writes_anything(expr *e)
{
    if (!e) return 0;
    if (e->kind == E_ASSIGN || e->kind == E_PREINC || e->kind == E_PREDEC ||
        e->kind == E_POSTINC || e->kind == E_POSTDEC) return 1;
    if (expr_writes_anything(e->a) || expr_writes_anything(e->b) ||
        expr_writes_anything(e->c)) return 1;
    for (int i = 0; i < e->n_args; i++)
        if (expr_writes_anything(e->args[i])) return 1;
    return 0;
}

// Two scalar types a parameter can alias without a conversion in between:
// the same int kind at the same width and signedness, or both float. Anything
// else (a narrowing to int8_t, an int passed to a float, a reference, a
// pointer) goes through the ordinary copy, which converts.
static int same_scalar(type *a, type *b)
{
    if (!a || !b || a->is_ref || b->is_ref || a->kind != b->kind) return 0;
    if (a->kind == TY_FLOAT) return 1;
    if (a->kind != TY_INT) return 0;
    return a->is_signed == b->is_signed && a->is_bool == b->is_bool &&
           a->nbits == b->nbits && a->nbits_uns == b->nbits_uns;
}

// Paste f's body here. `this_addr` is the object's address for a method (NULL
// for a free function); args are the declared arguments, in order. Returns 1
// when it emitted the call's value into the accumulator.
static int inline_call(func *f, expr *body, expr *this_addr, expr **args, int n_args)
{
    decl *plist[16]; int np = 0;
    for (decl *p = f->params; p && np < 16; p = p->next) plist[np++] = p;
    int first = this_addr ? 1 : 0;                  // param 0 is `this` on a method
    if (np - first != n_args || np > 16) return 0;  // default args: leave it alone

    // Copy propagation: an argument that is already a plain scalar variable of
    // exactly the parameter's type is not copied at all -- the parameter is
    // bound to that variable's own word, so `a[i]` reads `main_i` directly
    // instead of storing it into `op_index_i` and loading it straight back.
    // Only when the body writes nothing, so neither the caller's variable nor
    // the parameter can change under it. Decided here, in OUR scope, where the
    // argument's name means what the caller meant.
    char *bound[16] = {0};
    if (!expr_writes_anything(body)) {
        for (int i = first; i < np; i++) {
            expr *src = args[i - first];
            sym *vs = simple_mem_var(src);
            if (vs && vs->asm_name && same_scalar(infer_type(src), plist[i]->dtype))
                bound[i] = strdup(vs->asm_name);
        }
    }

    // the rest go into the callee's own words. `this` is computed LAST, so its
    // address is still in the accumulator when the body opens with `LOD this`
    // -- the peephole then drops that reload of the word just stored.
    for (int i = first; i < np; i++) {
        if (bound[i]) continue;
        expr *src = args[i - first];
        if (plist[i]->dtype && plist[i]->dtype->is_ref) gen_addr(src);
        else gen_expr_to(src, plist[i]->dtype);
        emit("SET %s_%s", f->asm_label, plist[i]->name);
    }
    if (first) {
        gen_addr(this_addr);
        emit("SET %s_%s", f->asm_label, plist[0]->name);
    }

    // then the body, in the CALLEE's scope, so its parameter names resolve to
    // the words we just filled
    char *sv_fn        = cur_func_name;
    type *sv_ret       = cur_func_ret;
    int   sv_rec       = cur_fn_recursive;
    type *sv_cls       = cur_method_class;
    const char *sv_stf = st_current_func();
    char *keep = sv_stf ? strdup(sv_stf) : NULL;

    cur_func_name    = f->asm_label;
    cur_func_ret     = f->ret;
    cur_fn_recursive = 0;
    cur_method_class = f->method_of;
    st_enter_func(f->asm_label);
    st_push_scope();
    for (int i = 0; i < np; i++) {
        char nm[256];
        if (bound[i]) snprintf(nm, sizeof(nm), "%s", bound[i]);
        else          snprintf(nm, sizeof(nm), "%s_%s", f->asm_label, plist[i]->name);
        st_add(SK_PARAM, plist[i]->name, nm, plist[i]->dtype);
    }
    for (int i = 0; i < np; i++) free(bound[i]);
    if (f->ret->is_ref) gen_addr(body); else gen_expr_to(body, f->ret);
    st_pop_scope();

    cur_func_name    = sv_fn;
    cur_func_ret     = sv_ret;
    cur_fn_recursive = sv_rec;
    cur_method_class = sv_cls;
    if (keep) { st_enter_func(keep); free(keep); } else st_leave_func();
    return 1;
}

static void gen_expr(expr *e)
{
    if (!e) return;
    infer_type(e);
    cfold_node(e);                        // literal-only int expression -> a literal

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
            // array/struct fields decay to their base address (the field IS
            // the storage, not a pointer to it). LDI 0 only for scalar fields.
            if (mf->ftype && (mf->ftype->kind == TY_ARRAY || mf->ftype->kind == TY_STRUCT))
                return;
            emit("LDI 0");
            return;
        }
        sym *stat = cur_class_static(e->sval);
        sym *s = stat ? stat : st_find(e->sval);
        if (!s) msg_error(e->line, "undefined '%s'", e->sval);
        if (s->stype && s->stype->is_ref) { gen_addr(e); emit("LDI 0"); return; }  // auto-deref
        if (s->kind == SK_FUNC) {
            // function used as a value -> its dispatch-table ID (function ptr)
            int id = fp_id_of(e->sval);
            if (id < 0) msg_internal("function '%s' not in dispatch table", e->sval);
            emit("LOD %d", id);
            return;
        }
        // frame scalar/pointer local: load mem[__fp + off] (arrays/structs in a
        // recursive function are rejected at declaration time)
        if (s->is_frame) { gen_addr(e); emit("LDI 0"); return; }
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
        emit("LDI 0");
        return;

    case E_INDEX: {
        // overloaded subscript: obj[i] on a class with operator[] -> obj.op_index(i)
        {
            type *bt0 = infer_type(e->a);
            if (bt0 && bt0->kind == TY_STRUCT && bt0->tag) {
                char *masm = resolve_method(bt0, "op_index");
                if (masm) {
                    sym *ms = st_find(masm);
                    type *rt = (ms && ms->kind == SK_FUNC) ? ms->stype : NULL;
                    {   // a tiny operator[] is pasted here instead of called:
                        // this is the hot one, an element access inside a loop
                        func *cf = func_by_label(masm);
                        expr *bd = cf ? inlinable_body(cf) : NULL;
                        if (bd && inline_call(cf, bd, e->a, &e->b, 1)) {
                            free(masm);
                            if (rt && rt->is_ref && rt->base &&
                                rt->base->kind != TY_STRUCT && rt->base->kind != TY_ARRAY)
                                emit("LDI 0");
                            return;
                        }
                    }
                    gen_addr(e->a); emit("PSH");              // this = &obj
                    type *pt = (ms && ms->param_types && ms->n_params > 1) ? ms->param_types[1] : NULL;
                    gen_arg(e->b, pt);
                    emit("CAL %s", masm);
                    free(masm);
                    // reference return to a scalar: the call yields an address;
                    // load the referent to get the rvalue.
                    if (rt && rt->is_ref && rt->base &&
                        rt->base->kind != TY_STRUCT && rt->base->kind != TY_ARRAY)
                        emit("LDI 0");
                    return;
                }
            }
        }
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
                // constant index -> LOD_V base k (offset baked in) instead of
                // computing the index and an indirect LDI.
                if (e->b->kind == E_INT_LIT && e->b->ival >= 0)
                    emit("LOD_V %s %ld", s->asm_name, e->b->ival);
                else {
                    gen_expr(e->b);
                    emit("LDI %s", s->asm_name);
                }
                return;
            }
        }
        gen_addr(e);
        emit("LDI 0");
        return;
    }

    case E_MEMBER:
    case E_PMEMBER: {
        // bitfield read: load the word, shift the field down, mask it off
        strct_field *bf = member_field(e);
        if (bf && bf->is_bitfield) {
            gen_addr(e);                    // &word (gen_addr adds the word offset)
            emit("LDI 0");                    // word value
            if (bf->bit_pos > 0) {          // logical shift right by bit_pos (stack form)
                emit("PSH"); emit("LOD %d", bf->bit_pos); emit("S_SHR");
            }
            emit("AND %ld", bf_mask(bf));
            if (bf->ftype && bf->ftype->kind == TY_INT && bf->ftype->is_signed &&
                bf->bit_width < g_nubits) {         // signed field: sign-extend, (v ^ s) - s
                long s = bf_word(1ULL << (bf->bit_width - 1));
                emit("XOR %ld", s);
                emit("ADD %ld", -s);
            }
            return;
        }
        // a field of array/struct type decays to its address (no load)
        if (e->etype && (e->etype->kind == TY_ARRAY || e->etype->kind == TY_STRUCT)) {
            gen_addr(e);
            return;
        }
        gen_addr(e);
        emit("LDI 0");
        return;
    }

    case E_ASSIGN:
        // compound `a OP= b` on a class that overloads operatorOP= -> a.op_OPeq(b).
        // (e->b is the desugared `a OP b`; its rhs operand is e->b->b.)
        if (e->op != OP_NONE) {
            type *lt = infer_type(e->a);
            const char *opm = op_compound_method_name(e->op);
            char *masm = (lt && lt->kind == TY_STRUCT && lt->tag && opm)
                       ? resolve_method(lt, opm) : NULL;
            if (masm) {
                sym *ms = st_find(masm);
                gen_addr(e->a); emit("PSH");                  // this = &lhs
                type *pt = (ms && ms->param_types && ms->n_params > 1) ? ms->param_types[1] : NULL;
                gen_arg(e->b->b, pt);                         // the rhs operand
                emit("CAL %s", masm);
                free(masm);
                return;
            }
        }
        gen_store(e->a, e->b);
        // assignment expression yields the stored value; for v1 we leave acc
        // as it is after the store (which for SET-path is the value, for
        // STI 0-path is also the value)
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
        // this form, which is reversed — they take the stack path instead.
        // Gate on the op: otherwise the lhs is evaluated here (a LOD) and then
        // re-evaluated by the stack path, leaving a dead LOD of the left operand.
        int mem_form_op = (op == OP_ADD || op == OP_MUL || op == OP_BAND ||
                           op == OP_BOR || op == OP_BXOR || op == OP_LT  ||
                           op == OP_GT  || op == OP_LE   || op == OP_GE  ||
                           op == OP_EQ  || op == OP_NE);
        // The operand is a plain scalar variable, or an int literal against an
        // int left side (the assembler places a numeric operand in data
        // memory). With a literal c, <= and >= are one instruction too:
        // a <= c is a < c+1, a >= c is a > c-1 (unsigned compares took the
        // sign-flip path above, so this is signed), guarded at the ends.
        const char *opnd = NULL; char litb[32]; long lit = 0; int is_lit = 0;
        cfold_node(e->b);                     // FL - 1 -> 28: then the literal memory form
        if (mem_form_op && e->b->kind == E_IDENT && lf == rf) {   // mem-form needs matching types
            sym *r = st_find(e->b->sval);
            // not a reference: its word holds the referent's address, and the
            // memory form would operate on that address (z + r gave z + &x)
            if (r && !r->is_frame && r->stype && !r->stype->is_ref &&
                r->stype->kind != TY_ARRAY && r->stype->kind != TY_STRUCT) opnd = r->asm_name;
        } else if (mem_form_op && e->b->kind == E_INT_LIT && !is_float && lt && lt->kind == TY_INT) {
            lit = e->b->ival; is_lit = 1;
            snprintf(litb, sizeof litb, "%ld", lit); opnd = litb;
        }
        if (opnd) {
            long lmax = (1L << (g_nubits - 1)) - 1, lmin = -lmax - 1;
            gen_expr(e->a);
            switch (op) {
                case OP_ADD: emit(is_float ? "F_ADD %s" : "ADD %s", opnd); return;
                case OP_MUL: emit(is_float ? "F_MLT %s" : "MLT %s", opnd); return;
                case OP_BAND: emit("AND %s", opnd); return;
                case OP_BOR:  emit("ORR %s", opnd); return;
                case OP_BXOR: emit("XOR %s", opnd); return;
                case OP_LT:   emit(is_float ? "F_GRE %s" : "GRE %s", opnd); return; // mem>acc == lhs<rhs
                case OP_GT:   emit(is_float ? "F_LES %s" : "LES %s", opnd); return; // mem<acc == lhs>rhs
                case OP_LE:
                    if (is_lit && lit < lmax) { emit("GRE %ld", lit + 1); return; }   // a < c+1
                    emit(is_float ? "F_LES %s" : "LES %s", opnd); emit("LIN"); return;
                case OP_GE:
                    if (is_lit && lit > lmin) { emit("LES %ld", lit - 1); return; }   // a > c-1
                    emit(is_float ? "F_GRE %s" : "GRE %s", opnd); emit("LIN"); return;
                case OP_EQ:   emit("EQU %s", opnd); return;
                case OP_NE:   emit("EQU %s", opnd); emit("LIN"); return;
                default: break;   // SUB/DIV/MOD/SHL/SHR -> stack path (correct order)
            }
        }
        // rhs is arr[const] on a fixed-address scalar array → the _V form
        // (base+offset baked in): ADD_V / MLT_V (+ float). Only ADD/MUL have a
        // _V opcode; for other ops arr[const] falls through to the stack path.
        if ((op == OP_ADD || op == OP_MUL) && lf == rf &&
            e->b->kind == E_INDEX && e->b->a->kind == E_IDENT &&
            e->b->b->kind == E_INT_LIT && e->b->b->ival >= 0) {
            sym *r = st_find(e->b->a->sval);
            if (r && r->kind != SK_PARAM && r->stype && r->stype->kind == TY_ARRAY
                && type_size_words(r->stype->base) == 1) {
                gen_expr(e->a);
                long k = e->b->b->ival;
                if (op == OP_ADD) emit(is_float ? "F_ADD_V %s %ld" : "ADD_V %s %ld", r->asm_name, k);
                else              emit(is_float ? "F_MLT_V %s %ld" : "MLT_V %s %ld", r->asm_name, k);
                return;
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

    case E_UNOP: {
        // unary operator overload: -a / +a on a class with operator-()/operator+()
        type *ot = infer_type(e->a);
        if (ot && ot->kind == TY_STRUCT && ot->tag) {
            const char *opm = op_unary_method_name(e->op);
            char *masm = opm ? resolve_method(ot, opm) : NULL;
            if (masm) {
                gen_addr(e->a); emit("PSH");              // this = &operand
                emit("CAL %s", masm);
                free(masm);
                return;
            }
        }
        // operand is a plain memory variable -> read it straight from memory
        // with the _M unary form instead of LOD x; <op>.
        {
            sym *mv = simple_mem_var(e->a);
            if (mv) switch (e->op) {
                case OP_NEG: emit(e->etype && e->etype->kind == TY_FLOAT ? "F_NEG_M %s" : "NEG_M %s", mv->asm_name); return;
                case OP_BNOT: emit("INV_M %s", mv->asm_name); return;
                case OP_LNOT: emit("LIN_M %s", mv->asm_name); return;
                default: break;
            }
        }
        gen_expr(e->a);
        switch (e->op) {
            case OP_POS: return;
            case OP_NEG: emit(e->etype && e->etype->kind == TY_FLOAT ? "F_NEG" : "NEG"); return;
            case OP_BNOT: emit("INV"); return;
            case OP_LNOT: emit("LIN"); return;
            default: msg_internal("unhandled unop");
        }
        return;
    }

    case E_PREINC: case E_PREDEC: {
        // ++lv / --lv : modify in place, result is the NEW value.
        if (gen_bitfield_step(e)) return;
        expr *lv = e->a;
        int is_float = lv->etype && lv->etype->kind == TY_FLOAT;
        int narrow = lv->etype && lv->etype->kind == TY_INT && lv->etype->nbits;   // wraps
        // step: 1 for scalars; sizeof(pointee) for pointers (pointer arithmetic)
        int step = (lv->etype && lv->etype->kind == TY_PTR) ? type_size_words(lv->etype->base) : 1;
        int delta = (e->kind == E_PREDEC) ? -step : step;
        // fast path: simple scalar identifier
        if (lv->kind == E_IDENT) {
            sym *s = st_find(lv->sval);
            if (s && !s->is_frame && s->stype && !s->stype->is_ref &&   // a reference steps its referent
                s->stype->kind != TY_ARRAY && s->stype->kind != TY_STRUCT) {
                emit("LOD %s", s->asm_name);
                if (is_float) emit("F_ADD %s", delta < 0 ? "-1.0" : "1.0");
                else          emit("ADD %d", delta);
                if (narrow) emit_wrap(lv->etype);
                emit("SET %s", s->asm_name);
                return;
            }
        }
        // general lvalue: &lv computed once, read-modify-write via LDI 0 / STI 0
        gen_addr(lv);                                    // acc = &lv
        emit("PSH");                                     // stack: [&lv]
        emit("LDI 0");                                     // acc = *lv
        if (is_float) emit("F_ADD %s", delta < 0 ? "-1.0" : "1.0");
        else          emit("ADD %d", delta);             // acc = new
        if (narrow) emit_wrap(lv->etype);
        emit("STI 0");                                     // mem[&lv] = new ; acc = new
        return;
    }
    case E_POSTINC: case E_POSTDEC: {
        // lv++ / lv-- : modify in place, result is the OLD value.
        if (gen_bitfield_step(e)) return;
        expr *lv = e->a;
        int is_float = lv->etype && lv->etype->kind == TY_FLOAT;
        int narrow = lv->etype && lv->etype->kind == TY_INT && lv->etype->nbits;   // wraps
        int step = (lv->etype && lv->etype->kind == TY_PTR) ? type_size_words(lv->etype->base) : 1;
        int delta = (e->kind == E_POSTDEC) ? -step : step;
        // fast path: simple scalar identifier
        if (lv->kind == E_IDENT) {
            sym *s = st_find(lv->sval);
            if (s && !s->is_frame && s->stype && !s->stype->is_ref &&   // a reference steps its referent
                s->stype->kind != TY_ARRAY && s->stype->kind != TY_STRUCT) {
                emit("LOD %s", s->asm_name);
                emit("PSH");                             // stack: [old]  (result)
                if (is_float) emit("F_ADD %s", delta < 0 ? "-1.0" : "1.0");
                else          emit("ADD %d", delta);
                if (narrow) emit_wrap(lv->etype);
                emit("SET %s", s->asm_name);
                emit("POP");                             // acc = old
                return;
            }
        }
        if (!is_float && !narrow) {
            // integer: recover old by undoing the delta (exact in integer math)
            gen_addr(lv);                                // acc = &lv
            emit("PSH");                                 // stack: [&lv]
            emit("LDI 0");                                 // acc = old
            emit("ADD %d", delta);                       // acc = new
            emit("STI 0");                                 // mem[&lv] = new ; acc = new
            emit("ADD %d", -delta);                      // acc = new - delta = old
            return;
        }
        // float (re-adding would round) or a wrapping type (the old value cannot
        // be recovered from the wrapped one): keep old in a temp instead
        {
            char tn[64]; snprintf(tn, sizeof(tn), "_pf%d", ++label_n);
            char *ta = mangle_local(tn);
            st_add(SK_LOCAL_VAR, tn, ta, t_int());
            log_var(cur_func_name ? cur_func_name : "global", tn, 1, 0);
            gen_addr(lv);                                // acc = &lv
            emit("SET %s", ta);                          // _pf = &lv
            emit("LDI 0");                                 // acc = old (mem[&lv])
            emit("PSH");                                 // stack: [old]  (result)
            emit("LOD %s", ta); emit("PSH");             // stack: [old, &lv]
            emit("LOD %s", ta); emit("LDI 0");             // acc = old (re-read)
            if (is_float) emit("F_ADD %s", delta < 0 ? "-1.0" : "1.0");   // acc = new
            else        { emit("ADD %d", delta); emit_wrap(lv->etype); }
            emit("STI 0");                                 // mem[&lv] = new ; stack: [old]
            emit("POP");                                 // acc = old
            free(ta);
            return;
        }
    }

    case E_CAST: {
        type *from = infer_type(e->a);
        type *to   = e->target_t;
        // signed int<->float cast of a plain memory variable -> the _M form
        int i2f = from && to && type_is_int(from) && from->is_signed && type_is_float(to);
        int f2i = from && to && type_is_float(from) && type_is_int(to) && to->is_signed;
        sym *mv = (i2f || f2i) ? simple_mem_var(e->a) : NULL;
        if (mv) { emit(i2f ? "I2F_M %s" : "F2I_M %s", mv->asm_name); return; }
        gen_expr_to(e->a, to);   // int<->float, unsigned via helpers, -> bool as != 0;
        return;                  // any other cast reinterprets the bits: no-op
    }

    case E_NEW: {
        type *t = e->target_t;
        int words = type_size_words(t);
        if (e->a && needs_construct(t)) {             // new T[n] of objects: construct each
            char *nv = new_temp("nwn"), *pv = new_temp("nwp");
            gen_expr(e->a); emit("SET %s", nv);
            if (words != 1) { emit("PSH"); emit("LOD %d", words); emit("S_MLT"); }
            emit("PSH"); emit("CAL malloc"); emit("SET %s", pv);
            objaddr a = { 1, pv };
            emit_construct_n(t, a, 0, nv);
            emit("LOD %s", pv);                                   // result: the array
            free(nv); free(pv);
            return;
        }
        if (e->a) {                                   // new T[n] -> malloc(n*words)
            gen_expr(e->a);
            if (words != 1) { emit("PSH"); emit("LOD %d", words); emit("S_MLT"); }
            emit("PSH"); emit("CAL malloc");
            return;
        }
        emit("LOD %d", words); emit("PSH"); emit("CAL malloc");   // acc = ptr
        int poly = (t->kind == TY_STRUCT && t->n_vtbl > 0);
        func *cf = (t->kind == TY_STRUCT && t->tag) ? resolve_ctor(t, e->args, e->n_args) : NULL;
        int has_ctor = cf != NULL;
        if (!poly && !has_ctor) return;                            // plain alloc, ptr in acc
        emit("SET __new_p");                                       // stash the new object
        if (poly) {                                                // mem[obj+0] = &vtable
            emit("LOD __new_p"); emit("PSH"); emit("LEA %s__vtable", t->tag); emit("STI 0");
        }
        if (has_ctor) {
            emit("LOD __new_p"); emit("PSH");                      // result (kept on stack bottom)
            emit("LOD __new_p"); emit("PSH");                      // this (arg 0)
            decl *p = cf->params ? cf->params->next : NULL;        // skip `this`
            for (int i = 0; i < e->n_args; i++) {
                gen_arg(e->args[i], p ? p->dtype : NULL);
                if (p) p = p->next;
            }
            for (; p; p = p->next) gen_arg(p->init, p->dtype);     // default args
            emit("CAL %s", cf->asm_label);
            emit("POP");                                           // acc = result
        } else {
            emit("LOD __new_p");                                   // acc = result
        }
        return;
    }
    case E_TEMPOBJ: {
        // T(args): build a hidden stack temporary, run its ctor, yield its address.
        type *t = e->target_t;
        if (!t || t->kind != TY_STRUCT || !t->tag) { msg_error(e->line, "T(args) on non-class type"); return; }
        char nm[32]; snprintf(nm, sizeof(nm), "_tmp%d", ++label_n);
        char *aname = mangle_local(nm);
        st_add(SK_LOCAL_VAR, nm, aname, t);
        log_var(cur_func_name ? cur_func_name : "global", nm, innermost_code(t), 0);
        emit("#array %s %d %d", aname, agg_fill_code(t), type_size_words(t));
        objaddr a = { 0, aname };
        emit_construct(t, a, e->args, e->n_args);
        emit("LEA %s", aname);                                     // result = &temporary
        free(aname);
        return;
    }
    case E_DELETE: {
        type *pt = infer_type(e->a);
        type *t = pt ? pt->base : NULL;
        int   dslot = -1;
        char *dtor  = NULL;
        if (!e->ival && t && t->kind == TY_STRUCT && t->tag) {
            dslot = vtbl_slot(t, "dtor");
            if (dslot < 0) {
                dtor = resolve_method(t, "dtor");
                sym *ds = dtor ? st_find(dtor) : NULL;
                if (!(ds && ds->kind == SK_FUNC)) { free(dtor); dtor = NULL; }
            }
        }
        // Evaluate the operand ONCE. A virtual dtor reached here may recurse
        // indirectly (e.g. ~P does `delete this->child`) and clobber a fixed-
        // address `this`; re-evaluating `e->a` for the free() would then read a
        // stale member and free the wrong object (a double-free). So keep one
        // copy on the data stack for free() across the dtor call.
        gen_expr(e->a);
        if (dslot >= 0 || dtor) {
            emit("PSH");                            // copy kept for free()
            emit("PSH");                            // copy consumed as the dtor's `this`
            if (dslot >= 0) {                       // virtual dtor: dispatch via the vptr
                emit("LDI 0"); if (dslot) emit("ADD %d", dslot); emit("LDI 0");
                emit("SET _fp_id");
                emit_dispatch_chain();
            } else {
                emit("CAL %s", dtor); free(dtor);
            }
            emit("CAL free");                       // consumes the kept copy
        } else {
            emit("PSH"); emit("CAL free");
        }
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
        else                     emit("#array %s %d %d", an, agg_fill_code(t), type_size_words(t));
        emit_initz(an, 0, t, e->cinit);
        emit("LEA %s", an);
        if (t->kind != TY_ARRAY && t->kind != TY_STRUCT) emit("LDI 0");  // scalar value
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
            int vslot = vtbl_slot(ct, e->a->member);
            char *masm = resolve_method(ct, e->a->member);   // may be NULL for a pure virtual
            if (!masm && vslot < 0) msg_error(e->line, "no method '%s' in class '%s'", e->a->member, ct->tag);
            sym *ms = masm ? st_find(masm) : NULL;
            if (vslot >= 0) {
                // virtual dispatch: fid = mem[mem[this] + slot], then indirect-call
                if (e->a->kind == E_MEMBER) gen_addr(obj); else gen_expr(obj);  // acc = this
                emit("PSH");                                  // this -> arg 0 (kept on stack)
                emit("LDI 0");                                  // acc = vptr = mem[this]
                if (vslot) emit("ADD %d", vslot);
                emit("LDI 0");                                  // acc = fid = mem[vptr+slot]
                emit("SET _fp_id");
                for (int i = 0; i < e->n_args; i++) {
                    type *pt = (ms && ms->param_types && i + 1 < ms->n_params) ? ms->param_types[i + 1] : NULL;
                    gen_arg(e->args[i], pt);
                }
                emit_dispatch_chain();
                free(masm);
                return;
            }
            {   // a tiny accessor is pasted here instead of called
                func *cf = func_by_label(masm);
                expr *bd = cf ? inlinable_body(cf) : NULL;
                if (bd && e->a->kind == E_MEMBER &&
                    inline_call(cf, bd, obj, e->args, e->n_args)) { free(masm); return; }
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
                // the OUT port is an integer word: the hardware has no float
                // output, so a float value is shipped as its raw bit pattern --
                // a wrong number. Warn (don't block; no implicit convert): the
                // caller should cast, e.g. out(port, (int)x).
                { type *vt = infer_type(e->args[1]);
                  if (vt && vt->kind == TY_FLOAT)
                      msg_warning(e->line, "out() sends an integer word; this float is output as its raw bit pattern (wrong value) -- cast to int, e.g. out(port, (int)x)"); }
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
            if (!strcmp(fn, "__construct_members")) {
                // synth, first thing in every constructor: construct the base
                // (unless the member-init list does: args[1] = 1) and the member
                // objects, in declaration order. args[0] = this. The base's ctor
                // is called directly, so the vptr stays the derived class's.
                type *cls = cur_method_class;
                if (!cls || e->n_args < 2) return;
                func *bc = (!e->args[1]->ival && cls->base_class) ? resolve_ctor(cls->base_class, NULL, 0) : NULL;
                int any = bc != NULL;
                #define LISTED(f) listed_member(e, (f)->name)
                for (strct_field *f = cls->fields; f && !any; f = f->next) {
                    int n; type *et = array_elems(f->ftype, &n);
                    if (!f->inherited && !f->is_bitfield && !LISTED(f) && needs_construct(et)) any = 1;
                }
                if (!any) return;
                char *tv = new_temp("cmt");
                gen_expr(e->args[0]); emit("SET %s", tv);            // tv = this
                objaddr self = { 1, tv };
                if (bc) emit_ctor_call(bc, self, NULL, 0);
                for (strct_field *f = cls->fields; f; f = f->next) {
                    int n; type *et = array_elems(f->ftype, &n);
                    if (f->inherited || f->is_bitfield || LISTED(f) || !needs_construct(et)) continue;
                    char *fa = new_temp("cmf");
                    emit("LOD %s", tv); if (f->offset) emit("ADD %d", f->offset); emit("SET %s", fa);
                    objaddr m = { 1, fa };
                    emit_construct_n(et, m, n, NULL);
                    free(fa);
                }
                #undef LISTED
                free(tv);
                return;
            }
            if (!strcmp(fn, "__construct_at")) {
                // synth, from a member-init list: construct member args[0]
                // with the arguments that follow. With no constructor to take
                // them, `member(x)` is the copy `member = x` it always was.
                type *mt = infer_type(e->args[0]);
                if (e->n_args == 2 && !resolve_ctor(mt, e->args + 1, 1)) {
                    gen_store(e->args[0], e->args[1]);
                    return;
                }
                char *ma = new_temp("cat");
                gen_addr(e->args[0]); emit("SET %s", ma);
                objaddr m = { 1, ma };
                emit_construct(mt, m, e->args + 1, e->n_args - 1);
                free(ma);
                return;
            }
            if (!strcmp(fn, "__zerofill")) {   // synth: zero-fill N words at &field
                // emitted for an aggregate `= {}` default member initializer so a
                // heap object's array/struct field is zeroed (static storage is
                // already .mif-zeroed). args: [0]=field ident (-> &field), [1]=N.
                if (e->n_args != 2) msg_error(e->line, "__zerofill takes 2 args");
                long nw = e->args[1]->ival;
                if (nw <= 0) return;
                gen_addr(e->args[0]);                      // acc = &field (this + offset)
                int id = ++label_n;
                char pn[32], kn[32];
                snprintf(pn, sizeof(pn), "_zfp%d", id);
                snprintf(kn, sizeof(kn), "_zfk%d", id);
                char *zp = mangle_local(pn), *zk = mangle_local(kn);
                st_add(SK_LOCAL_VAR, pn, zp, t_int());
                st_add(SK_LOCAL_VAR, kn, zk, t_int());
                log_var(cur_func_name ? cur_func_name : "global", pn, 1, 0);
                log_var(cur_func_name ? cur_func_name : "global", kn, 1, 0);
                emit("SET %s", zp);                        // zp = &field
                emit("LOD %ld", nw); emit("SET %s", zk);   // zk = N
                emit("@Lzf_t%d NOP", id);
                emit("LOD %s", zk); emit("JIZ Lzf_e%d", id);                 // while zk != 0
                emit("LOD %s", zp); emit("PSH"); emit("LOD 0"); emit("STI 0"); // *zp = 0
                emit("LOD %s", zp); emit("P_LOD 1"); emit("S_ADD"); emit("SET %s", zp);          // zp++
                emit("LOD %s", zk); emit("P_LOD 1"); emit("NEG"); emit("S_ADD"); emit("SET %s", zk); // zk--
                emit("JMP Lzf_t%d", id);
                emit("@Lzf_e%d NOP", id);
                free(zp); free(zk);
                return;
            }
            func *tmpl = find_template(fn);            // function template -> monomorphize
            if (tmpl && tmpl->n_params == e->n_args) {
                func *inst = get_instance(tmpl, e->args, e->n_args, e->gtypes, e->n_gtypes);
                decl *p = inst->params;
                for (int i = 0; i < e->n_args; i++) {
                    gen_arg(e->args[i], p ? p->dtype : NULL);
                    if (p) p = p->next;
                }
                emit("CAL %s", inst->asm_label);
                return;
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
            // parser-emitted call to a mangled method name with an explicit
            // leading `this` (e.g. a base-ctor init `Base__ctor(this, args)`).
            // Resolve overloads by the args after `this` so an overloaded base
            // constructor picks the right label rather than the bare name.
            if (cg_unit) {
                func *bestm = NULL; int bests = -1;
                for (int i = 0; i < cg_unit->n_funcs; i++) {
                    func *f = cg_unit->funcs[i];
                    if (!f->method_of || strcmp(f->name, fn)) continue;
                    decl *p0 = f->params ? f->params->next : NULL;   // skip `this`
                    int required = 0;
                    for (decl *q = p0; q; q = q->next) if (!q->init) required++;
                    int nuser = e->n_args - 1;                       // args[0] is `this`
                    if (nuser < required || nuser > f->n_params - 1) continue;
                    int score = 0, ok = 1; decl *p = p0;
                    for (int a = 1; a < e->n_args && p; a++, p = p->next) {
                        char pc = type_code(p->dtype), ac = type_code(infer_type(e->args[a]));
                        if (pc == ac) score += 2;
                        else if ((pc == 'i' && ac == 'f') || (pc == 'f' && ac == 'i')) score += 1;
                        else { ok = 0; break; }
                    }
                    if (ok && score > bests) { bests = score; bestm = f; }
                }
                if (bestm) {
                    decl *p = bestm->params;
                    for (int i = 0; i < e->n_args; i++) {
                        gen_arg(e->args[i], p ? p->dtype : NULL);
                        if (p) p = p->next;
                    }
                    for (; p; p = p->next) gen_arg(p->init, p->dtype);
                    emit("CAL %s", bestm->asm_label);
                    return;
                }
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
            // unqualified sibling-method call inside a method body -> this->fn(args)
            if (cur_method_class) {
                int vslot = vtbl_slot(cur_method_class, fn);
                char *masm = resolve_method(cur_method_class, fn);
                if (masm || vslot >= 0) {
                    sym *ms = masm ? st_find(masm) : NULL;
                    if (vslot >= 0) {
                        gen_load_this(); emit("PSH");        // this -> arg 0
                        emit("LDI 0");                          // acc = vptr = mem[this]
                        if (vslot) emit("ADD %d", vslot);
                        emit("LDI 0");                          // acc = fid = mem[vptr+slot]
                        emit("SET _fp_id");
                        for (int i = 0; i < e->n_args; i++) {
                            type *pt = (ms && ms->param_types && i + 1 < ms->n_params) ? ms->param_types[i + 1] : NULL;
                            gen_arg(e->args[i], pt);
                        }
                        emit_dispatch_chain();
                        free(masm);
                        return;
                    }
                    gen_load_this(); emit("PSH");
                    for (int i = 0; i < e->n_args; i++) {
                        type *pt = (ms && ms->param_types && i + 1 < ms->n_params) ? ms->param_types[i + 1] : NULL;
                        gen_arg(e->args[i], pt);
                    }
                    emit("CAL %s", masm);
                    free(masm);
                    return;
                }
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
// cursor that a designator resets. What the list leaves out takes its default
// member initializer, or zero (see emit_omitted).
static void emit_initz(const char *base, int off, type *t, initz *z);

// Set while a local's initializer is emitted: a local keeps fixed storage, so a
// second call finds the first call's values there, and an omitted part must be
// stored as zero. Static storage starts as the .mif's zeros and needs nothing.
static int g_initz_fill = 0;

// does value-initializing a t store anything besides zeros (a default member
// initializer somewhere inside)?
static int has_member_defaults(type *t)
{
    int n; t = array_elems(t, &n);
    if (!t || t->kind != TY_STRUCT || t->is_union) return 0;
    for (strct_field *f = t->fields; f; f = f->next)
        if (f->dinit || has_member_defaults(f->ftype)) return 1;
    return 0;
}

// zero n words of block base from word off: single stores, a loop for a run.
// The zero of a float is the 0.0 constant, not the word 0 (the encoder gives
// 0.0 the most negative exponent, and EQU compares words): t, the type the
// run belongs to, picks it the way agg_fill_code picks the .mif fill of a
// global, so a local and a global of the same type hold the same zero.
static void emit_zero_words(const char *base, int off, int n, const type *t)
{
    if (n <= 0) return;
    const char *zero = (agg_fill_code(t) == 2) ? "0.0" : "0";
    if (n <= 4) {
        for (int i = 0; i < n; i++) { emit("LOD %d", off + i); emit("PSH"); emit("LOD %s", zero); emit("STI %s", base); }
        return;
    }
    if (n >= 16) {
        // four words a turn: the loop control (the test on zi, its step, the
        // JMP back) is paid once per four stores, 5 instructions a word where
        // one a turn takes 7. The n % 4 lowest words are stored singly first;
        // the loop then covers [lo, off+n-1] top-down, zi the top of a group.
        int r = n % 4, lo = off + r;
        for (int i = 0; i < r; i++) { emit("LOD %d", off + i); emit("PSH"); emit("LOD %s", zero); emit("STI %s", base); }
        int id = ++label_n;
        char *zi = new_temp("zwi");
        emit("LOD %d", off + n - 1); emit("SET %s", zi);
        emit("@Lzw_t%d NOP", id);
        emit("PSH"); emit("LOD %s", zero); emit("STI %s", base);           // base[zi]   = 0 (acc = zi)
        for (int j = 1; j < 4; j++) {                                      // base[zi-j] = 0
            emit("LOD %s", zi); emit("ADD %d", -j);
            emit("PSH"); emit("LOD %s", zero); emit("STI %s", base);
        }
        emit("LOD %s", zi); emit("ADD %d", -(lo + 3));
        emit("JIZ Lzw_e%d", id);                                           // until zi - 3 == lo
        emit("ADD %d", lo + 3 - 4);                                        // acc = zi - 4
        emit("SET %s", zi);
        emit("JMP Lzw_t%d", id);
        emit("@Lzw_e%d NOP", id);
        free(zi);
        return;
    }
    // one index, walking down from the last word to `off`, and the loop enters
    // with it in acc (SET, STI and JIZ leave acc alone): 7 instructions a word
    // when off == 0, 8 otherwise, where two counters took 12
    int id = ++label_n;
    char *zi = new_temp("zwi");
    emit("LOD %d", off + n - 1); emit("SET %s", zi);
    emit("@Lzw_t%d NOP", id);
    emit("PSH"); emit("LOD %s", zero); emit("STI %s", base);               // base[zi] = 0
    emit("LOD %s", zi);
    if (off) emit("ADD %d", -off);
    emit("JIZ Lzw_e%d", id);                                               // until zi == off
    if (off != 1) emit("ADD %d", off - 1);
    emit("SET %s", zi);
    emit("JMP Lzw_t%d", id);
    emit("@Lzw_e%d NOP", id);
    free(zi);
}

// value-initialise a local aggregate or array (`T v{};`, `T v[N] = {};`): a
// local keeps fixed storage, so the zeros must be stored on every entry. A
// class that is constructed (a constructor, or a vtable) is zeroed first and
// then constructed as usual, so a user default constructor still runs; any
// other type goes through the empty braced list, which stores the zeros and
// the default member initializers.
static void emit_value_init(const char *base, type *t)
{
    int n; type *et = array_elems(t, &n);
    if (needs_construct(et)) {
        emit_zero_words(base, 0, type_size_words(t), t);
        emit_construct_decl(t, base, NULL, 0);
    } else {
        initz empty; memset(&empty, 0, sizeof empty); empty.is_list = 1;
        g_initz_fill = 1; emit_initz(base, 0, t, &empty); g_initz_fill = 0;
    }
}

// a sub-object of type t at word off that the braced list left out: its default
// member initializers, and zeros where there are none (stored for a local only)
static void emit_omitted(const char *base, int off, type *t)
{
    if (has_member_defaults(t)) {
        initz empty; memset(&empty, 0, sizeof empty); empty.is_list = 1;
        emit_initz(base, off, t, &empty);
    } else if (g_initz_fill) {
        emit_zero_words(base, off, type_size_words(t), t);
    }
}

static void emit_initz(const char *base, int off, type *t, initz *z)
{
    if (!z) return;
    if (!z->is_list) {
        infer_type(z->e);
        int agg = t && (t->kind == TY_ARRAY || t->kind == TY_STRUCT);
        if (!agg) {
            emit("LOD %d", off); emit("PSH");
            gen_expr_to(z->e, t);                            // convert per slot
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
                emit("LOD %s", st); if (i) emit("ADD %d", i); emit("LDI 0");
                emit("STI %s", base);
            }
            free(st);
        }
        return;
    }
    if (t && t->kind == TY_ARRAY) {
        int esz = type_size_words(t->base);
        int n = t->arr_size;
        char *done = calloc(n > 0 ? n : 1, 1);
        int cursor = 0;
        for (int k = 0; k < z->n; k++) {
            int idx = cursor;
            if (z->desigs[k].kind == DESIG_FIELD)
                msg_error(z->line, "field designator in array initializer");
            if (z->desigs[k].kind == DESIG_INDEX) idx = z->desigs[k].idx;
            cursor = idx + 1;
            if (idx >= 0 && idx < n) done[idx] = 1;
            emit_initz(base, off + idx * esz, t->base, z->items[k]);
        }
        int defaults = has_member_defaults(t->base);
        if (defaults || g_initz_fill) {
            for (int i = 0; i < n; ) {                  // each run of omitted elements
                if (done[i]) { i++; continue; }
                int j = i; while (j < n && !done[j]) j++;
                if (defaults) for (int k = i; k < j; k++) emit_omitted(base, off + k * esz, t->base);
                else          emit_zero_words(base, off + i * esz, (j - i) * esz, t->base);
                i = j;
            }
        }
        free(done);
    } else if (t && t->kind == TY_STRUCT) {
        strct_field *f = t->fields;
        strct_field **wr = calloc(z->n + 1, sizeof(strct_field*));
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
            wr[k] = target;
            emit_initz(base, off + target->offset, target->ftype, z->items[k]);
        }
        if (t->is_union) {                              // members overlap: only an empty list fills
            if (z->n == 0 && g_initz_fill) emit_zero_words(base, off, type_size_words(t), t);
        } else {
            for (strct_field *q = t->fields; q; q = q->next) {    // the members left out
                int written = 0;
                for (int k = 0; k < z->n; k++) if (wr[k] == q) written = 1;
                if (written || q->is_bitfield) continue;
                int scalar = q->ftype && q->ftype->kind != TY_ARRAY && q->ftype->kind != TY_STRUCT;
                if (q->dinit && scalar) {               // its default member initializer
                    emit("LOD %d", off + q->offset); emit("PSH");
                    gen_expr_to((expr*)q->dinit, q->ftype);
                    emit("STI %s", base);
                } else {
                    emit_omitted(base, off + q->offset, q->ftype);
                }
            }
        }
        free(wr);
    } else {
        // braced list wrapping a scalar, e.g. `{ x }` -> take the first element
        if (z->n > 0) emit_initz(base, off, t, z->items[0]);
    }
}

// A `static` local is initialised the first time control reaches its
// declaration, not at program start: C++ runs its initialiser (and its
// constructor) on that first pass and never again, so a function that is
// never called never builds its static. The word `<name>__once` guards it,
// zeroed for every static at main entry (so a reset re-runs them) and set
// after the initialiser. TODO.md item 11(d).
static void emit_static_guard_open(const char *aname, char **l_done)
{
    char *l_init = fresh_label("st_init");
    *l_done      = fresh_label("st_done");
    emit("LOD %s__once", aname);
    emit("JIZ %s", l_init);          // still zero -> first time here
    emit("JMP %s", *l_done);
    emit("@%s NOP", l_init);
    free(l_init);
}

static void emit_static_guard_close(const char *aname, char *l_done)
{
    emit("LOD 1");
    emit("SET %s__once", aname);
    emit("@%s NOP", l_done);
    free(l_done);
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
            else gen_expr_to(d->init, d->dtype);
            emit("STI 0");
        }
        return;
    }

    char *aname = mangle_local(d->name);
    { sym *ls = st_add(SK_LOCAL_VAR, d->name, aname, d->dtype); ls->is_const = d->is_const; }
    int arr_words = (d->dtype && d->dtype->kind == TY_ARRAY) ? type_size_words(d->dtype) : 0;
    log_var(cur_func_name ? cur_func_name : "global", d->name,
            innermost_code(d->dtype), arr_words);
    // A `static` local keeps fixed storage, but its initialiser runs HERE, the
    // first time control reaches the declaration, under its `__once` guard.
    int is_static = (d->sclass == SC_STATIC);
    char *st_done = NULL;
    if (is_static) {
        int n_el; type *et = array_elems(d->dtype, &n_el);
        int is_obj = et && et->kind == TY_STRUCT && et->tag;
        if (d->init || d->binit || is_obj) emit_static_guard_open(aname, &st_done);
        else is_static = 2;      /* nothing to run: storage only */
    }
    if (d->dtype && d->dtype->kind == TY_ARRAY) {
        // multi-dim arrays flatten to total word count; element type is the innermost scalar
        if (d->init_file) emit("#arrays %s %d %d \"%s\"", aname, innermost_code(d->dtype), arr_words, d->init_file);
        else              emit("#array %s %d %d",         aname, innermost_code(d->dtype), arr_words);
        if (st_done && d->binit)    emit_initz(aname, 0, d->dtype, d->binit);
        else if (st_done)           emit_construct_decl(d->dtype, aname, NULL, 0);
        else if (!is_static && d->binit) { g_initz_fill = 1; emit_initz(aname, 0, d->dtype, d->binit); g_initz_fill = 0; }
        else if (!is_static && d->vinit) emit_value_init(aname, d->dtype);           // T v[N] = {};
        else if (!is_static)        emit_construct_decl(d->dtype, aname, NULL, 0);   // objects: each element
    } else if (d->dtype && d->dtype->kind == TY_STRUCT) {
        emit("#array %s %d %d", aname, agg_fill_code(d->dtype), type_size_words(d->dtype));
        type *ct = d->dtype;
        if (st_done && d->binit)   emit_initz(aname, 0, d->dtype, d->binit);
        else if (st_done && d->init) copy_to_block(aname, d->init, type_size_words(d->dtype));
        else if (st_done)          emit_construct_decl(ct, aname, d->ctor_args, d->n_ctor_args);
        else if (is_static)     { /* storage only, nothing to run */ }
        else if (d->binit)      { g_initz_fill = 1; emit_initz(aname, 0, d->dtype, d->binit); g_initz_fill = 0; }
        else if (d->init)       copy_to_block(aname, d->init, type_size_words(d->dtype)); // = struct expr
        else if (d->vinit)      emit_value_init(aname, d->dtype);                         // T v{}; / T v = {};
        else if (ct->tag) emit_construct_decl(ct, aname, d->ctor_args, d->n_ctor_args);
    } else if (d->init && (!is_static || st_done)) {
        if (d->dtype && d->dtype->is_ref) gen_addr(d->init);   // bind reference to address
        else gen_expr_to(d->init, d->dtype);
        emit("SET %s", aname);
    } else if (d->vinit && !is_static) {
        // int x{}; float f{}; T *p{}: the float zero is the 0.0 constant
        emit("LOD %s", agg_fill_code(d->dtype) == 2 ? "0.0" : "0"); emit("SET %s", aname);
    }
    if (st_done) emit_static_guard_close(aname, st_done);
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

// emit destructor calls for the class-typed locals declared directly in this
// block, in reverse declaration order (RAII at normal block exit). Note: an
// early return/break/continue inside the block bypasses these — a known gap.
static void emit_block_dtors(stmt *block)
{
    for (int i = block->n_items - 1; i >= 0; i--) {
        stmt *it = block->items[i];
        if (!it || it->kind != S_DECL) continue;
        decl *ds[32]; int n = 0;
        for (decl *d = it->decls; d && n < 32; d = d->next) ds[n++] = d;
        for (int j = n - 1; j >= 0; j--) {
            decl *d = ds[j];
            if (!d->dtype || d->dtype->kind != TY_STRUCT || !d->dtype->tag) continue;
            char *dtor = resolve_method(d->dtype, "dtor");
            if (dtor) {
                char *an = mangle_local(d->name);
                emit("LEA %s", an); emit("PSH"); emit("CAL %s", dtor);
                free(an); free(dtor);
            }
        }
    }
}

// ---- early-exit RAII: live-local tracking + dtor emission ------------------
static void live_push(const char *mangled, const char *dtor)
{
    if (g_live_n < MAX_LIVE) {
        g_live[g_live_n].name = strdup(mangled);
        g_live[g_live_n].dtor = strdup(dtor);
        g_live_n++;
    }
}
static void live_pop_to(int mark)
{
    while (g_live_n > mark) { g_live_n--; free(g_live[g_live_n].name); free(g_live[g_live_n].dtor); }
}
// emit dtor calls for live class locals from the top down to (not including)
// `mark`, in reverse construction order. Does NOT pop g_live — the jump leaves;
// the codegen-time pop happens at the enclosing block's normal exit.
static void emit_live_dtors(int mark)
{
    for (int i = g_live_n - 1; i >= mark; i--)
        { emit("LEA %s", g_live[i].name); emit("PSH"); emit("CAL %s", g_live[i].dtor); }
}

// ---- loop-invariant address hoisting ----------------------------------------
// In `for (...) ... p[i*n + k] ...` the address part p + i*n does not change
// inside the loop when p, i and n are not written there: it is computed once,
// before the loop, into a pointer temporary t, and the access becomes t[k]
// (3 instructions where the multiply-add form took 6). This is the Cholesky
// inner loop of test46. Only what is provably safe is touched:
//   - p is a plain pointer local or parameter (not a reference, not in a
//     stack frame), and the invariant part is int literals and int locals /
//     parameters combined with + - *, with a variable or an operator in it
//     (a bare literal costs nothing each turn); a lone invariant variable
//     needs a varying part next to it (h[k + d] -> t[k], t = h + d);
//   - none of those names is written in the loop (assignment, ++/--, a
//     declaration inside it, passed to a reference parameter or to a callee
//     whose parameters are unknown, address taken), nor escapes anywhere in
//     the function (the same, or bound to a reference), so no alias can
//     write it either;
//   - not in a recursive function (its locals live in a frame), nor in one
//     with goto / labels / inline asm.
// The hoisted add runs even when the loop runs zero times: it is pure int
// arithmetic, with nothing to trap or observe.

typedef struct { const char **v; int n, cap; } nameset;
static void ns_add(nameset *s, const char *n)
{
    if (!n) return;
    for (int i = 0; i < s->n; i++) if (!strcmp(s->v[i], n)) return;
    if (s->n == s->cap) { s->cap = s->cap ? 2 * s->cap : 16; s->v = realloc(s->v, s->cap * sizeof *s->v); }
    s->v[s->n++] = n;
}
static int ns_has(const nameset *s, const char *n)
{
    for (int i = 0; i < s->n; i++) if (!strcmp(s->v[i], n)) return 1;
    return 0;
}

static stmt   *lih_body     = NULL;   // body of the function being emitted
static int     lih_ready    = 0;      // lih_escaped / lih_off computed for it
static int     lih_off      = 0;      // goto / label / asm / recursion: no hoisting
static nameset lih_escaped  = {0};

// written == 1: collect what a loop may write (assignments, ++/--, names it
// declares, call arguments, address taken). written == 0: what escapes in the
// whole function (address taken, call arguments, reference bindings).
static void lih_scan_initz(initz *z, nameset *s, int written);
static void lih_scan_expr(expr *e, nameset *s, int written)
{
    if (!e) return;
    switch (e->kind) {
    case E_ASSIGN:
        if (written && e->a && e->a->kind == E_IDENT) ns_add(s, e->a->sval);
        break;
    case E_PREINC: case E_PREDEC: case E_POSTINC: case E_POSTDEC:
        if (written && e->a && e->a->kind == E_IDENT) ns_add(s, e->a->sval);
        break;
    case E_ADDR:
        if (e->a && e->a->kind == E_IDENT) ns_add(s, e->a->sval);
        break;
    case E_CALL: {
        // an argument can be written by the callee only through a reference
        // parameter: a known function whose parameter is by value leaves the
        // caller's variable alone (an unknown callee is assumed to write)
        sym *fs = (e->a && e->a->kind == E_IDENT) ? st_find(e->a->sval) : NULL;
        if (fs && (fs->kind != SK_FUNC || fs->n_params != e->n_args)) fs = NULL;   // another overload: unknown
        for (int i = 0; i < e->n_args; i++) {
            if (!e->args[i] || e->args[i]->kind != E_IDENT) continue;
            int by_value = fs && fs->param_types && i < fs->n_params &&
                           fs->param_types[i] && !fs->param_types[i]->is_ref;
            if (!by_value) ns_add(s, e->args[i]->sval);
        }
        if (e->a && (e->a->kind == E_MEMBER || e->a->kind == E_PMEMBER) && e->a->a && e->a->a->kind == E_IDENT)
            ns_add(s, e->a->a->sval);                         // obj.method(): may write obj
        break;
    }
    default: break;
    }
    lih_scan_expr(e->a, s, written); lih_scan_expr(e->b, s, written); lih_scan_expr(e->c, s, written);
    for (int i = 0; i < e->n_args; i++) lih_scan_expr(e->args[i], s, written);
    if (e->cinit) lih_scan_initz(e->cinit, s, written);
}
static void lih_scan_initz(initz *z, nameset *s, int written)
{
    if (!z) return;
    if (!z->is_list) { lih_scan_expr(z->e, s, written); return; }
    for (int i = 0; i < z->n; i++) lih_scan_initz(z->items[i], s, written);
}
static void lih_scan_stmt(stmt *st, nameset *s, int written)
{
    if (!st) return;
    if (st->kind == S_GOTO || st->kind == S_LABEL || st->kind == S_ASM) lih_off = 1;
    for (decl *d = st->kind == S_DECL ? st->decls : NULL; d; d = d->next) {
        if (written) ns_add(s, d->name);
        else if (d->dtype && d->dtype->is_ref && d->init && d->init->kind == E_IDENT) ns_add(s, d->init->sval);
        lih_scan_expr(d->init, s, written);
        lih_scan_initz(d->binit, s, written);
        for (int i = 0; i < d->n_ctor_args; i++) {
            lih_scan_expr(d->ctor_args[i], s, written);
            if (d->ctor_args[i] && d->ctor_args[i]->kind == E_IDENT) ns_add(s, d->ctor_args[i]->sval);
        }
    }
    lih_scan_expr(st->e1, s, written); lih_scan_expr(st->e2, s, written); lih_scan_expr(st->e3, s, written);
    lih_scan_stmt(st->init_stmt, s, written);
    lih_scan_stmt(st->body, s, written); lih_scan_stmt(st->body2, s, written);
    for (int i = 0; i < st->n_items; i++) lih_scan_stmt(st->items[i], s, written);
}

// a plain local / parameter of type k, not a reference, not in a frame, not
// written in the loop, not escaping the function
static int lih_plain_var(expr *e, const nameset *w, type_kind k)
{
    if (!e || e->kind != E_IDENT) return 0;
    sym *s = st_find(e->sval);
    if (!s || (s->kind != SK_LOCAL_VAR && s->kind != SK_PARAM) || s->is_frame) return 0;
    if (!s->stype || s->stype->kind != k || s->stype->is_ref) return 0;
    return !ns_has(w, e->sval) && !ns_has(&lih_escaped, e->sval);
}
static int lih_invariant(expr *e, const nameset *w)
{
    if (!e) return 0;
    if (e->kind == E_INT_LIT) return 1;
    if (e->kind == E_IDENT) return lih_plain_var(e, w, TY_INT);
    if (e->kind == E_BINOP && (e->op == OP_ADD || e->op == OP_SUB || e->op == OP_MUL))
        return lih_invariant(e->a, w) && lih_invariant(e->b, w);
    return 0;
}
static int lih_expr_eq(expr *x, expr *y)
{
    if (!x || !y) return x == y;
    if (x->kind != y->kind) return 0;
    if (x->kind == E_INT_LIT) return x->ival == y->ival;
    if (x->kind == E_IDENT)   return !strcmp(x->sval, y->sval);
    if (x->kind == E_BINOP)   return x->op == y->op && lih_expr_eq(x->a, y->a) && lih_expr_eq(x->b, y->b);
    return 0;
}
static void lih_flatten(expr *e, expr **t, int *n)       // the terms of an ADD chain
{
    if (e && e->kind == E_BINOP && e->op == OP_ADD && *n < 14) { lih_flatten(e->a, t, n); lih_flatten(e->b, t, n); }
    else if (*n < 16) t[(*n)++] = e;
}

typedef struct { const char *base; expr *inv; char *tmp; } lih_hoist;
static lih_hoist lih_h[16]; static int lih_nh;

static void lih_rewrite_expr(expr *e, const nameset *w)
{
    if (!e) return;
    lih_rewrite_expr(e->a, w); lih_rewrite_expr(e->b, w); lih_rewrite_expr(e->c, w);
    for (int i = 0; i < e->n_args; i++) lih_rewrite_expr(e->args[i], w);
    if (e->kind != E_INDEX || !lih_plain_var(e->a, w, TY_PTR)) return;
    expr *t[16]; int n = 0, has_op = 0;
    lih_flatten(e->b, t, &n);
    expr *inv = NULL, *var = NULL;
    for (int i = 0; i < n; i++) {
        if (lih_invariant(t[i], w)) { if (t[i]->kind != E_INT_LIT) has_op = 1; inv = inv ? ast_binop(OP_ADD, inv, t[i], e->line) : t[i]; }
        else                        var = var ? ast_binop(OP_ADD, var, t[i], e->line) : t[i];
    }
    // worth a temporary when the invariant part costs something each turn
    // (a variable or an operation, not a bare literal) and something varies
    // (h[k + d] -> t[k] with t = h + d); a whole-invariant index needs an op
    if (!inv || !has_op || (!var && inv->kind != E_BINOP)) return;
    const char *tmp = NULL;
    for (int i = 0; i < lih_nh; i++)
        if (!strcmp(lih_h[i].base, e->a->sval) && lih_expr_eq(lih_h[i].inv, inv)) tmp = lih_h[i].tmp;
    if (!tmp) {
        if (lih_nh == 16) return;
        char nm[32]; snprintf(nm, sizeof nm, "_lip%d", ++label_n);
        char *an = mangle_local(nm);
        st_add(SK_LOCAL_VAR, nm, an, st_find(e->a->sval)->stype);
        log_var(cur_func_name ? cur_func_name : "global", nm, 1, 0);
        free(an);
        lih_h[lih_nh].base = e->a->sval; lih_h[lih_nh].inv = inv; lih_h[lih_nh].tmp = strdup(nm);
        tmp = lih_h[lih_nh++].tmp;
    }
    e->a = ast_ident((char *)tmp, e->line);
    e->b = var ? var : ast_int_lit(0, e->line);
    e->etype = NULL;
}
static void lih_rewrite_stmt(stmt *st, const nameset *w)
{
    if (!st) return;
    for (decl *d = st->kind == S_DECL ? st->decls : NULL; d; d = d->next) {
        lih_rewrite_expr(d->init, w);
        for (int i = 0; i < d->n_ctor_args; i++) lih_rewrite_expr(d->ctor_args[i], w);
    }
    lih_rewrite_expr(st->e1, w); lih_rewrite_expr(st->e2, w); lih_rewrite_expr(st->e3, w);
    lih_rewrite_stmt(st->init_stmt, w);
    lih_rewrite_stmt(st->body, w); lih_rewrite_stmt(st->body2, w);
    for (int i = 0; i < st->n_items; i++) lih_rewrite_stmt(st->items[i], w);
}

// called at a `for`, after its init: emit the hoisted addresses, rewrite the
// accesses that use them
static void lih_for(stmt *s)
{
    if (!lih_ready) {
        lih_ready = 1; lih_escaped.n = 0;
        lih_off = cur_fn_recursive;
        lih_scan_stmt(lih_body, &lih_escaped, 0);
    }
    if (lih_off) return;
    nameset w = {0};
    lih_scan_expr(s->e1, &w, 1); lih_scan_expr(s->e2, &w, 1);
    lih_scan_stmt(s->body, &w, 1);
    lih_nh = 0;
    lih_rewrite_expr(s->e1, &w); lih_rewrite_expr(s->e2, &w);
    lih_rewrite_stmt(s->body, &w);
    for (int i = 0; i < lih_nh; i++) {
        expr *base = ast_ident((char *)lih_h[i].base, s->line);
        expr *as = ast_assign(ast_ident(lih_h[i].tmp, s->line), ast_binop(OP_ADD, base, lih_h[i].inv, s->line), s->line);
        infer_type(as); gen_expr(as);
    }
    free(w.v);
}

// Wrapper: stamp this statement's source line so every instruction it emits
// maps back to it in pc_<proc>_mem.txt, then restore on exit so the enclosing
// statement's line resumes after a nested block returns. The real dispatch is
// in gen_stmt_inner (many early returns, hence the single-exit wrapper).
static void gen_stmt(stmt *s)
{
    if (!s) return;
    int saved_line = cg_line;
    if (s->line > 0) cg_line = s->line;
    gen_stmt_inner(s);
    cg_line = saved_line;
}

static void gen_stmt_inner(stmt *s)
{
    if (!s) return;
    switch (s->kind) {
    case S_NULL: return;
    case S_EXPR: if (s->e1) { infer_type(s->e1); gen_expr(s->e1); } return;
    case S_BLOCK: {
        st_push_scope();
        int live_mark = g_live_n;
        for (int i = 0; i < s->n_items; i++) gen_stmt(s->items[i]);
        emit_block_dtors(s);
        live_pop_to(live_mark);     // pop this block's tracked live locals
        st_pop_scope();
        return;
    }

    case S_DECL:
        for (decl *d = s->decls; d; d = d->next) {
            declare_local(d);
            // track constructed class locals so an early exit can run their dtors
            if (d->dtype && d->dtype->kind == TY_STRUCT && d->dtype->tag) {
                char *dt = resolve_method(d->dtype, "dtor");
                if (dt) { char *an = mangle_local(d->name); live_push(an, dt); free(an); free(dt); }
            }
        }
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
        lih_for(s);                                  // loop-invariant addresses, once
        // Test at the bottom: the condition is checked once on entry, then
        // right after the step, jumping back when it holds -- no JMP per
        // turn, and with nothing (no label) between the step and the test
        // the peephole drops the test's reload of the just-stepped variable
        // (`k < 10`: LOD k; ADD 1; SET k; LES 9; JIZ top). `continue` still
        // lands on the step. Costs the condition's code once more.
        // (no entry test at all when `k = c0` already satisfies `k OP c1`)
        if (s->e1 && !for_entry_holds(s)) { infer_type(s->e1); gen_bool(s->e1, end); }
        emit("@%s NOP", top);
        loop_push(cont, end);
        gen_stmt(s->body);
        loop_pop();
        emit("@%s NOP", cont);
        if (s->e2) { infer_type(s->e2); gen_expr(s->e2); }
        if (s->e1) gen_jump_true(s->e1, top);
        else       emit("JMP %s", top);
        emit("@%s NOP", end);
        st_pop_scope();
        free(top); free(cont); free(end);
        return;
    }

    case S_BREAK:
        if (brk_top > 0) { emit_live_dtors(brk_live[brk_top-1]); emit("JMP %s", brk_stk[brk_top-1]); }
        else msg_error(s->line, "break outside of loop/switch");
        return;
    case S_CONTINUE:
        if (loop_top == 0) msg_error(s->line, "continue outside of loop");
        emit_live_dtors(loop_live[loop_top-1]);
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
            } else if (cur_func_ret && cur_func_ret->is_ref) {
                gen_addr(s->e1);   // reference return: yield the referent's address
            } else {
                gen_expr_to(s->e1, cur_func_ret);
            }
        }
        if (g_live_n > 0) {                  // RAII: run dtors for in-scope class locals first
            int hasval = s->e1 && cur_func_ret && cur_func_ret->kind != TY_VOID;
            char *rs = NULL;
            if (hasval) {                    // protect the return value across the dtor CALs
                char rn[32]; snprintf(rn, sizeof(rn), "_rsv%d", ++label_n);
                rs = mangle_local(rn);
                st_add(SK_LOCAL_VAR, rn, rs, t_int());
                log_var(cur_func_name ? cur_func_name : "global", rn, 1, 0);
                emit("SET %s", rs);
            }
            emit_live_dtors(0);
            if (hasval) { emit("LOD %s", rs); free(rs); }
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
                    emit_verbatim(buf);  // user inline asm: verbatim, never fused
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
    g_nbmant = mant;

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
    emit("#FROUND 2");   // C++ always gets the most precise float datapath (round to nearest even)
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
            emit("#array %s %d %d", d->name, agg_fill_code(d->dtype), type_size_words(d->dtype));
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
    int saved = cg_line;
    for (int i = 0; i < u->n_globals; i++) {
        decl *d = u->globals[i];
        if (!d->dtype) continue;
        // map this global's init instructions to its declaration line, so they
        // show as that C++ source line in GTKWave (not -1/INTERNAL scaffolding).
        if (d->line > 0) cg_line = d->line;
        if (d->dtype->kind == TY_ARRAY || d->dtype->kind == TY_STRUCT) {
            if      (d->binit) emit_initz(d->name, 0, d->dtype, d->binit);
            else if (d->init)  copy_to_block(d->name, d->init, type_size_words(d->dtype));
            continue;
        }
        if (!d->init) continue;
        gen_expr_to(d->init, d->dtype);
        emit("SET %s", d->name);
    }
    // then the global objects' constructors, in declaration order: as in C++,
    // a constructor runs after every global value above is in place
    for (int i = 0; i < u->n_globals; i++) {
        decl *d = u->globals[i];
        if (!d->dtype || d->binit || d->init) continue;
        if (d->dtype->kind != TY_ARRAY && d->dtype->kind != TY_STRUCT) continue;
        if (d->line > 0) cg_line = d->line;
        emit_construct_decl(d->dtype, d->name, d->ctor_args, d->n_ctor_args);
    }
    cg_line = saved;
}

// emit the one-time initializers for `static` locals (collected pre-pass)
// Zero every static local's `__once` guard at main entry. The initialisers
// themselves run at their declarations (see emit_static_guard_open): all this
// has to do is make a fresh start -- including after a reset, which re-runs
// main but does not reload the data memory -- look like one.
static void emit_static_once_flags(void)
{
    int saved = cg_line;
    for (int i = 0; i < n_stinit; i++) {
        stinit_e *e = &stinits[i];
        if (e->line > 0) cg_line = e->line;   // map to the static local's decl line
        emit("LOD 0");
        emit("SET %s__once", e->base);
    }
    cg_line = saved;
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
        emit("LOD __fp"); emit("LDI 0"); emit("SET __fp");    // FP = caller FP (saved at frame base)
        emit("LOD __ret");
    }
    emit("RET");
}

static void emit_function(func *f, unit *u, int is_main)
{
    cg_line = -1;                      // prologue / @label NOP are INTERNAL; the
                                       // body's gen_stmt stamps real source lines
    if (!f->asm_label) f->asm_label = f->name;
    cur_func_name    = f->asm_label;   // unique label keeps overloads' locals distinct
    cur_func_ret     = f->ret;
    cur_fn_recursive = f->is_recursive;
    cur_method_class = f->method_of;
    lih_body = f->body; lih_ready = 0; // loop-invariant hoisting analyses this body lazily
    live_pop_to(0);                    // fresh early-exit RAII tracking per function
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
            // log under cur_func_name (== f->asm_label), the SAME prefix
            // mangle_local stamps into the .asm operand, so asmcomp's
            // sim_is_var reconstructs <func>_<var> and matches. For overloaded
            // functions f->name != asm_label, so f->name would mismatch.
            log_var(cur_func_name, plist[i]->name, type_code_for(plist[i]->dtype),
                    plist[i]->dtype && plist[i]->dtype->kind == TY_ARRAY ? plist[i]->dtype->arr_size : 0);
            free(aname);
        }
    }

    emit("@%s NOP", f->asm_label);

    if (!is_main && f->ret && f->ret->kind == TY_STRUCT)
        emit("#array _ret_%s 1 %d", f->asm_label, type_size_words(f->ret));

    if (cur_fn_recursive) {
        // prologue: push a new frame
        emit("LOD __sp"); emit("PSH"); emit("LOD __fp"); emit("STI 0");      // mem[SP] = FP
        emit("LOD __sp"); emit("SET __fp");                                // FP = SP
        emit("LOD __sp"); emit("ADD %d", f->frame_size); emit("SET __sp"); // SP += frame_size
        for (int i = np - 1; i >= 0; i--) {                                // args -> frame slots
            sym *sy = st_find(plist[i]->name);
            emit("POP"); emit("SET __argv");
            emit("LOD __fp"); if (sy->frame_off) emit("ADD %d", sy->frame_off); emit("PSH");
            emit("LOD __argv"); emit("STI 0");
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
        emit_string_inits(); emit_global_scalar_inits(u); emit_static_once_flags();
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

// unsigned -> float, value in acc both ways. I2F reads a signed word, so a
// word with bit 31 set is halved first, keeping its lowest bit as a sticky bit
// so the rounding stays exact, then converted and doubled.
static void emit_u2f(void)
{
    emit("@u2f NOP");
    emit("SET u2f_x");
    emit("GRE 0"); emit("JIZ u2f_lo");                               // bit 31 clear: plain I2F
    emit("LOD u2f_x"); emit("PSH"); emit("LOD 1"); emit("S_SHR");    // x >> 1, logical
    emit("PSH"); emit("LOD u2f_x"); emit("AND 1"); emit("S_ORR");    // | (x & 1), the sticky bit
    emit("I2F"); emit("F_MLT 2.0");
    emit("RET");
    emit("@u2f_lo NOP");
    emit("LOD u2f_x"); emit("I2F");
    emit("RET");
}

// float -> unsigned, value in acc both ways. F2I saturates at INT_MAX, so a
// value at or above 2^31 is brought down by 2^31 (exact in that range),
// converted, and given bit 31 back.
static void emit_f2u(void)
{
    emit("@f2u NOP");
    emit("SET f2u_f");
    emit("F_GRE 2147483648.0"); emit("JIZ f2u_hi");                  // below 2^31: plain F2I
    emit("LOD f2u_f"); emit("F2I");
    emit("RET");
    emit("@f2u_hi NOP");
    emit("LOD f2u_f"); emit("F_ADD -2147483648.0"); emit("F2I");
    emit("XOR %ld", bf_word(1ULL << (g_nubits - 1)));               // + 2^31
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
    emit("LOD __m_cur"); emit("PSH"); emit("LOD 1"); emit("NEG"); emit("S_ADD"); emit("LDI 0");
    emit("SET __m_csz");                               // csz = mem[cur-1] (block size)
    // if (csz >= sz) reuse: !(csz < sz)
    emit("LOD __m_csz"); emit("PSH"); emit("LOD __m_sz"); emit("S_LES"); emit("LIN");
    emit("JIZ mal_next");
    emit("LOD __m_cur"); emit("LDI 0"); emit("SET __m_lnk");          // link = mem[cur]
    emit("LOD __m_prev"); emit("JIZ mal_head");
    emit("LOD __m_prev"); emit("PSH"); emit("LOD __m_lnk"); emit("STI 0"); // mem[prev]=link
    emit("JMP mal_reuse");
    emit("@mal_head NOP");
    emit("LOD __m_lnk"); emit("SET __flist");          // __flist = link
    emit("@mal_reuse NOP");
    emit("LOD __m_cur"); emit("RET");                  // return payload (cur)
    emit("@mal_next NOP");
    emit("LOD __m_cur"); emit("SET __m_prev");
    emit("LOD __m_cur"); emit("LDI 0"); emit("SET __m_cur");          // cur = mem[cur] (next)
    emit("JMP mal_loop");
    // ---- bump-allocate: need = hp + sz + 1 ; require need <= hend -----------
    emit("@mal_bump NOP");
    emit("LOD __hp"); emit("PSH"); emit("LOD __m_sz"); emit("S_ADD"); emit("PSH"); emit("LOD 1"); emit("S_ADD");
    emit("SET __m_need");
    emit("LOD __m_need"); emit("PSH"); emit("LOD __hend"); emit("S_GRE"); emit("LIN"); // need<=hend
    emit("JIZ mal_oom");
    emit("LOD __hp"); emit("PSH"); emit("LOD __m_sz"); emit("STI 0");      // mem[hp]=sz
    emit("LOD __hp"); emit("ADD 1"); emit("SET __m_ret");                // ret=hp+1 (payload)
    emit("LOD __m_need"); emit("SET __hp");                              // hp=need
    emit("LOD __m_ret"); emit("RET");
    emit("@mal_oom NOP");
    emit("LOD 0"); emit("RET");                                         // NULL

    // ---- free(p) : push payload p onto the free list -----------------------
    emit("@free NOP");
    emit("POP"); emit("SET __m_cur");                  // p (payload addr)
    emit("LOD __m_cur"); emit("JIZ free_ret");         // free(0) is a no-op
    emit("LOD __m_cur"); emit("PSH"); emit("LOD __flist"); emit("STI 0");  // mem[p]=__flist
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
            emit("LOD %d", id); emit("STI 0");          // mem[vtable+slot] = id
            if (impl) free(impl);
        }
    }
}

static void write_cmm_log(const char *tmp_dir)
{
    char path[2048]; snprintf(path, sizeof(path), "%s/cmm_log.txt", tmp_dir);
    FILE *f = fopen(path, "w");
    if (!f) { fprintf(stderr, "cppcomp: cannot open %s\n", path); return; }
    for (int i = 0; i < varlog_n; i++) {
        if (varlog[i].size > 0)
            fprintf(f, "%s %s %d %d\n", varlog[i].func, varlog[i].var, varlog[i].type_code, varlog[i].size);
        else
            fprintf(f, "%s %s %d\n", varlog[i].func, varlog[i].var, varlog[i].type_code);
    }
    fprintf(f, "num_ins %d\n", ins_count);
    fclose(f);
}

// trad_cmm.txt: the source the pc_<proc>_mem.txt line numbers index into, so
// GTKWave can show the text for the line the processor is executing. Three
// fixed header rows (the negative scaffolding codes) followed by every source
// line numbered from 1. src_path is the file cppcomp actually compiled (the
// preprocessed pp.cpp), so its line numbers match the AST's ->line stamps.
static void write_trad_cmm(const char *tmp_dir, const char *src_path)
{
    char path[2048]; snprintf(path, sizeof(path), "%s/trad_cmm.txt", tmp_dir);
    FILE *out = fopen(path, "w");
    if (!out) { fprintf(stderr, "cppcomp: cannot open %s\n", path); return; }
    FILE *in  = fopen(src_path, "r");
    if (!in)  { fprintf(stderr, "cppcomp: cannot open %s\n", src_path); fclose(out); return; }

    fputs("-1 INTERNAL\n"    , out);   // pc code -1: scaffolding (NOP, prologue)
    fputs("-2 void main();\n", out);   // pc code -2: entry call
    fputs("-3 END\n"         , out);   // pc code -3: @fim halt loop

    char line[1024];
    int  n = 1;
    while (fgets(line, sizeof(line), in)) fprintf(out, "%d %s", n++, line);

    fclose(in);
    fclose(out);
}

// ---- recursion analysis (hybrid frames) ------------------------------------
// A function needs a stack frame iff it can reach itself in the call graph.
// Edges: a direct call F->G; an indirect (function-pointer) call in F adds an
// edge F->A for EVERY address-taken function A (conservative: an fp could call
// any of them). Non-recursive functions keep fast fixed-address locals.
static void collect_calls_expr(expr *e, unit *u, char *row, int *indirect)
{
    if (!e) return;
    // `delete p` dispatches p's (possibly virtual) destructor — an indirect call
    // into the address-taken dtor impls, which can re-enter the same dtor (e.g. a
    // tree dtor doing `delete child`). Treat it like an indirect call so such a
    // dtor is detected as recursive and gets a stack frame (its `this` must not
    // live at a fixed address that a nested dtor invocation would clobber).
    if (e->kind == E_DELETE) { *indirect = 1; collect_calls_expr(e->a, u, row, indirect); return; }
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

// ---- function-template monomorphization ------------------------------------
// Templates are captured (u->templates) with t_tparam placeholder types. A call
// to a template deduces concrete types from its arguments and instantiates a
// concrete clone (added to u->funcs as an overload named the same), so the
// existing overload resolution/labeling picks the right instance per type.

// non-type template parameter values for the instance currently being emitted
// (a class-template method clone); indexed by parameter slot k.
static int g_subst_vals[8]; static int g_subst_nvals = 0;

static type *subst_type(type *t, type **a, int n)
{
    if (!t) return t;
    if (t->tparam > 0 && t->tparam <= n) return a[t->tparam - 1];
    if (t->kind == TY_PTR || t->kind == TY_ARRAY) {
        type *nb = subst_type(t->base, a, n);
        int sz = t->arr_size;
        if (t->kind == TY_ARRAY && sz >= NTP_BASE) {        // non-type-param length
            int k = sz - NTP_BASE; sz = (k < g_subst_nvals) ? g_subst_vals[k] : 0;
        }
        if (nb == t->base && sz == t->arr_size) return t;
        type *nt = (t->kind == TY_PTR) ? t_ptr(nb) : t_array(nb, sz);
        nt->is_ref = t->is_ref;
        return nt;
    }
    return t;
}

static expr *clone_expr(expr *e, type **a, int n)
{
    if (!e) return NULL;
    expr *c = malloc(sizeof(expr)); *c = *e;       // shallow copy (strings shared)
    c->etype = NULL;                                // re-infer per instance
    // a non-type template parameter referenced in an expression is an int with a
    // sentinel value — replace it with the concrete argument for this instance.
    if (e->kind == E_INT_LIT && e->ival >= NTP_BASE && (e->ival - NTP_BASE) < g_subst_nvals)
        c->ival = g_subst_vals[e->ival - NTP_BASE];
    c->target_t = subst_type(e->target_t, a, n);
    c->a = clone_expr(e->a, a, n);
    c->b = clone_expr(e->b, a, n);
    c->c = clone_expr(e->c, a, n);
    if (e->n_args) {
        c->args = malloc(sizeof(expr*) * e->n_args);
        for (int i = 0; i < e->n_args; i++) c->args[i] = clone_expr(e->args[i], a, n);
    }
    return c;
}

static decl *clone_decl(decl *d, type **a, int n)
{
    if (!d) return NULL;
    decl *c = malloc(sizeof(decl)); *c = *d;
    c->dtype = subst_type(d->dtype, a, n);
    c->init  = clone_expr(d->init, a, n);
    if (d->n_ctor_args) {
        c->ctor_args = malloc(sizeof(expr*) * d->n_ctor_args);
        for (int i = 0; i < d->n_ctor_args; i++) c->ctor_args[i] = clone_expr(d->ctor_args[i], a, n);
    }
    c->next = clone_decl(d->next, a, n);
    return c;
}

static stmt *clone_stmt(stmt *s, type **a, int n)
{
    if (!s) return NULL;
    stmt *c = malloc(sizeof(stmt)); *c = *s;
    c->e1 = clone_expr(s->e1, a, n);
    c->e2 = clone_expr(s->e2, a, n);
    c->e3 = clone_expr(s->e3, a, n);
    c->body = clone_stmt(s->body, a, n);
    c->body2 = clone_stmt(s->body2, a, n);
    c->init_stmt = clone_stmt(s->init_stmt, a, n);
    if (s->n_items) {
        c->items = malloc(sizeof(stmt*) * s->n_items);
        for (int i = 0; i < s->n_items; i++) c->items[i] = clone_stmt(s->items[i], a, n);
    }
    c->decls = clone_decl(s->decls, a, n);
    return c;
}

// instantiated template clones (each with a unique asm_label); their bodies are
// emitted after @fim (reached only via CAL), draining the list as new instances
// appear during emission.
// (g_inst / g_n_inst are declared near cg_unit, at the top: the inliner
//  looks instances up long before this point in the file)

static func *find_template(const char *name)
{
    for (int i = 0; i < cg_unit->n_templates; i++)
        if (!strcmp(cg_unit->templates[i]->name, name)) return cg_unit->templates[i];
    return NULL;
}

// deduce each type parameter of template `t` from the call's argument types
static void deduce_targs(func *t, expr **cargs, int ncargs, type **targs)
{
    int j = 0;
    for (decl *p = t->params; p && j < ncargs; p = p->next, j++)
        if (p->dtype && p->dtype->tparam > 0 && p->dtype->tparam <= 8)
            targs[p->dtype->tparam - 1] = infer_type(cargs[j]);
    for (int i = 0; i < t->n_tparams; i++) if (!targs[i]) targs[i] = t_int();
}

// get (creating if needed) the concrete instance of `t` for these argument types
static func *get_instance(func *t, expr **cargs, int ncargs, type **expl, int nexpl)
{
    type *targs[8] = {0};
    deduce_targs(t, cargs, ncargs, targs);
    for (int i = 0; i < nexpl && i < 8; i++) targs[i] = expl[i];   // explicit args win
    char lbl[96]; int k = sprintf(lbl, "%s_T", t->name);
    for (int i = 0; i < t->n_tparams && k < (int)sizeof(lbl) - 1; i++)
        lbl[k++] = type_code(targs[i]);          // label over ALL tparams (incl. explicit-only)
    lbl[k] = 0;
    for (int i = 0; i < g_n_inst; i++) if (!strcmp(g_inst[i]->asm_label, lbl)) return g_inst[i];
    func *f = malloc(sizeof(func)); *f = *t;
    f->n_tparams = 0;
    f->ret    = subst_type(t->ret, targs, t->n_tparams);
    f->params = clone_decl(t->params, targs, t->n_tparams);
    f->body   = clone_stmt(t->body, targs, t->n_tparams);
    f->asm_label = strdup(lbl);
    if (g_n_inst < 256) g_inst[g_n_inst++] = f;
    return f;
}

// Real class-template monomorphization: for each requested instance Name<vals>,
// clone the template's methods with the non-type values substituted, retype
// `this` to the concrete class, register the mangled symbols (so call sites
// resolve) and queue them for emission (reusing the post-@fim instance list).
static void instantiate_ctmpl_methods(unit *u)
{
    for (int ii = 0; ii < u->n_ctinsts; ii++) {
        ctinst *ins = u->ctinsts[ii];
        ctmpl  *ct  = ins->tmpl;
        int     plen = (int)strlen(ct->proto->tag) + 2;     // strip "Proto__"
        g_subst_nvals = ins->nvals;
        for (int v = 0; v < ins->nvals && v < 8; v++) g_subst_vals[v] = ins->vals[v];
        type **ta = ins->ntargs ? ins->targs : NULL;        // type-parameter bindings
        int    nta = ins->ntargs;
        for (int m = 0; m < ct->n_methods; m++) {
            func *src = ct->methods[m];
            func *f = malloc(sizeof(func)); *f = *src;
            f->n_tparams = 0;
            f->method_of = ins->concrete;
            f->ret    = subst_type(src->ret, ta, nta);
            f->params = clone_decl(src->params, ta, nta);
            if (f->params) f->params->dtype = t_ptr(ins->concrete);   // retype `this`
            f->body   = clone_stmt(src->body, ta, nta);
            char *nm  = cg_mangle_method(ins->concrete->tag, src->name + plen);
            f->name = nm; f->asm_label = nm;
            sym *s = st_add(SK_FUNC, nm, nm, f->ret);
            s->n_params = f->n_params;
            s->param_types = malloc(sizeof(type*) * (f->n_params > 0 ? f->n_params : 1));
            { int i = 0; for (decl *p = f->params; p; p = p->next, i++) s->param_types[i] = p->dtype; }
            if (g_n_inst < 256) g_inst[g_n_inst++] = f;
        }
        g_subst_nvals = 0;
    }
}

// ---- post-emit peephole ----------------------------------------------------
// A bare PSH immediately followed by a load-class op (no label between them →
// they always execute as a pair) fuses into the op's P_ / PF_ variant, which
// pushes the accumulator as part of the same instruction. The list below is
// exactly the ops that have such a fused opcode in the ISA.
//   PSH; LOD x      -> P_LOD x        PSH; NEG_M x   -> P_NEG_M x
//   PSH; F_NEG_M x  -> PF_NEG_M x     PSH; F2I_M x   -> P_F2I_M x   ...
// Returns the fused text for the instruction that follows a PSH, or NULL.
static char *fuse_after_psh(const char *t)
{
    static const char *fusable[] = {
        "LOD","LOD_V","NEG_M","F_NEG_M","ABS_M","F_ABS_M","PST_M","F_PST_M",
        "NRM_M","I2F_M","F2I_M","INV_M","LIN_M","INN","F_INN", NULL };
    size_t mlen = strcspn(t, " ");           // first token = mnemonic
    char mnem[16];
    if (mlen == 0 || mlen >= sizeof(mnem)) return NULL;
    memcpy(mnem, t, mlen); mnem[mlen] = 0;
    int ok = 0;
    for (int i = 0; fusable[i]; i++) if (!strcmp(mnem, fusable[i])) { ok = 1; break; }
    if (!ok) return NULL;
    // float ops (F_xxx) take the PF_ prefix (P + F_xxx); the rest take P_.
    const char *pfx = (mnem[0] == 'F' && mnem[1] == '_') ? "P" : "P_";
    const char *rest = t + mlen;             // operand part incl. leading space
    size_t n = strlen(pfx) + mlen + strlen(rest) + 1;
    char *r = malloc(n);
    snprintf(r, n, "%s%s%s", pfx, mnem, rest);
    return r;
}

// `LOD <name>` followed by a unary acc op (NEG/ABS/PST/NRM/I2F/F2I/INV/LIN and
// their F_ variants) reads the operand straight from memory: <op>_M <name>.
// ONLY a memory-NAME operand (not LOD_V, not a literal `LOD 5`) — the _M opcode
// takes a data address, not a constant. Returns fused text, or NULL.
static char *fuse_lod_unary(const char *lod, const char *un)
{
    if (strncmp(lod, "LOD ", 4) != 0) return NULL;     // not a plain LOD (excludes LOD_V)
    const char *opnd = lod + 4;
    if (!((opnd[0] >= 'a' && opnd[0] <= 'z') ||
          (opnd[0] >= 'A' && opnd[0] <= 'Z') || opnd[0] == '_')) return NULL; // literal
    static const struct { const char *acc, *mem; } u[] = {
        {"NEG","NEG_M"},{"F_NEG","F_NEG_M"},{"ABS","ABS_M"},{"F_ABS","F_ABS_M"},
        {"PST","PST_M"},{"F_PST","F_PST_M"},{"NRM","NRM_M"},{"I2F","I2F_M"},
        {"F2I","F2I_M"},{"INV","INV_M"},{"LIN","LIN_M"}, {NULL,NULL} };
    const char *mem = NULL;
    for (int i = 0; u[i].acc; i++) if (!strcmp(un, u[i].acc)) { mem = u[i].mem; break; }
    if (!mem) return NULL;
    size_t n = strlen(mem) + 1 + strlen(opnd) + 1;
    char *r = malloc(n);
    snprintf(r, n, "%s %s", mem, opnd);
    return r;
}

// `@L NOP`, or `@L1 @L2 NOP`: a line whose only instruction is a NOP carrying
// labels. Its labels can ride on the next instruction instead.
static int is_label_only_nop(const char *t)
{
    size_t n = strlen(t);
    if (n < 5 || strcmp(t + n - 4, " NOP")) return 0;
    return t[0] == '@';
}

static void peephole(void)
{
    int w;
    // pass 0: a store immediately followed by a reload of the SAME cell is
    // redundant -- SET leaves the value in the accumulator, so the LOD is a
    // no-op. (Same operand, no label on the LOD.)
    //   SET x;     LOD x      -> SET x
    //   SET_V a k; LOD_V a k  -> SET_V a k
    w = 0;
    for (int r = 0; r < g_ibuf_n; r++) {
        if (w > 0 && !g_ibuf[w-1].nofuse && !g_ibuf[r].nofuse) {
            const char *s = g_ibuf[w-1].text, *l = g_ibuf[r].text;
            int drop = (!strncmp(s,"SET ",4)   && !strncmp(l,"LOD ",4)   && !strcmp(s+4,l+4))
                    || (!strncmp(s,"SET_V ",6) && !strncmp(l,"LOD_V ",6) && !strcmp(s+6,l+6));
            if (drop) { free(g_ibuf[r].text); continue; }   // drop the reload, keep the store
        }
        g_ibuf[w++] = g_ibuf[r];
    }
    g_ibuf_n = w;
    // pass 0b: a method's `this` word that nothing ever reads. A method takes
    // `this` off the stack into <fn>_this, and an expanded accessor stores it
    // there too, but when the body uses it only while it is still in the
    // accumulator (pass 0 dropped that reload) no instruction reads the word:
    // every SET of it is dead, and so is the word. SET leaves the accumulator
    // as it was, so dropping it changes nothing else. Only `this`: a user
    // variable nobody reads still shows in the waveform, `this` never does.
    {
        int nread = 0, cap = 64;
        char **read = malloc(cap * sizeof(char *));
        for (int r = 0; r < g_ibuf_n; r++) {           // every name read anywhere
            const char *t = g_ibuf[r].text;
            int is_set = !g_ibuf[r].nofuse && !strncmp(t, "SET ", 4);
            for (const char *p = t; *p; ) {
                while (*p == ' ' || *p == '\t') p++;
                const char *q = p;
                while (*q && *q != ' ' && *q != '\t') q++;
                size_t len = (size_t)(q - p);
                if (len > 5 && !strncmp(q - 5, "_this", 5) && !(is_set && p == t + 4)) {
                    if (nread == cap) { cap *= 2; read = realloc(read, cap * sizeof(char *)); }
                    char *nm = malloc(len + 1);
                    memcpy(nm, p, len); nm[len] = 0;
                    read[nread++] = nm;
                }
                p = q;
            }
        }
        w = 0;
        for (int r = 0; r < g_ibuf_n; r++) {
            const char *t = g_ibuf[r].text;
            if (!g_ibuf[r].nofuse && !strncmp(t, "SET ", 4)) {
                size_t len = strlen(t + 4);
                int used = 1;
                if (len > 5 && !strcmp(t + 4 + len - 5, "_this")) {
                    used = 0;
                    for (int k = 0; k < nread && !used; k++) used = !strcmp(read[k], t + 4);
                }
                if (!used) { free(g_ibuf[r].text); continue; }
            }
            g_ibuf[w++] = g_ibuf[r];
        }
        g_ibuf_n = w;
        for (int k = 0; k < nread; k++) free(read[k]);
        free(read);
    }
    // pass 1: LOD <name>; <unary> -> <unary>_M <name>. Run first so the new _M
    // op can then be picked up by the PSH pass below (PSH; NEG_M -> P_NEG_M).
    w = 0;
    for (int r = 0; r < g_ibuf_n; r++) {
        if (w > 0 && !g_ibuf[w-1].nofuse && !g_ibuf[r].nofuse) {
            char *fused = fuse_lod_unary(g_ibuf[w-1].text, g_ibuf[r].text);
            if (fused) {
                free(g_ibuf[w-1].text);
                g_ibuf[w-1].text = fused;
                g_ibuf[w-1].line = g_ibuf[r].line;
                free(g_ibuf[r].text);
                continue;
            }
        }
        g_ibuf[w++] = g_ibuf[r];
    }
    g_ibuf_n = w;
    // pass 2: PSH; <load> -> P_<load>   and   SET <x>; POP -> SET_P <x>
    w = 0;
    for (int r = 0; r < g_ibuf_n; r++) {
        if (w > 0 && !g_ibuf[w-1].nofuse && !g_ibuf[r].nofuse) {
            // a bare JMP to the label on the very next line just falls through:
            // drop the JMP, keep the labelled line. (Exact label, not a prefix.)
            if (!strncmp(g_ibuf[w-1].text, "JMP ", 4) && g_ibuf[r].text[0] == '@') {
                const char *tgt = g_ibuf[w-1].text + 4;   // label after "JMP "
                const char *lab = g_ibuf[r].text + 1;     // label after "@"
                size_t tl = strlen(tgt);
                if (!strncmp(tgt, lab, tl) && (lab[tl] == ' ' || lab[tl] == '\0')) {
                    free(g_ibuf[w-1].text);
                    g_ibuf[w-1] = g_ibuf[r];              // JMP dropped; label line takes its slot
                    continue;
                }
            }
            // PSH followed by a load-class op -> the op's P_/PF_ variant
            if (!strcmp(g_ibuf[w-1].text, "PSH") && g_ibuf[r].text[0] != '@') {
                char *fused = fuse_after_psh(g_ibuf[r].text);
                if (fused) {
                    free(g_ibuf[w-1].text);
                    g_ibuf[w-1].text = fused;
                    g_ibuf[w-1].line = g_ibuf[r].line;   // map to the load's source line
                    free(g_ibuf[r].text);
                    continue;
                }
            }
            // a plain SET followed by POP fuses into SET_P (store then pop).
            // Not SET_V (no _V_P opcode); not a labeled POP.
            if (!strcmp(g_ibuf[r].text, "POP") && !strncmp(g_ibuf[w-1].text, "SET ", 4)) {
                const char *opnd = g_ibuf[w-1].text + 3;     // " <name>"
                char *fused = malloc(5 + strlen(opnd) + 1);  // "SET_P" + " <name>"
                sprintf(fused, "SET_P%s", opnd);
                free(g_ibuf[w-1].text);
                g_ibuf[w-1].text = fused;                    // keep the SET's source line
                free(g_ibuf[r].text);
                continue;
            }
        }
        g_ibuf[w++] = g_ibuf[r];
    }
    g_ibuf_n = w;
    // pass 2b: a constant offset added to a pointer just before it is
    // dereferenced rides in the raw base of the LDI / STI instead (the
    // hardware adds the base to the acc / to the stack top):
    //   ADD k; LDI 0        -> LDI k          reads mem[p + k]
    //   ADD k; P_x; STI 0   -> P_x; STI k     the P_ op now pushes p, STI adds k
    // k a non-negative integer literal; no label between, no verbatim asm.
    w = 0;
    for (int r = 0; r < g_ibuf_n; r++) {
        int k; char end;
        if (!g_ibuf[r].nofuse && sscanf(g_ibuf[r].text, "ADD %d%c", &k, &end) == 1 && k >= 0
            && r + 1 < g_ibuf_n && !g_ibuf[r+1].nofuse) {
            const char *n1 = g_ibuf[r+1].text;
            if (!strcmp(n1, "LDI 0")) {
                char *f = malloc(24); snprintf(f, 24, "LDI %d", k);
                free(g_ibuf[r].text); free(g_ibuf[r+1].text);
                g_ibuf[w] = g_ibuf[r+1]; g_ibuf[w].text = f; w++;
                r += 1; continue;
            }
            if (r + 2 < g_ibuf_n && !g_ibuf[r+2].nofuse && n1[0] == 'P' && (n1[1] == '_' || n1[1] == 'F')
                && !strcmp(g_ibuf[r+2].text, "STI 0")) {
                char *f = malloc(24); snprintf(f, 24, "STI %d", k);
                free(g_ibuf[r].text); free(g_ibuf[r+2].text);
                g_ibuf[w++] = g_ibuf[r+1];
                g_ibuf[w] = g_ibuf[r+2]; g_ibuf[w].text = f; w++;
                r += 2; continue;
            }
        }
        g_ibuf[w++] = g_ibuf[r];
    }
    g_ibuf_n = w;
    // pass 3: an instruction that follows an unconditional JMP or a RET, and
    // carries no label, cannot be reached: control either falls through (it
    // cannot, the jump took it away) or arrives at a label (it has none).
    w = 0;
    int dead = 0;
    for (int r = 0; r < g_ibuf_n; r++) {
        const char *t = g_ibuf[r].text;
        if (t[0] == '@' || g_ibuf[r].nofuse) dead = 0;     // a label makes it live again
        if (dead) { free(g_ibuf[r].text); continue; }
        if (!g_ibuf[r].nofuse &&
            (!strncmp(t, "JMP ", 4) || !strcmp(t, "RET"))) dead = 1;
        g_ibuf[w++] = g_ibuf[r];
    }
    g_ibuf_n = w;
    // pass 4: a label rides on the next instruction instead of on a NOP of its
    // own, which is how cmmcomp writes labels (`@main @Lwh1 LOD 1`) and what
    // the assembler expects. Emitting `@L NOP` cost one instruction -- one
    // cycle -- per label, around a tenth of a C++ program. Runs LAST: the
    // passes above match on a bare mnemonic, which a label in front would hide.
    w = 0;
    for (int r = 0; r < g_ibuf_n; r++) {
        // collect a run of label-only NOPs, then hang their labels on whatever
        // real instruction follows (if any, and if it is not verbatim asm)
        int e = r;
        while (e < g_ibuf_n && !g_ibuf[e].nofuse && g_ibuf[e].text[0] == '@' &&
               is_label_only_nop(g_ibuf[e].text)) e++;
        if (e > r && e < g_ibuf_n && !g_ibuf[e].nofuse) {
            size_t n = 1;
            for (int k = r; k <= e; k++) n += strlen(g_ibuf[k].text) + 1;
            char *merged = malloc(n);
            size_t at = 0;
            for (int k = r; k < e; k++) {                 // the labels, NOP dropped
                size_t len = strlen(g_ibuf[k].text) - 4;  // without the trailing " NOP"
                memcpy(merged + at, g_ibuf[k].text, len);
                at += len;
                merged[at++] = ' ';
                free(g_ibuf[k].text);
            }
            size_t ilen = strlen(g_ibuf[e].text);         // then the instruction
            memcpy(merged + at, g_ibuf[e].text, ilen);
            merged[at + ilen] = 0;
            free(g_ibuf[e].text);
            g_ibuf[w] = g_ibuf[e];                        // keep ITS source line
            g_ibuf[w].text = merged;
            w++;
            r = e;
            continue;
        }
        g_ibuf[w++] = g_ibuf[r];
    }
    g_ibuf_n = w;
}

// write the buffer to the .asm and the pc_<proc>_mem.txt (one 20-bit line per
// instruction), setting the final ins_count. Run AFTER peephole().
static void flush_ibuf(FILE *out)
{
    for (int i = 0; i < g_ibuf_n; i++) {
        fputs(g_ibuf[i].text, out); fputc('\n', out);
        if (g_ibuf[i].text[0] != '#') { ins_count++; emit_pc_line(g_ibuf[i].line); }
    }
}

static void free_ibuf(void)
{
    for (int i = 0; i < g_ibuf_n; i++) free(g_ibuf[i].text);
    free(g_ibuf); g_ibuf = NULL; g_ibuf_n = g_ibuf_cap = 0;
}

void codegen(FILE *out_file, unit *u, const char *tmp_dir, const char *src_path)
{
    out_f = out_file;
    ins_count = 0; label_n = 0; varlog_n = 0; has_main = 0; strtab_n = 0; fptab_n = 0;
    g_uses_udiv = 0; g_uses_u2f = 0; g_uses_f2u = 0; n_stinit = 0; g_n_inst = 0;
    g_nubits = (u->nubits >= 0) ? u->nubits : CFG_NUBITS;
    cg_unit = u;

    // pc_<proc>_mem.txt: filled in lockstep with emit() from here on. Open it
    // before emit_header so the leading NOP is logged too. cg_line starts at -1
    // (INTERNAL): all program scaffolding before the first statement maps there.
    cg_line = -1;
    if (tmp_dir && u->prname) {
        char path[2048];
        snprintf(path, sizeof(path), "%s/pc_%s_mem.txt", tmp_dir, u->prname);
        f_line = fopen(path, "w");
    }

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
    instantiate_ctmpl_methods(u);   // clone class-template methods per instance

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

    cg_line = -3;                 // END marker (trad_cmm.txt row -3)
    emit("@fim JMP fim");
    cg_line = -1;                 // post-@fim helpers are INTERNAL scaffolding

    // template instances (also reached only via CAL); the loop bound grows as
    // emitting one instance discovers calls to further template instances
    for (int i = 0; i < g_n_inst; i++) emit_function(g_inst[i], u, 0);

    // helpers: after @fim so main falls through into the halt loop, not a
    // helper (they are only ever entered through CAL), and after the template
    // instances so a helper that only an instance uses is not missed.
    if (g_uses_udiv) emit_udivmod();
    if (g_uses_heap) emit_heap();
    if (g_uses_u2f)  emit_u2f();
    if (g_uses_f2u)  emit_f2u();

    // all instructions buffered: fuse, then write the .asm + pc_mem (ins_count
    // is finalised here, after fusion, so num_ins matches the emitted program).
    peephole();
    flush_ibuf(out_file);
    free_ibuf();

    if (f_line) { fclose(f_line); f_line = NULL; }

    if (tmp_dir) write_cmm_log(tmp_dir);
    if (tmp_dir && src_path) write_trad_cmm(tmp_dir, src_path);
}
