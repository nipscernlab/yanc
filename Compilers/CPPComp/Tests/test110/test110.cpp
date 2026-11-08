// Enums as C++ has them (#134, #131). Each line printed is what C++ gives:
//
// out(0, f(c))      -> 6    Color is a type: Color c = BLUE, BLUE = GREEN + 1
// out(0, d)         -> 5    enum Color d, the C spelling, still works
// out(0, A), B      -> -1 0 a negative value, and the next counts on from it
// out(0, C)         -> 9    A + 10: any constant expression
// out(0, D)         -> 8    1 << 3
// out(0, (int)S::Y) -> 3    an enum class with an underlying type

enum Color { RED, GREEN = 5, BLUE };
enum E : int { A = -1, B, C = A + 10, D = 1 << 3 };
enum class S : unsigned { X = 2, Y };

int f(Color c) { return c; }

void main(void)
{
    Color c = BLUE;
    enum Color d = GREEN;
    out(0, f(c));
    out(0, d);
    out(0, A);
    out(0, B);
    out(0, C);
    out(0, D);
    out(0, (int)S::Y);
}
