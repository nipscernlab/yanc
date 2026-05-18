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
typedef struct {
    int type;
    int id;
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
    EXPR_LITERAL,   // INUM / FNUM / CNUM materialized into v_name
    EXPR_VAR,       // reference to a declared variable
    EXPR_BINOP,     // a <op> b   (op in +, -, *, /, %, &, |, ^, <<, >>, comparisons, &&, ||, ...)
    EXPR_UNOP       // <op> a     (op in -, !, ~)
} expr_kind;

typedef struct expr_node {
    expr_kind kind;
    int line;                          // source line (1-based)
    int type;                          // data type (1=int, 2=float, 3=comp, 5=comp const)
    int id;                            // EXPR_LITERAL / EXPR_VAR: index into v_name
    int op;                            // EXPR_BINOP / EXPR_UNOP: operator code
    struct expr_node *left, *right;    // EXPR_BINOP: both; EXPR_UNOP: left only
} expr_node;

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
