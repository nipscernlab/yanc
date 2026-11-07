// Templates and the heap (#129). A string literal, a new, a static local and
// a function pointer inside a template, and a new that finds no memory. Each
// line printed is what C++ gives:
//
// out(0, first(1))        -> 66   "AB" in a function template: 'A' + 1
// out(0, *make(7))        -> 7    new in a function template, used nowhere else
// out(0, b.get())         -> 9    new in a class template's method
// out(0, b.initial())     -> 98   "box" in a class template's method: 'b'
// out(0, count(1))        -> 102  a static local of a template starts at 100
// out(0, apply(21))       -> 42   a function pointer taken in a template
// out(0, n), 4 times      -> 1 2 3 4
//                         the arena has 2048 words; the two news above took
//                         2 words each (a word holds a block's size), and
//                         each new int[500] takes 501: the fifth finds no
//                         memory and ends the program, as an uncaught
//                         std::bad_alloc does, so 99 never comes out.

template <typename T> int first(T x) { const char *s = "AB"; return s[0] + x; }
template <typename T> T *make(T v) { T *p = new T; *p = v; return p; }
template <typename T> struct Box {
    T *p;
    void set(T v) { p = new T; *p = v; }
    T get() { return *p; }
    int initial() { const char *s = "box"; return s[0]; }
};
template <typename T> T count(T step) { static T n = 100; n = n + step; return n; }
int twice(int v) { return 2 * v; }
template <typename T> T apply(T v) { int (*f)(int) = twice; return f(v); }

void main(void)
{
    out(0, first(1));
    out(0, *make(7));
    Box<int> b;
    b.set(9);
    out(0, b.get());
    out(0, b.initial());
    count(1);
    out(0, count(1));
    out(0, apply(21));
    int n = 0;
    for (int k = 0; k < 10; k++) {
        int *big = new int[500];
        big[499] = k;
        n = n + 1;
        out(0, n);
    }
    out(0, 99);
}
