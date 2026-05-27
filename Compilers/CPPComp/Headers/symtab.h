// ----------------------------------------------------------------------------
// CPPComp — symbol table -------------------------------------------------------
// ----------------------------------------------------------------------------
// Lexical-scoped table: file scope at depth 0, function scope at 1, then
// nested blocks. Each symbol carries its assigned asm-level name (mangled
// for locals as <funcname>_<varname>).
// ----------------------------------------------------------------------------

#ifndef CPPCOMP_SYMTAB_H
#define CPPCOMP_SYMTAB_H

#include "ast.h"

typedef enum {
    SK_GLOBAL_VAR,
    SK_LOCAL_VAR,
    SK_PARAM,
    SK_FUNC,
    SK_TYPEDEF,
    SK_ENUM_CONST,
    SK_STRUCT_TAG
} sym_kind;

typedef struct sym {
    sym_kind kind;
    char    *name;        // C-level name
    char    *asm_name;    // name as it appears in the emitted .asm

    type    *stype;       // for variables: declared type; for funcs: return type
    int      is_const;    // variable declared `const`
    long     enum_val;    // for SK_ENUM_CONST
    type    *struct_t;    // for SK_STRUCT_TAG

    // for a local/param of a recursive function: lives in the stack frame
    int      is_frame;    // 1 = address is __fp + frame_off (not a fixed name)
    int      frame_off;   // word offset within the frame

    // function metadata
    int       n_params;
    type    **param_types;
    expr    **param_defaults;   // declaration-site default-arg expressions; NULL when none
    int       defined;

    struct sym *next;
} sym;

void st_init(void);

sym  *st_add    (sym_kind kind, const char *name, const char *asm_name, type *stype);
sym  *st_add_enum(const char *name, long val);
sym  *st_add_typedef(const char *name, type *t);
sym  *st_add_tag(const char *tag, type *t);
sym  *st_find_tag(const char *tag);

sym  *st_find       (const char *name);
sym  *st_find_local (const char *name);

void  st_push_scope (void);
void  st_pop_scope  (void);

void  st_enter_func (const char *fn);
void  st_leave_func (void);
const char *st_current_func(void);

#endif
