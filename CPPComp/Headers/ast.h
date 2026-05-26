// ----------------------------------------------------------------------------
// CPPComp — abstract syntax tree -----------------------------------------------
// ----------------------------------------------------------------------------

#ifndef CPPCOMP_AST_H
#define CPPCOMP_AST_H

#include "types.h"

// ----------------------------------------------------------------------------
// expressions ----------------------------------------------------------------
// ----------------------------------------------------------------------------

typedef enum {
    E_INT_LIT, E_FLOAT_LIT, E_CHAR_LIT, E_STRING_LIT,
    E_IDENT,
    E_BINOP, E_UNOP,
    E_ASSIGN,             // pure '='; compound forms lower to '=' + binop in parser
    E_INDEX,              // a[i] — generic; works for arrays AND pointers
    E_CALL,
    E_MEMBER,             // s.f
    E_PMEMBER,            // s->f (sugar for (*s).f, but we keep a node to ease codegen)
    E_ADDR,               // &x
    E_DEREF,              // *p
    E_PREINC, E_PREDEC, E_POSTINC, E_POSTDEC,
    E_CAST,               // (T)x
    E_SIZEOF_T,           // sizeof(T)
    E_SIZEOF_E,           // sizeof(expr)
    E_TERNARY,            // a ? b : c
    E_COMMA,              // a, b
    E_COMPOUND,           // (T){ init }  — C99 compound literal
    E_GENERIC,            // _Generic(ctrl, T1: e1, ..., default: eD)  — C11
    E_NEW,                // new T / new T(args) / new T[n]  (C++)
    E_DELETE,             // delete p / delete[] p           (C++)
    E_TEMPOBJ             // T(args) — a temporary object built on the stack (C++)
} expr_kind;

typedef enum {
    OP_ADD, OP_SUB, OP_MUL, OP_DIV, OP_MOD,
    OP_BAND, OP_BOR, OP_BXOR,
    OP_SHL, OP_SHR,
    OP_EQ, OP_NE, OP_LT, OP_GT, OP_LE, OP_GE,
    OP_LAND, OP_LOR,
    OP_NEG, OP_BNOT, OP_LNOT, OP_POS,
    OP_NONE                      // sentinel: a plain '=' (not a compound assign)
} op_kind;

typedef struct expr expr;
typedef struct stmt stmt;
typedef struct decl decl;
typedef struct func func;

struct expr {
    expr_kind kind;
    int       line;
    type     *etype;     // assigned during type-check

    long      ival;      // INT_LIT / CHAR_LIT
    double    fval;      // FLOAT_LIT
    char     *sval;      // IDENT name / STRING_LIT bytes
    int       slen;      // length of STRING_LIT (excludes terminator)
    op_kind   op;        // BINOP/UNOP

    expr     *a, *b, *c; // generic children (lhs, rhs, third)
    expr    **args;      // CALL args
    int       n_args;
    type     *target_t;  // CAST target / SIZEOF_T target / COMPOUND type
    char     *member;    // MEMBER / PMEMBER field name
    struct initz *cinit; // COMPOUND literal initializer
    type    **gtypes;    // GENERIC: assoc types parallel to args (NULL = default);
                         // also CALL: explicit template type arguments fn<T...>(...)
    int       n_gtypes;  // number of explicit template type args on a CALL
};

// ----------------------------------------------------------------------------
// statements -----------------------------------------------------------------
// ----------------------------------------------------------------------------

typedef enum {
    S_NULL, S_EXPR, S_BLOCK,
    S_IF, S_WHILE, S_DOWHILE, S_FOR,
    S_SWITCH, S_CASE, S_DEFAULT,
    S_BREAK, S_CONTINUE, S_RETURN, S_GOTO, S_LABEL,
    S_DECL, S_ASM
} stmt_kind;

struct stmt {
    stmt_kind kind;
    int       line;

    expr     *e1, *e2, *e3;
    stmt     *body, *body2, *init_stmt;
    stmt    **items;        // BLOCK contents / SWITCH body items
    int       n_items;
    decl     *decls;        // S_DECL: linked-list of declarations
    char     *label;        // GOTO target / LABEL name
};

// ----------------------------------------------------------------------------
// declarations ---------------------------------------------------------------
// ----------------------------------------------------------------------------

typedef enum { SC_NONE = 0, SC_STATIC, SC_EXTERN, SC_TYPEDEF } storage_class;

// one initializer designator: positional, `.field`, or `[index]` (C99).
// Single level per element; chains like `.a.x` are not represented (but the
// element it designates may itself be a nested brace list).
enum { DESIG_NONE = 0, DESIG_FIELD = 1, DESIG_INDEX = 2 };
typedef struct { int kind; char *name; int idx; } init_desig;

// recursive aggregate initializer: a braced list whose items are either scalar
// expressions or further braced lists (so `{{1,2},{3,4}}` nests). Each item
// carries an optional single-level designator.
typedef struct initz initz;
struct initz {
    int          is_list;    // 1 = braced { ... }; 0 = scalar expr in `e`
    expr        *e;          // when !is_list
    initz      **items;      // when is_list
    init_desig  *desigs;     // parallel to items
    int          n;
    int          line;
};

