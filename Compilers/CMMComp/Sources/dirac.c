// ----------------------------------------------------------------------------
// the Dirac notation, computed in loops ---------------------------------------
// ----------------------------------------------------------------------------
//
// docs/design/dirac-expressions.md, step (b). A Dirac expression is checked
// for shape by ast.c (shapecheck_expr); here it becomes loops with today's
// instructions:
//
// - every element of the result is a scalar expression tree, built from the
//   Dirac tree for one position (row, col) of the result: a ket |a⟩ at row r
//   is a[r], |M| is M[r][c], c X is X(r, c)*c, a product with an inner size
//   k > 1 is a sum over k (EXPR_DIRAC_SUM). The scalar walker (ast_emit_expr)
//   emits that tree, with its types and its mixing of int and float; the
//   array reads are EXPR_DIRAC_ELEM leaves, emitted here;
// - a loop counter runs from -n*step up to 0, so that the JIZ after its
//   increment closes the loop with no comparison; the base of each array is
//   moved to its end by the constant offset of LDI_V / STI_V;
// - an index of a matrix is a sum of loop variables, never a multiplication:
//   a loop whose counter steps by the row length walks the rows, and a
//   variable derived from a loop steps by another stride when needed;
// - the sum of a product stays on the stack; its first term is taken out of
//   the loop, with constant addresses;
// - a scalar part (a coefficient, a function call) is computed once, before
//   the loops, and a part that does not change in the inner loop (K[r] in
//   |K⟩⟨Px|) once per turn of the outer one;
// - when the target is read at another place than the one written (a # |M|a⟩)
//   or a product inside a chain would be computed again for every element
//   (|A|B|x⟩), the result goes through one area, __dirac_tmp, shared by every
//   statement of the program (section 3.4, requirement 6).

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../Headers/ast.h"
#include "../Headers/global.h"
#include "../Headers/variaveis.h"
#include "../Headers/diretivas.h"
#include "../Headers/funcoes.h"
#include "../Headers/oper.h"
#include "../Headers/stdlib.h"
#include "../Headers/messages.h"
#include "../Headers/dirac.h"

// ----------------------------------------------------------------------------
// state ----------------------------------------------------------------------
// ----------------------------------------------------------------------------

#define DL_MAXLOOP 64
#define DL_MAXSTR  8
#define DL_MAXHOI  64

// a loop: its counter __dk<num> runs from -n*step up to 0 in steps of step;
// the index it stands for is first + the turn (0 .. n-1)
typedef struct {
    int n, first, step;
    int num;
    int ns, st[DL_MAXSTR]; long w[DL_MAXSTR]; // strides the index is used with, and how often
    int nd, ds[DL_MAXSTR], dv[DL_MAXSTR];     // derived variables __dd<dv>: value stride*turn - stride*n
    int nh; expr_node **hn[DL_MAXHOI]; int hv[DL_MAXHOI]; // parts computed at the top of each turn
} dl_loop;

// an index: sum of st[i] * (the index of loop lv[i]) + k
typedef struct { int nt; int lv[2]; int st[2]; long k; } dl_ix;

// a position in a row or a column: loop lv (< 0: none) plus k
typedef struct { int lv; int k; } dl_pos;

typedef struct { int arr; dl_ix ix; } dl_elem;               // EXPR_DIRAC_ELEM
typedef struct { int lv; expr_node *first, *body, *src; } dl_sum; // EXPR_DIRAC_SUM
typedef struct { int off; } dl_tmpref;                       // a REF to a block of __dirac_tmp

static dl_loop loops[DL_MAXLOOP];
static int     n_loops;
static int     n_names;          // __dk / __dd / __dh numbers of the statement
static int     depth;            // statements in the making (a hoist can hold another)
static int     label_n;          // __Ld<n>, never reused
static int     tmp_at;           // first free word of __dirac_tmp in this statement
static int     tmp_max;          // words of __dirac_tmp the program needs
static int     tmp_type = 1;     // 2 when a float block was used
static int     tmp_id   = -1;    // v_table id of __dirac_tmp
static int     ctx_type;         // element type of the expression in the making
static int     cur_line;
static int     flat_mode;        // a matrix is read over one flat index
static int     cur_dst = -1;     // the array the statement writes (-1: none)

// the first cell of an array as a scalar operand: a v_table entry with the
// array's name (variaveis.c keeps the count)
extern int v_count;
#define DL_MAXALIAS 256
static int alias_arr[DL_MAXALIAS], alias_id[DL_MAXALIAS], n_alias;

// the statement-level parts computed before the loops
static expr_node **hoist_pre[DL_MAXHOI]; static int hoist_pre_v[DL_MAXHOI]; static int n_hoist_pre;

// what the accumulator holds right now: a sum of loop variables (key), valid
// while no instruction has been emitted since (num_ins == acc_mark)
static char acc_key[128];
static int  acc_mark = -1;

// the lines that went through __dirac_tmp, for the summary
#define DL_MAXLINES 64
static int tmp_lines[DL_MAXLINES], tmp_words[DL_MAXLINES], n_tmp_lines;

// ----------------------------------------------------------------------------
// small helpers --------------------------------------------------------------
// ----------------------------------------------------------------------------

static void die(const char *fmt, int line) {fprintf(stderr, fmt, line); exit(EXIT_FAILURE);}

static expr_node *node_new(expr_kind k, int type)
{
    expr_node *n = calloc(1, sizeof(*n));
    if (!n) {fprintf(stderr, MSG_ERR_OUT_OF_MEMORY); exit(EXIT_FAILURE);}
    n->kind = k;
    n->line = cur_line;
    n->type = type;
    n->rows = n->cols = 1;
    return n;
}

static void key_set(const char *k) {snprintf(acc_key, sizeof acc_key, "%s", k); acc_mark = num_ins;}
static int  key_is (const char *k) {return acc_mark == num_ins && strcmp(acc_key, k) == 0;}

