// ----------------------------------------------------------------------------
// AST types for cmmcomp's expression and statement codegen -------------------
// ----------------------------------------------------------------------------
//
// Two parallel tree shapes live here:
//
//   - `expr_node` carries the expression subtrees the parser builds while
//     reducing `exp`. The bison %union holds `expr_node *` directly. Codegen
//     is deferred: when a statement-level consumer calls ast_emit_expr(), the
//     walker traverses the tree and dispatches to the existing oper_*/exec_*/
//     arr_*/pplus_*/id2exp/num2exp helpers in post-order DFS - exactly the
//     order the parser used to reduce, which keeps the assembly stream
//     byte-identical to the older inline-emit compiler.
//
//   - `stmt_node` carries whole statements. Each function body and each
//     compound body (if / while / switch) accumulates its statements in a
//     STMT_BLOCK while parsing; the closing reduce hands the block to
//     stmt_emit, which walks the tree and emits every instruction at body-
//     close time (no parse-time emit, no textual replay).

#ifndef YANC_AST_H
#define YANC_AST_H

// ----------------------------------------------------------------------------
// expression result POD -------------------------------------------------------
// ----------------------------------------------------------------------------
//
// Returned by ast_emit_expr() and by the oper_*/exec_*/id2exp/num2exp/arr_*/
// pplus_* helpers inside the walker. Carries enough to describe where the
// freshly-emitted value lives so the next emit helper can dispatch.
//
// type encoding:
//   0 = undefined / void
//   1 = int
//   2 = float
//   3 = comp (real half / variable)
//   4 = comp (imag half)
//   5 = comp constant (e.g. 3+7.5i)
//
// id is the index into v_table; id == 0 means "the result lives in the
// accumulator, not in a variable".
typedef struct {
    int type;
    int id;
} expr;

expr expr_make(int type, int id);

// ----------------------------------------------------------------------------
// expression AST nodes -------------------------------------------------------
// ----------------------------------------------------------------------------
//
// Each `exp` reduction builds one of these and hands it up the bison stack
// without emitting assembly. The struct is intentionally flat (no union);
// per-node overhead is a few ints, in exchange for `n->left` / `n->id`
// staying simple at every call site. Codegen happens later through
// ast_emit_expr().

typedef enum {
    EXPR_LITERAL,      // INUM / FNUM / CNUM materialized into v_table
    EXPR_VAR,          // reference to a declared variable
    EXPR_BINOP,        // a <op> b   (op in +, -, *, /, %, &, |, ^, <<, >>, comparisons, &&, ||, ...)
    EXPR_UNOP,         // <op> a     (op in -, !, ~)
    EXPR_ARRAY_INDEX,  // array[idx]  (1D auto / reversed; 2D adds right)
    EXPR_PPLUS,        // postfix ++ (scalar id, or array[idx] / [idx][idx2])
    EXPR_STDLIB_CALL,  // in() / fin() / pst() / abs() / sign() / sqrt() / ... (op picks which)
    EXPR_FUNC_CALL,    // user-defined function call f(a, b, ...) - args[] is n-ary
    EXPR_INNER         // Dirac inner product <a|b> ; left/right are the two vector refs
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
    OP_STD_TAN,   // tan(x)
    OP_STD_COSH,  // cosh(x)         -> hyperbolic cosine
    OP_STD_SINH,  // sinh(x)         -> hyperbolic sine
    OP_STD_TANH,  // tanh(x)         -> hyperbolic tangent
    OP_STD_EXP,   // exp(x)          -> e^x
    OP_STD_LOG,   // log(x)          -> natural logarithm (ln)
    OP_STD_POW,   // pow(x, y)       -> x^y  (int literal y: square-and-multiply; else exp(y*log x))
    OP_STD_FLOOR, // floor(x)        -> largest integral float <= x
    OP_STD_CEIL,  // ceil(x)         -> smallest integral float >= x
    OP_STD_ROUND, // round(x)        -> nearest integral float, ties away from zero
    OP_STD_REAL,  // real(x)         -> real part of a comp
    OP_STD_IMAG,  // imag(x)         -> imag part of a comp
    OP_STD_COMP,  // complex(x, y)   -> x + y*i
    OP_STD_FASE,  // fase(x)         -> phase of a comp (radians)
    OP_STD_MOD2,  // mod2(x)         -> squared magnitude of a comp
    OP_STD_CONJ,  // conj(x)         -> complex conjugate (real -> x+0i); always comp
} expr_op;

