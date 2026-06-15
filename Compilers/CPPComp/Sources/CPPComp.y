/* ----------------------------------------------------------------------------
   CPPComp — bison grammar for the YANC C subset
   ----------------------------------------------------------------------------
   Grammar is loosely C99, restricted to constructs the target can execute:
     - scalar types int/float/char/void, pointers, 1-D arrays, structs
     - all expression operators, all looping/conditional constructs
     - functions with array/pointer params (passed via LEA + LDA/STA)
     - typedef, enum, const, static (storage class is mostly cosmetic here)
   What's intentionally omitted from v1: function pointers, variadic args,
   unions, multi-D arrays, VLAs.
   ---------------------------------------------------------------------------- */

%{
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../Headers/ast.h"
#include "../Headers/types.h"
#include "../Headers/symtab.h"
#include "../Headers/messages.h"

int   yylex  (void);
extern int   yylineno;
void  yyerror(const char *s);

unit *g_unit = NULL;    // populated as we parse, used by lexer for #pragma yanc
int   g_str_len = 0;    // set by lexer alongside yylval.sval for STRING_LIT

// staging state for declarations and functions ------------------------------

static int            ts_typedef = 0;       // we're inside a `typedef ...;`
static storage_class  ts_sclass  = SC_NONE;
static int            ts_is_const = 0;

// staged function params (added to symtab on body entry)
static decl  *param_head = NULL;
static decl  *param_tail = NULL;
static int    param_count = 0;

static void unit_add_func(func *f);   // defined below; used by method_finish

// signature string (one type-code per param) for overload disambiguation
static char *params_sig(decl *params)
{
    int n = 0; for (decl *p = params; p; p = p->next) n++;
    char *s = malloc(n + 1); int i = 0;
    for (decl *p = params; p; p = p->next) s[i++] = type_code(p->dtype);
    s[i] = 0; return s;
}
static char *sym_sig(sym *s)
{
    char *str = malloc(s->n_params + 1);
    for (int i = 0; i < s->n_params; i++) str[i] = type_code(s->param_types[i]);
    str[s->n_params] = 0; return str;
}
static void params_reset(void) { param_head = param_tail = NULL; param_count = 0; }
static void params_append(decl *d)
{
    if (!param_head) param_head = d; else param_tail->next = d;
    param_tail = d; param_count++;
}

// staged "current base type" used by declarators after `base_type` is reduced
static type *cur_base = NULL;
// stack of structs/unions currently being assembled (supports nested inline defs)
static type *cur_struct_stk[16];
static int   cur_struct_sp = 0;
static type *cur_struct = NULL;            // == top of the stack (or NULL)
static void cur_struct_push(type *t) { cur_struct_stk[cur_struct_sp++] = cur_struct; cur_struct = t; }
static void cur_struct_pop (void)    { cur_struct = cur_struct_stk[--cur_struct_sp]; }
// the class whose body we are currently parsing (for `this` + method mangling)
static type *cur_class = NULL;
// constructor member-initializer list, lowered to `member = expr` statements
// prepended to the ctor body
static stmt *g_ctor_inits[32];
static int   g_n_ctor_inits = 0;
// mangle a method name: Class::name -> "Class__name" (asm-level symbol)
static char *mangle_method(const char *cls, const char *name)
{
    char *m = malloc(strlen(cls) + strlen(name) + 3);
    sprintf(m, "%s__%s", cls, name);
    return m;
}

// Function-template names. The lexer returns TEMPLATE_NAME (not IDENT) for these
// so the parser can accept explicit template arguments `name<T...>(args)` without
// the `<` colliding with the comparison operator (a template name is never the
// operand of `<`). Registered when a function template is defined.
static char *g_tmpl_names[256];
static int   g_n_tmpl_names = 0;
static void tmpl_name_register(const char *n) {
    for (int i = 0; i < g_n_tmpl_names; i++) if (!strcmp(g_tmpl_names[i], n)) return;
    if (g_n_tmpl_names < 256) g_tmpl_names[g_n_tmpl_names++] = strdup(n);
}
int tmpl_name_is(const char *n) {   // consulted by the lexer (classify_ident)
    for (int i = 0; i < g_n_tmpl_names; i++) if (!strcmp(g_tmpl_names[i], n)) return 1;
    return 0;
}

// a unary operator overload shares the token of a binary one ('-' / '+'); a
// zero-parameter operator-/operator+ is the unary form, given a distinct method
// name (op_neg / op_pos) so it does not collide with the binary overload.
static const char *op_arity_name(const char *name, int nuser_params)
{
    if (nuser_params == 0) {
        if (!strcmp(name, "op_sub")) return "op_neg";
        if (!strcmp(name, "op_add")) return "op_pos";
    }
    return name;
}

// register an in-class method declaration (prototype only, no body). Mirrors
// the free-function prototype path: stashes param types AND default-arg
// expressions on the sym so a later out-of-class definition can replay them.
// Does NOT mark the sym defined and does NOT emit a func.
static void register_method_decl(type *ret, const char *name, decl *params, int nparams)
{
    char *mname = mangle_method(cur_class->tag, name);
    sym *s = st_find(mname);
    if (!s) s = st_add(SK_FUNC, mname, mname, ret);
    s->n_params = nparams + 1;
    s->param_types = malloc(sizeof(type*) * s->n_params);
    s->param_types[0] = t_ptr(cur_class);
    { int i = 1; for (decl *p = params; p; p = p->next, i++) s->param_types[i] = p->dtype; }
    s->param_defaults = malloc(sizeof(expr*) * s->n_params);
    s->param_defaults[0] = NULL;   /* `this` has no default */
    { int i = 1; for (decl *p = params; p; p = p->next, i++) s->param_defaults[i] = p->init; }
}

// replay default args recorded by a prior in-class declaration onto the
// out-of-class definition's params (which by C++ rules cannot repeat them).
// Index 0 in s->param_defaults is the implicit `this`; user params start at 1.
static void replay_method_defaults(const char *cls_tag, const char *name, decl *params)
{
    char *mname = mangle_method(cls_tag, name);
    sym *s = st_find(mname);
    free(mname);
    if (!s || !s->param_defaults) return;
    decl *p = params; int i = 1;
    for (; p && i < s->n_params; p = p->next, i++) {
        if (!p->init && s->param_defaults[i]) p->init = s->param_defaults[i];
    }
}

// ---- lambda IIFE: a no-capture, no-param lambda immediately invoked,
// `[]() { body }()`, used as a global initializer (the C++14 idiom for
// computing a const table). The body becomes a synthesized free function
// `lam_N` and the whole expression lowers to the plain call `lam_N()`.
// Function-parse state does not nest (st_enter_func holds ONE cur_func),
// so a lambda inside a function body is rejected with a clear message.
static int  lambda_seq = 0;
static char lambda_name[32];
/* the enclosing declaration's specifier flags are LIVE while its initializer
   parses; the lambda body has its own declarations, so the flags must be
   cleared on entry (like function_def does) and RESTORED on exit — the outer
   declaration still reads them after the initializer. The outer `const` would
   otherwise leak onto body locals, publishing `int k = 1` as a foldable
   SK_ENUM_CONST and turning `++k` into `++1` (not an lvalue). */
static int           lambda_sv_const, lambda_sv_typedef;
static storage_class lambda_sv_sclass;
static type         *lambda_sv_base;

static void lambda_enter(void)
{
    if (st_current_func())
        msg_error(yylineno, "a lambda is only supported as a global initializer"
                            " (immediately-invoked, no captures)");
    snprintf(lambda_name, sizeof lambda_name, "lam_%d", lambda_seq++);
    sym *s = st_add(SK_FUNC, lambda_name, lambda_name, t_int());
    s->defined = 1;
    st_enter_func(lambda_name);
    st_push_scope();
    lambda_sv_const   = ts_is_const;  ts_is_const = 0;
    lambda_sv_typedef = ts_typedef;   ts_typedef  = 0;
    lambda_sv_sclass  = ts_sclass;    ts_sclass   = SC_NONE;
    lambda_sv_base    = cur_base;
}

// return-type deduction: resolve `return <ident>` against the body's own
// declarations (the accumulate-and-return idiom), or take a literal's type.
static type *lambda_find_decl(stmt *s, const char *name)
{
    if (!s) return NULL;
    if (s->kind == S_DECL)
        for (decl *d = s->decls; d; d = d->next)
            if (d->name && strcmp(d->name, name) == 0) return d->dtype;
    type *t;
    if ((t = lambda_find_decl(s->body, name)))      return t;
    if ((t = lambda_find_decl(s->body2, name)))     return t;
    if ((t = lambda_find_decl(s->init_stmt, name))) return t;
    for (int i = 0; i < s->n_items; i++)
        if ((t = lambda_find_decl(s->items[i], name))) return t;
    return NULL;
}

static expr *lambda_find_return(stmt *s)
{
    if (!s) return NULL;
    if (s->kind == S_RETURN) return s->e1;
    expr *e;
    if ((e = lambda_find_return(s->body)))      return e;
    if ((e = lambda_find_return(s->body2)))     return e;
    if ((e = lambda_find_return(s->init_stmt))) return e;
    for (int i = 0; i < s->n_items; i++)
        if ((e = lambda_find_return(s->items[i]))) return e;
    return NULL;
}

static expr *lambda_finish(stmt *body)
{
    expr *r = lambda_find_return(body);
    type *ret = NULL;
    if (r) {
        if      (r->kind == E_IDENT)     ret = lambda_find_decl(body, r->sval);
        else if (r->kind == E_INT_LIT)   ret = t_int();
        else if (r->kind == E_FLOAT_LIT) ret = t_float();
    }
    if (!ret)
        msg_error(yylineno, "cannot deduce the lambda's return type"
                            " (return a local variable or a literal)");
    sym *s = st_find(lambda_name);
    if (s) s->stype = ret;
    func *f = ast_func(ret, strdup(lambda_name), NULL, 0, body, yylineno);
    unit_add_func(f);
    st_pop_scope();
    st_leave_func();
    ts_is_const = lambda_sv_const;
    ts_typedef  = lambda_sv_typedef;
    ts_sclass   = lambda_sv_sclass;
    cur_base    = lambda_sv_base;
    return ast_call(ast_ident(strdup(lambda_name), yylineno), NULL, 0, yylineno);
}

// open a method's scope: register the mangled symbol, inject `this` as param 0,
// stage the declared params. Called from method_def's mid-rule action.
static void method_enter(type *ret, const char *name, decl *params, int nparams)
{
    char *mname = mangle_method(cur_class->tag, name);
    sym *s = st_find(mname);
    if (!s) s = st_add(SK_FUNC, mname, mname, ret);
    s->defined = 1;
    /* param_types = [this, declared...] so call sites can spot reference params */
    s->n_params = nparams + 1;
    s->param_types = malloc(sizeof(type*) * s->n_params);
    s->param_types[0] = t_ptr(cur_class);
    { int i = 1; for (decl *p = params; p; p = p->next, i++) s->param_types[i] = p->dtype; }
    st_enter_func(mname);
    st_push_scope();
    decl *thisd = ast_decl(t_ptr(cur_class), strdup("this"), NULL, 0);
    thisd->next = params;
    st_add(SK_PARAM, "this", NULL, t_ptr(cur_class));
    for (decl *p = params; p; p = p->next) st_add(SK_PARAM, p->name, NULL, p->dtype);
    param_head = thisd; param_tail = NULL; param_count = nparams + 1;
    ts_sclass = SC_NONE; ts_is_const = 0;
}

// desugar `for (T x : arr) body` over a fixed-size array into an index loop:
//   for (int __ri = 0; __ri < N; __ri = __ri+1) { T x = arr[__ri]; body }
// `vt` may be `auto` (deduce element type) or a reference (bind to each element).
static stmt *make_range_for(type *vt, char *var, char *container, stmt *body, int line)
{
    static int rc = 0;
    sym *cs = st_find(container);
    int   n    = (cs && cs->stype && cs->stype->kind == TY_ARRAY) ? cs->stype->arr_size : 0;
    type *elem = (cs && cs->stype && cs->stype->kind == TY_ARRAY) ? cs->stype->base : t_int();
    if (vt->is_auto)                       vt = elem;            // auto x
    else if (vt->is_ref && vt->base && vt->base->is_auto) vt = t_ref(elem);  // auto& x
    char ivar[24]; sprintf(ivar, "__ri%d", rc++);

    decl *id = ast_decl(t_int(), strdup(ivar), ast_int_lit(0, line), line);
    stmt *init = ast_stmt(S_DECL, line); init->decls = id;
    expr *cond = ast_binop(OP_LT, ast_ident(strdup(ivar), line), ast_int_lit(n, line), line);
    expr *step = ast_assign(ast_ident(strdup(ivar), line),
                    ast_binop(OP_ADD, ast_ident(strdup(ivar), line), ast_int_lit(1, line), line), line);
    decl *vd = ast_decl(vt, var,
                    ast_index(ast_ident(strdup(container), line), ast_ident(strdup(ivar), line), line), line);
    stmt *vdecl = ast_stmt(S_DECL, line); vdecl->decls = vd;
    stmt *blk = ast_stmt(S_BLOCK, line);
    blk->items = malloc(sizeof(stmt*) * 2); blk->items[0] = vdecl; blk->items[1] = body; blk->n_items = 2;
    stmt *f = ast_stmt(S_FOR, line);
    f->init_stmt = init; f->e1 = cond; f->e2 = step; f->body = blk;
    return f;
}

// For each field with a default member initializer that the ctor's member-init
// list does NOT already cover, append a synthesized `field = dinit;` statement
// to g_ctor_inits. Called from each ctor finalization (in-class and out-of-class)
// after ctor_init_opt populated g_ctor_inits and before it gets prepended to
// the body. Scalar inits land here; aggregate `= {}` defaults are dropped during
// parsing for now (see [[cppcomp-blind-port]]).
static void inject_default_member_inits(type *cls)
{
    if (!cls || cls->kind != TY_STRUCT) return;
    for (strct_field *f = cls->fields; f; f = f->next) {
        if (!f->dinit && !f->dzero) continue;
        int already = 0;
        for (int i = 0; i < g_n_ctor_inits; i++) {
            stmt *s = g_ctor_inits[i];
            if (s->kind == S_EXPR && s->e1 && s->e1->kind == E_ASSIGN &&
                s->e1->a && s->e1->a->kind == E_IDENT &&
                strcmp(s->e1->a->sval, f->name) == 0) {
                already = 1; break;
            }
        }
        if (already) continue;
        stmt *s = ast_stmt(S_EXPR, yylineno);
        if (f->dinit) {
            s->e1 = ast_assign(ast_ident(strdup(f->name), yylineno), (expr*)f->dinit, yylineno);
        } else {
            /* aggregate `= {}`: synthesize `__zerofill(&field, N)` (a codegen
               builtin) so the ctor zero-fills the field's N words at runtime. */
            int nwords = type_size_words(f->ftype);
            expr **args = malloc(sizeof(expr*) * 2);
            args[0] = ast_ident(strdup(f->name), yylineno);   /* codegen does gen_addr -> &field */
            args[1] = ast_int_lit(nwords, yylineno);
            s->e1 = ast_call(ast_ident(strdup("__zerofill"), yylineno), args, 2, yylineno);
        }
        if (g_n_ctor_inits < 32) g_ctor_inits[g_n_ctor_inits++] = s;
    }
}

// a copy constructor is a ctor taking exactly one parameter that is a reference
// to its own class -> mangle it distinctly as "copyctor" (so it can coexist
// with a regular "ctor"); otherwise it's the normal "ctor".
static const char *ctor_kind(decl *params)
{
    if (params && !params->next && params->dtype && params->dtype->is_ref
        && params->dtype->base == cur_class)
        return "copyctor";
    return "ctor";
}

// record a static data-member name on the class (resolved as a shared global)
static void add_static(type *cls, const char *name)
{
    if (!cls) return;
    cls->statics = realloc(cls->statics, sizeof(char*) * (cls->n_statics + 1));
    cls->statics[cls->n_statics++] = strdup(name);
}

// register a virtual method name into the class's vtable (override reuses slot)
static void add_vmethod(type *cls, const char *name)
{
    if (!cls) return;
    for (int i = 0; i < cls->n_vtbl; i++) if (!strcmp(cls->vtbl[i], name)) return;
    cls->vtbl = realloc(cls->vtbl, sizeof(char*) * (cls->n_vtbl + 1));
    cls->vtbl[cls->n_vtbl++] = strdup(name);
}

// function-template capture: while parsing `template<...> ret f(...)`, type
// parameters are t_tparam placeholders and the function is captured (not emitted)
static int   g_in_template = 0;
static int   g_tparam_n = 0;
// non-type template parameter tracking (for real class-template monomorphization)
static int   g_tparam_isval[16];   // per param: 1 = non-type (value) parameter
static int   g_ct_nontype = 0;     // current template has >=1 non-type parameter
static func *g_ctm[128];           // methods collected for the current class template
static int   g_n_ctm = 0;

// finalize a method into a free function tagged with its class.
static void method_finish(type *ret, const char *name, stmt *body)
{
    func *f = ast_func(ret, mangle_method(cur_class->tag, name), param_head, param_count, body, yylineno);
    f->method_of = cur_class;
    // class-template methods are captured for real monomorphization (cloned per
    // instantiation Name<args>). A template with a non-type parameter cannot also
    // be emitted type-erased (its fields carry sentinel array sizes); a pure
    // type-parameter template is ALSO emitted erased so a bare use `Vector v;`
    // (no explicit <T>) keeps working as before.
    if (g_in_template) {
        if (g_n_ctm < 128) g_ctm[g_n_ctm++] = f;
        if (!g_ct_nontype) unit_add_func(f);
    } else {
        unit_add_func(f);
    }
    st_pop_scope();
    st_leave_func();
    params_reset();
}

// open a STATIC method's scope: like method_enter but WITHOUT injecting `this`.
// A static method is conceptually a free function whose name happens to live
// in the class's namespace; it has no per-instance state. We lower it to a
// free function with mangled name `Class__name` and no implicit first arg.
static void static_method_enter(type *ret, const char *name, decl *params, int nparams)
{
    char *mname = mangle_method(cur_class->tag, name);
    sym *s = st_find(mname);
    if (!s) s = st_add(SK_FUNC, mname, mname, ret);
    s->defined = 1;
    s->n_params = nparams;
    if (nparams > 0) {
        s->param_types = malloc(sizeof(type*) * nparams);
        int i = 0;
        for (decl *p = params; p; p = p->next, i++) s->param_types[i] = p->dtype;
    }
    st_enter_func(mname);
    st_push_scope();
    for (decl *p = params; p; p = p->next) st_add(SK_PARAM, p->name, NULL, p->dtype);
    param_head = params; param_tail = NULL; param_count = nparams;
    ts_sclass = SC_NONE; ts_is_const = 0;
}

// finalize a static method into a free function. method_of stays NULL so
// codegen routes calls via the bare-name path (no implicit `this`).
static void static_method_finish(type *ret, const char *name, stmt *body)
{
    func *f = ast_func(ret, mangle_method(cur_class->tag, name), param_head, param_count, body, yylineno);
    /* method_of intentionally NULL: this is a static -- looks/acts like a free fn */
    if (g_in_template) {
        if (g_n_ctm < 128) g_ctm[g_n_ctm++] = f;
        if (!g_ct_nontype) unit_add_func(f);
    } else {
        unit_add_func(f);
    }
    st_pop_scope();
    st_leave_func();
    params_reset();
}

// enum counter (reset per enum_specifier)
static long  enum_counter = 0;
static char *cur_enum_prefix = NULL;   // set for `enum class E` -> enumerators are E__name

// helpers --------------------------------------------------------------------

static char *xstrdup(const char *s) { char *r = malloc(strlen(s)+1); strcpy(r,s); return r; }

static void unit_add_global(decl *d)
{
    if (!g_unit) msg_internal("unit not allocated");
    g_unit->globals = realloc(g_unit->globals, sizeof(*g_unit->globals) * (g_unit->n_globals + 1));
    g_unit->globals[g_unit->n_globals++] = d;
}

static void unit_add_func(func *f)
{
    g_unit->funcs = realloc(g_unit->funcs, sizeof(*g_unit->funcs) * (g_unit->n_funcs + 1));
    g_unit->funcs[g_unit->n_funcs++] = f;
}

static void unit_add_template(func *f)
{
    f->n_tparams = g_tparam_n;
    g_unit->templates = realloc(g_unit->templates, sizeof(func*) * (g_unit->n_templates + 1));
    g_unit->templates[g_unit->n_templates++] = f;
}

// register a class template (with non-type parameters) for real monomorphization
static void unit_add_ctmpl(ctmpl *c)
{
    g_unit->ctmpls = realloc(g_unit->ctmpls, sizeof(ctmpl*) * (g_unit->n_ctmpls + 1));
    g_unit->ctmpls[g_unit->n_ctmpls++] = c;
}

static ctmpl *find_ctmpl(const char *name)
{
    for (int i = 0; g_unit && i < g_unit->n_ctmpls; i++)
        if (!strcmp(g_unit->ctmpls[i]->name, name)) return g_unit->ctmpls[i];
    return NULL;
}

// replace a sentinel (non-type-parameter) array length with its concrete value
static type *subst_field_type(type *t, int *vals, int nvals)
{
    if (!t) return t;
    if (t->kind == TY_ARRAY && t->arr_size >= NTP_BASE) {
        int k = t->arr_size - NTP_BASE;
        int n = (k < nvals) ? vals[k] : 0;
        return t_array(subst_field_type(t->base, vals, nvals), n);
    }
    return t;
}

// instantiate `Name<vals...>` to a concrete class type (built once, then reused);
// records a codegen request so the template's methods get cloned + emitted.
static type *instantiate_ctmpl(ctmpl *ct, int *vals, int nvals)
{
    char suffix[32]; int p = 0;
    p += snprintf(suffix + p, sizeof(suffix) - p, "_T");
    for (int i = 0; i < nvals; i++) p += snprintf(suffix + p, sizeof(suffix) - p, "%s%d", i ? "_" : "", vals[i]);
    char tag[128]; snprintf(tag, sizeof(tag), "%s%s", ct->name, suffix);

    sym *ex = st_find(tag);
    if (ex && ex->kind == SK_TYPEDEF) return ex->stype;   // already instantiated

    type *c = t_make_struct(xstrdup(tag));
    c->base_class = ct->proto->base_class;
    c->n_vtbl = ct->proto->n_vtbl;
    if (c->n_vtbl) { c->vtbl = malloc(sizeof(char*) * c->n_vtbl);
                     for (int i = 0; i < c->n_vtbl; i++) c->vtbl[i] = xstrdup(ct->proto->vtbl[i]); }
    for (strct_field *f = ct->proto->fields; f; f = f->next)
        t_struct_add_field(c, f->name, subst_field_type(f->ftype, vals, nvals));
    t_struct_seal(c, g_unit && g_unit->nubits > 0 ? g_unit->nubits : 16);
    st_add_tag(xstrdup(tag), c);
    st_add_typedef(xstrdup(tag), c);

    ctinst *ins = calloc(1, sizeof(ctinst));
    ins->tmpl = ct; ins->nvals = nvals; ins->concrete = c; ins->suffix = xstrdup(suffix);
    for (int i = 0; i < nvals && i < 8; i++) ins->vals[i] = vals[i];
    g_unit->ctinsts = realloc(g_unit->ctinsts, sizeof(ctinst*) * (g_unit->n_ctinsts + 1));
    g_unit->ctinsts[g_unit->n_ctinsts++] = ins;
    return c;
}

// substitute type parameters (t_tparam, carried as tparam>0) with their concrete
// bindings, recursing through pointer/array/reference wrappers.
static type *subst_tparam_type(type *t, type **targs, int ntargs)
{
    if (!t) return t;
    if (t->tparam > 0 && t->tparam <= ntargs) return targs[t->tparam - 1];
    if (t->kind == TY_PTR) {
        type *nb = subst_tparam_type(t->base, targs, ntargs);
        if (nb == t->base) return t;
        type *nt = t_ptr(nb); nt->is_ref = t->is_ref; return nt;
    }
    if (t->kind == TY_ARRAY) {
        type *nb = subst_tparam_type(t->base, targs, ntargs);
        if (nb == t->base) return t;
        return t_array(nb, t->arr_size);
    }
    return t;
}

// one-letter code per concrete type, for the instance tag suffix (must be stable
// per type so call sites and emitted methods agree on the mangled name).
static char ct_type_code(type *t)
{
    if (!t) return 'i';
    if (t->kind == TY_FLOAT) return 'f';
    if (t->kind == TY_PTR || t->kind == TY_ARRAY) return 'p';
    if (t->kind == TY_STRUCT) return 'S';
    return 'i';
}

// instantiate `Name<args...>` where each arg is either a type (targs[k] != NULL,
// isval[k] == 0) or a non-type value (vals[k], isval[k] == 1). Subsumes the
// pure-type and pure-value paths: subst_tparam_type substitutes the type-param
// placeholders first, then subst_field_type rewrites the value-sentinels in
// array sizes. The two passes are orthogonal on the proto's field types.
static type *instantiate_ctmpl_mixed(ctmpl *ct, type **targs, int *vals, int *isval, int n)
{
    char suffix[64]; int p = 0;
    p += snprintf(suffix + p, sizeof(suffix) - p, "_T");
    for (int i = 0; i < n && p < (int)sizeof(suffix) - 16; i++) {
        if (isval[i]) p += snprintf(suffix + p, sizeof(suffix) - p, "%s%d", i ? "_" : "", vals[i]);
        else          suffix[p++] = ct_type_code(targs[i]);
    }
    suffix[p] = 0;
    char tag[128]; snprintf(tag, sizeof(tag), "%s%s", ct->name, suffix);

    sym *ex = st_find(tag);
    if (ex && ex->kind == SK_TYPEDEF) return ex->stype;   // already instantiated

    type *c = t_make_struct(xstrdup(tag));
    c->base_class = ct->proto->base_class;
    c->n_vtbl = ct->proto->n_vtbl;
    if (c->n_vtbl) { c->vtbl = malloc(sizeof(char*) * c->n_vtbl);
                     for (int i = 0; i < c->n_vtbl; i++) c->vtbl[i] = xstrdup(ct->proto->vtbl[i]); }
    for (strct_field *f = ct->proto->fields; f; f = f->next) {
        type *ft = f->ftype;
        ft = subst_tparam_type(ft, targs, n);   // type-param placeholders -> concrete types
        ft = subst_field_type (ft, vals,  n);   // NTP_BASE sentinels in arr_size -> concrete ints
        t_struct_add_field(c, f->name, ft);
    }
    t_struct_seal(c, g_unit && g_unit->nubits > 0 ? g_unit->nubits : 16);
    st_add_tag(xstrdup(tag), c);
    st_add_typedef(xstrdup(tag), c);

    ctinst *ins = calloc(1, sizeof(ctinst));
    ins->tmpl = ct; ins->concrete = c; ins->suffix = xstrdup(suffix);
    ins->nvals  = n; ins->ntargs = n;          // total parameter count; isval[] distinguishes
    for (int i = 0; i < n && i < 8; i++) {
        ins->vals[i]  = isval[i] ? vals[i]  : 0;
        ins->targs[i] = isval[i] ? NULL     : targs[i];
    }
    g_unit->ctinsts = realloc(g_unit->ctinsts, sizeof(ctinst*) * (g_unit->n_ctinsts + 1));
    g_unit->ctinsts[g_unit->n_ctinsts++] = ins;
    return c;
}

// instantiate `Name<types...>` to a concrete class type: substitute the type
// parameters into the proto's field types and record a codegen request so the
// template's methods get cloned with the same bindings.
static type *instantiate_ctmpl_t(ctmpl *ct, type **targs, int n)
{
    char suffix[32]; int p = 0;
    p += snprintf(suffix + p, sizeof(suffix) - p, "_T");
    for (int i = 0; i < n && p < (int)sizeof(suffix) - 1; i++) suffix[p++] = ct_type_code(targs[i]);
    suffix[p] = 0;
    char tag[128]; snprintf(tag, sizeof(tag), "%s%s", ct->name, suffix);

    sym *ex = st_find(tag);
    if (ex && ex->kind == SK_TYPEDEF) return ex->stype;   // already instantiated

    type *c = t_make_struct(xstrdup(tag));
    c->base_class = ct->proto->base_class;
    c->n_vtbl = ct->proto->n_vtbl;
    if (c->n_vtbl) { c->vtbl = malloc(sizeof(char*) * c->n_vtbl);
                     for (int i = 0; i < c->n_vtbl; i++) c->vtbl[i] = xstrdup(ct->proto->vtbl[i]); }
    for (strct_field *f = ct->proto->fields; f; f = f->next)
        t_struct_add_field(c, f->name, subst_tparam_type(f->ftype, targs, n));
    t_struct_seal(c, g_unit && g_unit->nubits > 0 ? g_unit->nubits : 16);
    st_add_tag(xstrdup(tag), c);
    st_add_typedef(xstrdup(tag), c);

    ctinst *ins = calloc(1, sizeof(ctinst));
    ins->tmpl = ct; ins->nvals = 0; ins->concrete = c; ins->suffix = xstrdup(suffix);
    ins->ntargs = n;
    for (int i = 0; i < n && i < 8; i++) ins->targs[i] = targs[i];
    g_unit->ctinsts = realloc(g_unit->ctinsts, sizeof(ctinst*) * (g_unit->n_ctinsts + 1));
    g_unit->ctinsts[g_unit->n_ctinsts++] = ins;
    return c;
}

// wrap base type with N levels of pointer
static type *apply_pointers(type *base, int stars) { while (stars-- > 0) base = t_ptr(base); return base; }

// build a 1-argument call `fn(arg)` — used to desugar new/delete to malloc/free
static expr *mk_call1(const char *fn, expr *arg, int line)
{
    expr **args = malloc(sizeof(expr*));
    args[0] = arg;
    return ast_call(ast_ident(strdup(fn), line), args, 1, line);
}

// wrap base type with the given array dimensions, innermost-last (row-major):
// dims [3,4] over int  ->  array(3, array(4, int))
static type *build_array_type(type *base, const int *dims, int n)
{
    type *t = base;
    for (int i = n - 1; i >= 0; i--) t = t_array(t, dims[i]);
    return t;
}

// process a fully-typed declarator and produce a decl node
static decl *make_decl(type *base, int stars, char *name, const int *dims, int ndims, int line, expr *init)
{
    type *t = apply_pointers(base, stars);
    t = build_array_type(t, dims, ndims);
    decl *d = ast_decl(t, name, init, line);
    d->sclass = ts_sclass;
    // `const T *p` qualifies the pointee, not the pointer — the variable stays
    // writable (pointee-constness isn't enforced on this single-word target).
    // `const T x` / `const T a[]` still marks the object itself const.
    d->is_const = (stars > 0) ? 0 : ts_is_const;
    return d;
}

// builtin type-specifier accumulator -----------------------------------------
// C lets the keywords appear in any multiplicity/order (e.g. `long unsigned
// int`, `unsigned long long`). We OR a flag per keyword (counting `long`),
// then collapse to the few storage types the YANC actually has: every signed
// integer -> one int word, every unsigned integer -> one uint word, every
// floating type -> the one float word. char keeps its own (signed) word.
enum {
    TS_VOID   = 1<<0, TS_CHAR  = 1<<1, TS_INT    = 1<<2, TS_FLOAT = 1<<3,
    TS_DOUBLE = 1<<4, TS_SIGN  = 1<<5, TS_UNSIGN = 1<<6, TS_SHORT = 1<<7,
    TS_BOOL   = 1<<8, TS_LONG1 = 1<<9, TS_LONG2  = 1<<10,
    TS_LONG   = 1<<11   // sentinel returned by the `long` spec; folded by ts_add
};

static int ts_add(int acc, int spec)
{
    if (spec == TS_LONG)
        return acc | ((acc & TS_LONG1) ? TS_LONG2 : TS_LONG1);
    return acc | spec;
}

static type *resolve_builtin(int f)
{
    if (f & TS_VOID)                 return t_void();
    if (f & (TS_FLOAT | TS_DOUBLE))  return t_float();   // float/double/long double
    if (f & TS_BOOL)                 return t_uint();
    if (f & TS_CHAR)                 return (f & TS_UNSIGN) ? t_uint() : t_char();
    return (f & TS_UNSIGN) ? t_uint() : t_int();         // short/int/long/long long
}

// compile-time integer constant evaluator (for _Static_assert). Handles int/
// char literals, sizeof(type) (= words on this target), enum constants, casts,
// and the integer operators. Returns 0 if the expression isn't a constant.
static int const_eval(expr *e, long *out)
{
    if (!e) return 0;
    long a, b;
    switch (e->kind) {
    case E_INT_LIT: case E_CHAR_LIT: *out = e->ival; return 1;
    case E_SIZEOF_T: *out = type_size_words(e->target_t); return 1;
    case E_CAST:     return const_eval(e->a, out);
    case E_IDENT: {
        sym *s = st_find(e->sval);
        if (s && s->kind == SK_ENUM_CONST) { *out = s->enum_val; return 1; }
        /* class-qualified fallback: inside a class body, a bare identifier
           may refer to a `static const`/`static constexpr` member that was
           registered under its mangled name `Class__name`. */
        if (cur_class && cur_class->tag) {
            char *mname = mangle_method(cur_class->tag, e->sval);
            s = st_find(mname);
            free(mname);
            if (s && s->kind == SK_ENUM_CONST) { *out = s->enum_val; return 1; }
        }
        return 0;
    }
    case E_UNOP:
        if (!const_eval(e->a, &a)) return 0;
        switch (e->op) {
        case OP_NEG: *out = -a; return 1;
        case OP_POS: *out =  a; return 1;
        case OP_BNOT:*out = ~a; return 1;
        case OP_LNOT:*out = !a; return 1;
        default: return 0;
        }
    case E_TERNARY:
        if (!const_eval(e->a, &a)) return 0;
        return const_eval(a ? e->b : e->c, out);
    case E_BINOP:
        if (!const_eval(e->a, &a) || !const_eval(e->b, &b)) return 0;
        switch (e->op) {
        case OP_ADD: *out = a + b; return 1;
        case OP_SUB: *out = a - b; return 1;
        case OP_MUL: *out = a * b; return 1;
        case OP_DIV: *out = b ? a / b : 0; return 1;
        case OP_MOD: *out = b ? a % b : 0; return 1;
        case OP_BAND:*out = a & b; return 1;
        case OP_BOR: *out = a | b; return 1;
        case OP_BXOR:*out = a ^ b; return 1;
        case OP_SHL: *out = a << b; return 1;
        case OP_SHR: *out = a >> b; return 1;
        case OP_EQ:  *out = (a == b); return 1;
        case OP_NE:  *out = (a != b); return 1;
        case OP_LT:  *out = (a <  b); return 1;
        case OP_GT:  *out = (a >  b); return 1;
        case OP_LE:  *out = (a <= b); return 1;
        case OP_GE:  *out = (a >= b); return 1;
        case OP_LAND:*out = (a && b); return 1;
        case OP_LOR: *out = (a || b); return 1;
        default: return 0;
        }
    default: return 0;
    }
}

static void check_static_assert(expr *cond, const char *msg, int line)
{
    long v;
    if (!const_eval(cond, &v))
        msg_error(line, "_Static_assert requires a constant integer expression");
    if (v == 0)
        msg_error(line, "static assertion failed%s%s", msg ? ": " : "", msg ? msg : "");
}
%}

