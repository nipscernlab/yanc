// test94: global const arrays and string literals in the data memory
// (2026-10-20). A `const` array whose initializer is made of constants, and
// every string literal, is written by asmcomp straight into the data .mif
// (#arrayd) instead of being stored by code at main entry. A non-const array
// keeps its code (the .mif is not reloaded on reset). Expected (hand-computed):
//   ti = {3, -2, 'A'} in 5 words              -> 3 -2 65 0 0
//   tf = {1.5f, -0.25f, 4}, each times 8      -> 12 -2 32
//   ts = "Oi": its words and sizeof           -> 79 105 0 3
//   tm[2][3] = {{1,2,3},{4,5,6}}: [1][2], [0][1] -> 6 2
//   td[5] = {[3] = 9, 1}: [3], [4], [0]       -> 9 1 0
//   tb = {true, 0, 5}                         -> 1 0 1
//   tu = {4000000000u}: tu[0] > 3000000000u   -> 1
//   t8 = {200, -1} as int8_t                  -> -56 -1
//   mut = {5, 6, 7}, not const, mut[1]++      -> 7
//   a string literal through a pointer: "xyz"[2] -> 122
//   ms[] = "ab", not const: ms[1], sizeof, and after[] beside it untouched
//                                             -> 98 3 8 9
// Before: `char s[] = "..."` got no words (`#array ts 1 0`): ts read 0 8
// -2147483648, sizeof(ms) was 0, and the copy of "ab" wrote past the array.
#include <cstdint>

const int      ti[5]    = {3, -2, 'A'};
const float    tf[3]    = {1.5f, -0.25f, 4};
const char     ts[]     = "Oi";
const int      tm[2][3] = {{1, 2, 3}, {4, 5, 6}};
const int      td[5]    = {[3] = 9, 1};
const bool     tb[3]    = {true, 0, 5};
const unsigned tu[1]    = {4000000000u};
const int8_t   t8[2]    = {200, -1};
int            mut[3]   = {5, 6, 7};
char           ms[]     = "ab";
int            after[2] = {8, 9};

void main(void)
{
    for (int k = 0; k < 5; k++) out(0, ti[k]);
    for (int k = 0; k < 3; k++) out(0, (int)(tf[k] * 8.0f));
    for (int k = 0; k < 3; k++) out(0, ts[k]);
    out(0, (int)sizeof(ts));
    out(0, tm[1][2]); out(0, tm[0][1]);
    out(0, td[3]); out(0, td[4]); out(0, td[0]);
    for (int k = 0; k < 3; k++) out(0, tb[k]);
    out(0, tu[0] > 3000000000u);
    out(0, t8[0]); out(0, t8[1]);
    mut[1]++;
    out(0, mut[1]);
    const char *p = "xyz";
    out(0, p[2]);
    out(0, ms[1]); out(0, (int)sizeof(ms)); out(0, after[0]); out(0, after[1]);
}
