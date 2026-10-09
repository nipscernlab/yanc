// A global named like an instruction: the assembler read SET ADD as two instructions (output 0 instead of 5).
#pragma yanc prname p
int ADD;
int main() { ADD = 5; out(0, ADD); return 0; }
