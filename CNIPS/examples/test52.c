// Tagged-union "variant" value type (dynamic typing in C). Exercises an enum
// tag, a union nested inside a struct, struct-by-value params/return, member
// dispatch via switch, and mixed int/float coercion.
#pragma yanc prname tst52

enum { T_INT, T_FLOAT, T_STR };

struct value {
    int tag;
    union { int i; float f; char *s; } u;
};

struct value mk_int(int x)   { struct value v; v.tag = T_INT;   v.u.i = x; return v; }
struct value mk_float(float x){ struct value v; v.tag = T_FLOAT; v.u.f = x; return v; }

int as_int(struct value v) {
    switch (v.tag) {
    case T_INT:   return v.u.i;
    case T_FLOAT: return (int)v.u.f;
    case T_STR:   return v.u.s[0];      // first character
    }
    return -1;
}

struct value add(struct value a, struct value b) {
    float x = (a.tag == T_FLOAT) ? a.u.f : (float)a.u.i;
    float y = (b.tag == T_FLOAT) ? b.u.f : (float)b.u.i;
    return mk_float(x + y);
}

void main(void) {
    struct value a = mk_int(5);
    struct value b = mk_float(2.5);
    out(0, as_int(a));        // 5
    out(0, as_int(b));        // 2

    struct value c = add(a, b);   // 7.5
    out(0, as_int(c));        // 7
    out(0, c.tag);            // 1 (T_FLOAT)

    struct value s;
    s.tag = T_STR;
    s.u.s = "Hi";
    out(0, as_int(s));        // 'H' = 72
}