static void ctr_name(char *b, size_t sz, int lv) {snprintf(b, sz, "__dk%d", loops[lv].num);}

// a scalar variable of the compiler, of type t, as a v_table entry
static int scalar_var(int t)
{
    char name[32]; snprintf(name, sizeof name, "__dh%c%d", t == 1 ? 'i' : 'f', n_names++);
    int id = find_var(name);
    if (id < 0) {add_var(name); id = find_var(name);}
    v_table[id].type = t;
    v_table[id].used = 1;
    return id;
}

static int new_loop(int n, int first)
{
    if (n_loops == DL_MAXLOOP) die(MSG_ERR_DIRAC_NOT_YET, cur_line);
    dl_loop *l = &loops[n_loops];
    memset(l, 0, sizeof *l);
    l->n = n; l->first = first; l->step = 1; l->num = n_names++;
    return n_loops++;
}

static int is_num(const expr_node *n) {return n->rows == 1 && n->cols == 1;}

// ----------------------------------------------------------------------------
// building the element ------------------------------------------------------
// ----------------------------------------------------------------------------

static void ix_add(dl_ix *ix, dl_pos p, int stride)
{
    if (p.lv < 0) {ix->k += (long)stride * p.k; return;}
    ix->lv[ix->nt] = p.lv; ix->st[ix->nt] = stride; ix->nt++;
    ix->k += (long)stride * p.k;
}

static expr_node *elem_new(int arr, int type, dl_ix ix)
{
    expr_node *n = node_new(EXPR_DIRAC_ELEM, type);
    dl_elem *e = calloc(1, sizeof *e);
    e->arr = arr; e->ix = ix;
    n->dl = e;
    n->id = arr;
    return n;
}

// the cell of an array (or of a block of __dirac_tmp) at (r, c)
static expr_node *elem_of(expr_node *ref, dl_pos r, dl_pos c)
{
    dl_ix ix = {0};
    int arr = ref->id, cols;
    if (ref->dl) {                                   // a block of __dirac_tmp
        ix.k = ((dl_tmpref *)ref->dl)->off;
        cols = ref->cols;
    } else cols = v_table[arr].siz2;

    if      (ref->op == DIRAC_ROLE_KET) ix_add(&ix, r, 1);
    else if (ref->op == DIRAC_ROLE_BRA) ix_add(&ix, c, 1);
    else if (flat_mode) ix_add(&ix, r, 1);           // r is the flat index
    else {ix_add(&ix, r, cols); ix_add(&ix, c, 1);}

    int type = ref->dl ? ctx_type : v_table[arr].type;
    if (type > 2) die(MSG_ERR_NOT_IMPL_COMPLEX, cur_line);
    v_table[arr].used = 1;

    // the first cell of an array of the program is the array's own name: a
    // memory operand (F_MLT x), with no load of its own. Not the target's,
    // which is checked for being read at other cells (reads_elsewhere)
    if (ix.nt == 0 && ix.k == 0 && !ref->dl && arr != cur_dst) {
        int id = -1;
        for (int j = 0; j < n_alias; j++) if (alias_arr[j] == arr) id = alias_id[j];
        if (id < 0) {
            if (n_alias == DL_MAXALIAS) return elem_new(arr, type, ix);
            add_var(v_table[arr].name);                // the same name: find_var still finds the array first
            id = v_count - 1;
            v_table[id].type = type;
            v_table[id].used = 1;
            alias_arr[n_alias] = arr; alias_id[n_alias++] = id;
        }
        return expr_var(type, id);
    }
    return elem_new(arr, type, ix);
}

static void hoist_pre_add(expr_node **slot, int v)
{
    if (n_hoist_pre == DL_MAXHOI) die(MSG_ERR_DIRAC_NOT_YET, cur_line);
    hoist_pre[n_hoist_pre] = slot; hoist_pre_v[n_hoist_pre++] = v;
}

// a number inside the Dirac expression: a literal or a variable stays as it
// is, anything else is computed before the loops; an int where the elements
// are float is converted once, there
static expr_node *scalar_of(expr_node *n)
{
    typecheck_expr(n);
    if (n->type > 2) die(MSG_ERR_NOT_IMPL_COMPLEX, cur_line);
    int conv = (n->type == 1 && ctx_type == 2);

    if (n->kind == EXPR_LITERAL && !conv) return expr_lit(n->type, n->id);
    if (n->kind == EXPR_VAR     && !conv) {v_table[n->id].used = 1; return expr_var(n->type, n->id);}
    if (n->kind == EXPR_LITERAL) {
        long k = atol(v_table[n->id].name);
        if (k < (1L << nbmant)) {
            char b[32]; snprintf(b, sizeof b, "%ld.0", k);
            return expr_lit(2, exec_fnum(b));
        }
    }

    // computed before the loops into a variable of the compiler, once
    for (int j = 0; j < n_hoist_pre; j++)
        if (*hoist_pre[j] == n) return expr_var(conv ? 2 : n->type, hoist_pre_v[j] & ~(1 << 30));
    int t = conv ? 2 : n->type;
    int v = scalar_var(t);
    expr_node **slot = malloc(sizeof *slot);
    *slot = n;
    hoist_pre_add(slot, v | (conv << 30));
    return expr_var(t, v);
}

static expr_node *zero_of(int t)
{
    return t == 2 ? expr_lit(2, exec_fnum("0.0")) : expr_lit(1, exec_inum("0"));
}

static expr_node *build(expr_node *n, dl_pos r, dl_pos c, int top);

static expr_node *mul(expr_node *a, expr_node *b) {return expr_binop(OP_MUL, a, b);}

