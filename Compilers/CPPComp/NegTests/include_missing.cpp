// A missing #include names the file and line that asked for it (#132).

#include "nope.h"

void main(void) { out(0, 1); }
