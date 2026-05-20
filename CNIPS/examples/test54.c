// A hand-rolled string library (systems / pointers). Exercises raw char*
// pointer arithmetic, dereference loads and stores, char comparison, and
// building strings in a buffer.
#pragma yanc prname tst54

int my_strlen(char *s) {
    int n = 0;
    while (*s) { n = n + 1; s = s + 1; }
    return n;
}

int my_strcmp(char *a, char *b) {
    while (*a && *a == *b) { a = a + 1; b = b + 1; }
    return *a - *b;
}

void my_strcpy(char *d, char *s) {
    while (*s) { *d = *s; d = d + 1; s = s + 1; }
    *d = 0;
}

void my_strcat(char *d, char *s) {
    while (*d) d = d + 1;       // walk to the terminator
    my_strcpy(d, s);
}

int my_atoi(char *s) {
    int n = 0, sign = 1;
    if (*s == '-') { sign = -1; s = s + 1; }
    while (*s >= '0' && *s <= '9') { n = n * 10 + (*s - '0'); s = s + 1; }
    return sign * n;
}

void main(void) {
    out(0, my_strlen("hello"));      // 5
    out(0, my_strcmp("abc", "abc")); // 0
    out(0, my_strcmp("abc", "abd")); // -1

    char buf[32];
    my_strcpy(buf, "foo");
    my_strcat(buf, "bar");           // "foobar"
    out(0, my_strlen(buf));          // 6
    out(0, (int)buf[3]);             // 'b' = 98

    out(0, my_atoi("-123"));         // -123
    out(0, my_atoi("4567"));         // 4567
}
