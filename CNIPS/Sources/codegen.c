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
static int   g_nubits  = 16;     // effective word width (for unsigned compares)
static char *cur_func_name = NULL;
static type *cur_func_ret  = NULL;

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
static void loop_push(char *c, char *b)
{
    if (loop_top >= MAX_LOOPS) msg_internal("loop nesting too deep");
    loop_stk[loop_top].cont_l = c; loop_stk[loop_top].break_l = b; loop_top++;
}
static void loop_pop(void) { loop_top--; }

// switches need their own break stack (continue isn't allowed but pop expects same)
static char *sw_break_stk[MAX_LOOPS];
static int   sw_top = 0;

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

// ---- forward decls ---------------------------------------------------------

static void gen_expr (expr *e);
static void gen_addr (expr *e);
static void gen_store(expr *lv, expr *val);
static void gen_stmt (stmt *s);
static void gen_bool (expr *e, const char *jz_target);
static type *infer_type(expr *e);

// ---- string-literal pre-scan (assigns each "..." a global label) -----------

static void scan_strings_stmt(stmt *s);

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
    scan_strings_expr(e->a);
    scan_strings_expr(e->b);
    scan_strings_expr(e->c);
    for (int i = 0; i < e->n_args; i++) scan_strings_expr(e->args[i]);
}

static void scan_strings_stmt(stmt *s)
{
    if (!s) return;
    scan_strings_expr(s->e1); scan_strings_expr(s->e2); scan_strings_expr(s->e3);
    scan_strings_stmt(s->body); scan_strings_stmt(s->body2); scan_strings_stmt(s->init_stmt);
    for (int i = 0; i < s->n_items; i++) scan_strings_stmt(s->items[i]);
    for (decl *d = s->decls; d; d = d->next) {
        scan_strings_expr(d->init);
        for (int i = 0; i < d->n_init; i++) scan_strings_expr(d->init_list[i]);
    }
}

// ---- address-taken-function pre-scan ---------------------------------------
// Any function name used as a value (not as the immediate callee of a direct
// call) is address-taken and needs a dispatch-table slot.

static void scan_fp_stmt(stmt *s);

