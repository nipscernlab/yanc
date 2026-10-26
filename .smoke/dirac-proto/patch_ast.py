p = 'C:/nipscern/yanc-dirac/Compilers/CMMComp/Sources/ast.c'
s = open(p, encoding='utf-8', newline='').read().replace('\r\n', '\n')
R = []

# 1. expression constructors -------------------------------------------------
R.append(("""expr_node *expr_inner(expr_node *a, expr_node *b)
{
    expr_node *n = enode_new(EXPR_INNER);
    n->left  = a;
    n->right = b;
    return n;
}
""", """// Dirac chains. While the parser reads a chain, it is a left-nested product
// of EXPR_DIRAC_REF leaves whose roles are still unknown (op = 0); the first
// leaf keeps op = -1 when the chain was opened by ⟨. dirac_close gives every
// leaf its role once the closing token is known.
static expr_node *dirac_ref(int id)
{
    expr_node *n = enode_new(EXPR_DIRAC_REF);
    n->type = v_table[id].type;
    n->id   = id;
    return n;
}

expr_node *dirac_open(int opened_by_bra, int id)
{
    expr_node *n = dirac_ref(id);
    n->op = opened_by_bra ? -1 : 0;
    return n;
}

expr_node *dirac_push(expr_node *chain, int id)
{
    return expr_dirac_prod(chain, dirac_ref(id));
}

// the leaves of a chain in source order
static int dirac_leaves(expr_node *n, expr_node **out, int k)
{
    if (n->kind == EXPR_DIRAC_PROD) {
        k = dirac_leaves(n->left, out, k);
        return dirac_leaves(n->right, out, k);
    }
    if (k < 256) out[k] = n;
    return k + 1;
}

expr_node *dirac_close(expr_node *chain, int closed_by_ket)
{
    expr_node *leaf[256];
    int k = dirac_leaves(chain, leaf, 0);
    if (k > 256) {fprintf(stderr, MSG_ERR_SYNTAX, chain->line); exit(EXIT_FAILURE);}
    int opened_by_bra = (leaf[0]->op == -1);

    // ⟨a⟩ has no bar: neither an inner product nor a vector
    if (k == 1 && opened_by_bra && closed_by_ket)
        {fprintf(stderr, MSG_ERR_SYNTAX, chain->line); exit(EXIT_FAILURE);}

    for (int j = 0; j < k; j++) {
        if      (j == 0     && opened_by_bra) leaf[j]->op = DIRAC_ROLE_BRA;
        else if (j == k - 1 && closed_by_ket) leaf[j]->op = DIRAC_ROLE_KET;
        else                                  leaf[j]->op = DIRAC_ROLE_MAT;
    }
    return chain;
}

expr_node *expr_dirac_prod(expr_node *left, expr_node *right)
{
    expr_node *n = enode_new(EXPR_DIRAC_PROD);
    n->left  = left;
    n->right = right;
    return n;
}

expr_node *expr_dirac_const(int which)
{
    expr_node *n = enode_new(EXPR_DIRAC_CONST);
    n->op = which;
    return n;
}

expr_node *expr_dirac_in(int port)
{
    expr_node *n = enode_new(EXPR_DIRAC_IN);
    n->id = port;
    return n;
}
"""))

# 2. dump --------------------------------------------------------------------
R.append(("""        case EXPR_INNER:       return "INNER";
    }""", """        case EXPR_DIRAC_REF:   return "DIRAC_REF";
        case EXPR_DIRAC_PROD:  return "DIRAC_PROD";
        case EXPR_DIRAC_CONST: return "DIRAC_CONST";
        case EXPR_DIRAC_IN:    return "DIRAC_IN";
    }"""))
R.append(("""        case EXPR_INNER:
            /* left/right (vector refs) get printed by the recursive walk */
            break;
    }""", """        case EXPR_DIRAC_REF:
            fprintf(stderr, " id=%d (%s) role=%d", n->id, v_table[n->id].name, n->op);
            break;
        case EXPR_DIRAC_PROD:
            /* left/right get printed by the recursive walk */
            break;
        case EXPR_DIRAC_CONST:
            fprintf(stderr, " %s", n->op == DIRAC_EYE ? "|I|" : "|0>");
            break;
        case EXPR_DIRAC_IN:
            fprintf(stderr, " port=%d", n->id);
            break;
    }
    if (n->rows) fprintf(stderr, " shape=%dx%d", n->rows, n->cols);"""))

