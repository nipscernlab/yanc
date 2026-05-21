#pragma yanc prname test12
// Tier 3: function templates (type-erased to a word), `auto`, range-for.
template<class T> T maxv(T a, T b) { return a > b ? a : b; }
template<typename T> T addv(T a, T b) { return a + b; }

template<class T>
struct Box { T v; };

void main(void) {
    out(0, maxv(3, 7));        // 7
    out(0, maxv(9, 2));        // 9
    out(0, addv(4, 5));        // 9

    Box b;                      // type-erased class template (1-word field)
    b.v = 42;
    out(0, b.v);                // 42

    auto x = 10;                // auto deduces int
    auto y = x + 5;
    out(0, y);                  // 15
    auto z = addv(100, 23);     // auto from a template call
    out(0, z);                  // 123

    int arr[4];
    arr[0] = 2; arr[1] = 4; arr[2] = 6; arr[3] = 8;
    int sum = 0;
    for (auto e : arr) sum = sum + e;     // range-for by value
    out(0, sum);                          // 20
    for (int& r : arr) r = r + 1;         // range-for by reference (mutates arr)
    out(0, arr[3]);                       // 9
}
