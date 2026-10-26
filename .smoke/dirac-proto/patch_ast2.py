p = 'C:/nipscern/yanc-dirac/Compilers/CMMComp/Sources/ast.c'
s = open(p, encoding='utf-8', newline='').read().replace('\r\n', '\n')
R = []

R.append(("""static void emit_cond_int_load(expr e, int is_switch)""",
"""// ----------------------------------------------------------------------------
// Dirac statements -----------------------------------------------------------
// ----------------------------------------------------------------------------
// The expression is checked for shape, then matched against the forms the
// unrolled generators of stdlib.c compute (exec_Mv, exec_cv, ...), called
// with the arguments the old grammar rules gave them, so the assembly is the
// same. Any other combination stops with MSG_ERR_DIRAC_NOT_YET until the
// expressions are computed in loops (docs/design/dirac-expressions.md, step b).

static int is_ref(const expr_node *n, int role)
{
    return n && n->kind == EXPR_DIRAC_REF && n->op == role;
}

static int is_const(const expr_node *n, int which)
{
    return n && n->kind == EXPR_DIRAC_CONST && n->op == which;
}

static int is_prod(const expr_node *n)
{
    return n && n->kind == EXPR_DIRAC_PROD;
}

// the target array dst against the shape of the expression
static void dirac_check_target(int dst, expr_node *r, int line)
{
    char *name = rem_fname(v_table[dst].name, fname);
    if (v_table[dst].type == 0) {fprintf(stderr, MSG_ERR_DECL_FIRST, line, name); exit(EXIT_FAILURE);}

    int rows = 1, cols = 1, fits;
    if (v_table[dst].isar == 1) {
        rows = v_table[dst].size;
        fits = (dim_fits(r->rows, rows) && r->cols == 1) || (r->rows == 1 && dim_fits(r->cols, rows));
    } else if (v_table[dst].isar == 2) {
        rows = v_table[dst].size; cols = v_table[dst].siz2;
        fits = dim_fits(r->rows, rows) && dim_fits(r->cols, cols);
    } else fits = 0;

    if (!fits) {
        char b1[64], b2[64];
        fprintf(stderr, MSG_ERR_DIRAC_TARGET_SHAPE, line, name,
                shape_text(rows, cols, b1, sizeof b1),
                shape_text(r->rows, r->cols, b2, sizeof b2));
        exit(EXIT_FAILURE);
    }
}

static void dirac_emit(int dst, expr_node *r, int line)
{
    shapecheck_expr(r);
    typecheck_expr(r);
    dirac_check_target(dst, r, line);

    expr_node *L = r->left, *R = r->right;

    if (is_prod(r) && is_ref(L, DIRAC_ROLE_MAT) && is_ref(R, DIRAC_ROLE_KET))    // |M|a⟩
        exec_Mv(dst, L->id, R->id);
    else if (is_prod(r) && is_number(L) && is_ref(R, DIRAC_ROLE_KET))            // c|b⟩
        exec_cv(dst, ast_emit_expr(L), R->id);
    else if (r->kind == EXPR_BINOP && r->op == OP_ADD && is_ref(L, DIRAC_ROLE_KET)
             && is_prod(R) && is_number(R->left) && is_ref(R->right, DIRAC_ROLE_KET)) // |b⟩ + c|d⟩
        exec_apcb(dst, L->id, ast_emit_expr(R->left), R->right->id);
    else if (is_prod(r) && is_ref(L, DIRAC_ROLE_KET) && is_ref(R, DIRAC_ROLE_BRA))  // |a⟩⟨b|
        exec_vvt(dst, L->id, R->id);
    else if (r->kind == EXPR_BINOP && r->op == OP_SUB && is_ref(L, DIRAC_ROLE_MAT)
             && is_prod(R) && is_ref(R->left, DIRAC_ROLE_KET) && is_ref(R->right, DIRAC_ROLE_BRA)) // |B| - |a⟩⟨b|
        exec_Mmvvt(dst, L->id, R->left->id, R->right->id);
    else if (is_prod(r) && is_number(L) && is_ref(R, DIRAC_ROLE_MAT))            // c|B|
        exec_cM(dst, ast_emit_expr(L), R->id);
    else if (is_prod(r) && is_number(L) && is_const(R, DIRAC_EYE))               // c|I|
        exec_cI(dst, ast_emit_expr(L));
    else if (is_const(r, DIRAC_ZERO))                                            // |0⟩
        exec_v0(dst);
    else if (is_prod(r) && is_number(L) && R->kind == EXPR_DIRAC_IN)             // c|in(p)⟩
        exec_cvin(dst, ast_emit_expr(L), R->id);
    else {fprintf(stderr, MSG_ERR_DIRAC_NOT_YET, line); exit(EXIT_FAILURE);}
}

// out(p, X) / fout(p, X): a number as before; a vector c|a⟩ through exec_vout
static void dirac_out(stmt_node *n)
{
    expr_node *r = n->rhs;
    shapecheck_expr(r);

    if (is_number(r)) {
        if (n->op) exec_fout(n->id, ast_emit_expr(r));
        else       exec_out (n->id, ast_emit_expr(r));
        return;
    }
    if (r->rows != 1 && r->cols != 1) {
        char b[64];
        fprintf(stderr, MSG_ERR_DIRAC_OUT_MATRIX, n->line, shape_text(r->rows, r->cols, b, sizeof b));
        exit(EXIT_FAILURE);
    }
    typecheck_expr(r);
    if (!n->op && is_prod(r) && is_number(r->left) && is_ref(r->right, DIRAC_ROLE_KET))  // out(p, c|a⟩)
        exec_vout(n->id, ast_emit_expr(r->left), r->right->id);
    else {fprintf(stderr, MSG_ERR_DIRAC_NOT_YET, n->line); exit(EXIT_FAILURE);}
}

static void emit_cond_int_load(expr e, int is_switch)"""))