# 3. typecheck ---------------------------------------------------------------
R.append(("""        case EXPR_INNER:
            // <a|b> reduces via exec_vtv, which returns the element type of
            // the first operand. left is an EXPR_VAR whose type already
            // carries v_table[id].type, so just propagate.
            n->type = lt;
            break;
""", """        case EXPR_DIRAC_PROD:
            // the element type of a product: comp > float > int, as the
            // arithmetic operators (an inner product <a|b> of two arrays of
            // one type has that type, as exec_vtv returns)
            n->type = (lt > rt) ? lt : rt;
            break;
"""))

# 4. shapes, before the Sethi-Ullman helpers ----------------------------------
R.append(("""// Sethi-Ullman labelling: how many hardware-stack slots the subtree needs""",
"""// ----------------------------------------------------------------------------
// shapes (Dirac notation) -----------------------------------------------------
// ----------------------------------------------------------------------------
// Every node gets (rows, cols): (1, 1) a number, (N, 1) a ket, (1, N) a bra,
// (N, M) a matrix, DIRAC_ANY a size the context decides. See
// docs/design/dirac-expressions.md section 3.2.

static int is_number(const expr_node *n) {return n->rows == 1 && n->cols == 1;}
static int dim_fits (int a, int b)       {return a == DIRAC_ANY || b == DIRAC_ANY || a == b;}
static int dim_pick (int a, int b)       {return a == DIRAC_ANY ? b : a;}

// a shape in words, as the messages print it
static const char *shape_text(int rows, int cols, char *buf, size_t sz)
{
    if      (rows == 1 && cols == 1)         snprintf(buf, sz, "%s", MSG_SHAPE_NUMBER);
    else if (cols == 1 && rows == DIRAC_ANY) snprintf(buf, sz, "%s", MSG_SHAPE_ANY_VECTOR);
    else if (cols == 1)                      snprintf(buf, sz, MSG_SHAPE_VECTOR, rows);
    else if (rows == 1)                      snprintf(buf, sz, MSG_SHAPE_ROW, cols);
    else if (rows == DIRAC_ANY || cols == DIRAC_ANY) snprintf(buf, sz, "%s", MSG_SHAPE_ANY_MATRIX);
    else                                     snprintf(buf, sz, MSG_SHAPE_MATRIX, rows, cols);
    return buf;
}

static void shape_not_number(const expr_node *n)
{
    char b[64];
    fprintf(stderr, MSG_ERR_DIRAC_NOT_SCALAR, n->line, shape_text(n->rows, n->cols, b, sizeof b));
    exit(EXIT_FAILURE);
}

void shapecheck_expr(expr_node *n)
{
    if (!n || n->rows != 0) return;  // checked already

    shapecheck_expr(n->left);
    shapecheck_expr(n->right);
    for (int i = 0; i < n->n_args; i++) shapecheck_expr(n->args[i]);

    expr_node *L = n->left, *R = n->right;
    char b1[64], b2[64];

    switch (n->kind)
    {
        case EXPR_DIRAC_REF: {
            int id = n->id;
            char *name = rem_fname(v_table[id].name, fname);
            if (v_table[id].type == 0) {fprintf(stderr, MSG_ERR_DECL_FIRST, n->line, name); exit(EXIT_FAILURE);}
            if (n->op == DIRAC_ROLE_MAT) {
                if (v_table[id].isar != 2) {fprintf(stderr, MSG_ERR_NOT_A_MATRIX, n->line, name); exit(EXIT_FAILURE);}
                n->rows = v_table[id].size;
                n->cols = v_table[id].siz2;
            } else {
                if (v_table[id].isar != 1) {fprintf(stderr, MSG_ERR_NOT_A_VECTOR, n->line, name); exit(EXIT_FAILURE);}
                n->rows = (n->op == DIRAC_ROLE_KET) ? v_table[id].size : 1;
                n->cols = (n->op == DIRAC_ROLE_KET) ? 1 : v_table[id].size;
            }
            return;
        }

        case EXPR_DIRAC_CONST:
            n->rows = DIRAC_ANY;
            n->cols = (n->op == DIRAC_EYE) ? DIRAC_ANY : 1;
            return;

        case EXPR_DIRAC_IN:
            n->rows = DIRAC_ANY;
            n->cols = 1;
            return;

        case EXPR_DIRAC_PROD:
            if      (is_number(L)) {n->rows = R->rows; n->cols = R->cols;}
            else if (is_number(R)) {n->rows = L->rows; n->cols = L->cols;}
            else if (!dim_fits(L->cols, R->rows)) {
                fprintf(stderr, MSG_ERR_DIRAC_PRODUCT_SHAPE, n->line,
                        shape_text(L->rows, L->cols, b1, sizeof b1),
                        shape_text(R->rows, R->cols, b2, sizeof b2));
                exit(EXIT_FAILURE);
            }
            else {n->rows = L->rows; n->cols = R->cols;}
            return;

        case EXPR_BINOP:
            if ((n->op == OP_ADD || n->op == OP_SUB) && !(is_number(L) && is_number(R))) {
                if (!dim_fits(L->rows, R->rows) || !dim_fits(L->cols, R->cols)) {
                    fprintf(stderr, MSG_ERR_DIRAC_SUM_SHAPE, n->line,
                            shape_text(L->rows, L->cols, b1, sizeof b1),
                            shape_text(R->rows, R->cols, b2, sizeof b2));
                    exit(EXIT_FAILURE);
                }
                n->rows = dim_pick(L->rows, R->rows);
                n->cols = dim_pick(L->cols, R->cols);
                return;
            }
            break;

        case EXPR_UNOP:
            if (n->op == OP_NEG) {n->rows = L->rows; n->cols = L->cols; return;}
            break;

        default: break;
    }

    // everything else takes numbers and gives a number
    if (L && !is_number(L)) shape_not_number(L);
    if (R && !is_number(R)) shape_not_number(R);
    for (int i = 0; i < n->n_args; i++) if (!is_number(n->args[i])) shape_not_number(n->args[i]);
    n->rows = n->cols = 1;
}

// Sethi-Ullman labelling: how many hardware-stack slots the subtree needs"""))

