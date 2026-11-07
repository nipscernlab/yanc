// Assigning to a const global, which the parser folds to its value, says
// so (#132: said "expression is not an lvalue").

const int k = 3;

void main(void) { k = 4; out(0, k); }