R.append(("""        case STMT_OUT:
            if (n->op) exec_fout(n->id, ast_emit_expr(n->rhs));
            else       exec_out (n->id, ast_emit_expr(n->rhs));
            break;""", """        case STMT_OUT:
            dirac_out(n);
            break;"""))

R.append(("""        case STMT_VOUT:
            exec_vout(n->id, ast_emit_expr(n->rhs), n->id2);
            break;

""", ""))

i0 = s.index("        case STMT_DIRAC:\n            switch (n->op) {")
i1 = s.index("break;", s.index("case DIRAC_SHIFT:", i0))
i1 = s.index("\n", s.index("}", i1)) + 1   # the closing brace of the switch
blk = s[i0:i1]
s = s[:i0] + """        case STMT_DIRAC:
            dirac_emit(n->id, n->rhs, n->line);
            break;

        case STMT_DIRAC_SHIFT:
            exec_shift(n->id, ast_emit_expr(n->rhs), n->id2);
            break;
""" + s[i1:]
print("--- replaced block:\n" + blk)

R.append(("""        case STMT_DECLAR_MV:
            // base array #array directive(s), then Mv codegen.
            declar_arr_1d_emit(n->id, n->id2, -1);
            exec_Mv(n->id, n->id3, n->id4);
            break;

        case STMT_DECLAR_CV:
            // base array #array directive(s), then walk the scalar coeff
            // and let exec_cv do the c|v⟩ codegen.
            declar_arr_1d_emit(n->id, n->id2, -1);
            exec_cv(n->id, ast_emit_expr(n->rhs), n->id3);
            break;""", """        case STMT_DECLAR_DIRAC:
            // base array #array directive(s), then the Dirac expression
            declar_arr_1d_emit(n->id, n->id2, -1);
            dirac_emit(n->id, n->rhs, n->line);
            break;"""))

for a, b in R:
    assert s.count(a) == 1, a[:70]
    s = s.replace(a, b)

open(p, 'w', encoding='utf-8', newline='').write(s.replace('\n', '\r\n'))
print("ok")