static expr_node *build(expr_node *n, dl_pos r, dl_pos c, int top)
{
    dl_pos z = {-1, 0};

    switch (n->kind)
    {
        case EXPR_DIRAC_REF:   return elem_of(n, r, c);
        case EXPR_DIRAC_CONST:
            if (n->op == DIRAC_ZERO) return zero_of(ctx_type);
            die(MSG_ERR_DIRAC_NOT_YET, cur_line);   // |I| only on its own (dl_eye)
            break;
        case EXPR_DIRAC_IN:
            return expr_stdlib(ctx_type == 2 ? OP_STD_FIN : OP_STD_IN, n->id, NULL, NULL);
        case EXPR_BINOP:
            if (is_num(n)) break;
            return expr_binop(n->op, build(n->left, r, c, 0), build(n->right, r, c, 0));
        case EXPR_UNOP:
            if (is_num(n)) break;
            return expr_unop(n->op, build(n->left, r, c, 0));
        case EXPR_DIRAC_PROD: {
            if (is_num(n) && !top) break;
            expr_node *L = n->left, *R = n->right;
            // the scalar second, as the generators of before: v[k]*c
            if (is_num(L)) return mul(build(R, r, c, 0), scalar_of(L));
            if (is_num(R)) return mul(build(L, r, c, 0), scalar_of(R));
            int k = (L->cols != DIRAC_ANY) ? L->cols : R->rows;
            if (k == DIRAC_ANY) die(MSG_ERR_DIRAC_NOT_YET, cur_line);
            expr_node *first = mul(build(L, r, z, 0), build(R, z, c, 0));
            if (k == 1) return first;                          // an outer product
            if (k == 2) {                                      // two terms: no loop
                dl_pos one = {-1, 1};
                return expr_binop(OP_ADD, first, mul(build(L, r, one, 0), build(R, one, c, 0)));
            }
            int lv = new_loop(k - 1, 1);
            dl_pos p = {lv, 0};
            expr_node *s = node_new(EXPR_DIRAC_SUM, 0);
            dl_sum *d = calloc(1, sizeof *d);
            d->lv = lv; d->first = first; d->src = n;
            d->body = mul(build(L, r, p, 0), build(R, p, c, 0));
            typecheck_expr(d->first);
            typecheck_expr(d->body);
            s->type = d->first->type;
            s->dl = d;
            return s;
        }
        default: break;
    }
    return scalar_of(n);
}

// ----------------------------------------------------------------------------
// what each part depends on --------------------------------------------------
// ----------------------------------------------------------------------------
// deps: a bit per loop; volatile: reads an input (must run where it is)

typedef unsigned long long dl_set;

static dl_set deps(expr_node *n, int *vol)
{
    if (!n) return 0;
    dl_set s = 0;
    if (n->kind == EXPR_DIRAC_ELEM) {
        dl_elem *e = n->dl;
        for (int i = 0; i < e->ix.nt; i++) s |= 1ULL << e->ix.lv[i];
        return s;
    }
    if (n->kind == EXPR_DIRAC_SUM) {
        dl_sum *d = n->dl;
        s = deps(d->first, vol) | (deps(d->body, vol) & ~(1ULL << d->lv));
        return s;
    }
    if (n->kind == EXPR_STDLIB_CALL && (n->op == OP_STD_IN || n->op == OP_STD_FIN)) *vol = 1;
    if (n->kind == EXPR_FUNC_CALL) *vol = 1;
    s |= deps(n->left, vol) | deps(n->right, vol);
    for (int i = 0; i < n->n_args; i++) s |= deps(n->args[i], vol);
    return s;
}

// a SUM that does not depend on every loop around it would be computed again
// for nothing: returns it, so that its product goes to __dirac_tmp first
static expr_node *find_waste(expr_node *n, dl_set around)
{
    if (!n) return NULL;
    if (n->kind == EXPR_DIRAC_SUM) {
        dl_sum *d = n->dl;
        int vol = 0;
        dl_set s = deps(n, &vol);
        if ((around & ~s) != 0 && !vol) return n;
        expr_node *w = find_waste(d->first, around);
        return w ? w : find_waste(d->body, around | (1ULL << d->lv));
    }
    expr_node *w = find_waste(n->left, around);
    if (!w) w = find_waste(n->right, around);
    for (int i = 0; !w && i < n->n_args; i++) w = find_waste(n->args[i], around);
    return w;
}

// moves the parts that do not change in the innermost loop around them to
// the top of the deepest loop they depend on. path: the loops around, outer
// first
static void hoist_inv(expr_node **slot, int *path, int np)
{
    expr_node *n = *slot;
    if (!n) return;
    if (n->kind == EXPR_LITERAL || n->kind == EXPR_VAR) return;

    int vol = 0;
    dl_set s = deps(n, &vol);
    if (np > 0 && !vol && !(s & (1ULL << path[np - 1]))) {
        int at = -1;                                     // the deepest loop it depends on
        for (int i = np - 1; i >= 0; i--) if (s & (1ULL << path[i])) {at = path[i]; break;}
        typecheck_expr(n);
        int v = scalar_var(n->type);
        expr_node **keep = malloc(sizeof *keep);
        *keep = n;
        if (at < 0) hoist_pre_add(keep, v);
        else {
            dl_loop *l = &loops[at];
            if (l->nh == DL_MAXHOI) die(MSG_ERR_DIRAC_NOT_YET, cur_line);
            l->hn[l->nh] = keep; l->hv[l->nh++] = v;
        }
        *slot = expr_var(n->type, v);
        return;
    }
    if (n->kind == EXPR_DIRAC_SUM) {
        dl_sum *d = n->dl;
        hoist_inv(&d->first, path, np);
        int p2[DL_MAXLOOP + 1]; memcpy(p2, path, np * sizeof *path); p2[np] = d->lv;
        hoist_inv(&d->body, p2, np + 1);
        return;
    }
    hoist_inv(&n->left, path, np);
    hoist_inv(&n->right, path, np);
    for (int i = 0; i < n->n_args; i++) hoist_inv(&n->args[i], path, np);
}

