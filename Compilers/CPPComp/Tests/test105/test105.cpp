#include <limits>
// The preprocessor and the lexer of #130 (and two items of #129). This file
// starts with the UTF-8 mark (EF BB BF) Windows editors write, and its first
// line is an #include that has to be read. Each line printed is what C++ gives:
//
// out(0, K)                 -> 5      a /* */ after a #define may span lines
// out(0, ADD(1, 2))         -> 3      a macro call goes on in the next line
// out(0, x)                 -> 3      #define x x + 1 is not expanded again
// out(0, PP_OK)             -> 1      #if reads 010 as 8, 64-bit hex, '\'' and true
// out(0, SKIP_OK)           -> 1      an #if in a skipped group is not evaluated
// out(0, '\x41'), '\101'    -> 65 65  hex and octal escapes in a char
// out(0, '\a')              -> 7
// out(0, s[0] + s[1] + s[2]) -> 137   "\x41\101\a" is 65, 65, 7
// out(0, max == 2147483647) -> 1      <limits>, included on line 1
// out(0, sqrt(16))          -> 4      <cmath>, included through a macro

/*
#include "not_there.h"
*/
#define K 5 /* a comment that
   goes on */
#define ADD(a, b) ((a) + (b))
#define HDR <cmath>
#include HDR
#
#pragma not_yanc ignored
#if 010 == 8 && 0xFFFFFFFF != 0x7FFFFFFF && '\'' == 39 && '\x41' == 65 && true
#define PP_OK 1
#else
#define PP_OK 0
#endif
#if 0
#if this is not evaluated (
#endif
#elif 1
#define SKIP_OK 1
#endif

void main(void)
{
    int x = 2;
#define x x + 1
    out(0, K);
    out(0, ADD(1,
               2));
    out(0, x);
    out(0, PP_OK);
    out(0, SKIP_OK);
    out(0, '\x41');
    out(0, '\101');
    out(0, '\a');
    const char *s = "\x41\101\a";
    out(0, s[0] + s[1] + s[2]);
    out(0, std::numeric_limits<int>::max() == 2147483647);
    out(0, (int)(sqrt(16.0f) + 0.5f));
}
