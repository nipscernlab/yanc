// ----------------------------------------------------------------------------
// CPPComp — symbol table -------------------------------------------------------
// ----------------------------------------------------------------------------

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "../Headers/symtab.h"
#include "../Headers/messages.h"
#include "../../common/xstr.h"

// one list per open scope, as deep as the program nests; scope 0 is global
static sym **scopes;                   // ordinary identifiers
static sym **tagscopes;                // struct tags (separate namespace)
static int   scope_n, scope_cap;
static int   scope_floor = 1;          // st_find looks from the top down to here, then global
static char *cur_func;

void st_init(void)
{
    scope_n  = 0;
    st_push_scope();                   // the global scope
    cur_func = NULL;
}

void st_push_scope(void)
{
    if (scope_n >= scope_cap) {        // scopes and tagscopes grow together
        int cap = scope_cap;
        XGROW(scopes, scope_n, cap);
        XGROW(tagscopes, scope_n, scope_cap);
    }
    scopes[scope_n] = NULL; tagscopes[scope_n] = NULL;
    scope_n++;
}

void st_pop_scope(void)
{
    if (scope_n <= 1) msg_internal("st_pop_scope at global");
    scope_n--;
}

void st_enter_func(const char *fn)
{
    if (cur_func) free(cur_func);
    cur_func = xstrdup(fn);
}

void st_leave_func(void) { if (cur_func) { free(cur_func); cur_func = NULL; } }
int  st_in_func   (void) { return cur_func != NULL; }

const char *st_current_func(void) { return cur_func; }

static sym *new_sym(sym_kind kind, const char *name, const char *asm_name, type *t)
{
    sym *s = xcalloc(1, sizeof(sym));
    s->kind = kind;
    s->name = xstrdup(name);
    s->asm_name = asm_name ? xstrdup(asm_name) : xstrdup(name);
    s->stype = t;
    return s;
}

sym *st_add(sym_kind kind, const char *name, const char *asm_name, type *t)
{
    sym *s = new_sym(kind, name, asm_name, t);
    int slot = (kind == SK_GLOBAL_VAR || kind == SK_FUNC || kind == SK_TYPEDEF || kind == SK_STRUCT_TAG)
              ? 0
              : scope_n - 1;
    if (kind == SK_STRUCT_TAG) {
        s->next = tagscopes[slot];
        tagscopes[slot] = s;
    } else {
        s->next = scopes[slot];
        scopes[slot] = s;
    }
    return s;
}

sym *st_add_enum(const char *name, long val)
{
    sym *s = st_add(SK_ENUM_CONST, name, name, NULL);
    s->enum_val = val;
    s->stype = NULL;
    return s;
}

sym *st_add_typedef(const char *name, type *t)
{
    return st_add(SK_TYPEDEF, name, name, t);
}

sym *st_add_tag(const char *tag, type *t)
{
    sym *s = st_add(SK_STRUCT_TAG, tag, tag, NULL);
    s->struct_t = t;
    return s;
}

sym *st_find(const char *name)
{
    for (int i = scope_n - 1; i >= 0; i = (i == scope_floor ? 0 : i - 1)) {
        for (sym *s = scopes[i]; s; s = s->next)
            if (strcmp(s->name, name) == 0) return s;
        if (i == 0) break;
    }
    return NULL;
}

// the scopes open now become invisible to st_find, but the global one, until
// st_restore_floor: a function body expanded inline sees its own names and
// the globals, never the locals of the function it is expanded in
int  st_hide_scopes(void)       { int old = scope_floor; scope_floor = scope_n; return old; }
void st_restore_floor(int old)  { scope_floor = old; }

sym *st_find_local(const char *name)
{
    int i = scope_n - 1;
    for (sym *s = scopes[i]; s; s = s->next)
        if (strcmp(s->name, name) == 0) return s;
    return NULL;
}

sym *st_find_tag(const char *tag)
{
    for (int i = scope_n - 1; i >= 0; i--)
        for (sym *s = tagscopes[i]; s; s = s->next)
            if (strcmp(s->name, tag) == 0) return s;
    return NULL;
}