// ----------------------------------------------------------------------------
// the steps of the loops ----------------------------------------------------
// ----------------------------------------------------------------------------
// A loop's counter steps by the stride its index is used with most, weighed
// by how deep inside other loops the use is; another stride gets a derived
// variable, stepped with the counter.

static void use_ix(const dl_ix *ix, int deeper)
{
    for (int i = 0; i < ix->nt; i++) {
        dl_loop *l = &loops[ix->lv[i]];
        long w = 1; for (int d = 0; d < deeper && d < 6; d++) w *= 1000;
        int j;
        for (j = 0; j < l->ns; j++) if (l->st[j] == ix->st[i]) break;
        if (j == l->ns) {if (l->ns == DL_MAXSTR) die(MSG_ERR_DIRAC_NOT_YET, cur_line); l->st[l->ns] = ix->st[i]; l->w[l->ns++] = 0;}
        l->w[j] += w;
    }
}

// path depth of each loop, to weigh a use by the loops inside the one it names
static void count_uses(expr_node *n, int np)
{
    if (!n) return;
    if (n->kind == EXPR_DIRAC_ELEM) {use_ix(&((dl_elem *)n->dl)->ix, np); return;}
    if (n->kind == EXPR_DIRAC_SUM) {
        dl_sum *d = n->dl;
        count_uses(d->first, np);
        count_uses(d->body, np + 1);
        return;
    }
    count_uses(n->left, np); count_uses(n->right, np);
    for (int i = 0; i < n->n_args; i++) count_uses(n->args[i], np);
}

static void choose_steps(int from)
{
    for (int lv = from; lv < n_loops; lv++) {
        dl_loop *l = &loops[lv];
        int best = 0;
        for (int j = 1; j < l->ns; j++) if (l->w[j] > l->w[best]) best = j;
        l->step = l->ns ? l->st[best] : 1;
        l->nd = 0;
        for (int j = 0; j < l->ns; j++) if (l->st[j] != l->step) {
            l->ds[l->nd] = l->st[j]; l->dv[l->nd++] = n_names++;
        }
    }
}

// the variable that holds stride * turn (minus stride * n) of loop lv
static void var_of(char *b, size_t sz, int lv, int stride)
{
    dl_loop *l = &loops[lv];
    if (stride == l->step) {ctr_name(b, sz, lv); return;}
    for (int j = 0; j < l->nd; j++) if (l->ds[j] == stride) {snprintf(b, sz, "__dd%d", l->dv[j]); return;}
    die(MSG_ERR_DIRAC_NOT_YET, cur_line);
}

// the variables of an index, the innermost loop first, and the constant
// offset that makes the sum of them the cell's address
static int ix_vars(const dl_ix *ix, char v[2][32], long *off)
{
    int ord[2] = {0, 1};
    if (ix->nt == 2 && ix->lv[1] > ix->lv[0]) {ord[0] = 1; ord[1] = 0;}   // inner loops are allocated later
    *off = ix->k;
    for (int i = 0; i < ix->nt; i++) {
        int t = ord[i], lv = ix->lv[t], s = ix->st[t];
        var_of(v[i], 32, lv, s);
        *off += (long)s * (loops[lv].first + loops[lv].n);
    }
    return ix->nt;
}

// puts the index of ix in the accumulator (pushing what is there when
// acc_ok); returns the constant offset
static long ix_load(const dl_ix *ix, char *key, size_t ksz)
{
    char v[2][32]; long off;
    int nt = ix_vars(ix, v, &off);
    snprintf(key, ksz, "%s%s%s", v[0], nt > 1 ? "+" : "", nt > 1 ? v[1] : "");
    if (key_is(key)) {if (acc_ok) add_instr("PSH\n"); return off;}
    if (key_is(v[0])) {if (acc_ok) add_instr("PSH\n");}
    else add_instr("%s %s\n", acc_ok ? "P_LOD" : "LOD", v[0]);
    if (nt > 1) add_instr("ADD %s\n", v[1]);
    key_set(key);
    return off;
}

static const char *arr_name(int arr) {return v_table[arr].name;}

// ----------------------------------------------------------------------------
// emission ------------------------------------------------------------------
// ----------------------------------------------------------------------------

expr dirac_elem_emit(expr_node *n)
{
    dl_elem *e = n->dl;
    if (e->ix.nt == 0)
        add_instr("%s %s %ld\n", acc_ok ? "P_LOD_V" : "LOD_V", arr_name(e->arr), e->ix.k);
    else {
        char key[128];
        long off = ix_load(&e->ix, key, sizeof key);
        add_instr("LDI_V %s %ld\n", arr_name(e->arr), off);
    }
    acc_ok = 1;
    return expr_make(n->type, 0);
}

// a value the walker left in memory goes to the accumulator
static expr in_acc(expr e)
{
    if (e.id == 0) return e;
    add_instr("%s %s\n", acc_ok ? "P_LOD" : "LOD", v_table[e.id].name);
    acc_ok = 1;
    return expr_make(e.type, 0);
}

static void emit_hoist(expr_node *n, int v)
{
    int conv = (v >> 30) & 1; v &= ~(1 << 30);
    acc_ok = 0;
    expr e = ast_emit_expr(n);
    if (conv && e.type == 1 && e.id != 0) add_instr("I2F_M %s\n", v_table[e.id].name);
    else {
        e = in_acc(e);
        if (conv && e.type == 1) add_instr("I2F\n");
    }
    add_instr("SET %s\n", v_table[v].name);
    acc_ok = 0;
}

