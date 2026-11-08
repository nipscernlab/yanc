// #131: a typedef inside a function was seen in main too
void g(void) { typedef int T; T t = 1; out(0, t); }
void main(void) { T u = 2; out(0, u); g(); }
