#pragma yanc prname test75
// An input word at or above 2^31 must arrive with its bits intact. The
// Verilator harness read the file with `fscanf("%d")` into an int, which
// saturates at INT_MAX on overflow: 3000000000, 2^31 and 4294967295 all
// became 0x7fffffff, three different words collapsed into one, while Icarus
// took the low 32 bits. The harness now reads wide and truncates, so both
// simulators deliver the same bits.
//
// The port has no type, so the bench writes signed decimal: 3000000000 comes
// back as -1294967296, the negative carrying the same bits. Comparing against
// a host run means printing the host value with %d too.
//
// `cmm_bigin` is the C+- twin of this test, run under Icarus with the same
// six input words. TODO item 11(b).

void main(void) {
    for (int k = 0; k < 6; k++) {
        int x = in(0);
        out(0, x);
    }
}
