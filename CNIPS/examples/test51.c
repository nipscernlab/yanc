// Infix expression evaluator, shunting-yard style (parsing). Single-digit
// operands, + - * / with precedence and parentheses. Exercises char parsing,
// two stacks (values and operators), and pointer out-parameters for the stack
// tops.
#pragma yanc prname tst51

int prec(char op) {
    if (op == '+' || op == '-') return 1;
    if (op == '*' || op == '/') return 2;
    return 0;
}

int apply(int a, int b, char op) {
    if (op == '+') return a + b;
    if (op == '-') return a - b;
    if (op == '*') return a * b;
    if (op == '/') return a / b;
    return 0;
}

// pop two values and one operator, push the result
void reduce(int *vals, int *vtop, char *ops, int *otop) {
    int b = vals[*vtop - 1]; *vtop = *vtop - 1;
    int a = vals[*vtop - 1]; *vtop = *vtop - 1;
    vals[*vtop] = apply(a, b, ops[*otop - 1]); *vtop = *vtop + 1;
    *otop = *otop - 1;
}

int eval(char *s) {
    int vals[32]; int vtop = 0;
    char ops[32]; int otop = 0;
    int i = 0;
    while (s[i] != 0) {
        char ch = s[i];
        if (ch >= '0' && ch <= '9') {
            vals[vtop] = ch - '0'; vtop = vtop + 1;
        } else if (ch == '(') {
            ops[otop] = ch; otop = otop + 1;
        } else if (ch == ')') {
            while (otop > 0 && ops[otop - 1] != '(') reduce(vals, &vtop, ops, &otop);
            otop = otop - 1;                       // discard '('
        } else {
            while (otop > 0 && prec(ops[otop - 1]) >= prec(ch)) reduce(vals, &vtop, ops, &otop);
            ops[otop] = ch; otop = otop + 1;
        }
        i = i + 1;
    }
    while (otop > 0) reduce(vals, &vtop, ops, &otop);
    return vals[0];
}

void main(void) {
    out(0, eval("3+4*2"));       // 11
    out(0, eval("2*3+4"));       // 10
    out(0, eval("(2+3)*4"));     // 20
    out(0, eval("8-4-2"));       // 2  (left associative)
    out(0, eval("2*3*4"));       // 24
    out(0, eval("9-2*3"));       // 3
    out(0, eval("(8-2)/3"));     // 2
}
