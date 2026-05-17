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
// Replaces the historic "int et" carrier that packed (type, id) via
//     et = type * OFST + id
// Now an expression value is just a tiny POD struct, passed by value through
// the bison value stack. Helpers expr_to_et / expr_of_et bridge to the legacy
// int representation so we can migrate consumers one by one without breaking
// the codegen in oper.c / data_assign.c / saltos.c / funcoes.c / etc.
//
// type encoding (matches the old OFST scheme):
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

expr expr_make (int type, int id);
int  expr_to_et(expr e);   // pack to the legacy int et
expr expr_of_et(int et);   // unpack from a legacy int et

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
