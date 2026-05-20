// CPPComp bootstrap smoke test — valid C subset for now (C++ features come next)
#pragma yanc prname test1

int add(int a, int b) { return a + b; }

void main(void) {
    int s = 0;
    for (int i = 1; i <= 10; i = i + 1) s = add(s, i);
    out(0, s);          // 55
    out(0, sizeof(int)); // 1 (word-addressed target)
}
