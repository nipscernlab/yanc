/* ----------------------------------------------------------------------------
   CNIPS — bison grammar for the YANC C subset
   ----------------------------------------------------------------------------
   Grammar is loosely C99, restricted to constructs the target can execute:
     - scalar types int/float/char/void, pointers, 1-D arrays, structs
     - all expression operators, all looping/conditional constructs
     - functions with array/pointer params (passed via LEA + LDA/STA)
     - typedef, enum, const, static (storage class is mostly cosmetic here)
   What's intentionally omitted from v1: function pointers, variadic args,
   unions, multi-D arrays, VLAs.
   ---------------------------------------------------------------------------- */

%{
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "..\Headers\ast.h"
#include "..\Headers\types.h"
#include "..\Headers\symtab.h"
#include "..\Headers\messages.h"

int   yylex  (void);
extern int   yylineno;
void  yyerror(const char *s);

unit *g_unit = NULL;    // populated as we parse, used by lexer for #pragma yanc
int   g_str_len = 0;    // set by lexer alongside yylval.sval for STRING_LIT

// staging state for declarations and functions ------------------------------

static int            ts_typedef = 0;       // we're inside a `typedef ...;`
static storage_class  ts_sclass  = SC_NONE;
static int            ts_is_const = 0;

// staged function params (added to symtab on body entry)
static decl  *param_head = NULL;
static decl  *param_tail = NULL;
static int    param_count = 0;

static void params_reset(void) { param_head = param_tail = NULL; param_count = 0; }
static void params_append(decl *d)
{
    if (!param_head) param_head = d; else param_tail->next = d;
    param_tail = d; param_count++;
}

// staged "current base type" used by declarators after `base_type` is reduced
static type *cur_base = NULL;
// stack of structs/unions currently being assembled (supports nested inline defs)
static type *cur_struct_stk[16];
static int   cur_struct_sp = 0;
static type *cur_struct = NULL;            // == top of the stack (or NULL)
static void cur_struct_push(type *t) { cur_struct_stk[cur_struct_sp++] = cur_struct; cur_struct = t; }
static void cur_struct_pop (void)    { cur_struct = cur_struct_stk[--cur_struct_sp]; }
// enum counter (reset per enum_specifier)
static long  enum_counter = 0;

// helpers --------------------------------------------------------------------

static char *xstrdup(const char *s) { char *r = malloc(strlen(s)+1); strcpy(r,s); return r; }

static void unit_add_global(decl *d)
{
    if (!g_unit) msg_internal("unit not allocated");
    g_unit->globals = realloc(g_unit->globals, sizeof(*g_unit->globals) * (g_unit->n_globals + 1));
    g_unit->globals[g_unit->n_globals++] = d;
}

static void unit_add_func(func *f)
{
    g_unit->funcs = realloc(g_unit->funcs, sizeof(*g_unit->funcs) * (g_unit->n_funcs + 1));
    g_unit->funcs[g_unit->n_funcs++] = f;
}

// wrap base type with N levels of pointer
static type *apply_pointers(type *base, int stars) { while (stars-- > 0) base = t_ptr(base); return base; }

// build a 1-argument call `fn(arg)` — used to desugar new/delete to malloc/free
static expr *mk_call1(const char *fn, expr *arg, int line)
{
    expr **args = malloc(sizeof(expr*));
    args[0] = arg;
    return ast_call(ast_ident(strdup(fn), line), args, 1, line);
}

// wrap base type with the given array dimensions, innermost-last (row-major):
// dims [3,4] over int  ->  array(3, array(4, int))
static type *build_array_type(type *base, const int *dims, int n)
{
    type *t = base;
    for (int i = n - 1; i >= 0; i--) t = t_array(t, dims[i]);
    return t;
}

// process a fully-typed declarator and produce a decl node
static decl *make_decl(type *base, int stars, char *name, const int *dims, int ndims, int line, expr *init)
{
    type *t = apply_pointers(base, stars);
    t = build_array_type(t, dims, ndims);
    decl *d = ast_decl(t, name, init, line);
    d->sclass = ts_sclass;
    // `const T *p` qualifies the pointee, not the pointer — the variable stays
    // writable (pointee-constness isn't enforced on this single-word target).
    // `const T x` / `const T a[]` still marks the object itself const.
    d->is_const = (stars > 0) ? 0 : ts_is_const;
    return d;
}

// builtin type-specifier accumulator -----------------------------------------
// C lets the keywords appear in any multiplicity/order (e.g. `long unsigned
// int`, `unsigned long long`). We OR a flag per keyword (counting `long`),
// then collapse to the few storage types the YANC actually has: every signed
// integer -> one int word, every unsigned integer -> one uint word, every
// floating type -> the one float word. char keeps its own (signed) word.
enum {
    TS_VOID   = 1<<0, TS_CHAR  = 1<<1, TS_INT    = 1<<2, TS_FLOAT = 1<<3,
    TS_DOUBLE = 1<<4, TS_SIGN  = 1<<5, TS_UNSIGN = 1<<6, TS_SHORT = 1<<7,
    TS_BOOL   = 1<<8, TS_LONG1 = 1<<9, TS_LONG2  = 1<<10,
    TS_LONG   = 1<<11   // sentinel returned by the `long` spec; folded by ts_add
};

static int ts_add(int acc, int spec)
{
    if (spec == TS_LONG)
        return acc | ((acc & TS_LONG1) ? TS_LONG2 : TS_LONG1);
    return acc | spec;
}

static type *resolve_builtin(int f)
{
    if (f & TS_VOID)                 return t_void();
    if (f & (TS_FLOAT | TS_DOUBLE))  return t_float();   // float/double/long double
    if (f & TS_BOOL)                 return t_uint();
    if (f & TS_CHAR)                 return (f & TS_UNSIGN) ? t_uint() : t_char();
    return (f & TS_UNSIGN) ? t_uint() : t_int();         // short/int/long/long long
}

// compile-time integer constant evaluator (for _Static_assert). Handles int/
// char literals, sizeof(type) (= words on this target), enum constants, casts,
// and the integer operators. Returns 0 if the expression isn't a constant.
static int const_eval(expr *e, long *out)
{
    if (!e) return 0;
    long a, b;
    switch (e->kind) {
    case E_INT_LIT: case E_CHAR_LIT: *out = e->ival; return 1;
    case E_SIZEOF_T: *out = type_size_words(e->target_t); return 1;
    case E_CAST:     return const_eval(e->a, out);
    case E_IDENT: {
        sym *s = st_find(e->sval);
        if (s && s->kind == SK_ENUM_CONST) { *out = s->enum_val; return 1; }
        return 0;
    }
    case E_UNOP:
        if (!const_eval(e->a, &a)) return 0;
        switch (e->op) {
        case OP_NEG: *out = -a; return 1;
        case OP_POS: *out =  a; return 1;
        case OP_BNOT:*out = ~a; return 1;
        case OP_LNOT:*out = !a; return 1;
        default: return 0;
        }
    case E_TERNARY:
        if (!const_eval(e->a, &a)) return 0;
        return const_eval(a ? e->b : e->c, out);
    case E_BINOP:
        if (!const_eval(e->a, &a) || !const_eval(e->b, &b)) return 0;
        switch (e->op) {
        case OP_ADD: *out = a + b; return 1;
        case OP_SUB: *out = a - b; return 1;
        case OP_MUL: *out = a * b; return 1;
        case OP_DIV: *out = b ? a / b : 0; return 1;
        case OP_MOD: *out = b ? a % b : 0; return 1;
        case OP_BAND:*out = a & b; return 1;
        case OP_BOR: *out = a | b; return 1;
        case OP_BXOR:*out = a ^ b; return 1;
        case OP_SHL: *out = a << b; return 1;
        case OP_SHR: *out = a >> b; return 1;
        case OP_EQ:  *out = (a == b); return 1;
        case OP_NE:  *out = (a != b); return 1;
        case OP_LT:  *out = (a <  b); return 1;
        case OP_GT:  *out = (a >  b); return 1;
        case OP_LE:  *out = (a <= b); return 1;
        case OP_GE:  *out = (a >= b); return 1;
        case OP_LAND:*out = (a && b); return 1;
        case OP_LOR: *out = (a || b); return 1;
        default: return 0;
        }
    default: return 0;
    }
}

static void check_static_assert(expr *cond, const char *msg, int line)
{
    long v;
    if (!const_eval(cond, &v))
        msg_error(line, "_Static_assert requires a constant integer expression");
    if (v == 0)
        msg_error(line, "static assertion failed%s%s", msg ? ": " : "", msg ? msg : "");
}
%}

