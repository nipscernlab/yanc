// ----------------------------------------------------------------------------
// CPPComp — symbol table -------------------------------------------------------
// ----------------------------------------------------------------------------

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "../Headers/symtab.h"
#include "../Headers/messages.h"

#define MAX_SCOPES 64

static sym *scopes[MAX_SCOPES];        // ordinary identifiers
static sym *tagscopes[MAX_SCOPES];     // struct tags (separate namespace)
static int  scope_n;
static char *cur_func;

static char *xstrdup(const char *s)
{
    char *r = (char*)malloc(strlen(s) + 1);
    if (!r) msg_internal("oom");
    strcpy(r, s); return r;
}

void st_init(void)
{
    for (int i = 0; i < MAX_SCOPES; i++) { scopes[i] = NULL; tagscopes[i] = NULL; }
    scope_n  = 1;
    cur_func = NULL;
}

void st_push_scope(void)
{
    if (scope_n >= MAX_SCOPES) msg_internal("scope too deep");
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

const char *st_current_func(void) { return cur_func; }

static sym *new_sym(sym_kind kind, const char *name, const char *asm_name, type *t)
{
    sym *s = (sym*)calloc(1, sizeof(sym));
    if (!s) msg_internal("oom");
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
    for (int i = scope_n - 1; i >= 0; i--)
        for (sym *s = scopes[i]; s; s = s->next)
            if (strcmp(s->name, name) == 0) return s;
    return NULL;
}

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
