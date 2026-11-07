// #134: a local array in a recursive function is refused (docs/language-cpp.md)
int f(int n)
{
    int t[4];
    t[0] = n;
    return n == 0 ? 0 : t[0] + f(n - 1);
}
void main(void) { out(0, f(3)); }
