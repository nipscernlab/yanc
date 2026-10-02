// test88: a recursive function nobody calls costs nothing. cppcomp sets up
// its software call stack (__cstk, 1024 words) in main when a function is
// recursive; the set-up sits in #IFLIVE <recursive functions> / #ENDLIVE, so
// asm_reach drops it, and the array, when none of them is reached.
// hardware.txt: no __cstk in the data memory, no multiplier.
#pragma yanc prname test88

int fact(int n) { if (n < 2) return 1; return n * fact(n - 1); }

int twice(int u) { return u + u; }

void main(void) {
    out(0, twice(21));         // 42
}