%union {
    long    ival;
    double  fval;
    char   *sval;
    type   *typ;
    expr   *exp;
    stmt   *st;
    decl   *dcl;
    func   *fn;
    int     intval;
    op_kind opkind;
    struct { expr **arr; int n; } elist;
    initz *iz;
    struct { initz *z; init_desig d; } iitem;
    struct { type *t; expr *e; } gassoc;
    struct { type **ts; expr **es; int n; } galist;
    struct { stmt **arr; int n; } slist;
    struct { decl  *head; int n; } dlist;
    struct { int dims[8]; int n; } dimlist;
    struct { type **arr; int n; } talist;
    /* mixed template-arg list: each slot is either a type (targs[k] != NULL,
       isval[k] == 0) or a value (targs[k] == NULL, vals[k] = const, isval[k]==1). */
    struct { type *targs[8]; int vals[8]; int isval[8]; int n; } mxlist;
}

%token <ival>  INT_LIT CHAR_LIT
%token <fval>  FLOAT_LIT
%token <sval>  IDENT TYPEDEF_NAME STRING_LIT TEMPLATE_NAME

%token KW_VOID KW_INT KW_FLOAT KW_CHAR KW_UNSIGNED KW_SIGNED
%token KW_SHORT KW_LONG KW_DOUBLE KW_BOOL
%token KW_IF KW_ELSE KW_WHILE KW_FOR KW_DO KW_SWITCH KW_CASE KW_DEFAULT
%token KW_BREAK KW_CONTINUE KW_RETURN KW_GOTO
%token KW_STRUCT KW_UNION KW_TYPEDEF KW_ENUM KW_SIZEOF KW_ASM KW_NEW KW_DELETE
%token KW_CLASS KW_THIS KW_PUBLIC KW_PRIVATE KW_PROTECTED
%token KW_NAMESPACE KW_USING TOK_SCOPE KW_VIRTUAL KW_OPERATOR
%token KW_TEMPLATE KW_TYPENAME KW_AUTO KW_OVERRIDE KW_FINAL KW_CPPCAST
%token KW_STATIC KW_EXTERN KW_CONST KW_STATIC_ASSERT KW_GENERIC

