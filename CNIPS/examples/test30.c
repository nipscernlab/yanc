// Stack-based bytecode VM (systems / interpreters). Exercises enum, struct,
// array-of-struct with nested initializers, struct-by-value copy, switch,
// functions, and 32-bit integers.
#pragma yanc prname tst30
#pragma yanc nubits 32
#pragma yanc nbmant 23
#pragma yanc nbexpo 8

enum { OP_PUSH, OP_ADD, OP_SUB, OP_MUL, OP_NEG, OP_DUP, OP_HALT };

struct ins { int op; int arg; };

int stack[32];
int sp = 0;

void push(int v) { stack[sp] = v; sp = sp + 1; }
int  pop(void)   { sp = sp - 1; return stack[sp]; }

// run a program to completion and return the value left on top of the stack
int run(struct ins *prog) {
    int pc = 0;
    for (;;) {
        struct ins in = prog[pc];      // struct copy by value
        pc = pc + 1;
        switch (in.op) {
        case OP_PUSH: push(in.arg); break;
        case OP_ADD:  { int b = pop(); int a = pop(); push(a + b); } break;
        case OP_SUB:  { int b = pop(); int a = pop(); push(a - b); } break;
        case OP_MUL:  { int b = pop(); int a = pop(); push(a * b); } break;
        case OP_NEG:  push(-pop()); break;
        case OP_DUP:  { int a = pop(); push(a); push(a); } break;
        case OP_HALT: return pop();
        }
    }
}

void main(void) {
    // (3 + 4) * 5 - 2 = 33
    struct ins p1[] = {
        {OP_PUSH, 3}, {OP_PUSH, 4}, {OP_ADD, 0},
        {OP_PUSH, 5}, {OP_MUL, 0},
        {OP_PUSH, 2}, {OP_SUB, 0},
        {OP_HALT, 0}
    };
    out(0, run(p1));     // 33

    // square via DUP: 9 * 9 = 81, then negate
    struct ins p2[] = {
        {OP_PUSH, 9}, {OP_DUP, 0}, {OP_MUL, 0},
        {OP_NEG, 0}, {OP_HALT, 0}
    };
    out(0, run(p2));     // -81
}
