// Locks in the fixed CPPComp target: every sizeof==1, short/long fold to
// 32-bit (full-width arithmetic, no 16-bit overflow), IEEE-754 single float.
#pragma yanc prname test2
void main(void) {
    out(0, sizeof(char));        // 1
    out(0, sizeof(short));       // 1
    out(0, sizeof(int));         // 1
    out(0, sizeof(long));        // 1
    out(0, sizeof(long long));   // 1
    out(0, sizeof(float));       // 1
    out(0, sizeof(double));      // 1
    long  a = 1000000;           // 32-bit range, not 16-bit
    short b = 1000;
    out(0, a + b);               // 1001000  (full 32-bit arithmetic)
    float f = 3.5;
    out(0, (int)(f * 2.0));      // 7  (IEEE single)
}
