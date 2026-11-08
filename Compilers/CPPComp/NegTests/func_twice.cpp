// #134: a function defined twice
int f(int a) { return a; }
int f(int a) { return a + 1; }
void main(void) { out(0, f(1)); }
