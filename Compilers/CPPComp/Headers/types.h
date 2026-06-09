// ----------------------------------------------------------------------------
// CPPComp — type system --------------------------------------------------------
// ----------------------------------------------------------------------------
// All types reduce to one of a small set of base kinds. The processor only
// has NUBITS-wide words, so int/char/short/long all share the int kind and
// the type just keeps a flag for signedness. Pointers and arrays wrap a base
// type. Structs are a named field list with offsets resolved at decl time.
// ----------------------------------------------------------------------------

#ifndef CPPCOMP_TYPES_H
#define CPPCOMP_TYPES_H

typedef enum {
    TY_VOID  = 0,
    TY_INT   = 1,    // 1-word integer (NUBITS wide)
    TY_FLOAT = 2,    // 1-word float   (NBMANT+NBEXPO+1 wide)
    TY_PTR   = 3,    // pointer to a base type
    TY_ARRAY = 4,    // fixed-size array of a base type
    TY_STRUCT= 5,    // struct (tagged)
    TY_FUNC  = 6     // function type
} type_kind;

typedef struct type type;
typedef struct strct_field strct_field;

struct type {
    type_kind kind;
    type     *base;        // PTR target; ARRAY element type; FUNC return
    int       arr_size;    // ARRAY length (0 = unknown — only legal in params)
    int       is_const;
    int       is_signed;   // for TY_INT
    int       is_ref;      // C++ reference: a TY_PTR that auto-derefs on use
    int       is_auto;     // C++ `auto`: real type deduced from the initializer
    int       tparam;      // >0: template type parameter (index+1); behaves as int
                           // until substituted by a concrete type at instantiation

    // struct-specific
    char         *tag;     // struct/union tag name (e.g. "point")
    strct_field  *fields;  // linked list, in declaration order
    int           size;    // struct/union size in words (set when fields are sealed)
    int           is_union;// 1 = union (all fields share offset 0, size = max field)
    type         *base_class; // C++ single inheritance: the parent class (or NULL)
    char        **vtbl;       // virtual method names by slot (polymorphic class)
    int           n_vtbl;     // number of virtual slots (0 = not polymorphic)
    char        **statics;    // names of static data members (shared globals)
    int           n_statics;

    // function-specific
    type **params;
    int    n_params;
    int    is_variadic;    // currently always 0; reserved
};

struct strct_field {
    char        *name;
    type        *ftype;
    int          offset;     // word offset from the start of the struct
    int          is_bitfield;// 1 = packed bitfield
    int          bit_pos;    // start bit within the word (bitfields only)
    int          bit_width;  // width in bits (bitfields only)
    void        *dinit;      // default member initializer (expr*); NULL when none
    int          dzero;      // 1 = aggregate `= {}` default: zero-fill all words in the ctor
    strct_field *next;
};

// canonical singletons
type *t_void(void);
type *t_int (void);
type *t_uint(void);    // unsigned int (is_signed = 0)
type *t_float(void);
type *t_char(void);    // 1-word signed (CHAR_BIT == NUBITS on this target)

type *t_ptr  (type *to);
type *t_ref  (type *to);   // C++ T& — a pointer flagged is_ref (auto-deref)
type *t_auto (void);       // C++ auto — placeholder, deduced from initializer
type *t_tparam(int idx);   // template type parameter #idx (int-like until substituted)
type *t_array(type *of, int n);

type *t_make_struct(char *tag);                                  // forward decl
type *t_make_union (char *tag);                                  // union variant
void  t_struct_add_field(type *st, char *name, type *ft);        // returns offset via update
void  t_struct_prepend_field(type *st, char *name, type *ft);    // insert at offset 0 (vptr)
void  t_struct_add_bitfield(type *st, char *name, type *ft, int width); // packed bitfield
type *t_struct_seal(type *st, int word_bits);                    // lays out fields/bitfields, computes size
strct_field *t_struct_find(type *st, const char *name);

// helpers
int   type_size_words(const type *t);    // size in words (1 for scalar/ptr; N for arrays; struct size)
char  type_code(const type *t);          // 1-char signature code (overload mangling)
int   type_is_int   (const type *t);
int   type_is_float (const type *t);
int   type_is_arith (const type *t);     // int or float
int   type_is_ptr_or_arr(const type *t);
int   type_is_scalar(const type *t);     // not struct, not array
int   type_compatible(const type *a, const type *b);
const char *type_str(const type *t);     // human-readable, static buffer

// "decay": array-of-T → ptr-to-T (used in arg passing and rvalue contexts)
type *type_decay(type *t);

#endif
