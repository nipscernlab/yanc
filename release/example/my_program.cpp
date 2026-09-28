int main() {
    int acc = 0;
    for (int k = 1; k <= 10; ++k) acc += k;
    out(0, acc);            // writes 55 to output port 0
    return 0;
}
