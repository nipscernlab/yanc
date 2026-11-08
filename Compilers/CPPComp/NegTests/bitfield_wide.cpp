// #131: a 40-bit field left word 0 empty
struct W { int a : 40; };
void main(void) { W w; w.a = 1; out(0, w.a); }