struct decl {
    type   *dtype;
    char   *name;
    expr   *init;
    char   *init_file;       // for arrays initialised from "file.txt"
    initz  *binit;           // for aggregates initialised with { ... }
    storage_class sclass;
    int     is_const;        // declared `const` — assignment to it is an error
    expr  **ctor_args;       // `T v(args)` — stack construction arguments
    int     n_ctor_args;
    decl   *next;            // chain within one declaration statement
    int     line;
};

// ----------------------------------------------------------------------------
// function & translation unit ------------------------------------------------
// ----------------------------------------------------------------------------

struct func {
    type   *ret;
    char   *name;
    decl   *params;          // linked list via .next
    int     n_params;
    stmt   *body;            // a compound stmt
    storage_class sclass;
    int     line;
    int     is_recursive;    // participates in a call-graph cycle -> needs frames
    int     frame_size;      // words of stack frame (when is_recursive)
    type   *method_of;       // non-NULL: this is a class method; its class type
    char   *asm_label;       // emitted symbol (mangled by signature when overloaded)
    int     n_tparams;       // >0: a function template (params use t_tparam types)
};

// A non-type template parameter (e.g. `int N`) is referenced as an int with a
// sentinel value NTP_BASE+k (k = the parameter index). Array sizes that depend
// on it carry the same sentinel in `arr_size`. Substitution at instantiation
// replaces the sentinel with the concrete argument value. The base is chosen
// far above any realistic array length / constant.
#define NTP_BASE  0x7F000000

// A class template that has at least one NON-TYPE parameter is monomorphized for
// real (type-param-only templates stay type-erased as before). `proto` is the
// placeholder class (fields may carry sentinel array sizes); `methods` are the
// captured, not-yet-emitted member functions.
typedef struct ctmpl {
    char   *name;
    struct type *proto;
    struct func **methods;
    int     n_methods;
    int     n_tparams;
    int     isval[8];        // per parameter: 1 = non-type (value), 0 = type
} ctmpl;

// A concrete instantiation request `Name<vals...>`: the codegen clones the
// template's methods with these values substituted and emits them under labels
// suffixed by `suffix`.
typedef struct ctinst {
    ctmpl  *tmpl;
    int     vals[8];
    int     nvals;
    struct type *targs[8];   // type-parameter bindings (NULL for non-type slots)
    int     ntargs;
    char   *suffix;          // e.g. "_T4" / "_Tf" — appended to the tag and labels
    struct type *concrete;
} ctinst;

typedef struct {
    decl  **globals;
    int     n_globals;
    func  **funcs;
    int     n_funcs;
    func  **templates;       // captured function templates (instantiated on use)
    int     n_templates;
    ctmpl **ctmpls;          // captured class templates with non-type parameters
    int     n_ctmpls;
    ctinst **ctinsts;        // requested class-template instantiations
    int     n_ctinsts;

    char   *prname;
    int     nubits, nbmant, nbexpo, nugain, ndstac, sdepth, nuioin, nuioou, fftsiz, itradd;
} unit;

// ----------------------------------------------------------------------------
// constructors ---------------------------------------------------------------
// ----------------------------------------------------------------------------

expr *ast_int_lit  (long v, int line);
expr *ast_float_lit(double v, int line);
expr *ast_char_lit (long v, int line);
expr *ast_string_lit(char *bytes, int len, int line);    // takes ownership of bytes
expr *ast_ident    (char *n, int line);
expr *ast_assign   (expr *l, expr *r, int line);
expr *ast_assign_op(op_kind o, expr *l, expr *r, int line);   // compound `l OP= r`
expr *ast_binop    (op_kind o, expr *l, expr *r, int line);
expr *ast_unop     (op_kind o, expr *operand, int line);
expr *ast_index    (expr *arr, expr *idx, int line);
expr *ast_call     (expr *callee, expr **args, int nargs, int line);
expr *ast_member   (expr *base, char *name, int line);
expr *ast_pmember  (expr *base, char *name, int line);
expr *ast_addr     (expr *x, int line);
expr *ast_deref    (expr *p, int line);
expr *ast_xfix     (expr_kind k, expr *operand, int line);
expr *ast_cast     (type *t, expr *e, int line);
expr *ast_sizeof_t (type *t, int line);
expr *ast_compound (type *t, struct initz *z, int line);   // (T){ init }
expr *ast_new      (type *t, expr **args, int nargs, expr *count, int line);  // new
expr *ast_tempobj  (type *t, expr **args, int nargs, int line);              // T(args) temporary
expr *ast_delete   (expr *p, int is_array, int line);                         // delete
expr *ast_generic  (expr *ctrl, type **ts, expr **es, int n, int line);  // _Generic
expr *ast_sizeof_e (expr *e, int line);
expr *ast_ternary  (expr *c, expr *a, expr *b, int line);
expr *ast_comma    (expr *a, expr *b, int line);

stmt *ast_stmt     (stmt_kind k, int line);
decl *ast_decl     (type *t, char *n, expr *init, int line);

initz *ast_initz_expr(expr *e, int line);            // scalar initializer
initz *ast_initz_list(int line);                     // empty braced list
void   initz_add     (initz *z, initz *item, init_desig d);
func *ast_func     (type *ret, char *n, decl *params, int np, stmt *body, int line);
unit *ast_unit     (void);

const char *op_name(op_kind o);

#endif