typedef struct expr_node {
    expr_kind kind;
    int line;                          // source line (1-based)
    int type;                          // data type (1=int, 2=float, 3=comp, 5=comp const)
    int id;                            // EXPR_LITERAL / EXPR_VAR: index into v_table
    int op;                            // EXPR_BINOP / EXPR_UNOP: operator code
    struct expr_node *left, *right;    // EXPR_BINOP: both; EXPR_UNOP: left only
    struct expr_node **args;           // EXPR_FUNC_CALL: argument expressions (owned)
    int n_args;                        // EXPR_FUNC_CALL: argument count

    // Idempotency cache: the walker auto-marks a node after it emits it, so
    // calling ast_emit_expr() twice on the same node returns the first
    // result instead of re-emitting. Producers never set these by hand.
    int  emitted;
    expr cached;
} expr_node;

// constructors. Leaf nodes (literal, var, array_index, pplus, func_call)
// take the result type from the producer rule that already knows it.
// Composite nodes (binop, unop, stdlib, inner) do NOT - typecheck_expr
// fills n->type bottom-up before codegen runs. All return a heap node
// owned by the caller; expr_free() recurses through children.
expr_node *expr_lit  (int type, int id);
expr_node *expr_var  (int type, int id);
expr_node *expr_binop(int op, expr_node *left, expr_node *right);
expr_node *expr_unop (int op, expr_node *operand);

// id is the array's v_table index. reversed = 0 for x[i] (auto),
// 1 for x[i) (FFT bit-reversed). idx2 is NULL for 1D access.
expr_node *expr_array_index(int type, int id, int reversed,
                            expr_node *idx, expr_node *idx2);

// id is the variable being post-incremented. idx / idx2 are NULL for a
// scalar id++; 1D id[i]++ has idx and NULL; 2D id[i][j]++ has both.
expr_node *expr_pplus(int type, int id, expr_node *idx, expr_node *idx2);

// generic stdlib call. `port` is the INUM port number for IN/FIN/OUT and
// 0 for everything else. a/b are operand expressions (NULL if unused).
expr_node *expr_stdlib(int op, int port, expr_node *a, expr_node *b);

// user-defined function call. id is the function's v_table index. args is a
// heap array of length n_args; the node takes ownership of both the array
// and the nodes it points at (expr_free recurses).
expr_node *expr_func_call(int type, int id, expr_node **args, int n_args);

// Dirac inner product <a|b>. a and b are vector references (typically built
// via expr_var() with the array's v_table index).
expr_node *expr_inner(expr_node *a, expr_node *b);

// frees the node and every descendant recursively
void expr_free(expr_node *n);

// dumps a human-readable indented tree to stderr (for debugging)
void expr_dump(expr_node *n);

// recursively walks an expression tree and emits the corresponding assembly
// via the existing oper_* / exec_* / arr_* / pplus_* / num2exp / id2exp
// helpers. Returns the result POD the same way those helpers do. Driven by
// every consumer of an exp value (via the EE() macro in CMMComp.y), plus
// directly by vcall() for void statement-level function calls.
expr ast_emit_expr(expr_node *n);
void typecheck_expr(expr_node *n);   // bottom-up: fills n->type on composites

// ----------------------------------------------------------------------------
// statement AST --------------------------------------------------------------
// ----------------------------------------------------------------------------
//
// Each top-level statement reduces into a stmt_node and the grammar appends
// it to the current body's STMT_BLOCK via stmt_append. The walker fires
// at body close (func_ret / if_stmt / if_fim / while_stmt / end_switch) and
// emits every kid in source order.

