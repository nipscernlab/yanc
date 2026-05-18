// ----------------------------------------------------------------------------
// AST nodes for statements (incremental migration from inline codegen) -------
// ----------------------------------------------------------------------------

#ifndef YANC_AST_H
#define YANC_AST_H
//
// Step 1 (this file): just the data structure. Not wired into the parser
// nor into the emit path yet. Lets us look at the shape of the tree before
// we start touching CMMComp.y.
//
// Only statements/blocks live in the tree for now. Expressions stay as the
// existing "et" integer tag - migrating them is a much bigger job and is
// not part of this step.

typedef enum {
    AST_RAW,     // verbatim assembly text (escape hatch for not-yet-migrated emits)
    AST_BLOCK,   // sequence of child statements
    AST_IF,      // if (cond) body [else els]
    AST_WHILE,   // while (cond) body
    AST_BREAK,   // break; (inside while)
    AST_SWITCH   // switch (cond) { cases }
} ast_kind;

// ----------------------------------------------------------------------------
// expressions ----------------------------------------------------------------
// ----------------------------------------------------------------------------
//
// Carries an expression value through the bison stack and into every codegen
// consumer. Passed by value (small POD), no heap.
//
// type encoding:
//   0 = undefined / void
//   1 = int
//   2 = float
//   3 = comp (real half / variable)
//   4 = comp (imag half)
//   5 = comp constant (e.g. 3+7.5i)
//
// id is the index into v_name; id == 0 means "the result lives in the
// accumulator, not in a variable".
// forward declared because expr carries an optional pointer to its AST view
struct expr_node;

typedef struct {
    int type;
    int id;
    struct expr_node *node;  // optional AST: populated as producers migrate,
                             // NULL while a reduction hasn't been ported yet
} expr;

expr expr_make(int type, int id);

// ----------------------------------------------------------------------------
// expression AST (tree form, used by the upcoming codegen-pass refactor) ----
// ----------------------------------------------------------------------------
//
// Coexists with the flat `expr` POD above. Today the parser still emits
// assembly inline as it reduces `exp`, and the bison stack carries `expr`.
// The plan is to switch the bison stack to `expr_node *`, defer codegen
// until parse is done, and run analysis/optimization passes (constant
// folding, peephole, dead code, etc.) over the tree before emitting.
//
// This first step only declares the types - no constructors, no walker,
// no parser changes. The struct is intentionally flat (no union); per-node
// memory overhead is a few ints, in exchange for `n->id` / `n->left`
// staying simple at every call site.

typedef enum {
    EXPR_LITERAL,      // INUM / FNUM / CNUM materialized into v_name
    EXPR_VAR,          // reference to a declared variable
    EXPR_BINOP,        // a <op> b   (op in +, -, *, /, %, &, |, ^, <<, >>, comparisons, &&, ||, ...)
    EXPR_UNOP,         // <op> a     (op in -, !, ~)
    EXPR_ARRAY_INDEX,  // array[idx]  (1D auto / reversed; 2D adds right)
    EXPR_PPLUS,        // postfix ++ (scalar id, or array[idx] / [idx][idx2])
    EXPR_STDLIB_CALL,  // in() / fin() / pst() / abs() / sign() / sqrt() / ... (op picks which)
    EXPR_FUNC_CALL     // user-defined function call f(a, b, ...) - args[] is n-ary
} expr_kind;

// Operator codes carried by EXPR_BINOP / EXPR_UNOP. Names mirror the
// historical oper_*() functions in oper.c so the migration of each
// consumer reads as a one-to-one mapping. Grows as more rules switch.
typedef enum {
    OP_NEG,   // unary -  (oper_neg)
    OP_LIN,   // unary !  (oper_lin, "logical inverter" in the ISA)
    OP_INV,   // unary ~  (oper_inv)
    OP_ADD,   // x + y    (oper_soma)
    OP_SUB,   // x - y    (oper_subt)
    OP_MUL,   // x * y    (oper_mult)
    OP_DIV,   // x / y    (oper_divi)
    OP_MOD,   // x % y    (oper_mod)
    OP_LT,    // x < y    (oper_cmp,  type=0)
    OP_GT,    // x > y    (oper_cmp,  type=1)
    OP_EQ,    // x == y   (oper_cmp,  type=2)
    OP_GE,    // x >= y   (oper_greq)
    OP_LE,    // x <= y   (oper_leeq)
    OP_NE,    // x != y   (oper_dife)
    OP_LAN,   // x && y   (oper_lanor, type=0)
    OP_LOR,   // x || y   (oper_lanor, type=1)
    OP_SHL,   // x << y   (oper_shift, type=0)
    OP_SHR,   // x >> y   (oper_shift, type=1)
    OP_SSHR,  // x >>> y  (oper_shift, type=2)  signed shift right
    OP_AND,   // x & y    (oper_bitw,  type=0)
    OP_OR,    // x | y    (oper_bitw,  type=1)
    OP_XOR,   // x ^ y    (oper_bitw,  type=2)
    // stdlib calls (EXPR_STDLIB_CALL). port lives in id for IN/FIN/OUT.
    OP_STD_IN,    // in(port)        -> int
    OP_STD_FIN,   // fin(port)       -> float
    OP_STD_PST,   // pst(x)          -> clears if negative
    OP_STD_ABS,   // abs(x)          -> |x|
    OP_STD_SIGN,  // sign(x, y)      -> y with sign of x
    OP_STD_NRM,   // norm(x)         -> x / NUGAIN
    OP_STD_SQRT,  // sqrt(x)
    OP_STD_ATAN,  // atan(x)
    OP_STD_SIN,   // sin(x)
    OP_STD_COS,   // cos(x)
    OP_STD_REAL,  // real(x)         -> real part of a comp
    OP_STD_IMAG,  // imag(x)         -> imag part of a comp
    OP_STD_COMP,  // complex(x, y)   -> x + y*i
    OP_STD_FASE,  // fase(x)         -> phase of a comp (radians)
    OP_STD_MOD2,  // mod2(x)         -> squared magnitude of a comp
} expr_op;

