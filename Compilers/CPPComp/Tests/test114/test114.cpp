// cin >> a number too big for the word (#58), on a 16-bit processor: fail()
// is set and the value is the nearest the type holds, as C++ gives; before,
// the number wrapped (70000 came out 4464). The input, from cade_0.txt:
// "70000 65535 40000 -32768 -32769 99999999999999999999 -5". Each pair
// printed is (value, fail()), what C++ gives on a 16-bit int:
//
// unsigned 70000                -> 65535 1
// unsigned 65535                -> 65535 0
// int 40000                     -> 32767 1
// int -32768                    -> -32768 0
// int -32769                    -> -32768 1
// unsigned 99999999999999999999 -> 65535 1   past any word: every digit taken
// unsigned -5                   -> 65531 0   a minus wraps an unsigned, as in C++
//
// out() writes the word as signed: 65535 shows as -1, 65531 as -5.

#pragma yanc nubits 16
#define YANC_LINK_SYNC 0
#define YANC_ECHO 0
#include <iostream>
using namespace std;

void main(void)
{
    unsigned u; int i;
    cin >> u; out(0, (int)u); out(0, cin.fail()); cin.clear();
    cin >> u; out(0, (int)u); out(0, cin.fail()); cin.clear();
    cin >> i; out(0, i);      out(0, cin.fail()); cin.clear();
    cin >> i; out(0, i);      out(0, cin.fail()); cin.clear();
    cin >> i; out(0, i);      out(0, cin.fail()); cin.clear();
    cin >> u; out(0, (int)u); out(0, cin.fail()); cin.clear();
    cin >> u; out(0, (int)u); out(0, cin.fail()); cin.clear();
}
