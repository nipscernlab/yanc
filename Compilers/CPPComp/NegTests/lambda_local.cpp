// #134: a lambda is accepted only as a global initializer
void main(void)
{
    int k = []() { return 3; }();
    out(0, k);
}