// opens loop lv: derived variables, then the counter, then the label. The
// first load pushes what the accumulator holds when push is set
static void loop_open(int lv, int push, char *top, char *end)
{
    dl_loop *l = &loops[lv];
    char c[32]; ctr_name(c, sizeof c, lv);
    snprintf(top, 32, "__Ld%d", label_n);
    snprintf(end, 32, "__Ld%de", label_n++);
    for (int j = 0; j < l->nd; j++) {
        add_instr("%s %ld\n", push ? "P_LOD" : "LOD", -(long)l->ds[j] * l->n); push = 0;
        add_instr("SET __dd%d\n", l->dv[j]);
    }
    add_instr("%s %ld\n", push ? "P_LOD" : "LOD", -(long)l->step * l->n);
    add_instr("SET %s\n", c);
    add_sinst(0, "@%s ", top);
    key_set(c);
    acc_ok = 0;
}

// the parts computed at the top of each turn
static void loop_hoists(int lv)
{
    dl_loop *l = &loops[lv];
    for (int j = 0; j < l->nh; j++) emit_hoist(*l->hn[j], l->hv[j]);
}

// steps the derived variables and the counter, and closes the loop
static void loop_close(int lv, int push, const char *top, const char *end)
{
    dl_loop *l = &loops[lv];
    char c[32]; ctr_name(c, sizeof c, lv);
    for (int j = 0; j < l->nd; j++) {
        add_instr("%s __dd%d\n", push ? "P_LOD" : "LOD", l->dv[j]); push = 0;
        add_instr("ADD %d\n", l->ds[j]);
        add_instr("SET __dd%d\n", l->dv[j]);
    }
    add_instr("%s %s\n", push ? "P_LOD" : "LOD", c);
    add_instr("ADD %d\n", l->step);
    add_instr("SET %s\n", c);
    add_instr("JIZ %s\n", end);
    add_instr("JMP %s\n", top);
    add_sinst(0, "@%s ", end);
}

// a sum over k: the first term, then the loop, the sum on the stack
expr dirac_sum_emit(expr_node *n)
{
    dl_sum *d = n->dl;
    expr e = in_acc(ast_emit_expr(d->first));
    int t = e.type;

    char top[32], end[32];
    loop_open(d->lv, 1, top, end);
    loop_hoists(d->lv);
    acc_ok = 0;
    expr b = in_acc(ast_emit_expr(d->body));
    oper_soma(expr_make(t, 0), b);
    acc_ok = 1;
    loop_close(d->lv, 1, top, end);
    add_instr("POP\n");
    acc_ok = 1;
    return expr_make(t, 0);
}

// ----------------------------------------------------------------------------
// statements ----------------------------------------------------------------
// ----------------------------------------------------------------------------

// where the element goes
typedef struct {
    int kind;          // 0 an array (STI_V), 1 an output port, 2 an output port as fout
    int arr, port;     // the array (v_table id, may be __dirac_tmp) or the port
    long base;         // the block of __dirac_tmp
    int type;          // the array's element type
    int cols;          // a matrix: its row length (0: a vector)
} dl_dst;

static void enter(int line)
{
    if (depth++ == 0) {n_names = 0; tmp_at = 0; n_loops = 0; n_hoist_pre = 0;}
    cur_line = line;
}

static void leave(void) {depth--;}

static int tmp_block(int words, int line)
{
    if (tmp_id < 0) {
        add_var("__dirac_tmp");
        tmp_id = find_var("__dirac_tmp");
        v_table[tmp_id].isar = 1;
        v_table[tmp_id].used = 1;
    }
    if (ctx_type == 2) tmp_type = 2;
    v_table[tmp_id].type = tmp_type;
    int at = tmp_at;
    tmp_at += words;
    if (tmp_at > tmp_max) tmp_max = tmp_at;
    if (n_tmp_lines < DL_MAXLINES) {tmp_lines[n_tmp_lines] = line; tmp_words[n_tmp_lines++] = words;}
    return at;
}

// stores the value in the accumulator (or in memory) at the index pushed
static void store(expr e, const dl_dst *d, long off)
{
    if (e.id != 0) {                                          // the index is still in the acc
        if (d->type == 2 && e.type == 1) add_instr("P_I2F_M %s\n", v_table[e.id].name);
        else if (d->type == 1 && e.type == 2) {
            fprintf(stdout, MSG_WARN_INT_RECV_FLOAT, cur_line, rem_fname(v_table[d->arr].name, fname));
            add_instr("P_F2I_M %s\n", v_table[e.id].name);
        }
        else add_instr("P_LOD %s\n", v_table[e.id].name);
    } else {
        if (d->type == 2 && e.type == 1) add_instr("I2F\n");
        if (d->type == 1 && e.type == 2) {
            fprintf(stdout, MSG_WARN_INT_RECV_FLOAT, cur_line, rem_fname(v_table[d->arr].name, fname));
            add_instr("F2I\n");
        }
    }
    add_instr("STI_V %s %ld\n", arr_name(d->arr), off);
    acc_ok = 0;
}

static void emit_target(expr_node *el, const dl_dst *d, const dl_ix *six)
{
    if (d->kind == 0 && six->nt == 0) {                // one cell: a plain store
        acc_ok = 0;
        expr e = in_acc(ast_emit_expr(el));
        if (d->type == 2 && e.type == 1) add_instr("I2F\n");
        if (d->type == 1 && e.type == 2) {
            fprintf(stdout, MSG_WARN_INT_RECV_FLOAT, cur_line, rem_fname(v_table[d->arr].name, fname));
            add_instr("F2I\n");
        }
        add_instr("SET_V %s %ld\n", arr_name(d->arr), six->k + d->base);
        acc_ok = 0;
    } else if (d->kind == 0) {
        char key[128];
        acc_ok = 0;
        long off = ix_load(six, key, sizeof key);
        acc_ok = 1;                       // the index is the value to keep: the first load pushes it
        key_set(key);
        expr e = ast_emit_expr(el);
        store(e, d, off + d->base);
    } else {
        acc_ok = 0;
        expr e = ast_emit_expr(el);
        if (d->kind == 2) exec_fout(d->port, e);
        else              exec_out (d->port, e);
        acc_ok = 0;
    }
}