%union {
    long    ival;
    double  fval;
    char   *sval;
    type   *typ;
    expr   *exp;
    stmt   *st;
    decl   *dcl;
    func   *fn;
    int     intval;
    op_kind opkind;
    struct { expr **arr; int n; } elist;
    initz *iz;
    struct { initz *z; init_desig d; } iitem;
    struct { type *t; expr *e; } gassoc;
    struct { type **ts; expr **es; int n; } galist;
    struct { stmt **arr; int n; } slist;
    struct { decl  *head; int n; } dlist;
    struct { int dims[8]; int n; } dimlist;
}

%token <ival>  INT_LIT CHAR_LIT
%token <fval>  FLOAT_LIT
%token <sval>  IDENT TYPEDEF_NAME STRING_LIT

%token KW_VOID KW_INT KW_FLOAT KW_CHAR KW_UNSIGNED KW_SIGNED
%token KW_SHORT KW_LONG KW_DOUBLE KW_BOOL
%token KW_IF KW_ELSE KW_WHILE KW_FOR KW_DO KW_SWITCH KW_CASE KW_DEFAULT
%token KW_BREAK KW_CONTINUE KW_RETURN KW_GOTO
%token KW_STRUCT KW_UNION KW_TYPEDEF KW_ENUM KW_SIZEOF KW_ASM KW_NEW KW_DELETE
%token KW_STATIC KW_EXTERN KW_CONST KW_STATIC_ASSERT KW_GENERIC

%token TOK_EQ TOK_NE TOK_LE TOK_GE TOK_SHL TOK_SHR TOK_LAND TOK_LOR
%token TOK_INC TOK_DEC
%token TOK_PLUSEQ TOK_MINUSEQ TOK_STAREQ TOK_SLASHEQ TOK_PERCENTEQ
%token TOK_AMPEQ TOK_PIPEEQ TOK_CARETEQ TOK_SHLEQ TOK_SHREQ
%token TOK_ARROW TOK_ELLIPSIS

%type <typ>   type_specifier struct_specifier union_specifier enum_specifier base_type decl_specifiers
%type <intval> pointers storage_or_qual_list storage_or_qual
%type <intval> builtin_spec builtin_type_seq
%type <exp>   expr assignment_expr conditional_expr logical_or_expr logical_and_expr
%type <exp>   bit_or_expr bit_xor_expr bit_and_expr equality_expr relational_expr
%type <exp>   shift_expr additive_expr multiplicative_expr cast_expr unary_expr
%type <exp>   postfix_expr primary_expr opt_expr
%type <iz>    init_item_list init_val
%type <iitem> init_item
%type <gassoc> generic_assoc
%type <galist> generic_assoc_list
%type <st>    stmt compound_stmt selection_stmt iteration_stmt jump_stmt
%type <st>    expr_stmt labeled_stmt block_item local_decl
%type <slist> block_item_list
%type <dlist> init_declarator_list param_list non_empty_param_list
%type <dcl>   init_declarator param_declarator
%type <dimlist> array_suffix
%type <elist> argument_list non_empty_argument_list
%type <opkind> assign_op

