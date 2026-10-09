// A global main_x and the local x of main were one word (output 14 instead of 8).
#pragma yanc prname p
int main_x = 3;
int main() { int x = 5; out(0, x + main_x); return 0; }
