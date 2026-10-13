// An overload of f taking an int is f_Oi in the .asm: a user function f_Oi took the same label.
int f(int a) { return a; }
float f(float a) { return a; }
int f_Oi(int a) { return a + 1; }
int main() { out(0, f(1) + f_Oi(1)); return 0; }
