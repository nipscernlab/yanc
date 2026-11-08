// #131: &g of an overloaded g called a label that did not exist
int g(int a) { return a + 1; }
int g(float a) { return 7; }
void main(void) { int (*fp)(int) = &g; out(0, fp(3)); }
