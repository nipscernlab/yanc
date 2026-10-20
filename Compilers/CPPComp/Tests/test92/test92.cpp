// test92: overload choices cppcomp got wrong, found while writing <iostream>
// (2026-10-20). Before the fix an overloaded method was called by its bare
// name, a label that does not exist; f(char) beside f(int) was "a
// redefinition" (char, bool and unsigned were one type with int for
// overloading, and true was an int); a reference parameter (int&, float&) never
// matched; << and >> could not be overloaded; and `a << x << y` could not
// chain on a reference result. Expected (hand-computed):
//   P::put(int) 3, put(float) 100, put(const char*) 200          -> 3 100 200
//   free f(int) / f(char) / f(bool) / f(unsigned) / f(float)
//     on 7, 'A', true, 7u, 2.5f                                  -> 1 2 3 4 5
//   f on a char variable picks f(char); g(int) alone takes a char -> 2 66
//   S << 5 << 'A' << "hi" << 7.5f, chained: 1005 2065 4104 4105 5007
//   R >> i >> x (int& and float& overloads): i = 41, x = 2       -> 41 2
//   an overloaded method called from a sibling method: 30        -> 30
//   an overloaded method with a default argument: k(1) = 1 + 10  -> 11
class P {
public:
    int v;
    void put(int x)         { v = x; }
    void put(float x)       { v = 100; }
    void put(const char *s) { v = 200; }
    int  twice(int x)       { return x + x; }
    int  twice(float x)     { return 0; }
    int  call()             { return twice(15); }
    int  k(int a, int b = 10) { return a + b; }
    int  k(float a)         { return 0; }
};

int f(int x)      { return 1; }
int f(char x)     { return 2; }
int f(bool x)     { return 3; }
int f(unsigned x) { return 4; }
int f(float x)    { return 5; }
int g(int x)      { return x; }

class S {
public:
    int n;
    S& operator<<(int x)         { out(0, 1000 + x); return *this; }
    S& operator<<(char c)        { out(0, 2000 + c); return *this; }
    S& operator<<(const char *s) { while (*s) { out(0, 4000 + *s); s++; } return *this; }
    S& operator<<(float x)       { out(0, 5000 + (int)x); return *this; }
};

class R {
public:
    int n;
    R& operator>>(int& x)   { x = 41; return *this; }
    R& operator>>(float& x) { x = 2.0f; return *this; }
};

S s;
R r;

void main(void)
{
    P p;
    p.put(3);       out(0, p.v);
    p.put(2.5f);    out(0, p.v);
    p.put("a");     out(0, p.v);
    out(0, f(7)); out(0, f('A')); out(0, f(true)); out(0, f(7u)); out(0, f(2.5f));
    char c = 'B';
    out(0, f(c)); out(0, g(c));
    s << 5 << 'A' << "hi" << 7.5f;
    int i = 0; float x = 0.0f;
    r >> i >> x;
    out(0, i); out(0, (int)x);
    out(0, p.call());
    out(0, p.k(1));
}
