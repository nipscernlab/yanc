// Circular FIFO queue / ring buffer (data structures). Exercises a struct with
// an embedded array, pointer-to-struct, modulo wrap-around, and pointer
// out-parameters.
#pragma yanc prname tst45
#define CAP 8

struct ring { int data[CAP]; int head; int tail; int count; };

void rb_init(struct ring *r) { r->head = 0; r->tail = 0; r->count = 0; }

int rb_push(struct ring *r, int v) {
    if (r->count == CAP) return 0;            // full
    r->data[r->tail] = v;
    r->tail = (r->tail + 1) % CAP;
    r->count = r->count + 1;
    return 1;
}

int rb_pop(struct ring *r, int *out) {
    if (r->count == 0) return 0;              // empty
    *out = r->data[r->head];
    r->head = (r->head + 1) % CAP;
    r->count = r->count - 1;
    return 1;
}

void main(void) {
    struct ring r;
    rb_init(&r);

    for (int i = 1; i <= 5; i = i + 1) rb_push(&r, i * 10);   // 10..50
    out(0, r.count);          // 5

    int v;
    rb_pop(&r, &v); out(0, v);   // 10
    rb_pop(&r, &v); out(0, v);   // 20

    rb_push(&r, 60);
    rb_push(&r, 70);
    out(0, r.count);          // 5

    int sum = 0;
    while (rb_pop(&r, &v)) sum = sum + v;     // drain 30+40+50+60+70
    out(0, sum);              // 250
    out(0, r.count);          // 0
    out(0, rb_pop(&r, &v));   // 0 (empty)
}