typedef enum {
    STMT_ASSIGN,        // id = exp;
    STMT_PPLUS,         // id++; / id[idx]++; / id[idx][idx2]++;
    STMT_ARRAY_ASSIGN,  // id[idx] = exp;  /  id[idx) = exp;  /  id[idx][idx2] = exp;
    STMT_RETURN,        // return exp;  (rhs set)  /  return;  (rhs NULL)
    STMT_OUT,           // out(port, exp);  (op=0)  /  fout(port, exp);  (op=1)
    STMT_COPY,          // copy(exp, dst_id);
    STMT_VOUT,          // out(port, exp | vector_id BRA);  Dirac vector output
    STMT_DIRAC,         // Dirac linear-algebra assignments (op discriminates)
    STMT_VOID_CALL,     // f(args);  user-defined void function call statement
    STMT_DIRE_INTER,    // #PRACA directive (interrupt return target)
    STMT_DIRE_TOAQUI,   // #TOAQUI directive (cheguei pin trigger address)
    STMT_BLOCK,         // ordered list of child stmt_nodes (function / if /
                        // while / switch body). Walker iterates kids.
    STMT_IF,            // if (cond) then_body [else else_body];  label = if id
    STMT_WHILE,         // while (cond) body;  label = while id
    STMT_DO,            // do body while (cond);  label = while id (shares Lwh<n>
                        // namespace); cond tested at the bottom, body runs >=1x;
                        // op=1 emits the Lwh<n>cont continue label before the test
    STMT_SWITCH,        // switch (cond) body;  id=swit_id, id2=case_max
    STMT_CASE_LABEL,    // case <val>:   id=swit_id, id2=case_idx, id3=val_id, id4=val_type
    STMT_DEFAULT_LABEL, // default:      id=swit_id, id2=case_idx
    STMT_SWITCH_BREAK,  // break; inside a switch case (id=swit_id)
    STMT_BREAK_WHILE,   // break; inside a while loop  (id=enclosing while label)
    STMT_CONTINUE,      // continue; (id=enclosing loop label, op=1 if a for -> jump
                        // to its step label Lwh<id>cont, op=0 if a while -> jump to top)
    STMT_DECLAR_ARR_1D, // int/float/comp x[N];  or with file init  id=id_var, id2=id_arg, id3=id_fname
    STMT_DECLAR_ARR_2D, // int/float/comp x[N][M];  id=id_var, id2=id_x, id3=id_y, id4=id_fname
    STMT_DECLAR_MV,     // float a[N] # |M|v⟩;     id=id_var, id2=id_N, id3=id_M, id4=id_v
    STMT_DECLAR_CV,     // float a[N] # c|v⟩;      id=id_var, id2=id_N, id3=id_v, rhs=c expression
    STMT_FUNC_HEADER,   // @label + optional JMP main + per-param SET[_P];
                        // id=func v_table index, op=needs_jmp_main flag,
                        // params[]/n_params=param ids in declaration order
    STMT_DIRECTIVE,     // #PRNAME / #NUBITS / ... ; dir_str=directive token,
                        // id=v_table index of the operand value
    STMT_FUNC,          // a full function definition: walks body (which begins
                        // with STMT_FUNC_HEADER) then emits the trailing
                        // @fim JMP fim (main) or RET (void). id=func v_table
                        // index, body=function body stmt_list
} stmt_kind;

// Sub-operation for STMT_DIRAC. The 10 Dirac forms share enough structure
// to live under one kind, dispatched by op in the walker.
typedef enum {
    DIRAC_MV,      // dst # |M|a⟩;             3 ids
    DIRAC_CV,      // dst # c|b⟩;              2 ids + rhs (scalar c)
    DIRAC_APCB,    // dst # |b⟩ + c|d⟩;        3 ids + rhs (scalar c)
    DIRAC_VVT,     // dst # |a⟩⟨b|;            3 ids
    DIRAC_MMVVT,   // dst # |B| - |a⟩⟨b|;      4 ids
    DIRAC_CM,      // dst # c|B|;              2 ids + rhs (scalar c)
    DIRAC_CI,      // dst # c|I|;              1 id  + rhs (scalar c)
    DIRAC_V0,      // dst # |0⟩;               1 id
    DIRAC_CVIN,    // dst # c|in(port)⟩;       2 ids + rhs (scalar c)
    DIRAC_SHIFT,   // dst # c -> |a⟩;          2 ids + rhs (scalar c)
} dirac_op;

