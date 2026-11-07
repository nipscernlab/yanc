// A recursive function template and nothing else recursive (#128): its
// instance is made while main is generated, after the call stack's setup
// is written, which therefore has to be there already. Gave 0, and 1 before
// the instance knew it was recursive.

template <typename T> T fact(T n)
{
    if (n <= 1) return 1;
    T r = fact(n - 1);
    return n * r;
}

void main(void) { out(0, fact(5)); }   // 120
