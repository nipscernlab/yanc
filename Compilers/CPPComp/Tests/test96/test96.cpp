// A float literal is rounded once, from its text, as in C+- (issue #12).
// cppcomp read it into a host double and printed that with %.17g, so the
// assembler rounded a second time: a literal within 2^-53 of the point halfway
// between two SAPHO floats could come out one unit off. This one (72 digits)
// lies a hair below the halfway point between 1.0 and 1.0 + 2^-22 at 32/23/8,
// so it is 1.0; through the double it became 1.0000001192092896, above that
// point, and was encoded as 1.0 + 2^-22 (1967128577).
// Expected outputs: 1967128576 -180355072 1 1 (the words of 1.0 and -1.0)
#include <bit>

void main(void)
{
    float a = 1.0000001192092895507804029670527456996609316774993203580379486083984375f;
    float b = -1.0000001192092895507804029670527456996609316774993203580379486083984375f;
    out(0, std::bit_cast<int>(a));                // 1967128576, the word of 1.0
    out(0, std::bit_cast<int>(b));                // -180355072, the word of -1.0
    out(0, a == 1.0f);                            // 1
    out(0, b == -1.0f);                           // 1
}
