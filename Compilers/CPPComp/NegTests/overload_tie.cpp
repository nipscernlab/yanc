// #131: f(1, 1) took the first overload, where C++ calls it ambiguous
int f(int a, float b) { return 1; }
int f(float a, int b) { return 2; }
void main(void) { out(0, f(1, 1)); }
