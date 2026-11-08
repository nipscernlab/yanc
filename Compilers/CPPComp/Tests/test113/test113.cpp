// sizeof of a string literal (#140): in C++ a literal of n characters is an
// array of n + 1, the last one its 0; here a char is a word. Each line printed
// is what C++ gives:
//
// out(0, sizeof("xyz"))           -> 4
// out(0, sizeof("ab" "cd"))       -> 5    adjacent literals are one
// out(0, sizeof(""))              -> 1    only the 0
// out(0, sizeof(buf))             -> 4    a char array sized by a constant sizeof
// out(0, sizeof(p))               -> 1    a pointer to the literal is a pointer
static_assert(sizeof("xyz") == 4, "a literal's sizeof counts its 0");

void main(void)
{
    out(0, sizeof("xyz"));
    out(0, sizeof("ab" "cd"));
    out(0, sizeof(""));
    char buf[sizeof("abc")];
    out(0, sizeof(buf));
    const char *p = "xyz";
    out(0, sizeof(p));
}
