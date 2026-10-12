#pragma yanc prname test33
// Knowingly-complex example: a recursive-descent arithmetic evaluator that
// builds a polymorphic expression tree on the heap and evaluates it. Combines
// mutual recursion, virtual dispatch, a virtual destructor that frees the whole
// tree, operator precedence, and C-string pointer walking — all in tiny memory
// (each call builds then frees its tree, so the heap is reused).

class Node {
public:
    virtual int eval() = 0;
    virtual ~Node() {}
};

class Num : public Node {
    int val;
public:
    Num(int v) : val(v) {}
    int eval() override { return val; }
};

class BinOp : public Node {
    char  op;
    Node* l;
    Node* r;
public:
    BinOp(char o, Node* a, Node* b) : op(o), l(a), r(b) {}
    int eval() override {
        int a = l->eval();
        int b = r->eval();
        if (op == '+') return a + b;
        if (op == '-') return a - b;
        if (op == '*') return a * b;
        return b ? a / b : 0;
    }
    ~BinOp() override { delete l; delete r; }   // cascade-free the subtree
};

const char* cur;            // parser cursor

Node* parseExpr();          // forward decl (mutual recursion)

Node* parseFactor() {
    if (*cur == '(') {
        cur = cur + 1;                 // '('
        Node* e = parseExpr();
        cur = cur + 1;                 // ')'
        return e;
    }
    int n = 0;
    while (*cur >= '0' && *cur <= '9') {
        n = n * 10 + (*cur - '0');
        cur = cur + 1;
    }
    return new Num(n);
}

Node* parseTerm() {
    Node* n = parseFactor();
    while (*cur == '*' || *cur == '/') {
        char op = *cur; cur = cur + 1;
        n = new BinOp(op, n, parseFactor());
    }
    return n;
}

Node* parseExpr() {
    Node* n = parseTerm();
    while (*cur == '+' || *cur == '-') {
        char op = *cur; cur = cur + 1;
        n = new BinOp(op, n, parseTerm());
    }
    return n;
}

int evalStr(const char* s) {
    cur = s;
    Node* ast = parseExpr();
    int r = ast->eval();
    delete ast;                        // virtual dtor frees the whole tree
    return r;
}

void main(void) {
    out(0, evalStr("2+3*4"));          // 14  (precedence)
    out(0, evalStr("(2+3)*4"));        // 20  (parentheses)
    out(0, evalStr("10-2-3"));         // 5   (left associativity)
    out(0, evalStr("100/5/2"));        // 10
    out(0, evalStr("2*(3+4)-5"));      // 9
}
