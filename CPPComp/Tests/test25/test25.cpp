#pragma yanc prname test25
// Realistic embedded example: a UART-style command classifier.
// Compares an incoming command string against a known set and returns a code,
// then builds a short reply into a mutable char buffer. Exercises const char*,
// string literals, pointer deref/arithmetic, and a writable char[] buffer.

int streq(const char* a, const char* b) {
    while (*a && *b) {
        if (*a != *b) return 0;
        a = a + 1;
        b = b + 1;
    }
    return *a == *b;            // equal only if both hit '\0' together
}

int classify(const char* cmd) {
    if (streq(cmd, "on"))    return 1;
    if (streq(cmd, "off"))   return 2;
    if (streq(cmd, "reset")) return 9;
    return 0;                  // unknown
}

int copy_str(char* dst, const char* src) {
    int n = 0;
    while (*src) {
        *dst = *src;
        dst = dst + 1;
        src = src + 1;
        n = n + 1;
    }
    *dst = 0;
    return n;                  // length copied
}

void main(void) {
    out(0, classify("on"));      // 1
    out(0, classify("off"));     // 2
    out(0, classify("reset"));   // 9
    out(0, classify("xyz"));     // 0

    char buf[8];
    int len = copy_str(buf, "ack");
    out(0, len);                 // 3
    out(0, buf[0]);              // 'a' = 97
    out(0, buf[1]);              // 'c' = 99
    out(0, buf[2]);              // 'k' = 107
    out(0, buf[3]);              // 0 (terminator)
}
