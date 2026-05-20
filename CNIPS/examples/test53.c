// "Object-oriented" C: polymorphism via a method table of function pointers
// stored inside the struct (a hand-rolled vtable). Exercises function-pointer
// struct fields, pointer-to-struct, and indirect dispatch through a field.
#pragma yanc prname tst53

struct shape {
    int a, b;
    int (*area)(struct shape *);
    int (*perim)(struct shape *);
};

int rect_area (struct shape *s) { return s->a * s->b; }
int rect_perim(struct shape *s) { return 2 * (s->a + s->b); }
int sq_area   (struct shape *s) { return s->a * s->a; }
int sq_perim  (struct shape *s) { return 4 * s->a; }

void main(void) {
    struct shape r;
    r.a = 3; r.b = 4;
    r.area = rect_area; r.perim = rect_perim;

    struct shape q;
    q.a = 5; q.b = 0;
    q.area = sq_area; q.perim = sq_perim;

    // dispatch through the object's own method pointers
    out(0, r.area(&r));    // 12
    out(0, r.perim(&r));   // 14
    out(0, q.area(&q));    // 25
    out(0, q.perim(&q));   // 20

    // iterate over an array of shapes polymorphically
    struct shape shapes[2];
    shapes[0] = r;
    shapes[1] = q;
    int total = 0;
    for (int i = 0; i < 2; i = i + 1) total = total + shapes[i].area(&shapes[i]);
    out(0, total);         // 12 + 25 = 37
}