typedef struct stmt_node {
    stmt_kind kind;
    int       line;     // source line where the node was built (1-based)
    int       emitted;  // idempotency guard: the walker sets it after emitting
                        // the node and no-ops on a revisit (e.g. a for-loop's
                        // init/step parked on two slots can be reached twice)

    int               id;    // primary int (LHS variable / array / port / dst)
    int               id2;   // secondary int
    int               id3;   // tertiary int (DIRAC: third var id)
    int               id4;   // quaternary int (DIRAC_MMVVT only)
    int               op;    // small flag (fft / fout / dirac_op / ...)
    struct expr_node *rhs;   // value/source expression (owned)
    struct expr_node *idx;   // PPLUS / ARRAY_ASSIGN: 1D/2D index (NULL for scalar)
    struct expr_node *idx2;  // PPLUS / ARRAY_ASSIGN: 2D second index (NULL otherwise)

    // STMT_BLOCK: child statements (owned via stmt_free recursion)
    struct stmt_node **kids;
    int                kids_n;
    int                kids_cap;

    // STMT_FUNC_HEADER: parameter ids in declaration order (owned, freed by stmt_free)
    int               *params;
    int                n_params;

    // STMT_DIRECTIVE: directive token like "#PRNAME" / "#NUBITS" / ...
    // Pointed at a string literal (the grammar's dire_exec call site), so
    // not owned and not freed.
    const char        *dir_str;

    // compound control flow (IF / WHILE / SWITCH): cond is the condition
    // expression evaluated at walker time. then_body/else_body/body are
    // STMT_BLOCKs owned by this node.
    struct expr_node *cond;
    struct stmt_node *then_body;
    struct stmt_node *else_body;
    struct stmt_node *body;
} stmt_node;

// constructors (caller owns the returned node).
stmt_node *stmt_assign(int id, struct expr_node *rhs);

// scalar id++:      idx=NULL,    idx2=NULL
// 1D    id[i]++:    idx=<index>, idx2=NULL
// 2D    id[i][j]++: idx=<i>,     idx2=<j>
stmt_node *stmt_pplus (int id, struct expr_node *idx, struct expr_node *idx2);

// 1D     id[i]  = rhs:  idx=<i>, idx2=NULL, fft=0
// 1D rev id[i)  = rhs:  idx=<i>, idx2=NULL, fft=1
// 2D     id[i][j]=rhs:  idx=<i>, idx2=<j>,  fft=0
stmt_node *stmt_array_assign(int id,
                             struct expr_node *idx,
                             struct expr_node *idx2,
                             struct expr_node *rhs,
                             int fft);

// rhs=NULL -> void return; rhs=<expr> -> value return.
stmt_node *stmt_return(struct expr_node *rhs);

// out(port, rhs)  -> fout_flag=0
// fout(port, rhs) -> fout_flag=1
stmt_node *stmt_out(int port, struct expr_node *rhs, int fout_flag);

// copy(rhs, dst_id);   no AST node for the destination, just its var id.
stmt_node *stmt_copy(struct expr_node *rhs, int dst_id);

// out(port, rhs | vector_id BRA);  Dirac vector output.
stmt_node *stmt_vout(int port, struct expr_node *rhs, int vector_id);

// statement-form user void call: f(args);  inner is the EXPR_FUNC_CALL with
// the popped args already bound. Walker invokes ast_emit_expr on it.
stmt_node *stmt_void_call (struct expr_node *call);

// #PRACA directive (interrupt entry-point marker) used inside funcs.
stmt_node *stmt_dire_inter(void);

// #TOAQUI directive (PC tracker marker -> cheguei pin)
stmt_node *stmt_dire_toaqui(void);

// Empty STMT_BLOCK; grow it with stmt_block_push.
stmt_node *stmt_block     (void);
void       stmt_block_push(stmt_node *blk, stmt_node *kid);

// Compound control-flow constructors (cond/body trees are owned).
stmt_node *stmt_if          (int label, struct expr_node *cond,
                             stmt_node *then_body, stmt_node *else_body);
stmt_node *stmt_while       (int label, struct expr_node *cond, stmt_node *body);
stmt_node *stmt_do          (int label, struct expr_node *cond, stmt_node *body);
stmt_node *stmt_switch      (int swit_id, int case_max,
                             struct expr_node *cond, stmt_node *body);
stmt_node *stmt_case_label  (int swit_id, int case_idx, int val_id, int val_type);
stmt_node *stmt_default_label(int swit_id, int case_idx);
stmt_node *stmt_switch_break(int swit_id);
stmt_node *stmt_break_while (int while_label);

