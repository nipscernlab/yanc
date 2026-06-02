// ----------------------------------------------------------------------------
// CPPComp — type system --------------------------------------------------------
// ----------------------------------------------------------------------------

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../Headers/types.h"
#include "../Headers/messages.h"

static void *xcalloc(size_t n)
{
    void *p = calloc(1, n);
    if (!p) msg_internal("out of memory");
    return p;
}

static char *xstrdup(const char *s)
{
    if (!s) return NULL;
    char *r = (char*)malloc(strlen(s) + 1);
    if (!r) msg_internal("oom");
    strcpy(r, s); return r;
}

// ----------------------------------------------------------------------------
// canonical singletons -------------------------------------------------------
// ----------------------------------------------------------------------------

static type *make_basic(type_kind k, int is_signed)
{
    type *t = (type*)xcalloc(sizeof(type));
    t->kind = k; t->is_signed = is_signed;
    return t;
}

static type *T_VOID  = NULL;
static type *T_INT   = NULL;
static type *T_UINT  = NULL;
static type *T_FLOAT = NULL;
static type *T_CHAR  = NULL;

type *t_void (void) { if (!T_VOID)  T_VOID  = make_basic(TY_VOID,  0); return T_VOID;  }
type *t_int  (void) { if (!T_INT)   T_INT   = make_basic(TY_INT,   1); return T_INT;   }
type *t_uint (void) { if (!T_UINT)  T_UINT  = make_basic(TY_INT,   0); return T_UINT;  }
type *t_float(void) { if (!T_FLOAT) T_FLOAT = make_basic(TY_FLOAT, 1); return T_FLOAT; }
type *t_char (void) { if (!T_CHAR)  T_CHAR  = make_basic(TY_INT,   1); return T_CHAR;  }

type *t_ptr(type *to)
{
    type *t = (type*)xcalloc(sizeof(type));
    t->kind = TY_PTR; t->base = to;
    return t;
}

type *t_ref(type *to)
{
    type *t = t_ptr(to);
    t->is_ref = 1;
    return t;
}

type *t_auto(void)
{
    type *t = (type*)xcalloc(sizeof(type));
    t->kind = TY_INT; t->is_signed = 1; t->is_auto = 1;
    return t;
}

type *t_tparam(int idx)
{
    type *t = (type*)xcalloc(sizeof(type));
    t->kind = TY_INT; t->is_signed = 1; t->tparam = idx + 1;   // int-like until substituted
    return t;
}

type *t_array(type *of, int n)
{
    type *t = (type*)xcalloc(sizeof(type));
    t->kind = TY_ARRAY; t->base = of; t->arr_size = n;
    return t;
}

// ----------------------------------------------------------------------------
// structs --------------------------------------------------------------------
// ----------------------------------------------------------------------------

type *t_make_struct(char *tag)
{
    type *t = (type*)xcalloc(sizeof(type));
    t->kind = TY_STRUCT;
    t->tag  = xstrdup(tag);
    t->fields = NULL;
    t->size = -1;            // -1 = incomplete until t_struct_seal
    t->is_union = 0;
    return t;
}

type *t_make_union(char *tag)
{
    type *t = t_make_struct(tag);
    t->is_union = 1;
    return t;
}

static strct_field *append_field(type *st, char *name, type *ft)
{
    strct_field *f = (strct_field*)xcalloc(sizeof(strct_field));
    f->name = xstrdup(name);
    f->ftype = ft;
    f->next = NULL;
    if (!st->fields) st->fields = f;
    else { strct_field *p = st->fields; while (p->next) p = p->next; p->next = f; }
    return f;
}

void t_struct_add_field(type *st, char *name, type *ft) { append_field(st, name, ft); }

// insert a field at the FRONT (offset 0 after sealing) — used to place a vptr
void t_struct_prepend_field(type *st, char *name, type *ft)
{
    strct_field *f = (strct_field*)xcalloc(sizeof(strct_field));
    f->name = xstrdup(name); f->ftype = ft;
    f->next = st->fields;
    st->fields = f;
}

void t_struct_add_bitfield(type *st, char *name, type *ft, int width)
{
    strct_field *f = append_field(st, name, ft);
    f->is_bitfield = 1;
    f->bit_width   = width;
}

