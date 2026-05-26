// Function overloading (by arity and by parameter type) + default arguments.
#pragma yanc prname test8
int   add(int a, int b)          { return a + b; }
int   add(int a, int b, int c)   { return a + b + c; }   // overload by arity
float add(float a, float b)      { return a + b; }        // overload by type
int   neg(int x)                 { return -x; }           // not overloaded
int   scale(int x, int k = 2)    { return x * k; }        // default argument

void main(void) {
    out(0, add(2, 3));                      // 5
    out(0, add(2, 3, 4));                   // 9
    out(0, (int)(add(1.5, 2.5) * 10.0));    // 40  (float overload -> 4.0)
    out(0, neg(7));                         // -7
    out(0, scale(5));                       // 10  (default k=2)
    out(0, scale(5, 3));                    // 15
}
