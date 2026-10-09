// The local ADD of S is S_ADD in the .asm, an instruction, though neither name is one.
#pragma yanc prname p
int S(int ADD) { return ADD + 1; }
int main() { out(0, S(4)); return 0; }