static void scan_fp_expr(expr *e)
{
    if (!e) return;
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

static void scan_fp_stmt(stmt *s)
{
    if (!s) return;
    scan_fp_expr(s->e1); scan_fp_expr(s->e2); scan_fp_expr(s->e3);
    scan_fp_stmt(s->body); scan_fp_stmt(s->body2); scan_fp_stmt(s->init_stmt);
    for (int i = 0; i < s->n_items; i++) scan_fp_stmt(s->items[i]);
    for (decl *d = s->decls; d; d = d->next) {
        scan_fp_expr(d->init);
        for (int i = 0; i < d->n_init; i++) scan_fp_expr(d->init_list[i]);
    }
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
        sym *s = st_find(e->sval);
        if (!s) msg_error(e->line, "undefined '%s'", e->sval);
        if (s->kind == SK_FUNC) e->etype = s->stype;
        else                    e->etype = s->stype;
        break;
    }
    case E_BINOP: {
        type *lt = infer_type(e->a);
        type *rt = infer_type(e->b);
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
        if (e->a->kind == E_IDENT) {
            const char *fn = e->a->sval;
            // builtins: in() returns int, out() returns void; both are not "declared" identifiers
            if (!strcmp(fn, "in"))       { e->etype = t_int();  break; }
            if (!strcmp(fn, "out"))      { e->etype = t_void(); break; }
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

static void gen_addr(expr *e)
{
    switch (e->kind) {
    case E_IDENT: {
        sym *s = st_find(e->sval);
        if (!s) msg_error(e->line, "undefined '%s'", e->sval);
        // for an array param: the variable stores the address, so loading
        // its value is &arr[0]
        if (s->kind == SK_PARAM && s->stype && s->stype->kind == TY_ARRAY)
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

// ---- store: lv = val -------------------------------------------------------

static void gen_store(expr *lv, expr *val)
{
    // reject assignment to a const variable (direct, by-name)
    if (lv->kind == E_IDENT) {
        sym *cs = st_find(lv->sval);
        if (cs && cs->is_const) msg_error(lv->line, "assignment to const '%s'", lv->sval);
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
        if (s && s->stype && s->stype->kind != TY_ARRAY && s->stype->kind != TY_STRUCT) {
            gen_expr(val);
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
                gen_expr(val);                // value → acc
                emit("STI %s", s->asm_name);
                return;
            }
        }
    }
    // general path via STA
    gen_addr(lv);
    emit("PSH");
    gen_expr(val);
    emit("STA");
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
        sym *s = st_find(e->sval);
        if (!s) msg_error(e->line, "undefined '%s'", e->sval);
        if (s->kind == SK_FUNC) {
            // function used as a value -> its dispatch-table ID (function ptr)
            int id = fp_id_of(e->sval);
            if (id < 0) msg_internal("function '%s' not in dispatch table", e->sval);
            emit("LOD %d", id);
            return;
        }
        if (s->stype && s->stype->kind == TY_ARRAY) {
            // array decays to base address
            if (s->kind == SK_PARAM) emit("LOD %s", s->asm_name);
            else                     emit("LEA %s", s->asm_name);
            return;
        }
        if (s->stype && s->stype->kind == TY_STRUCT)
            msg_error(e->line, "struct by-value not supported in v1");
        emit("LOD %s", s->asm_name);
        return;
    }

    case E_ADDR:
        gen_addr(e->a);
        return;

    case E_DEREF:
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
        int is_float = (e->etype && e->etype->kind == TY_FLOAT);
        // operand signedness (for >> and comparisons)
        type *lt = infer_type(e->a);
        type *rt = infer_type(e->b);
        int lhs_uns = lt && lt->kind == TY_INT && !lt->is_signed;
        int uns_cmp = (lhs_uns) || (rt && rt->kind == TY_INT && !rt->is_signed);

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
        if (e->b->kind == E_IDENT) {
            sym *r = st_find(e->b->sval);
            if (r && r->stype && r->stype->kind != TY_ARRAY && r->stype->kind != TY_STRUCT) {
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
        // general path
        gen_expr(e->a); emit("PSH"); gen_expr(e->b);
        switch (op) {
            case OP_ADD: emit(is_float ? "SF_ADD" : "S_ADD"); return;
            case OP_SUB: if (is_float) emit("SF_SU1"); else { emit("NEG"); emit("S_ADD"); } return;
            case OP_MUL: emit(is_float ? "SF_MLT" : "S_MLT"); return;
            case OP_DIV: emit(is_float ? "SF_DIV" : "S_DIV"); return;
            case OP_MOD: emit("S_MOD"); return;
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
            if (s && s->stype && s->stype->kind != TY_ARRAY && s->stype->kind != TY_STRUCT) {
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
            if (s && s->stype && s->stype->kind != TY_ARRAY && s->stype->kind != TY_STRUCT) {
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

    case E_SIZEOF_T: emit_load_int(type_size_words(e->target_t)); return;
    case E_SIZEOF_E: emit_load_int(type_size_words(infer_type(e->a))); return;

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
            sym *s = st_find(fn);
            if (s && s->kind == SK_FUNC) {
                for (int i = 0; i < e->n_args; i++) { gen_expr(e->args[i]); emit("PSH"); }
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
            for (int i = 0; i < e->n_args; i++) { gen_expr(e->args[i]); emit("PSH"); }
            gen_expr(fpv);                          // acc = function ID
            emit("SET _fp_id");
            char *done = fresh_label("ic_done");
            for (int i = 0; i < fptab_n; i++) {
                char *skip = fresh_label("ic_s");
                emit("LOD _fp_id");
                emit("EQU %d", i);                  // 1 if this ID matches
                emit("JIZ %s", skip);               // no match -> next candidate
                emit("CAL %s", fptab[i]);           // match -> direct (depth-1) call
                emit("JMP %s", done);
                emit("@%s NOP", skip);
                free(skip);
            }
            emit("@%s NOP", done);                  // result (if any) left in acc
            free(done);
            return;
        }
    }
    }
    msg_internal("unhandled expr kind %d", e->kind);
}

// ---- statement codegen -----------------------------------------------------

// emit element-by-element stores for an aggregate initialiser {a,b,c}.
// `base` is the array/struct base name as it appears in the asm (declared via
// #array). Handles a 1-D scalar array or a struct of scalar fields; each store
// is `LOD <offset>; PSH; <value>; STI base` (mem[base+offset] = value).
static void emit_aggregate_init(const char *base, type *t, expr **list, int n)
{
    if (t->kind == TY_ARRAY) {
        int elem = type_size_words(t->base);
        for (int k = 0; k < n; k++) {
            emit("LOD %d", k * elem);
            emit("PSH");
            infer_type(list[k]);
            gen_expr(list[k]);
            emit("STI %s", base);
        }
    } else if (t->kind == TY_STRUCT) {
        strct_field *f = t->fields;
        for (int k = 0; k < n && f; k++, f = f->next) {
            emit("LOD %d", f->offset);
            emit("PSH");
            infer_type(list[k]);
            gen_expr(list[k]);
            emit("STI %s", base);
        }
    } else {
        msg_error(list[0]->line, "aggregate initializer requires an array or struct");
    }
}

static void declare_local(decl *d)
{
    char *aname = mangle_local(d->name);
    { sym *ls = st_add(SK_LOCAL_VAR, d->name, aname, d->dtype); ls->is_const = d->is_const; }
    int arr_words = (d->dtype && d->dtype->kind == TY_ARRAY) ? type_size_words(d->dtype) : 0;
    log_var(cur_func_name ? cur_func_name : "global", d->name,
            innermost_code(d->dtype), arr_words);
    if (d->dtype && d->dtype->kind == TY_ARRAY) {
        // multi-dim arrays flatten to total word count; element type is the innermost scalar
        if (d->init_file) emit("#arrays %s %d %d \"%s\"", aname, innermost_code(d->dtype), arr_words, d->init_file);
        else              emit("#array %s %d %d",         aname, innermost_code(d->dtype), arr_words);
        if (d->init_list) emit_aggregate_init(aname, d->dtype, d->init_list, d->n_init);
    } else if (d->dtype && d->dtype->kind == TY_STRUCT) {
        emit("#array %s 1 %d", aname, type_size_words(d->dtype));
        if (d->init_list) emit_aggregate_init(aname, d->dtype, d->init_list, d->n_init);
    } else if (d->init) {
        infer_type(d->init);
        gen_expr(d->init);
        emit("SET %s", aname);
    }
    free(aname);
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
        if (loop_top > 0)      emit("JMP %s", loop_stk[loop_top-1].break_l);
        else if (sw_top > 0)   emit("JMP %s", sw_break_stk[sw_top-1]);
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
            gen_expr(s->e1);
        }
        if (cur_func_name && strcmp(cur_func_name, "main") == 0) emit("JMP fim");
        else emit("RET");
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
        // chain-of-ifs implementation: evaluate discriminant once into a temp,
        // emit one EQU per case, fall through naturally.
        infer_type(s->e1);
        // allocate a temp local for the discriminant
        char tmp[64]; snprintf(tmp, sizeof(tmp), "_sw_%d", ++label_n);
        char *tmp_name = mangle_local(tmp);
        st_add(SK_LOCAL_VAR, tmp, tmp_name, t_int());
        log_var(cur_func_name ? cur_func_name : "global", tmp, 1, 0);
        gen_expr(s->e1);
        emit("SET %s", tmp_name);

        char *end = fresh_label("sw_end");
        sw_break_stk[sw_top++] = end;

        // first pass: collect cases (and default) labels, emit the dispatch chain
        // we expect s->body to be a block of items (or a single stmt).
        // walk block items: for each S_CASE or S_DEFAULT we mint a label;
        // for non-case stmts inside, we emit them inline AFTER the dispatch.
        // Approach: split body into (case_match_chain) + (body with @case_X labels)
        stmt *body = s->body;
        stmt **items; int n_items;
        if (body && body->kind == S_BLOCK) { items = body->items; n_items = body->n_items; }
        else { items = &body; n_items = body ? 1 : 0; }

        // pre-mint labels
        char **case_labels = calloc(n_items, sizeof(char*));
        char *default_label = NULL;
        for (int i = 0; i < n_items; i++) {
            if (items[i]->kind == S_CASE) case_labels[i] = fresh_label("case");
            else if (items[i]->kind == S_DEFAULT) { default_label = fresh_label("default"); case_labels[i] = default_label; }
        }
        // dispatch chain
        for (int i = 0; i < n_items; i++) {
            if (items[i]->kind == S_CASE) {
                emit("LOD %s", tmp_name);
                emit("EQU %ld", items[i]->e1->ival);
                emit("LIN");
                emit("JIZ %s", case_labels[i]);
                // double-LIN means it inverts twice; after EQU, value is already 0/1. After single LIN, 1↔0. So `JIZ case` jumps when LIN→0, i.e., when EQU was 1, i.e., when equal. ✓
            }
        }
        if (default_label) emit("JMP %s", default_label);
        else               emit("JMP %s", end);

        // bodies, with labels at case points; fallthrough is automatic
        st_push_scope();
        for (int i = 0; i < n_items; i++) {
            if (items[i]->kind == S_CASE || items[i]->kind == S_DEFAULT) {
                emit("@%s NOP", case_labels[i]);
                gen_stmt(items[i]->body);
            } else {
                gen_stmt(items[i]);
            }
        }
        st_pop_scope();

        emit("@%s NOP", end);
        sw_top--;
        free(case_labels);
        free(end);
        free(tmp_name);
        return;
    }

    case S_CASE: case S_DEFAULT:
        msg_error(s->line, "case/default outside switch");
        return;
    }
}

// ---- top-level emission ----------------------------------------------------

static int has_main = 0;

static void emit_header(unit *u)
{
    emit("NOP");
    emit("#PRNAME %s", u->prname ? u->prname : "prog");
    emit("#NUBITS %d", u->nubits >= 0 ? u->nubits : CFG_NUBITS);
    emit("#NDSTAC %d", u->ndstac >= 0 ? u->ndstac : CFG_NDSTAC);
    emit("#SDEPTH %d", u->sdepth >= 0 ? u->sdepth : CFG_SDEPTH);
    emit("#NUIOIN %d", u->nuioin >= 0 ? u->nuioin : CFG_NUIOIN);
    emit("#NUIOOU %d", u->nuioou >= 0 ? u->nuioou : CFG_NUIOOU);
    emit("#NBMANT %d", u->nbmant >= 0 ? u->nbmant : CFG_NBMANT);
    emit("#NBEXPO %d", u->nbexpo >= 0 ? u->nbexpo : CFG_NBEXPO);
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
            if (d->init_list) emit_aggregate_init(d->name, d->dtype, d->init_list, d->n_init);
            continue;
        }
        if (!d->init) continue;
        infer_type(d->init);
        gen_expr(d->init);
        emit("SET %s", d->name);
    }
}

static void emit_function(func *f, unit *u, int is_main)
{
    cur_func_name = f->name;
    cur_func_ret  = f->ret;
    st_enter_func(f->name);
    st_push_scope();

    // register params in local scope with mangled names
    for (decl *p = f->params; p; p = p->next) {
        char *aname = mangle_local(p->name);
        st_add(SK_PARAM, p->name, aname, p->dtype);
        log_var(f->name, p->name, type_code_for(p->dtype),
                p->dtype && p->dtype->kind == TY_ARRAY ? p->dtype->arr_size : 0);
        free(aname);
    }

    emit("@%s NOP", f->name);

    // pop args in reverse order (caller pushes left-to-right)
    if (f->n_params > 0) {
        decl *plist[64]; int np = 0;
        for (decl *p = f->params; p && np < 64; p = p->next) plist[np++] = p;
        for (int i = np - 1; i >= 0; i--) {
            sym *sy = st_find(plist[i]->name);
            emit("POP");
            emit("SET %s", sy->asm_name);
        }
    }

    if (is_main) { emit_string_inits(); emit_global_scalar_inits(u); }

    gen_stmt(f->body);

    if (!is_main) emit("RET");

    st_pop_scope();
    st_leave_func();
    cur_func_name = NULL;
    cur_func_ret  = NULL;
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

void codegen(FILE *out_file, unit *u, const char *tmp_dir)
{
    out_f = out_file;
    ins_count = 0; label_n = 0; varlog_n = 0; has_main = 0; strtab_n = 0; fptab_n = 0;
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

    // pre-scan every function body and global initialiser for string literals
    // and address-taken functions before any codegen emits a reference.
    for (int i = 0; i < u->n_funcs; i++) { scan_strings_stmt(u->funcs[i]->body); scan_fp_stmt(u->funcs[i]->body); }
    for (int i = 0; i < u->n_globals; i++) {
        scan_strings_expr(u->globals[i]->init);
        scan_fp_expr(u->globals[i]->init);
        for (int k = 0; k < u->globals[i]->n_init; k++) { scan_strings_expr(u->globals[i]->init_list[k]); scan_fp_expr(u->globals[i]->init_list[k]); }
    }

    emit_header(u);
    emit_global_arrays(u);
    emit_string_arrays();
    emit("JMP main");

    for (int i = 0; i < u->n_funcs; i++) {
        if (strcmp(u->funcs[i]->name, "main") != 0) emit_function(u->funcs[i], u, 0);
    }
    for (int i = 0; i < u->n_funcs; i++) {
        if (strcmp(u->funcs[i]->name, "main") == 0) emit_function(u->funcs[i], u, 1);
    }

    emit("@fim JMP fim");

    if (tmp_dir) write_cmm_log(tmp_dir);
}
