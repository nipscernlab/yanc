// #134: there are no variadic functions (it said "syntax error")
int sum(int n, ...) { return n; }
void main(void) { out(0, sum(1, 2)); }
