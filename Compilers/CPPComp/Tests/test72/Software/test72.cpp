#pragma yanc prname test72
// INT_MIN / -1 is the one signed quotient that does not fit in a word. Verilog
// defines it as the wrapped result (INT_MIN) and Icarus computed that, but
// Verilator's runtime guarded the host divide trap and returned 0, so the same
// program printed different numbers under the two simulators (TODO item 11a).
// `ula_div` now names the case, and this test pins the answer on both.
//
// Everything here is a run-time value: a literal INT_MIN / -1 would be folded
// by the compiler, which is a separate question from what the hardware does.

int int_min(void)  { return -2147483647 - 1; }   // INT_MIN, unfoldable at the use
int minus_one(void) { return -1; }

void main(void) {
    int a = int_min();
    int b = minus_one();

    out(0, a / b);          // INT_MIN: the wrapped quotient
    out(0, a % b);          // 0
    out(0, a / 1);          // INT_MIN: no overflow, unchanged
    out(0, (a + 1) / b);    // INT_MAX: one above INT_MIN divides normally
    out(0, 6 / b);          // -6: an ordinary divide still works
    out(0, a / 2);          // -1073741824: INT_MIN by something else

    // the same guard must not fire on values that merely look alike
    out(0, 1073741824 / b); // -1073741824: MSB set only in the result
    out(0, b / b);          // 1: -1 / -1
    out(0, b / a);          // 0: -1 / INT_MIN truncates to zero
}