%token TOK_EQ TOK_NE TOK_LE TOK_GE TOK_SHL TOK_SHR TOK_LAND TOK_LOR
%token TOK_INC TOK_DEC
%token TOK_PLUSEQ TOK_MINUSEQ TOK_STAREQ TOK_SLASHEQ TOK_PERCENTEQ
%token TOK_AMPEQ TOK_PIPEEQ TOK_CARETEQ TOK_SHLEQ TOK_SHREQ
%token TOK_ARROW TOK_ELLIPSIS

%type <sval>  qualified_id op_name
%type <typ>   type_specifier struct_specifier class_specifier union_specifier enum_specifier base_type decl_specifiers base_clause
%type <intval> pointers storage_or_qual_list storage_or_qual
%type <intval> builtin_spec builtin_type_seq
%type <exp>   expr assignment_expr conditional_expr logical_or_expr logical_and_expr
%type <exp>   bit_or_expr bit_xor_expr bit_and_expr equality_expr relational_expr
%type <exp>   shift_expr additive_expr multiplicative_expr cast_expr unary_expr
%type <exp>   postfix_expr primary_expr opt_expr
%type <iz>    init_item_list init_val
%type <iitem> init_item
%type <gassoc> generic_assoc
%type <galist> generic_assoc_list
%type <st>    stmt compound_stmt selection_stmt iteration_stmt jump_stmt
%type <st>    expr_stmt labeled_stmt block_item local_decl
%type <slist> block_item_list
%type <dlist> init_declarator_list param_list non_empty_param_list
%type <dcl>   init_declarator param_declarator
%type <dimlist> array_suffix ct_arg_list
%type <talist>  type_arg_list
%type <mxlist>  ct_mixed_arg ct_mixed_arg_list
%type <elist> argument_list non_empty_argument_list
%type <opkind> assign_op

/* dangling-else: shift KW_ELSE over reducing an if-without-else */
%nonassoc IFX
%nonassoc KW_ELSE

%%

translation_unit:
      external_decl
    | translation_unit external_decl
    ;

external_decl:
      function_def
    | declaration                   { /* declaration's action adds it to globals (or registers typedef) */ }
    | template_prefix function_def  { g_in_template = 0; }  /* function template (monomorphized per use) */
    | template_prefix declaration   { g_in_template = 0; }  /* class template (type-erased) */
    | KW_NAMESPACE IDENT '{' translation_unit '}' { free($2); }   /* transparent namespace */
    | KW_NAMESPACE IDENT '{' '}'                   { free($2); }
    | KW_USING KW_NAMESPACE qualified_id ';'       { free($3); }  /* using-directive (no-op) */
    | KW_USING KW_NAMESPACE IDENT ';'              { free($3); }
    | KW_USING qualified_id ';'                    { free($2); }  /* using-declaration `using N::name;` -> no-op: namespaces are transparent so the name already resolves unqualified */
    | KW_USING IDENT '=' base_type pointers ';'    {            /* type alias `using T = U;` */
          st_add_typedef($2, apply_pointers($4, $5)); free($2);
      }
    | KW_STATIC_ASSERT '(' conditional_expr ',' STRING_LIT ')' ';' { check_static_assert($3, $5, yylineno); }
    | KW_STATIC_ASSERT '(' conditional_expr ')' ';'                { check_static_assert($3, NULL, yylineno); }
    ;

/* Template prefix. Each type parameter is a t_tparam placeholder (int-like until
   substituted). FUNCTION templates are captured and monomorphized per concrete
   argument type at the call site (so float instantiations use float ops). CLASS
   templates stay type-erased: the placeholder behaves as one machine word. */
template_prefix:
      KW_TEMPLATE '<' { g_tparam_n = 0; g_ct_nontype = 0; g_n_ctm = 0;
                        for (int i = 0; i < 16; i++) g_tparam_isval[i] = 0; }
      tparam_list '>' { g_in_template = 1; }
    ;
tparam_list:
      tparam
    | tparam_list ',' tparam
    ;