/* dangling-else: shift KW_ELSE over reducing an if-without-else */
%nonassoc IFX
%nonassoc KW_ELSE

%%

translation_unit:
      external_decl
    | translation_unit external_decl
    ;

external_decl:
      function_def
    | declaration                   { /* declaration's action adds it to globals (or registers typedef) */ }
    | KW_STATIC_ASSERT '(' conditional_expr ',' STRING_LIT ')' ';' { check_static_assert($3, $5, yylineno); }
    | KW_STATIC_ASSERT '(' conditional_expr ')' ';'                { check_static_assert($3, NULL, yylineno); }
    ;

/* ----- declarations ------------------------------------------------------- */

declaration:
      decl_specifiers init_declarator_list ';' {
          if (!ts_typedef) {
              for (decl *d = $2.head; d; ) {
                  decl *nx = d->next; d->next = NULL;
                  /* function prototypes have already been registered as SK_FUNC
                     in init_declarator — skip adding them as globals here */
                  sym *existing = st_find(d->name);
                  if (existing && existing->kind == SK_FUNC) {
                      /* nothing more to do */
                  } else {
                      int is_def = (d->sclass != SC_EXTERN);   // `extern x;` is only a declaration
                      if (existing && existing->kind == SK_GLOBAL_VAR) {
                          if (is_def && existing->defined)
                              msg_error(d->line, "redefinition of global '%s' "
                                  "(if this comes from a header, guard it with #ifndef or #pragma once)", d->name);
                          if (is_def) { existing->defined = 1; unit_add_global(d); }
                          /* extern re-declaration of a known global: nothing to emit */
                      } else {
                          sym *gs = st_add(SK_GLOBAL_VAR, d->name, d->name, d->dtype);
                          gs->defined = is_def;
                          if (is_def) unit_add_global(d);   /* extern-only: declared, no storage */
                      }
                  }
                  d = nx;
              }
          }
          ts_typedef = 0; ts_sclass = SC_NONE; ts_is_const = 0;
      }
    | decl_specifiers ';' {
          /* bare `struct foo { ... };` declares the tag, nothing else */
          ts_typedef = 0; ts_sclass = SC_NONE; ts_is_const = 0;
      }
    ;

decl_specifiers:
      base_type                              { $$ = $1; }
    | storage_or_qual_list base_type         { $$ = $2; }
    | base_type storage_or_qual_list         { $$ = $1; }
    ;

storage_or_qual_list:
      storage_or_qual                        { $$ = $1; }
    | storage_or_qual_list storage_or_qual   { $$ = $1 | $2; }
    ;

storage_or_qual:
      KW_STATIC    { ts_sclass = SC_STATIC; $$ = 0; }
    | KW_EXTERN    { ts_sclass = SC_EXTERN; $$ = 0; }
    | KW_TYPEDEF   { ts_typedef = 1; $$ = 0; }
    | KW_CONST     { ts_is_const = 1; $$ = 0; }
    ;

/* base_type stashes the resolved type in a static so init_declarators can grab it */
base_type:
      type_specifier                         { cur_base = $1; $$ = $1; }
    ;

type_specifier:
      builtin_type_seq                       { $$ = resolve_builtin($1); }
    | struct_specifier                       { $$ = $1;        }
    | union_specifier                        { $$ = $1;        }
    | enum_specifier                         { $$ = $1;        }
    | TYPEDEF_NAME                           {
          sym *s = st_find($1);
          if (!s || s->kind != SK_TYPEDEF) { msg_error(yylineno, "unknown type '%s'", $1); }
          $$ = s->stype;
          free($1);
      }
    ;

/* one or more builtin keywords in any order, folded into flags (see ts_add) */
builtin_type_seq:
      builtin_spec                           { $$ = ts_add(0, $1); }
    | builtin_type_seq builtin_spec          { $$ = ts_add($1, $2); }
    ;

builtin_spec:
      KW_VOID      { $$ = TS_VOID;   }
    | KW_CHAR      { $$ = TS_CHAR;   }
    | KW_INT       { $$ = TS_INT;    }
    | KW_SHORT     { $$ = TS_SHORT;  }
    | KW_LONG      { $$ = TS_LONG;   }
    | KW_FLOAT     { $$ = TS_FLOAT;  }
    | KW_DOUBLE    { $$ = TS_DOUBLE; }
    | KW_BOOL      { $$ = TS_BOOL;   }
    | KW_SIGNED    { $$ = TS_SIGN;   }
    | KW_UNSIGNED  { $$ = TS_UNSIGN; }
    ;

struct_specifier:
      KW_STRUCT IDENT '{' { cur_struct_push(t_make_struct($2)); st_add_tag($2, cur_struct); } field_list '}' {
          type *done = cur_struct;
          t_struct_seal(done, g_unit && g_unit->nubits > 0 ? g_unit->nubits : 16);
          cur_struct_pop();          // restore the enclosing struct (nested defs)
          $$ = done;
          free($2);
      }
    | KW_STRUCT IDENT {
          sym *s = st_find_tag($2);
          if (!s) {
              /* forward-declare incomplete tag — caller better seal it before sizeof */
              type *t = t_make_struct($2);
              st_add_tag($2, t);
              $$ = t;
          } else {
              $$ = s->struct_t;
          }
          free($2);
      }
    | KW_STRUCT '{' { cur_struct_push(t_make_struct("")); } field_list '}' {
          /* anonymous struct (e.g. a struct-typed field with no tag) */
          type *done = cur_struct;
          t_struct_seal(done, g_unit && g_unit->nubits > 0 ? g_unit->nubits : 16);
          cur_struct_pop();
          $$ = done;
      }
    ;

