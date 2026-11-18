// a std::vector's size is known only when the program runs: not yet (docs/design/nn-functions.md)
#include <vector>
int main() { std::vector<float> v; v.push_back(1.0f); relu(v); out(0, (int)v[0]); return 0; }