typedef struct expr_node {
    expr_kind kind;
    int line;                          // source line (1-based)
    int type;                          // data type (1=int, 2=float, 3=comp, 5=comp const)
    int id;                            // EXPR_LITERAL / EXPR_VAR: index into v_name
    int op;                            // EXPR_BINOP / EXPR_UNOP: operator code
    struct expr_node *left, *right;    // EXPR_BINOP: both; EXPR_UNOP: left only
    struct expr_node **args;           // EXPR_FUNC_CALL: argument expressions (owned)
    int n_args;                        // EXPR_FUNC_CALL: argument count
} expr_node;

// constructors: type is passed in (no promotion logic yet - callers compute
// it when they care). All return a heap node owned by the caller.
expr_node *expr_lit  (int type, int id);
expr_node *expr_var  (int type, int id);
expr_node *expr_binop(int op, int type, expr_node *left, expr_node *right);
expr_node *expr_unop (int op, int type, expr_node *operand);

// id is the array's v_name index. reversed = 0 for x[i] (auto),
// 1 for x[i) (FFT bit-reversed). idx2 is NULL for 1D access.
expr_node *expr_array_index(int type, int id, int reversed,
                            expr_node *idx, expr_node *idx2);

// id is the variable being post-incremented. idx / idx2 are NULL for a
// scalar id++; 1D id[i]++ has idx and NULL; 2D id[i][j]++ has both.
expr_node *expr_pplus(int type, int id, expr_node *idx, expr_node *idx2);

// generic stdlib call. `port` is the INUM port number for IN/FIN/OUT and
// 0 for everything else. a/b are operand expressions (NULL if unused).
expr_node *expr_stdlib(int op, int type, int port, expr_node *a, expr_node *b);

// user-defined function call. id is the function's v_name index. args is a
// heap array of length n_args; the node takes ownership of both the array
// and the nodes it points at (expr_free recurses).
expr_node *expr_func_call(int type, int id, expr_node **args, int n_args);

// frees the node and every descendant recursively
void expr_free(expr_node *n);

// dumps a human-readable indented tree to stderr (for debugging)
void expr_dump(expr_node *n);

typedef struct ast_node {
    ast_kind kind;
    int      line;            // source line where the node was built (1-based)

    // AST_IF/AST_WHILE: label number from push_if/push_while
    // AST_SWITCH:      swit_cnt (current switch's id)
    int      label;
    int      case_max;        // AST_SWITCH: highest case index used (case_cnt at end)

    // AST_RAW ----------------------------------------------------------------
    char    *text;            // heap-owned, NUL-terminated assembly chunk

    // AST_BLOCK --------------------------------------------------------------
    struct ast_node **kids;
    int      kids_n;
    int      kids_cap;

    // AST_IF / AST_WHILE / AST_SWITCH ---------------------------------------
    // cond is currently always NULL: the condition is emitted inline before
    // body capture starts, so it does not need its own node yet. Kept in the
    // struct so a future pass that captures the cond as well has a slot.
    struct ast_node *cond;
    struct ast_node *body;    // then-branch / while-body / switch-body
    struct ast_node *els;     // else-branch (AST_IF only, NULL if absent)
} ast_node;

// constructors (all return a heap node owned by the caller).
// For AST_IF / AST_WHILE: label is the number returned by push_if / push_while
// at parse time, used by the emit walker to spell out @Lif%d / @Lwh%d markers.
// For AST_SWITCH:         swit_id is swit_cnt, case_max is case_cnt when the
// closing brace was reduced; the walker uses them to spell out the trailing
// @sw_case_<swit>_<case_max+1> and @switch_end_<swit> labels.
ast_node *ast_raw     (const char *text);
ast_node *ast_block   (void);
ast_node *ast_if      (int label, ast_node *body, ast_node *els);
ast_node *ast_while   (int label, ast_node *body);
ast_node *ast_break   (void);
ast_node *ast_switch  (int swit_id, int case_max, ast_node *body);

// appends kid to a BLOCK node (takes ownership of kid)
void      ast_block_push(ast_node *blk, ast_node *kid);

// frees the node and every descendant recursively
void      ast_free    (ast_node *n);

// walks a node and emits the corresponding assembly via add_instr/add_sinst
// (incomplete: only the migrated node kinds are handled, the rest abort)
void      ast_emit    (ast_node *n);

#endif // YANC_AST_H