tparam:    /* IDENT first time; TYPEDEF_NAME if this param name was used before */
      KW_TYPENAME IDENT         { st_add_typedef($2, t_tparam(g_tparam_n++)); free($2); }
    | KW_CLASS    IDENT         { st_add_typedef($2, t_tparam(g_tparam_n++)); free($2); }
    | KW_TYPENAME TYPEDEF_NAME  { st_add_typedef($2, t_tparam(g_tparam_n++)); free($2); }
    | KW_CLASS    TYPEDEF_NAME  { st_add_typedef($2, t_tparam(g_tparam_n++)); free($2); }
    | KW_INT IDENT              {   /* non-type parameter: N is an int constant
                                       referenced via the NTP_BASE sentinel */
          st_add_enum($2, NTP_BASE + g_tparam_n);
          g_tparam_isval[g_tparam_n] = 1; g_ct_nontype = 1; g_tparam_n++;
          free($2);
      }
    ;

/* a namespace-qualified name N::x (or A::B::x); namespaces are transparent on
   this target, so qualification collapses to the final name. */
qualified_id:
      IDENT TOK_SCOPE IDENT          { free($1); $$ = $3; }
    | qualified_id TOK_SCOPE IDENT   { free($1); $$ = $3; }
    ;

/* ----- declarations ------------------------------------------------------- */

declaration:
      decl_specifiers init_declarator_list ';' {
          if (!ts_typedef) {
              for (decl *d = $2.head; d; ) {
                  decl *nx = d->next; d->next = NULL;
                  /* function prototypes have already been registered as SK_FUNC
                     in init_declarator — skip adding them as globals here */
                  sym *existing = st_find(d->name);
                  if (existing && existing->kind == SK_FUNC) {
                      /* nothing more to do */
                  } else {
                      int is_def = (d->sclass != SC_EXTERN);   // `extern x;` is only a declaration
                      if (existing && existing->kind == SK_GLOBAL_VAR) {
                          if (is_def && existing->defined)
                              msg_error(d->line, "redefinition of global '%s' "
                                  "(if this comes from a header, guard it with #ifndef or #pragma once)", d->name);
                          if (is_def) { existing->defined = 1; unit_add_global(d); }
                          /* extern re-declaration of a known global: nothing to emit */
                      } else {
                          sym *gs = st_add(SK_GLOBAL_VAR, d->name, d->name, d->dtype);
                          gs->defined = is_def;
                          if (is_def) unit_add_global(d);   /* extern-only: declared, no storage */
                      }
                  }
                  /* a const-qualified integer with a constant initializer is a
                     compile-time constant (like C++): also register it as a
                     constant so it folds in array bounds / constexprs */
                  if (d->is_const && d->dtype && d->dtype->kind == TY_INT && d->init) {
                      long cv;
                      if (const_eval(d->init, &cv)) st_add_enum(d->name, cv);
                  }
                  d = nx;
              }
          }
          ts_typedef = 0; ts_sclass = SC_NONE; ts_is_const = 0;
      }
    | decl_specifiers ';' {
          /* bare `struct foo { ... };` declares the tag, nothing else */
          ts_typedef = 0; ts_sclass = SC_NONE; ts_is_const = 0;
      }
    ;

decl_specifiers:
      base_type                              { $$ = $1; }
    | storage_or_qual_list base_type         { $$ = $2; }
    | base_type storage_or_qual_list         { $$ = $1; }
    ;

storage_or_qual_list:
      storage_or_qual                        { $$ = $1; }
    | storage_or_qual_list storage_or_qual   { $$ = $1 | $2; }
    ;

storage_or_qual:
      KW_STATIC    { ts_sclass = SC_STATIC; $$ = 0; }
    | KW_EXTERN    { ts_sclass = SC_EXTERN; $$ = 0; }
    | KW_TYPEDEF   { ts_typedef = 1; $$ = 0; }
    | KW_CONST     { ts_is_const = 1; $$ = 0; }
    ;

/* base_type stashes the resolved type in a static so init_declarators can grab it */
base_type:
      type_specifier                         { cur_base = $1; $$ = $1; }
    ;

type_specifier:
      builtin_type_seq                       { $$ = resolve_builtin($1); }
    | struct_specifier                       { $$ = $1;        }
    | class_specifier                        { $$ = $1;        }
    | union_specifier                        { $$ = $1;        }
    | enum_specifier                         { $$ = $1;        }
    | TYPEDEF_NAME                           {
          sym *s = st_find($1);
          if (!s || s->kind != SK_TYPEDEF) { msg_error(yylineno, "unknown type '%s'", $1); }
          $$ = s->stype;
          free($1);
      }
    | TYPEDEF_NAME '<' ct_mixed_arg_list '>' {
          /* class-template instantiation `Name<args...>` where each arg can be
             a type or a non-type value (the ctmpl's isval[] dictates which slot
             expects which; mismatches are caught by the substitution step). */
          ctmpl *ct = find_ctmpl($1);
          if (!ct) { msg_error(yylineno, "'%s' is not a class template", $1); $$ = t_int(); }
          else     { $$ = instantiate_ctmpl_mixed(ct, $3.targs, $3.vals, $3.isval, $3.n); }
          free($1);
      }
    | IDENT TOK_SCOPE TYPEDEF_NAME '<' ct_mixed_arg_list '>' {
          /* namespace-qualified mixed-arg instantiation `N::Name<args...>` */
          free($1);
          ctmpl *ct = find_ctmpl($3);
          if (!ct) { msg_error(yylineno, "'%s' is not a class template", $3); $$ = t_int(); }
          else     { $$ = instantiate_ctmpl_mixed(ct, $5.targs, $5.vals, $5.isval, $5.n); }
          free($3);
      }
    | IDENT TOK_SCOPE TYPEDEF_NAME           {
          /* namespace-qualified type N::Type (transparent -> resolve Type) */
          free($1);
          sym *s = st_find($3);
          if (!s || s->kind != SK_TYPEDEF) { msg_error(yylineno, "unknown type '%s'", $3); }
          $$ = s->stype;
          free($3);
      }
    | KW_AUTO                                { $$ = t_auto(); }   /* deduced at codegen */
    ;

/* one or more builtin keywords in any order, folded into flags (see ts_add) */
builtin_type_seq:
      builtin_spec                           { $$ = ts_add(0, $1); }
    | builtin_type_seq builtin_spec          { $$ = ts_add($1, $2); }
    ;

builtin_spec:
      KW_VOID      { $$ = TS_VOID;   }
    | KW_CHAR      { $$ = TS_CHAR;   }
    | KW_INT       { $$ = TS_INT;    }
    | KW_SHORT     { $$ = TS_SHORT;  }
    | KW_LONG      { $$ = TS_LONG;   }
    | KW_FLOAT     { $$ = TS_FLOAT;  }
    | KW_DOUBLE    { $$ = TS_DOUBLE; }
    | KW_BOOL      { $$ = TS_BOOL;   }
    | KW_SIGNED    { $$ = TS_SIGN;   }
    | KW_UNSIGNED  { $$ = TS_UNSIGN; }
    ;

struct_specifier:
      KW_STRUCT IDENT '{' { cur_struct_push(t_make_struct($2)); st_add_tag($2, cur_struct); st_add_typedef($2, cur_struct); } field_list '}' {
          type *done = cur_struct;
          t_struct_seal(done, g_unit && g_unit->nubits > 0 ? g_unit->nubits : 16);
          cur_struct_pop();          // restore the enclosing struct (nested defs)
          $$ = done;
          free($2);
      }
    | KW_STRUCT IDENT {
          sym *s = st_find_tag($2);
          if (!s) {
              /* forward-declare incomplete tag — caller better seal it before sizeof */
              type *t = t_make_struct($2);
              st_add_tag($2, t);
              $$ = t;
          } else {
              $$ = s->struct_t;
          }
          free($2);
      }
    | KW_STRUCT TYPEDEF_NAME {
          /* `struct Tag` where Tag is also a typedef (e.g. a self-referential
             struct, since we register the bare name as a type too) */
          sym *s = st_find_tag($2);
          if (!s) { type *t = t_make_struct($2); st_add_tag($2, t); $$ = t; }
          else    { $$ = s->struct_t; }
          free($2);
      }
    | KW_STRUCT '{' { cur_struct_push(t_make_struct("")); } field_list '}' {
          /* anonymous struct (e.g. a struct-typed field with no tag) */
          type *done = cur_struct;
          t_struct_seal(done, g_unit && g_unit->nubits > 0 ? g_unit->nubits : 16);
          cur_struct_pop();
          $$ = done;
      }
    ;

/* A class is a struct that may also contain member functions and access
   labels, and whose bare name is usable as a type. Methods lower to free
   functions taking an implicit `this`; inline method bodies see members
   declared above them (single-pass). */
class_specifier:
      KW_CLASS IDENT base_clause '{' {
          type *t = t_make_struct($2);
          cur_struct_push(t);
          st_add_tag($2, t);
          st_add_typedef($2, t);          // bare class name usable as a type
          cur_class = t;
          if ($3) {                       // single inheritance: lay the base first
              t->base_class = $3;
              for (strct_field *f = $3->fields; f; f = f->next)
                  t_struct_add_field(t, f->name, f->ftype);
              t->n_vtbl = $3->n_vtbl;     // inherit the base's virtual slots
              if (t->n_vtbl) {
                  t->vtbl = malloc(sizeof(char*) * t->n_vtbl);
                  for (int i = 0; i < t->n_vtbl; i++) t->vtbl[i] = strdup($3->vtbl[i]);
              }
          }
      } member_list '}' {
          type *done = cur_struct;
          /* a polymorphic class carries a vptr at offset 0 (inherited if the base
             already provides one, else inserted now ahead of its own fields) */
          if (done->n_vtbl > 0 && (!done->fields || strcmp(done->fields->name, "__vptr") != 0))
              t_struct_prepend_field(done, "__vptr", t_int());
          t_struct_seal(done, g_unit && g_unit->nubits > 0 ? g_unit->nubits : 16);
          cur_struct_pop();
          cur_class = NULL;
          /* a class template: capture it for real monomorphization. Non-type
             parameters bind to values (sentinels); type parameters bind to a
             concrete type at each `Name<args>` use. */
          if (g_in_template) {
              ctmpl *ct = calloc(1, sizeof(ctmpl));
              ct->name = xstrdup(done->tag); ct->proto = done; ct->n_tparams = g_tparam_n;
              for (int i = 0; i < g_tparam_n && i < 8; i++) ct->isval[i] = g_tparam_isval[i];
              ct->n_methods = g_n_ctm;
              ct->methods = malloc(sizeof(func*) * (g_n_ctm ? g_n_ctm : 1));
              for (int i = 0; i < g_n_ctm; i++) ct->methods[i] = g_ctm[i];
              unit_add_ctmpl(ct);
          }
          $$ = done;
          free($2);
      }
    | KW_CLASS IDENT {
          sym *s = st_find_tag($2);
          if (!s) { type *t = t_make_struct($2); st_add_tag($2, t); st_add_typedef($2, t); $$ = t; }
          else    { $$ = s->struct_t; }
          free($2);
      }
    ;

/* optional base-class clause `: [public|private|protected] Base` */
base_clause:
      /* empty */                  { $$ = NULL; }
    | ':' access_opt TYPEDEF_NAME  {
          sym *s = st_find($3);
          if (!s || s->kind != SK_TYPEDEF) msg_error(yylineno, "unknown base class '%s'", $3);
          $$ = s ? s->stype : NULL;
          free($3);
      }
    ;

access_opt:
      /* empty */     {}
    | KW_PUBLIC       {}
    | KW_PRIVATE      {}
    | KW_PROTECTED    {}
    ;

member_list:
      /* empty */
    | member_list member
    ;

member:
      access_label
    | field_decl
    | method_def
    | method_decl
    | ctor_def
    | ctor_decl
    | dtor_def
    | dtor_decl
    | static_member
    | struct_specifier ';'   /* nested struct/class type declaration */
    | enum_specifier ';'     /* nested enum / enum class declaration */
    ;

/* in-class method declaration: prototype only, body provided out-of-class. */
method_decl:
      base_type IDENT '(' param_list ')' fn_quals ';'
          { register_method_decl($1, $2, $4.head, $4.n); free($2); }
    | base_type '*' IDENT '(' param_list ')' fn_quals ';'
          { register_method_decl(t_ptr($1), $3, $5.head, $5.n); free($3); }
    | base_type '&' IDENT '(' param_list ')' fn_quals ';'
          { register_method_decl(t_ref($1), $3, $5.head, $5.n); free($3); }
    | KW_CONST base_type '&' IDENT '(' param_list ')' fn_quals ';'
          { register_method_decl(t_ref($2), $4, $6.head, $6.n); free($4); }
    | KW_VIRTUAL base_type IDENT '(' param_list ')' fn_quals ';'
          { add_vmethod(cur_class, $3); register_method_decl($2, $3, $5.head, $5.n); free($3); }
    | KW_VIRTUAL base_type '*' IDENT '(' param_list ')' fn_quals ';'
          { add_vmethod(cur_class, $4); register_method_decl(t_ptr($2), $4, $6.head, $6.n); free($4); }
    ;

/* in-class ctor declaration: prototype only, body provided out-of-class. */
ctor_decl:
      TYPEDEF_NAME '(' param_list ')' fn_quals ';'
          { register_method_decl(t_void(), ctor_kind($3.head), $3.head, $3.n); free($1); }
    ;

/* in-class destructor declaration: prototype only, body provided out-of-class. */
dtor_decl:
      '~' TYPEDEF_NAME '(' ')' fn_quals ';'
          { register_method_decl(t_void(), "dtor", NULL, 0); free($2); }
    | KW_VIRTUAL '~' TYPEDEF_NAME '(' ')' fn_quals ';'
          { add_vmethod(cur_class, "dtor"); register_method_decl(t_void(), "dtor", NULL, 0); free($3); }
    ;

/* static data member: a single shared global `Class__name` (not a per-object
   field). In-class init is treated as its definition. */
