// #131: a function named like a template was "syntax error"
template <typename T> int h(T a) { return 1; }
int h(int a) { return 2; }
void main(void) { out(0, h(5)); out(0, h(1.5f)); }
