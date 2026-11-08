// #134: a false static_assert stops the compilation
static_assert(sizeof(int) == 2, "a word is not two");
void main(void) { out(0, 1); }