static_member:
      KW_STATIC base_type IDENT ';' {
          decl *d = ast_decl($2, mangle_method(cur_class->tag, $3), NULL, yylineno);
          unit_add_global(d); add_static(cur_class, $3); free($3);
      }
    | KW_STATIC base_type IDENT '=' assignment_expr ';' {
          decl *d = ast_decl($2, mangle_method(cur_class->tag, $3), $5, yylineno);
          unit_add_global(d); add_static(cur_class, $3); free($3);
      }
    /* `static const ...` forms: bison's shift/reduce default routes the KW_CONST
       through field_decl's base_type otherwise, and field_decl has no
       initializer slot, so `static const int X = N;` fails. Spell it out here. */
    | KW_STATIC KW_CONST base_type IDENT ';' {
          decl *d = ast_decl($3, mangle_method(cur_class->tag, $4), NULL, yylineno);
          unit_add_global(d); add_static(cur_class, $4); free($4);
      }
    | KW_STATIC KW_CONST base_type IDENT '=' assignment_expr ';' {
          char *mname = mangle_method(cur_class->tag, $4);
          decl *d = ast_decl($3, mname, $6, yylineno);
          unit_add_global(d); add_static(cur_class, $4);
          /* if the init folds to an int constant, also publish it as an enum
             constant under the MANGLED name so const_eval (template args /
             array sizes) can fold qualified `Class::X` AND, via cur_class
             fallback, the bare `X` from inside the class body. */
          long v;
          if (const_eval($6, &v)) st_add_enum(mname, v);
          free($4);
      }
    ;

/* constructor: `ClassName(params) { body }` — the class name lexes as a
   TYPEDEF_NAME inside its own body. Lowered to method `Class__ctor`. */
ctor_def:
      TYPEDEF_NAME '(' param_list ')' { g_n_ctor_inits = 0; } ctor_init_opt
          { method_enter(t_void(), ctor_kind($3.head), $3.head, $3.n); }
      compound_stmt
          {
              stmt *body = $8;
              inject_default_member_inits(cur_class);   // DMI fields not in member-init list
              if (g_n_ctor_inits > 0 && body) {     // prepend `member = expr;` stmts
                  int total = g_n_ctor_inits + body->n_items;
                  stmt **arr = malloc(sizeof(stmt*) * total);
                  for (int i = 0; i < g_n_ctor_inits; i++) arr[i] = g_ctor_inits[i];
                  for (int j = 0; j < body->n_items; j++) arr[g_n_ctor_inits + j] = body->items[j];
                  body->items = arr; body->n_items = total;
              }
              method_finish(t_void(), ctor_kind($3.head), body); free($1);
          }
    ;

/* optional member-initializer list `: x(a), y(b)` */
ctor_init_opt:
      /* empty */
    | ':' mem_init_list
    ;
mem_init_list:
      mem_init
    | mem_init_list ',' mem_init
    ;
mem_init:
      IDENT '(' assignment_expr ')' {
          /* data member: `member(expr)` -> `member = expr;` */
          stmt *s = ast_stmt(S_EXPR, yylineno);
          s->e1 = ast_assign(ast_ident($1, yylineno), $3, yylineno);
          if (g_n_ctor_inits < 32) g_ctor_inits[g_n_ctor_inits++] = s;
      }
    | TYPEDEF_NAME '(' argument_list ')' {
          /* base-class init: `Base(args)` -> Base__ctor(this, args); */
          expr **a = malloc(sizeof(expr*) * ($3.n + 1));
          a[0] = ast_ident(strdup("this"), yylineno);
          for (int i = 0; i < $3.n; i++) a[i + 1] = $3.arr[i];
          expr *call = ast_call(ast_ident(mangle_method($1, "ctor"), yylineno), a, $3.n + 1, yylineno);
          stmt *s = ast_stmt(S_EXPR, yylineno); s->e1 = call;
          if (g_n_ctor_inits < 32) g_ctor_inits[g_n_ctor_inits++] = s;
          free($1);
      }
    ;

/* destructor: `~ClassName() { body }` -> method `Class__dtor`. A virtual dtor
   adds a "dtor" vtable slot so `delete basePtr` dispatches to the derived one. */
dtor_def:
      '~' TYPEDEF_NAME '(' ')' fn_quals
          { method_enter(t_void(), "dtor", NULL, 0); }
      compound_stmt
          { method_finish(t_void(), "dtor", $7); free($2); }
    | KW_VIRTUAL '~' TYPEDEF_NAME '(' ')' fn_quals
          { add_vmethod(cur_class, "dtor"); method_enter(t_void(), "dtor", NULL, 0); }
      compound_stmt
          { method_finish(t_void(), "dtor", $8); free($3); }
    ;

access_label:
      KW_PUBLIC    ':'   { /* access control is cosmetic on this target */ }
    | KW_PRIVATE   ':'   { }
    | KW_PROTECTED ':'   { }
    ;

/* inline method: `ret name(params) { body }` inside a class body. Lowered to a
   free function `Class__name` with an implicit leading `this` parameter. The
   prefix `base_type IDENT` is shared with field_decl (no nullable `pointers`)
   so the field/method choice is decided by the `(` after the name. A leading
   `*` gives a pointer-return method. */
method_def:
      base_type IDENT '(' param_list ')' fn_quals
          { method_enter($1, $2, $4.head, $4.n); }
      compound_stmt
          { method_finish($1, $2, $8); free($2); }
    | base_type '*' IDENT '(' param_list ')' fn_quals
          { method_enter(t_ptr($1), $3, $5.head, $5.n); }
      compound_stmt
          { method_finish(t_ptr($1), $3, $9); free($3); }
    /* reference-return method: `T& name(params) { body }` and `const T& name(params) ...`.
       The leading `const` is cosmetic on this target (same convention as fn_quals KW_CONST
       and `KW_CONST param_declarator`). The return type becomes t_ref(T). */
    | base_type '&' IDENT '(' param_list ')' fn_quals
          { method_enter(t_ref($1), $3, $5.head, $5.n); }
      compound_stmt
          { method_finish(t_ref($1), $3, $9); free($3); }
    | KW_CONST base_type '&' IDENT '(' param_list ')' fn_quals
          { method_enter(t_ref($2), $4, $6.head, $6.n); }
      compound_stmt
          { method_finish(t_ref($2), $4, $10); free($4); }
    | KW_VIRTUAL base_type IDENT '(' param_list ')' fn_quals
          { add_vmethod(cur_class, $3); method_enter($2, $3, $5.head, $5.n); }
      compound_stmt
          { method_finish($2, $3, $9); free($3); }
    | KW_VIRTUAL base_type '*' IDENT '(' param_list ')' fn_quals
          { add_vmethod(cur_class, $4); method_enter(t_ptr($2), $4, $6.head, $6.n); }
      compound_stmt
          { method_finish(t_ptr($2), $4, $10); free($4); }
    | KW_VIRTUAL base_type IDENT '(' param_list ')' '=' INT_LIT ';'
          { add_vmethod(cur_class, $3); free($3); }   /* pure virtual: vtable slot only */
    | base_type KW_OPERATOR op_name '(' param_list ')' fn_quals
          { method_enter($1, op_arity_name($3, $5.n), $5.head, $5.n); }
      compound_stmt
          { method_finish($1, op_arity_name($3, $5.n), $9); }
    | base_type '&' KW_OPERATOR op_name '(' param_list ')' fn_quals
          { method_enter(t_ref($1), op_arity_name($4, $6.n), $6.head, $6.n); }
      compound_stmt
          { method_finish(t_ref($1), op_arity_name($4, $6.n), $10); }
    /* static method: `static T name(params) { body }` — lowered to a free
       function `Class__name` WITHOUT an implicit `this` param. */
    | KW_STATIC base_type IDENT '(' param_list ')' fn_quals
          { static_method_enter($2, $3, $5.head, $5.n); }
      compound_stmt
          { static_method_finish($2, $3, $9); free($3); }
    | KW_STATIC base_type '*' IDENT '(' param_list ')' fn_quals
          { static_method_enter(t_ptr($2), $4, $6.head, $6.n); }
      compound_stmt
          { static_method_finish(t_ptr($2), $4, $10); free($4); }
    ;

/* trailing method qualifiers — accepted and ignored (cosmetic on this target) */
fn_quals:
      /* empty */
    | fn_quals KW_OVERRIDE
    | fn_quals KW_FINAL
    | fn_quals KW_CONST
    ;

/* overloadable binary operators -> a stable method name (asm-label charset) */
op_name:
      '+'    { $$ = "op_add"; }
    | '-'    { $$ = "op_sub"; }
    | '*'    { $$ = "op_mul"; }
    | '/'    { $$ = "op_div"; }
    | TOK_EQ { $$ = "op_eq";  }
    | TOK_NE { $$ = "op_ne";  }
    | '<'    { $$ = "op_lt";  }
    | '>'    { $$ = "op_gt";  }
    | TOK_LE { $$ = "op_le";  }
    | TOK_GE { $$ = "op_ge";  }
    | '='    { $$ = "op_assign"; }
    | '[' ']' { $$ = "op_index"; }
    | TOK_PLUSEQ  { $$ = "op_addeq"; }
    | TOK_MINUSEQ { $$ = "op_subeq"; }
    | TOK_STAREQ  { $$ = "op_muleq"; }
    | TOK_SLASHEQ { $$ = "op_diveq"; }
    ;

union_specifier:
      KW_UNION IDENT '{' { cur_struct_push(t_make_union($2)); st_add_tag($2, cur_struct); st_add_typedef($2, cur_struct); } field_list '}' {
          type *done = cur_struct;
          t_struct_seal(done, g_unit && g_unit->nubits > 0 ? g_unit->nubits : 16);
          cur_struct_pop();
          $$ = done;
          free($2);
      }
    | KW_UNION TYPEDEF_NAME {
          /* `union Tag` where Tag is also a typedef (we register the bare name
             as a type too, mirroring struct) */
          sym *s = st_find_tag($2);
          if (!s) { type *t = t_make_union($2); st_add_tag($2, t); $$ = t; }
          else    { $$ = s->struct_t; }
          free($2);
      }
    | KW_UNION IDENT {
          sym *s = st_find_tag($2);
          if (!s) {
              type *t = t_make_union($2);
              st_add_tag($2, t);
              $$ = t;
          } else {
              $$ = s->struct_t;
          }
          free($2);
      }
    | KW_UNION '{' { cur_struct_push(t_make_union("")); } field_list '}' {
          /* anonymous union (e.g. a union-typed field with no tag) */
          type *done = cur_struct;
          t_struct_seal(done, g_unit && g_unit->nubits > 0 ? g_unit->nubits : 16);
          cur_struct_pop();
          $$ = done;
      }
    ;

field_list:
      field_decl
    | field_list field_decl
    ;

field_decl:
      base_type field_declarator_list ';'
    | KW_CONST base_type field_declarator_list ';'   /* `const T x;` — const is cosmetic on this target */
    ;

field_declarator_list:
      field_declarator
    | field_declarator_list ',' field_declarator
    ;

/* explicit alternatives (no nullable pointers/array_suffix) so the bitfield
   `IDENT : N` form doesn't clash with a plain `IDENT` field on lookahead */
field_declarator:
      IDENT {
          t_struct_add_field(cur_struct, $1, cur_base);
          free($1);
      }
    | IDENT '=' assignment_expr {
          /* default member initializer `T name = expr;` (C++11). The init is
             attached to the field and replayed in every ctor body that doesn't
             already write `name` via its member-init list. */
          t_struct_add_field(cur_struct, $1, cur_base);
          strct_field *f = t_struct_find(cur_struct, $1);
          if (f) f->dinit = $3;
          free($1);
      }
    | IDENT '=' '{' '}' {
          /* default member initializer `T name = {};` — zero/value-init. Scalar
             (and pointer) fields get a 0 dinit; aggregate (array/struct) fields
             are flagged `dzero` so the ctor zero-fills every word, so heap objects
             get zeroed too (not just .mif-backed static storage). */
          t_struct_add_field(cur_struct, $1, cur_base);
          strct_field *f = t_struct_find(cur_struct, $1);
          if (f && cur_base) {
              if (cur_base->kind == TY_INT || cur_base->kind == TY_PTR)
                  f->dinit = ast_int_lit(0, yylineno);
              else if (cur_base->kind == TY_FLOAT)
                  f->dinit = ast_float_lit(0.0f, yylineno);
              else
                  f->dzero = 1;     /* array/struct aggregate: zero-fill at ctor time */
          }
          free($1);
      }
    | IDENT '[' conditional_expr ']' {
          /* size is any constant expression: a literal, an enum constant, or a
             non-type template parameter (a sentinel int, resolved at instantiation) */
          long v;
          if (!const_eval($3, &v)) msg_error(yylineno, "array field size must be constant");
          t_struct_add_field(cur_struct, $1, t_array(cur_base, (int)v));
          free($1);
      }
    | '*' IDENT {
          t_struct_add_field(cur_struct, $2, t_ptr(cur_base));
          free($2);
      }
    | IDENT ':' INT_LIT {
          /* bitfield `name : width` (e.g. unsigned flags : 3) */
          t_struct_add_bitfield(cur_struct, $1, cur_base, (int)$3);
          free($1);
      }
    | ':' INT_LIT {
          /* anonymous bitfield (padding / alignment) */
          t_struct_add_bitfield(cur_struct, "", cur_base, (int)$2);
      }
    | '(' '*' IDENT array_suffix ')' '(' param_list ')' {
          /* function-pointer field `ret (*name)(params)` — an int function id;
             with array_suffix, `ret (*name[N])(params)` is an int[] of fn ids. */
          type *t = build_array_type(t_int(), $4.dims, $4.n);
          t_struct_add_field(cur_struct, $3, t);
          free($3);
      }
    ;