union_specifier:
      KW_UNION IDENT '{' { cur_struct_push(t_make_union($2)); st_add_tag($2, cur_struct); } field_list '}' {
          type *done = cur_struct;
          t_struct_seal(done, g_unit && g_unit->nubits > 0 ? g_unit->nubits : 16);
          cur_struct_pop();
          $$ = done;
          free($2);
      }
    | KW_UNION IDENT {
          sym *s = st_find_tag($2);
          if (!s) {
              type *t = t_make_union($2);
              st_add_tag($2, t);
              $$ = t;
          } else {
              $$ = s->struct_t;
          }
          free($2);
      }
    | KW_UNION '{' { cur_struct_push(t_make_union("")); } field_list '}' {
          /* anonymous union (e.g. a union-typed field with no tag) */
          type *done = cur_struct;
          t_struct_seal(done, g_unit && g_unit->nubits > 0 ? g_unit->nubits : 16);
          cur_struct_pop();
          $$ = done;
      }
    ;

field_list:
      field_decl
    | field_list field_decl
    ;

field_decl:
      base_type field_declarator_list ';'
    ;

field_declarator_list:
      field_declarator
    | field_declarator_list ',' field_declarator
    ;

/* explicit alternatives (no nullable pointers/array_suffix) so the bitfield
   `IDENT : N` form doesn't clash with a plain `IDENT` field on lookahead */
field_declarator:
      IDENT {
          t_struct_add_field(cur_struct, $1, cur_base);
          free($1);
      }
    | IDENT '[' INT_LIT ']' {
          t_struct_add_field(cur_struct, $1, t_array(cur_base, (int)$3));
          free($1);
      }
    | '*' IDENT {
          t_struct_add_field(cur_struct, $2, t_ptr(cur_base));
          free($2);
      }
    | IDENT ':' INT_LIT {
          /* bitfield `name : width` (e.g. unsigned flags : 3) */
          t_struct_add_bitfield(cur_struct, $1, cur_base, (int)$3);
          free($1);
      }
    | ':' INT_LIT {
          /* anonymous bitfield (padding / alignment) */
          t_struct_add_bitfield(cur_struct, "", cur_base, (int)$2);
      }
    | '(' '*' IDENT ')' '(' param_list ')' {
          /* function-pointer field `ret (*name)(params)` — an int function id */
          t_struct_add_field(cur_struct, $3, t_int());
          free($3);
      }
    ;

enum_specifier:
      KW_ENUM '{' { enum_counter = 0; } enum_list '}'              { $$ = t_int(); }
    | KW_ENUM IDENT '{' { enum_counter = 0; } enum_list '}'        { $$ = t_int(); free($2); }
    | KW_ENUM IDENT                                                { $$ = t_int(); free($2); }
    ;

enum_list:
      enum_item
    | enum_list ',' enum_item
    ;

enum_item:
      IDENT                                  { st_add_enum($1, enum_counter++); free($1); }
    | IDENT '=' INT_LIT                      { enum_counter = $3; st_add_enum($1, enum_counter++); free($1); }
    ;

/* ----- declarators -------------------------------------------------------- */

pointers:
      /* empty */         { $$ = 0; }
    | pointers '*'        { $$ = $1 + 1; }
    | pointers KW_CONST   { $$ = $1; }   /* `T * const` — const ptr, ignored */
    ;

/* zero or more [N] suffixes, in source order (row-major). n==0 means scalar. */
array_suffix:
      /* empty */                 { $$.n = 0; }
    | array_suffix '[' ']'        { $$ = $1; if ($$.n < 8) $$.dims[$$.n++] = 0;        }  /* unsized — params only */
    | array_suffix '[' conditional_expr ']' {
          long v;
          if (!const_eval($3, &v)) msg_error(yylineno, "array size must be a constant expression");
          $$ = $1; if ($$.n < 8) $$.dims[$$.n++] = (int)v;   /* enum/macro/sizeof/arith ok */
      }
    ;

init_declarator_list:
      init_declarator                                { $$.head = $1; $$.n = 1; }
    | init_declarator_list ',' init_declarator       {
          decl *d = $1.head; while (d->next) d = d->next; d->next = $3;
          $$.head = $1.head; $$.n = $1.n + 1;
      }
    ;