# 5. ast_emit_expr: a number only --------------------------------------------
R.append(("""    typecheck_expr(n);  // annotate composite n->type before codegen
    expr e = ast_emit_expr_impl(n);""",
"""    typecheck_expr(n);  // annotate composite n->type before codegen
    shapecheck_expr(n); // a vector or a matrix has no place in a scalar result
    if (n && !is_number(n)) shape_not_number(n);
    expr e = ast_emit_expr_impl(n);"""))

R.append(("""        case EXPR_INNER: {
            // left / right are EXPR_VAR children carrying the vector ids
            return exec_vtv(n->left->id, n->right->id);
        }
    }""", """        case EXPR_DIRAC_PROD:
            // a number from the Dirac notation: today only <a|b> (exec_vtv)
            if (n->left->kind  == EXPR_DIRAC_REF && n->left->op  == DIRAC_ROLE_BRA &&
                n->right->kind == EXPR_DIRAC_REF && n->right->op == DIRAC_ROLE_KET)
                return exec_vtv(n->left->id, n->right->id);
            fprintf(stderr, MSG_ERR_DIRAC_NOT_YET, n->line);
            exit(EXIT_FAILURE);

        case EXPR_DIRAC_REF:    // not a number: stopped by shapecheck_expr
        case EXPR_DIRAC_CONST:
        case EXPR_DIRAC_IN:
            break;
    }"""))

# 6. statement constructors --------------------------------------------------
R.append(("""stmt_node *stmt_vout(int port, expr_node *rhs, int vector_id)
{
    stmt_node *n = snode_new(STMT_VOUT);
    n->id  = port;
    n->id2 = vector_id;
    n->rhs = rhs;
    return n;
}

""", ""))

R.append(("""stmt_node *stmt_declar_Mv(int id_var, int id_N, int id_M, int id_v)
{
    stmt_node *n = snode_new(STMT_DECLAR_MV);
    n->id  = id_var;
    n->id2 = id_N;
    n->id3 = id_M;
    n->id4 = id_v;
    return n;
}

stmt_node *stmt_declar_cv(int id_var, int id_N, expr_node *c, int id_v)
{
    stmt_node *n = snode_new(STMT_DECLAR_CV);
    n->id  = id_var;
    n->id2 = id_N;
    n->id3 = id_v;
    n->rhs = c;
    return n;
}""", """stmt_node *stmt_declar_dirac(int id_var, int id_N, expr_node *rhs)
{
    stmt_node *n = snode_new(STMT_DECLAR_DIRAC);
    n->id  = id_var;
    n->id2 = id_N;
    n->rhs = rhs;
    return n;
}"""))

for a, b in R:
    assert s.count(a) == 1, a[:70]
    s = s.replace(a, b)

i0 = s.index("static stmt_node *dirac_new(dirac_op op)")
i1 = s.index("// ----------------------------------------------------------------------------\n// STMT_BLOCK + stmt_list scaffold stack")
s = s[:i0] + """stmt_node *stmt_dirac(int dst, expr_node *rhs)
{
    stmt_node *n = snode_new(STMT_DIRAC);
    n->id  = dst;
    n->rhs = rhs;
    return n;
}

stmt_node *stmt_dirac_shift(int dst, expr_node *c, int a)
{
    stmt_node *n = snode_new(STMT_DIRAC_SHIFT);
    n->id  = dst;
    n->id2 = a;
    n->rhs = c;
    return n;
}

""" + s[i1:]

open(p, 'w', encoding='utf-8', newline='').write(s.replace('\n', '\r\n'))
print("ok")
