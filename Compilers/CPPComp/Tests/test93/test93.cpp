// test93: <iostream>, <cstdio> and <cstring> in a program that only writes
// (2026-10-20). The screen link (1) is used, the keyboard link (0) is not:
// <yanc_term>'s keyboard code is never called and leaves the processor, and
// before the fix its `#LKSYNC 0` stayed behind and asmcomp refused the
// program ("#LKSYNC 0, but the program has no TOM or CAD on link 0"). The
// text goes to the testbench's toma_1.txt; the string functions are checked
// on port 0. Expected (hand-computed), b = "abc" then "abcdef":
//   strlen("abcdef") 6; strcmp(b, "abcdef") 0; strcmp("abd", b) > 0: 1;
//   strcmp("ab", b) < 0: 1; strncmp(b, "abX", 2) 0; strchr(b, 'd') - b 3;
//   strrchr(r, 'X') - r 3 (r = "aXbX"); strstr(b, "cde") - b 2;
//   strstr(b, "zz") == 0: 1; strncpy(a, "xy", 5): a[1] 'y' 121, a[4] 0;
//   strncat(a, "12345", 2): "xy12", strlen 4; memcmp equal 0, memcmp(m, n, 3)
//   -1 (3 < 4); memmove overlapping right: 1 1 2 3; memchr finds 3 at index 3
#include <iostream>
#include <cstdio>
#include <cstring>
using namespace std;

void main(void)
{
    char a[16], b[16];
    const char *r = "aXbX";
    int m[4] = {1, 2, 3, 4};
    int n[4] = {1, 2, 4, 0};
    cout << "test93 " << 42 << endl;
    puts("puts");
    putchar('!'); putchar(10);
    strcpy(b, "abc");
    strcat(b, "def");
    out(0, strlen(b));
    out(0, strcmp(b, "abcdef"));
    out(0, strcmp("abd", b) > 0);
    out(0, strcmp("ab", b) < 0);
    out(0, strncmp(b, "abX", 2));
    out(0, strchr(b, 'd') - b);
    out(0, strrchr(r, 'X') - r);
    out(0, strstr(b, "cde") - b);
    out(0, strstr(b, "zz") == 0);
    strncpy(a, "xy", 5);
    out(0, a[1]); out(0, a[4]);
    strncat(a, "12345", 2);
    out(0, strlen(a));
    out(0, memcmp(m, m, 4));
    out(0, memcmp(m, n, 3));
    memmove(m + 1, m, 3);
    out(0, m[0]); out(0, m[1]); out(0, m[2]); out(0, m[3]);
    out(0, (int*)memchr(m, 3, 4) - m);
}