init_declarator:
      pointers IDENT array_suffix {
          if (ts_typedef) {
              type *t = apply_pointers(cur_base, $1);
              t = build_array_type(t, $3.dims, $3.n);
              st_add_typedef($2, t);
              /* still produce a decl so list grouping works; it'll be ignored at the declaration level */
              $$ = ast_decl(t, $2, NULL, yylineno);
          } else {
              $$ = make_decl(cur_base, $1, $2, $3.dims, $3.n, yylineno, NULL);
          }
      }
    | pointers IDENT array_suffix '=' assignment_expr {
          if (ts_typedef) msg_error(yylineno, "typedef cannot have an initializer");
          $$ = make_decl(cur_base, $1, $2, $3.dims, $3.n, yylineno, $5);
      }
    | pointers IDENT array_suffix '=' '{' '}' {
          /* empty brace init — leave memory at its .mif default (zero) */
          $$ = make_decl(cur_base, $1, $2, $3.dims, $3.n, yylineno, NULL);
      }
    | pointers IDENT array_suffix '=' '{' init_item_list '}' {
          /* aggregate initialiser: array or struct, one value per slot/field */
          decl *d = make_decl(cur_base, $1, $2, $3.dims, $3.n, yylineno, NULL);
          d->binit = $6;
          /* infer an unsized outer dimension `[]` from the initializer count */
          if (d->dtype && d->dtype->kind == TY_ARRAY && d->dtype->arr_size == 0 && $6 && $6->is_list)
              d->dtype->arr_size = $6->n;
          $$ = d;
      }
    | pointers IDENT array_suffix STRING_LIT {
          /* `int arr[N] "file.txt";` — initialise the array from a data file at synth time */
          decl *d = make_decl(cur_base, $1, $2, $3.dims, $3.n, yylineno, NULL);
          d->init_file = $4;
          $$ = d;
      }
    | '(' '*' IDENT array_suffix ')' '(' param_list ')' {
          /* function pointer `ret (*fp)(params)` — held as an int function id;
             with array_suffix, `ret (*arr[N])(params)` is an int array of ids */
          type *t = build_array_type(t_int(), $4.dims, $4.n);
          $$ = ast_decl(t, $3, NULL, yylineno);
      }
    | '(' '*' IDENT array_suffix ')' '(' param_list ')' '=' '{' init_item_list '}' {
          /* `ret (*arr[N])(params) = { f, g, ... }` */
          type *t = build_array_type(t_int(), $4.dims, $4.n);
          decl *d = ast_decl(t, $3, NULL, yylineno);
          d->binit = $11;
          if (t->kind == TY_ARRAY && t->arr_size == 0 && $11->is_list) t->arr_size = $11->n;
          $$ = d;
      }
    | pointers IDENT '(' param_list ')' {
          /* function declaration (prototype) — register, attach signature */
          type *ret = apply_pointers(cur_base, $1);
          sym *s = st_add(SK_FUNC, $2, $2, ret);
          s->n_params = $4.n;
          if ($4.n > 0) {
              s->param_types = malloc(sizeof(type*) * $4.n);
              int i = 0;
              for (decl *p = $4.head; p; p = p->next, i++) s->param_types[i] = p->dtype;
          }
          $$ = ast_decl(ret, $2, NULL, yylineno); /* placeholder; not registered as global var */
      }
    ;

/* comma-separated initialisers inside { }. Each value is a scalar expression
   or a nested brace list, optionally prefixed by one C99 designator
   (`.field =` / `[index] =`). */
init_val:
      assignment_expr               { $$ = ast_initz_expr($1, yylineno); }
    | '{' init_item_list '}'        { $$ = $2; }
    ;

init_item:
      init_val                          { $$.z = $1; $$.d.kind = DESIG_NONE;  $$.d.name = NULL; $$.d.idx = 0; }
    | '.' IDENT '=' init_val            { $$.z = $4; $$.d.kind = DESIG_FIELD; $$.d.name = $2;   $$.d.idx = 0; }
    | '[' INT_LIT ']' '=' init_val      { $$.z = $5; $$.d.kind = DESIG_INDEX; $$.d.name = NULL; $$.d.idx = (int)$2; }
    ;

init_item_list:
      init_item                       { $$ = ast_initz_list(yylineno); initz_add($$, $1.z, $1.d); }
    | init_item_list ',' init_item    { $$ = $1; initz_add($$, $3.z, $3.d); }
    ;

/* ----- function definition ------------------------------------------------ */

function_def:
      decl_specifiers pointers IDENT '(' param_list ')' {
          /* return type comes from $1 (a stack value): cur_base would be
             clobbered by the params' and body's own base_type reductions. */
          type *ret = apply_pointers($1, $2);
          sym *s = st_find($3);
          if (s && s->kind == SK_FUNC && s->defined)
              msg_error(yylineno, "redefinition of function '%s'%s", $3,
                  strcmp($3, "main") == 0 ? " (a program has exactly one main)" : "");
          if (s && s->kind != SK_FUNC)
              msg_error(yylineno, "'%s' redeclared as a function (already a variable)", $3);
          if (!s) {
              s = st_add(SK_FUNC, $3, $3, ret);
              s->n_params = $5.n;
              if ($5.n > 0) {
                  s->param_types = malloc(sizeof(type*) * $5.n);
                  int i = 0;
                  for (decl *p = $5.head; p; p = p->next, i++) s->param_types[i] = p->dtype;
              }
          }
          s->defined = 1;
          /* enter function scope and stage params for the body */
          st_enter_func($3);
          st_push_scope();
          for (decl *p = $5.head; p; p = p->next) {
              type *pt = p->dtype;
              if (pt->kind == TY_ARRAY) {
                  /* arrays decay to pointers in param position, but we keep the array
                     flavour so codegen can route LDA/STA correctly. The runtime
                     value is the base address either way. */
              }
              st_add(SK_PARAM, p->name, NULL, pt);
          }
          param_head = $5.head; param_tail = NULL; param_count = $5.n;
          ts_sclass = SC_NONE; ts_is_const = 0;
      }
      compound_stmt {
          /* finalize the function ($1 = decl_specifiers return type) */
          func *f = ast_func(apply_pointers($1, $2), $3, param_head, param_count, $8, yylineno);
          unit_add_func(f);
          st_pop_scope();
          st_leave_func();
          params_reset();
      }
    ;

param_list:
      /* empty */                            { $$.head = NULL; $$.n = 0; }
    | non_empty_param_list                   {
          /* `(void)` parses as a single abstract void param — normalise to 0 */
          if ($1.n == 1 && $1.head->dtype && $1.head->dtype->kind == TY_VOID) { $$.head = NULL; $$.n = 0; }
          else $$ = $1;
      }
    ;

non_empty_param_list:
      param_declarator                       { $$.head = $1; $$.n = 1; }
    | non_empty_param_list ',' param_declarator {
          decl *d = $1.head; while (d->next) d = d->next; d->next = $3;
          $$.head = $1.head; $$.n = $1.n + 1;
      }
    ;