// the loops of the target, from loop index li of the list tl
static void emit_nest(const int *tl, int nl, int li, expr_node *el, const dl_dst *d, const dl_ix *six)
{
    if (li == nl) {emit_target(el, d, six); return;}
    int lv = tl[li];
    char top[32], end[32];
    loop_open(lv, 0, top, end);
    loop_hoists(lv);
    emit_nest(tl, nl, li + 1, el, d, six);
    loop_close(lv, 0, top, end);
    acc_ok = 0;
}

static void collect_target_uses(const dl_ix *six, int nl) {use_ix(six, nl);}

static void emit_statement(expr_node *r, const dl_dst *d0, int rows, int cols, int line);

// the target's element tree, with the loops of the target in tl; rebuilt
// after each product that has to go to __dirac_tmp first
static expr_node *build_target(expr_node *r, int rows, int cols, int flat, int *tl, int *nl, dl_ix *six, int line)
{
    for (;;) {
        n_loops = 0; n_hoist_pre = 0;
        *nl = 0;
        dl_pos pr = {-1, 0}, pc = {-1, 0};
        memset(six, 0, sizeof *six);
        if (flat) {
            int n = rows * cols;
            if (n > 1) {pr.lv = new_loop(n, 0); tl[(*nl)++] = pr.lv;}
            ix_add(six, pr, 1);
        } else {
            if (rows > 1) {pr.lv = new_loop(rows, 0); tl[(*nl)++] = pr.lv;}
            if (cols > 1) {pc.lv = new_loop(cols, 0); tl[(*nl)++] = pc.lv;}
            ix_add(six, pr, cols > 1 ? cols : 1);
            if (cols > 1) ix_add(six, pc, 1);
        }
        flat_mode = flat;
        expr_node *el = build(r, pr, flat ? pr : pc, 1);

        dl_set around = 0;
        for (int i = 0; i < *nl; i++) around |= 1ULL << tl[i];
        expr_node *w = find_waste(el, around);
        if (!w) return el;

        // the product goes to a block of __dirac_tmp first, and the
        // expression reads the block
        expr_node *p = ((dl_sum *)w->dl)->src;
        expr_node copy = *p;
        int words = p->rows * p->cols;
        int off = tmp_block(words, line);
        fprintf(stdout, MSG_INFO_DIRAC_TEMP_CHAIN, line, words);
        dl_dst td = {0, tmp_id, 0, off, ctx_type, p->cols > 1 && p->rows > 1 ? p->cols : 0};
        emit_statement(&copy, &td, p->rows, p->cols, line);

        dl_tmpref *tr = calloc(1, sizeof *tr); tr->off = off;
        p->kind = EXPR_DIRAC_REF;
        p->op   = (p->cols == 1) ? DIRAC_ROLE_KET : (p->rows == 1) ? DIRAC_ROLE_BRA : DIRAC_ROLE_MAT;
        p->id   = tmp_id;
        p->left = p->right = NULL;
        p->dl   = tr;
    }
}

static int has_arr(expr_node *n, int arr)
{
    if (!n) return 0;
    if (n->kind == EXPR_DIRAC_ELEM) return ((dl_elem *)n->dl)->arr == arr;
    if (n->kind == EXPR_DIRAC_SUM) {dl_sum *s = n->dl; return has_arr(s->first, arr) || has_arr(s->body, arr);}
    if (has_arr(n->left, arr) || has_arr(n->right, arr)) return 1;
    for (int i = 0; i < n->n_args; i++) if (has_arr(n->args[i], arr)) return 1;
    return 0;
}

// does every array leaf of the target read it at the cell it writes?
static int reads_elsewhere(expr_node *n, int arr, const dl_ix *six, long base)
{
    if (!n) return 0;
    if (n->kind == EXPR_DIRAC_ELEM) {
        dl_elem *e = n->dl;
        if (e->arr != arr) return 0;
        if (e->ix.nt != six->nt || e->ix.k != six->k + base) return 1;
        for (int i = 0; i < six->nt; i++)
            if (e->ix.lv[i] != six->lv[i] || e->ix.st[i] != six->st[i]) return 1;
        return 0;
    }
    if (n->kind == EXPR_DIRAC_SUM) {
        dl_sum *s = n->dl;
        // inside a sum the cell read changes while the one written does not
        return reads_elsewhere(s->first, arr, six, base) || has_arr(s->body, arr);
    }
    if (reads_elsewhere(n->left, arr, six, base) || reads_elsewhere(n->right, arr, six, base)) return 1;
    for (int i = 0; i < n->n_args; i++) if (reads_elsewhere(n->args[i], arr, six, base)) return 1;
    return 0;
}

// can the expression be computed over one flat index? Only matrices of the
// target's shape, numbers, |0⟩ and |in⟩: then cell (r, c) is cell r*cols + c
// of every array in it
static int flat_ok(expr_node *n)
{
    if (!n) return 1;
    if (is_num(n) && n->kind != EXPR_DIRAC_REF) return 1;
    switch (n->kind) {
        case EXPR_DIRAC_REF:   return n->op == DIRAC_ROLE_MAT;
        case EXPR_DIRAC_CONST: return n->op == DIRAC_ZERO;
        case EXPR_DIRAC_IN:    return 1;
        case EXPR_BINOP: case EXPR_UNOP: return flat_ok(n->left) && flat_ok(n->right);
        case EXPR_DIRAC_PROD:  return (is_num(n->left) && flat_ok(n->right)) || (is_num(n->right) && flat_ok(n->left));
        default: return 0;
    }
}

