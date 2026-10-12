// test86: each stack as deep as the program goes (asm_depth, asm_share.h).
// top -> mid -> leaf is three calls deep, and arguments wait on the data
// stack across them; hardware.txt holds SDEPTH to the depth asm_depth works
// out. The data-stack depth is set by the pragma below, which must win.
#pragma yanc prname test86
#pragma yanc ndstac 20

int leaf(int a, int b) { if (a > 100) return a - b; return a + b; }
int mid(int a)         { int s = leaf(a, 1); if (s > 50) s = s - 50; return s + leaf(a, 2); }
int top(int a, int b, int c) { return mid(a) + b * c; }

void main(void) {
    out(0, top(1, 2, 3));      // mid(1) = 2 + 3 = 5; 5 + 6 = 11
    out(0, mid(60));           // leaf(60,1) = 61 -> 11; + 62 = 73
    out(0, top(200, 1, 1));    // mid(200) = 199 - 50 + 198 = 347; + 1 = 348
}