param_declarator:
      KW_CONST param_declarator              { $$ = $2; }   /* `const T ...` — ignored */
    | base_type pointers IDENT array_suffix {
          type *t = apply_pointers($1, $2);
          t = build_array_type(t, $4.dims, $4.n);
          $$ = ast_decl(t, $3, NULL, yylineno);
      }
    | base_type pointers {
          /* abstract (unnamed) parameter — e.g. the `int` in `int (*f)(int)` */
          $$ = ast_decl(apply_pointers($1, $2), "_anon", NULL, yylineno);
      }
    | base_type '(' '*' IDENT ')' '(' param_list ')' {
          /* function-pointer parameter — int function ID */
          $$ = ast_decl(t_int(), $4, NULL, yylineno);
      }
    ;

/* ----- statements --------------------------------------------------------- */

stmt:
      compound_stmt                          { $$ = $1; }
    | selection_stmt                         { $$ = $1; }
    | iteration_stmt                         { $$ = $1; }
    | jump_stmt                              { $$ = $1; }
    | labeled_stmt                           { $$ = $1; }
    | expr_stmt                              { $$ = $1; }
    | KW_ASM '(' STRING_LIT ')' ';'          {
          /* basic inline assembly: the string is emitted verbatim into the
             .asm (split on '\n'). Operands must use asm-level names. */
          stmt *s = ast_stmt(S_ASM, yylineno); s->label = $3; $$ = s;
      }
    | KW_STATIC_ASSERT '(' conditional_expr ',' STRING_LIT ')' ';' { check_static_assert($3, $5, yylineno);  $$ = ast_stmt(S_NULL, yylineno); }
    | KW_STATIC_ASSERT '(' conditional_expr ')' ';'                { check_static_assert($3, NULL, yylineno); $$ = ast_stmt(S_NULL, yylineno); }
    ;

compound_stmt:
      '{' { st_push_scope(); } block_item_list '}' {
          stmt *s = ast_stmt(S_BLOCK, yylineno);
          s->items = $3.arr; s->n_items = $3.n;
          st_pop_scope();
          $$ = s;
      }
    | '{' '}' {
          stmt *s = ast_stmt(S_BLOCK, yylineno);
          s->items = NULL; s->n_items = 0;
          $$ = s;
      }
    ;

block_item_list:
      block_item                             { $$.arr = malloc(sizeof(stmt*)); $$.arr[0] = $1; $$.n = 1; }
    | block_item_list block_item             {
          $$.arr = realloc($1.arr, sizeof(stmt*) * ($1.n + 1));
          $$.arr[$1.n] = $2;
          $$.n = $1.n + 1;
      }
    ;

block_item:
      stmt                                   { $$ = $1; }
    | local_decl                             { $$ = $1; }
    ;

/* a local declaration produces an S_DECL stmt with chained decls in s->decls */
local_decl:
      decl_specifiers init_declarator_list ';' {
          stmt *s = ast_stmt(S_DECL, yylineno);
          if (ts_typedef) {
              /* typedef at local scope: registration already happened in init_declarator */
              s->decls = NULL;
          } else {
              s->decls = $2.head;
              /* publish names in current local scope */
              for (decl *d = $2.head; d; d = d->next)
                  st_add(SK_LOCAL_VAR, d->name, NULL, d->dtype);
          }
          ts_typedef = 0; ts_sclass = SC_NONE; ts_is_const = 0;
          $$ = s;
      }
    ;

expr_stmt:
      ';'                                    { $$ = ast_stmt(S_NULL, yylineno); }
    | expr ';'                               { stmt *s = ast_stmt(S_EXPR, yylineno); s->e1 = $1; $$ = s; }
    ;

selection_stmt:
      KW_IF '(' expr ')' stmt %prec IFX {
          stmt *s = ast_stmt(S_IF, yylineno); s->e1 = $3; s->body = $5; s->body2 = NULL; $$ = s;
      }
    | KW_IF '(' expr ')' stmt KW_ELSE stmt {
          stmt *s = ast_stmt(S_IF, yylineno); s->e1 = $3; s->body = $5; s->body2 = $7; $$ = s;
      }
    | KW_SWITCH '(' expr ')' stmt {
          stmt *s = ast_stmt(S_SWITCH, yylineno); s->e1 = $3; s->body = $5; $$ = s;
      }
    ;

labeled_stmt:
      KW_CASE expr ':' stmt {
          stmt *s = ast_stmt(S_CASE, yylineno); s->e1 = $2; s->body = $4; $$ = s;
      }
    | KW_DEFAULT ':' stmt {
          stmt *s = ast_stmt(S_DEFAULT, yylineno); s->body = $3; $$ = s;
      }
    | IDENT ':' stmt {
          stmt *s = ast_stmt(S_LABEL, yylineno); s->label = $1; s->body = $3; $$ = s;
      }
    ;

iteration_stmt:
      KW_WHILE '(' expr ')' stmt {
          stmt *s = ast_stmt(S_WHILE, yylineno); s->e1 = $3; s->body = $5; $$ = s;
      }
    | KW_DO stmt KW_WHILE '(' expr ')' ';' {
          stmt *s = ast_stmt(S_DOWHILE, yylineno); s->e1 = $5; s->body = $2; $$ = s;
      }
    | KW_FOR '(' opt_expr ';' opt_expr ';' opt_expr ')' stmt {
          stmt *s = ast_stmt(S_FOR, yylineno);
          if ($3) { stmt *is = ast_stmt(S_EXPR, yylineno); is->e1 = $3; s->init_stmt = is; }
          s->e1 = $5; s->e2 = $7; s->body = $9;
          $$ = s;
      }
    | KW_FOR '(' local_decl opt_expr ';' opt_expr ')' stmt {
          stmt *s = ast_stmt(S_FOR, yylineno);
          s->init_stmt = $3;
          s->e1 = $4; s->e2 = $6; s->body = $8;
          $$ = s;
      }
    ;