enum_specifier:
      KW_ENUM '{' { enum_counter = 0; } enum_list '}'              { $$ = t_int(); }
    | KW_ENUM IDENT '{' { enum_counter = 0; } enum_list '}'        { $$ = t_int(); free($2); }
    | KW_ENUM IDENT                                                { $$ = t_int(); free($2); }
    | KW_ENUM KW_CLASS IDENT '{' { enum_counter = 0; cur_enum_prefix = $3; } enum_list '}' {
          /* scoped enum: enumerators are E__name, accessed as E::name; E is a type */
          st_add_typedef($3, t_int());
          cur_enum_prefix = NULL;
          $$ = t_int(); free($3);
      }
    ;

enum_list:
      enum_item
    | enum_list ',' enum_item
    ;

enum_item:
      IDENT {
          char *nm = cur_enum_prefix ? mangle_method(cur_enum_prefix, $1) : strdup($1);
          st_add_enum(nm, enum_counter++); free(nm); free($1);
      }
    | IDENT '=' INT_LIT {
          char *nm = cur_enum_prefix ? mangle_method(cur_enum_prefix, $1) : strdup($1);
          enum_counter = $3; st_add_enum(nm, enum_counter++); free(nm); free($1);
      }
    ;

/* ----- declarators -------------------------------------------------------- */

pointers:
      /* empty */         { $$ = 0; }
    | pointers '*'        { $$ = $1 + 1; }
    | pointers KW_CONST   { $$ = $1; }   /* `T * const` — const ptr, ignored */
    ;

/* zero or more [N] suffixes, in source order (row-major). n==0 means scalar. */
array_suffix:
      /* empty */                 { $$.n = 0; }
    | array_suffix '[' ']'        { $$ = $1; if ($$.n < 8) $$.dims[$$.n++] = 0;        }  /* unsized — params only */
    | array_suffix '[' { $<typ>$ = cur_base; } conditional_expr ']' {
          /* A sizeof(T) / cast / new in the size expression reduces its own
             base_type and clobbers the global cur_base that the enclosing
             declarator reads afterwards -- e.g. `float a[sizeof(int)]` would
             register `a` as int[]. Restore the declarator's base type captured
             before the size expression. This makes array_suffix transparent to
             cur_base for every rule that uses it (the init_declarator forms read
             cur_base directly or via their own mid-rule capture). */
          cur_base = $<typ>3;
          long v;
          if (!const_eval($4, &v)) msg_error(yylineno, "array size must be a constant expression");
          $$ = $1; if ($$.n < 8) $$.dims[$$.n++] = (int)v;   /* enum/macro/sizeof/arith ok */
      }
    ;

/* explicit template TYPE arguments `<int>` / `<float, int>` for fn<T...>(args) */
type_arg_list:
      base_type pointers {
          $$.arr = malloc(sizeof(type*)); $$.arr[0] = apply_pointers($1, $2); $$.n = 1;
      }
    | type_arg_list ',' base_type pointers {
          $$ = $1; $$.arr = realloc($$.arr, sizeof(type*) * ($$.n + 1));
          $$.arr[$$.n++] = apply_pointers($3, $4);
      }
    ;

/* non-type template arguments `<4>` / `<4, 8>` — constant integer expressions.
   Uses shift_expr (below relational) so a closing `>` is not eaten as `operator>`. */
ct_arg_list:
      shift_expr {
          long v; if (!const_eval($1, &v)) msg_error(yylineno, "template argument must be a constant");
          $$.n = 0; if ($$.n < 8) $$.dims[$$.n++] = (int)v;
      }
    | ct_arg_list ',' shift_expr {
          long v; if (!const_eval($3, &v)) msg_error(yylineno, "template argument must be a constant");
          $$ = $1; if ($$.n < 8) $$.dims[$$.n++] = (int)v;
      }
    ;

/* MIXED class-template arguments `<T, N>` / `<T>` / `<N>`. Each entry is either
   a type or a constant int. Used by `Name<args>` to drive instantiate_ctmpl_mixed.
   shift_expr (not relational_expr) keeps the closing '>' from being eaten as
   the greater-than operator. */
ct_mixed_arg:
      base_type pointers {
          $$.n = 1;
          $$.targs[0] = apply_pointers($1, $2);
          $$.vals[0]  = 0;
          $$.isval[0] = 0;
      }
    | shift_expr {
          long v;
          if (!const_eval($1, &v)) msg_error(yylineno, "template argument must be a constant");
          $$.n = 1;
          $$.targs[0] = NULL;
          $$.vals[0]  = (int)v;
          $$.isval[0] = 1;
      }
    ;
ct_mixed_arg_list:
      ct_mixed_arg                            { $$ = $1; }
    | ct_mixed_arg_list ',' ct_mixed_arg      {
          $$ = $1;
          if ($$.n < 8) {
              $$.targs[$$.n] = $3.targs[0];
              $$.vals [$$.n] = $3.vals [0];
              $$.isval[$$.n] = $3.isval[0];
              $$.n++;
          }
      }
    ;

init_declarator_list:
      init_declarator                                { $$.head = $1; $$.n = 1; }
    | init_declarator_list ',' init_declarator       {
          decl *d = $1.head; while (d->next) d = d->next; d->next = $3;
          $$.head = $1.head; $$.n = $1.n + 1;
      }
    ;

init_declarator:
      '&' IDENT '=' { $<typ>$ = cur_base; } assignment_expr {
          /* reference variable `T& r = lvalue;` — r binds to the address of the
             initializer and auto-derefs on every use. Save cur_base before
             the initializer in case a cast/sizeof/new inside it reduces a
             new base_type and clobbers the global. */
          decl *d = ast_decl(t_ref($<typ>4), $2, $5, yylineno);
          d->sclass = ts_sclass;
          $$ = d;
      }
    | pointers IDENT '(' { $<typ>$ = cur_base; } non_empty_argument_list ')' {
          /* direct-init `T v(args)` — capture the base type before the args (a
             `new`/cast/sizeof inside them would otherwise clobber cur_base).
             Requires >=1 arg: `T v()` is a function declaration (most vexing
             parse), so the empty form must fall through to the prototype /
             function-definition rules instead. */
          decl *d = make_decl($<typ>4, $1, $2, NULL, 0, yylineno, NULL);
          d->ctor_args = $5.arr; d->n_ctor_args = $5.n;
          $$ = d;
      }
    | pointers IDENT '{' '}' {
          /* direct list-init `T v{};` — empty brace, value-init (zero fields) */
          $$ = make_decl(cur_base, $1, $2, NULL, 0, yylineno, NULL);
      }
    | pointers IDENT '{' { $<typ>$ = cur_base; } init_item_list '}' {
          /* direct list-init `T v{a, b, ...};` — aggregate init without `=`,
             same semantics as `T v = {a, b, ...}` (line 1446 below). Save
             cur_base before the list items in case a cast inside one of
             them reduces a new base_type and clobbers the global. */
          decl *d = make_decl($<typ>4, $1, $2, NULL, 0, yylineno, NULL);
          d->binit = $5;
          $$ = d;
      }
    | pointers IDENT array_suffix {
          if (ts_typedef) {
              type *t = apply_pointers(cur_base, $1);
              t = build_array_type(t, $3.dims, $3.n);
              st_add_typedef($2, t);
              /* still produce a decl so list grouping works; it'll be ignored at the declaration level */
              $$ = ast_decl(t, $2, NULL, yylineno);
          } else {
              $$ = make_decl(cur_base, $1, $2, $3.dims, $3.n, yylineno, NULL);
          }
      }
    | pointers IDENT array_suffix '=' { $<typ>$ = cur_base; } assignment_expr {
          /* Mid-rule save of cur_base: a cast / sizeof / new inside the
             initializer would reduce its own `base_type` and clobber the
             global cur_base, so capture the declarator's base type BEFORE
             parsing the initializer. Same pattern as the direct-init form
             at the top of init_declarator. */
          if (ts_typedef) msg_error(yylineno, "typedef cannot have an initializer");
          $$ = make_decl($<typ>5, $1, $2, $3.dims, $3.n, yylineno, $6);
      }
    | pointers IDENT array_suffix '=' '{' '}' {
          /* empty brace init — leave memory at its .mif default (zero) */
          $$ = make_decl(cur_base, $1, $2, $3.dims, $3.n, yylineno, NULL);
      }
    | pointers IDENT array_suffix '=' '{' { $<typ>$ = cur_base; } init_item_list '}' {
          /* aggregate initialiser: array or struct, one value per slot/field.
             Same cur_base capture as above — designators / nested values may
             contain casts that clobber the global. */
          decl *d = make_decl($<typ>6, $1, $2, $3.dims, $3.n, yylineno, NULL);
          d->binit = $7;
          /* infer an unsized outer dimension `[]` from the initializer count */
          if (d->dtype && d->dtype->kind == TY_ARRAY && d->dtype->arr_size == 0 && $7 && $7->is_list)
              d->dtype->arr_size = $7->n;
          $$ = d;
      }
    | pointers IDENT array_suffix STRING_LIT {
          /* `int arr[N] "file.txt";` — initialise the array from a data file at synth time */
          decl *d = make_decl(cur_base, $1, $2, $3.dims, $3.n, yylineno, NULL);
          d->init_file = $4;
          $$ = d;
      }
    | '(' '*' IDENT array_suffix ')' '(' param_list ')' {
          /* function pointer `ret (*fp)(params)` — held as an int function id;
             with array_suffix, `ret (*arr[N])(params)` is an int array of ids.
             As a typedef (`typedef ret (*Name)(params);`) Name aliases that type. */
          type *t = build_array_type(t_int(), $4.dims, $4.n);
          if (ts_typedef) st_add_typedef($3, t);
          $$ = ast_decl(t, $3, NULL, yylineno);
      }
    | '(' '*' IDENT array_suffix ')' '(' param_list ')' '=' '{' init_item_list '}' {
          /* `ret (*arr[N])(params) = { f, g, ... }` */
          type *t = build_array_type(t_int(), $4.dims, $4.n);
          decl *d = ast_decl(t, $3, NULL, yylineno);
          d->binit = $11;
          if (t->kind == TY_ARRAY && t->arr_size == 0 && $11->is_list) t->arr_size = $11->n;
          $$ = d;
      }
    | pointers IDENT '(' param_list ')' {
          /* function declaration (prototype) — register, attach signature.
             Default-arg expressions from the prototype are stashed on the
             sym so the matching definition (which won't repeat them) can
             pick them up. */
          type *ret = apply_pointers(cur_base, $1);
          sym *s = st_add(SK_FUNC, $2, $2, ret);
          s->n_params = $4.n;
          if ($4.n > 0) {
              s->param_types    = malloc(sizeof(type*) * $4.n);
              s->param_defaults = malloc(sizeof(expr*) * $4.n);
              int i = 0;
              for (decl *p = $4.head; p; p = p->next, i++) {
                  s->param_types[i]    = p->dtype;
                  s->param_defaults[i] = p->init;
              }
          }
          $$ = ast_decl(ret, $2, NULL, yylineno); /* placeholder; not registered as global var */
      }
    ;

/* comma-separated initialisers inside { }. Each value is a scalar expression
   or a nested brace list, optionally prefixed by one C99 designator
   (`.field =` / `[index] =`). */
init_val:
      assignment_expr               { $$ = ast_initz_expr($1, yylineno); }
    | '{' init_item_list '}'        { $$ = $2; }
    ;

init_item:
      init_val                          { $$.z = $1; $$.d.kind = DESIG_NONE;  $$.d.name = NULL; $$.d.idx = 0; }
    | '.' IDENT '=' init_val            { $$.z = $4; $$.d.kind = DESIG_FIELD; $$.d.name = $2;   $$.d.idx = 0; }
    | '[' INT_LIT ']' '=' init_val      { $$.z = $5; $$.d.kind = DESIG_INDEX; $$.d.name = NULL; $$.d.idx = (int)$2; }
    ;

init_item_list:
      init_item                       { $$ = ast_initz_list(yylineno); initz_add($$, $1.z, $1.d); }
    | init_item_list ',' init_item    { $$ = $1; initz_add($$, $3.z, $3.d); }
    ;

/* ----- function definition ------------------------------------------------ */