// continue; -- loop_label is the enclosing loop's while-label. cont_lbl picks the
// jump target: a for or do-while jumps to Lwh<n>cont (the for runs its step there,
// the do-while re-tests its bottom condition); a while jumps to the top (Lwh<n>).
stmt_node *stmt_continue    (int loop_label, int cont_lbl);

// Array declarations. The v_table side effects (v_type / v_isar / v_size /
// v_fnid + f_log writes) already ran at parse time when the constructor was
// called; the stmt_node only carries what the walker needs to emit the
// `#array` directive (and, for 2D, the LOD/SET arr_size helper instructions).
stmt_node *stmt_declar_arr_1d(int id_var, int id_arg, int id_fname);
stmt_node *stmt_declar_arr_2d(int id_var, int id_x, int id_y, int id_fname);

// Dirac-initialized 1D array declarations. The base array's #array directive
// is emitted by the same code path as stmt_declar_arr_1d; the Mv / cv stuff
// is then walked through exec_Mv / exec_cv.
stmt_node *stmt_declar_Mv   (int id_var, int id_N, int id_M, int id_v);
stmt_node *stmt_declar_cv   (int id_var, int id_N, struct expr_node *c, int id_v);

// Function prologue: @label + optional JMP main (when this is the first
// non-main function) + per-parameter SET[_P]. params[] is consumed (takes
// ownership of the heap array; freed by stmt_free).
stmt_node *stmt_func_header (int func_id, int needs_jmp_main,
                             int *params, int n_params);

// Top-level directive (#PRNAME / #NUBITS / ...). The dir argument is a
// string literal kept by reference (not freed). id is the v_table index
// of the operand (the value that follows the directive in source).
stmt_node *stmt_directive   (const char *dir, int id);

// Full function definition: walks body then emits the trailing JMP fim
// (for main) or RET (for void). id = function's v_table index. body is
// owned (freed by stmt_free).
stmt_node *stmt_func        (int func_id, stmt_node *body);

// Body-list stack. stmt_list_open() pushes a fresh STMT_BLOCK; every
// stmt_append call records the just-built stmt_node on the top block.
// stmt_list_close() pops the top block and returns ownership to the
// caller. parse_init opens the bottom of the stack (the program-wide
// list); the walker fires from prog_emit at `fim` and emits the block's
// children in source order. Nested blocks (function body, if-then /
// if-else, while body, switch body, case body) are pushed by their
// opening reduce and popped by the matching closer.
void       stmt_list_open       (void);
stmt_node *stmt_list_close      (void);

// Read the current top STMT_BLOCK without popping it. Helpers like
// case_test / switch_break append directly to it.
stmt_node *stmt_list_top        (void);

// Dirac linear-algebra assignments. One constructor per syntactic form; each
// internally tags the stmt_node with the corresponding dirac_op. `c` is the
// scalar factor expression where applicable (NULL otherwise).
stmt_node *stmt_dirac_Mv    (int dst, int M, int a);
stmt_node *stmt_dirac_cv    (int dst, struct expr_node *c, int b);
stmt_node *stmt_dirac_apcb  (int dst, int b, struct expr_node *c, int d);
stmt_node *stmt_dirac_vvt   (int dst, int a, int b);
stmt_node *stmt_dirac_Mmvvt (int dst, int B, int a, int b);
stmt_node *stmt_dirac_cM    (int dst, struct expr_node *c, int M);
stmt_node *stmt_dirac_cI    (int dst, struct expr_node *c);
stmt_node *stmt_dirac_v0    (int dst);
stmt_node *stmt_dirac_cvin  (int dst, struct expr_node *c, int port);
stmt_node *stmt_dirac_shift (int dst, struct expr_node *c, int a);

// walks a stmt_node and emits via the same helpers the inline grammar
// actions used to call (ass_set, etc.).
void       stmt_emit(stmt_node *n);

// frees the node and every descendant (rhs included).
void       stmt_free(stmt_node *n);

// Appends a just-built stmt_node to the currently-open STMT_BLOCK (the top of
// the body-list stack opened by stmt_list_open / parse_init). No codegen runs
// here -- the node is walked later by stmt_emit, driven by prog_emit at `fim`
// or by an enclosing body closer (if_fim / while_stmt / end_switch). The
// statement -> deferred-AST migration is complete; every add_sinst now fires
// from the walker, never from a parse-time action.
void       stmt_append(stmt_node *n);

#endif // YANC_AST_H
