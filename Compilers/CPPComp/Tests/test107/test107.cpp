// A recursion whose frames do not fit the call stack (#129). A recursive
// function keeps its frames in __cstk, 1024 words; down() has 16 locals, so
// a frame takes about 20 words and some 50 fit. down(30) fits and prints 30;
// down(100) needs more than fit, and the frame past the end ends the
// program, as a stack overflow ends a C++ program, instead of writing over
// the memory next to the stack and going on: neither 100 nor 7 comes out.
// (The calls stay below 128, the depth of the processor's own return stack.)

int down(int n)
{
    if (n == 0) return 0;
    int a = n, b = n + 1, c = n + 2, d = n + 3, e = n + 4, f = n + 5, g = n + 6, h = n + 7;
    int i2 = n + 8, j = n + 9, k = n + 10, l = n + 11, m = n + 12, o = n + 13, p = n + 14, q = n + 15;
    int r = down(n - 1);
    int s = a + b + c + d + e + f + g + h + i2 + j + k + l + m + o + p + q - 16 * n - 120;
    return r + 1 + s;
}

void main(void)
{
    out(0, down(30));
    out(0, down(100));
    out(0, 7);
}