opt_expr:
      /* empty */                            { $$ = NULL; }
    | expr                                   { $$ = $1; }
    ;

jump_stmt:
      KW_BREAK ';'                           { $$ = ast_stmt(S_BREAK, yylineno); }
    | KW_CONTINUE ';'                        { $$ = ast_stmt(S_CONTINUE, yylineno); }
    | KW_RETURN ';'                          { $$ = ast_stmt(S_RETURN, yylineno); }
    | KW_RETURN expr ';'                     { stmt *s = ast_stmt(S_RETURN, yylineno); s->e1 = $2; $$ = s; }
    | KW_GOTO IDENT ';'                      { stmt *s = ast_stmt(S_GOTO, yylineno); s->label = $2; $$ = s; }
    ;

/* ----- expressions (C precedence cascade) -------------------------------- */

expr:
      assignment_expr                        { $$ = $1; }
    | expr ',' assignment_expr               { $$ = ast_comma($1, $3, yylineno); }
    ;

assignment_expr:
      conditional_expr                       { $$ = $1; }
    | unary_expr '=' assignment_expr         { $$ = ast_assign($1, $3, yylineno); }
    | unary_expr assign_op assignment_expr   {
          /* a OP= b  ⇒  a = a OP b */
          /* note: $1 is referenced twice — codegen treats this as if the lvalue
             is evaluated exactly once. for simple lvalues (IDENT, ARRAY, MEMBER,
             DEREF) this is safe semantically; for side-effecting lvalues it
             would double-evaluate. v1 accepts the simpler model. */
          $$ = ast_assign($1, ast_binop($2, $1, $3, yylineno), yylineno);
      }
    ;

assign_op:
      TOK_PLUSEQ      { $$ = OP_ADD; }
    | TOK_MINUSEQ     { $$ = OP_SUB; }
    | TOK_STAREQ      { $$ = OP_MUL; }
    | TOK_SLASHEQ     { $$ = OP_DIV; }
    | TOK_PERCENTEQ   { $$ = OP_MOD; }
    | TOK_AMPEQ       { $$ = OP_BAND; }
    | TOK_PIPEEQ      { $$ = OP_BOR; }
    | TOK_CARETEQ     { $$ = OP_BXOR; }
    | TOK_SHLEQ       { $$ = OP_SHL; }
    | TOK_SHREQ       { $$ = OP_SHR; }
    ;

conditional_expr:
      logical_or_expr                                  { $$ = $1; }
    | logical_or_expr '?' expr ':' conditional_expr    { $$ = ast_ternary($1, $3, $5, yylineno); }
    ;

logical_or_expr:
      logical_and_expr                       { $$ = $1; }
    | logical_or_expr TOK_LOR logical_and_expr { $$ = ast_binop(OP_LOR, $1, $3, yylineno); }
    ;

logical_and_expr:
      bit_or_expr                            { $$ = $1; }
    | logical_and_expr TOK_LAND bit_or_expr  { $$ = ast_binop(OP_LAND, $1, $3, yylineno); }
    ;

bit_or_expr:
      bit_xor_expr                           { $$ = $1; }
    | bit_or_expr '|' bit_xor_expr           { $$ = ast_binop(OP_BOR, $1, $3, yylineno); }
    ;

bit_xor_expr:
      bit_and_expr                           { $$ = $1; }
    | bit_xor_expr '^' bit_and_expr          { $$ = ast_binop(OP_BXOR, $1, $3, yylineno); }
    ;

bit_and_expr:
      equality_expr                          { $$ = $1; }
    | bit_and_expr '&' equality_expr         { $$ = ast_binop(OP_BAND, $1, $3, yylineno); }
    ;

equality_expr:
      relational_expr                        { $$ = $1; }
    | equality_expr TOK_EQ relational_expr   { $$ = ast_binop(OP_EQ, $1, $3, yylineno); }
    | equality_expr TOK_NE relational_expr   { $$ = ast_binop(OP_NE, $1, $3, yylineno); }
    ;

relational_expr:
      shift_expr                             { $$ = $1; }
    | relational_expr '<' shift_expr         { $$ = ast_binop(OP_LT, $1, $3, yylineno); }
    | relational_expr '>' shift_expr         { $$ = ast_binop(OP_GT, $1, $3, yylineno); }
    | relational_expr TOK_LE shift_expr      { $$ = ast_binop(OP_LE, $1, $3, yylineno); }
    | relational_expr TOK_GE shift_expr      { $$ = ast_binop(OP_GE, $1, $3, yylineno); }
    ;

shift_expr:
      additive_expr                          { $$ = $1; }
    | shift_expr TOK_SHL additive_expr       { $$ = ast_binop(OP_SHL, $1, $3, yylineno); }
    | shift_expr TOK_SHR additive_expr       { $$ = ast_binop(OP_SHR, $1, $3, yylineno); }
    ;

additive_expr:
      multiplicative_expr                    { $$ = $1; }
    | additive_expr '+' multiplicative_expr  { $$ = ast_binop(OP_ADD, $1, $3, yylineno); }
    | additive_expr '-' multiplicative_expr  { $$ = ast_binop(OP_SUB, $1, $3, yylineno); }
    ;

multiplicative_expr:
      cast_expr                              { $$ = $1; }
    | multiplicative_expr '*' cast_expr      { $$ = ast_binop(OP_MUL, $1, $3, yylineno); }
    | multiplicative_expr '/' cast_expr      { $$ = ast_binop(OP_DIV, $1, $3, yylineno); }
    | multiplicative_expr '%' cast_expr      { $$ = ast_binop(OP_MOD, $1, $3, yylineno); }
    ;