type *t_struct_seal(type *st, int word_bits)
{
    if (word_bits <= 0) word_bits = 16;
    if (st->is_union) {
        // every member overlaps at offset 0; size is the widest member
        int max = 0;
        for (strct_field *f = st->fields; f; f = f->next) {
            f->offset = 0; f->bit_pos = 0;
            int sz = f->is_bitfield ? 1 : type_size_words(f->ftype);
            if (sz > max) max = sz;
        }
        st->size = max;
    } else {
        int word = 0;   // current word index
        int bit  = 0;   // next free bit in the current word
        for (strct_field *f = st->fields; f; f = f->next) {
            if (f->is_bitfield) {
                if (f->bit_width == 0) { if (bit > 0) { word++; bit = 0; } continue; } // zero-width: align
                if (bit + f->bit_width > word_bits) { word++; bit = 0; }                // doesn't fit -> next word
                f->offset  = word;
                f->bit_pos = bit;
                bit += f->bit_width;
            } else {
                if (bit > 0) { word++; bit = 0; }   // flush a partially-filled word
                f->offset = word;
                word += type_size_words(f->ftype);
            }
        }
        if (bit > 0) word++;
        st->size = word;
    }
    return st;
}

strct_field *t_struct_find(type *st, const char *name)
{
    for (strct_field *f = st->fields; f; f = f->next)
        if (strcmp(f->name, name) == 0) return f;
    return NULL;
}

// ----------------------------------------------------------------------------
// helpers --------------------------------------------------------------------
// ----------------------------------------------------------------------------

int type_size_words(const type *t)
{
    if (!t) return 0;
    switch (t->kind) {
        case TY_VOID:   return 0;
        case TY_INT:    return 1;
        case TY_FLOAT:  return 1;
        case TY_PTR:    return 1;
        case TY_ARRAY:  return t->arr_size * type_size_words(t->base);
        case TY_STRUCT: return t->size > 0 ? t->size : 0;
        case TY_FUNC:   return 0;
    }
    return 0;
}

int type_is_int   (const type *t) { return t && t->kind == TY_INT;   }
int type_is_float (const type *t) { return t && t->kind == TY_FLOAT; }
int type_is_arith (const type *t) { return type_is_int(t) || type_is_float(t); }
int type_is_ptr_or_arr(const type *t) { return t && (t->kind == TY_PTR || t->kind == TY_ARRAY); }
int type_is_scalar(const type *t) { return t && t->kind != TY_STRUCT && t->kind != TY_ARRAY; }

// one-char signature code for a parameter type, used to mangle overloaded
// function names and to match a call's arguments to the right overload.
char type_code(const type *t)
{
    if (!t) return 'i';
    switch (t->kind) {
        case TY_FLOAT:  return 'f';
        case TY_PTR:    return 'p';   // covers references (a ref is a flagged ptr)
        case TY_ARRAY:  return 'p';
        case TY_STRUCT: return 'S';
        default:        return 'i';   // int / char / bool / enum all fold to one word
    }
}

int type_compatible(const type *a, const type *b)
{
    if (a == b) return 1;
    if (!a || !b) return 0;
    if (a->kind == b->kind) {
        if (a->kind == TY_PTR)   return type_compatible(a->base, b->base);
        if (a->kind == TY_ARRAY) return type_compatible(a->base, b->base);
        return 1;
    }
    // pointer ↔ array decay
    if ((a->kind == TY_PTR && b->kind == TY_ARRAY) || (a->kind == TY_ARRAY && b->kind == TY_PTR))
        return type_compatible(a->base, b->base);
    // int ↔ float silently coerce (we'll add casts on demand)
    if (type_is_arith(a) && type_is_arith(b)) return 1;
    // int ↔ pointer freely (C allows with warnings)
    if ((a->kind == TY_INT && (b->kind == TY_PTR || b->kind == TY_ARRAY))) return 1;
    if ((b->kind == TY_INT && (a->kind == TY_PTR || a->kind == TY_ARRAY))) return 1;
    return 0;
}

type *type_decay(type *t)
{
    if (t && t->kind == TY_ARRAY) return t_ptr(t->base);
    return t;
}

const char *type_str(const type *t)
{
    static char buf[256];
    if (!t) { snprintf(buf, sizeof(buf), "<null>"); return buf; }
    switch (t->kind) {
        case TY_VOID:   snprintf(buf, sizeof(buf), "void");   break;
        case TY_INT:    snprintf(buf, sizeof(buf), "int");    break;
        case TY_FLOAT:  snprintf(buf, sizeof(buf), "float");  break;
        case TY_PTR:    snprintf(buf, sizeof(buf), "%s*", type_str(t->base)); break;
        case TY_ARRAY:  snprintf(buf, sizeof(buf), "%s[%d]", type_str(t->base), t->arr_size); break;
        case TY_STRUCT: snprintf(buf, sizeof(buf), "struct %s", t->tag ? t->tag : "<anon>"); break;
        case TY_FUNC:   snprintf(buf, sizeof(buf), "<func>"); break;
    }
    return buf;
}