// |A|B|x⟩ is read as |A|(|B|x⟩), so that B x is a vector computed once
static void reassoc(expr_node *n)
{
    if (!n) return;
    reassoc(n->left); reassoc(n->right);
    if (n->kind != EXPR_DIRAC_PROD) return;
    expr_node *L = n->left, *R = n->right;
    if (L->kind == EXPR_DIRAC_PROD && !is_num(L) && !is_num(L->left) && !is_num(L->right) && R->cols == 1 && !is_num(R)) {
        expr_node *A = L->left, *B = L->right;
        L->left = B; L->right = R;                 // L becomes B x
        L->rows = B->rows; L->cols = R->cols;
        L->type = 0; typecheck_expr(L);
        n->left = A; n->right = L;
        reassoc(n->right);
    }
}

// a whole statement: the target d0 (an array, a block of __dirac_tmp or a
// port) gets the expression r, of the shape rows x cols
static void emit_statement(expr_node *r, const dl_dst *d0, int rows, int cols, int line)
{
    enter(line);
    int save_loops = n_loops, save_pre = n_hoist_pre;
    dl_loop save_l[DL_MAXLOOP]; memcpy(save_l, loops, sizeof loops);
    expr_node **save_hp[DL_MAXHOI]; int save_hv[DL_MAXHOI];
    memcpy(save_hp, hoist_pre, sizeof hoist_pre); memcpy(save_hv, hoist_pre_v, sizeof hoist_pre_v);
    int save_ctx = ctx_type, save_flat = flat_mode, save_dst = cur_dst;

    dl_dst d = *d0;
    cur_dst = (d.kind == 0) ? d.arr : -1;
    int flat = rows > 1 && cols > 1 && flat_ok(r);
    int tl[4], nl; dl_ix six;
    reassoc(r);
    expr_node *el = build_target(r, rows, cols, flat, tl, &nl, &six, line);

    // the target read at another cell than the one written: the result goes
    // through __dirac_tmp, and is copied at the end
    int copy_back = 0; dl_dst real = d;
    if (d.kind == 0 && d.arr != tmp_id && reads_elsewhere(el, d.arr, &six, 0)) {
        int words = rows * cols;
        int off = tmp_block(words, line);
        fprintf(stdout, MSG_INFO_DIRAC_TEMP, line, rem_fname(v_table[d.arr].name, fname), words);
        d.arr = tmp_id; d.base = off; d.type = ctx_type;
        copy_back = 1;
    }

    hoist_inv(&el, tl, nl);
    count_uses(el, nl);
    for (int lv = 0; lv < n_loops; lv++)
        for (int j = 0; j < loops[lv].nh; j++) count_uses(*loops[lv].hn[j], 0);
    if (d.kind == 0) collect_target_uses(&six, 0);
    choose_steps(0);

    for (int j = 0; j < n_hoist_pre; j++) emit_hoist(*hoist_pre[j], hoist_pre_v[j]);
    emit_nest(tl, nl, 0, el, &d, &six);

    if (copy_back) {                                  // __dirac_tmp -> the target, one flat loop
        int n = rows * cols;
        n_loops = 0;
        dl_pos p = {-1, 0};
        if (n > 1) p.lv = new_loop(n, 0);
        dl_ix ix = {0}; ix_add(&ix, p, 1);
        dl_ix tix = ix; tix.k += d.base;
        expr_node *src = elem_new(tmp_id, d.type, tix);
        int one[1] = {p.lv};
        if (n > 1) {loops[p.lv].ns = 0; use_ix(&ix, 0); choose_steps(0);}
        emit_nest(one, n > 1, 0, src, &real, &ix);
    }

    ctx_type = save_ctx; flat_mode = save_flat; cur_dst = save_dst;
    memcpy(loops, save_l, sizeof loops); n_loops = save_loops;
    memcpy(hoist_pre, save_hp, sizeof hoist_pre); memcpy(hoist_pre_v, save_hv, sizeof hoist_pre_v);
    n_hoist_pre = save_pre;
    leave();
}

// c|I| and |I| on their own: zeros, then the diagonal
static int dl_eye(int dst, expr_node *r, int line)
{
    expr_node *c = NULL;
    if (r->kind == EXPR_DIRAC_CONST && r->op == DIRAC_EYE) c = NULL;
    else if (r->kind == EXPR_DIRAC_PROD && is_num(r->left) && r->right->kind == EXPR_DIRAC_CONST && r->right->op == DIRAC_EYE) c = r->left;
    else return 0;

    enter(line);
    int R = v_table[dst].size, C = v_table[dst].siz2, t = v_table[dst].type;
    ctx_type = t;
    n_loops = 0; n_hoist_pre = 0;
    if (t > 2) die(MSG_ERR_NOT_IMPL_COMPLEX, line);
    expr_node *val = c ? scalar_of(c) : expr_lit(t, t == 2 ? exec_fnum("1.0") : exec_inum("1"));
    for (int j = 0; j < n_hoist_pre; j++) emit_hoist(*hoist_pre[j], hoist_pre_v[j]);

    dl_dst d = {0, dst, 0, 0, t, C};
    // the zeros, over one flat index
    dl_pos p = {-1, 0};
    if (R * C > 1) p.lv = new_loop(R * C, 0);
    dl_ix ix = {0}; ix_add(&ix, p, 1);
    if (p.lv >= 0) {use_ix(&ix, 0); choose_steps(p.lv);}
    int one[1] = {p.lv};
    emit_nest(one, p.lv >= 0, 0, zero_of(t), &d, &ix);

    // the diagonal: a loop whose counter steps by C + 1
    int n = R < C ? R : C;
    dl_pos q = {-1, 0};
    int from = n_loops;
    if (n > 1) q.lv = new_loop(n, 0);
    dl_ix dx = {0}; ix_add(&dx, q, C + 1);
    if (q.lv >= 0) {use_ix(&dx, 0); choose_steps(from);}
    one[0] = q.lv;
    emit_nest(one, q.lv >= 0, 0, val, &d, &dx);
    leave();
    return 1;
}

