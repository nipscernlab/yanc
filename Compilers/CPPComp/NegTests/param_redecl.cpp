// #131: a local named like the parameter hid it, and the parameter was lost
int f(int a) { int a = 5; return a; }
void main(void) { out(0, f(1)); }