cast_expr:
      unary_expr                             { $$ = $1; }
    | '(' base_type pointers ')' cast_expr   {
          type *t = apply_pointers($2, $3);
          $$ = ast_cast(t, $5, yylineno);
      }
    | '(' base_type pointers ')' '{' init_item_list '}' {
          /* C99 compound literal (struct/scalar/pointer type) */
          type *t = apply_pointers($2, $3);
          $$ = ast_compound(t, $6, yylineno);
      }
    ;

unary_expr:
      postfix_expr                           { $$ = $1; }
    | TOK_INC unary_expr                     { $$ = ast_xfix(E_PREINC, $2, yylineno); }
    | TOK_DEC unary_expr                     { $$ = ast_xfix(E_PREDEC, $2, yylineno); }
    | '&' cast_expr                          { $$ = ast_addr($2, yylineno); }
    | '*' cast_expr                          { $$ = ast_deref($2, yylineno); }
    | '+' cast_expr                          { $$ = ast_unop(OP_POS, $2, yylineno); }
    | '-' cast_expr                          { $$ = ast_unop(OP_NEG, $2, yylineno); }
    | '~' cast_expr                          { $$ = ast_unop(OP_BNOT, $2, yylineno); }
    | '!' cast_expr                          { $$ = ast_unop(OP_LNOT, $2, yylineno); }
    | KW_SIZEOF unary_expr                   { $$ = ast_sizeof_e($2, yylineno); }
    | KW_SIZEOF '(' base_type pointers ')'   {
          type *t = apply_pointers($3, $4);
          $$ = ast_sizeof_t(t, yylineno);
      }
    | KW_NEW base_type pointers              {
          /* new T  ->  (T*)malloc(sizeof(T))  (ctor wiring comes with classes) */
          type *t = apply_pointers($2, $3);
          $$ = ast_cast(t_ptr(t), mk_call1("malloc", ast_sizeof_t(t, yylineno), yylineno), yylineno);
      }
    | KW_NEW base_type pointers '[' expr ']' {
          /* new T[n]  ->  (T*)malloc(n * sizeof(T)) */
          type *t = apply_pointers($2, $3);
          expr *sz = ast_binop(OP_MUL, $5, ast_sizeof_t(t, yylineno), yylineno);
          $$ = ast_cast(t_ptr(t), mk_call1("malloc", sz, yylineno), yylineno);
      }
    | KW_DELETE cast_expr                    { $$ = mk_call1("free", $2, yylineno); }
    | KW_DELETE '[' ']' cast_expr            { $$ = mk_call1("free", $4, yylineno); }
    ;

postfix_expr:
      primary_expr                           { $$ = $1; }
    | postfix_expr '[' expr ']'              { $$ = ast_index($1, $3, yylineno); }
    | postfix_expr '(' argument_list ')'     { $$ = ast_call($1, $3.arr, $3.n, yylineno); }
    | postfix_expr '.' IDENT                 { $$ = ast_member($1, $3, yylineno); }
    | postfix_expr TOK_ARROW IDENT           { $$ = ast_pmember($1, $3, yylineno); }
    | postfix_expr TOK_INC                   { $$ = ast_xfix(E_POSTINC, $1, yylineno); }
    | postfix_expr TOK_DEC                   { $$ = ast_xfix(E_POSTDEC, $1, yylineno); }
    ;

argument_list:
      /* empty */                            { $$.arr = NULL; $$.n = 0; }
    | non_empty_argument_list                { $$ = $1; }
    ;

non_empty_argument_list:
      assignment_expr                        { $$.arr = malloc(sizeof(expr*)); $$.arr[0] = $1; $$.n = 1; }
    | non_empty_argument_list ',' assignment_expr {
          $$.arr = realloc($1.arr, sizeof(expr*) * ($1.n + 1));
          $$.arr[$1.n] = $3;
          $$.n = $1.n + 1;
      }
    ;

primary_expr:
      IDENT                                  {
          /* enum constants become int literals; everything else is an IDENT */
          sym *s = st_find($1);
          if (s && s->kind == SK_ENUM_CONST) { $$ = ast_int_lit(s->enum_val, yylineno); free($1); }
          else $$ = ast_ident($1, yylineno);
      }
    | INT_LIT                                { $$ = ast_int_lit($1, yylineno); }
    | FLOAT_LIT                              { $$ = ast_float_lit($1, yylineno); }
    | CHAR_LIT                               { $$ = ast_char_lit($1, yylineno); }
    | STRING_LIT                             { $$ = ast_string_lit($1, g_str_len, yylineno); }
    | '(' expr ')'                           { $$ = $2; }
    | KW_GENERIC '(' assignment_expr ',' generic_assoc_list ')' {
          $$ = ast_generic($3, $5.ts, $5.es, $5.n, yylineno);
      }
    ;

/* C11 type-generic selection associations */
generic_assoc:
      base_type pointers ':' assignment_expr   { $$.t = apply_pointers($1, $2); $$.e = $4; }
    | KW_DEFAULT ':' assignment_expr           { $$.t = NULL; $$.e = $3; }   /* default */
    ;

generic_assoc_list:
      generic_assoc {
          $$.ts = malloc(sizeof(type*)); $$.es = malloc(sizeof(expr*));
          $$.ts[0] = $1.t; $$.es[0] = $1.e; $$.n = 1;
      }
    | generic_assoc_list ',' generic_assoc {
          $$.ts = realloc($1.ts, sizeof(type*) * ($1.n + 1));
          $$.es = realloc($1.es, sizeof(expr*) * ($1.n + 1));
          $$.ts[$1.n] = $3.t; $$.es[$1.n] = $3.e; $$.n = $1.n + 1;
      }
    ;

%%

void yyerror(const char *s)
{
    msg_error(yylineno, "%s", s);
}