function_def:
      decl_specifiers pointers IDENT '(' param_list ')' {
          /* return type comes from $1 (a stack value): cur_base would be
             clobbered by the params' and body's own base_type reductions. */
          type *ret = apply_pointers($1, $2);
          if (g_in_template) tmpl_name_register($3);   // lexer emits TEMPLATE_NAME for it
          sym *s = st_find($3);
          if (s && s->kind != SK_FUNC)
              msg_error(yylineno, "'%s' redeclared as a function (already a variable)", $3);
          /* C++ overloading: a same-name function with a DIFFERENT signature is a
             new overload; only an identical signature already-defined is an error. */
          char *nsig = params_sig($5.head);
          int reuse = 0;
          if (s && s->kind == SK_FUNC) {
              char *osig = sym_sig(s);
              int same = (strcmp(osig, nsig) == 0);
              free(osig);
              if (same && s->defined)
                  msg_error(yylineno, "redefinition of function '%s'%s", $3,
                      strcmp($3, "main") == 0 ? " (a program has exactly one main)" : "");
              if (same) reuse = 1;          /* matching prototype -> attach the body */
          }
          free(nsig);
          if (!reuse) {
              s = st_add(SK_FUNC, $3, $3, ret);
              s->n_params = $5.n;
              if ($5.n > 0) {
                  s->param_types = malloc(sizeof(type*) * $5.n);
                  int i = 0;
                  for (decl *p = $5.head; p; p = p->next, i++) s->param_types[i] = p->dtype;
              }
          } else if (s->param_defaults && $5.n == s->n_params) {
              /* a prior prototype recorded default args — replay them onto the
                 definition's params so call-site resolve_overload sees defaults. */
              decl *p = $5.head; int i = 0;
              for (; p && i < s->n_params; p = p->next, i++) {
                  if (!p->init && s->param_defaults[i]) p->init = s->param_defaults[i];
              }
          }
          s->defined = 1;
          /* enter function scope and stage params for the body */
          st_enter_func($3);
          st_push_scope();
          for (decl *p = $5.head; p; p = p->next) {
              type *pt = p->dtype;
              if (pt->kind == TY_ARRAY) {
                  /* arrays decay to pointers in param position, but we keep the array
                     flavour so codegen can route LDA/STA correctly. The runtime
                     value is the base address either way. */
              }
              st_add(SK_PARAM, p->name, NULL, pt);
          }
          param_head = $5.head; param_tail = NULL; param_count = $5.n;
          ts_sclass = SC_NONE; ts_is_const = 0;
      }
      compound_stmt {
          /* finalize the function ($1 = decl_specifiers return type) */
          func *f = ast_func(apply_pointers($1, $2), $3, param_head, param_count, $8, yylineno);
          if (g_in_template) unit_add_template(f);   /* captured; instantiated per use */
          else               unit_add_func(f);
          st_pop_scope();
          st_leave_func();
          params_reset();
      }
    /* out-of-class regular method definition: `Ret Class::method(params) fn_quals { body }`.
       The class must already exist (its body declared the prototype). Defaults from the
       in-class declaration replay onto these params; the body sees `this` and member fields
       via method_enter / method_finish's existing cur_class wiring. */
    | decl_specifiers pointers TYPEDEF_NAME TOK_SCOPE IDENT '(' param_list ')' fn_quals {
          sym *cs = st_find_tag($3);
          if (!cs || !cs->struct_t)
              msg_error(yylineno, "out-of-class definition for unknown class '%s'", $3);
          cur_class = cs->struct_t;
          replay_method_defaults($3, $5, $7.head);
          method_enter(apply_pointers($1, $2), $5, $7.head, $7.n);
      }
      compound_stmt {
          method_finish(apply_pointers($1, $2), $5, $11);
          cur_class = NULL;
          free($3); free($5);
      }
    /* out-of-class reference-return method: `Ret& Class::method(...) { body }` and
       `const Ret& Class::method(...) { body }`. decl_specifiers already absorbs a leading
       `const`. The `&` between return-type and class becomes t_ref on the return. */
    | decl_specifiers pointers '&' TYPEDEF_NAME TOK_SCOPE IDENT '(' param_list ')' fn_quals {
          sym *cs = st_find_tag($4);
          if (!cs || !cs->struct_t)
              msg_error(yylineno, "out-of-class definition for unknown class '%s'", $4);
          cur_class = cs->struct_t;
          replay_method_defaults($4, $6, $8.head);
          method_enter(t_ref(apply_pointers($1, $2)), $6, $8.head, $8.n);
      }
      compound_stmt {
          method_finish(t_ref(apply_pointers($1, $2)), $6, $12);
          cur_class = NULL;
          free($4); free($6);
      }
    /* out-of-class constructor: `Class::Class(params) fn_quals : inits { body }`.
       Both names lex as TYPEDEF_NAME; we require they match. */
    | TYPEDEF_NAME TOK_SCOPE TYPEDEF_NAME '(' param_list ')' fn_quals {
          if (strcmp($1, $3) != 0)
              msg_error(yylineno, "qualified id '%s::%s' is not a constructor", $1, $3);
          sym *cs = st_find_tag($1);
          if (!cs || !cs->struct_t)
              msg_error(yylineno, "out-of-class ctor for unknown class '%s'", $1);
          cur_class = cs->struct_t;
          g_n_ctor_inits = 0;
      }
      ctor_init_opt {
          const char *kind = ctor_kind($5.head);
          replay_method_defaults($1, kind, $5.head);
          method_enter(t_void(), kind, $5.head, $5.n);
      }
      compound_stmt {
          stmt *body = $11;
          inject_default_member_inits(cur_class);   /* DMI fields not in member-init list */
          if (g_n_ctor_inits > 0 && body) {     /* prepend `member = expr;` stmts */
              int total = g_n_ctor_inits + body->n_items;
              stmt **arr = malloc(sizeof(stmt*) * total);
              for (int i = 0; i < g_n_ctor_inits; i++) arr[i] = g_ctor_inits[i];
              for (int j = 0; j < body->n_items; j++) arr[g_n_ctor_inits + j] = body->items[j];
              body->items = arr; body->n_items = total;
          }
          method_finish(t_void(), ctor_kind($5.head), body);
          cur_class = NULL;
          free($1); free($3);
      }
    /* out-of-class destructor: `Class::~Class() { body }`. Both names lex as
       TYPEDEF_NAME; require they match. The in-class `~Class();` decl registers
       the Class__dtor prototype; this supplies the body under that same symbol. */
    | TYPEDEF_NAME TOK_SCOPE '~' TYPEDEF_NAME '(' ')' fn_quals {
          if (strcmp($1, $4) != 0)
              msg_error(yylineno, "qualified id '%s::~%s' is not a destructor", $1, $4);
          sym *cs = st_find_tag($1);
          if (!cs || !cs->struct_t)
              msg_error(yylineno, "out-of-class dtor for unknown class '%s'", $1);
          cur_class = cs->struct_t;
          method_enter(t_void(), "dtor", NULL, 0);
      }
      compound_stmt {
          method_finish(t_void(), "dtor", $9);
          cur_class = NULL;
          free($1); free($4);
      }
    ;

param_list:
      /* empty */                            { $$.head = NULL; $$.n = 0; }
    | non_empty_param_list                   {
          /* `(void)` parses as a single abstract void param — normalise to 0 */
          if ($1.n == 1 && $1.head->dtype && $1.head->dtype->kind == TY_VOID) { $$.head = NULL; $$.n = 0; }
          else $$ = $1;
      }
    ;

non_empty_param_list:
      param_declarator                       { $$.head = $1; $$.n = 1; }
    | non_empty_param_list ',' param_declarator {
          decl *d = $1.head; while (d->next) d = d->next; d->next = $3;
          $$.head = $1.head; $$.n = $1.n + 1;
      }
    ;

param_declarator:
      KW_CONST param_declarator              { $$ = $2; }   /* `const T ...` — ignored */
    | base_type pointers IDENT array_suffix {
          type *t = apply_pointers($1, $2);
          t = build_array_type(t, $4.dims, $4.n);
          $$ = ast_decl(t, $3, NULL, yylineno);
      }
    | base_type '&' IDENT {
          /* reference parameter `T& name` — pass-by-address, auto-deref on use */
          $$ = ast_decl(t_ref($1), $3, NULL, yylineno);
      }
    | base_type pointers IDENT '=' assignment_expr {
          /* default argument `T name = expr` — the default is the param's init */
          $$ = ast_decl(apply_pointers($1, $2), $3, $5, yylineno);
      }
    | base_type pointers {
          /* abstract (unnamed) parameter — e.g. the `int` in `int (*f)(int)` */
          $$ = ast_decl(apply_pointers($1, $2), "_anon", NULL, yylineno);
      }
    | base_type '(' '*' IDENT ')' '(' param_list ')' {
          /* function-pointer parameter — int function ID */
          $$ = ast_decl(t_int(), $4, NULL, yylineno);
      }
    ;

/* ----- statements --------------------------------------------------------- */

stmt:
      compound_stmt                          { $$ = $1; }
    | selection_stmt                         { $$ = $1; }
    | iteration_stmt                         { $$ = $1; }
    | jump_stmt                              { $$ = $1; }
    | labeled_stmt                           { $$ = $1; }
    | expr_stmt                              { $$ = $1; }
    | KW_ASM '(' STRING_LIT ')' ';'          {
          /* basic inline assembly: the string is emitted verbatim into the
             .asm (split on '\n'). Operands must use asm-level names. */
          stmt *s = ast_stmt(S_ASM, yylineno); s->label = $3; $$ = s;
      }
    | KW_STATIC_ASSERT '(' conditional_expr ',' STRING_LIT ')' ';' { check_static_assert($3, $5, yylineno);  $$ = ast_stmt(S_NULL, yylineno); }
    | KW_STATIC_ASSERT '(' conditional_expr ')' ';'                { check_static_assert($3, NULL, yylineno); $$ = ast_stmt(S_NULL, yylineno); }
    ;

compound_stmt:
      '{' { st_push_scope(); } block_item_list '}' {
          stmt *s = ast_stmt(S_BLOCK, yylineno);
          s->items = $3.arr; s->n_items = $3.n;
          st_pop_scope();
          $$ = s;
      }
    | '{' '}' {
          stmt *s = ast_stmt(S_BLOCK, yylineno);
          s->items = NULL; s->n_items = 0;
          $$ = s;
      }
    ;

block_item_list:
      block_item                             { $$.arr = malloc(sizeof(stmt*)); $$.arr[0] = $1; $$.n = 1; }
    | block_item_list block_item             {
          $$.arr = realloc($1.arr, sizeof(stmt*) * ($1.n + 1));
          $$.arr[$1.n] = $2;
          $$.n = $1.n + 1;
      }
    ;

block_item:
      stmt                                   { $$ = $1; }
    | local_decl                             { $$ = $1; }
    ;

/* a local declaration produces an S_DECL stmt with chained decls in s->decls */
local_decl:
      decl_specifiers init_declarator_list ';' {
          stmt *s = ast_stmt(S_DECL, yylineno);
          if (ts_typedef) {
              /* typedef at local scope: registration already happened in init_declarator */
              s->decls = NULL;
          } else {
              s->decls = $2.head;
              /* publish names in current local scope */
              for (decl *d = $2.head; d; d = d->next) {
                  st_add(SK_LOCAL_VAR, d->name, NULL, d->dtype);
                  /* `const int X = expr;` / `constexpr int X = expr;` whose
                     init folds to a constant: also publish as SK_ENUM_CONST so
                     const_eval (array bounds, template args) accepts it. */
                  if (ts_is_const && d->init && d->dtype && d->dtype->kind == TY_INT) {
                      long v;
                      if (const_eval(d->init, &v)) st_add_enum(d->name, v);
                  }
              }
          }
          ts_typedef = 0; ts_sclass = SC_NONE; ts_is_const = 0;
          $$ = s;
      }
    ;

expr_stmt:
      ';'                                    { $$ = ast_stmt(S_NULL, yylineno); }
    | expr ';'                               { stmt *s = ast_stmt(S_EXPR, yylineno); s->e1 = $1; $$ = s; }
    ;

selection_stmt:
      KW_IF '(' expr ')' stmt %prec IFX {
          stmt *s = ast_stmt(S_IF, yylineno); s->e1 = $3; s->body = $5; s->body2 = NULL; $$ = s;
      }
    | KW_IF '(' expr ')' stmt KW_ELSE stmt {
          stmt *s = ast_stmt(S_IF, yylineno); s->e1 = $3; s->body = $5; s->body2 = $7; $$ = s;
      }
    | KW_SWITCH '(' expr ')' stmt {
          stmt *s = ast_stmt(S_SWITCH, yylineno); s->e1 = $3; s->body = $5; $$ = s;
      }
    ;

labeled_stmt:
      KW_CASE expr ':' stmt {
          stmt *s = ast_stmt(S_CASE, yylineno); s->e1 = $2; s->body = $4; $$ = s;
      }
    | KW_DEFAULT ':' stmt {
          stmt *s = ast_stmt(S_DEFAULT, yylineno); s->body = $3; $$ = s;
      }
    | IDENT ':' stmt {
          stmt *s = ast_stmt(S_LABEL, yylineno); s->label = $1; s->body = $3; $$ = s;
      }
    ;

iteration_stmt:
      KW_WHILE '(' expr ')' stmt {
          stmt *s = ast_stmt(S_WHILE, yylineno); s->e1 = $3; s->body = $5; $$ = s;
      }
    | KW_DO stmt KW_WHILE '(' expr ')' ';' {
          stmt *s = ast_stmt(S_DOWHILE, yylineno); s->e1 = $5; s->body = $2; $$ = s;
      }
    | KW_FOR '(' opt_expr ';' opt_expr ';' opt_expr ')' stmt {
          stmt *s = ast_stmt(S_FOR, yylineno);
          if ($3) { stmt *is = ast_stmt(S_EXPR, yylineno); is->e1 = $3; s->init_stmt = is; }
          s->e1 = $5; s->e2 = $7; s->body = $9;
          $$ = s;
      }
    | KW_FOR '(' local_decl opt_expr ';' opt_expr ')' stmt {
          stmt *s = ast_stmt(S_FOR, yylineno);
          s->init_stmt = $3;
          s->e1 = $4; s->e2 = $6; s->body = $8;
          $$ = s;
      }
    | KW_FOR '(' decl_specifiers pointers IDENT ':' IDENT ')' stmt {  /* range-for, by value */
          $$ = make_range_for(apply_pointers($3, $4), $5, $7, $9, yylineno); free($7);
      }
    | KW_FOR '(' decl_specifiers '&' IDENT ':' IDENT ')' stmt {       /* range-for, by reference */
          $$ = make_range_for(t_ref($3), $5, $7, $9, yylineno); free($7);
      }
    ;

opt_expr:
      /* empty */                            { $$ = NULL; }
    | expr                                   { $$ = $1; }
    ;

jump_stmt:
      KW_BREAK ';'                           { $$ = ast_stmt(S_BREAK, yylineno); }
    | KW_CONTINUE ';'                        { $$ = ast_stmt(S_CONTINUE, yylineno); }
    | KW_RETURN ';'                          { $$ = ast_stmt(S_RETURN, yylineno); }
    | KW_RETURN expr ';'                     { stmt *s = ast_stmt(S_RETURN, yylineno); s->e1 = $2; $$ = s; }
    | KW_GOTO IDENT ';'                      { stmt *s = ast_stmt(S_GOTO, yylineno); s->label = $2; $$ = s; }
    ;

/* ----- expressions (C precedence cascade) -------------------------------- */

expr:
      assignment_expr                        { $$ = $1; }
    | expr ',' assignment_expr               { $$ = ast_comma($1, $3, yylineno); }
    ;

