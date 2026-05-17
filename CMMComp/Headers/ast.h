// ----------------------------------------------------------------------------
// AST nodes for statements (incremental migration from inline codegen) -------
// ----------------------------------------------------------------------------
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
    AST_BREAK    // break;
} ast_kind;

typedef struct ast_node {
    ast_kind kind;
    int      line;            // source line where the node was built (1-based)

    // AST_RAW ----------------------------------------------------------------
    char    *text;            // heap-owned, NUL-terminated assembly chunk

    // AST_BLOCK --------------------------------------------------------------
    struct ast_node **kids;
    int      kids_n;
    int      kids_cap;

    // AST_IF / AST_WHILE -----------------------------------------------------
    struct ast_node *cond;    // chunk that loads the condition into the accumulator
    struct ast_node *body;    // then-branch / while-body
    struct ast_node *els;     // else-branch (AST_IF only, NULL if absent)
} ast_node;

// constructors (all return a heap node owned by the caller)
ast_node *ast_raw     (const char *text);
ast_node *ast_block   (void);
ast_node *ast_if      (ast_node *cond, ast_node *body, ast_node *els);
ast_node *ast_while   (ast_node *cond, ast_node *body);
ast_node *ast_break   (void);

// appends kid to a BLOCK node (takes ownership of kid)
void      ast_block_push(ast_node *blk, ast_node *kid);

// frees the node and every descendant recursively
void      ast_free    (ast_node *n);

// walks a node and emits the corresponding assembly via add_instr/add_sinst
// (incomplete: only the migrated node kinds are handled, the rest abort)
void      ast_emit    (ast_node *n);
