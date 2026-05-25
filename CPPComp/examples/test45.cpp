#pragma yanc prname test45
// In-class method/ctor prototypes paired with out-of-class definitions.
// Exercises:
//   - ctor declared inside class with default args, defined outside
//   - regular method declared inside, defined outside
//   - const-qualified method (fn_quals propagates)
//   - default args replayed from in-class decl onto the out-of-class def

class Counter {
public:
    Counter(int start = 10, int step = 2);
    int next();
    int peek() const;
private:
    int v_;
    int step_;
};

Counter::Counter(int start, int step) : v_(start), step_(step) {}

int Counter::next() {
    v_ = v_ + step_;
    return v_;
}

int Counter::peek() const { return v_; }

void main(void) {
    // all defaults
    Counter c;
    out(0, c.peek());   // 10
    out(0, c.next());   // 12
    out(0, c.next());   // 14

    // override one default
    Counter d(100);
    out(0, d.peek());   // 100
    out(0, d.next());   // 102

    // explicit args
    Counter e(0, 3);
    out(0, e.next());   // 3
    out(0, e.next());   // 6
}
