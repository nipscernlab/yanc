// Kernighan & Pike's recursive regular-expression matcher (The Practice of
// Programming). Supports literal chars, '.' (any), 'c*' (zero-or-more), '^'
// (anchor start) and '$' (anchor end). match -> matchhere -> matchstar ->
// matchhere is a call-graph cycle, so all three get stack frames. Exercises
// `const char *` params, pointer indexing/arithmetic (re[0], re+2), the
// `*text++` post-increment-through-deref idiom, and do/while loops.
#pragma yanc prname tst64
#pragma yanc sdepth 128
#pragma yanc ndstac 128

int matchhere(const char *re, const char *text);
int matchstar(int c, const char *re, const char *text);

int match(const char *re, const char *text) {
    if (re[0] == '^') return matchhere(re + 1, text);
    do {
        if (matchhere(re, text)) return 1;
    } while (*text++ != '\0');
    return 0;
}

int matchhere(const char *re, const char *text) {
    if (re[0] == '\0') return 1;
    if (re[1] == '*') return matchstar(re[0], re + 2, text);
    if (re[0] == '$' && re[1] == '\0') return *text == '\0';
    if (*text != '\0' && (re[0] == '.' || re[0] == *text))
        return matchhere(re + 1, text + 1);
    return 0;
}

int matchstar(int c, const char *re, const char *text) {
    do {                                  // greedy-ish: try shortest first
        if (matchhere(re, text)) return 1;
    } while (*text != '\0' && (*text++ == c || c == '.'));
    return 0;
}

void main(void) {
    out(0, match("a*b", "aaab"));            // 1
    out(0, match("a*b", "b"));               // 1
    out(0, match("a.c", "abc"));             // 1
    out(0, match("^hello", "hello world"));  // 1
    out(0, match("world$", "hello world"));  // 1
    out(0, match("x*y", "z"));               // 0
    out(0, match(".*", "anything"));         // 1
    out(0, match("ab$", "abc"));             // 0
}
