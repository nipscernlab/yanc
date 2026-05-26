#pragma yanc prname test42
// static const / static constexpr int member inside a class. The grammar now
// has an explicit "static const" prefix in static_member so the const isn't
// pulled into field_decl's base_type by bison's default shift (field_decl has
// no initializer slot, which caused a syntax error on `static const int X = N`).

class Buf {
public:
    static const     int CAPACITY  = 10;
    static constexpr int THRESHOLD = 3;
    static           int counter   = 0;
};

void main(void) {
    // qualified access
    out(0, Buf::CAPACITY);              // 10
    out(0, Buf::THRESHOLD);             // 3
    out(0, Buf::counter);               // 0

    // mutate the non-const static
    Buf::counter = Buf::counter + Buf::THRESHOLD;
    out(0, Buf::counter);               // 3
    out(0, Buf::counter + Buf::CAPACITY); // 13
}