assignment_expr:
      conditional_expr                       { $$ = $1; }
    | unary_expr '=' assignment_expr         { $$ = ast_assign($1, $3, yylineno); }
    | unary_expr assign_op assignment_expr   {
          /* a OP= b  ⇒  a = a OP b (for class lhs, codegen may instead dispatch
             a user-defined operatorOP=). $1 is referenced twice — codegen treats
             the lvalue as evaluated once; safe for simple lvalues (IDENT, ARRAY,
             MEMBER, DEREF). */
          $$ = ast_assign_op($2, $1, $3, yylineno);
      }
    ;

assign_op:
      TOK_PLUSEQ      { $$ = OP_ADD; }
    | TOK_MINUSEQ     { $$ = OP_SUB; }
    | TOK_STAREQ      { $$ = OP_MUL; }
    | TOK_SLASHEQ     { $$ = OP_DIV; }
    | TOK_PERCENTEQ   { $$ = OP_MOD; }
    | TOK_AMPEQ       { $$ = OP_BAND; }
    | TOK_PIPEEQ      { $$ = OP_BOR; }
    | TOK_CARETEQ     { $$ = OP_BXOR; }
    | TOK_SHLEQ       { $$ = OP_SHL; }
    | TOK_SHREQ       { $$ = OP_SHR; }
    ;

conditional_expr:
      logical_or_expr                                  { $$ = $1; }
    | logical_or_expr '?' expr ':' conditional_expr    { $$ = ast_ternary($1, $3, $5, yylineno); }
    ;

logical_or_expr:
      logical_and_expr                       { $$ = $1; }
    | logical_or_expr TOK_LOR logical_and_expr { $$ = ast_binop(OP_LOR, $1, $3, yylineno); }
    ;

logical_and_expr:
      bit_or_expr                            { $$ = $1; }
    | logical_and_expr TOK_LAND bit_or_expr  { $$ = ast_binop(OP_LAND, $1, $3, yylineno); }
    ;

bit_or_expr:
      bit_xor_expr                           { $$ = $1; }
    | bit_or_expr '|' bit_xor_expr           { $$ = ast_binop(OP_BOR, $1, $3, yylineno); }
    ;

bit_xor_expr:
      bit_and_expr                           { $$ = $1; }
    | bit_xor_expr '^' bit_and_expr          { $$ = ast_binop(OP_BXOR, $1, $3, yylineno); }
    ;

bit_and_expr:
      equality_expr                          { $$ = $1; }
    | bit_and_expr '&' equality_expr         { $$ = ast_binop(OP_BAND, $1, $3, yylineno); }
    ;

equality_expr:
      relational_expr                        { $$ = $1; }
    | equality_expr TOK_EQ relational_expr   { $$ = ast_binop(OP_EQ, $1, $3, yylineno); }
    | equality_expr TOK_NE relational_expr   { $$ = ast_binop(OP_NE, $1, $3, yylineno); }
    ;

relational_expr:
      shift_expr                             { $$ = $1; }
    | relational_expr '<' shift_expr         { $$ = ast_binop(OP_LT, $1, $3, yylineno); }
    | relational_expr '>' shift_expr         { $$ = ast_binop(OP_GT, $1, $3, yylineno); }
    | relational_expr TOK_LE shift_expr      { $$ = ast_binop(OP_LE, $1, $3, yylineno); }
    | relational_expr TOK_GE shift_expr      { $$ = ast_binop(OP_GE, $1, $3, yylineno); }
    ;

shift_expr:
      additive_expr                          { $$ = $1; }
    | shift_expr TOK_SHL additive_expr       { $$ = ast_binop(OP_SHL, $1, $3, yylineno); }
    | shift_expr TOK_SHR additive_expr       { $$ = ast_binop(OP_SHR, $1, $3, yylineno); }
    ;

additive_expr:
      multiplicative_expr                    { $$ = $1; }
    | additive_expr '+' multiplicative_expr  { $$ = ast_binop(OP_ADD, $1, $3, yylineno); }
    | additive_expr '-' multiplicative_expr  { $$ = ast_binop(OP_SUB, $1, $3, yylineno); }
    ;

multiplicative_expr:
      cast_expr                              { $$ = $1; }
    | multiplicative_expr '*' cast_expr      { $$ = ast_binop(OP_MUL, $1, $3, yylineno); }
    | multiplicative_expr '/' cast_expr      { $$ = ast_binop(OP_DIV, $1, $3, yylineno); }
    | multiplicative_expr '%' cast_expr      { $$ = ast_binop(OP_MOD, $1, $3, yylineno); }
    ;

cast_expr:
      unary_expr                             { $$ = $1; }
    | '(' base_type pointers ')' cast_expr   {
          type *t = apply_pointers($2, $3);
          $$ = ast_cast(t, $5, yylineno);
      }
    | '(' base_type pointers ')' '{' init_item_list '}' {
          /* C99 compound literal (struct/scalar/pointer type) */
          type *t = apply_pointers($2, $3);
          $$ = ast_compound(t, $6, yylineno);
      }
    ;

unary_expr:
      postfix_expr                           { $$ = $1; }
    | KW_CPPCAST '<' base_type pointers '>' '(' expr ')' {
          /* static_cast / reinterpret_cast / const_cast / dynamic_cast<T>(e) */
          $$ = ast_cast(apply_pointers($3, $4), $7, yylineno);
      }
    | TOK_INC unary_expr                     { $$ = ast_xfix(E_PREINC, $2, yylineno); }
    | TOK_DEC unary_expr                     { $$ = ast_xfix(E_PREDEC, $2, yylineno); }
    | '&' cast_expr                          { $$ = ast_addr($2, yylineno); }
    | '*' cast_expr                          { $$ = ast_deref($2, yylineno); }
    | '+' cast_expr                          { $$ = ast_unop(OP_POS, $2, yylineno); }
    | '-' cast_expr                          { $$ = ast_unop(OP_NEG, $2, yylineno); }
    | '~' cast_expr                          { $$ = ast_unop(OP_BNOT, $2, yylineno); }
    | '!' cast_expr                          { $$ = ast_unop(OP_LNOT, $2, yylineno); }
    | KW_SIZEOF unary_expr                   { $$ = ast_sizeof_e($2, yylineno); }
    | KW_SIZEOF '(' base_type pointers ')'   {
          type *t = apply_pointers($3, $4);
          $$ = ast_sizeof_t(t, yylineno);
      }
    | KW_NEW base_type pointers              {
          type *t = apply_pointers($2, $3);
          $$ = ast_new(t, NULL, 0, NULL, yylineno);          /* new T (default ctor) */
      }
    | KW_NEW base_type pointers '(' argument_list ')' {
          type *t = apply_pointers($2, $3);
          $$ = ast_new(t, $5.arr, $5.n, NULL, yylineno);     /* new T(args) */
      }
    | KW_NEW base_type pointers '[' expr ']' {
          type *t = apply_pointers($2, $3);
          $$ = ast_new(t, NULL, 0, $5, yylineno);            /* new T[n] */
      }
    | KW_DELETE cast_expr                    { $$ = ast_delete($2, 0, yylineno); }
    | KW_DELETE '[' ']' cast_expr            { $$ = ast_delete($4, 1, yylineno); }
    ;

postfix_expr:
      primary_expr                           { $$ = $1; }
    | postfix_expr '[' expr ']'              { $$ = ast_index($1, $3, yylineno); }
    | postfix_expr '(' argument_list ')'     { $$ = ast_call($1, $3.arr, $3.n, yylineno); }
    | postfix_expr '.' IDENT                 { $$ = ast_member($1, $3, yylineno); }
    | postfix_expr TOK_ARROW IDENT           { $$ = ast_pmember($1, $3, yylineno); }
    | postfix_expr TOK_INC                   { $$ = ast_xfix(E_POSTINC, $1, yylineno); }
    | postfix_expr TOK_DEC                   { $$ = ast_xfix(E_POSTDEC, $1, yylineno); }
    ;

argument_list:
      /* empty */                            { $$.arr = NULL; $$.n = 0; }
    | non_empty_argument_list                { $$ = $1; }
    ;

non_empty_argument_list:
      assignment_expr                        { $$.arr = malloc(sizeof(expr*)); $$.arr[0] = $1; $$.n = 1; }
    | non_empty_argument_list ',' assignment_expr {
          $$.arr = realloc($1.arr, sizeof(expr*) * ($1.n + 1));
          $$.arr[$1.n] = $3;
          $$.n = $1.n + 1;
      }
    ;

primary_expr:
      IDENT                                  {
          /* enum constants become int literals; everything else is an IDENT */
          sym *s = st_find($1);
          if (s && s->kind == SK_ENUM_CONST) { $$ = ast_int_lit(s->enum_val, yylineno); free($1); }
          else $$ = ast_ident($1, yylineno);
      }
    | INT_LIT                                { $$ = ast_int_lit($1, yylineno); }
    | FLOAT_LIT                              { $$ = ast_float_lit($1, yylineno); }
    | CHAR_LIT                               { $$ = ast_char_lit($1, yylineno); }
    | STRING_LIT                             { $$ = ast_string_lit($1, g_str_len, yylineno); }
    | '(' expr ')'                           { $$ = $2; }
    | qualified_id                           {
          sym *s = st_find($1);
          if (s && s->kind == SK_ENUM_CONST) { $$ = ast_int_lit(s->enum_val, yylineno); free($1); }
          else $$ = ast_ident($1, yylineno);
      }
    | KW_THIS                                { $$ = ast_ident(strdup("this"), yylineno); }
    | '[' ']' '(' ')' { lambda_enter(); } compound_stmt '(' ')' {
          /* lambda IIFE as a global initializer: body -> hidden fn, expr -> call */
          $$ = lambda_finish($6);
      }
    | TEMPLATE_NAME                          { $$ = ast_ident($1, yylineno); }   /* deduced call / use */
    | TEMPLATE_NAME '<' type_arg_list '>' '(' argument_list ')' {
          /* explicit template arguments: fn<T...>(args) */
          expr *c = ast_call(ast_ident($1, yylineno), $6.arr, $6.n, yylineno);
          c->gtypes = $3.arr; c->n_gtypes = $3.n;
          $$ = c;
      }
    | IDENT TOK_SCOPE TEMPLATE_NAME '<' type_arg_list '>' '(' argument_list ')' {
          /* namespace-qualified explicit template call N::fn<T...>(args) */
          free($1);
          expr *c = ast_call(ast_ident($3, yylineno), $8.arr, $8.n, yylineno);
          c->gtypes = $5.arr; c->n_gtypes = $5.n;
          $$ = c;
      }
    | IDENT TOK_SCOPE TEMPLATE_NAME          { free($1); $$ = ast_ident($3, yylineno); }  /* N::fn deduced */
    | TYPEDEF_NAME '(' argument_list ')'     {
          /* T(args) — a temporary object constructed on the stack */
          sym *s = st_find($1);
          type *t = (s && s->kind == SK_TYPEDEF) ? s->stype : NULL;
          if (!t) msg_error(yylineno, "unknown type '%s'", $1);
          $$ = ast_tempobj(t, $3.arr, $3.n, yylineno);
          free($1);
      }
    | TYPEDEF_NAME TOK_SCOPE IDENT           {
          /* Class::member — a scoped enumerator (E::name) or a static data member */
          char *m = mangle_method($1, $3);
          sym *s = st_find(m);
          if (s && s->kind == SK_ENUM_CONST) { $$ = ast_int_lit(s->enum_val, yylineno); free(m); }
          else $$ = ast_ident(m, yylineno);
          free($1); free($3);
      }
    | TYPEDEF_NAME '<' ct_mixed_arg_list '>' TOK_SCOPE IDENT '(' argument_list ')' {
          /* `Class<args>::static_method(args)` — static call on a class-template
             instantiation. Instantiate the class (so its cloned methods get the
             concrete mangled tag), then build a call to `<concrete_tag>__method`. */
          ctmpl *ct = find_ctmpl($1);
          const char *cls_tag = $1;
          if (ct) {
              type *inst = instantiate_ctmpl_mixed(ct, $3.targs, $3.vals, $3.isval, $3.n);
              if (inst && inst->tag) cls_tag = inst->tag;
          }
          char *mname = mangle_method(cls_tag, $6);
          $$ = ast_call(ast_ident(mname, yylineno), $8.arr, $8.n, yylineno);
          free($1); free($6);
      }
    | IDENT TOK_SCOPE TYPEDEF_NAME '<' ct_mixed_arg_list '>' TOK_SCOPE IDENT '(' argument_list ')' {
          /* `N::Class<args>::static_method(args)` — namespace transparent. */
          free($1);
          ctmpl *ct = find_ctmpl($3);
          const char *cls_tag = $3;
          if (ct) {
              type *inst = instantiate_ctmpl_mixed(ct, $5.targs, $5.vals, $5.isval, $5.n);
              if (inst && inst->tag) cls_tag = inst->tag;
          }
          char *mname = mangle_method(cls_tag, $8);
          $$ = ast_call(ast_ident(mname, yylineno), $10.arr, $10.n, yylineno);
          free($3); free($8);
      }
    | KW_GENERIC '(' assignment_expr ',' generic_assoc_list ')' {
          $$ = ast_generic($3, $5.ts, $5.es, $5.n, yylineno);
      }
    ;

/* C11 type-generic selection associations */
generic_assoc:
      base_type pointers ':' assignment_expr   { $$.t = apply_pointers($1, $2); $$.e = $4; }
    | KW_DEFAULT ':' assignment_expr           { $$.t = NULL; $$.e = $3; }   /* default */
    ;

generic_assoc_list:
      generic_assoc {
          $$.ts = malloc(sizeof(type*)); $$.es = malloc(sizeof(expr*));
          $$.ts[0] = $1.t; $$.es[0] = $1.e; $$.n = 1;
      }
    | generic_assoc_list ',' generic_assoc {
          $$.ts = realloc($1.ts, sizeof(type*) * ($1.n + 1));
          $$.es = realloc($1.es, sizeof(expr*) * ($1.n + 1));
          $$.ts[$1.n] = $3.t; $$.es[$1.n] = $3.e; $$.n = $1.n + 1;
      }
    ;

%%

void yyerror(const char *s)
{
    msg_error(yylineno, "%s", s);
}
