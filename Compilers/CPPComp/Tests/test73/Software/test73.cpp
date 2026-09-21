#pragma yanc prname test73
// INT_MIN / -1 is the one signed quotient that does not fit in a word. Verilog
// defines it as the wrapped result (INT_MIN) and Icarus computed that, but the
// C++ simulator's runtime guarded the host divide trap and returned 0, so the
// same program printed different numbers under the two simulators (TODO 11a).
// `ula_div` now names the case, and this test pins the answer on both.
//
// This is the twin of test72, routed to the OTHER simulator: the .in file
// beside it makes the regress simulate with Verilator, which is the one that
// used to disagree. Keep the two bodies in sync.
//
// The divisor comes from the input port, so nothing here can be folded at
// compile time, and reading it also keeps the `in`/`req_in` ports alive (the
// harness needs them; Verilator prunes top-level inputs a program never reads).

int int_min(void) { return -2147483647 - 1; }   // INT_MIN, unfoldable at the use

void main(void) {
    int a = int_min();
    int b = in(0);          // -1, from test73.in

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