// ----------------------------------------------------------------------------
// interface -----------------------------------------------------------------
// ----------------------------------------------------------------------------

void dirac_loops(int dst, expr_node *r, int line)
{
    if (dl_eye(dst, r, line)) return;

    int t = v_table[dst].type;
    if (t > 2) die(MSG_ERR_NOT_IMPL_COMPLEX, line);
    int save = ctx_type;
    ctx_type = (r->type == 1 || r->type == 2) ? r->type : t;
    if (r->type > 2) die(MSG_ERR_NOT_IMPL_COMPLEX, line);

    int rows, cols;
    if (v_table[dst].isar == 2) {rows = v_table[dst].size; cols = v_table[dst].siz2;}
    else if (r->rows == 1 && r->cols != 1) {rows = 1; cols = v_table[dst].size;}   // a bra into a vector
    else {rows = v_table[dst].size; cols = 1;}

    dl_dst d = {0, dst, 0, 0, t, v_table[dst].isar == 2 ? cols : 0};
    acc_ok = 0;
    emit_statement(r, &d, rows, cols, line);
    acc_ok = 0;
    ctx_type = save;
}

void dirac_loops_out(int port, expr_node *r, int fout, int line)
{
    int save = ctx_type;
    if (r->type > 2) die(MSG_ERR_NOT_IMPL_COMPLEX, line);
    ctx_type = (r->type == 1 || r->type == 2) ? r->type : (fout ? 2 : 1);
    int rows = r->rows, cols = r->cols;
    if (rows == DIRAC_ANY || cols == DIRAC_ANY) die(MSG_ERR_DIRAC_NOT_YET, line);   // |0⟩, |in(p)⟩ alone: no size
    dl_dst d = {fout ? 2 : 1, 0, port, 0, ctx_type, 0};
    acc_ok = 0;
    emit_statement(r, &d, rows, cols, line);
    acc_ok = 0;
    ctx_type = save;
}

// a Dirac product that is a number, inside a scalar expression (⟨a|b⟩,
// ⟨a|M|b⟩): the walker calls this with what the accumulator holds in acc_ok
expr dirac_number(expr_node *n)
{
    int live = acc_ok;
    enter(n->line);
    int save_loops = n_loops, save_pre = n_hoist_pre, save_ctx = ctx_type;
    dl_loop save_l[DL_MAXLOOP]; memcpy(save_l, loops, sizeof loops);
    expr_node **save_hp[DL_MAXHOI]; int save_hv[DL_MAXHOI];
    memcpy(save_hp, hoist_pre, sizeof hoist_pre); memcpy(save_hv, hoist_pre_v, sizeof hoist_pre_v);

    if (n->type > 2) die(MSG_ERR_NOT_IMPL_COMPLEX, n->line);
    ctx_type = n->type ? n->type : 1;
    reassoc(n);
    int save_flat = flat_mode, save_dst = cur_dst;
    cur_dst = -1;
    int nl; int tl[4]; dl_ix six;
    expr_node *el = build_target(n, 1, 1, 0, tl, &nl, &six, n->line);
    hoist_inv(&el, tl, 0);
    count_uses(el, 0);
    for (int lv = 0; lv < n_loops; lv++)
        for (int j = 0; j < loops[lv].nh; j++) count_uses(*loops[lv].hn[j], 0);
    choose_steps(0);

    // the parts computed first: the value the walker holds waits in memory
    int save_v = 0;
    if (n_hoist_pre && live) {
        save_v = scalar_var(ctx_type);
        add_instr("SET %s\n", v_table[save_v].name);
    }
    for (int j = 0; j < n_hoist_pre; j++) emit_hoist(*hoist_pre[j], hoist_pre_v[j]);
    if (save_v) {add_instr("LOD %s\n", v_table[save_v].name); acc_ok = 1;}
    else acc_ok = live;

    expr e = in_acc(ast_emit_expr(el));

    ctx_type = save_ctx; flat_mode = save_flat; cur_dst = save_dst;
    memcpy(loops, save_l, sizeof loops); n_loops = save_loops;
    memcpy(hoist_pre, save_hp, sizeof hoist_pre); memcpy(hoist_pre_v, save_hv, sizeof hoist_pre_v);
    n_hoist_pre = save_pre;
    leave();
    return e;
}

// after the walker: the directive of __dirac_tmp, right after #PRNAME, and
// the summary
void dirac_finish(const char *asm_file)
{
    if (tmp_max == 0) return;

    FILE *f = fopen(asm_file, "r");
    if (!f) {fprintf(stderr, MSG_ERR_CANT_OPEN_FILE, asm_file); exit(EXIT_FAILURE);}
    char *text = NULL; size_t len = 0, cap = 0; char line[4096]; int done = 0;
    while (fgets(line, sizeof line, f)) {
        char add[128] = "";
        if (!done && strncmp(line, "#PRNAME", 7) == 0) {
            snprintf(add, sizeof add, "#array __dirac_tmp %d %d\n", tmp_type, tmp_max);
            done = 1;
        }
        size_t k = strlen(line), a = strlen(add);
        if (len + k + a + 1 > cap) {cap = 2 * (len + k + a + 1); text = realloc(text, cap);}
        memcpy(text + len, line, k); len += k;
        memcpy(text + len, add, a);  len += a;
        text[len] = '\0';
    }
    fclose(f);
    if ((f = fopen(asm_file, "w"))) {if (text) fputs(text, f); fclose(f);}
    free(text);

    fprintf(stdout, MSG_INFO_DIRAC_TEMP_TOTAL, tmp_max);
    for (int i = 0; i < n_tmp_lines; i++) fprintf(stdout, MSG_INFO_DIRAC_TEMP_LINE, tmp_lines[i], tmp_words[i]);
}
