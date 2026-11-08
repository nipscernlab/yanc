// <iostream> and <cstdio> on the links (#134): with YANC_LINK_SYNC 0, std::cin
// reads link 0, which the testbench feeds from cade_0.txt, and std::cout writes
// link 1, which it writes to toma_1.txt (golden_toma_1.txt). YANC_ECHO 0: what
// is read is not written back. The input is "-12 4000000000 2.5 Z word",
// Enter, "abc def", Enter. Each line printed is what C++ gives:
//
// out(0, i)                    -> -12
// out(0, u == 4000000000u)     -> 1     an unsigned past 2^31
// out(0, (int)(f * 2.0f))      -> 5     2.5
// out(0, c)                    -> 90    'Z'
// out(0, w[0]), w[3], w[4]     -> 119 100 0   "word", ended
// out(0, cin.fail()), bad      -> 1 0   "abc" is no int: fail, 0, nothing taken
// out(0, cin.fail())           -> 0     after clear()
// out(0, cin.peek()), get()    -> 97 97 'a' seen, then taken
// out(0, cin.get())            -> 32    ignore(2) took "bc"; then the blank
// out(0, getchar())            -> 100   'd'
// out(0, line[0]), line[2], line[3] -> 101 10 0  fgets: "ef", Enter, the end
//
// and on the screen (toma_1.txt): put('A'), write("xyz", 2), oct 8 then dec
// 9, 1.2e45 (#131: the exponent stopped at 38), and puts("ok"), which adds
// the Enter: "Axy10 9 1.2e+45 ok" and Enter.

#define YANC_LINK_SYNC 0
#define YANC_ECHO 0
#include <iostream>
#include <cstdio>
using namespace std;

void main(void)
{
    int i; unsigned u; float f; char c; char w[8];
    cin >> i >> u >> f >> c >> w;
    out(0, i);
    out(0, u == 4000000000u);
    out(0, (int)(f * 2.0f));
    out(0, c);
    out(0, w[0]); out(0, w[3]); out(0, w[4]);
    int bad = 7;
    cin >> bad;
    out(0, cin.fail()); out(0, bad);
    cin.clear();
    out(0, cin.fail());
    out(0, cin.peek()); out(0, cin.get());
    cin.ignore(2);
    out(0, cin.get());
    out(0, getchar());
    char line[10];
    fgets(line, 10, stdin);
    out(0, line[0]); out(0, line[2]); out(0, line[3]);
    cout.put('A');
    cout.write("xyz", 2);
    cout << oct << 8 << dec << ' ' << 9 << ' ' << 1.2e45f << ' ';
    puts("ok");
}
