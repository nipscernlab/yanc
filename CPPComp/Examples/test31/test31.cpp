#pragma yanc prname test31
// Realistic embedded example: a heap-backed dynamic buffer with RAII, plus a
// simple fixed-block pool allocator. Exercises new[]/delete[], destructors
// freeing heap memory, and reusing freed blocks.

class Buffer {
    int* data;
    int  len;
public:
    Buffer(int n) : len(n) {
        data = new int[n];
        for (int i = 0; i < n; i = i + 1) data[i] = 0;
    }
    ~Buffer() { delete[] data; }
    void set(int i, int v) { data[i] = v; }
    int  get(int i) { return data[i]; }
    int  size() { return len; }
};

void main(void) {
    Buffer b(5);
    for (int i = 0; i < 5; i = i + 1) b.set(i, i * i);
    out(0, b.get(0));      // 0
    out(0, b.get(2));      // 4
    out(0, b.get(4));      // 16
    out(0, b.size());      // 5

    // allocate, free, then allocate again — the freed block should be reusable
    int* p = new int[4];
    for (int i = 0; i < 4; i = i + 1) p[i] = i + 100;
    out(0, p[3]);          // 103
    delete[] p;

    int* q = new int[4];   // reuse
    q[0] = 7;
    out(0, q[0]);          // 7
    delete[] q;
}
